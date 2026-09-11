// Main shared state and low-level compatibility implementation.
#include <Arduino.h>
//
//	Main Control INO
//


#include "Common.h"
#include "DF_Main.h"
#include "FileSys.h"
#include "Eeprom.h"
#include "ENow.h"
#include "ServoMotor.h"
#include "Potentiometer.h"
#include "BobbinMotor.h"
#include "Encoder.h"
#include "Led.h"
#include "Diag.h"
#include "LmMotor.h"
#include "Version.h"
#include <Update.h>
#include <FreeRTOSConfig.h>

//--- Structure Define

// 메인보드 문자열
String strMainBoard[10] =
{
	"",				// 0 = dummy
	"MBD_V10",		// 1
	"MBD_V11",		// 2
};

// LED Define 
typedef struct _ledCont
{
	unsigned int oldcont;			// Control Kind
	unsigned int cont;			// Control Kind
	unsigned int colorNo;		// Color No
	unsigned int colorMany;		// Color many
	unsigned int bTime;			// blinking Time
	unsigned int dTime;			// Diming Time
	unsigned int bStep;			// bllink STEP
	unsigned int dStep;			// Dimm STEP
	unsigned int dColorCnt;		// dimm Out Color [ Order]
	unsigned int dReqCnt;		//
	unsigned int rsv_e;
}stLedCont;
typedef stLedCont *ptrLedCont;

//
#define LED_CONTROL_10MS	10
#define LED_CONTROL_20MS	20
#define LED_CONTROL_50MS	50


//--- LED
extern const stLedCont initledCont[LED_IDX_MAX] =
{
	{0, },								// 0 -dummy
	// old,cont,...ColorNo,.....ColorMany,....... blink Time, Dim Time,..... bStep, dStep , dClrCnt, dReqCnt, Resrved
	{ 3, 3, 	3, 		6, 		500,	3000, 	0,	0,	1,	(3000/2/LED_CONTROL_10MS), 0	},	// 1- Cent		, Dimming
	{ 1, 1, 	3, 		1, 		500, 	3000, 	0,	0,	3,	(3000/2/LED_CONTROL_10MS), 0	},	// 2- Buttom	, ON
	{ 0, 0, 	3, 		1, 		500, 	3000,	0,	0,	0,	(3000/2/LED_CONTROL_10MS), 0	},	// 3- Left		, OFF
	{ 0, 0, 	3, 		1, 		500, 	3000, 	0,	0,	0,	(3000/2/LED_CONTROL_10MS), 0	},	// 4 -Right		, OFF
};

stLedCont ledCont[LED_IDX_MAX];

unsigned int ledLight=100;				// Light [%]

unsigned int ledDimmHoldTime[LED_IDX_MAX] = 	// Hold Time 100ms
{ 
	10,		// 0 -Dummy
	//
	10,		// 1- Center
	10,		// 2-Buttom
	10,		// 3- Left
	10,		// 4-Right
};

#define COLOR_ORDER	10
unsigned int ledColorOrder[LED_IDX_MAX][COLOR_ORDER] = 
{
	{0,},
	//
	{0,1,2,3,4,5,6,7,8,9},		// 1- Center
	{0,1,2,3,4,5,6,7,8,9},		// 2-Buttom
	{0,1,2,3,4,5,6,7,8,9},		// 3-Left
	{0,1,2,3,4,5,6,7,8,9},		// 4-Right
};
//---LED END---

//--- i2c EEPROM

// Define Diag Code
typedef struct _diagCode
{
	int catNo;			// XX-
	int itemNo;			//  -YY
	int act;			//     -A
	int para;			//       -PPP
}stDiagCode;
typedef stDiagCode *ptrDiagCode;


//=== CREATOR OBJECT
ENow eNow;
Potentiometer torqMotor;		// torq Motor

ServoMotor sMotor;				// servoMortor객체

BobbinMotor bbnMotor;					//Bobbin Motor(BLDC DC)
Encoder mainEnc;				// Bobbin Encoder + LM Encoder
//Encoder lmEnc;

LmMotor lmMotor;		// LM MOT객체
// LM Encoder 제어는?

Led extLed;		// RGB

Eeprom eNvm;

//Diag mDiag;		//Mode Diag Object

FileSys fsInfo;	// manufactory Info

//------------------------------------------------------------

//=== ## ESP Now Data
//now_message now_msg;

String eulerStr;
//String battLvlStr = "999";
String battLvlStr = "-1";			// 배터리잔량 문자열값
//String oldbattLvlStr;

int ibatteryLvl = UNKNOWN;			// 현재 배터리 잔량
int oldibatteryLvl = UNKNOWN;		// old 배터리 잔량
int ibatChargeLvl = UNKNOWN;		// 충전표시용 Level

//---------------------------------------------------------
// 방향 : 성공/에러
#define ERR_OK		0
#define ERR_CW		-1
#define ERR_CCW		-2

// 정수 :  성공, 에러 방향
//---------------------------------------------------------
#define ERR_NG			-1
#define ERR_FAIL			ERR_NG
#define ERR_RETRY_OVER		ERR_NG
#define ERR_WIRE_BROKEN		ERR_NG

#define ERR_RANGE_OVER	-2
#define ERR_ENC_NOT_STABLE	ERR_RANGE_OVER

#define ERR_PARA_NG		-3

#define ERR_TO_OVER		-4
#define ERR_NOT_SIG_CHG	-5
#define ERR_NOT_ROTATE	-6
#define ERR_FS_WRITE	-7
#define ERR_FW_CAL_NG	-8

//---------------------------------------------------------
// 문자열 : 성공, 에러
#define STR_ERR_OK			"00"
#define STR_ERR_NG			"-1"
#define STR_ERR_FAIL		STR_ERR_NG
#define STR_ERR_RANGE_OVER	"-2"
#define STR_ERR_PARA_NG		"-3"


/*------------------------------------------------------------------
   PC-BOARD CDM Format (STRING)
   : STX(3 char)+Data(8~n char)+ETX(%)
      STX : 3 char, "$xx", (ex $01)
      Data : 8~n Char, "xxxxxxxx", (ex 11111111, 00000000)
      ETX : 1 char, ''%''
------------------------------------------------------------------*/
// Define PC-MAIN STX String
//
//=============== COMMAND ============================
// 2) STX_STR
String STX_MAINALIVE       = "$00";
//String STX_MAINCONN       = "$00";
String STX_INFO_REQ        = "$01";
	#define MAIN_IF_VER			"00"
	#define NUM_MAIN_IF_VER		0
	#define ROD_BOARD_TYPE		"01"
	#define NUM_ROD_BOARD_TYPE 	1
	#define ROD_REGIST_TO_SEC	"02"	// 릴등록 FW의 T/O시간 2문자(10~40)초, 기본값 25 SEC
	#define NUM_ROD_REGIST_TO_SEC	2
	#define IMU_INTVAL_TIME		"03"	// IMU 간격 4문자 (0000 ~ 1000)[ms]
	#define NUM_IMU_INTVAL_TIME	3

	#define MANUF_INFO_READ		"04"
	#define NUM_MANUF_INFO_READ	4
	#define MANUF_INFO_SAVE 	"05"
	#define NUM_MANUF_INFO_SAVE	5
	#define MANUF_INFO_DEL	 	"06"
	#define NUM_MANUF_INFO_DEL	6

	#define INSTALL_INFO_READ 		"07"
	#define NUM_INSTALL_INFO_READ 		7
	#define INSTALL_INFO_SAVE 		"08"
	#define NUM_INSTALL_INFO_SAVE 		8
	#define INSTALL_INFO_DEL		"09"
	#define NUM_INSTALL_INFO_DEL		9

	#define MAINMOT_INFO_READ		"10"			// MAIN MOT(TORQUE 기본값 읽기)
	#define NUM_MAINMOT_INFO_READ		10
	#define MAINMOT_INFO_SAVE		"11"			// MAIN MOT(TORQUE 기본값 쓰기)
	#define NUM_MAINMOT_INFO_SAVE		11

	#define MAIN_BOARD_TYPE		"12"			// 1-구보드, 2-신보드(TBD)
	#define NUM_MAIN_BOARD_TYPE 	12

	#define BLDC_MOT_LIMIT_READ			"13"			// BLDC 제한값 읽기)
	#define NUM_BLDC_MOT_LIMIT_READ		13
	#define BLDC_MOT_LIMIT_WRITE		"14"			// BLDC 제한값 쓰기)
	#define NUM_BLDC_MOT_LIMIT_WRITE	14
	

String STX_TORQUE_MOTOR   = "$02";
String STX_SERVO_MOTOR    = "$03";

String STX_BBN_MOTOR     = "$04";

String STX_BREAK_MOTOR    = "$05";
String STX_LED_CONT			= "$06";

String STX_RESERVE07		= "$07";	//RSV
String STX_IMU_SET			= "$08";

String STX_IMU_DATA_STS		= "$09";


String STX_VER_READ       = "$10";		// STX Version Read
String STX_GAME_STS       = "$11";		// STX GANE STATUS
String OLD_STX_BAT_LVL    = "$0C";		// BAT LVL
String STX_BAT_LVL        = "$12";		// BAT LVL
String STX_BUTT_SWC       = "$13";		// BUTTON SWITCH
String STX_HAND_ENC       = "$14";		// HANDLE ENCODER

String STX_DEV_CONT_REQ   = "$15";
String STX_GET_ADDR       = "$16";
String STX_SET_ADDR       = "$17";
String STX_APSTS_RESP     = "$18";
String STX_MOT_OUT_RATE_SET   = "$19";

String STX_ROD_CONN			="$20";
String STX_IMU_CONN			= "$21";
String STX_PWRON_STS        = "$22";			// PowerOn Reset STX
	#define CONT_MAIN_BOARD	1
	#define REEL_MAIN_BOARD 2

String STX_VRT_MOT_CONT      = "$23";			// VRT Motor ON/OFF Control
String STX_BTN_LED_CONT      = "$24";			// Button LED
	#define BTN_LED_ALL_POSI	0
	#define BTN_LED_LEFT_POSI	1
	#define BTN_LED_RIGHT_POSI	2
	#define BTN_LED_LAST_POSI	BTN_LED_RIGHT_POSI		// 주의, LED개수가 추가되면 수정필요.
	
String STX_GAME_STS_WAT     = "$25";		// STX GANE STATUS
	#define GAME_xxx	0

String STX_DEV_SET_REQ     = "$26";		// STX 장치 기능 요구
	#define STR_MAINMOT_MINSET_REQ		"01"		// 메인모터 최수출력 자동설정 요구

String STX_TM_INPUT_REQ     = "$27";		// STX INPUT TEST
String STX_TM_OUTPUT_REQ     = "$28";		// STX OUT TEST

String STX_AP_INFO_SEND     = "$29";		// STX GANE STATUS
	#define DEF_AP_TYPE		1			// AP TYPE

String STX_ROD_REGIST		="$30";			// ROD_REGIST
	// AP 수신 ACT
	#define ROD_REGI_AP_ENTRY	1
	#define ROD_REGI_AP_CANCEL	2
	// FW송신 MSG,DATA
	#define ROD_REGI_FW_CANCEL		"03%"
	#define ROD_REGI_WRITE_START	"04%"
	#define ROD_REGI_FINISH			"05%"

//장치에러 체크 결과 LOG, AP에 출력
String STX_DEV_CHK_ERR_LOG	= "$89";		//

String STX_DEV_FUNCTION_REQ	= "$98";		// LM Para SET
	#define STR_WIRE_AGING_TEST		"01"
	#define NUM_WIRE_AGING_TEST		01
	

String STX_DATA_PARA_SET	= "$99";		// LM Para SET
	
String STX_DIAG_INPUT		= "$DI";		// STX Diag Input
String STX_DIAG_OUTPUT		= "$DO";		// STX Diag OutPut
String STX_DIAG_FUNC		= "$DF";		// STX Diag Function
String STX_DIAG_SET			= "$DS";		// STX Diag SET(Save)
String STX_DIAG_EXIT		= "$DE";		// STX Diag OutPut

String STX_DNLD_EXEC		= "$DN";		// STX Download (V1081)

// SET IF STX
String STX_WAVE_SET       = "$F0";		// WAVE(너울)Set STX
String STX_BITE_SET       = "$F1";		// BITE STX
String STX_HIT_SET        = "$F2";		// HIT STX
String STX_BREAK_SET      = "$F3";		// BREAK STX

String STX_TOKTOK_SET     = "$F4";		// BLDC TOKTOK STX
String STX_EXTLED_SET     = "$F5";		// BLDC TOKTOK STX

String STX_RANDING_START  = "$F6";		// BLDC TOKTOK STX
String STX_TORQ_ADJ_SET   = "$F8";		// Torque Motor - ADJ Value Set

String STX_TORQ_TUN_SET   = "$F9";		// SET TORQ  HIT & HOLD
String STX_LED_SET		  = "$FA";

String STX_ENB_SET        = "$FF";		// ENB STX

String STX_TEST_EXEC      = "$TE";		// TEST EXECUTE(V108) for test



//================ RESP =======================
// Define RESP STX
String OLD_RESP_STX_BUTTON      = "$06";
String OLD_RESP_STX_ENCODER     = "$07";
String RESP_STX_BUTTON          = "$13";
String RESP_STX_ENCODER         = "$14";
String STS_APSTS_REQ         	= "$18";

String RESP_STX_ROD_CONN		= "$20";
// Add Define RESP_STX
String RESP_STX_IMU_CONNECT     = "$21";
	#define RESP_IMU_FULLSTR_CONN		"$2101%"
	#define RESP_IMU_FULLSTR_DISCONN	"$2100%"

String STS_PWRON_STS         	= "$22";			// PowerOn Reset STS

// OLD_IF

//===================================================
//=== AP CMD STX "INT" DEFINE
#define STX_CMD_MAINALIVE			0
#define STX_CMD_INFO_REQ			1
#define STX_CMD_TORQUE_MOTOR		2
#define STX_CMD_SERVO_MOTOR			3
#define STX_CMD_BLDC_MOTOR			4
#define STX_CMD_LED_CONT			6		// MAIN LED
#define STX_CMD_IMU_SET				8
#define STX_STS_IMU_DATA_STS		9	// RESP

#define STX_CMD_VER_READ			10
#define STX_CMD_GAME_STS			11
#define STX_STS_BAT_LVL				12	//RESP
#define STX_STS_BUTT_SWC			13	//RESP
#define STX_STS_HAND_ENC			14	//RESP

#define STX_CMD_DEV_CONT_REQ		15
#define STX_CMD_GET_ADDR			16
#define STX_CMD_SET_ADDR			17
#define STX_CMD_APSTS_RESP			18		// AP RESP
#define STS_CMD_APSTS_REQ			18		// MAIN STS(REQ)
#define STX_CMD_MOT_OUT_RATE_SET	19

#define STX_STS_ROD_CONN			20	//RESP
#define STX_STS_IMU_CONN			21	//RESP
#define STX_STS_PWRON_STS			22		// STS, NOT USE

#define STX_CMD_VRT_MOT_CONT		23
#define STX_CMD_BTN_LED_CONT		24

#define STX_CMD_ROD_REGIST			30		// REEL REGISTRATION
#define STX_STS_ROD_REGIST			30		// REEL REGISTRATION


//--------ROD REGIST TIME DEFINE---------------
#define ROD_REGI_CANCEL_TO		(25*1000)	//  SEC
#define ROD_REGI_END_TO_1ST		(1500)	// 1.5 SEC
#define ROD_REGI_END_TO_2ND		(4000)	// 4 SEC


//====DEFINE END ===============================


//-----------------------------------
// IMP 
// 1) STX_CMD_INT
#define DEF_STX_GET_ADDR     16



//==============================

// Define CMD Data
String STATE_DISABLE   = "00000000";		// Game Disable = TimeOver = Game Waiting
String STATE_WAITING   = "00000000";		// Game Disable = TimeOver = Game Waiting
String STATE_ENABLE    = "11111111";		// Game Enable
String STATE_READY     = "22222222";		// Game Ready


//==============================

#define STATE_CONN_UNKNOWN     "-1";		// UNKNOWN

#define STATE_DISCONN          "00";
#define STATE_CONNECTED        "01";
#define STATE_DISCONN_NOTINIT  "11";
#define STATE_DISCONN_NOTPEER  "12";
#define STATE_DISCONN_SLEEP    "88";
#define STATE_DISCONN_MAC      "99";


#define IMU_CONN_2CHAR_INT 11



//====Variable=======================================================

bool isGameEnable = false;				// MAIN Board ENB/DIS
//bool isGameStby = true;			// GAME STBY

//bool isSlaveConn = false;			// NOT USE

#define STR_BOOTINFO_DEFAULT	"06028"	// (V108) "0"(AutoReboot) + "60"(전원ON시간,초) + "28"(USB 안정화시간,초)
#define	DEF_PWRON_DELAY_TIME	60
#define	DEF_PWRON_DELAY_TMAX	99
#define	DEF_PWRON_DELAY_TMIN	10
#define	DEF_USB_SAFE_TIME		28

unsigned long	g_RecvCmdCount=0;		// (V108)
unsigned long	g_LastCmdTick;			// (V108)
int				g_SerialEnable=0;		// (V108)
int				g_ResetReason=-1;		// (V108)
int				g_AutoReboot=0;			// (V108)
//int				g_PwrOnDelayTime=DEF_PWRON_DELAY_TIME;		// (V108)
//int				g_UsbSafeTime=DEF_USB_SAFE_TIME;			// (V108)


//이유 :  CONFIG 구별없이 변수를 할당하므로,
// 		혹 CONFIG COMPILE SWITCH를 ENB(변경)하지 않고.
//		FW를 만들면, 보드타입을 읽지 않으므로, 초기 설정값은 OLD보드로 한다.
unsigned short mainBoardType = MAIN_BOARD_V10;	//기본 : OLD Board, 
unsigned short mainBoardTypedData = 0;	// 0x00 or 0x1F


// ROD BOARD TYPE
#define ROD_BOARD_V1	0
#define ROD_BOARD_V2	1
unsigned short rodBoardType = ROD_BOARD_V2;	//New Board

String strRodBoardType = "001";		// "000"-OLD BOARD, "001"- NEW BOARD


// AP연결 상태 
int ap_conn_status = UNKNOWN;
int old_ap_conn_status = UNKNOWN;
unsigned int apAliveConn_NotRecvCnt = 0;		// 1 sec마다 10회 안오는 경우, 끊김.
unsigned int apAliveElse_NotRecvCnt = 0;		// 1 sec마다 10회 안오는 경우, 끊김.

/*
Slave Status & Disconn Reason
*/
int rod_conn_status = ROD_UNKNOWN;
int old_rod_conn_status = ROD_UNKNOWN;

String g_RespVersion = "";						// Main + Rod + IMU

//int fSlaveSleep = 0;				// Sleep Flag

//=== Define System Timer, 10ms, 100ms ===
bool isPowerOnReady = false;

unsigned long curr_ms_tick = 0;			// 1ms Curr tick
unsigned long old_ms_tick = 0;			// 1ms Old tick

unsigned long chk_ms_tick = 0;			// 1ms Check Curr tick

unsigned int count_1ms = 0;
unsigned int count_10ms = 0;
unsigned int count_100ms = 0;
unsigned int count_500ms = 0;
unsigned int count_1sec = 0;

unsigned int sys_count_1ms = 0;
unsigned int sys_count_10ms = 0;
unsigned int sys_count_100ms = 0;
unsigned int sys_count_500ms = 0;
unsigned int sys_count_1sec = 0;
unsigned int sys_count_5sec = 0;


#define TO_1MS_TICK 1
#define TO_10MS_TICK 10
#define TO_100MS_TICK 100

//--- Define Time SET & CHECK macro

#define setTO(var) (var = curr_ms_tick)				// a = old time;
#define getTO(var) ( (curr_ms_tick < var)? (0xFFFFFFFF - var + curr_ms_tick + 1 ) :  (curr_ms_tick - var) )
#define checkTO(var,TO) ( getTO(var) > TO)	//NG?

#define setLmEnc(var) (var = mainEnc.lmCnt)
#define checkLmEnc(var,TO) (TO < (mainEnc.lmCnt - var))


//== Add Condition Check Variable FOR LOG
int reqServoMotor = 2;
int oldServoMotor = -1;
int absReqServoMotor = 2;
int oldAbsServoMotor = -1;

short recv_TorqMotor_Flag = 0;	// 수신여부
int reqTorqueMotor = DEFAULT_TORQ; //60;
int oldTorqueMotor = DEFAULT_TORQ; //60;
int reqBreakMotor = 0;
int oldBreakMotor = 0;
long reqImuMeasFlag = 0;			// IMU STOP
long oldImuMeasFlag = 0;

// CR250221a
short defaultTorqueMotor = DEFAULT_TORQ;
short bldcLimitVal = BLDC_LIMIT_MIN;

int cntRightButtonOn = 0;		// Btn On Flag
int cntRightButtonOff = 0;		// Btn Off Flag
int rightBtnOnCount = 0;		// Count Button ON

int exeWave = 0;
int exeBite = 0;
int exeHit = 0;				// Hit Area : Hit ~ Success or Fail
int exeHit_1st = 0;			// Hit Impact
int exeResist = 0;
int exeFight = 0;		// Not Use
int exeHoldon = 0;		// OLD FLAG
int exeHoldOff = 0;

// NEW BITE IF
int bitePtn_Flag = 0;		//추후 개선
int exeBitePtn1flag = 0;		// Bite Shallow
int exeBitePtn2flag = 0;		// Bite Deep
int exeBitePtn3flag = 0;		// Bite TokTok

// NEW HOLDON IF
short holdOnPtnAll_Flag = 0;		// HOLD ON  PTN 전체
short holdOnPtn1_Flag = 0;		// 각 패턴FALG

//
int exeFastHoldOn = 0;
int exeFastHoldOff = 0;
int exeButtonCombo = 0;
int exeButtonComboOff = 0;
//
int exeRanding = 0;
int exeSuccessFail = 0;
int exeContinue = 0;
int exeGameOver = 0;

int execContinue2Ready = 0;

int exeProgramEnd = 0;

// Hit시 Servo FW제어 삭제, AP가 제어.
//int exeResistServoENB = 1;		// Servo Only ENB/DIS
int exeResistServoENB = 0;		// Servo Only ENB/DIS	Change 24/7/26.
int exeHoldonServoENB = 1;		// Servo Only ENB/DIS

int biteLogOut = 1;

int gameLevel = 2;				// Game Level ( Easy/ Normal/ Hard)

int gameStatus = PROG_END;
int oldgameStatus = -1;

int fishTypeFixENB = 0;		// 0- Auto, 1-Fix
int fishTypeAuto = 3;	// Auto Fish Kind 7 Case ( 1 ~ 7 ) From Break Value
int fishTypeFix = 3;		// Select(Fix) Fish Kind ( 1 ~ 20 )
//=== test end ===


//=== WAVE TEST =====================================================================
// Wave Test Case
// 0 - Not Control
// 1 - Servo(Angle) Only
// 2 - Torque Only
// 3 - Servo(Angle) & Torque Both
//int waveControlCase = 4; // 2;	// default 1(BLDC Only)
int waveControlCase = 5; //4; // 2;	// default 1(Torq Only)

// Wave Control Torq Type
// 0 - Not Control
// 1 - Type1 : Weak - Easy ?
// 2- Type2 : normal
// 3- type3 : Strong - Hard ? 
#define WAVE_NOT_CONT	0
#define WAVE_WEAK		1
#define WAVE_NORMAL		2
#define WAVE_STRONG		3
#define WAVE_MAX		4
int waveType = WAVE_WEAK;				// Wave Type  0:Dummy, 1~12 (?)

typedef struct _stWaveServoSet
{
	int angle1;		// Return Angle
	int angle2;		// Target Angle
	int interval;
}stWaveServoSet;

// Default Servo for Wave
// 0 - Angle1 - 0 DEGREE
// 30 - Angle2 - 30 DEGREE
// 1500 - Interval 1500ms
stWaveServoSet stWaveServoVal = { -10, 10, 1500 };

#define WAVE_PWR_MAX	(1+12)		// 0 -Dummy

// TBD, On-Off Time Type 1 ~ upto 12
typedef struct _stWaveTorqSet
{
	int rtTorq;			// Return Torq
	int tgTorq;			// Target Torq
	int offInterval;
	int onInterval;

	unsigned int pwr[WAVE_PWR_MAX];
}stWaveTorqSet;

typedef stWaveTorqSet *stWavePtr;

// Default Torque for Wave
// 60 - Torq1 - 60 Set
// 75 - Torq2 - 75 Set
// 2000 - Interval 2000ms

//stWaveTorqSet stWaveTorqVal = { 70, 80, 2000, 70, 75, 80 };
//stWaveTorqSet stWaveTorqVal = { DEFAULT_TORQ, DEFAULT_TORQ+20, 2000, 2000, DEFAULT_TORQ+20, DEFAULT_TORQ+30, DEFAULT_TORQ+40 };
stWaveTorqSet stWaveTorqVal = { 
	DEFAULT_TORQ-20, DEFAULT_TORQ, 900, 1100,		// return Pwr, Pull Pwr, Return Time, Pull Time
	
	0,							// 0 - Dummy 
	DEFAULT_TORQ-4, 			// 1
	DEFAULT_TORQ,
	DEFAULT_TORQ+4, 
	DEFAULT_TORQ+(4*2), 
	DEFAULT_TORQ+(4*3),			// 5 
	76, 
	80, 
	80, 
	80, 
	80,							// 10
	90, 
	90																		// 11,12
	};

// not use

#define DEFAULT_BBN		9		// 16W_BELT

stWaveTorqSet stWaveBdutyVal = {
	9, 9 , 400, 1700, 				// return Pwr, Pull Pwr, Return Time, Pull Time
	
	0,							// Dummy
	DEFAULT_BBN,			// 1 
	DEFAULT_BBN+(2*1), 
	DEFAULT_BBN+(2*2),
	DEFAULT_BBN+(2*3),
	DEFAULT_BBN+(2*4),		// 5
	15, 15, 15, 15, 15,		// type 6~ 10
	15, 15					// type 11,12
	};	// return, Targert, CW 3 sec, Off 3 secL1-12%, L2-15%, L3-18%

// USE
typedef struct _stWavePower
{
	unsigned int pwr[WAVE_PWR_MAX];
}stWavePower;
typedef stWavePower *stWavePwrPtr;
stWavePower stWavePwrBbn = {
	0, 			// 0-Dummy
// 1
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*0), //7 ,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*1), //8, 
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*2), //9, 
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*3), //10, 
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*4), //11, 
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
//11
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	DEFAULT_BBN_PWR+(WAVE_ADD_BBN_PWR*5), //12,
	};


//=== Wave End ===

// === BITE Test ====================================
int biteTestCase = 2;		// Test BITE Control Case ( Servo & Torq Conbination Condition)
int biteTorqCase = 3;	// Test BITE Torq Case ( Torq Control Case : Just Torq/ IncDecTorq/ withFloats Torq(cnt))
int biteCnt = 3;		// 1 BITE is Operation 1~3times AND Deep Control
int biteType = 3;		// BITE Strong Type ( 5 step )

typedef struct _biteTorqSet
{
	int targetTorq;
	int targetInterval;
	int incTorq;
	int incInterval;
	int decTorq;
	int decInterval;
	//
	int shallowTorq;
	int shallowInterval;
	int deepTorq;
	int deepInterval;

  	int torqType[1+8];	// torq[0]-Dummy
}biteTorqSet;


#define DEEP_ADD_TORQ	25
#define DEEP_STEP_TORQ	6
//  Default
// target Torq = 140, target Interval = 700 ms
// inc Torq = 20 each, inc Interval = 100 ms
// dec Torq = 10 each, dec Interval = 100 ms
// End Torq = 60 (Fixed)
// shallow Torq = 90, shallow interval = 500 ms
// deep Torq = 120, deep Interval = 2000 ms
biteTorqSet stBiteTorqVal = 
{ 
	// Torq, Time[ms]

	//DEFAULT_TORQ+40, 700, 	// Target (Hit)
	DEFAULT_TORQ+30, 700,	 // Target (Hit)
	 15, 200,	// inc
	 15, 200,	// dec
	//DEFAULT_TORQ+20, 700,	// shallow
	//DEFAULT_TORQ+20, 700,	// shallow
	DEFAULT_TORQ+20, 500,	// shallow
	//DEFAULT_TORQ+60, 2000,	// deep
	//DEFAULT_TORQ+40, 2000,	// deep
	DEFAULT_TORQ+40, 1500,	// deep
	
	DEFAULT_TORQ,			// Torq[0]-Dummy			
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*0),	//1 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*1), 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*2), 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*3),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*4),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*5),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*6),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*7)	//8
};

#define START_TORQ  DEFAULT_TORQ+10 //60
#define END_TORQ    DEFAULT_TORQ // 60

#define OFF_TORQ	(DEFAULT_TORQ-20)

#define WF_END_TORQ    DEFAULT_TORQ+20		// with Floats END Torq

#define HIT_LOW_TORQ    DEFAULT_TORQ+20		// HIT LOW Torq
#define HIT_END_TORQ    DEFAULT_TORQ+20		// HIT END Torq

#define BITE_WAIT_TIME	300 //500		// 1000 ms
//===Bite End====================


//=== Hit ===
int hitServoAngle = 10;			// Angle 10 : -10 Degree ~ + 10 Degree Repeat
int hitServoInterval = 200;		// Interval Angle Change Time

int torqResistFix = 0;			// Auto(0), Fix(1)

typedef struct _hitTorqSet
{
	int targetTorq;
	int targetInterval;

	int holdonTorq;
	int holdonInterval;
	
	int lowTorq;
	int lowInterval;

	int midTorq;
	int midInterval;

	int torqType[1+12];		// torq[0]-Dummy, 12 Type
} hitTorqSet;

#define HIT_TORQ_ADD_VAL 20
#define HIT_TORQ_STEP_VAL 3

hitTorqSet stHitTorqVal = 
{ 
	 DEFAULT_TORQ+10, 1500,		// Target
	 //DEFAULT_TORQ+30, 1500,		// HoldOn
	 DEFAULT_TORQ+20, 1000,		// HoldOn

	 DEFAULT_TORQ,	  1000,		// Low = Target
	 DEFAULT_TORQ+15, 1500,		// Mid = (Resist+Low) /2
		
	0,				// Torq[0]-Dummy
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*0),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*1),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*2),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*3),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*4),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*5),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*6),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*7),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*8),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*9),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*10),
	DEFAULT_TORQ + (HIT_TORQ_STEP_VAL*11),		
	// Target by Fish Kind
};

#define HIT_WAIT_TIME	50 //500		// 1000 ms

	
//===HIt END===

//=== Break===
typedef struct _stBreakSet
{
	int val[9];
}stBreakSet;

stBreakSet stBreakVal[1+3] =
{
	//0-dummy, Fish1 , Fish2 , Fish3 , Fish4 , Fish5 , Fish6 , Fish7
	{0, },											// type 0 - Dummy
	{0,     215, 220, 225, 230, 235, 240, 245, 245 }, 	// type1 - weak Type Mecha
	{0,		205, 210, 215, 220, 225, 230, 235, 235 },		// type2 - Normal Type Mecha
	{0,		195, 200, 205, 210, 215, 220, 225, 225 }		// type3 - Stong Type Mecha
};

int breakType = 1;		// Type3
int targetBreakVal = 200;	// Break Val
int outBreakVal = 100;		// Dec Break Val each Time

int decBreakCnt = 0;


//===Break End===

//test
long lpCnt = 0;


// Bobbin TEST
int DiagInMonitor = 0;


// exec Flag
int diagInFlag_ResetSwc = 0;
int diagInFlag_BootSwc = 0;
int diagInFlag_WdtLbSig = 0;
int diagInFlag_SelectBoard =0;

int diagInFlag_BobbinEncA = 0;
int diagInFlag_BobbinEncB = 0;
int diagInFlag_BobbinEncDir = 0;
int diagInFlag_AngleHomeSns = 0;
int diagInFlag_BbnBldcFG = 0;

int diagInFlag_PonSwc = 0;
int diagInFlag_PcUsb5V = 0;
int diagInFlag_Interlock = 0;




// Sensor Value
int sensorResetSwc = 0;
int oldsensorResetSwc = 0;
int sensorBootSwc = 1;
int oldsensorBootSwc = 1;
int sensorWdtFbSig = 0;
int oldsensorWdtFbSig = 0;
int sensorSelBoard = 0;
int oldsensorSelBoard = 0;

int sensorBbnEncA = 0;
int oldsensorBbnEncA = 0;
int sensorBbnEncB = 0;
int oldsensorBbnEncB = 0;
unsigned int sensorBbnEncCnt = 0;
unsigned int oldsensorBbnEncCnt = 0;
int sensorBbnEncDir = 0;
int oldsensorBbnEncDir = 0;

int sensorAngHomeSns = 0;
int oldsensorAngHomeSns = 0;
int sensorBbnFG = 0;
int oldsensorBbnFG = 0;

int sensorPonSwc = 0;
int oldsensorPonSwc = 0;
int sensorUsb5V = 0;
int oldsensorUsb5V = 0;
int sensorDoorIlk = 0;
int oldsensorDoorIlk = 0;

// DIAG 03-07 ~ 03-12, Define ITEM NO
#define DIAG_SEN_XXX	0
//
#define DIAG_SEN_LS01	7
#define DIAG_SEN_LS02	8
#define DIAG_SEN_LS03	9
#define DIAG_SEN_LS04	10
#define DIAG_SEN_LMFG	11
#define DIAG_SEN_LMDIR	12
#define DIAG_SEN_LMPULSE	13
#define DIAG_RSV14		14

#define DIAG_SEN__MAX	15

#define SEN_MAX_SIZE	32
typedef struct _DIAGSEN
{
	short flag;
	short old;
	short curr;
}STDIAGSEN;
STDIAGSEN stDiagSen[SEN_MAX_SIZE];

unsigned int oldDiaglmEncPulse = 0;


// Diag Exec Flag (OUT)

int diagOutFlag_WDToff = 0;

int diagOutFlag_LedCent_R = 0;
int diagOutFlag_LedCent_G = 0;
int diagOutFlag_LedCent_B = 0;
int diagOutFlag_LedCent_RGB = 0;

int diagOutflag_bbnTimeOut = 0;		// Timeout Monitor Check Flag
unsigned int bbnTimeout_step = 0x00;
unsigned long bbnTimeout_Val = 3000;	// SET TimeOut VAL

int diagBbnflag_TokTokControl = 0;
unsigned int bbnTokTok_step = 0x00;
int bbnTokTok_OnDuty = 15;			// 15 Percent(%)
unsigned long bbnTokTok_OnTime = 200;		// 1sec = 1000 ms
int bbnTokTok_OffDuty = 0;			// 0 - OFF
unsigned long bbnTokTok_OffTime = 200;		// 1sec = 1000 ms



// Continue Torq
int continueTorq = DEFAULT_TORQ; //60;
int currTorq = DEFAULT_TORQ; //60;
int oldTorq = DEFAULT_TORQ; //60;

/*-----------------------------------------------------------
Continue Torq Control
------------------------------------------------------------*/
int rdy_outTorq		= DEFAULT_TORQ; //60;
int rdy_eachTime	= 1000;			// 1000ms
int rdy_cnt			= 0;
int rdy_incTorq		= 10;
int rdy_reqCnt		= 0;

//
int		gLevel	= 1;
int		gZone	= 1;
int		gStage	= 1;
String	rdyMsg;

//
int recvMainConn_Pwr1stCnt = 0;

//

typedef struct _paraCheck
{
	int idx;
	int min;
	int max;
}stParaCheck;

#define GAME_STS_LVL	11
#define GAME_STS_LVL	11
#define MAX_PARA_CNT	20

// TBD
stParaCheck paraCheck[MAX_PARA_CNT] =
{
   //idx, min, max
	{ 0, 			},	//dummy
   	{ 1,			},	// ~~
   	
	{ GAME_STS_LVL,	1,	3	},	// Level
	

};

int mainMode = 0;		// 0=idle, 1= normal, Diag etc
int oldmainMode = 0;

int ledControlMode = 1;	// 1 - Control ON( At NOMAL ) , 0 - Control OFF( at DIAG )

// bbn Action Kind (SetVal / Wave / ~ HoldOn / Finish )
unsigned int bbnActKind = 0; 			// BBNACT_SETVAL ( 0-Set, 1-=Wave, 2-Bite /  ~ / 6-Raning)
unsigned int bbnOpKind = 1;				// OpKind(Wave/Bite/Hit...Randing)
unsigned int bbnTokCnt = 0;

// BLDC Control Instead Angle Motor
int exeFlag_bldcWave = 0;
int exeFlag_bldcBite = 0;
int exeFlag_bldcHit = 0;
int exeFlag_bldcFight = 0;
int exeFlag_bldcHoldOn = 0;
int exeFlag_bldcRanding = 0;
int exeFlag_bldcSuccess = 0;

int exeFlag_bbnMotorStop = 0;	// AP recv BBN(BLDC) motor Cont

int exeFlag_biteBldcTokTokEnd = 0;

int exeFlag_holonBldcTokTokEnd = 0;


int exeFlag_torqMotorStop = 0;	// AP recv BBN(BLDC) motor Cont

// AP INFO
int restPowerDist = 0;


//  BBN MOTOR
unsigned long bbnTokTO = 0;
unsigned int oldbbnTokStep = 0x00;

typedef struct _BBN_ACT
{
	int onDuty;
	unsigned long onTime;
	int offDuty;
	unsigned long offTime;	
}BBN_ACT;

enum
{
	BBNACT_SETVAL = 0,
	BBNACT_WAVE,			//1
	BBNACT_BITE,			//2
	BBNACT_HIT,				//3
	BBNACT_FIGHT,			//4
	BBNACT_HOLDON,			//5
	BBNACT_RANDING,			//6

	BBNACT_TBLMAX		// 7
}eBBNACTKIND;

#define MUL_BBN_PWR 2		// 16W

BBN_ACT sBbnActTbl[BBNACT_TBLMAX] =	// +1 Dummy : [0]
{

					// ON_DUTY,	ON_TIME,	OFF_DUTY,	OFF_TIME
	// 0 - Set Val
	{  ( DEFAULT_BBN_PWR+(13*MUL_BBN_PWR)),  1000,   0,   1000 },		// 20, 22 - 9%	, (13+ 13*2)= 13+26 = 39
	// 1 - Wave
	{  ( DEFAULT_BBN_PWR+(13*MUL_BBN_PWR)),  2000,   0,   2000 },		// 20, 28 - 10%	, (13+ 13*2)= 13+26 = 39
	// 2 - Bite
	//{  ( 25),    50,   0,     50 },	// 25, 30 - 12%
	//{  ( DEFAULT_BBN_PWR+(17*MUL_BBN_PWR)),    70,   0,     70 },		// 30, 30 - 12%	, (13 + 17*2)=13+34 = 47
	{  ( DEFAULT_BBN_PWR+(15*MUL_BBN_PWR)),    70,   0,     70 },		// 30, 30 - 12%	, (13 + 17*2)=13+34 = 47 => (13+30) = 43
	// 3 - Hit
	//{  (255*20/100),  300,   0,  100 },  // 51 - 18%
	//{  ( DEFAULT_BBN_PWR+(22*MUL_BBN_PWR)),   300,   0,    100 },		// 35, 40 - 18%	, (13+22*2)=13+44 = 57
	{  ( DEFAULT_BBN_PWR+(40-DEFAULT_BBN_PWR)),   300,   0,    100 },		// 35, 40 - 18%	, (13+22*2)=13+44 = 57 => 50%(40)
	// 4 - Fighting
	{  ( DEFAULT_BBN_PWR+(12*MUL_BBN_PWR)),   100,   0,     70 },		// 25 - 10%		, (13+ 12*2)=13+24 = 37
	// 5 - HoldOn
	//{  ( 76),   150,   0,     50 },		// 76 - 30%
	//{  ( DEFAULT_BBN_PWR+(27*MUL_BBN_PWR)),   150,   0,     50 },		// 40, - 30%	, (13+ 27*2)=13+54 = 67 
	{  ( DEFAULT_BBN_PWR+(46-DEFAULT_BBN_PWR)),   150,   0,     50 },		// 40, - 30%	, (13+ 27*2)=13+54 = 67 => 60%( 46)
	// 6 - Randing
	//{  ( DEFAULT_BBN_PWR+(22*MUL_BBN_PWR)),   100,   0,     70 },		// 35 - 25%
	{  ( DEFAULT_BBN_PWR+(14*MUL_BBN_PWR)),   100,   0,     70 },		// 25 - 10%		,(13 + 14*2)=13+28 = 51

};

//int bbnReqCnt[BBNACT_TBLMAX] = {10, 3, 5, 5, 5, 7, 5};
int bbnReqCnt[BBNACT_TBLMAX] = {
		10,			// 0, set 
		10,			// 1, wave
		4,			// 2, bite
		3,			// 3, hit
		0,			// 4, fight, // 0 = Unlimited Loop
		4,			// 5, Hold
		5			// 6, Randing
};

int bbnRptCnt = 0;

//int		i_fishKind;

String strFishKind;
float 	fishKg_flt;
double	fishKg_dbl;

int fishSize = 0;
int fishRare = 0;

int holdDir = 0;
int startEnd = 0;

int randingPtn = 0;

#define LEVEL_MIN 1			// easy
#define LEVEL_MAX 3			// hard

// New BITE ( Ptn3  = TokTok )
BBN_ACT	sBbnBiteVal = 
{
	( DEFAULT_BBN_PWR+(15*MUL_BBN_PWR)),		// OnDuty
	70,		// OnTime
	0,			// Off Duty
	70		// Off Duty
};

// New HOLD IF
unsigned long sBbnHoldVal_onTime = 100;
unsigned long sBbnHoldVal_offTime = 50;

#define FISH_MAX (1+6+1)
int sBbnHoldVal[FISH_MAX] =
{
	0,			// 0 - Dummy
	34,			// 1
	40,			// 2
	45,			// 3
	50,			// 4
	56,			// 5
	61,			// 6
	67			// 7 -Reserved
};

typedef struct _RGB
{
	int r;
	int g;
	int b;
}sLEDRGB;
typedef sLEDRGB *ptrRGB;

sLEDRGB sRGB[LED_IDX_MAX] = 
{
	// R  G   B
	{0,},			// 0- Dumy
	//
	{255,255,255},	// Cent
	{0,	0,	255},	// Btm
	{0,	0,	255},	// Left
	{0,	0,	255},	// Right
};

sLEDRGB outRGB[LED_IDX_MAX] = 
{
	// R  G  B
	{0,},			// 0- Dumy
	//
	{255,255,255},	// Cent
	{0,	0,	255},	// Btm
	{0,	0,	255},	// Left
	{0,	0,	255},	// Right
};

#define COLOR_MAX_NO 16
extern const sLEDRGB sColorTBL[COLOR_MAX_NO]
{
	/*0*/		{255,	255,	255},	//	White
	/*1*/		{255,	0,	0},			//	Red
	/*2*/		{0,	255,	0},			//	Lime
	/*3*/		{0,	0,	255},			//	Blue
	/*4*/		{255,	255,	0},		//	Yellow
	/*5*/		{0,	255,	255},		//	Cyan / Aqua
	/*6*/		{255,	0,	255},		//	Magenta / Fuchsia
	//---------------
	/*7*/		{128,	0,	0},			//	Maroon
	/*8*/		{128,	128,	0},		//	Olive
	/*9*/		{0,	128,	0},			//	Green
	/*10*/		{128,	0,	128},		//	Purple
	/*11*/		{0,	128,	128},		//	Teal
	/*12*/		{0,	0,	128},			//	Navy
	//--------------
	/*13*/		{192,	192,	192},	//	Silver
	/*14*/		{128,	128,	128},	//	Gray
	/*15*/		{0,	0,	0},				//	Black
};


unsigned long bbnDiagTO = 0;

#define BBN_PWM_DUTY_MIN 1
#define BBN_PWM_DUTY_MAX 255

#define BBN_WAIT_TIME_MIN 1
#define BBN_WAIT_TIME_MAX 30000

#define BBN_RPT_CNT_MIN	1
#define BBN_RPT_CNT_MAX	255

//
#define ROD_RESP_OVER_CNT 		(5) //(3)	// ( 10 )		// 10  SEC
unsigned int rodAlive_SendCnt = 0;

//
int imu_conn_status = UNKNOWN;
int oldimu_conn_status = UNKNOWN;

//
int exeResistOnceComp = 0;

// Ext LED VAR
int dimming_time = SIPPAGOE;
int olddimming_time = -1;
unsigned int ledoffHoldTimeCnt = (100/10);		// 100ms


//=== SENSOR 1MS, 10MS
typedef struct _sensor
{
	unsigned int old2;
	unsigned int old;
	unsigned int curr;
	unsigned int lvl;
	unsigned int le;
	unsigned int te;
}sensor;
typedef sensor* ptrSensor;

// DEFINE SENSOR NO & SENSOR MAX CNT
#define SENSOR_1	0
#define SENSOR_2	1
#define SENSOR_3	2
#define SENSOR_4	3

#define SENSOR_MAX	4	// MAC COUNT

// 1ms SEN, 6개 데이타 * 4개
sensor sensor1ms[SENSOR_MAX] = 
	{ 
		{0,0,0,0,0,0 },		// senorMap 1 
		{0,0,0,0,0,0 },		// Snesor Map2
		{0,0,0,0,0,0 },		// Sensor map3
		{0,0,0,0,0,0 }		// Snesor Map4
	};

// 10ms SEN
sensor sensor10ms[SENSOR_MAX] = 
	{ 
		{0,0,0,0,0,0 },		// senorMap 1 
		{0,0,0,0,0,0 },		// Snesor Map2
		{0,0,0,0,0,0 },		// Sensor map3
		{0,0,0,0,0,0 }		// Snesor Map4
	};

#define BIT_0	0
#define BIT_1	1
#define BIT_2	2
#define BIT_3	3
#define BIT_4	4
#define BIT_5	5
#define BIT_6	6
#define BIT_7	7
#define BIT_8	8
#define BIT_9	9
#define BIT_10	10
#define BIT_11	11
#define BIT_12	12
#define BIT_13	13
#define BIT_14	14
#define BIT_15	15


#define SNS_boot		BIT_0 //+ (SENSOR_1*16)
#define SNS_usb5V		BIT_1 //+ (SENSOR_1*16)
#define SNS_PwrOffSwc	BIT_2 // 3 //+ (SENSOR_1*16)
#define SNS_AcOff		BIT_3 // AC Off(V108)
//
#define SNS_RSV04		BIT_4	//rsv
//
#define SNS_bbnMotFg	BIT_5 // 2 //+ (SENSOR_1*16)
#define SNS_bbnEncA		BIT_6 // 4 //+ (SENSOR_1*16)
#define SNS_bbnEncB		BIT_7 // 5 //+ (SENSOR_1*16)

#define SNS_LM_HOME		BIT_8
#define SNS_LM_LEFT		BIT_9
#define SNS_LM_RIGHT	BIT_10
#define SNS_RSV11		BIT_11	//rsv
#define SNS_RSV12		BIT_12	//rsv
#define SNS_LMMOT_FG	BIT_13
#define SNS_RSV14		BIT_14	//rsv
#define SNS_LM_ENC		BIT_15




#define BIT_SNS_BOOT	0x0001	// bitt 0
#define BIT_SNS_USB5V	0x0002	// bitt 1
#define BIT_SNS_PWRSWC	0x0004	// 0x0008	// 2, bitt 3
#define BIT_SNS_ACOFF	0x0008	// 0x0040	// 3, bitt 6					// 전원 관련 센서(V108)
#define	BIT_SNS_PWRALL	(BIT_SNS_USB5V|BIT_SNS_PWRSWC|BIT_SNS_ACOFF)		// 전원 관련 센서(V108)
//
#define BIT_SNS_RSV4	0x0010	// 0x0080	// 4, bitt 7
//
#define BIT_SNS_BBNFG	0x0020	// 0x0004	//5,  bitt 2
#define BIT_SNS_BBNENCA	0x0040	// 0x0010	//6,  bitt 4
#define BIT_SNS_BBNENCB	0x0080	// 0x0020	//7,  bitt 5
#define BIT_SNS_BBNALL	(BIT_SNS_BBNFG|BIT_SNS_BBNENCA|BIT_SNS_BBNENCB)		// BBN 관련 센서 전체(V108)
#define BIT_SNS_ENCAB	(BIT_SNS_BBNENCA|BIT_SNS_BBNENCB)					// ENC 관련 센서 전체(V108)
//--LM
#define BIT_SNS_LMHOME_BIT8		0x0100
#define BIT_SNS_LMLEFT_BIT9		0x0200
#define BIT_SNS_LMRIGHT_BIT10	0x0400

#define BIT_SNS_LMMOTFG			0x2000
#define BIT_SNS_LMENC			0x8000

#define SNS_ON		1
#define SNS_OFF		0

//==== 1MS Define Senseo Map Define
#define	LVL01_BBNALL	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BBNALL))		// BBN 관련 센서 전체(V108)
#define LVL01_BBNFG		(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BBNFG))
#define LVL01_BBNENCA	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BBNENCA))
#define LVL01_BBNENCB	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BBNENCB))
#define LVL01_LMMOTFG	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_LMMOTFG))
#define LVL01_LMENC		(sensor1ms[SENSOR_1].lvl & (BIT_SNS_LMENC))

#define LVL01_LMHOME	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_LMHOME_BIT8))
#define LVL01_LMLEFT	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_LMLEFT_BIT9))
#define LVL01_LMRIGHT	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_LMRIGHT_BIT10))

#define LE01_BBNFG		(sensor1ms[SENSOR_1].le & (BIT_SNS_BBNFG))
#define LE01_BBNENCA	(sensor1ms[SENSOR_1].le & (BIT_SNS_BBNENCA))
#define LE01_BBNENCB	(sensor1ms[SENSOR_1].le & (BIT_SNS_BBNENCB))
#define LE01_LMMOTFG	(sensor1ms[SENSOR_1].le & (BIT_SNS_LMMOTFG))
#define LE01_LMENC		(sensor1ms[SENSOR_1].le & (BIT_SNS_LMENC))

#define LE01_LMHOME		(sensor1ms[SENSOR_1].le & (BIT_SNS_LMHOME_BIT8))
#define LE01_LMLEFT		(sensor1ms[SENSOR_1].le & (BIT_SNS_LMLEFT_BIT9))
#define LE01_LMRIGHT	(sensor1ms[SENSOR_1].le & (BIT_SNS_LMRIGHT_BIT10))

#define TE01_BBNFG		(sensor1ms[SENSOR_1].te & (BIT_SNS_BBNFG))
#define TE01_BBNENCA	(sensor1ms[SENSOR_1].te & (BIT_SNS_BBNENCA))
#define TE01_BBNENCB	(sensor1ms[SENSOR_1].te & (BIT_SNS_BBNENCB))
#define TE01_LMMOTFG	(sensor1ms[SENSOR_1].te & (BIT_SNS_LMMOTFG))
#define TE01_LMENC		(sensor1ms[SENSOR_1].te & (BIT_SNS_LMENC))

#define TE01_LMHOME		(sensor1ms[SENSOR_1].te & (BIT_SNS_LMHOME_BIT8))
#define TE01_LMLEFT		(sensor1ms[SENSOR_1].te & (BIT_SNS_LMLEFT_BIT9))
#define TE01_LMRIGHT	(sensor1ms[SENSOR_1].te & (BIT_SNS_LMRIGHT_BIT10))


//==== 10MS Define Senseo Map Define
#define LVL10_BOOT		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BOOT))
#define LVL10_USB5V		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_USB5V))
#define LVL10_PWROFF_SWC (sensor10ms[SENSOR_1].lvl & (BIT_SNS_PWRSWC))		// OFF SWITCH
#define LVL10_ACOFF		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_ACOFF))		// (V108)
#define	LVL10_PWRALL	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_PWRALL))		// 파워 관련 센서 전체(V108)
// USE DIAG
#define LVL10_BBNFG		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNFG))
#define LVL10_BBNENCA	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNENCA))
#define LVL10_BBNENCB	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNENCB))
#define	LVL10_ENCAB		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_ENCAB))		// ENC 관련 센서 전체(V108)

#define lev10_lmHome	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMHOME_BIT8))? 1:0)
#define lev10_lmLeft	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMLEFT_BIT9))? 1:0)
#define lev10_lmRight	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMRIGHT_BIT10))? 1:0)

#define LVL10_LMMOTFG	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMMOTFG))
#define LVL10_LMENC		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMENC))

//
#define LE10_BBNFG		(sensor10ms[SENSOR_1].le & (BIT_SNS_BBNFG))

#define LE10_USB5V		(sensor10ms[SENSOR_1].le & (BIT_SNS_USB5V))
#define LE10_PWROFF_SWC	(sensor10ms[SENSOR_1].le & (BIT_SNS_PWRSWC))

#define LE10_LMHOME		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMHOME_BIT8))
#define LE10_LMLEFT		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMLEFT_BIT9))
#define LE10_LMRIGHT	(sensor10ms[SENSOR_1].le & (BIT_SNS_LMRIGHT_BIT10))

#define LE10_LMMOTFG	(sensor10ms[SENSOR_1].le & (BIT_SNS_LMMOTFG))
#define LE10_LMENC		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMENC))

//
#define TE10_BBNFG		(sensor10ms[SENSOR_1].te & (BIT_SNS_BBNFG))

#define TE10_USB5V		(sensor10ms[SENSOR_1].te & (BIT_SNS_USB5V))
#define TE10_PWROFF_SWC	(sensor10ms[SENSOR_1].te & (BIT_SNS_PWRSWC))

#define TE10_LMHOME		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMHOME_BIT8))
#define TE10_LMLEFT		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMLEFT_BIT9))
#define TE10_LMRIGHT	(sensor10ms[SENSOR_1].te & (BIT_SNS_LMRIGHT_BIT10))

#define TE10_LMMOTFG	(sensor10ms[SENSOR_1].te & (BIT_SNS_LMMOTFG))
#define TE10_LMENC		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMENC))

//
int recvPwrOffExec_Resp = 0;


//---------MKT TEST_1------------------------


//---- MKT TEST_2 : 20 Step---------------------

unsigned int	motTestAct = 0;		// 0 : REL, 1:TEST
unsigned int	motTestMainIdx = 20;	// Test Dafault 12 ( range : 1 ~ 20 )
unsigned int	motTestBbnIdx = 20;		// Test Dafault 12 ( range : 1 ~ 20 )


//---ROD REGIST BC----
uint8_t rcv_src_addr_back[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };		// RCV src  ADDR
uint8_t rcv_dest_addr[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// RCV dest ADDR
//const uint8_t broad_cast_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};	// REL
uint8_t broad_cast_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };	// REL

volatile int rodRegistMode = 0;		// ROD REGI MODE(SEQ)


// LM MOT
#define LM_POSI_UNKNOWN		0
#define LM_POSI_HOME		1		// HOME STOP
#define LM_POSI_HOMEMOVE	2		// HOME Move

#define LM_POSI_LEFT		10		// Left STOP
#define LM_POSI_LEFTEND		11
#define LM_POSI_LEFTMOVE	12

#define LM_POSI_RIGHT		20		// Right Stop
#define LM_POSI_RIGHTEND	21		// Right Stop
#define LM_POSI_RIGHTMOVE	22		// Right Stop

#define LM_POSI_MAX		30		//

int lmPosi = LM_POSI_UNKNOWN;
int oldlmPosi = LM_POSI_UNKNOWN;

//-- LM SENSOR LEVEL
#define LM_HOME_SEN_ON		HIGH //LOW		// LOW ACTIVE
#define LM_HOME_SEN_OFF		LOW // HIGH
#define LM_LEFT_SEN_ON		LOW
#define LM_LEFT_SEN_OFF		HIGH
#define LM_RIGHT_SEN_ON		LOW
#define LM_RIGHT_SEN_OFF	HIGH

//=== LM ACT(HOME/Lf/Rt/Rtn) Var===
short lmHomeNG_Flag = 0;		// 0-OK, 1-NG
short lmLeftNG_Flag = 0;
short lmRightNG_Flag = 0;		// 0-OK, 1-NG

short lmFishLevel = 0;
short lmLeftDuty;
short lmRightDuty;
short lmReturnDuty;

short endTmReqDuty = DEFAULT_TORQ;	//DEFAULT_TORQ_60;
short leftEndTmReqDuty = DEFAULT_TORQ;	//DEFAULT_TORQ_60;
short rightEndTmReqDuty = DEFAULT_TORQ;	//DEFAULT_TORQ_60;

short lmHome_Init = 0;
short lmHome_Flag = 0;
short lmLeft_Flag = 0;		// AP 좌버티기 개시 명령
short lmRight_Flag = 0;
short lmReturn_Flag = 0;

short lmHold_Stop_Flag = 0;		// AP HOLDON STOP명령

//=== LM ACT(HOME/Lf/Rt/Rtn) Var===
short lmHomeRptCnt = 0;

short lmHomeStep = 0;
short lmLeftStep = 0;
short lmRightStep = 0;
short lmReturnStep = 0;

//----------------------------------------------------
//	TM :
//	BM:
//	LM:
//----------------------------------------------------
typedef struct _motdutyTbl
{
	short lm;
	short lmTime;
	short bm;
	short bmTime;
	short tm;
	short tmTime;
}STLMOTDUTYTBL;
typedef STLMOTDUTYTBL *ptrSTMOTDUTYTBL;

#define FISH_LVL_AUTO	0	// AP Rev Level
#define FISH_LVL_1	1
#define FISH_LVL_2	2
#define FISH_LVL_3	3
#define FISH_LVL_MAX	(1 + FISH_LVL_3)

#define MOT_PWR_LVL_a	1
#define MOT_PWR_LVL_b	2
#define MOT_PWR_LVL_c	3
#define MOT_PWR_LVL_MAX	(1 + MOT_PWR_LVL_c)

// WIRE CONTROL POWER
STLMOTDUTYTBL stLmWireDutyTbl[2][MOT_PWR_LVL_MAX] =
{
	// 0 -Dummy
	{
		{0, },
	},
	// Fish 1- Level
	{
		{0,},	// dummy
		// Lm     /Bm      /Tm		
		{ 10, 10, 10, 40,  60, 40},	// a
		{ 80,100,  5,100,  70,100},	// b
		{ 80,200, 20, 50,  80, 50},	// c
	},
};

// HOLD ACT : FISH + POWER
STLMOTDUTYTBL stLmDutyTbl[FISH_LVL_MAX][MOT_PWR_LVL_MAX] =
{
//(REQ_SPEC)		// REL
	// 0 -Dummy
	{
		{0, },
	},
	// Fish 1- Level
	{
		{0,},	// dummy
		// Lm	  /Bm	   /Tm		
		{ 10, 10, 13,100,  67,100}, // a
		{ 80,100,  5,100,  67,100}, // b
		{ 80,200, 13,200,  67,200}, // c
	},
	// Fish 2- Level
	{
		{0,},	// dummy
		// power a,b,c : tm/bm/lm
		{ 10, 10, 19,100,  67,100},
		{150,100,  5,100,  67,100},
		{150,200, 13,200,  80,200},
	},
	// Fish 3- Level
	{
		{0,},	// dummy
		// power a,b,c : tm/bm/lm
		{ 10, 10, 25,100,  67,100},
		{200,100,  5,100,  67,100},
		{200,200, 13,200,  90,200},
	},
// KPS-TEST
};

// HOME Control TBL
STLMOTDUTYTBL stLmHomeTbl =
{
	// lm	bm		tm
	40,100, 15,100, 60,100		//
};

STLMOTDUTYTBL stLmReturnTbl =
{
	// lm	bm		tm
	60,100, 20,100, 60,100		//
};

volatile int nvm01_home_stopWait_time = 1;

//
//
// HOME 센서 OFF(LS01)시 HOME위치 실시
//
#define LM_HOME_RETRY_CNT	3	// 3회

#define LM_HOME_STOP_WAIT_TIME	0	// 5 sec

#define LM_HOME_MOVE_DUTY	40
#define LM_HOME_DIR_CHANGE_TIME		100	// 100ms

#define LM_START_WAIT_TIME 10 //300

#define LM_HOME_TO_TIME			3000	// 3 sec
#define LM_MOVE_ERR_TO_TIME		3000	// 3 SEC
#define LM_MOVE_TO_TIME	3000

//
short fishLevel = FISH_LVL_1;
short rcvlmPwr = MOT_PWR_LVL_b;

// 버티기 시작,종료 로 AP변경
short lmLeftStartReq = 0;
short lmRightStartReq = 0;

#define LM_PRIORITY_NO		0
#define LM_PRIORITY_LEFT	1
#define LM_PRIORITY_RIGHT	2
short lmPriority = LM_PRIORITY_NO;

//	TBD-LM , COMPILE SWITCH 정사 필요
//---------- LM END----------------

//======= AUTO TENSION VAR Define====
short fishPwr = 1;			// 1~100 [power]
short gmWatStatus = 0;		// unknown
short oldGmWatStatus = 0;		// unknown

#define	TIME_SLEEP_LIGHT		60
#define	TIME_SLEEP_DEEP			120

// Sleep status ---------------------------------
#define	SS_NONE			0		// sleep 상태 아님
#define	SS_WAIT			1		// 게임 대기 상태 sleep
#define	SS_EXIT			2		// AP 종료 상태 sleep
#define	SS_DOWN			3		// 메인보드 Down 상태 sleep

int   gRodSleepStat = 0;		// ROD Sleep status 0: non sleep, 1: sleep ing
int   gRodSleepMode = 0;		// 0: non sleep, 1: sleep mode, 2: sleep deep(AP Exit)
int   gRodSleepTime = 0;		// 0: non sleep, else: sleep time(sec)

//----- LM JIG WIRE TEN
volatile unsigned short wireLongCheck_Flag = 0;		// CCW 연속 5Pulse, CW 연속 5Pulse
volatile unsigned short lmWireControl_Flag = 0;		// 1c<->1a 모터 제어
volatile unsigned int targetWirePulse = 0;
volatile unsigned int currWirePulse = 0;
volatile unsigned int oldcurrWirePulse = 0;

//===== CONFIG =========================
#define DF_CONFIG_UNKNOWN		0
#define DF_CONFIG_STAND_ALONE	1
#define DF_CONFIG_LMJIG			99

short dfConfig = DF_CONFIG_UNKNOWN;

//===== AP Type ======
unsigned short apType = AP_IS_DF;

String strImuInterval = "0000";	//



//======= Device Ready Check =============
	//1) Flag
unsigned short devReadychkFlag = 0;
unsigned short devCheckAllFlag = 0;					// (V108)

unsigned short devCheckBbnMotFlag = 0;
unsigned short devCheckBbnEncFlag = 0;
//unsigned int devCheckBbnEncBFlag = 0;
unsigned short devCheckTorqFlag = 0;
unsigned short devCheckLmMotFlag = 0;

unsigned short devCheckBldc24VFlag = 0;
//unsigned short devCheckWireBroken_Flag = 0;	

unsigned short mainMotAutoSet_Flag = 0;

unsigned short wireAgingTest_Flag = 0;

	// 2) Step
unsigned short devBbnAllChkStep = 0;					// (V108)
unsigned short devBbnMotChkStep = 0;
unsigned short devBbnEncChkStep = 0;
//unsigned int devBbnEncBChkStep = 0;
unsigned short devTorqChkStep = 0;
unsigned short devLmMotChkStep = 0;

unsigned short devBldc24VChkStep = 0;	// bldc 24V check STEP
//unsigned short devCheckWireBroken_Step = 0;


unsigned short mainMotAutoSet_Step = 0;	//

unsigned short wireAgingTest_Step = 0;	//


	// ERR(Flag) SET
unsigned short devChkErrOccure = 0;
//unsigned short occure_WireBrokenErr_Flag = 0;

//-------- 대기 OFF제어 변수 ------------------------------------------------
unsigned short stbyTorqOffControl_Req_Flag = 0;
unsigned short stbyTorqOffControl_Flag = 0;
unsigned short stbyTorqOffControlStep = 0;			// IDLE

//------------------------------------------------------------------------------


//=====인터럽트 관련 VAR=====

volatile unsigned short sSys100UsCnt = 0;		// 0 ~9
volatile unsigned long lSys100UsTime = 0;		// FREE RUN
volatile unsigned long lSys1MsTime = 0;	// FREE RUN

hw_timer_t *timer = NULL;  // 타이머 핸들
volatile unsigned long lLastTime = 0;  // 마지막 GPIO 인터럽트 시간
volatile unsigned long lCurrentTime = 0;  // 마지막 GPIO 인터럽트 시간
volatile unsigned long iEncIntrIntervalTime = 0;  // 두 인터럽트 간 시간 간격

#define ENC_INTV_BUFF_SIZE		256
typedef struct _encIsrDataBuff
{
	unsigned long tick;
	unsigned long width;
	unsigned short dir;	
	unsigned short enc;	
	unsigned short dist;
}encIsrDataBuff;
encIsrDataBuff stEncIntvBuff[ENC_INTV_BUFF_SIZE] =
{
	{0,0,0,0,0},
};
//volatile unsigned long stEncIntvBuff[ENC_INTV_BUFF_SIZE];
volatile unsigned short sEncBuffIdx=0;

volatile bool bIsrEncInterruptFlag = false;  // 인터럽트 발생 여부를 추적하는 플래그
volatile unsigned int	iIsrEncInterruptCnt = 0;

//---- PID ISR제어
//--- ISR - TASK 임계 영역 보호를 위한 뮤텍스
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  // Mutex 초기화

// PID 제어를 위한 변수들
double dLastDiff = 0;
double dIntegral = 0;
double dIsrOutDuty = 0;

//--------------------------------------------------
#define PID_EXEC_ENC_MIN_TIME_1MS	15		// 간격 15ms
#define PID_EXEC_ENC_MIN_TIME_100US	(15*10)		// 간격 15ms

unsigned int iWireDistance = 0;	//현재  Wire 거리
unsigned int iTargetDist = 0;	// 목표 Wire 거리

bool bHookRandingCheckOnce = 0;

unsigned short pidExecFlag = 0;	// FLAG
unsigned short pidExecOutLog = 0;	// Target

//------------------------------------------------------------------
double Kp = 0.5;  // 비례 상수
double Ki = 0.2;  // 적분 상수
double Kd = 0.1;  // 미분 상수

short sMotorSpeed = 0;

int iIsrbbnEncCnt = (WIRE_HOME_BBN_CNT);			// 3 Meter = 3 * 100cm * 10 mm
unsigned short sIsrEncBdata;
unsigned short sIsrBbnEncDir;
//------------------------------------------------------------------
#define	MAX_SAVE_LEN	4096
int	g_SavePtr = 0;
int	g_ProcPtr = 0;
unsigned char g_readBuf[MAX_SAVE_LEN];

#define	DN_MAX_BUF	2
#define	DN_MAX_LEN	0x8000
#define	DN_RCV_PTR	0x1000

int				g_DnExecFlag=0;							// 다운로드 진행 Flag
int				g_DnExecStep=0;							// 다운로드 실행 스텝
int				g_DnRecvNxt=0;							// 다운로드 다음 시퀀스 번호
int				g_DnRecvSeq=0;							// 다운로드 최종 시퀀스 번호
unsigned short	g_DnDataSum=0;
unsigned short	g_DnCalcSum=0;
int				g_DnRecvLen=0;							// 수신 압축 데이터 길이
int				g_DnRecvIdx=0;							// 수신 데이터 Save Index
int				g_DnExecIdx=0;							// 수신 프레임 Exec Index
int				g_DnSaveIdx=0;							// 수신 프레임 Save Index
int				g_DnSaveLen[DN_MAX_BUF]={0,0};			// 수신 프레임 데이터 길이
unsigned char	g_DnExecBuf[DN_MAX_LEN];				// OTA 저장 버퍼
unsigned char	g_DnSaveBuf[DN_MAX_BUF][DN_MAX_LEN];	// 수신 프레임 버퍼(실제 버퍼)
unsigned char*	g_DnRecvBuf;							// 수신 프레임 버퍼(처리 버퍼 주소)

volatile unsigned int g_idleCountCore0 = 0;
#define	MAX_IDLE_CORE	11893125
//---------------------------------------------------------
// 
//============END VAR============================

//==============================================
//
//	FUNCTION
//
//
//==============================================

//=== CPU RESET
void esp_soft_reset();
void esp_hard_reset();
void SerialPortEnable();
void SerialPortDisable();
void SerialPortPinInput();
void hex2asc(unsigned long hx, char* buf, int len);
void HexDump(unsigned long toff, unsigned char* buff, int len);
void debugPort_Off();
void debugPort_On();
void subAC_Off();
void subAC_On();
void DC24V_B_off();
void ledoff_BoardLedAllOff();
void ledOff_MbCent();
void ledOff_MbBtm();
void ledOff_OptLeft();
void ledOff_OptRight();
void extLed_AllOff();
void extLed_controlOff();
void ledOn_MbBtm();
void torqOff();
void torqDefault();
int GetDefValue4torqMotor();
void servoDefault();
void bbnBldcOff();
void lmMotOff();
void breakMotorOff();
void motor_AllOff();
void motor_AllDefault();
void led_AllOff();
void output_AllOff();
void reelOut_AllOff();
void reelOut_AllOff_ForceStep();
String fillZero2String5Char(int _int);
String stringSign6Char(long _int);
String fillZero2String3Char(unsigned int uint);
String fillZero2StringSign4Char(int _int);
String fillZero4Char(unsigned int _val);
String fillZero2StringSign6Char(int _int);
String StringFormat(char *fmt, int val);
void rotateChangeCallback(int _dir, int _cnt);
void battLevelSet( String msg);
void setRod_Connected();
void uartRecvHandlerImp();
bool isParaCheckOk(int idx, int val);
void waveType_TorqSet();
int HitType_TorqSet(int _fishKind);
int isExecBldc();
void torqMot_OffControlStart(unsigned int onTime);
void torqMot_OffControlStop();
void torqMotor_OffTime_Control();
void bbnMotor_OffTime_Start(unsigned int time);
void bbnMotor_OffTime_Control();
void lmMotor_OffTime_Start(unsigned int time);
void lmMotor_OffTime_Control();
void clear_ExecFlag();
void execGameOver();
void setVal_Hit_MainPwr(String msg);
void SetIMU_Measure_Out_OnOff(unsigned int _setClr);
void sendSleepEnable(int Mode, int Time, int Id);
void sendCurrentStatus();
void initSet_LedContVal(int Act);
void ledCont_OffStart(unsigned int pos);
void ledCont_OnStart(unsigned int pos);
void ledCont_BlinkStart(unsigned int pos);
void execledCont_Blink(int pos);
void ledCont_DimmStart(unsigned int pos);
void execledCont_Dimm(int pos);
void old_execCentLed_Dimming_Control(int color, int sec);
void extLed_Off(unsigned int idx);
void extLed_On(unsigned int idx);
void extLed_Control();
void ana_TestExecution(String msg);
void analedContCmd(String msg);
void anaRanding(String msg);
void anaFastHold(String msg);
void anaFastComboBtn(String msg);
void sendRodVrtCmd(int act, unsigned int cnt, unsigned int ontime, unsigned int offtime);
void sendRodBtnLedCmd(int posi, int act, unsigned int cnt, unsigned int ontime, unsigned int offtime);
short lmFish_3Level_Set(short pwr);
void lmReturnOrHome_Start();
void wireHomeControl_Start();
void wireHomeControl_Stop();
void wireHome_Control();
void setWireHome_EncCnt();
void anaGameStatusWAT(String msg);
void stbyTorOffStop_Check();
void anaGameStatus(String msg);
void anaDeviceInfoReq(String msg);
void anaGameEnbOver(String msg);
void apAlive_Check();
void send2RodAlive_CntCheck();
void rodRegistToStop();
void rodRegistToStart();
void rodRegi3secToStart();
void rodRegistToControl();
void rodRegistCancel();
void anaRodRegist(String msg);
void rodRegistExec(String msg);
void tmInput_BoardElecMonitorExec(short _exec);
void tmInput_BbnMotorMonitorExec(short _exec);
void anaTmInputTestReq(String msg);
void anaTmOutputTestReq(String msg);
void anaApInfo(String msg);
void anaMainAlive(String msg);
void devChkReady_Control();
void resp_MainBoard_Ver();
void devchk_OkEnd();
void send2resp_FwSelfControlCmd(String Cmd, int torq);
String stringTo3char(int val);
void devChkMsgSend(String msg);
void devchk_bbnAllStart();
void devchk_lmMotStart();
void devCheckFlagStep_AllClear();
void devchk_bbnAll_Control();
void devchk_lmMot_Control();
void devchk_lmEnc_Control();
void devReadyCheck();
void send_Error_Clr();
void anaDeviceContReq(String msg);
void anaRestDistance(String msg);
void ana_AngleControl( String msg );
void ana_MainMotControl( String msg );
void ana_BobbinControl_Check(String msg);
void ana_LineMotControl_Check(String msg);
void ana_BobbinControl(String msg);
void ana_LmMotControl(String msg);
void lmHome_Control_Start(int _init, int tmReqDuty);
void lmHome_Control_Stop();
void lmLeft_Control_Start(int fish, int pwr, int ReqTm);
void lmLeft_Control_Return();
void lmLeft_Control_Stop();
void lmRight_Control_Start(int fish, int pwr, int ReqTm);
void lmRight_Control_Return();
void lmRight_Control_Stop();
void lmCenter_Control_Start(int fish, int pwr);
void lmCenter_Control_Stop();
void lmReturn_Control_Start(int pwr);
void lmReturn_Control_Stop();
void lmHoldOn_Stop();
void lmHome_Control_2();
void lmHome_Control();
void lmPosition_Set(int posi);
void lmLeft_Control();
void lmRight_Control();
void lmReturn_Control();
void anaMotOutRateSet(String msg);
void checkRodTimer();
void set_rod_conn_status(unsigned int sts);
void condition_fw_check_10ms();
void apPatternReq_Bite_Hold();
void testHand();
void diagInputMonitor();
void diagInputMotorSensorAllOff();
int randomNum(int min, int max);
void biteType_TorqSet(int _fishKind);
int fishKind_autoFromBreak(int breakval);
int fishKind_FromKg(double fishKg);
void execWave_servo();
void execWave_torque();
void execWave_bbn();
void execWave_bbn_Torq();
void execWave_torq_bbn_Start();
void execWave_torq_bbn();
void execBite_servo();
void execBite_torque_justTarget();
void execBite_torque_incTarget();
void rdyincTorq_Stop();
void rdyincTorq_Start();
void setReadyTorque_incTarget(int targetTorq, int incTime, int incTorq);
void setReadyPosition_Start(int option);
void execReady_TorqControl();
void execBite_BldcStart();
void execBite_BldcControl();
void execBite_torquewf_Start(int reqCnt);
void execBite_torquewf_Control(int reqCnt);
void execBite_ptn1_Start();
void execBite_ptn2_Start();
void execBite_ptn3_Start();
void execBite_ptn_Flag_Clear();
void exeBiteFlag_Clr_check();
void execBite_ptn_Stop();
void execBite_ptn1_Control();
void execBite_ptn2_Control();
void execBite_ptn3_Control();
void execHit_BldcStart();
void execHit_1st_BldcControl();
void execHiting();
void execHit_FightBldcStop();
void execHit_FightBldcStart(unsigned _ptn);
void execHit_FightBldcControl();
void execHit_ResistRandomStart();
void execHit_ResistRandom_ToCheck();
void execHit_HoldServo();
void execHit_ResistTorqStop();
void execHit_ResistTorqStart();
void execHit_ResistTorqControl();
void execHit_Holdon_BldcStart();
void execHit_Holdon_BldcControl();
void execHoldon_ptn1_Start();
void execHoldon_ptn1_BldcControl();
void Check_HoldOnPtn_AllClr();
void execHit_HoldOn_TorqStop();
void execHit_HoldOn_TorqStart();
void execHit_HoldOn_TorqControl();
void execHit_SetBreakOnceStart();
void send_breakmot(int _val);
void execHit_BreakStop();
void execHit_BreakStart();
void execHit_BreakControl();
void execHit_BreakDecStop();
void execHit_BreakDecStart();
void execHit_BreakDecControl();
void execFastHold_SetVal();
void execFastHold_Start();
void execFastHold_Control();
void execButtonCombo_SetVal();
void execButtonCombo_Start();
void execButtonCombo_Control();
void execRanding_SetVal();
void execRanding_Start();
void execRanding_Control();
void setAfter1sec_LogOut_Start();
void setAfterTo_LogOut_Control();
void execSuccessFail();
void execPointUp();
void execEnd();
void diagInput_AllOff();
void diagInput_BoardInAllOff();
void diagInput_BoardInAllOn();
void DiagInput_MotInAllOff();
void DiagInput_MotInAllOn();
void DiagInput_MainEcoderInAllOn();
void DiagInput_MechaInAllOff();
void DiagInput_ElecInAllOff();
void DiagInput_ElecInAllOn();
void diagInput_Cat0(String _msg);
void diagIput_Cat1(String _msg);
void diagInput_Cat3(String _msg);
void diagInput_Cat5(String _msg);
void diagInputTest(String _msg);
void diagOutput_AllOff();
void diagOutput_BoardOutAllOff();
void setRGBcolor(int pos, int Color);
void setOutRGBcolor(int pos, int Color);
void diagLmMotor_TimeoutStart(unsigned int _time);
void diagLmMotor_TimeOutStop();
void diagLmMotor_TimeoutControl();
void diagBbnMotor_TimeoutStart(unsigned int _time);
void diagBbnMotor_TimeoutControl();
void daigBbnMotor_TokTokEnd();
void diagBbnMotor_TokTokStart(unsigned int _ackKind, unsigned int _cnt);
void diagBbnMotor_TokTokControl(unsigned _opKind);
void diagOut_Cat1(String _msg);
void diagOut_Cat3(String _msg);
void diagOutputTest(String _msg);
void findDiagCode(String msg, stDiagCode code);
int controlDiagInput(String msg);
int controlDiagOutput(String msg);
void diagOutAllOff();
void Set_WaveData( String msg);
void Set_BiteData( String msg);
void Set_HitData(String msg);
void Set_BreakData( String msg);
void Set_ExtledData(String msg);
void Set_BbnTokTokData( String msg);
void Set_TorqAdjValData( String msg);
void Set_LedValue( String msg);
void Set_TorqHitHoldOn( String msg);
void Set_EnbData( String msg);
void subAcOff_WithOtherOff();
int CheckAppCmd();
int CheckPowerSwitch();
void check_PowerSwitch_Change_10ms();
void imuDataOutCmdSend(int onOff);
void usb5vChange_Check();
int DecodeLZ(unsigned char* src, unsigned char* dst, int opt);
void Exec_Download(String msg);
void Log_Control_Step();
void reelAllOff_SendControl();
void lmLongWireCheck_Start();
void lmLongWireCheck_Stop();
void checkBbnCcwCheck();
void lmWireControl_Start();
void lmWireControl_End();
void lmWireControl_Stop();
void lmWireControl();
void wat_isrLogOut();
void t1ms_Process_Exec();
void t10ms_Process_Exec();
void t100ms_Process_Exec();
void t500ms_Process_Exec();
void t1sec_Process_Exec();
void sendCmdSts_MainPwrOn();
void logUartRecvParsing();
void logRecvHandler();
void extIOuartRecvHandler();
void imuInterval_Measure();
void nowRecvHandler();
void uartRecvParsing();
void uartRecvHandler();
String anaVrtMotCmd(String msg);
void ana_SetDataSave(String msg);
String anaBtnLedCmd(String msg);
String anaDevSetReq(String msg);
String anaDevSet_MainMotor_AutoSet(String msg);
String anaDevFunctionReq(String msg);
String anaDevFunc_wireAging(String msg);
void mainMot_MinOut_AutoSet_Start(String msg);
void mainMot_MinOut_AutoSet_Stop(String msg);
void torqMotor_DefaultOut();
void mainMot_MinOut_AutoSet_Control1_Period();
void mainMot_MinOut_AutoSet_Control2_Time();
void mainMot_MinOut_AutoSet_Control3_36PulseTime();
void stbyTorqOffControl_Start();
void stbyTorqOffControl_Stop(int flag);
void stbyTorqOff_ReqCheck();
void stbyTorqOff_Control();
void wireAging_Test_Start(String msg);
unsigned short splitTextToStrings(String msg, String *s);
void wireAging_Test_Stop(String msg);
void wireAging_Test_Control();
void boardLed_Control();
void read_input_1ms();
void read_input_10ms();
void read_input_100ms();
void eeprom_test();
void subAc_Control_atReset();
short setConfig();
void ioLedOffSetting();
void ioPinSetting();
void wat_PidParaSet(String msg);
void wat_HookRandCheckExec();
void wat_PidStart_TargetSet(short _targetDist);
void wat_PidStop();
void spi_out(int _pin, int _val);
void system_counter();
int getBoardType();
void DF_Main_Application_Setup();
void DF_Main_Application_Loop();
void esp_soft_reset()
{
	ESP.restart();
}

void esp_hard_reset()
{
		esp_soft_reset();
}

// (V108) 성남공장 부팅불 대책 함수 시작-------
void SerialPortEnable()
{
	Serial.begin(115200);			// begin 
	Serial.setTimeout(10);			// wait 10ms, Default 1SEC
	int	nBeforeHeap = ESP.getFreeHeap();
	Serial.setRxBufferSize(DN_MAX_LEN);		// 8KB 가능 (PSRAM 활성화 시)
	int	nAfterHeap = ESP.getFreeHeap();

	g_SerialEnable = 1;
	LogPrintln(" LG] USBCDC SerialPortEnable(): " + String(nBeforeHeap) + " -> " + String(nAfterHeap));
}

void SerialPortDisable()
{
	int	nBeforeHeap = ESP.getFreeHeap();
	Serial.end();					// begin 
	int	nAfterHeap = ESP.getFreeHeap();
	pinMode(USB_DM, INPUT_PULLDOWN);	// INPUT Check
	pinMode(USB_DP, INPUT_PULLDOWN);	// INPUT Check

	g_SerialEnable = 0;
	LogPrintln(" LG] USBCDC SerialPortDisable(): " + String(nBeforeHeap) + " -> " + String(nAfterHeap));
}

void SerialPortPinInput()
{
	pinMode(USB_DM, INPUT_PULLDOWN);	// INPUT Check
	pinMode(USB_DP, INPUT_PULLDOWN);	// INPUT Check
//	digitalWrite(USB_DM, LOW);
//	digitalWrite(USB_DP, LOW);

	LogPrintln(" LG] USBCDC SerialPortPinInput()");
}
// (V108) 성남공장 부팅불 대책 함수 종료-------

// Hexa dump
char	hextbl[] = { "0123456789ABCDEF" };
struct	Lins {
	char	toff[8];
	char	spb1[2];
	char	hxdb[16][3];
	char	spb2[3];
	char	chdb[16];
	char	crlf;
	char	delm;
}	Lins;

void hex2asc(unsigned long hx, char* buf, int len)
{
	int	i;
	for (i = 0; i < len; i++, hx >>= 4)
		buf[len - i - 1] = hextbl[hx & 0x000f];
}

void HexDump(unsigned long toff, unsigned char* buff, int len)
{
	int	i, j, k;

	for (k = 0; k < len; k += 16, buff += 16)
	{
		memset((char*)&Lins, ' ', sizeof(Lins));
		hex2asc(toff + k, &Lins.toff[4], 4);
		j = ((len - k) < 16) ? (len - k) : 16;

		for (i = 0; i < j; i++)
		{
			if ((i & 0xf) == 8)
				Lins.hxdb[i][0] = '-';
			hex2asc(buff[i], &Lins.hxdb[i][1], 2);
			Lins.chdb[i] = ((buff[i] >= ' ') && (buff[i] <= 0x7e)) ? buff[i] : '.';
		}
		Lins.crlf = '\0';
		Lins.delm = '\0';
		LogPrintln(&Lins.toff[0]);
	}
}

// == Main Direct Control ===
// 1. DC 24V_B Off
// 2. SUB AC OFF
// 3. Board LED

void debugPort_Off()
{
	digitalWrite(FW_DEBUG_PIN, LOW);

}

void debugPort_On()
{
	digitalWrite(FW_DEBUG_PIN, HIGH);

}

#define PWR_ON	1
#define PWR_OFF	0
int pwrMode = PWR_ON;	//PWR_ON
//
void subAC_Off()
{
	digitalWrite(SUB_ACOFF_PIN, SUBAC_OFF_LVL);
	pwrMode = PWR_OFF;


}

void subAC_On()
{
	//digitalWrite(SUB_ACOFF_PIN, !(SUBAC_OFF_LVL));
	digitalWrite(SUB_ACOFF_PIN, SUBAC_ON_LVL);
	pwrMode = PWR_ON;


}


void DC24V_B_off()
{
	// TBD
}

//--- Board led---
void ledoff_BoardLedAllOff()
{
	digitalWrite(BD_LED2_PIN, LOW);
	if(DF_CONFIG_LMJIG != dfConfig)
	{
		digitalWrite(BD_LED3_PIN, LOW);
		digitalWrite(BD_LED4_PIN, LOW);
	}
}

//--- External LED OFF---
void ledOff_MbCent()
{
	extLed.off(LEDPOS_CENT_RGB);
}

void ledOff_MbBtm()
{
	extLed.off(LEDPOS_BTM);
}

void ledOff_OptLeft()
{
	extLed.off(LEDPOS_LEFT);
}

void ledOff_OptRight()
{
	extLed.off(LEDPOS_RIGHT);
}

// 1회 OFF
void extLed_AllOff()
{
	ledOff_MbCent();
	ledOff_MbBtm();
	ledOff_OptLeft();
	ledOff_OptRight();
}

// LED CONTROL OFF
void extLed_controlOff()
{
	unsigned int i;
	for(i=LED_IDX_CENT; i<LED_IDX_MAX; i++)
	{
		ledCont[i].cont = 0;	// OFF
	}
}

//--- External LED ON---
void ledOn_MbBtm()
{
	extLed.on(LEDPOS_BTM, (int)LED_MAX_DUTY);
}


//--- MOTOR OFF---
void torqOff()
{
	torqMotor.setValue(0);
}

void torqDefault()
{
	//torqMotor.setValue(torqMotor.defaultVal);
	torqMotor.setValue(DEFAULT_TORQ);
}

// (V108) 토크모터 초기값 출력 혹은 AP요구값 출력
int GetDefValue4torqMotor()
{
	return (recv_TorqMotor_Flag ? reqTorqueMotor : defaultTorqueMotor);
}

void servoDefault()
{
}

void bbnBldcOff()
{
	bbnMotor.offBldc();
}

//
void lmMotOff()
{
	lmMotor.offBldc();
}

#define ROD_BREAK_OFF_STR_DATA	"000"
void breakMotorOff()
{
	if(ROD_BOARD_V1 == rodBoardType)		// OLD BOARD Only
	{
		//String str = STX_BREAK_MOTOR + "00000000";
		eNow.write(DF_Protocol_MainToRod_Break, ROD_BREAK_OFF_STR_DATA);	  // Break OFF
	}
}

void motor_AllOff()
{
	// Motor OFF
  	torqOff();
  	servoDefault();
  	bbnBldcOff();
	lmMotOff();
	//breakMotorOff();
}

void motor_AllDefault()
{
	torqDefault();
	servoDefault();
	bbnBldcOff();
	lmMotOff();
	//breakMotorOff();
}

void led_AllOff()
{
	// 외부LED, Cent/Btm/Opt_Left,Opt_Right
	extLed_AllOff();
	//보드 LED, digitalWrite(BD_LED4_PIN, LOW);		// LED4 ON at PowerOn Reset
	ledoff_BoardLedAllOff();
}

void output_AllOff()
{
	// Motor OFF
  torqOff();
  servoDefault();
  bbnBldcOff();
  lmMotOff();

	// 외부, Cent/Btm/Opt_Left,Opt_Right
  extLed_AllOff();
  //digitalWrite(BD_LED4_PIN, LOW);	  // LED4 ON at PowerOn Reset
  ledoff_BoardLedAllOff();
}

volatile int reelOut_AllOff_Flag = 0;		// CLR
volatile int reelAlloffStep = 0;			// IDLE
void reelOut_AllOff()
{
	//Reel Out All OFF
	eNow.write(DF_Protocol_MainToRod_AllOutputsOff, "00");		// ALL OFF
}

//----- Now Send TERM---
#define SEND_NOW_STEP_TIME	10		// 10ms

// 10ms Force Step for POWER ON 1st
void reelOut_AllOff_ForceStep()
{
}

/*----------------------------------------------------------
 INT 2 STRING, for 5 CHAR, Fill ZERO(0)
----------------------------------------------------------*/
String fillZero2String5Char(int _int)
{
  String str;
  if(99999 < _int) {_int = 99999; }
  if (_int < 10)
  {
    str = "0000" + String(_int);
  }
  else if (_int < 100)
  {
    str = "000" + String(_int);
  }
  else if (_int < 1000)
  {
    str = "00" + String(_int);
  }
  else if (_int < 10000)
  {
    str = "0" + String(_int);
  }
  else
  {
    str = String(_int);
  }
  return str;
}


//-----------------------------------------------------------
//  5 char Limit -99999 ~ +99999
//-----------------------------------------------------------
String stringSign6Char(long _int)
{
	String str;
  if(99999 < _int) {_int = 99999; }
  else if(-99999 > _int) {_int = -99999; }

  if( 0 < _int) // +
  {
  	str = "+" + String(_int);
  }
  else if( 0 == _int)	// 0
  {
  	str = "000000";
  }
  else
  {
  	str = String(_int);
  }

  return str;
}

//
String fillZero2String3Char(unsigned int uint)
{
  String str;
  if(0)	{ }
 /*
  else if(-999 > uint) { _int = -999; }
  else if( 999 < _int) { _int =  999; }
  
  if(-99 >  _int )	// 3 char
  {
  	str = "-" + String(_int*-1);
  }
  else if(-9 >  _int )	// 2 char
  {
  	str = "-0" + String(_int*-1);
  }
  else if(0 >  _int )	// 1 char
  {
  	str = "-00" + String(_int*-1);
  }
  else if(0 ==  uint )	// 0
  {
  	str = "000";
  }
 */

  // 3 CHAR
  else if (10 > uint)	// 00 + 1char
  {
    str = "00" + String(uint);
  }
  else if (100 > uint)	// 0 + 2char
  {
    str = "0" + String(uint);
  }
  else					// 3char
  {
    str = String(uint);
  }
  return str;
}

/*----------------------------------------------------------
 INT 2 STRING, for 5 CHAR, Fill ZERO(0)
----------------------------------------------------------*/
String fillZero2StringSign4Char(int _int)
{
  String str;
  if(-999 > _int) { _int = -999; }
  else if( 999 < _int) { _int =  999; }
  
  if(-99 >  _int )	// 3 char
  {
  	str = "-" + String(_int*-1);
  }
  else if(-9 >  _int )	// 2 char
  {
  	str = "-0" + String(_int*-1);
  }
  else if(0 >  _int )	// 1 char
  {
  	str = "-00" + String(_int*-1);
  }
  else if(0 ==  _int )	// 4 char
  {
  	str = "0000";
  }
  else if (10 > _int)
  {
    str = "+00" + String(_int);
  }
  else if (100 > _int)
  {
    str = "+0" + String(_int);
  }
  else
  {
    str = "+" + String(_int);
  }
  return str;
}


String fillZero4Char(unsigned int _val)
{
	// Check Range
 	if(9999 < _val) { _val = 9999; }
	
	// Change to String 4 Char
  if(0) {}	// dummy
  else if (  10 > _val) { return "000" + String(_val); }
  else if ( 100 > _val) { return "00" + String(_val); }
  else if (1000 > _val) { return "0" + String(_val); }
  else 					{ return String(_val); }

}

/*----------------------------------------------------------
 INT 2 STRING, for 5 CHAR, Fill ZERO(0)
----------------------------------------------------------*/
String fillZero2StringSign6Char(int _int)
{
  String str;
  if(-99999 > _int) { _int = -99999; }
  else if( 99999 < _int) { _int =  99999; }
  
  if(-9999 >  _int )	// 3 char
  {
  	str = "-" + String(_int*-1);
  }
  else if(-999 >  _int )	// 3 char
  {
  	str = "-0" + String(_int*-1);
  }
  else if(-99 >  _int )	// 3 char
  {
  	str = "-00" + String(_int*-1);
  }
  else if(-9 >  _int )	// 2 char
  {
  	str = "-000" + String(_int*-1);
  }
  else if(0 >  _int )	// 1 char
  {
  	str = "-0000" + String(_int*-1);
  }
  else if(0 ==  _int )	// 4 char
  {
  	str = "000000";
  }
  else if (10 > _int)	// ~9 : 1 char
  {
    str = "+0000" + String(_int);
  }
  else if (100 > _int)	// ~99 : 2 char
  {
    str = "+000" + String(_int);
  }
  else if (1000 > _int)
  {
    str = "+00" + String(_int);
  }
  else if (10000 > _int)	// ~9999 : 4 char
  {
    str = "+0" + String(_int);
  }
  else
  {
    str = "+" + String(_int);
  }
  return str;
}

String StringFormat(char *fmt, int val)
{
	char buf[32];
	snprintf(buf, sizeof(buf), fmt, val);
	return buf;
}

/*--------------------------------------------------------
	callback Bobbin Encoder Rotate CALLBACK FUNC
---------------------------------------------------------*/
