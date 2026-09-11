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
const stLedCont initledCont[LED_IDX_MAX] =
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

//#if (IO_SERVO) - NG, 작성된 부분이 너무 많음. 추후 삭제.  TBD
ServoMotor sMotor;				// servoMortor객체
//#endif

BobbinMotor bbnMotor;					//Bobbin Motor(BLDC DC)
Encoder mainEnc;				// Bobbin Encoder + LM Encoder
//Encoder lmEnc;
//#define lmEnc mainEnc	// 

LmMotor lmMotor;		// LM MOT객체
// LM Encoder 제어는?

Led extLed;		// RGB

Eeprom eNvm;

//Diag mDiag;		//Mode Diag Object

FileSys fsInfo;	// manufactory Info

//------------------------------------------------------------

//=== ## ESP Now Data
//now_message now_msg;

String now_msg_str_cb = "";	// String ( ID+msg)
String now_msg_str = "";	// String ( ID+msg)
volatile int now_rcv_id_cb = 0;		// cb Get ID
volatile int now_rcv_id = 0;		// Ana ID

volatile int now_rcved = 0;		// 0 - not Rcv, 1- Rcved

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
#if (NEW_IF)
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
	
#else
String STX_GAMEENB        = "$01";
#endif

String STX_TORQUE_MOTOR   = "$02";
String STX_SERVO_MOTOR    = "$03";

#if (NEW_IF)
String STX_BBN_MOTOR     = "$04";
#else
String STX_BBN_MOTOR     = "$40";
#endif

String STX_BREAK_MOTOR    = "$05";
#if (NEW_IF)
String STX_LED_CONT			= "$06";
#endif

#if (NEW_IF)
String STX_RESERVE07		= "$07";	//RSV
  #if(IO_LM_MOT)
String STX_LM_MOTOR     	= "$07";
  #endif
#else
String STX_REST_DIST		= "$07";		// Move to GameStatus at NEW_IF
#endif
String STX_IMU_SET			= "$08";

String STX_IMU_DATA_STS		= "$09";


#if (NEW_IF)
String STX_VER_READ       = "$10";		// STX Version Read
String STX_GAME_STS       = "$11";		// STX GANE STATUS
String OLD_STX_BAT_LVL    = "$0C";		// BAT LVL
String STX_BAT_LVL        = "$12";		// BAT LVL
String STX_BUTT_SWC       = "$13";		// BUTTON SWITCH
String STX_HAND_ENC       = "$14";		// HANDLE ENCODER
#else
String STX_VER_READ       = "$0A";		// STX Version Read
String STX_GAME_STS       = "$0B";		// STX GANE STATUS
String STX_BAT_LVL		  = "$0C";		// BAT LVL
#endif

#if (NEW_IF)
String STX_DEV_CONT_REQ   = "$15";
String STX_GET_ADDR       = "$16";
String STX_SET_ADDR       = "$17";
String STX_APSTS_RESP     = "$18";
String STX_MOT_OUT_RATE_SET   = "$19";
#else
String STX_DEV_CONT_REQ   = "$10";
String STX_GET_ADDR       = "$11";
String STX_SET_ADDR       = "$12";
String STX_APSTS_RESP     = "$13";
#endif

#if (NEW_IF)
String STX_ROD_CONN			="$20";
String STX_IMU_CONN			= "$21";
String STX_PWRON_STS        = "$22";			// PowerOn Reset STX
	#define CONT_MAIN_BOARD	1
	#define REEL_MAIN_BOARD 2
#endif

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
#if (NEW_IF)
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
#else
String RESP_STX_BUTTON          = "$06";
String RESP_STX_ENCODER         = "$07";
String STS_APSTS_REQ         	= "$13";

String RESP_STX_ROD_CONN     	= "$04";
// Add Define RESP_STX
String RESP_STX_IMU_CONNECT     = "$05";
	#define RESP_IMU_FULLSTR_CONN		"$0511111111%"
	#define RESP_IMU_FULLSTR_DISCONN	"$0500000000%"

#endif

//===================================================
//=== AP CMD STX "INT" DEFINE
#define STX_CMD_MAINALIVE			0
#define STX_CMD_INFO_REQ			1
#define STX_CMD_TORQUE_MOTOR		2
#define STX_CMD_SERVO_MOTOR			3
#define STX_CMD_BLDC_MOTOR			4
#define STX_CMD_LED_CONT			6		// MAIN LED
//#define STX_CMD_REST_DIST			7
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
//#define ROD_REGI_END_TO_2ND		(2*1000)	// 2 SEC
#define ROD_REGI_END_TO_2ND		(4000)	// 4 SEC


//====DEFINE END ===============================


//-----------------------------------
// IMP 
// 1) STX_CMD_INT
#if (NEW_IF)
#define DEF_STX_GET_ADDR     16
#else
#define DEF_STX_GET_ADDR     11
#endif



//==============================

// Define CMD Data
String STATE_DISABLE   = "00000000";		// Game Disable = TimeOver = Game Waiting
String STATE_WAITING   = "00000000";		// Game Disable = TimeOver = Game Waiting
String STATE_ENABLE    = "11111111";		// Game Enable
String STATE_READY     = "22222222";		// Game Ready


//==============================

#if (NEW_IF)
#define STATE_CONN_UNKNOWN     "-1";		// UNKNOWN

#define STATE_DISCONN          "00";
#define STATE_CONNECTED        "01";
#define STATE_DISCONN_NOTINIT  "11";
#define STATE_DISCONN_NOTPEER  "12";
#define STATE_DISCONN_SLEEP    "88";
#define STATE_DISCONN_MAC      "99";

#else
//#define STATE_CONN_UNKNOWN     "000000--";		// UNKNOWN
#define STATE_CONN_UNKNOWN     "000000-1%";		// UNKNOWN

#define STATE_CONNECTED        "11111111";
#define STATE_DISCONN          "00000000";
#define STATE_DISCONN_NOTINIT  "00000011";
#define STATE_DISCONN_NOTPEER  "00000012";
#define STATE_DISCONN_SLEEP    "00000088";
#define STATE_DISCONN_MAC      "000000FF";

#endif

//#define IMU_CONN_3CHAR_INT 111
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
//#define checkTO(var,TO) ( (curr_ms_tick - var) > TO)	// if ( (curr - old) > TO )
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
#if 0	//TORQ_10W
	DEFAULT_TORQ+40, DEFAULT_TORQ+45, DEFAULT_TORQ+50, DEFAULT_TORQ+55, DEFAULT_TORQ+60, DEFAULT_TORQ+65, DEFAULT_TORQ+70, DEFAULT_TORQ+75
#endif
#if 1	// TORQ_30W
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*0),	//1 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*1), 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*2), 
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*3),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*4),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*5),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*6),
	DEFAULT_TORQ+DEEP_ADD_TORQ+(DEEP_STEP_TORQ*7)	//8
#endif
};

#define START_TORQ  DEFAULT_TORQ+10 //60
#define END_TORQ    DEFAULT_TORQ // 60

#define OFF_TORQ	(DEFAULT_TORQ-20)

#define WF_END_TORQ    DEFAULT_TORQ+20		// with Floats END Torq

#define HIT_LOW_TORQ    DEFAULT_TORQ+20		// HIT LOW Torq
#define HIT_END_TORQ    DEFAULT_TORQ+20		// HIT END Torq

//#define BITE_WAIT_TIME	1000		// 1000 ms
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
#if 0 	// TORQ_10W
	DEFAULT_TORQ, DEFAULT_TORQ+10, DEFAULT_TORQ+20, DEFAULT_TORQ+30, DEFAULT_TORQ+40, DEFAULT_TORQ+50, DEFAULT_TORQ+60,
	DEFAULT_TORQ+70, DEFAULT_TORQ+80, DEFAULT_TORQ+90, DEFAULT_TORQ+100, DEFAULT_TORQ+110		// Target by Fish Kind
	// DEFAULT_TORQ, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180
#endif
#if 1	// TORQ_30W_BELT
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
#endif
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

#if 0
typedef struct _diagCbTable
{
	int actNo;	
	void (*diagcallback)(int para);
}stDiagCbTable;

#define MAX_DIAG_OUT_NUM_FIX (1+3)
enum
{
	DIAG_ALL_OFF = 0,
	DIAG_OUT_TORQUE,
	DIAG_OUT_SERVO,
	DIAG_OUT_BOBBIN,

	MAX_DIAG_OUT_NUM
};

stDiagCbTable stDiagOutTb[MAX_DIAG_OUT_NUM][20] =
{
	// 0 - ALL OFF
	{
	  {0, diagOutAllOff},
	},
	// 1 - TORQUE MOTOR
	{ {0, torqOff},
	  {1, torqOff },
	},
	// 2 - SERVO MOTOR
	{ 
	  {0, servoDefault},
	  {1, servoDefault},
	},
	// 3 - BOBBIN MOTOR
	{ 
	  {0, bobbinOff},
	  {1, bobbinOff},
	  {2, bobbinOff},
	  {3, bobbinOff},
	  {4, bobbinOff},
	  {5, bobbinOff},
	  {6, bobbinOff},
	}
};
#endif

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

#if BBN_MOT_38W
#define MUL_BBN_PWR 1		// 38W
#else
#define MUL_BBN_PWR 2		// 16W
#endif

BBN_ACT sBbnActTbl[BBNACT_TBLMAX] =	// +1 Dummy : [0]
{
#if 0 	// 240901 - Very Strong (10W => 30W)
	// 0 - Set Val
	{  ( 22),   1000,   0,   1000 },	// 22 - 9%
	// 1 - Wave
	{  ( 28),  2000,   0,   2000 },	// 28 - 10%
	// 2 - Bite
	{  ( 30),    50,   0,     50 },		// 30 - 12%
	// 3 - Hit
	//{  (255*20/100),  300,   0,  100 },		// 51 - 18%
	{  ( 40),   300,   0,    100 },		// 40 - 18%
	// 4 - Fighting
	{  ( 25),   100,   0,     70 },		// 25 - 10%
	// 5 - HoldOn
	//{  ( 76),   150,   0,     50 },		// 76 - 30%
	{  ( 50),   150,   0,     50 },		// 76 - 30%
	// 6 - Randing
	{  ( 63),   100,   0,     70 },		// 63 - 25%
#endif

					// ON_DUTY,	ON_TIME,	OFF_DUTY,	OFF_TIME
#if 1	// 240908 TUnning #)W
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
#endif

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
const sLEDRGB sColorTBL[COLOR_MAX_NO]
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
//#define LVL01_BOOT	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BOOT))
//#define LVL01_USB5V	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_USB5V))
//#define LVL01_PWRSWC	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_PWRSWC))
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

//#define TE01_USB5V	(sensor1ms[SENSOR_1].te & (0x0001 << SNS_usb5V))
//#define TE01_PWRON_SWC	(sensor1ms[SENSOR_1].te & (BIT_SNS_USB5V))

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
#if (MKT_TEST_1)
unsigned int f9setFlag = 0;	// Not TEST

unsigned int HitMainTorq = 100;
unsigned int HoldMainTorq = 120;
#endif


//---- MKT TEST_2 : 20 Step---------------------

unsigned int	motTestAct = 0;		// 0 : REL, 1:TEST
unsigned int	motTestMainIdx = 20;	// Test Dafault 12 ( range : 1 ~ 20 )
unsigned int	motTestBbnIdx = 20;		// Test Dafault 12 ( range : 1 ~ 20 )


//---ROD REGIST BC----
uint8_t rcv_src_addr_cb[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };		// RCV src  ADDR
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
//#define LMHOME_SEN_ON HIGH
//#define LMHOME_SEN_OFF LOW
#define LM_HOME_SEN_ON		HIGH //LOW		// LOW ACTIVE
#define LM_HOME_SEN_OFF		LOW // HIGH
//#define LM_HOME_SEN_ON		0	//LOW
//#define LM_HOME_SEN_OFF		1	//HIGH
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
#if (0)	// NOT USE
	// 0 -Dummy
	{
		{0, },
	},
	// Fish 1- Level
	{
		{0,},	// dummy
		// power a,b,c : tm/bm/lm		
		{10,100,13,100,66,100},
		{20,100,47,100,70,100},
		{30,100,67,100,75,100},
	},
	// Fish 2- Level
	{
		{0,},	// dummy
		// power a,b,c : tm/bm/lm
		{20,100,13,100,66,100},
		{30,100,47,100,70,100},
		{40,100,67,100,80,100},
	},
	// Fish 3- Level
	{
		{0,},	// dummy
		// power a,b,c : tm/bm/lm
		{30,100,13,100,70,100},
		{40,100,47,100,80,100},
		{50,100,67,100,90,100},
	},

#endif
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
//#define ENC_INTV_BUFF_SIZE		1024
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
//#define PID_EXEC_ENC_MIN_TIME	10		// 간격 10ms
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
void esp_soft_reset()
{
	ESP.restart();
}

void esp_hard_reset()
{
	#if (CPU_RESET_PIN_SUPPORT)
		pinMode(RESET_PIN, OUTPUT);		// EN 핀을 출력 모드로 설정
		digitalWrite(RESET_PIN, LOW);		// EN 핀을 Low로 설정
		delay(400);		// 500ms 대기
		digitalWrite(RESET_PIN, HIGH);	// EN 핀을 High로 설정
		delay(100); 	  // 100ms 대기
		//pinMode(RESET_PIN, INPUT); 	  // EN 핀을 출력 모드로 설정
    #else
		esp_soft_reset();
	#endif
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
#if(!IO_LM_MOT)
	digitalWrite(FW_DEBUG_PIN, LOW);
#endif

  #if LOG_ELEC_IO
	LogPrintln(" LG] IOout DebugPort OFF");
  #endif
}

void debugPort_On()
{
#if(!IO_LM_MOT)
	digitalWrite(FW_DEBUG_PIN, HIGH);
#endif

#if LOG_ELEC_IO
	LogPrintln(" LG] IOout DebugPort ON");
#endif
}

#define PWR_ON	1
#define PWR_OFF	0
int pwrMode = PWR_ON;	//PWR_ON
//
void subAC_Off()
{
	digitalWrite(SUB_ACOFF_PIN, SUBAC_OFF_LVL);
	pwrMode = PWR_OFF;

  #if LOG_ELEC_IO
	LogPrintln(" LG] IOout SUB AC OFF");
  #endif

}

void subAC_On()
{
	//digitalWrite(SUB_ACOFF_PIN, !(SUBAC_OFF_LVL));
	digitalWrite(SUB_ACOFF_PIN, SUBAC_ON_LVL);
	pwrMode = PWR_ON;

#if LOG_ELEC_IO
	LogPrintln(" LG] IOout SUB AC ON");
#endif

}


void DC24V_B_off()
{
	// TBD
}

//--- Board led---
void ledoff_BoardLedAllOff()
{
	digitalWrite(BD_LED2_PIN, LOW);
#if(!IO_LM_MOT)
	digitalWrite(BD_LED3_PIN, LOW);
	digitalWrite(BD_LED4_PIN, LOW);
#endif
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
#if IO_SERVO
	sMotor.setAngle(0);
#endif
}

void bbnBldcOff()
{
	bbnMotor.offBldc();
}

//
void lmMotOff()
{
	#if (IO_LM_MOT)
		lmMotor.offBldc();
	#endif
}

#define ROD_BREAK_OFF_STR_DATA	"000"
void breakMotorOff()
{
  #if (IO_BREAK_MOT)
	//String str = STX_BREAK_MOTOR + "00000000";
	//eNow.write(PID_ROD_CTRL_NUM, str);	  // Break
	eNow.write(STR_PID_ROD_BREAK, ROD_BREAK_OFF_STR_DATA);	  // Break OFF
  #else
	if(ROD_BOARD_V1 == rodBoardType)		// OLD BOARD Only
	{
		//String str = STX_BREAK_MOTOR + "00000000";
		eNow.write(STR_PID_ROD_BREAK, ROD_BREAK_OFF_STR_DATA);	  // Break OFF
	}
  #endif
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
	eNow.write(STR_PID_REEL_OUT_ALL_OFF, "00");		// ALL OFF
}

//----- Now Send TERM---
#define SEND_NOW_STEP_TIME	10		// 10ms
//#define SEND_NOW_STEP_TIME	20	// 20ms

// 10ms Force Step for POWER ON 1st
void reelOut_AllOff_ForceStep()
{
#if 0	// NOT USE
	//reelOutAllOffFlag = SET;

	// REEL OUT ALL OFF
	imuDataOutCmdSend(IMU_DATA_OFF);
	delay(SEND_NOW_STEP_TIME);
	//reellMotorOff();
	sendRodVrtCmd(0,0,0,0); 	// Vrt Mot OFF
	delay(SEND_NOW_STEP_TIME);
	//reelBtnLedOff();
	sendRodBtnLedCmd(0,0,0,0,0);	// BTN LED ALL OFF
	delay(SEND_NOW_STEP_TIME);
	//
	breakMotorOff();
	delay(SEND_NOW_STEP_TIME);
#endif
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
void rotateChangeCallback(int _dir, int _cnt)
{
  //eNow.write(PID_CTRL_NUM, str);
  String dir = (_dir)? "CW_":"CCW";
  LogPrintln(" LG] BBENC " +  dir + ", cnt: " + String(_cnt));
}

String old_send_now_id = "00";
//===================================================================================================
// eNow Sent Log
void sent_cb_esp_now_sts(const uint8_t* mac_addr, esp_now_send_status_t status)
{
	#if (LOG_NOW_CB_SENT_FAIL)
		if(ESP_NOW_SEND_SUCCESS == status)	// ok
		{
			old_send_now_id = eNow.sendPid;
		}
		else		// fail
		{
	//		LogPrintln(" nowSent Fail id:" + old_send_now_id + "->" + eNow.sendPid);
			old_send_now_id = "00";
		}
	#endif
	#if (LOG_NOW_CB_SENT_REGI)
		if(rodRegistMode)
		{
			String rts = (ESP_NOW_SEND_SUCCESS ==status ? "Ok" : "Fail");
			LogPrintln(" LG] nowSent " + rts +",id:" + old_send_now_id + "->" + eNow.sendPid + "," + String(eNow.sendSeqNo) );		// Result, SeqNo, Addr_6
		}
	#endif
}

unsigned int enowOverCnt = 0;
unsigned int nowRecvSeqNo = 0;

/*  =====================================
         ESP Now Receive Callback
  ===================================== */
void recv_cb_esp_now_msg(const uint8_t *mac_info, const uint8_t *data, int data_len) {
	// Enable & Waiting때 수신할 것.
  //if ( !(isGameEnable || isGameStby) )
  //  return;
	
#if (LOG_NOW_RCV_OVER_WRITE)
	//if (now_msg.pid != -1)
	if (0 != now_rcv_id_cb)
	{
		int old_now_rcv_id_cb = now_rcv_id_cb;
		enowOverCnt++;

		char ch1=(char)(*(data+0));
		char ch2=(char)(*(data+1));
		
		//LogPrintln(" LG] nowOver," + String(enowOverCnt) + ",id:" +String(now_msg.pid) +","+ String(*(data+32))  );	// Cnt,Id
		LogPrintln(" LG] nowOver cnt:" + String(enowOverCnt) + ",id:" +String(old_now_rcv_id_cb) +","+String(now_rcv_id_cb) +"->"+ String(ch1) + String(ch2) ); // Cnt,Id
	}
#endif

#if 0 	// Char TEST
	if(0 == enowOverCnt)
	{
		enowOverCnt++;
		char ch1=(char)(*(data+0));
		char ch2=(char)(*(data+1));
		LogPrintln(" LG] nowOver cnt:" + String(enowOverCnt) + ",id:" +String(old_now_rcv_id) +","+String(now_rcv_id) +"->"+ String(ch1) + String(ch2) ); // Cnt,Id
	}
#endif

#if 1	// MOVE to Loop RCV_HANDLE
		// == Address Check ==
		// 1. 통상 : 송신한 낚시대가 타켓주소와 일치하지 않는 경우, MSG는 버림.
		// 2. Rod 등록 : 모두 받아 처리함
		if(!rodRegistMode)
		{
			for(int i = 5; i > -1; i--)
			{ 
				if(slave_board_addr[i] != *(mac_info+i))
				{
					//now_msg.pid = -1;
					now_rcv_id_cb = 0;	// TBD
					return;
				}
			}
		}
		else if(1 == rodRegistMode)	// REGI진입상태만 (개시전)
		{
			// 수신주소 저장
			memcpy(rcv_src_addr_cb, mac_info, 6);			// Save Src Addr
			//memcpy(rcv_dest_addr, (mac_info+6), 6);		// Save Dest Addr
		}
		//-----------------------------
#endif

	//COPYRECV Message
	char chStr[128];
	memcpy((uint8_t *)chStr, data, data_len);		// Save MSG
	chStr[data_len] = NULL;
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		now_msg_str_cb = String(chStr);							// 수신메시지 저정
		now_rcv_id_cb = now_msg_str_cb.substring(0,2).toInt();	// PID 추출 // ID(String) => INT(id)
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
	
#if(LOG_NOW_RCV_CB_SEQNO)
	//if(10 == now_msg.pid)
	if(PID_ROD_ALIVE_RESP == now_rcv_id_cb)	// Alive(10)
	{
		nowRecvSeqNo++;			// Test Recv Seq No
	}
	LogPrintln(" LG] nowRcv CB id: 10, Seq:" + String(nowRecvSeqNo));
#endif
	
}


//
// Batt Rest Level Recved
//
void battLevelSet( String msg)
{
	  //battLvlStr = msg; 	  // Save Battery Level
	  battLvlStr = msg.substring(1,(1+2)); 	  			// STRING 2 CHAR
	  ibatteryLvl = battLvlStr.substring(0, 3).toInt();	// INT 3 CHAR
	  //ibatteryLvl = battLvlStr.substring(1).toInt();	// INT 2 CHAR
	  
	#if (LOG_FW_BAT_CHG)
	  LogPrintln(" FS] RdBat " + battLvlStr); // Log
	#endif

}

//
//	Recved ROD_ALIVE RESP (eNow RECV)
//
void setRod_Connected()
{
	rodAlive_SendCnt = 0;		// Count CLR
	rod_conn_status = ROD_CONN;	// OK Slave Connected
}


//-----------------------------------------------------
// Improve Handle RECV STX From AP
//-----------------------------------------------------
void uartRecvHandlerImp()
{
#if 0	// TBD
  if (Serial.available())						// Length of Serial Buffer from PC
  {
    String msg = Serial.readStringUntil('%');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
    msg.trim();									// Delete Space in String

	// Not Message
    if (msg == "")			// Return, if not Recv
    {
	  LogPrintln(" AP] STCMD " + msg);		// LOG-SERIAL1 
      return;
    }

	// STX ($ Check)
	String stx = msg.substring(1,2);
	if(stx != "$")
    {
	  LogPrintln(" AP] STCMD " + msg);		// LOG-SERIAL1 
      return;
    }

	
	//unsigned int = msg.substring(2,4).toHex();
	//[출처] [Java] String to Hex byte Array|작성자 죠낸또이또이
	
	//------Handle CMD Control -------
	// 1st - String CMD - IF


	// 2nd - INT CMD - Switch
	int stx2nd = msg.substring(2,4).toInt();

	switch(stx2nd)
	{
		case DEF_STX_GET_ADDR:
			break;
		default:
			if(ROD_CONN == rod_conn_status)
			{
			}
			break;
	}
	
  }
#endif
}


bool isParaCheckOk(int idx, int val)
{
/*
    int i;
	for(i=0; i<MAX_PARA_CNT; i++)
	{
		if(paraCheck[i] == idx) break;
	}
	if(MAX_PARA_CNT <= i)
	{
		LogPrintln(" LG] WONG Para IDX)
    	return false;
	}
	else
	{
  		if(paraCheck[idx].min > val) 	return false;
  		if(paraCheck[idx].max < val)	return false;
		return true;
	}
*/
}

//
// SET TORW_VAL & BBN_BLDC_VAL
//
void waveType_TorqSet()
{
	stWavePtr stPtr;
	int i=0;

  for(i=0; i<2; i++)
  {
	if(0 == i) { stPtr = &stWaveTorqVal; }
	else       { stPtr = &stWaveBdutyVal; }
	
	//switch(waveType)
	{
		if((WAVE_PWR_MAX-1) < waveType) { waveType = (WAVE_PWR_MAX-1); }
		stPtr->tgTorq = stPtr->pwr[waveType];
	}
  }
}


// SET Hit Torq
int HitType_TorqSet(int _fishKind)
{
	if     (  1 > _fishKind ) { _fishKind = 1; }	// 1 ~20
	else if( 12 < _fishKind ) { _fishKind = 12; }	// Real : 1 ~ 12
	
	return stHitTorqVal.torqType[_fishKind];
}




int isExecBldc()
{
	if(	exeFlag_bldcWave
	||	exeFlag_bldcBite
	||  exeFlag_bldcHit
	||  exeFlag_bldcFight
	||  exeFlag_bldcHoldOn
	||  exeFlag_bldcRanding
	||  exeFlag_bldcSuccess
	)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//-----------------------------------------------------------------
// 토크모터 시간지정시 OFF제어 
unsigned int torqMotorStop_Step = 0x00;
unsigned long torqMotor_StopTimeOut = 0;
//

void torqMot_OffControlStart(unsigned int onTime)
{
	torqMotor_StopTimeOut = (unsigned long)onTime;
	torqMotorStop_Step = 0x10;	// Start
	exeFlag_torqMotorStop = 1;		// Stop Timer Start
}


void torqMot_OffControlStop()
{
	exeFlag_torqMotorStop = 0;		// Stop Timer 
	torqMotorStop_Step = 0;	// IDLE
}


void torqMotor_OffTime_Control()
{
	static unsigned long to;
	
	switch(torqMotorStop_Step)
	{
		case 0x00:
			break;	// IDLE
			
		case 0x10:	// Start
			setTO(to);
			torqMotorStop_Step = 0x20;
			break;
			
		case 0x20:	// 20 step, WAIT STOP TIMEOVER
			if(checkTO(to, torqMotor_StopTimeOut))
			{
				torqOff();
				#if (1)
					Resp2ApPrintln(STX_TORQUE_MOTOR + "000%");	// OFF RESP
				#endif
				torqMotorStop_Step = 0x90;
			}
			break;
			
		case 0x30:	// 30 step
			break;
			
		case 0x40:	// 40 step
			break;

			
		case 0x90:	// END
			exeFlag_torqMotorStop = 0;
			torqMotorStop_Step = 0x00;
			break;
		
		default:
			LogPrintln(" LG] TORQM OffTime Control Step NG!!!");
			break;
	}
}




//
//
unsigned int bbnMotorStop_Step = 0x00;
unsigned long bbnMotor_StopTimeOut = 0;
unsigned int gBbnHallCount = 0;							// (V108) for BBN Motor Test

//
void bbnMotor_OffTime_Start(unsigned int time)
{
	bbnMotor_StopTimeOut = (unsigned long)time;
	bbnMotorStop_Step = 0x10;	// Start
	exeFlag_bbnMotorStop = 1;		// Stop Timer Start
}

// (V108) BBN 모터 동작시 FG 센서 변화량 측정
void bbnMotor_OffTime_Control()
{
	static unsigned long to;
	static unsigned long _oldSen;				// (V108)
	
	switch(bbnMotorStop_Step)
	{
		case 0x00:
			break;	// IDLE
			
		case 0x10:	// Start
			setTO(to);
			bbnMotorStop_Step = 0x20;
			_oldSen = LVL01_BBNFG;				// (V108) 센서상태 저장
			break;
			
		case 0x20:	// 20 step, WAIT STOP TIMEOVER
			if(checkTO(to, bbnMotor_StopTimeOut))
			{
				bbnMotor.offBldc();
			#if RESP_IO_BBN_BLDC
				Resp2ApPrintln(STX_BBN_MOTOR + "00000000%");	// OFF RESP
			#endif
				bbnMotorStop_Step = 0x90;
			}
			else if(LVL01_BBNFG != _oldSen)		// (V108) FG 신호 변화
			{
				_oldSen = LVL01_BBNFG;			// (V108) 센서상태 저장
				gBbnHallCount++;				// (V108) 신호변화 cnt 증가
			}
			break;
			
		case 0x30:	// 30 step
			break;
			
		case 0x40:	// 40 step
			break;

			
		case 0x90:	// END
			exeFlag_bbnMotorStop = 0;
			bbnMotorStop_Step = 0x00;
			break;
		
		default:
			LogPrintln(" LG] BBNMT OffTime Control Step NG!!!");
			break;
	}
}

//
// LM MOT T/O TBD-LM
//
void lmMotor_OffTime_Start(unsigned int time)
{
#if(IO_LM_MOT)
	//lmMotor_StopTimeOut = (unsigned long)time;
	//lmMotorStop_Step = 0x10;	// Start
	//exeFlag_lmMotorStop = 1;		// Stop Timer Start
#endif
}

void lmMotor_OffTime_Control()
{
#if(IO_LM_MOT)

#endif
}


//
//
void clear_ExecFlag()
{
	exeWave = 0;
	exeBite = 0;
	exeHit = 0;
	exeFight = 0;
	exeResist = 0;
	exeResistOnceComp = 0;
	exeHoldon = 0;
	exeHoldOff = 0;
	exeFastHoldOn = 0;
	exeFastHoldOff = 0;
	exeButtonCombo = 0;
	exeButtonComboOff = 0;
	exeRanding = 0;
	exeSuccessFail = 0;
}

void execGameOver()
{
	exeGameOver = 1; //

#if IO_SERVO
	//Angle -90 ~ +90 
	sMotor.maxAdjAngle = 90;
#endif
	
	motor_AllOff();

	// End Process : All Exeute Flag CEAR
	clear_ExecFlag();
	
	reelOut_AllOff();

}

void setVal_Hit_MainPwr(String msg)
{
  #if IO_SERVO
		//Angle -90 ~ +90 
		sMotor.maxAdjAngle = 90;
  	#if 0
	// HUD Follow
		// HIt Angel SET 
		hitServoInterval = (hitServoAngle * 2 * 25 / 10);		// interval = Angle * [ 2.5ms/Degree ]
		if(100 > hitServoInterval) { hitServoInterval = 100; }	// min = 100 ms
  	#else
		hitServoInterval = 200; 	// 200ms
  	#endif

  #endif
	
		// Hit Fish Type SET & Targ Torq
		if( fishTypeFixENB) 	// Fix ?
		{
			if (!torqResistFix) 	// Not FIX = Auto Caculation
			{
				stHitTorqVal.targetTorq = HitType_TorqSet(fishTypeFix);
				if ( LEVEL_NORMAL == gameLevel) 	// Level 2
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 12 / 10) + 28 );		// Level 2 Resist = Target*1.2 +28
				}
				else if ( LEVEL_HARD == gameLevel)	// Level 3
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 14 / 10) + 21 );		// Level 3 Resist = Target*1.4 +21
				}
				else						// Level 1
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq *  9 / 10) + 41 );		// Level 3 Resist = Target*0.9 +41
				}
				stHitTorqVal.midTorq = (stHitTorqVal.holdonTorq + stHitTorqVal.lowTorq)/2; 
			}
		}
		else						// Auto ?
		{
			// 6) Find Fish Type AUTO from Break Val
			if (!torqResistFix) 	// Not FIX = Auto Caculation
			{
				// TBD, fishKind = by Break, = by AP Info(Kg/Size/ Rare)
			#if 0
					fishTypeAuto = fishKind_autoFromBreak(reqBreakMotor);
			#endif
				
				stHitTorqVal.targetTorq = HitType_TorqSet(fishTypeAuto); 
				if ( LEVEL_NORMAL == gameLevel) 	// Level 2
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 12 / 10) + 28 );		// Level 2 Resist = Target*1.2 +28
				}
				else if ( LEVEL_HARD == gameLevel)	// Level 3
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 14 / 10) + 21 );		// Level 3 Resist = Target*1.4 +21
				}
				else						// Level 1
				{
					stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq *  9 / 10) + 41 );		// Level 3 Resist = Target*0.9 +41
				}
			#if (MKT_TEST_1)
				if(f9setFlag)
				{
					stHitTorqVal.holdonTorq = HitMainTorq;	// test
				}
			#endif
				stHitTorqVal.midTorq = (stHitTorqVal.holdonTorq + stHitTorqVal.lowTorq)/2; 
			}
		}
	
}

//
//
void SetIMU_Measure_Out_OnOff(unsigned int _setClr)
{
	String msg;
	//String msg = String(_setClr);
	if(0 == _setClr)
	{
		//msg = STX_IMU_SET + "00000000%";
		msg = "00";
	}
	else
	{
		//msg = STX_IMU_SET + "11111111%";
		msg = "01";
	}
	eNow.write(STR_PID_IMU_DATA_OUT, msg);		// Control & LOG , Send TO ROD
	reqImuMeasFlag = _setClr;
}


//=== Define CMD GAME STATUS--------
#define GS_PROG_START	1
//
#define GS_RSV_02		2
#define GS_RSV_03		3
#define GS_WIRE_RDY		4	// 22
//
#define GS_LVL_SEL		11
#define GS_ZONE_SEL		21
#define GS_CAST_WAIT	31
#define GS_CAST_COMP	41
#define GS_BITE_1ST		51
#define GS_BITE_PTN		52
#define GS_FAIL			61
#define GS_HIT			71
#define GS_FIGHT		72
#define GS_HOLDON		81
#define GS_RANDING		82
#define GS_SUCCESS		91
#define GS_CONTINUE		92
#define GS_GAME_OVER	98
//
#define GS_PROG_END		99
//------------------------------

void sendSleepEnable(int Mode, int Time, int Id)
{
	String strMsg = StringFormat("%01d", Mode) + StringFormat("%04d", Time);
	gRodSleepMode = Mode;
	gRodSleepTime = Time;
	eNow.write(STR_PID_SLEEP_ENABLE, strMsg);	// ROD Sleep enable
	LogPrintln(" LG] SLEEP_ENABLE[" + String(Id) + "]: " + strMsg);
}

//
//	Send to AP(FG), Main/Rod/Imu/Batt
//
void sendCurrentStatus()
{
	String respMsg;
	
		  //=== 1) ROD CONN
		  if(ROD_CONN == rod_conn_status)  // CONN
		  {
			  //sndMsgConn = RESP_STX_ROD_CONN + "11111" + battLvlStr + "%";	  // SLAVE CONN, with BAT Level
			  respMsg = RESP_STX_ROD_CONN + STATE_CONNECTED;	  // SLAVE CONN, with BAT Level
			  respMsg += "%";
		  }
		  else			  // DIS CONN
		  {
			  respMsg = RESP_STX_ROD_CONN + STATE_DISCONN;
			  respMsg += "%"; 	  //eNOW Open, TBD(%삭제)
		  }
		  Resp2ApPrintln(respMsg);		  // Send to AP
		  
		#if LOG_DEV_CONN
		  LogPrintln(" LG] RODCN "+respMsg);
		#endif
		
		  //=== 2) IMU Connetion SEND
		  if( CONNECT == imu_conn_status)	  // CONN
		  {
			  respMsg = RESP_STX_IMU_CONNECT + STATE_CONNECTED;	  //
			  respMsg += "%";
		  }
		  else			  // DIS CONN
		  {
			  respMsg = RESP_STX_IMU_CONNECT + STATE_DISCONN;	  //
			  respMsg += "%";
		  }
		  Resp2ApPrintln(respMsg);		  // Send to PC, IMU_CONN
		  
		#if LOG_DEV_CONN
		  LogPrintln(" FW] IMUCN " + respMsg);		// LOG-SERIAL1
		#endif

		//=== 3) BAT LEVEL SEND
		//if(ROD_CONN == rod_conn_status)  // CONN
		//{
		//  	sndMsgConn = STX_BAT_LVL + battLvlStr + "%";	//
		//}
		//else
		//{
		//	sndMsgConn = STX_BAT_LVL + "-1%";	//
		//}
		respMsg = STX_BAT_LVL + battLvlStr + "%";	//
	  	Resp2ApPrintln(respMsg);		  // Send to PC
	  	
		//BAT충전상태, 
		if(ROD_CONN == rod_conn_status)  // CONN
		{
			respMsg = STX_BAT_LVL + BAT_STR_CHAGER_NOCHANGE;   //
			Resp2ApPrintln(respMsg);			// Send to PC
		}
	  	
	  	
		#if LOG_DEV_CONN
		  LogPrintln(" FW] BATLV " + respMsg); 	  // LOG-SERIAL1
		#endif

		// ===4) ROD Board Type SEND
		respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;
		respMsg += strRodBoardType;		// Old/New
		respMsg += "%";
		Resp2ApPrintln(respMsg); 	  // Send to PC

		#if LOG_DEV_CONN
			LogPrintln(" FW] INFOR " + respMsg);	  // LOG-SERIAL1
		#endif

		//=== 5) ROD Regist Wait T/O SEC
		respMsg = STX_INFO_REQ + ROD_REGIST_TO_SEC;
		respMsg += String(ROD_REGI_CANCEL_TO/1000);	
		respMsg += "%";
		Resp2ApPrintln(respMsg);
		
		LogPrintln(" FW] INFOR wt" + respMsg);
}

//=== LED CONTROL ===

//
// Init Val LED CONTROL
//
void initSet_LedContVal(int Act)
{
	int i=0;

	// SET VAL
	memset(ledCont, 0, sizeof(ledCont));
	for(i=0; i<LED_IDX_MAX; i++)
	{
		if (Act) {
			ledCont[i].oldcont = 	initledCont[i].oldcont;
			ledCont[i].cont = 		initledCont[i].cont;
		}
		ledCont[i].oldcont = 	initledCont[i].oldcont;
		ledCont[i].cont = 		initledCont[i].cont;
		ledCont[i].colorNo = 	initledCont[i].colorNo;
		ledCont[i].colorMany = 	initledCont[i].colorMany;
		ledCont[i].bTime = 		initledCont[i].bTime;
		ledCont[i].dTime = 		initledCont[i].dTime;
		ledCont[i].bStep = 		initledCont[i].bStep;
		ledCont[i].dStep = 		initledCont[i].dStep;
		ledCont[i].dColorCnt = 	initledCont[i].dColorCnt;
		ledCont[i].dReqCnt = 	initledCont[i].dReqCnt;
		ledCont[i].rsv_e = 		initledCont[i].rsv_e;
	}

	// LED START
	for(i=LED_IDX_CENT; i<LED_IDX_MAX; i++)
	{
		switch(ledCont[i].cont)
		{
			case 0: //OFF
				ledCont_OffStart(i);
				break;
			case 1: //ON
				ledCont_OnStart(i);
				break;
			case 2: //Blinking
				ledCont_BlinkStart(i);
				break;
			case 3: //Dimming
				ledCont_DimmStart(i);
				break;
		}
	}

	// LED DISPLAY ENB
	ledControlMode = 1;
	// LED Light
	ledLight = 100;	// 100% at POWER ON & DIAG OUT
	
}

//--- LED Control Start
void ledCont_OffStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK & DIMM Step CLR
	ledCont[pos].bStep = 0;
	ledCont[pos].dStep = 0;
	// Off 처리
	extLed.off(pos+2);
}

void ledCont_OnStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK & DIMM Step CLR
	ledCont[pos].bStep = 0;
	ledCont[pos].dStep = 0;

	// Color Set
	if(LED_IDX_CENT == pos)
	{
		setRGBcolor(pos, (int)ledCont[pos].colorNo);
		extLed.RGBon(LEDPOS_CENT_RGB, sRGB[pos].r, sRGB[pos].g, sRGB[pos].b );	
	}
	else
	{
		extLed.on(pos+2, (int)LED_MAX_DUTY);
	}
	// On 처리
}

void ledCont_BlinkStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// DIMM Step CLR
	ledCont[pos].dStep = 0;

	// Color Set,	TBD
	setRGBcolor((int)pos, (int)ledCont[pos].colorNo );

	// Blink Start
	ledCont[pos].bStep = 10;	// Start,	// execledCont_Blink
}


//
// Call 10ms, LED BLINK CONTROL
//
void execledCont_Blink(int pos)
{
	static unsigned long _to[LED_IDX_MAX] ={0,0,0,0,0};
	static unsigned int _oldStep[LED_IDX_MAX] = {0,0,0,0,0};
	//
 #if (LOG_LED_STEP)
	if(_oldStep[pos] != ledCont[pos].bStep)
	{
		LogPrintln(" LG] StepChg Led Blink_control " + String(ledCont[pos].bStep) +"/step");
		_oldStep[pos] = ledCont[pos].bStep;
	}
#endif

	switch(ledCont[pos].bStep)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// Start , LED ON
			// 
			if(LED_IDX_CENT == pos)
			{
				extLed.RGBon((pos+2), sRGB[pos].r, sRGB[pos].g, sRGB[pos].b );
			}
			else
			{
				extLed.on((pos+2), (int)LED_MAX_DUTY);
			}

			setTO(_to[pos]);
			ledCont[pos].bStep = 11;	// On Wait
			break;
		case 11:			// ON WAIT
			if(checkTO(_to[pos], ledCont[pos].bTime))
			{
				ledCont[pos].bStep = 20;
			}
			break;
			
		case 20:		// OFF
			extLed.off(pos+2);
			
			setTO(_to[pos]);
			ledCont[pos].bStep = 21;	// OFF Wait
			
			break;
			
		case 21:	// OFF WAIT
			if(checkTO(_to[pos], ledCont[pos].bTime))
			{
				ledCont[pos].bStep = 10;
			}
			break;
			

		case 30:
			break;
		case 40:
			break;

		case 90:
			break;
			
		default:
			break;
	}
}

//
void ledCont_DimmStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK Step CLR
	ledCont[pos].bStep = 0;

	// Color Set, 
	//Start Color CNT
	if(7 > ledCont[pos].colorMany)
	{
		ledCont[pos].dColorCnt = COLOR_RED;				// Color Oder Oder = 1st( red) 
	}
	else
	{
		ledCont[pos].dColorCnt = COLOR_WHITE; 			// Color Oder Oder = 1st( White)
	}
	ledCont[pos].dReqCnt = (ledCont[pos].dTime/LED_CONTROL_10MS/2);	// ReqCnt = Cal CNT
	setRGBcolor((int)pos, (int)ledCont[pos].dColorCnt );

	// Dimm Start
	ledCont[pos].dStep = 10;	// Start,	// execledCont_Dimm
}


//
//
void execledCont_Dimm(int pos)
{
	static unsigned long _to[LED_IDX_MAX] ={ 0,0,0,0,0 };
	static unsigned int _oldStep[LED_IDX_MAX] = { 0,0,0,0,0 };
	static unsigned int _cnt[LED_IDX_MAX] = { 0,0,0,0,0 };
	//
 #if (LOG_LED_STEP)
	if(_oldStep[pos] != ledCont[pos].dStep)
	{
		LogPrintln(" LG] StepChg Led Blink_control " + String(ledCont[pos].dStep) +"/step");
		_oldStep[pos] = ledCont[pos].dStep;
	}
#endif
	
	switch(ledCont[pos].dStep)
	{
		case 0: // IDLE
			break;
				
		case 10:	// Start , Color CNT
			if(LED_IDX_BTM == pos)
			{
				setRGBcolor(pos, COLOR_BLUE);	// BTM is BLUE ONLY
			}
			else
			{
				setRGBcolor(pos, ledCont[pos].dColorCnt);	// FIX ORDER, outRGB is Dimming OUT VAL
				//setRGBcolor(pos, ledColorOrder[pos][ledCont[pos].dColorCnt]); //SET ORDER, outRGB is Dimming OUT VAL
			}
			//setOutRGBcolor(pos, ledCont[pos].dColorCnt);	// outRGB is Dimming OUT VAL
			_cnt[pos] = 0;
			ledCont[pos].dStep = 20;
			// 
			break;
			
		case 20:		// INC
			_cnt[pos]++;
			if(_cnt[pos] > ledCont[pos].dReqCnt)
			{
				extLed.RGBon((pos+2), sRGB[pos].r, sRGB[pos].g, sRGB[pos].b);	// FULL COLOR ON
				setTO(_to[pos]);
				ledCont[pos].dStep = 21;
			}
			else
			{
				outRGB[pos].r = (sRGB[pos].r) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].g = (sRGB[pos].g) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].b = (sRGB[pos].b) * _cnt[pos] / ledCont[pos].dReqCnt;
				extLed.RGBon((pos+2), outRGB[pos].r, outRGB[pos].g, outRGB[pos].b);
			}
			break;
		case 21:
			if(checkTO(_to[pos], ledDimmHoldTime[pos]))		// 200ms HOLD
			{
				ledCont[pos].dStep = 30;
			}
			break;

		case 30:		// DEC
			_cnt[pos]--;
			if(_cnt[pos] <= 0)		// Count  = 0;
			{
				extLed.RGBon((pos+2), 0,0,0);	// OFF
				setTO(_to[pos]);
				ledCont[pos].dStep = 31;
			}
			else
			{
				outRGB[pos].r=(sRGB[pos].r) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].g=(sRGB[pos].g) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].b=(sRGB[pos].b) * _cnt[pos] / ledCont[pos].dReqCnt;
				extLed.RGBon((pos+2), outRGB[pos].r, outRGB[pos].g, outRGB[pos].b);
			}
			break;
		case 31:
			if(checkTO(_to[pos], ledDimmHoldTime[pos]))		// 200ms HOLD
			{
				ledCont[pos].dStep = 40;
			}
			break;

		case 40:
			ledCont[pos].dColorCnt++;
			if(ledCont[pos].dColorCnt > ledCont[pos].colorMany)
			{
				if(7 > ledCont[pos].colorMany)
				{
					ledCont[pos].dColorCnt = COLOR_RED;
				}
				else
				{
					ledCont[pos].dColorCnt = COLOR_WHITE;
				}
			}
			else
			{
			}
			ledCont[pos].dStep = 10;
			break;

		case 50:
			break;
			
		default:
			break;
	}

}

// Move to UPPER
//---10ms Call Ext Led Dimming Control

unsigned int ext_led_dim_step = 10;
unsigned int old_ext_led_dim_step = 0x00;
//static unsigned long _to = 0;
unsigned long tot_time = 0;
unsigned int cnt = 0;
unsigned int reqCnt = 0;
unsigned int loopCnt = 0;
unsigned int outCnt = 0;

unsigned int bDuty = (LED_MAX_DUTY*10);
unsigned int updownStep = (1*10);

//
// 10ms Call, OLD Dimming
//
void old_execCentLed_Dimming_Control(int color, int sec)
{

	//
 #if LOG_LED_STEP
	if(old_ext_led_dim_step != ext_led_dim_step)
	{
		LogPrintln(" LG] StepChg centLed_control " + String(ext_led_dim_step) +" [step]");
		old_ext_led_dim_step = ext_led_dim_step;
	}
#endif

	switch(ext_led_dim_step)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// Start , Calculation
			reqCnt = (sec * 1000) / 2 / 10;	// 10ms Count
			loopCnt = (reqCnt+128)/256; // (256*2);
			if(1 > loopCnt)
			{
				loopCnt = 1;
			}
			
			if(256 > reqCnt)
			{
				updownStep = (256*10)/reqCnt;
			}
			else
			{
				updownStep = (1*10);
			}
			
			bDuty = (0*10);

			// TBD Color, int color

			ledOut(LED_CENT_R_PIN, 0);
			ledOut(LED_CENT_G_PIN, 0);
			ledOut(LED_CENT_B_PIN, bDuty);	// Blue
			ext_led_dim_step = 20;
			break;
			
		case 20:		// UP
			bDuty += updownStep ;
			if((LED_MAX_DUTY*10) < bDuty)		// Over
			{
				outCnt = 0;
				bDuty = 0;
				ext_led_dim_step = 29;
			}
			else
			{
				ext_led_dim_step = 21;
			}
			ledOut(LED_CENT_B_PIN, (bDuty/10)); // Blue
			break;
			
		case 21:
			outCnt++;
			if( !(outCnt % loopCnt) )
			{
				if(outCnt > reqCnt)			// Up END?
				{
					outCnt = 0;
					if( LED_MAX_DUTY*10 < bDuty) bDuty = (LED_MAX_DUTY*10);
					ledOut(LED_CENT_B_PIN, (bDuty/10));	// Blue
					ext_led_dim_step = 29;
				}
				else
				{
					ext_led_dim_step = 20;
				}
			}
			break;
			
		case 29:
			outCnt++;
			if(outCnt > ledoffHoldTimeCnt)
			{
				if( LED_MAX_DUTY*10 < bDuty) bDuty = (LED_MAX_DUTY*10);
				outCnt = 0;
				ext_led_dim_step = 30;
			}
			break;
			
		case 30:		// Down
			bDuty -= updownStep;
			if((0*10) > bDuty)		// Down Over
			{
				bDuty = (0*10);
				outCnt = 0;
				ext_led_dim_step = 32;		// HoldTime
			}
			else
			{
				ext_led_dim_step = 31;
			}
			ledOut(LED_CENT_B_PIN, (bDuty/10));	// Blue
			break;
			
		case 31:
			outCnt++;
			if( !(outCnt % loopCnt) )
			{
				if(outCnt > reqCnt)		// Down Over
				{
					bDuty = (0*10);
					outCnt = 0;
					ext_led_dim_step = 32;	// Hoild
				}
				else
				{
					ext_led_dim_step = 30;
				}
			}
			break;

		case 32:		// DOWN HOLD
			outCnt++;
			if(outCnt > ledoffHoldTimeCnt)
			{
				if( 0*10 > bDuty) bDuty = (0*10);
				outCnt = 0;
				ext_led_dim_step = 20;
			}
			break;
			
		default:
			break;
	}
}



// IF idx Position : 1~4  =>  innerPosition:3~6 [ Inner  other: 0 ~ 2 = RGB)
void extLed_Off(unsigned int idx)
{
	switch(idx)
	{
		case LED_IDX_CENT:
		case LED_IDX_BTM:
			extLed.off((idx+2));
			break;

		case LED_IDX_LEFT:
		case LED_IDX_RIGHT:
			break;
		
		default:
			break;
	}
}

//
//
void extLed_On(unsigned int idx)
{
	unsigned int color;
	
	switch(idx)
	{
		case LED_IDX_CENT:
			// Color Set
			color = ledCont[idx].colorNo;
			if(COLOR_MAGENTA < color) { color = COLOR_WHITE; }	// 6(Magenta) Over White SET
			setRGBcolor((int)idx, (int)color);
			extLed.RGBon((idx+2), sRGB[idx].r, sRGB[idx].g, sRGB[idx].b);
			break;
		
		case LED_IDX_BTM:
			extLed.on((idx+2), (int)LED_MAX_DUTY);	// LED_MAX
			break;
		
		case LED_IDX_LEFT:
		case LED_IDX_RIGHT:
			break;
			
		default:
			break;
	}
}

/*
void extLed_Blink(int idx)
{
}

void extLed_Dimm(int idx)
{
	if(LED_IDX_CENT == idx)
	{
		old_execCentLed_Dimming_Control(COLOR_BLUE, dimming_time);
	}
}
*/

//
//	Call 10ms Control
//
void extLed_Control() 
{
	unsigned int pos=0;
	// 10ms, EXT LED CONTROL
	if(PWR_ON == pwrMode)
	{
		#if (0)	// NOT_USE
		// LED DIMMING(10ms)
		if(olddimming_time != dimming_time)
		{
			olddimming_time = dimming_time;
			ext_led_dim_step = 10;
		}
		#endif
		
		//if(DIAG_MODE != mainMode)
		if(ledControlMode)
		{

		#if (0)	// NOT_USE
			// Blue Dimming
			old_execCentLed_Dimming_Control(COLOR_BLUE, dimming_time);
			//LED Control
			// TBD
		#else 	// TBD ~ing
			for(pos=LED_IDX_CENT; pos<LED_IDX_MAX; pos++)
			{
				if(LED_CON_BLINK == ledCont[pos].cont)		// 2 - blinking
				{
					//extLed_Blink(pos);
					execledCont_Blink(pos);
				}
				else if(LED_CON_DIMM == ledCont[pos].cont)	// 3 - Dimming
				{
					//if(LED_IDX_CENT == pos)		// TBD , Position & RGB Port MATCHING
					{
						//extLed_Dimm(pos);
						execledCont_Dimm(pos);		// RGB Port 
					}
				}
				else	// OFF or ON
				{
					if(ledCont[pos].oldcont != ledCont[pos].cont)
					{
						if(LED_CON_OFF == ledCont[pos].cont)	// 0 - OFF
						{
							extLed_Off(pos);
						}
						else if(LED_CON_ON == ledCont[pos].cont)	// 1 - ON(100%)
						{
							extLed_On(pos);
						}
						ledCont[pos].oldcont = ledCont[pos].cont;
					}
					else
					{
						// NA
					}
				}
			}
		#endif
		}
	}

}

//====== Uart CMD ANA===
//
//
//

// (V108) 엔코더 센서 변화 내역 콘솔에 출력
void ana_TestExecution(String msg)
{
	int act;
	int paraNG = 0;		// 0-OK
	String respMsg;
	String logMsg;
	static int nEncIdxSave;
	static int nBbnHallCount;
	int i;
	unsigned int act2, act3;
	
	act = msg.substring(3,(3+1)).toInt();	// 1
	switch(act)
	{
		case 0:
			nEncIdxSave = sEncBuffIdx;
			nBbnHallCount = gBbnHallCount;
			break;
			
		case 1:		// Enc INV LOG OUT
			for (i=0; nEncIdxSave != sEncBuffIdx; i++) {
				LogPrintln(" lg] EnInt Interval:" +String(i)
					+ ","+ String(stEncIntvBuff[nEncIdxSave].tick)
					+ ","+ String(stEncIntvBuff[nEncIdxSave].width)
					+ ","+ String(stEncIntvBuff[nEncIdxSave].dir)
					+ ","+ String(stEncIntvBuff[nEncIdxSave].enc)
					+ ","+ String(stEncIntvBuff[nEncIdxSave].dist)
					);
				nEncIdxSave = (nEncIdxSave + 1) & (ENC_INTV_BUFF_SIZE-1);
			}
		//	LogPrintf("%10d LG] HallCnt : (%d) -> (%d) = %d\r\n", curr_ms_tick, nBbnHallCount, gBbnHallCount, gBbnHallCount - nBbnHallCount);	// TTTT
			LogPrintln(" LG] HallCnt: (" + String(nBbnHallCount) + ") -> (" + String(nBbnHallCount) + ") = " + String(gBbnHallCount - nBbnHallCount));
			break;
		case 3:		// 
			eNow.write(STR_PID_ROD_ALIVE_CHK, "");
			break;
		case 4:		// Disable
		//	SerialPortPinInput();
			SerialPortDisable();
			break;
		case 5:		// Enable
			SerialPortEnable();
			break;
		case 6:		// Disable
			subAC_Off();
			break;
		case 7:		// Enable
			subAC_On();
			break;

		case 9:		// boot info
			if (msg.substring(4,4+1) == "-") {
				fsInfo.deleteInfo(BOOTING_INFO);
			}
			else {
				fsInfo.saveInfo(BOOTING_INFO, msg.substring(4, 4+5));
			}
			break;
		case '8':		// Enable
		//	LogPrintln(" DN] ApCmd Recved len:" + String(g_SavePtr));
		//	HexDump(0, g_readBuf, g_SavePtr);
			g_SavePtr = 0;
			LogPrintf(" DN] ExecFlag(%d) RecvSeq(%d) g_DnExecStep(%d) RecvLen(%d) RecvIdx(%d)\r\n",
				g_DnExecFlag, g_DnRecvSeq, g_DnExecStep, g_DnRecvLen, g_DnRecvIdx);
		//	HexDump(0, g_DnSaveBuf, g_DnRecvIdx);
			break;
		default:
			paraNG = 1;
			break;
	}
	if (act < 2) {
		LogPrintln(" LG] Encoder: Cnt(" + String(iIsrbbnEncCnt) + ") Dir(" + String(sIsrBbnEncDir) + ") Dist(" + String(iWireDistance) + ") Intv(" + String(iEncIntrIntervalTime) + ")");
	}
	
	// Response
	respMsg = msg.substring(0,(0+5)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);
}

void analedContCmd(String msg)
{
	int lightReadFlag = 0;
	
	unsigned int pos;
	unsigned int act;
	unsigned int color;
	unsigned int time;

	unsigned int light;

  	pos = msg.substring(3,(3+1)).toInt();
	if(1> pos) { pos = 0; } if(4 < pos) { pos = 0; } // error	// if Pos=0, Start실행하지 않음.

  	act = msg.substring(4,(4+1)).toInt();
	if(0> act) { act = 10; } if(9 < act) { act = 10; } // error // NO ACTION

	switch(act)
	{
		case 0:		// off
			ledCont[pos].cont = act;
			
			ledCont_OffStart(pos);
			break;
			
		case 1:		// on
			ledCont[pos].cont = act;

			color = msg.substring(5,(5+2)).toInt();
			if(0> color) { color = 0; } if(14 < color) { color = 0; } // error
			ledCont[pos].colorNo = color;
			
			ledCont_OnStart(pos);
			break;
			
		case 2:		// blink
			ledCont[pos].cont = act; 

			color = msg.substring(5,(5+2)).toInt();
			if(0> color) { color = 0; } if(14 < color) { color = 0; } // error
			ledCont[pos].colorNo = color; 

			time = msg.substring(7,(7+4)).toInt();
			if(10> time) { time = 10; } if(9999 < time) { time = 9999; } // error
			ledCont[pos].bTime = time; 

			ledCont_BlinkStart(pos);
			break;
			
		case 3:		// dimm
			ledCont[pos].cont = act; 

			if(LED_IDX_BTM == pos)
			{
				ledCont[pos].colorMany = 1;		// FIX 1 Color = Blue (HardFix)
			}
			else
			{
				color = msg.substring(5,(5+2)).toInt();
				if(1> color) { color = 1; } if(14 < color) { color = 14; } // if Range Over,  Set Defuat(6) Color Many
				ledCont[pos].colorMany = color;	// IF SPEC :  52 -> 2, 56 -> 6, 57->7 
			}

			time = msg.substring(7,(7+4)).toInt();
			if(10> time) { time = 10; } if(9999 < time) { time = 9999; } // error
			ledCont[pos].dTime = time; 
			ledCont_DimmStart(pos);
			break;

		case 7:			// Color Oder, $06x7cccccccccccc%
			ledColorOrder[pos][1] = msg.substring(5,(5+1)).toInt();
			ledColorOrder[pos][2] = msg.substring(6,(6+1)).toInt();
			ledColorOrder[pos][3] = msg.substring(7,(7+1)).toInt();
			ledColorOrder[pos][4] = msg.substring(8,(8+1)).toInt();
			ledColorOrder[pos][5] = msg.substring(9,(9+1)).toInt();
			ledColorOrder[pos][6] = msg.substring(10,(10+1)).toInt();
			ledColorOrder[pos][7] = msg.substring(11,(11+1)).toInt();
			ledColorOrder[pos][8] = msg.substring(12,(12+1)).toInt();
			ledColorOrder[pos][9] = msg.substring(13,(13+1)).toInt();
			//ledColorOrder[pos][10] = msg.substring(14,(14+1)).toInt();
			break;
			
		case 8:			// read
			lightReadFlag=1;
			break;
		
		case 9:			// write
			light = msg.substring(5).toInt();
			if(  10 > light) { light = 10; }
			if(  100 < light) { light = 100; }
			ledLight = light;			// Light % 2 char 99 => 100
			lightReadFlag=1;
			break;
			
		default:
			//pos = 0;	// NO ATION ( Include Act 4,5,6,7)
			break;
	}

	if(lightReadFlag)
	{
		lightReadFlag = 0;
		light = ledLight;
		if(  10 > light) { light = 10; }
		if ( 100 < light) { light = 100; }
		Resp2ApPrintln("$060" + String(act) + String(light)+"%");
	}
	else
	{
		#if (RESP_LED_CONT_CMD)
			Resp2ApPrintln(msg + "%");
		#endif
	}
	
}


//
//
//
void anaRanding(String msg)
{

  #if (NEW_IF)
  	randingPtn = msg.substring(5,(5+2)).toInt();
  #endif
	exeFight = 0;			// Fight UpDown Control STOP
	
	execRanding_SetVal();
	
	exeRanding = 1;
	execRanding_Start();
	gameStatus = GAME_RANDING;
	
	LogPrintln(" LG] GameS RANDING");	// LOG-SERIAL1
}

//
//
//
void anaFastHold(String msg)
{
	unsigned int iDir = msg.substring(5,(5+2)).toInt();
	unsigned int iStartStop = msg.substring(7,(7+2)).toInt();
	// TBD

	LogPrintln(" LG] FastH Dir/stop: " + String(iDir) +"," + String(iStartStop));
}

//
//
//
void anaFastComboBtn(String msg)
{
	unsigned int iStartStop = msg.substring(5,(5+2)).toInt();
	// TBD
	
	LogPrintln(" LG] FastB COMBO StartStop: " + String(iStartStop));
}


//volatile uint uIntTest = 0;
//void sendRodVrtCmd(uint_t cnt, uint_t ontime, uint_t offtime)
void sendRodVrtCmd(int act, unsigned int cnt, unsigned int ontime, unsigned int offtime)
{
	String msg = "0";
	int rts = 0;		// 0-OK, 1-NG
	//if( 5 < act) { act = 0; }	//

	switch(act)
	{
		case 0:
		case 1:
			msg = String(act);
			break;
		
		case 2:
		case 3:
			rts = 1;
			break;		// 처리 안함
		
		case 4:
			msg = String(act);
			if(99 < cnt){ cnt = 1;}	// 기본 1회
			if(9 < cnt) { msg += String(cnt); }			// 2CHAR
			else		{ msg += "0" + String(cnt); }

			if(999 < ontime) { ontime = 100;}
			msg += fillZero2String3Char(ontime);
			if(999 < offtime) { offtime = 100;}
			msg += fillZero2String3Char(offtime);
			break;
			
		default:
			rts = 1;
			break;		// 처리안함
	}

	// error판별 처리
	if(rts)	// error
	{
	  #if (LOG_VRT_AP_CMD_ERR)
		// error
		LogPrintln(" lg] VrtMot CMD Error");
	  #endif
	}
	else
	{
		eNow.write(STR_PID_REEL_VRT_CONT,msg);
	}
	
}

//
// send ROD BTN LED CMD , 27,xAccNNNFFF
//
void sendRodBtnLedCmd(int posi, int act, unsigned int cnt, unsigned int ontime, unsigned int offtime)
{
	String msg = "00";
	int rts = 0;		// 0-OK, 1-NG

	if(2 < posi)	// 0,1,2
	{ 
		LogPrintln(" lg] BtnLED POSI NG:" + String(posi));
		return; 
	}	//처리  중지

	msg = String(posi);
	msg += String(act);
	//if( 5 < act) { act = 0; }	// OFF

	switch(act)
	{
		case 0:
		case 1:
			//msg = String(act);
			break;
		
		case 2:
		case 3:
			rts = 1;
			break;		// 처리 안함
		
		case 4:
			if(99 < cnt){ cnt = 5;}			// 기본 5 번
			if(9 < cnt) { msg += String(cnt); }			// 2CHAR
			else		{ msg += "0" + String(cnt); }

			if(999 < ontime) { ontime = 100;}
			msg += fillZero2String3Char(ontime);
			if(999 < offtime) { offtime= 100;}
			msg += fillZero2String3Char(offtime);
			break;

		default:
			rts = 1;
			break;		// 처리안함
	}

	// error판별 처리
	if(rts)	// Error
	{
		// error
		LogPrintln(" lg] BtnLED CMD Error");
	}
	else
	{
		eNow.write(STR_PID_BTN_LED_CONT,msg);
	}
	
}

// GAME STATUS DEFINE
#define GMWAT_MIN	GMWAT_AP_INFO	// 주의 , MIN
//--------------------------------------------------------------
#define GMWAT_AP_INFO	1
#define GMWAT_STBY		2
#define GMWAT_PAY		3
#define GMWAT_WIREHOME	4
#define GMWAT_SELECT	5
#define GMWAT_POINTMOVE	6
#define GMWAT_WAVE		7
#define GMWAT_BITE		8
#define GMWAT_HOOK		9
#define GMWAT_HIT		10
#define GMWAT_FAIL		11
#define GMWAT_FIGHT		12
#define GMWAT_HOLDON	13
#define GMWAT_HARDACT	14
#define GMWAT_RANDING	15
#define GMWAT_SUCCESS	16
#define GMWAT_POINTUP	17
#define GMWAT_TUTORIAL	18
#define GMWAT_BONUS		19
#define GMWAT_STAGECLR	20
#define GMWAT_MALL		21
#define GMWAT_CONTINUE	22
#define GMWAT_OVER		23
//--------------------------------------------------------------
#define GMWAT_MAX	(1+GMWAT_OVER)	// 주의 MAX :  STATUS추가시 MAX변경 필요.
//--------------------------------------------------------------

// LM FISH LEVEL SET, by AP FISH_PWR
short lmFish_3Level_Set(short pwr)
{
	short level;
	if(0) {}
	else if(66 < pwr)	{level = FISH_LVL_3; }
	else if(33 < pwr)	{level = FISH_LVL_2; }
	else				{level = FISH_LVL_1; }

	return level;
}

// 실페시 CALL
void lmReturnOrHome_Start()
{
	if(0) {}
	else if(LM_POSI_RIGHT <= lmPosi)	// Right구동중
	{	
		lmRight_Control_Return();
	}
	else if(LM_POSI_LEFT <= lmPosi)		// Left구동중
	{
		lmLeft_Control_Return();
	}
	else	// Home Control
	{
		lmHome_Control_Start(0, reqTorqueMotor);	//
	}	
}

volatile short wireHome_Step=0;
volatile short wireHomeExe_Flag=0;


void wireHomeControl_Start()
{
	wireHomeExe_Flag=1;
	wireHome_Step=STEP_START;
}
void wireHomeControl_Stop()
{
	wireHomeExe_Flag=0;
	wireHome_Step=STEP_IDLE;
}

// TBD-WIREHOME
void wireHome_Control()
{
}

// Wire HOME 거리(3미터), 카운트(286카운트) 설정
void setWireHome_EncCnt()
{
	mainEnc.setBbnCnt(WIRE_HOME_BBN_CNT);
	
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		iIsrbbnEncCnt = WIRE_HOME_BBN_CNT;
		iWireDistance = WIRE_HOME_DIST;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
	
	#if (LOG_LM_WIRE_CONTROL)
		//LogPrintln(" lg] WireH WIRE_HOME:" + String(mainEnc.bbnCnt) +", Dist:" + String(iWireDistance));
		LogPrintln(" lg] WireH WIRE_HOME:" + String(WIRE_HOME_BBN_CNT) +", Dist:" + String(WIRE_HOME_DIST));
	#endif
}


//byte nvm_fish_level = FISH_LVL_AUTO;
//byte nvm_fish_level = FISH_LVL_2;
byte nvm_fish_level = FISH_LVL_AUTO;

/*---------------------------------------------------------------------------
	Analysis RECV GAME STATUS WTA( Wire Auto Tension )
---------------------------------------------------------------------------*/
void anaGameStatusWAT(String msg)
{
	// Control 
	short cmdNG = 0;	// 0-OK
	short paraNG = 0;	// 0-OK
	
	short act;
	short dir;
	short para;

	short para1 = 0;
	short para2 = 0;
	short para3 = 0;

	short fishType = 0;
	short ptn = 0;	// "023" - Deeop & TokTok
	
	short ptn1 = 0;
	short ptn2 = 0;
	short ptn3 = 0;

	String respMsg;
	String logMsg;

	short cmd = msg.substring(3,5).toInt();
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		gmWatStatus = cmd;	// STATUS SAVE
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료

	//--- CMD분석
	switch(cmd)
	{

		// (02)
		case GMWAT_STBY:

		#if (FUNC_STBY_OFF_CONT)
			// 장치에러 검지]중에는 종료후 OFF제어 실시를 예약한다.
			if(devReadychkFlag)
			{
				// 중지 & 예약
				stbyTorqOffControl_Flag = 0;
				stbyTorqOffControl_Stop(0);			// (V108) 토크모터 동작 안함.
				stbyTorqOffControl_Req_Flag = 1;	// 예약
			}
			// 대기중 [토크OFF제어] 중이 아니면 제어를 시작한다. 
			else if(!stbyTorqOffControl_Flag)	// TBD : 기능 무효(0)/유효(1) 설정 추가?
			{
				stbyTorqOffControl_Start();
			}
		#endif
			sendSleepEnable(SS_WAIT, TIME_SLEEP_LIGHT, 2);

			break;
		
		//(03)
		case GMWAT_PAY:
			gRodSleepMode = SS_NONE;
			break;
		//(04)
		case GMWAT_WIREHOME:	// WIRE HOME(04)
		  #if (0)	// TBD_WIREHOME
			wireHomeControl_Start();
			// WireHome제어 개시 : TORQ=70(움직이지 마세요 문구)
			// ENC안정 확인: 일정시간(3초)에 +-3초과하지 않으면 READY
			//BBN Enc CNT값 설정
			// WIREHOME_COMP응답
			// TM Defaut 출력
		 #else
			gRodSleepMode = SS_NONE;
			setWireHome_EncCnt();
		 #endif
			break;
		//(05)
		case GMWAT_SELECT:
			break;
		//(06)
		case GMWAT_POINTMOVE:	// 포인트 이동 화면
			gRodSleepMode = SS_NONE;
		  #if(IO_LM_MOT)
			if(wireLongCheck_Flag) { lmLongWireCheck_Stop(); }		// Wire Control Check Stop
			if(lmWireControl_Flag) { lmWireControl_End(); }
		  #endif

		  #if(AUTO_TENSION_PID)
		  	wat_PidStop();		// PID STOP
		  #endif
		  
			break;
		//(07)
		case GMWAT_WAVE:
			break;
			
		//==($2508xxx%)  입질 물고기 Power(0~100)
		case GMWAT_BITE:
			para1 = msg.substring(5).toInt();
			if(100 < para1) { para1 = 100; }
		  #if(IO_LM_MOT)
			fishPwr = para1;	// Fish Power SAVE
			fishLevel = lmFish_3Level_Set(fishPwr);
			#if (LOG_WAT_FISH_LEVEL)
		  		LogPrintln(" lg] wBite fish:" + String(fishPwr) +"," + String(fishLevel));
			#endif
		  #endif
			break;

		// (09)
		case GMWAT_HOOK:
		 #if (IO_LM_MOT)
			lmLongWireCheck_Start();
		 #endif
		 #if (AUTO_TENSION_PID)
		 	wat_HookRandCheckExec();
		 #endif
			break;
		//(10)
		case GMWAT_HIT:
			break;
		//(11)
		case GMWAT_FAIL:	// FAIL - LM RETURN
			//1) LM_RETURN
		  #if(IO_LM_MOT)
		  	lmReturnOrHome_Start();
		  #endif
			break;

		//(12)
		case GMWAT_FIGHT:
			#if(AUTO_TENSION_PID)
				if(GMWAT_HIT == oldGmWatStatus)		// hooking => Hit => Fight
				{
			  		wat_PidStop();	  // PID STOP
				}
			#endif
			break;
	
		//== ( $2513xy% ) x-Dir, Y-Act
		case GMWAT_HOLDON :		//HOLD ON START/STOP

		  #if(IO_LM_MOT)
			if(wireLongCheck_Flag) { lmLongWireCheck_Stop(); }		// Wire Control Check Stop
			if(lmWireControl_Flag) { lmWireControl_End(); }
		  #endif
		  
		  #if(CONF_LM_JIG && CONFG_LM_MOT_CONTROL)		// LM CONTROL
			dir = msg.substring(5,6).toInt();
			act = msg.substring(6,7).toInt();
			switch(act)
			{
				case 1:		// START- 11,21,31
					if(0) {}
					else if (1 == dir)		//구동 Right요구 (화면 :Left요구)
					{
						//lmLeft_Control_Stop();
						if(LM_POSI_LEFT <= lmPosi && LM_POSI_RIGHT > lmPosi)		// LEFT구동중?
						{
							lmRightStartReq = 1;	// Right Req SET
						}
						// 구동부는 RIGHT
						// nvm Fish Level : 0-Auto(AP recv Level), 1=Leve1, 2-level2, 3-level3
						if(nvm_fish_level){fishType = nvm_fish_level; }	// FIX = LEVE_2
						else			{fishType = fishLevel; }	// AP recv FishPwr
						lmRight_Control_Start(fishType, MOT_PWR_LVL_b, reqTorqueMotor); 
					}	// 화면-좌/구동우측, EndTorqDuty
					else if (2 == dir)		// Center
					{
					  #if 0	// NA
					  	#if 0
							lmCenter_Control_Start(fishLevel, rcvlmPwr);
					  	#else
							if(LM_POSI_RIGHT <= lmPosi)	{lmRight_Control_Return();}
							else if(LM_POSI_LEFT <= lmPosi)	{ lmLeft_Control_Return(); }
					  	#endif
					  #endif
					}	// 중
					else if (3 == dir)		// 구동:LEFT요구 (화면 : RIGHT시작)
					{
						//lmRight_Control_Stop();
						//lmLeft_Control_Stop();
						if(LM_POSI_RIGHT <= lmPosi && LM_POSI_MAX > lmPosi)		// RIGHT구동중?
						{
							lmLeftStartReq = 1;	// TBD
						}
						// nvm Fish Level : 0-Auto(AP recv Level), 1=Leve1, 2-level2, 3-level3
						if(nvm_fish_level){fishType = nvm_fish_level; }	// FIX = LEVE_2
						else			{fishType = fishLevel; }	// AP recv FishPwr
						lmLeft_Control_Start(fishType, MOT_PWR_LVL_b, reqTorqueMotor); 
					}	// 화면-우/구동 좌측,, EndTorqDuty
					break;
					
				case 2:		// 버티기 종료 - 구동 Return후 STOP - 21,22,32
					//lmHold_Stop_Flag = 1;		// HoldOn ACT STOP
					// 방향 요구 무시, 현재 구동방향으로 판단,
					if(0) {}
					else if(LM_POSI_RIGHT <= lmPosi)	// Right구동중
					{	
						lmRight_Control_Return();
					}
					else if(LM_POSI_LEFT <= lmPosi)		// Left구동중
					{
						lmLeft_Control_Return();
					}
					else	// Home Control
					{
						// NG 처리
						#if (0)
							lmHome_Control_Start();	// TBD
						#else
							// Error : 버티기 시작 없이 버티기 종료 보냄( 중앙버티기 시작,종료는 ?)
						#endif
					}
					break;
					
				default:
					break;
			}
		break;
		  #endif
		//(14)
		case GMWAT_HARDACT:
			break;

		//(15)
		case GMWAT_RANDING:
		  #if (IO_LM_MOT)
			   lmLongWireCheck_Start();
		  #endif
		  #if (AUTO_TENSION_PID)
			wat_HookRandCheckExec();
		  #endif
			break;
		//(16)
		case GMWAT_SUCCESS:
			#if(AUTO_TENSION_PID)
			  wat_PidStop();	  // PID STOP
			#endif
			break;
		// (17)
		case GMWAT_POINTUP:
		  #if (IO_LM_MOT)
			lmLongWireCheck_Stop();		// NennoTame
		  #endif
			break;
		// (18)
		case GMWAT_TUTORIAL:
			break;
		// (19)
		case GMWAT_BONUS:
			break;
		// (20)
		case GMWAT_STAGECLR:
			break;
		// (21)
		case GMWAT_MALL:
			break;
		// (22)
		case GMWAT_CONTINUE:
			break;
		// (23)
		case GMWAT_OVER:
			break;

		default:
			break;
	}

	// 상태변화시
	if(oldGmWatStatus != gmWatStatus)
	{
		#if (FUNC_STBY_OFF_CONT)
		if(GMWAT_STBY == oldGmWatStatus)
		{
			// 1) 대기상태 -> 다른상태로 이동, 대기중 토크OFF중지
			stbyTorOffStop_Check();
			#if (LOG_STBY_OFF_CHAMGE_STS)
				LogPrintln(" lg] StbyF Cancel Other STS");
			#endif
		}
		#endif

		// END) 상태 갱신
		oldGmWatStatus = gmWatStatus;
	}

#if (RESP_GMWAT_CMD)
	//--- GAME WAT STATUS SAVE
	if(0 < gmWatStatus && GMWAT_MAX > gmWatStatus)
	{
		respMsg = STX_GAME_STS_WAT + msg.substring(3,5) + "00%";	// 0-OK
	}
	else
	{
		// Error, NOT DEFINE STATUS
		respMsg = STX_GAME_STS_WAT + msg.substring(3,5) + "-1%";	// 1-NG
	}
	//--- RESP
	Resp2ApPrintln(respMsg);
#endif
	
}


//-------------------------------------------------------------------------------------------------------
//  대기중 외에는 OFF제어를 중지한다.
void stbyTorOffStop_Check()
{
	// 상태가 대기중 외의 경우, 무조건 중지
	if(GMWAT_STBY != gmWatStatus)
	{
		// 예약 플래그 클리어
		stbyTorqOffControl_Req_Flag = 0;
		// 제어 플래그 및 상태 클리어 & 모터 OFF
		stbyTorqOffControl_Stop(1);			// (V108) 토크모터 기본값 출력
	}

}

//-------------------------------------------------------------------------------------------------------
#define FIGHT_PTN_CW_OFF	0
#define FIGHT_PTN_CW_CCW	1

/*---------------------------------------------------------------------------
	Analysis RECV GAME STATUS
---------------------------------------------------------------------------*/
void anaGameStatus(String msg)
{
	// Control 
	int msgKind = msg.substring(3,5).toInt();
	int msgOption = msg.substring(5,7).toInt();
	int para = msg.substring(7,9).toInt();

	int para1 = 0;
	int para2 = 0;
	int para3 = 0;

	int fishType = 0;
	int ptn = 0;	// "023" - Deeop & TokTok
	
	int ptn1 = 0;
	int ptn2 = 0;
	int ptn3 = 0;

	switch(msgKind)
	{
		case 1 :		// PROGRAM Start
			//torqMotor.setValue(torqMotor.defaultVal);
			//motor_AllDefault();						// (V108) delete 토크 모터 기본값 출력 생략
			motor_AllOff();								// (V108) replace 모든 모터 off 시킴

			// SEND STATUS CNT CLR
			recvMainConn_Pwr1stCnt = 0;

			gameStatus = PROG_START;

			// Delete Send Current to MAIN
			
			//sendCurrentStatus();
			//recvMainConn_Pwr1stCnt++;

			// Send to ROD REQ_ROD_INFO
			eNow.write(STR_PID_ROD_INFO_REQ, "");		// 	Send TO ROD

			#if (FUNC_STBY_OFF_CONT)
				// STBY [토크OFF제어] 중이 아니면 OFF제어를 실시한다.
				// 삭제 -불필요(게임실행후 반드시 대기상태 옴)
				//if(!stbyTorqOffControl_Flag)
				//{
				//	stbyTorqOffControl_Start();
				//}
			#endif
			if (gRodSleepMode == SS_EXIT) {
				gRodSleepMode = SS_WAIT;
			}
			LogPrintln(" LG] GameS PROG Start ");
			
		#if (FUNC_RF_CERTIFICATION_TEST)			
			//rodReelVrtControl_Start(3, 500, 100);		// FIX
			sendRodVrtCmd(4, 1, 600, 100);	// act, cnt, time
			//delay(5);
			sendRodBtnLedCmd(2, 4, 5, 500, 200);	// posi,act,cnt,time
		#endif
 			break;

		// Program END
		case 99:	// Program End

			isGameEnable = false;				// MAIN DIS

			g_RecvCmdCount = 0;					// (V108)
//			g_FlagCmdStop  = 1;					// (V108)

			execHit_FightBldcStop();
			motor_AllOff();

			// TBD, AP수정 필요(통신처리 소용시간 대기후 프로그램 종료 필요) . IMU STOP SEND
			//SetIMU_Measure_Out_OnOff(IMU_DATA_OFF);			// IMU OUT = 0
			reelOut_AllOff();

			// SEND STATUS CNT CLR
			recvMainConn_Pwr1stCnt = 0;

			gameStatus = PROG_END;
			exeProgramEnd = 1; // Flag Clear

			sendSleepEnable(SS_EXIT, TIME_SLEEP_DEEP, 3);
			LogPrintln(" LG] GameS ProgEND");   // LOG-SERIAL1

		// 릴 진동모터, 버튼LED 제어
			//rodReelVrtControl_Start(3, 500, 100); //무의미, Main이 RESET되므로 실행 안됨.
 			break;

		case 53 :		// NEW Bite Pattern For FW BITE Control
			// $1153Kxyz%

			fishType = msg.substring(5,(5+1)).toInt();	// Fish Kind 1~ 6
			if(1 > fishType ) {fishType = 1; }
			if(6 < fishType ) {fishType = 6; }
		
			ptn = msg.substring(6,(6+3)).toInt();	// PTN2 = xyz
			// 1) xyz = "000"
			if(0 == ptn)
			{
				//	// NA
			}
			// 2) xyz = "999" Bite Stop
			else if(999 == ptn)	// Bite STOP
			{
				exeBite = 0;
				bitePtn_Flag = 0;
				execBite_ptn_Stop();

				//임시,  VRT Mot 1초 On/Off, 회수
				//rodReelVrtControl_Start(12, 50, 30);		// FIX
				sendRodVrtCmd(4, 10, 50, 50);
 			}
			
			// 2) else : ex) xyz = 123, 
			else	// BITE CONTROL REQ
			{
			
				fishTypeAuto = fishType;
		
				ptn1= msg.substring(6,(6+1)).toInt();
				ptn2= msg.substring(7,(7+1)).toInt();
				ptn3= msg.substring(8,(8+1)).toInt();
		
				if(0 != ptn1) { exeBitePtn1flag = 1; execBite_ptn1_Start(); }
				if(0 != ptn2) { exeBitePtn2flag = 1; execBite_ptn2_Start(); }
				if(0 != ptn3) { exeBitePtn3flag = 1; execBite_ptn3_Start(); }
		
				exeBite = 1;
				bitePtn_Flag = 1;

				//임시,  VRT Mot 1초 On/Off, 회수
				// 12회 제어하지만 모터특성(?)으로 6회 체감됨
				//rodReelVrtControl_Start(12, 50, 30);	// FIX
				sendRodVrtCmd(4, 10, 50, 50);
 			}
				
			#if (LOG_BITE_IMPROV)
				LogPrintln(" LG] GameS BITE_START fish:" + String(fishType) + " ptn: " + String(ptn));
			#endif

			break;

		case 84 :		// NEW HoldOn IF for FW Control
			// $1184KP%

			fishType = msg.substring(5,(5+1)).toInt();	// Fish Kind 1~ 6
			if(1 > fishType ) {fishType = 1; }
			if(6 < fishType ) {fishType = 6; }
		
			//ptn = msg.substring(6,(6+1)).toInt();	// PTN
			ptn = msg.substring(6).toInt();	// PTN
			if(0 == ptn)
			{
				//	// NA
			}
			// 2) else : 1=Exec, 2~9: Reserved
			else if(1 == ptn)
			{
			
				fishTypeAuto = fishType;
		
				if(0 != ptn) { execHoldon_ptn1_Start(); }
				//exeHoldon = 1;			// Old Flag
			}
				
			#if (LOG_HOLDON_IMPROV)
				LogPrintln(" LG] GameS HOLD_START fish:" + String(fishType) + " ptn: " + String(ptn));
			#endif

			break;		
			
//=============================================================
// 이하는 AP_MOT_CONT시 무효함.
			
#if (AP_MOT_CONT)	// AP MOTOR  ALL CONT
	// NA
#else		// NOT AP_MOT_CONT

  #if (NEW_IF)
		// GAME STBY (=WAIT)
	  	case 2:
			motor_AllDefault();
			// SEND STATUS CNT CLR
			gameStatus = GAME_WAIT;
			isGameEnable = false;				// MAIN DIS ( Not PayMent)

			LogPrintln(" LG] GameS STBY(wait) " + String(msgOption));
			break;

		// CARD PayMent
	  	case 3:
			isGameEnable = true;				// MAIN ENB ( PayMent)

			LogPrintln(" LG] GameS PayMent " + String(msgOption));
	  		break;
  #endif
		//case 22 :		// Ready Position
		case 4 :		// Ready Position
			motor_AllDefault();

			// TBD, min < > max
			rdyMsg = msg;
			setReadyPosition_Start(msgOption);

			LogPrintln(" LG] ExRDY Control, opt: " + String(msgOption));
			break;
			
  #if (NEW_IF)
		case 10 :		// Game Mode
			// TBD
			
			LogPrintln(" LG] GameS GameMODE: " + String(msg));
			break;

		case 12 :		// Charator Selet
			// TBD
			
			LogPrintln(" LG] GameS CH_SEL: " + String(msg));
			break;

		case 14 :		// Tutorial / Game Start
			// TBD
			
			LogPrintln(" LG] GameS TUTOR/GAME: " + String(msg));
			break;
  #endif			
		case 11 :		// Level Select
			motor_AllDefault();
			
			
			//save
			para1 = msg.substring(9,(9+2)).toInt();
			if(LEVEL_MIN > para1) { para1 = LEVEL_MIN;}
			if(LEVEL_MIN < para1) { para1 = LEVEL_MAX;}
			gameLevel = para1;			// Game Level ( 3 Level )
			
			waveType = gameLevel;		// Wave Type(3 level)

			gameStatus = GAME_STBY;
			
			LogPrintln(" LG] GameS Lvl " + String(gameLevel) + " [lvl]");
			break;
  #if (NEW_IF)			
		case 13 :		// Zone Select
			motor_AllDefault();
			//save
			// TBD, min < > max
			gZone = msg.substring(5,(5+2)).toInt();
			// TBD, min < > max
			gStage = msg.substring(7,(7+2)).toInt();
			//torqMotor.setValue(torqMotor.defaultVal);
			//torqMotor.setValue(65);	 // TBD
  #else
		case 21 :		// Zone Select
			motor_AllDefault();
			//save
			// TBD, min < > max
			gZone = msg.substring(7,(7+2)).toInt();
			// TBD, min < > max
			gStage = msg.substring(9,(9+2)).toInt();
			//torqMotor.setValue(torqMotor.defaultVal);
			//torqMotor.setValue(65); 	 // TBD
  #endif

			gameStatus = GAME_STBY;

			LogPrintln(" LG] GameS Zone " + String(gZone) + "," + String(gStage) + " [Stage/Zone]");
			break;
			
		case 31 :		// Casting Wait
			clear_ExecFlag();
			gameStatus = CAST_WAIT;

			LogPrintln(" LG] GameS CAST_WAIT" + String(msgOption));
			break;

  #if (NEW_IF)
		case 32 :		// Casting Complet
			para1 = msg.substring(5,(5+3)).toInt();
			if( 1 > para1) { para1 =  1; }
			if(99 < para1) { para1 = 99; }
  #else
		case 41 :		// Casting Complet
			para1 = msg.substring(8,(8+3)).toInt();
			if( 1 > para1) { para1 =  1; }
			if(99 < para1) { para1 = 99; }
  #endif
			restPowerDist = para1;

			gameStatus = CAST_COMP;

		// TBD, Wave Not Exe Condition Check ( each Zone )
			if(1)
			{
				waveType_TorqSet();
				exeWave = 1;
				execWave_torq_bbn_Start();
				gameStatus = GAME_WAVE;
			}
			
			LogPrintln(" LG] GameS CastCOMP, Dist: " + String(restPowerDist) + "/M, wTrq: " + String(stWaveTorqVal.tgTorq) + ", wBldc: " + String(stWaveBdutyVal.tgTorq));	  // LOG-SERIAL1
			break;

		case 51 :		// 1ST BITE = Fish Kind
			// TBD, min < > max
			strFishKind = msg.substring(5,(5+4));
			fishKg_flt = strFishKind.toFloat();
			fishKg_dbl = (double)fishKg_flt;

			// ADD 24.8.22
			fishTypeAuto = fishKind_FromKg(fishKg_dbl);
			biteType_TorqSet(fishTypeAuto);


			fishSize = msg.substring(9,(9+3)).toInt();
			fishRare = msg.substring(12,(12+1)).toInt();

			execBite_torquewf_Start(biteCnt); 
			exeBite = 1;

			gameStatus = GAME_BITE;

			LogPrintln(" LG] GameS Bite_1st " + String(fishKg_dbl) + "/Kg, " + String(fishSize) + "/cm, " + String(fishRare) + "/rare");	  // LOG-SERIAL1
			break;
			
		case 52 :		// 2ND~ BITE = Bite Pattern
			if(GAME_BITE == gameStatus)
			{
			  #if (NEW_IF)
				// TBD
				// Pattern MOVE
				int bitePtn = msg.substring(5,(5+3)).toInt();
			  #else
				// TBD
				// Pattern MOVE
				int bitePtn = msg.substring(8,(8+3)).toInt();
			  #endif
			
				LogPrintln(" LG] GameS, BitePtn: " + String(bitePtn));
			}
			else
			{
				// Error LOG
			}
			break;

		case 53 :		// Bite Pattern_2
			// $1153Kxyz%

			ptn = msg.substring(6,(6+3)).toInt();	// PTN2 = xyz
			// 1) xyz = "000"
			if(0 == ptn)
			{
				break;	// NA
			}
			// 2) xyz = "999" Bite Stop
			else if(999 == ptn)	// Bite STOP
			{
				exeBite = 0;
				execBite_ptn_Stop();				
			}

			// 2) else : ex) xyz = 123, 
			else
			{

				fishTypeAuto = msg.substring(5,(5+1)).toInt();	// Fish Kind 1~ 6
				if(1 > fishTypeAuto ) {fishTypeAuto = 1; }
				if(6 < fishTypeAuto ) {fishTypeAuto = 6; }

				int ptn1= msg.substring(6,(6+1)).toInt();
				int ptn2= msg.substring(7,(7+1)).toInt();
				int ptn3= msg.substring(8,(8+1)).toInt();

				if(0 != ptn1) { exeBitePtn1flag = 1; execBite_ptn1_Start(biteCnt); }
				if(0 != ptn2) { exeBitePtn2flag = 1; execBite_ptn2_Start(1); }
				if(0 != ptn3) { exeBitePtn3flag = 1; execBite_ptn3_Start(4); }

				exeBite = 1;
			}
			
		#if (LOG_BITE_IMPROV)
			LogPrint(" LG] fish: " + String(fishTypeAuto) + " ptn: " + String(ptn));
		#endif
		
			break;

		case 61 :		//  Fail
			// Fight Control STOP
			execHit_FightBldcStop();
			//torqDefault();
			motor_AllDefault();

			exeSuccessFail = 1;
			gameStatus = GAME_FAIL;
			execHit_BreakDecStop();

			LogPrintln(" LG] GameS FAIL");   // LOG-SERIAL1			break;
			break;
  #if (NEW_IF)
		case 62 :		//  HIT
  #else
		case 71 :		//  HIT
  #endif
			if( GAME_BITE == gameStatus)			// if ONLY Game BITE
			{
				setVal_Hit_MainPwr(msg);

				// 1) Beak Control ONCE HIT
				execHit_SetBreakOnceStart(); 	// NOT Effective ?
				// 2) Resist Once
				execHit_ResistRandomStart();		// Regist Random Timer START, After 3000 ms
				// 3) BBN Start
				execHit_BldcStart();
				exeHit = 1;
				exeHit_1st = 1; 		// 1st, BLDC Control at HIT

				gameStatus = GAME_HIT;			

				LogPrintln(" LG] GameS, ExeHit, FishKind: " + String(fishTypeAuto));	// LOG-SERIAL1
				LogPrintln(" LG] GameS Hit Main, FIX:" + String(torqResistFix) + ",Tgt:" + String(stHitTorqVal.targetTorq) + ",Hold:" + String(stHitTorqVal.holdonTorq) + ",low: " + String(stHitTorqVal.lowTorq) + ",Mid: " + String(stHitTorqVal.midTorq) );
				LogPrintln(" LG] GameS Hit Bldc, hit: " + String(sBbnActTbl[BBNACT_HIT].onDuty) + ",Hold: " + String(sBbnActTbl[BBNACT_HOLDON].onDuty)); // LOG-SERIAL1
			}
			else
			{
				// Error LOG
			}
			break;

  #if (NEW_IF)
		case 63 :		//  Fight
			restPowerDist = msg.substring(5).toInt();
			if( 10 > restPowerDist ) { restPowerDist = 10; }
			if(200 < restPowerDist ) { restPowerDist = 200; }
  #else
		case 72 :		//  Fight
			// SET REST DISTANCE, at 1st Only
			{
				restPowerDist = msg.substring(8).toInt();
				if( 10 > restPowerDist ) { restPowerDist = 10; }
				if(100 < restPowerDist ) { restPowerDist = 100; }
			
			}
  #endif

			// TBD, TORQ SET

			// TBD, BLDC SET
			
			exeFight = 1;
			
			gameStatus = GAME_FIGHT;

			execHit_FightBldcStart(FIGHT_PTN_CW_CCW);	// Rod1 UpDown Control PTN=CW-CCW
			// TBD , NEED? Break Control ar FIGHT
			execHit_BreakDecStart();

			LogPrintln(" LG] GameS Fight Dist = " + String(restPowerDist) + " /M");   // LOG-SERIAL1
			break;

  #if (NEW_IF)
		// $07 - Rest Distance
		case 64:
			anaRestDistance(msg);
			break;					
  #endif

		case 81 :		// HoldOn
			//exeFight = 0;			// Fight UpDown Control STOP
			execHit_FightBldcStop();	// CLR FLAG & STEP
			
			hitServoInterval = 100;		// 100ms

			gameStatus = GAME_HOLDON;

		  // TBD, min < > max
		#if (NEW_IF)
		  holdDir = msg.substring(5,(5+2)).toInt();
		  startEnd = msg.substring(7,(7+2)).toInt();
		#else
		  startEnd = msg.substring(9,(9+2)).toInt();
		#endif
		  if(1 == startEnd)	// Hold On Start
		  {
		  
			setVal_Hit_MainPwr(msg);
  #if IO_SERVO
			//Angle -30 ~ +30 
			sMotor.maxAdjAngle = 30;
  #endif

			// Stop Resist Control
			execHit_ResistTorqStop();
			// HOLDON START
			execHit_HoldOn_TorqStart();

			gameStatus = GAME_HOLDON;

			exeHoldon = 1;		// 버티기 시작
			
			LogPrintln(" LG] ExHod FishKind: " + String(fishTypeFix));   // LOG-SERIAL1
			LogPrintln(" LG] ExHod FIX: " + String(torqResistFix) + ", tRst: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
		  }
		  else if(2 == startEnd)	// Hold On END
		  {
			
  #if IO_SERVO
			//Angle -90 ~ +90 
			sMotor.maxAdjAngle = 90;
  #endif

			// TBD, Motor Control [ Return VAL] ???

			exeHoldOff = 1;	  // 버티기 종료
			//execHit_HoldOn_TorqStop();	// HoldOff Flag로 Loop종료 처리됨!

			gameStatus = GAME_FIGHT;
			
			LogPrintln(" LG] ExHod OFF");   // LOG-SERIAL1
		  }
		  else	// Unknow Oprtion
		  {
		  	LogPrintln(" LG] WRONG exe Para");
		  }
		  break;

  #if (NEW_IF)
		// Fast HOLD
		case 82 :		// Fast Hold
			anaFastHold(msg);
			LogPrintln(" LG] GameS FAST_HOLD" + msg);   // LOG-SERIAL1			break;
			break;
		// Button Combo
		case 83 :		// FastCOMBO BTN
			anaFastComboBtn(msg);
			LogPrintln(" LG] GameS COMBO_BTN" + msg);   // LOG-SERIAL1			break;
			break;
  #endif


  #if (NEW_IF)
		// Randing
		case 88 :		// Randing
  #else
		case 82 :		// Randing(Finsh)
  #endif
 			anaRanding(msg);
 			break;

		// Success
		case 91 :		// Success
			//torqDefault();
			////Angle -90 ~ +90 
			//sMotor.maxAdjAngle = 90;

			// Fight Control STOP
			execHit_FightBldcStop();

			motor_AllDefault();

			exeSuccessFail = 1;
			gameStatus = GAME_SUCCESS;
			execHit_BreakDecStop();

			LogPrintln(" LG] GameS SUCCESS");   // LOG-SERIAL1
      		break;
		
		// Continue
		case 92:		// Continue

			//exeFight = 0;			// Fight UpDown Control STOP
			execHit_FightBldcStop();	// CLR FLAG & STEP

			//torqMotor.setValue(torqMotor.defaultVal);	// Default Torq
			//torqDefault();
			motor_AllDefault();

	  	  #if CMD_CONTROL_2407E
			continueTorq = currTorq;			// FW자체 제어 토크
	  	  #else
			continueTorq = reqTorqueMotor;		// AP에서 요구받은 토크
	  	  #endif
			//
			exeContinue = 1;
		  
			gameStatus = GAME_CONTINUE;

		  	LogPrintln(" LG] GameS Contin, currTrq: " + String(continueTorq)); // LOG-SERIAL1
			break;

		// PointUP
		case 93:
			// TBD Action TBD
			execPointUp();
			gameStatus = GAME_POINTUP;
		  	LogPrintln(" LG] GameS PointUp "); // LOG-SERIAL1
			break;
			
		//Game Over
		case 98 :		 // Game Over  (99 => 98)

			//exeFight = 0;			// Fight UpDown Control STOP
			execHit_FightBldcStop();

			gameStatus = GAME_OVER;
			execGameOver();

			// TBD
			gameStatus = GAME_WAIT;
			isGameEnable = false;				// MAIN DIS ( Not PayMent)

			LogPrintln(" LG] GameS GAME_OVER");   // LOG-SERIAL1
      		break;

#endif		// AP_MOT_CONT : END

		default:
			break;
	}


	if(oldgameStatus != gameStatus)
	{
		// LOG
		//LogPrintln(" AP] GameS Change: " + msg + ",Kind: " + String(msgKind) + ", opt:" + String(msgOption) + ", STS: " + String(gameStatus) + ", old: " + String(oldgameStatus));	// LOG-SERIAL1
		LogPrintln(" AP] GameS chg: " + msg);

		oldgameStatus = gameStatus;
	}

}


//-------------------------------------------------
//
//	Ana INFO REQ & RESP INFO (ex IF VER)
//
//	$01kk%
//-------------------------------------------------
void anaDeviceInfoReq(String msg)
{

#if (NEW_IF)

	String respMsg = "";

	String strKind = msg.substring(3,(3+2));
	//String strPara = msg.substring(5,(5+2));

	int iKind = strKind.toInt();
	
	switch(iKind)
	{
		// IF VER
		case NUM_MAIN_IF_VER:
			respMsg = STX_INFO_REQ + MAIN_IF_VER + IfVer;
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq IF_Ver: " + IfVer);
			break;

		//BOARD TYPE
		case NUM_ROD_BOARD_TYPE:
			respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;
			respMsg += strRodBoardType;		// Old/New BOARD
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq ROD Board Type: " + strRodBoardType);
			break;

		// ROD Regist Wait T/O SEC (Deafult:25SEC)
		case NUM_ROD_REGIST_TO_SEC:
			respMsg = STX_INFO_REQ + ROD_REGIST_TO_SEC;
			respMsg += String(ROD_REGI_CANCEL_TO/1000);	
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq ROD Regist Wait T/O Sec: " + respMsg);
			break;

		// IMU 간격
		case NUM_IMU_INTVAL_TIME:		// IMU INTERVAL[ms]
			//if(AP_IS_TM == apType)
			{
				respMsg = STX_INFO_REQ + IMU_INTVAL_TIME + strImuInterval + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ImuInterval[ms]: " + respMsg);
			}
			break;

		// MANUFATORY INFO
		case NUM_MANUF_INFO_READ:		// 4
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(MANUF_INFO);
				respMsg = STX_INFO_REQ + MANUF_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Read[model,SN,Date]: " + respMsg);
			}
			break;

		case NUM_MANUF_INFO_SAVE:		// 5
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(MANUF_INFO, msg.substring(5));
				respMsg = STX_INFO_REQ + MANUF_INFO_SAVE + String((0 < result)? 1:0) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Save[model,SN,Date]:" + respMsg + ", len:" + String(result));
			}
			break;

		case NUM_MANUF_INFO_DEL:		// 6
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.deleteInfo(MANUF_INFO);
				respMsg = STX_INFO_REQ + MANUF_INFO_DEL + String(result) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Del[model,SN,Date]: " + respMsg);
			}
			break;

		// FIELD INSTALL  INFO
		case NUM_INSTALL_INFO_READ:		// 7
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(INSTALL_INFO);
				respMsg = STX_INFO_REQ + INSTALL_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Read[Date]: " + respMsg);
			}
			break;

		case NUM_INSTALL_INFO_SAVE:		// 8
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(INSTALL_INFO, msg.substring(5));
				respMsg = STX_INFO_REQ + INSTALL_INFO_SAVE + String((0 < result)? 1:0) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Save[Date]:" + respMsg + ", len:" + String(result));
			}
			break;

		case NUM_INSTALL_INFO_DEL:		// 9
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.deleteInfo(INSTALL_INFO);
				respMsg = STX_INFO_REQ + INSTALL_INFO_DEL + String(result) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Del[Date]: " + respMsg);
			}
			break;

		// FIELD INSTALL  INFO
		case NUM_MAINMOT_INFO_READ:		// 10
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(MAINMOT_INFO);
				respMsg = STX_INFO_REQ + MAINMOT_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				#if (LOG_MMOT_FS_RW)
					LogPrintln(" LG] InfRq MainMot Default Duty Info Read[Date]:" + respMsg);
				#endif
			}
			break;

		case NUM_MAINMOT_INFO_SAVE:		// 11
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(MAINMOT_INFO, msg.substring(5));
				// 파일저장 성공시, 값 갱신
				if(0 < result)	// write길이
				{
					defaultTorqueMotor = msg.substring(5).toInt();
				}
				
				respMsg = STX_INFO_REQ + MAINMOT_INFO_SAVE + ((0 < result)? String(defaultTorqueMotor): String(-1)) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				#if (LOG_MMOT_FS_RW)
					LogPrintln(" LG] InfRq MainMot Default Duty Write[Date]:" + respMsg + ", len:" + String(result));
				#endif
			}
			break;

		case NUM_MAIN_BOARD_TYPE:		// 12
			// Main보드타입 송신(무조건)
			respMsg = STX_INFO_REQ + MAIN_BOARD_TYPE + strMainBoard[mainBoardType]+ "%";
			Resp2ApPrintln(respMsg);
			
			#if (LOG_MAIN_BOARD_TYPE)
				LogPrintln("lg] MBDTY " + respMsg + ",data: " + String(mainBoardTypedData));
			#endif
			break;


		//-- BLDC LIMIT READ
		case NUM_BLDC_MOT_LIMIT_READ:		// 13
			if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(BLDC_LIMIT_INFO);
				//bldcLimitVal = respMsg.toInt();
				respMsg = STX_INFO_REQ + BLDC_MOT_LIMIT_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				#if (LOG_BLDC_FS_RW)
					LogPrintln(" LG] InfRq BLDC Mot LIMIT Read[Date]:" + respMsg);
				#endif
			}
			break;

		case NUM_BLDC_MOT_LIMIT_WRITE:		// 14,  $0114xxx%
			if(AP_IS_TM == apType)
			{
				short result;
				short val = msg.substring(5).toInt();
				//범외 밖 = NG, 저장안함
				if( (BLDC_LIMIT_MIN > val) || (BLDC_LIMIT_MAX < val) )
				//if( (BLDC_LIMIT_MIN > val) || (BLDC_LIMIT_MAX+1 < val) )	// TEST
				{
					result = ERR_PARA_NG;	// RANGE OVER
				}
				// 범위 내 = OK, 저장
				else
				{
				
					result= fsInfo.saveInfo(BLDC_LIMIT_INFO, String(val));
					// 파일저장 성공시, 값 갱신
					if(0 < result)	// write길이
					{
						bldcLimitVal = val;
					}
				}
				respMsg = STX_INFO_REQ + BLDC_MOT_LIMIT_WRITE + ((0 < result)? String(bldcLimitVal): String(result)) + "%";	//100 ~ 255
				Resp2ApPrintln(respMsg);

				#if (LOG_BLDC_FS_RW)
					LogPrintln(" LG] InfRq BLDC Mot LIMIT WRITE[Date]:" + respMsg + ",len:" + String(result));
				#endif
			}
			break;


		//--- CMD_PARA NG			
		default:
			Resp2ApPrintln(STX_INFO_REQ + strKind + "1%");	// NG Response
			break;
	}

#endif

}


/*----------------------------
	($01)GAME_ENB, GAME_WAIT
----------------------------*/
void anaGameEnbOver(String msg)
{
		 // test LOG
	 
	   // 3-1) Game STBY - Title Display( Creadit Waiting)
	   if (msg.endsWith(STATE_WAITING))		// Game Waiting ( 1st Title or Game Over )
	   {
		   isGameEnable = false;			   // MAIN DIS
		   //isGameStby = true;			   // GAME STBY
		   //motor_AllDefault();		// TBD HUD Follow
		   if(DIAG_MODE != mainMode)
		   {
			 #if(!IO_LM_MOT)
		   		digitalWrite(BD_LED3_PIN, LOW);    // LED3 OFF
		     #endif
		   }
			gameStatus = GAME_WAIT;
			execGameOver();
			LogPrintln(" AP] GamWT " + msg + ", o/c " + String(oldgameStatus) + "," + String(gameStatus));	  // GAME WAIT
			if(oldgameStatus != gameStatus)
			{
			   oldgameStatus = gameStatus;
	   		}

	   }
	   // 3-2) Game ENB - Creadit OK
	   else if (msg.endsWith(STATE_ENABLE))	// GAME ENB
	   {
		 //LogPrintln(" AP] GamEN " + msg);	   // LOG-SERIAL1
		 LogPrintln(" AP] GamEN " + msg + ", o/c:" + String(oldgameStatus) + "," + String(gameStatus)); 	 // GAME WAIT
		 
		 isGameEnable = true;				 // GAME ENB
		 //isGameStby = false;			 // GAME STBY
		   //motor_AllDefault();		// TBD HUD Follow
		 //torqMotor.setValue(65);	// TBD
		 torqDefault();
	   }
	   else
	   {
	   		// Error
	   }

}

#define AP_ALIVE_NOTRECV_TO	10		// 10 SEC
//---------------------------------------------------------------
// AP Alive Recved Check = [ AP-MAIN] Connection Check
//	DF프로그램 - FW간 통신 끊김 검지
//--------------------------------------------------------------
//	CALL 1 SEC
//--------------------------------------------------------------
void apAlive_Check()
{
	if( CONNECT == ap_conn_status)			// CONNECTED Only
	{
		if( AP_ALIVE_NOTRECV_TO > apAliveConn_NotRecvCnt)	// 10 SEC
		{
			apAliveConn_NotRecvCnt++;
			if (gRodSleepMode == SS_EXIT) {
				if (gmWatStatus == 2) {
					sendSleepEnable(SS_WAIT, TIME_SLEEP_LIGHT, 0);
				}
				else {
					sendSleepEnable(SS_NONE, 0, 0);
				}
			}
		}
		else
		{
			// 통신장애로  Alive Check명령이 오지 않으면
			// 모터 OFF, IMU출력 정지로 처리함.
			ap_conn_status = DISCONNECT;	// NOT CONNECTED, exec Once Only
			sendSleepEnable(SS_EXIT, TIME_SLEEP_DEEP, 1);
			motor_AllOff();

			reelOut_AllOff();
		  #if 0
			// REEL OUT OFF
			breakMotorOff();
			delay(20);
			imuDataOutCmdSend(IMU_DATA_OFF);
			delay(20);
			//reellMotorOff();
			sendRodVrtCmd(0,0,0,0);		// Vrt Mot OFF
			delay(20);
			//reelBtnLedOff();
			sendRodBtnLedCmd(0,0,0,0,0);
		#endif


		// 불필요 에러코드 통지 삭제, 릴등록시 통신끊김으로 통지됨.
		  	// AP에 장애출력(무의미)
		// 	Resp2ApPrintln("$159999%");		// AP 폴링 안함!!!
		//	LogPrintln(" LG] APCON AP Alive NOT RECV 10 sec");

			#if (NOT_USE_CODE)
				// 대기중에 텐션유지
				stbyTorqOffControl_Start();
			#endif
			
		}
	}

	// UNKNOWN (POWER ON)
	else if( UNKNOWN == ap_conn_status)
	{
	
		#if (NOT_USE_CODE)
			if( AP_ALIVE_NOTRECV_TO > apAliveElse_NotRecvCnt)	// 15 SEC
			{
				apAliveElse_NotRecvCnt++;
			}
			else
			{
				ap_conn_status = DISCONNECT;	// NOT CONNECTED, exec Once Only
				motor_AllOff();
				reelOut_AllOff();
			
				// 대기중에 텐션유지
				stbyTorqOffControl_Start();
			}
		#endif
	
	}
	else
	{
		// NA
	}
}

//
//
//	Call 1SEC
void send2RodAlive_CntCheck()
{
	int	CheckTime;

	if (rodAlive_SendCnt == 0) {
		if (gRodSleepMode == SS_NONE) {
			gRodSleepTime = 0;
		}
	}
	if (gRodSleepStat) {
		CheckTime = gRodSleepTime + 4;
	}
	else {
		// 1)무조건 Send , for Auto Recovery
//		eNow.write(STR_PID_ROD_ALIVE_CHK, "");
		eNow.write(STR_PID_ROD_ALIVE_CHK, String(gRodSleepMode));
		CheckTime = ROD_RESP_OVER_CNT;
	}

	// 2) Send CNT OVER Check
	if(CheckTime > rodAlive_SendCnt)	// Check Time OVER?
	{
		// NOT Over, Count UP
		rodAlive_SendCnt++;
	}
	else
	{
		// SET SLAVE DIS_CONN
		rod_conn_status = (ROD_DISCONN);
	}
	#if LOG_ROD_ALV_CHK
		LogPrintln(" LG] RD_AL Send Cnt: " + String(rodAlive_SendCnt));
	#endif
}

//---------------------------------------------
// 낚시대(REEL) 교체 CONTROL
//---------------------------------------------

unsigned int rodRegistToStep = 0;	// TO control Step

//TO정지
void rodRegistToStop()
{
	rodRegistToStep = 0;	// IDLE SET
  #if (LOG_ROD_REGI_TO_STEP)
	LogPrintln(" LG] rosRG TO Stop");
  #endif
}

// TO 확인 개시
void rodRegistToStart()
{
	rodRegistToStep = 10;	// 10SEC T/O Start	
#if (LOG_ROD_REGI_TO_STEP)
	LogPrintln(" LG] rosRG TO 20 Sec START");
#endif
}

void rodRegi3secToStart()
{
	rodRegistToStep = 30;	// 3SEC T/O  Start
#if (LOG_ROD_REGI_TO_STEP)
	LogPrintln(" LG] rosRG TO 3 Sec START");
#endif
}

//
// CALL 100ms ,  (20초 경과) / (3초 경과) Check Control
//
void rodRegistToControl()
{
	static unsigned int oldStep = 0;
	static unsigned long _to = 0;

	switch(rodRegistToStep)
	{
		case 0:
			break;
			
		case 10:	//Start
			setTO(_to);
			rodRegistToStep = 11;
			break;		
		case 11:
			if(checkTO(_to, ROD_REGI_CANCEL_TO))
			{
				// Rod Regist Cancel
				rodRegistCancel();		// MODE CLR
				rodRegistToStep = 0;		// to IDLE
				StsSendPrintln(STX_ROD_REGIST + ROD_REGI_FW_CANCEL); // AP에 송부(FW_등록취소: 20초 T/O)
				#if (LOG_ROD_REGI_TO_STEP)
					LogPrintln(" LG] rodReg 20 SEC T/O");
				#endif
			}
			break;

		case 30:	//Start
			rodRegistToStep = 31;
			setTO(_to);
			break;		
		case 31:
			if(checkTO(_to, ROD_REGI_END_TO_1ST))	// 1.5 SEC
			{
				StsSendPrintln(STX_ROD_REGIST+ ROD_REGI_FINISH); // AP에 송부(FW_등록완료: 3초 T/O)
				rodRegistToStep = 32;		// to IDLE
				#if (LOG_ROD_REGI_TO_STEP)
					LogPrintln(" LG] rodReg AP SEND FINISH");
				#endif
				setTO(_to);
			}
			break;
		case 32:
			if(checkTO(_to, ROD_REGI_END_TO_2ND))	// 3초
			{
				rodRegistToStep = 0;		// to IDLE
				rodRegistCancel();
				//ESP.restart();				// ESP Restart, rst:0xc (RTC_SW_CPU_RST),boot:0x28 (SPI_FAST_FLASH_BOOT)
				//esp_restart();			// Soft Reset, rst:0xc (RTC_SW_CPU_RST),boot:0x28 (SPI_FAST_FLASH_BOOT)
				esp_soft_reset();
				//esp_hard_reset();		// NOT IMP
			}
			break;
			
		default:
			break;
	}
}

// 등록 취소, Mode CLR
void rodRegistCancel()
{
	rodRegistMode = 0;
}

// 낚시대 등록 명령 분석 처리(진입, 취소)
void anaRodRegist(String msg)
{
	String respMsg;
	int Act = msg.substring(3).toInt();
	switch(Act)
	{
		case ROD_REGI_AP_ENTRY:		// 진입
			rodRegistMode = 1;
			LogPrintln(" LG] RodRg ApMsg:" + String(Act));
			// 진입처리
			// 1) 10초 Time Set
			rodRegistToStart();
			break;
		case ROD_REGI_AP_CANCEL:		// AP취소, FW자체 취소도 있음
			if(1 == rodRegistMode)		// 등록개시(2)이후는 Cancel 무시
			{
				rodRegistToStop();		// TO 취소
				rodRegistCancel();		// Mode CLR
				LogPrintln(" LG] RodRg ApMsg:" + String(Act));
				// 취소 처리
			}
			break;
		default:
			break;
	}
		
}

// Rod Regist EXEC
void rodRegistExec(String msg)
{
	  rodRegistMode = 2;	  //등록 개시 SET
	  //1) 등록 취소 T/O CANCEL
	  rodRegistToStop();
	  // 2) AP에 교체개시 송부
	  StsSendPrintln(STX_ROD_REGIST+ ROD_REGI_WRITE_START);   // AP에 송부(등록개시)
  #if (LOG_BC_ROD_ADDR_WRITE_STEP)
		  LogPrintln(" LG] RodRg Regist Mode:" + String(rodRegistMode));
  #endif
	
	  // 3)=== PEER 바꾸기
	  rodRegistMode = 3;	  //Peer변경
  #if (LOG_BC_ROD_ADDR_WRITE_STEP)
		  LogPrintln(" LG] RodRg Regist Mode:" + String(rodRegistMode));
  #endif
	  // 기존 PEER삭제
	  int peerRet;
	  peerRet = esp_now_del_peer(slave_board_addr);
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR DEL PEER:" + String(peerRet));
	  }
	  // 수신주소 PEER추가, TBD  : 수신한 STR_ADDR을 HEX로 변환하여 PEER추가는 추후
	  peerRet = eNow.addPeerAddress(rcv_src_addr_back);   // HEX
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR Add PEER: newROD" + String(peerRet));
	  }
	  // 4)=== ROD에 통지 ( Main주소 쓰기)
	  rodRegistMode = 4;
  #if (LOG_BC_ROD_ADDR_WRITE_STEP)
		  LogPrintln(" LG] RodRg Regist Mode:" + String(rodRegistMode));
  #endif
	  // BC PEER추가
	  peerRet = eNow.addPeerAddress(broad_cast_addr); // HEX
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR Add PEER: BC" + String(peerRet));
	  }
	  // == ROD에 주소 송부
	  String myAddr = eNow.getMyAddress();
	  eNow.writeBC(STR_PID_MAIN_ADDR_WRITE, myAddr);	  // 보내기전에 BC설정 필요?
  #if (LOG_BC_MAIN_ADDR_SEND)
		  LogPrintln(" LG] Write Send:" + myAddr);
  #endif
	  // ** 대기 (ROD Write완료 수신) - NA
	  
	  // **BC PEER 삭제
#if 1
	  peerRet = esp_now_del_peer(broad_cast_addr);
	  if (ESP_OK != peerRet)
	  {
		LogPrintln(" LG] ERROR nowPeer DEL: " + String(peerRet));
	  }
	  esp_now_peer_num_t rodPeer;
	  esp_now_get_peer_num(&rodPeer);
	  LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
#endif
	
	  // 5) 타켓주소 쓰기
	  rodRegistMode = 5;
  #if (LOG_BC_ROD_TART_ADDR_WRITE_TIME)
		  LogPrintln(" LG] RodRg Regist Mode:" + String(rodRegistMode));
		  chk_ms_tick = millis();
  #endif
	
	  //if((MAC_ADDR_OK_STR_LEN - 1) < now_cmd_data.length())
	  {
		  //bool errRet = eNow.setTargetAddress(now_cmd_data);	  // Write STRING, 처리시간 6ms
		  bool errRet = eNow.setTargetAddress(msg);	  // Write STRING, 처리시간 6ms
		  if(false == errRet)
		  {
			  LogPrintln(" LG] rodRg ERROR TRGT ADDR WRITE");
		  }
	  #if (LOG_BC_ROD_TART_ADDR_WRITE_TIME)
			  LogPrintln(" LG] RodRg TRGT Write Time:" + String(millis() - chk_ms_tick));
	  #endif
	  }
	  
	  String wrtAddr = eNow.getTargetAddress();
	  LogPrintln(" LG] rodRg Addr:" + wrtAddr);
	  
	  // 6) T/O처리 대기
	  rodRegistMode = 6;	  // 3초 대기 시작
  #if (LOG_BC_ROD_ADDR_WRITE_STEP)
		  LogPrintln(" LG] RodRg Regist MODE Write:" + String(rodRegistMode));
  #endif
	  rodRegi3secToStart(); 	  // 3초 /3초 대기 Start (1st3sec:FINISH send to AP, 2nd 3sec: restart)

}


//---------------------------------------------------------------------
// 보드 & System Elec Input
void tmInput_BoardElecMonitorExec(short _exec)
{
	if(AP_IS_TM == apType)
	{
		//tmInputAllcheck = _exec;
		diagInFlag_BootSwc = _exec;		// In 01-02
		diagInFlag_PonSwc = _exec;		// In 05-01
		diagInFlag_PcUsb5V = _exec;		// In 05-02
	}
}

void tmInput_BbnMotorMonitorExec(short _exec)
{
	if(AP_IS_TM == apType)
	{
		//tmInputAllcheck = _exec;
		diagInFlag_BobbinEncA = _exec;		// In 03-01
		diagInFlag_BobbinEncB = _exec;		// In 03-02
		diagInFlag_BbnBldcFG = _exec;			// In 03-05
	}
}


// TM DIAG INPUT TEST
void anaTmInputTestReq(String msg)
{
	// TM에서는 Cat 01-xx/Cat 05-xx, Cat 03-xx  의 All Mon ON/ OFF만 사용
	diagInputTest(msg);
}

// TM DIAG OUTPUT TEST
void anaTmOutputTestReq(String msg)
{
	// TM에서는 Cat 05-01 (Sub AC On/Off)만 사용
	diagOutputTest(msg);
}

//----------------------------------------------
//	AP타입 ( DF or TM ) 수신 처리
// $29kkxx%
//----------------------------------------------
void anaApInfo(String msg)
{

	short kind = msg.substring(3,(3+2)).toInt();
	short act = msg.substring(5,(5+2)).toInt();
	
	//String respMsg;
	//String logMsg;
	String send2rodMsg = msg.substring(3,(3+4));


	switch(kind)
	{
		// kk = 1	// AP Type
		case DEF_AP_TYPE:		// $2901 : RECV AP_TYPE(DF or TM)
			apType = act;	//AP가 통지하지 않으면, 기본값 AP=DF(게임프로그램)
			eNow.write(STR_PID_AP_INFO_SEND, send2rodMsg);

			// TM타입 수신시, 모든 IO자동 출력 : 보드 & Elec Input Monitor 시작
			if(AP_IS_TM == apType)	// TYPE is TM
			{
				//( 삭제 ) TM타입 수신시, 모든 IO 입력 자동 출력 : 보드 & Elec Input Monitor 시작
				#if (0)
					diagInput_BoardInAllOn();	// Cat 01-xx
					DiagInput_ElecInAllOn();	// Cat 05 -xx
					DiagInput_MotInAllOn();		// Cat 03-xx FG Only
					DiagInput_MainEcoderInAllOn();	// Cat 03-xx Concoder
				#endif
			}
			//
		  #if(LOG_AP_TYPE)
			LogPrintln(" lg] apTYP:" + String(act) + (act? ",TM" : ",AP"));
		  #endif
			break;

		default:
			break;
	}
	
}

/*----------------------------
	MAIN ALIVE(AP<->MAIN) 응답 처리
----------------------------*/
void anaMainAlive(String msg)
{

	//== 1) RESP MAIN CONN
	//str = STX_MAINALIVE + "11111" + battLvlStr + "%"; // CONN (1~99 %)
	String respMsg;
#if (NEW_IF)
	respMsg = STX_MAINALIVE; // CONN
#else
	respMsg = STX_MAINALIVE + STATE_CONNECTED; // CONN (1~99 %)
#endif
	respMsg += "%";
	Resp2ApPrintln(respMsg);		  // Resp to PC
	  		
	#if LOG_MAIN_CONN
		LogPrintln(" FW] Maliv " + msg);		// LOG-SERIAL1
	#endif

	// 2) OTHER STATE(ROD/BAT/IMU) SEND CHECK
  	if( (2 > recvMainConn_Pwr1stCnt) &&  (PROG_START == gameStatus ))	// 2 Times SEND
  	{
		recvMainConn_Pwr1stCnt++;
 		sendCurrentStatus();
 	}

	//3) AP-Main 연결 SET
	apAliveElse_NotRecvCnt = 0;
	apAliveConn_NotRecvCnt = 0;		// Not recv CNT CLR
	ap_conn_status = CONNECT;	// AP CONNECT
	
}
//------------------------------------------------------------------------------------



//---------------------------------------------
// Device Ready Check CALL 1ms
//
void devChkReady_Control()
{
	if(0)	{}	// dummy
#if 1			// (V108)	
	else if(devCheckAllFlag)		{ devchk_bbnAll_Control(); }
#else			// 이하 생략함
	else if(devCheckBbnMotFlag)		{ devchk_bbnMot_Control(); }
	else if(devCheckBbnEncFlag)		{ devchk_bbnEnc_Control(); }

	else if(devCheckBldc24VFlag)	{ devchk_bldc24v_Control(); }
	else if(devCheckTorqFlag)		{ devchk_torq_Control(); }
	//else if(devCheckWireBroken_Flag)	{ devchk_WireBroken_Control(); }	// 줄끊어짐 검지 추가 25/7/15
#endif
	else if(devCheckLmMotFlag)		{ devchk_lmMot_Control(); }
	// CHECK LAST
	else
	{
		// LM은 
	  #if (IO_LM_MOT)
		lmHome_Control_Start(1, reqTorqueMotor);		// LM 홈동작
	  #endif
		devchk_OkEnd();			// 모터 OFF, 보빈모터 초기값 출력(줄 텐션)
	}
	// LM MOT Check - Home Control
	// TBD-LM

}


void resp_MainBoard_Ver()
{
	String respMsg;
	// Main보드타입 송신(무조건)
	respMsg = STX_INFO_REQ + MAIN_BOARD_TYPE + strMainBoard[mainBoardType]+ "%";
	Resp2ApPrintln(respMsg);
}


//--------------------------------------------
// 장치 에러 검사 결과 : OK종료
void devchk_OkEnd()
{
	int outTorq = 0;

	// 1) Device CHK 종료 & 결과 응답
	//=== Last OK Msg Send
	devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
	devChkMsgSend(MAIN_DEV_CHK_OK);

	//메인보드 버전 송부
	resp_MainBoard_Ver();
	
	
	// 3) 토크 모터 OFF
	//  3-1) AP=TM이면 토크모터 OFF
	if(AP_IS_TM == apType)		// TM이면 토크모터 OFF
	{
		torqOff();
		outTorq = 0;
	}
	//  3-2) AP=게임이면 토크모터 최소값 출력(낚시줄 텐션 유지 목적)
	else
	{
/* (V108) 장치 체크 종료시 토크 모터 동작 생략
		if(recv_TorqMotor_Flag)
		{
			torqMotor.setValue(reqTorqueMotor); 	//DEFAULT( 70 ) OUT = BELT SPRING TENSION
			outTorq = reqTorqueMotor;
		}
		else
		{
			torqMotor.setValue(defaultTorqueMotor); 	//DEFAULT( 70 ) OUT = BELT SPRING TENSION
			outTorq = defaultTorqueMotor;
		}
*/					
	}
	// 토크모터 FW자체 출력 , AP에 응답.
	send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);

	#if (FUNC_STBY_OFF_CONT)
		//== 장치장애검지 OK(정상)종료시 [STBY토크OFF제어]요구 Check필요
		stbyTorqOff_ReqCheck();
	#endif

}

// FW SELF CMD 응답 : CMD + VAL
void send2resp_FwSelfControlCmd(String Cmd, int torq)
{
	Resp2ApPrintln(Cmd + stringTo3char(torq) + "%");
}

String stringTo3char(int val)
{
	String strRts;

	if(  0 > val)	val = 0;
	if(999 < val)	val = 999;

	if(0)				{} // Dummy
	else if(100 < val)	{strRts = String(val); }		// 3자리
	else if( 10 < val)	{strRts = "0" + String(val); }	// 0 + 2자리
	else				{strRts = "00" + String(val); }	// 00 + 1자리

	return strRts;
}

//
void devChkMsgSend(String msg)
{

	// 1st READY(0000) RESP
	msg = STX_DEV_CONT_REQ + msg;	// Ready 00-00 (xx-yy)
	//msg = STX_DEV_CONT_REQ + "6010";	// Ready 00-00 (xx-yy)
	msg += "%";
	Resp2ApPrintln(msg);
}

//-------------------------------------------
// Device Ready Check START form AP CMD
//
#if 1		// (V108) 장치 통합 체크 함수로 대체함
void devchk_bbnAllStart()
{
	devCheckAllFlag = 1;
	devBbnAllChkStep = 10;
}
#else		// (V108) 아래 함수 생략하고 위의 통합 함수로 대체함
void devchk_bbnMotStart()
{
	devCheckBbnMotFlag = 1;
	devBbnMotChkStep = 10;
}

void devchk_bbnEncStart()
{
	devCheckBbnEncFlag = 1;
	devBbnEncChkStep = 10;
}

void devchk_Bldc24vStart()
{
	devCheckBldc24VFlag = 1;	
	devBldc24VChkStep = 10;
}

void devchk_TorqStart()
{
	devCheckTorqFlag = 1;	
	devTorqChkStep = 10;
}
#endif
void devchk_lmMotStart()
{
	#if (IO_LM_MOT)
		// TBD-LM
		devCheckLmMotFlag = 1;
		devLmMotChkStep = 10;
	#endif
}

//--- All Flag & Step Clear
void devCheckFlagStep_AllClear()
{
#if 1		// (V108) 장치 통합 체크로 대체함
	devCheckAllFlag = 0;
	devBbnAllChkStep = 0;
#else		// (V108) 아래 제거
	// 1) FLAG클리어
	devCheckBbnMotFlag = 0;		// BLDC Motor Flag CLR
	devCheckBbnEncFlag = 0;	// EncA
	devCheckTorqFlag = 0;		// Torq Motor
	devCheckLmMotFlag = 0;
	devCheckBldc24VFlag = 0;
	//devCheckWireBroken_Flag = 0;

	// STEP 클리어
	devBbnMotChkStep = 0; 		// CLR STEP	
	devBbnEncChkStep = 0;
	devTorqChkStep = 0;
	devLmMotChkStep = 0;
	devBldc24VChkStep = 0;
	//devCheckWireBroken_Step = 0;
#endif	
}

#define DEVCHK_BBN_DIR_CHANGE_TO	50 // 100	// 300ms(OK후 CCW오버런)
#define DEVCHK_BBN_RETRY_OFF_TO		50	// 300ms(OK후 CCW오버런)

#define DEVCHK_ENC_SEN_TO	300 // 500 // 300		// 200

#define DEVCHK_TORQ_MOT_ON_TO	300 // 500	//800
#define DEVCHK_TORQ_MOT_STEP_TO	100	//800
#define DEVCHK_TORQ_START_DELAY_TO	150

#define DEVCHK_TORQ_MOT_OFF_TO	50 // 100	//800

#define DEVCHK_LM_MOT_TO	5000  //

//=== 모터확인시 토크 출력값 ( 너무세면 낚시대가 TV를 때림)
#define DEVCHK_BBN_DEFAULT_DUTY	30
#define DEVCHK_BBN_STEP_DUTY	10		//25	// 20

#define DEVCHK_BBN_WIRE_REL_DUTY	50		// (V108)
#define DEVCHK_BBN_WIRE_REL_ON_TO	300		// (V108)

#define DEVCHK_BLDC24V_REL_ON_TO	150 // 500	//800	// TBD, 시간확인 필요(1회전)

#define DEVCHK_BLDC24V_BREAK_TO		50		// Beak 50ms

#define DEVCHK_TORQ_TO_ENCA_1ST_ON_CHK_TO	600
#define DEVCHK_TORQ_TO_ENCA_1ST_ON_SPEC_TO	250 // 300

#define DEVCHK_BLDC24V_RETRY_WAIT_TO	500 // 300 // 100

#define DEVCHK_BLDC24V_OFF_WAIT_TO		1500	// 1.5 SEC

#define DEVCHK_BLDC24V_SEN_CNT_TO	1200 // 300 // 100

#define DEVCHK_TORQ_STOP_WAIT_TO	500
#define	DEVCHK_TORQ_MOT_CHK_CNT		(9*2)		// To

#define DEVCHK_LM_DUTY	35		//
//#define DEVCHK_LM_DUTY	50 // 35		//
//===



#define RETRY_CNT	3	// Retry 3회 (총 3회)
#define IN_SIG_CHG_CNT (3*2)	// Pulse 3회 => 신호변화 6회


typedef struct _stDevchkBbn
{
	// Duty, Time
	unsigned short duty;
	unsigned short ccw_ontime;	
	unsigned short cw_ontime;	
}stDevchkBbn;

#define DEVCHK_BBNMOT_SIZE	(1 + 3)
stDevchkBbn	stDevchkBbnMot[DEVCHK_BBNMOT_SIZE] =
{
	// duty, Ccw_on(풀림) , cw_on(감김) 시간
	{0,},	// Dummy
	// 1회
	{30, 300, 300},	// 
	// 2회
	{40, 250, 250},	//
	// 3회
	{50, 200, 200},	// 
};

// 보빈 모터 관련 전체 장애 검지(V108)
// 1) BLDC 모터 풀기 (CCW, Duty 50, 300ms)
//   - Hall 센서로 모터 동작 체크
//   - ENC A,B 센서 정상 체크
// 3) 토크 모터 감기
//   - ENC A 센서로 토크 모터 동작 체크
//
void devchk_bbnAll_Control()
{
	static unsigned long _to;
	static unsigned long _toSenChg;
	static unsigned long pulseWidthMs;
	
	static unsigned short olddevBbnAllChkStep = 0;
	static short errK = ERR_OK;

	static unsigned short _oldSen;
	unsigned short _newSen, _chgSen;

	static unsigned short _chgCnt  = 0;
	static unsigned short _chgCntA = 0;
	static unsigned short _chgCntB = 0;
	static unsigned short _chgCntF = 0;

	static int nDevChkErrCount = 0;
	static int nDevChkTorqDuty = 0;
	static int _nEncCount[4] = {0};
	static String msgErr;
	String msg;

	#if (LOG_BBN_MOT_STEP)
		// Step Change LOG OUT
		if(olddevBbnAllChkStep != devBbnAllChkStep )
		{
			LogPrintln(" lg] bbnSt DevChck BBN : " + String(devBbnAllChkStep));
			olddevBbnAllChkStep = devBbnAllChkStep;
		}
	#endif
	
	switch(devBbnAllChkStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			_oldSen = LVL01_BBNALL;
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			errK = ERR_OK;
			msgErr = MAIN_DEV_CHK_OK;
			setTO(_to);
			devBbnAllChkStep = 14;
			break;

		case 14:
			if(checkTO(_to, DEVCHK_TORQ_MOT_OFF_TO) )		// 50 ms
			{
				setTO(_to);
				devBbnAllChkStep = 20;
			}
			break;

		// CCW CHECK - Duty 50, 300ms 회전후 센서 체크
		case 20:
			_nEncCount[0] = iIsrbbnEncCnt;
			_nEncCount[1] = _nEncCount[2] = 0;
			bbnMotor.onBldc(BBN_MOT_CCW, DEVCHK_BBN_WIRE_REL_DUTY);
			_oldSen = LVL01_BBNALL;				// (V108)
			_chgCntA = 0;						// * Sensor A 신호변화 Cnt
			_chgCntB = 0;						// * Sensor B 신호변화 Cnt
			_chgCntF = 0;						// * Sensor F 신호변화 Cnt
			nDevChkTorqDuty = 0;
			setTO(_to);
			devBbnAllChkStep = 21;
			break;

		case 21:								// 300 ms 대기후 모터 off & 센서 체크
			if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )		// 300 ms
			{
				bbnMotor.offBldc();
				// CNT확인
				// 1) OK
				if ((IN_SIG_CHG_CNT < _chgCntF) && (IN_SIG_CHG_CNT < _chgCntA) && (IN_SIG_CHG_CNT < _chgCntB))		// (V108)
				{
					setTO(_to);
					devBbnAllChkStep = 29;	// OK, 100ms후 정지
				}
				// 2) NG
				else
				{
					if (IN_SIG_CHG_CNT >= _chgCntF)				// (V108)
						msgErr = MAIN_BLDC_MOT_ERR;				// *
					else if (IN_SIG_CHG_CNT >= _chgCntA)		// *
						msgErr = MAIN_BBNENCA_SEN_ERR;			// *
					else if (IN_SIG_CHG_CNT >= _chgCntB)		// *
						msgErr = MAIN_BBNENCB_SEN_ERR;			// *
					else
						msgErr = MAIN_BLDC_MOT_ERR;
					devBbnAllChkStep = 75;	// NG
					errK = ERR_CCW;
				}
				_nEncCount[1] = iIsrbbnEncCnt;

				#if (LOG_DEVCHK_BBN_MOT)
					LogPrintln(" lg] DEVBM FG SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif
			}
			else {
				_newSen = LVL01_BBNALL;						// (V108) 이하 수정 
				_chgSen = _oldSen ^ _newSen;				// *
				if(_chgSen)									// * BBN Sensor 신호 변화
				{
					_oldSen = _newSen;						// * 센서상태 저장
					if (_chgSen & BIT_SNS_BBNFG)			// *
						_chgCntF++;							// * F 신호변화 cnt + 1
					if (_chgSen & BIT_SNS_BBNENCA)			// *
						_chgCntA++;							// * A 신호변화 cnt + 1
					if (_chgSen & BIT_SNS_BBNENCB)			// *
						_chgCntB++;							// * B 신호변화 cnt + 1
				}
			}
			break;

		//1-2] OK, CW CHECK
		case 29:
			if(checkTO(_to, DEVCHK_BLDC24V_OFF_WAIT_TO))	// 1500ms(24V OFF WAIT) 대기후 토크모터 동작 및 FET 체크
			{
				devBbnAllChkStep = 30;	// CW 체크 Start
			}
			break;

		// 2] CW확인
		// 토크 모터 동작 & FET 장애 검지 시작
		case 30:
			_oldSen = LVL01_BBNENCA;
			_chgCnt = 0;			// 신호변화 Cnt
			_nEncCount[1] = iIsrbbnEncCnt;
//			nDevChkTorqDuty = GetDefValue4torqMotor() + 3 * ((nDevChkErrCount < 3) ? nDevChkErrCount : 3);
			nDevChkTorqDuty = defaultTorqueMotor + 3 * ((nDevChkErrCount <= 2) ? nDevChkErrCount : 2);
			if (MAIN_MOT_FS_MAX_DUTY < nDevChkTorqDuty)
				nDevChkTorqDuty = MAIN_MOT_FS_MAX_DUTY;
			torqMotor.setValue(nDevChkTorqDuty); 		// DEFAULT + 3 * Retry
			setTO(_to);
			devBbnAllChkStep = 34;
			break;

		case 34:
			if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || (DEVCHK_TORQ_MOT_CHK_CNT <= _chgCnt))		//WAIT 300 ms TO
			{
				torqOff();
				//if( (6*2) > _chgCnt)	// 펄스 미만 이면 NG
				//if( (15*2) > _chgCnt)	// 펄스 미만 이면 NG
				if( DEVCHK_TORQ_MOT_CHK_CNT > _chgCnt)	// 펄스 미만 이면 NG
				{
					nDevChkErrCount++;
					msgErr = MAIN_BOARD_BLDC_24V_ERR;
					errK = ERR_CW;
				}
				else
				{
					errK = ERR_OK;
				}
				_nEncCount[2] = iIsrbbnEncCnt;
				devBbnAllChkStep = 75;
				
				// LOG
				#if (LOG_BLDC24V_STEP)
					LogPrintln(" lg] DEVCK BLDC24V Pulse:" + String(pulseWidthMs) + "ms");
				#endif
			}
			else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				_oldSen = LVL01_BBNENCA;		// 센서상태 저장
				_chgCnt++;
				pulseWidthMs = getTO(_toSenChg);
				setTO(_toSenChg);
			}
			break;

		// 모터 정지, ERR/OK 분기
		case 75:
			bbnMotor.offBldc();
			// 검출결과, AP에 출력(확인용)
			Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + msgErr + (ERR_OK==errK? "OK":"NG") + ",Err:" + String(nDevChkErrCount)+",Cnt:" + String(_chgCnt)+"("+ String(_chgCntF)+","+ String(_chgCntA)+","+ String(_chgCntB)+"),ms:" + String(getTO(_to)) + ", Enc:" + String(_nEncCount[0]) + "," + String(_nEncCount[1]) + "," + String(_nEncCount[2])+ ",Duty:" + String(nDevChkTorqDuty));
			if(ERR_OK == errK)	// OK
			{
				devBbnAllChkStep = 90;		// 정상 종료
			}
			else	// ERROR CCW, CW
			{
				devBbnAllChkStep = 80;		// Error
			}
			break;


		// Error SET
		case 80:
			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devCheckFlagStep_AllClear();
			
			msg = STX_DEV_CONT_REQ + msgErr + "%";
			Resp2ApPrintln(msg);

			#if (LOG_DEVCHK_BBN_MOT)
				msg = ((ERR_CCW == errK)? "CCW":"CW_");
				LogPrintln(" LG] DevRq BLDC ERR 6010 " + msg);
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif
			
			break;
			
		//END_OK
		case 90:
			devCheckAllFlag = 0;	// CLR BLDC Flag
			devBbnAllChkStep = 0;		// Clr Step
			if (defaultTorqueMotor != nDevChkTorqDuty) {					// 정상 처리한 duty와 default와 다르면
				fsInfo.saveInfo(MAINMOT_INFO, String(nDevChkTorqDuty));		// 에러 체크 생략
				defaultTorqueMotor = nDevChkTorqDuty;						// default 토크모터 duty 설정
				nDevChkErrCount = 0;										// 에러 회수 초기화
			}
			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk BLDC MOT OK");
			#endif
			break;

		default:
			break;
	}
}

#define ENC_SEN_OK	0

#if 0
// BBN(BLDC)모터 장애 검지
void devchk_bbnMot_Control()
{
	static unsigned long _to;
	
	static unsigned short olddevBbnMotChkStep = 0;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;
	
	String msg;

	#if (LOG_BBN_MOT_STEP)
		// Step Change LOG OUT
		if(olddevBbnMotChkStep != devBbnMotChkStep )
		{
			LogPrintln(" lg] bbnSt DevChck BBN : " + String(devBbnMotChkStep));
			olddevBbnMotChkStep = devBbnMotChkStep;
		}
	#endif
	
	switch(devBbnMotChkStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			//_oldSen = LVL01_BBNFG;
			_oldSen = LVL10_BBNFG;
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			errK = ERR_OK;
			_retryCnt = 0;			// 반복회수 CLR
			devBbnMotChkStep = 20;
			//devBbnMotChkStep = 30;	// FOR CW TEST
			break;

		// CCW CHECK ( Repeat)
		case 20:
			_retryCnt++;		// 반복회수 증가
			if(RETRY_CNT < _retryCnt)	// Retry 3회 OVER
			{
				errK = ERR_CCW;		// CCW구분
				//devBbnMotChkStep = 75;	// ERROR

				bbnMotor.offBldc();
				
				setTO(_to);
				devBbnMotChkStep = 29;	// ERROR & CW확인
			}
			else
			{
				_oldSen = LVL10_BBNFG;
				_chgCnt = 0;			// 신호변화 Cnt
				//_out = ();	// 1-30, 2-40, 3-50
				bbnMotor.onBldc(BBN_MOT_CCW, stDevchkBbnMot[_retryCnt].duty);	// 줄 풀림
				setTO(_to);
				devBbnMotChkStep = 21;
			}
			break;

		case 21:
			if(checkTO(_to, stDevchkBbnMot[_retryCnt].ccw_ontime) )		//TO 400, 300, 200 ms
			{
				bbnMotor.offBldc();
				// CNT확인
				// 1) OK
				if(IN_SIG_CHG_CNT < _chgCnt)	// Pulse 3 이상? => OK
				{
					setTO(_to);
					devBbnMotChkStep = 29;	// OK, 100ms후 정지
				}
				// 2) NG
				else
				{
					setTO(_to);
					devBbnMotChkStep = 22;	// NG, 정지후 Retry
				}

				#if (LOG_DEVCHK_BBN_MOT)
					LogPrintln(" lg] DEVBM FG SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif
			
			}
			else if(LVL01_BBNFG != _oldSen)	// FG 신호 변화
			{
				_oldSen = LVL01_BBNFG;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}
			break;

		// 1-2] NG -> 50ms Wait & RETRY
		case 22:	//OFF Wait 50ms
			if(checkTO(_to, DEVCHK_BBN_RETRY_OFF_TO))	// TO
			{
				devBbnMotChkStep = 20;	// NG, RETRY
			}
			break;

		//1-2] OK, CW CHECK
		case 29:
			if(checkTO(_to, DEVCHK_BBN_DIR_CHANGE_TO))	// TO
			{
				_retryCnt = 0;			// 반복회수 CLR
				devBbnMotChkStep = 30;	// CW 체크 Start
			}
			break;

		// 2] CW확인, RETRY Check
		case 30:
			_retryCnt++;		// 반복회수 증가
			if(RETRY_CNT < _retryCnt)
			{
				if(ERR_OK == errK)
				{
					errK = ERR_CW;		// CW구분
				}
				devBbnMotChkStep = 75;	// ERROR
			}
			else
			{
				_oldSen = LVL10_BBNFG;
				_chgCnt = 0;			// 신호변화 Cnt
				bbnMotor.onBldc(BBN_MOT_CW, stDevchkBbnMot[_retryCnt].duty);
				setTO(_to);
				devBbnMotChkStep = 31;
			}
			break;

		case 31:
			if( checkTO(_to, stDevchkBbnMot[_retryCnt].cw_ontime))		//TO 400, 300, 200 ms
			{
				bbnMotor.offBldc();
				//	2-1) OK
				if(IN_SIG_CHG_CNT < _chgCnt)
				{
					setTO(_to);
					devBbnMotChkStep = 39;	// OK, END
				}
					// 2-2) NG, REPEAT
				else
				{
					setTO(_to);
					devBbnMotChkStep = 32;	// NG, 정지후 Retry
				}

				#if (LOG_DEVCHK_BBN_MOT)
					LogPrintln(" lg] DEVBM FG SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif

			}
			else if(LVL01_BBNFG != _oldSen)	// FG 신호 변화
			{
				_oldSen = LVL01_BBNFG;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}
			break;

		case 32:
			if(checkTO(_to, DEVCHK_BBN_RETRY_OFF_TO))	// TO
			{
				devBbnMotChkStep = 30;	//OK
			}
			break;

		//	2-2)OK. OFF => END
		case 39:
			if(checkTO(_to, DEVCHK_BBN_RETRY_OFF_TO))	// TO
			{
				devBbnMotChkStep = 90;	// CW 체크 Start
			}
			break;


		// 모터 정지, ERR/OK 분기
		case 75:
			bbnMotor.offBldc();
			if(ERR_OK == errK)	// OK
			{
				devBbnMotChkStep = 90;		// 정상 종료
			}
			else	// ERROR CCW, CW
			{
				devBbnMotChkStep = 80;		// Error
			}
			break;


		// Error SET
		case 80:
			//bbnMotor.offBldc();

			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devCheckFlagStep_AllClear();
			
			msg = STX_DEV_CONT_REQ + MAIN_BLDC_MOT_ERR;
			msg += "%";
			Resp2ApPrintln(msg);

			#if (LOG_DEVCHK_BBN_MOT)
				msg = ((ERR_CCW == errK)? "CCW":"CW_");
				LogPrintln(" LG] DevRq BLDC ERR 6010 " + msg);
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif
			
			break;
			
		//END_OK
		case 90:
			devCheckBbnMotFlag = 0;	// CLR BLDC Flag
			devBbnMotChkStep = 0;		// Clr Step
			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk BLDC MOT OK");
			#endif
			break;

		default:
			break;
		
	}
	
}

#define ENC_SEN_OK	0
#define ENC_SEN_A_ERR 1
#define ENC_SEN_B_ERR 2

// Device Ready Check STEP Control - BBN ENC A
//	BLDC CCW - ENC_A 2번 변화 Check, 300ms내에
//	BLDC  CW - ENC_B 2번 변화 Check, 300ms내에
//-------------------------------------------------
void devchk_bbnEnc_Control()
{
	static unsigned long _to;

	static unsigned short olddevBbnEncChkStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;

	static unsigned short _senErrK = ENC_SEN_OK;
	

	//static int _oldSenB;
	
	String msg;
		
	switch(devBbnEncChkStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			_oldSen = LVL01_BBNENCA;
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();
			
			_chgCnt = 0;
			_retryCnt = 0;
			_senErrK = ENC_SEN_OK;
			devBbnEncChkStep = 20;
			break;

		// CCW - SEN A - CHECK
		case 20:
			_oldSen = LVL01_BBNENCA;

			bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_DEFAULT_DUTY+0));

			setTO(_to);
			devBbnEncChkStep = 21;
			break;

		case 21:
			if(checkTO(_to, DEVCHK_ENC_SEN_TO))	// TO
			{
				bbnMotor.offBldc();
				if(IN_SIG_CHG_CNT < _chgCnt)
				{
				}
					// 2-2) NG, REPEAT
				else
				{
					_senErrK = ENC_SEN_A_ERR;
				}
				setTO(_to);
				//devBbnMotChkStep = 39;	// OK, END
				devBbnEncChkStep = 29;	// Set Error & ENC_B Check

				#if (LOG_ENC_SEN_ERR)
					LogPrintln(" lg] DEVBM ENC_A SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(_senErrK));
				#endif

			}
			else if(LVL01_BBNENCA != _oldSen)	// 1 times
			{
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}
			break;

		//	2-2)OK. OFF => END
		case 29:
			if(checkTO(_to, DEVCHK_BBN_DIR_CHANGE_TO))	// TO
			{
				_chgCnt = 0;
				_retryCnt = 0;
				devBbnEncChkStep = 30;	// CW 체크 Start
			}
			break;

			
		// CW - SEN B - CHECK
		case 30:
			_oldSen = LVL01_BBNENCB;

			bbnMotor.onBldc(BBN_MOT_CW, (DEVCHK_BBN_DEFAULT_DUTY+0));

			setTO(_to);
			devBbnEncChkStep = 31;
			break;
			
		case 31:
			if(checkTO(_to, DEVCHK_ENC_SEN_TO))	// TO
			{
				bbnMotor.offBldc();
				if(IN_SIG_CHG_CNT < _chgCnt)
				{
					// OK
				}
				else		// 2-2) NG, REPEAT
				{
					if(ENC_SEN_OK == _senErrK)
					{
						_senErrK = ENC_SEN_B_ERR;
					}
				}
				#if (LOG_ENC_SEN_ERR)
					LogPrintln(" lg] DEVBM ENC_B SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(_senErrK));
				#endif
				
				setTO(_to);
				if(ENC_SEN_OK == _senErrK)	//OK, END
				{
					devBbnEncChkStep = 90;
				}
				else		// NG,
				{
					devBbnEncChkStep = 80;	// Set Error & ENC_B Check
				}

			}
			else if(LVL01_BBNENCB != _oldSen)	// 1 times
			{
				_oldSen = LVL01_BBNENCB;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}
			break;

		// CW CHECK
		case 60:
			break;

		case 70:
			break;

		// Error SET
		case 80:
			bbnMotor.offBldc();

			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devBbnEncChkStep = 0;		// Clr Step
			devCheckFlagStep_AllClear();

			msg = STX_DEV_CONT_REQ;
			if (ENC_SEN_A_ERR == _senErrK)	{ msg += MAIN_BBNENCA_SEN_ERR; }
			else							{ msg += MAIN_BBNENCB_SEN_ERR; }
			msg += "%";
			Resp2ApPrintln(msg);

			#if (LOG_ENC_SEN_ERR)
				LogPrintln(" LG] DevRq BBN ENC SEN ERR: " + msg);
			#endif
			
			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif
			
			break;
			
		//END_OK
		case 90:
			devCheckBbnEncFlag = 0;	// CLR EncSen Flag
			devBbnEncChkStep = 0;		// Clr Step
			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk ENC SEN A,B OK");
			#endif
			break;

		default:
			break;
		
	}
	
}


//devBldc24VChkFlag = 0;

//-------------------------------------------------------------
#define DEVCHK_TORQ_BASE_DUTY	70	// 100 // 180  //   75	//70
#define DEVCHK_TORQ_STEP_DUTY	25 // 30

#define DEVCHK_TORQ_BLDC24V_DUTY	70 // 60		// BLDC 24V 장애검지
#define DEVCHK_TORQ_BLDC24V_STEP_DUTY	10	// 30


#define DEVCHK_TORQ_LAST_RELEASE_TIME	200 // 250	// 200	// 300	// Torq Release Time


// Mot On ~ 1st Enc Change TIME
void devchk_bldc24v_Control_old()
{
	static unsigned long _to;

	static unsigned short olddevBldc24VChkStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;
	
	String msg;


#if (LOG_BLDC24V_STEP)
		// Step Change LOG OUT
		if(olddevBldc24VChkStep != devBldc24VChkStep )
		{
			LogPrintln(" lg] B24st DevChck BLDC 24V STEP : " + String(devBldc24VChkStep));
			olddevBldc24VChkStep = devBldc24VChkStep;
		}
#endif
	
	switch(devBldc24VChkStep)
	{

		//IDLE
		case 0:
			break;

		// START
		case 10:
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			_oldSen = LVL01_BBNENCA;

			setTO(_to);
			devBldc24VChkStep = 11;
			break;

		// BLDC CCW로 돌려 줄을 풀어줌
		case 11:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );	// 줄 풀기(Duty 50)
				setTO(_to);
				devBldc24VChkStep = 12;
			}
			break;

		case 12:
			if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
			{
				bbnMotor.offBldc();
				setTO(_to);
				devBldc24VChkStep = 13;
			}
			break;

		case 13:
			if(checkTO(_to, 300) )	//WAIT 300 ms TO
			{
				_oldSen = LVL01_BBNENCA;
				torqMotor.setValue(DEVCHK_TORQ_BASE_DUTY); 	//DEFAULT(64) OUT = TENSION
				setTO(_to);
				devBldc24VChkStep = 14;
			}
			break;


		case 14:
			if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				torqOff();
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
				setTO(_to);
				devBldc24VChkStep = 15;
			}
			break;

		case 15:
			if(checkTO(_to, DEVCHK_BLDC24V_RETRY_WAIT_TO) )	//WAIT 300 ms TO
			{
				setTO(_to);
				devBldc24VChkStep = 20;
			}
			break;

		// 토크모터 체크 (CW방향 - 줄감기는 방향)
		case 20:
			_retryCnt++;		// 반복회수 증가
			if(RETRY_CNT < _retryCnt)	// Retry 3회 OVER
			{
				errK = ERR_CW;		// Error

				setTO(_to);
				devBldc24VChkStep = 75;	// ERROR
			}
			else
			{
				_oldSen = LVL01_BBNENCA;
				_chgCnt = 0;			// 신호변화 Cnt
				//_out = ();	// 1-30, 2-40, 3-50
				torqMotor.setValue(DEVCHK_TORQ_BASE_DUTY); 	//DEFAULT(64) OUT = TENSION
				setTO(_to);
				devBldc24VChkStep = 21;
			}
			break;

		// 센서 카운트,  TO => MOT_OFF
		case 21:
			if(checkTO(_to, DEVCHK_TORQ_TO_ENCA_1ST_ON_CHK_TO) )	// TO	600ms
			{
				// 1) NG
				torqOff();

				setTO(_to);
				devBldc24VChkStep = 22;	// NG, 정지후 Retry
					
				#if (LOG_DEVCHK_BLDC24V_ERR)
					LogPrintln(" lg] DEVCK BLDC_24V NG chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif

			}
			else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				torqOff();
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					// Mot On ~ EncA 시간 측정
					// 300ms미만이면 OK,
					// 300ms이상이면 NG : BLDC 24V NG
				if(checkTO(_to, DEVCHK_TORQ_TO_ENCA_1ST_ON_SPEC_TO) )	// 300ms OVER?
				{
						// Error => 반복 2회
					setTO(_to);
					devBldc24VChkStep = 22;	// NG, 정지후 Retry
				}
				else
				{
						// 정상
					errK = ERR_OK;
					setTO(_to);
					devBldc24VChkStep = 75;	// NG, 정지후 Retry
				}

			}
			break;

		// NG & REPEAT
		case 22:
			if(checkTO(_to, DEVCHK_BLDC24V_RETRY_WAIT_TO) )	// WAIT 300ms
			{
				setTO(_to);
				devBldc24VChkStep = 20;	// REPEAT
			}
			break;

		case 75:
			// 모터 OFF
			bbnMotor.offBldc();
			torqOff();

			if(ERR_OK == errK)
			{
				//setTO(_to);
				devBldc24VChkStep = 90;	// END-OK
			}
			else
			{
				//setTO(_to);
				devBldc24VChkStep = 80;	//NG
			}
			break;
			
		// Error SET
		case 80:
			devReadychkFlag = 0;	//에러 => DEVICE CHK종료
			devBldc24VChkStep = 0;
			devCheckFlagStep_AllClear();

			// Err MSG Send
			msg = STX_DEV_CONT_REQ + MAIN_BOARD_BLDC_24V_ERR;
			msg += "%";
			Resp2ApPrintln(msg);

			#if (LOG_DEVCHK_TORQ_ERR)
				LogPrintln(" LG] DevRq BLDC_24V ERR 6041");
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif

			break;

		// OK
		case 90:
			devCheckBldc24VFlag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
			devBldc24VChkStep = 0;		// Clr Step

			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk TORQ MOT OK");
			#endif

			break;

		default:
			break;
			
	}
	
}

//--------------------------------------------------------------------------
//	1.2초도안 14펄스 미만이면 속도느림(24V FET손상)으로 판단.
//
void devchk_bldc24v_Control()
{
	static unsigned long _to;

	static unsigned long _toSenChg;
	static unsigned long pulseWidthMs;

	static unsigned short olddevBldc24VChkStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;
	
	String msg;


#if (LOG_BLDC24V_STEP)
		// Step Change LOG OUT
		if(olddevBldc24VChkStep != devBldc24VChkStep )
		{
			LogPrintln(" lg] B24st DevChck BLDC 24V STEP : " + String(devBldc24VChkStep));
			olddevBldc24VChkStep = devBldc24VChkStep;
		}
#endif
	
	switch(devBldc24VChkStep)
	{

		//IDLE
		case 0:
			break;

		// START
		case 10:
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			errK = ERR_OK;
			_oldSen = LVL01_BBNENCA;

			setTO(_to);
			devBldc24VChkStep = 11;
			break;

		// BLDC CCW로 돌려 줄을 풀어줌
		case 11:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );	// 줄 풀기(Duty 50)
				setTO(_to);
				devBldc24VChkStep = 12;
			}
			break;

		case 12:
			//if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
			if(checkTO(_to, DEVCHK_BLDC24V_REL_ON_TO) )	//WAIT 100 ms TO
			{
				bbnMotor.offBldc();
				setTO(_to);
				devBldc24VChkStep = 13;
			}
			break;

		case 13:
			if(checkTO(_to, DEVCHK_BLDC24V_OFF_WAIT_TO) )	//24V OFF WAIT, 1500ms
			{
				_oldSen = LVL01_BBNENCA;
				_chgCnt = 0;			// 신호변화 Cnt
				torqMotor.setValue(DEVCHK_TORQ_BLDC24V_DUTY); 	//DEFAULT(64) OUT = TENSION
				setTO(_to);
				devBldc24VChkStep = 14;
			}
			break;


		case 14:
		  #if (TEST_BLDC24V_FORCE_ON)
			// 1.2초 동안 20 pulse이상 입력
			if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || ((20*2) < _chgCnt))	//WAIT 300 ms TO
		  #else
			//if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || ((14*2) < _chgCnt))	//WAIT 300 ms TO
			//if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || ((10*2) < _chgCnt))	//WAIT 300 ms TO
			// 1.2초 동안, 9 펄스이상-OK, 9펄스 미만 -NG
			if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || ((9*2) <= _chgCnt))	//WAIT 300 ms TO
		  #endif
			{
				int time = getTO(_to);
				torqOff();
				//if( (6*2) > _chgCnt)	// 펄스 미만 이면 NG
				//if( (15*2) > _chgCnt)	// 펄스 미만 이면 NG
				if( (9*2) > _chgCnt)	// 펄스 미만 이면 NG
				{
					errK = ERR_CW;
					setTO(_to);
					devBldc24VChkStep = 75;
				}
				else
				{
					bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );	// OK시, 모터 브레이크
					errK = ERR_OK;
					setTO(_to);
					devBldc24VChkStep = 30;
				}
				// 검출결과, AP에 출력(확인용)
				Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_BOARD_BLDC_24V_ERR + (ERR_OK==errK? "OK":"NG") + ",try:1,Cnt:" + String(_chgCnt)+",ms:" + String(time) );
				
				// LOG
				#if (LOG_BLDC24V_STEP)
					LogPrintln(" lg] DEVCK BLDC24V Pulse:" + String(pulseWidthMs) + "ms");
				#endif
			}
			else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
				_chgCnt++;
				pulseWidthMs = getTO(_toSenChg);
				setTO(_toSenChg);
			}
			break;


	// NOT USE
		case 15:
			if(checkTO(_to, DEVCHK_BLDC24V_RETRY_WAIT_TO) )	//WAIT 300 ms TO
			{
				setTO(_to);
				devBldc24VChkStep = 20;
			}
			break;

		// 토크모터 체크 (CW방향 - 줄감기는 방향)
		case 20:
			_retryCnt++;		// 반복회수 증가
			if(RETRY_CNT < _retryCnt)	// Retry 3회 OVER
			{
				errK = ERR_CW;		// Error

				setTO(_to);
				devBldc24VChkStep = 75;	// ERROR
			}
			else
			{
				_oldSen = LVL01_BBNENCA;
				_chgCnt = 0;			// 신호변화 Cnt
				//_out = ();	// 1-30, 2-40, 3-50
				torqMotor.setValue(DEVCHK_TORQ_BASE_DUTY); 	//DEFAULT(64) OUT = TENSION
				setTO(_to);
				devBldc24VChkStep = 21;
			}
			break;

		// 센서 카운트,  TO => MOT_OFF
		case 21:
			if(checkTO(_to, DEVCHK_TORQ_TO_ENCA_1ST_ON_CHK_TO) )	// TO	600ms
			{
				// 1) NG
				torqOff();

				setTO(_to);
				devBldc24VChkStep = 22;	// NG, 정지후 Retry
					
				#if (LOG_DEVCHK_BLDC24V_ERR)
					LogPrintln(" lg] DEVCK BLDC_24V NG chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif

			}
			else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				torqOff();
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					// Mot On ~ EncA 시간 측정
					// 300ms미만이면 OK,
					// 300ms이상이면 NG : BLDC 24V NG
				if(checkTO(_to, DEVCHK_TORQ_TO_ENCA_1ST_ON_SPEC_TO) )	// 300ms OVER?
				{
						// Error => 반복 2회
					setTO(_to);
					devBldc24VChkStep = 22;	// NG, 정지후 Retry
				}
				else
				{
						// 정상
					errK = ERR_OK;
					setTO(_to);
					devBldc24VChkStep = 75;	// NG, 정지후 Retry
				}

			}
			break;

		// NG & REPEAT
		case 22:
			if(checkTO(_to, DEVCHK_BLDC24V_RETRY_WAIT_TO) )	// WAIT 300ms
			{
				setTO(_to);
				devBldc24VChkStep = 20;	// REPEAT
			}
			break;
	// NOT USE ---------
	
		// OK, BLDC역전 50ms
		case 30:
			if(checkTO(_to, DEVCHK_BLDC24V_BREAK_TO) )	//BREAK 50ms
			{
				setTO(_to);
				devBldc24VChkStep = 75;	// REPEAT
			}
			break;


		// OK , NG 판단
		case 75:
			// 모터 OFF
			bbnMotor.offBldc();
			torqOff();

			if(ERR_OK == errK)
			{
				//setTO(_to);
				devBldc24VChkStep = 90;	// END-OK
			}
			else
			{
				//setTO(_to);
				devBldc24VChkStep = 80;	//NG
			}
			break;
			
		// Error SET
		case 80:
			devReadychkFlag = 0;	//에러 => DEVICE CHK종료
			devBldc24VChkStep = 0;
			devCheckFlagStep_AllClear();

			// Err MSG Send
			msg = STX_DEV_CONT_REQ + MAIN_BOARD_BLDC_24V_ERR;
			msg += "%";
			Resp2ApPrintln(msg);

			// 검지시간 및 엔코더 수를 AP에 출력(발생상화 판단용) 추가

			
			#if (LOG_DEVCHK_TORQ_ERR)
				LogPrintln(" LG] DevRq BLDC_24V ERR 6041");
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 ERR종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif
			
			break;

		// OK
		case 90:
			devCheckBldc24VFlag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
			devBldc24VChkStep = 0;		// Clr Step

			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk TORQ MOT OK");
			#endif

			break;

		default:
			break;
			
	}
	
}


// Device Ready Check STEP Control - TORQ
//	TORQ ON(CW) - ENC_A  2번 변화 Check, 300ms내에
//-------------------------------------------------
void devchk_torq_Control()
{
	static unsigned long _to;

	static unsigned short olddevTorqChkStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;
	
	String msg;

	
	switch(devTorqChkStep)
	{
		//IDLE
		case 0:
			break;

		// START
		case 10:
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			_oldSen = LVL01_BBNENCA;

			setTO(_to);
			devTorqChkStep = 11;
			break;

		// BLDC CCW로 돌려 줄을 풀어줌
		case 11:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY));	// 줄 풀기
				setTO(_to);
				devTorqChkStep = 12;
			}
			break;

		case 12:
			if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO))	//WAIT 500 ms TO
			{
				bbnMotor.offBldc();
				setTO(_to);
				devTorqChkStep = 13;
			}
			break;

		case 13:
			if(checkTO(_to, DEVCHK_TORQ_STOP_WAIT_TO))	//WAIT 50 ms TO
			{
				setTO(_to);
				devTorqChkStep = 20;
			}
			break;

		// 토크모터 체크 (CW방향 - 줄감기는 방향)
		case 20:
			_retryCnt++;		// 반복회수 증가
			if(RETRY_CNT < _retryCnt)	// Retry 3회 OVER
			{
				errK = ERR_CW;		// CCW구분

				setTO(_to);
				devTorqChkStep = 75;	// ERROR
			}
			else
			{
				_oldSen = LVL01_BBNENCA;
				_chgCnt = 0;			// 신호변화 Cnt
				//_out = ();	// 1-30, 2-40, 3-50
				torqMotor.setValue(DEVCHK_TORQ_BASE_DUTY + (_retryCnt*DEVCHK_TORQ_STEP_DUTY) - DEVCHK_TORQ_STEP_DUTY ); 	//DEFAULT(64) OUT = TENSION
				setTO(_to);
				devTorqChkStep = 21;
			}
			break;

		// 센서 카운트,  TO => MOT_OFF
		case 21:
			if(checkTO(_to, (DEVCHK_TORQ_START_DELAY_TO + DEVCHK_TORQ_MOT_ON_TO + DEVCHK_TORQ_MOT_STEP_TO - (_retryCnt*DEVCHK_TORQ_MOT_STEP_TO))))	// TO
			{
				// 1) OK
				torqOff();
				if(IN_SIG_CHG_CNT < _chgCnt)	// Pulse 3 이상? => OK
				{
					setTO(_to);
					devTorqChkStep = 75;	// OK,
				}
				// 2) NG
				else
				{
					setTO(_to);
					devTorqChkStep = 22;	// NG, 정지후 Retry
				}

				#if (LOG_DEVCHK_BBN_MOT)
					LogPrintln(" lg] DEVBM FG SEN chg:" + String(_chgCnt) + " cnt, Err:" + String(errK));
				#endif

			}
			else if(LVL01_BBNENCA != _oldSen)	// 1 times
			{
				_oldSen = LVL01_BBNENCA;	// 센서상태 저장
				_chgCnt++;
			}
			break;

		// NG & REPEAT
		case 22:
			if(checkTO(_to, DEVCHK_TORQ_MOT_OFF_TO))	// WAIT 100ms
			{
				setTO(_to);
				devTorqChkStep = 20;	// REPEAT
			}
			break;

		case 75:
			// 모터 OFF
			bbnMotor.offBldc();
			torqOff();

			if(ERR_OK == errK)
			{
				devTorqChkStep = 90;	// END-OK
			}
			else
			{
				devTorqChkStep = 80;	//NG
			}
			break;
			
		// Error SET
		case 80:
			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devTorqChkStep = 0;
			devCheckFlagStep_AllClear();

			// Err MSG Send
			msg = STX_DEV_CONT_REQ + MAIN_TORQ_MOT_ERR;
			msg += "%";
			Resp2ApPrintln(msg);

			#if (LOG_DEVCHK_TORQ_ERR)
				LogPrintln(" LG] DevRq TORQ_MOT ERR 6011");
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 ERR 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			#endif
			
			break;

		// OK
		case 90:
			devCheckTorqFlag = 0;	// CLR BLDC Flag
			devTorqChkStep = 0;		// Clr Step

			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk TORQ MOT OK");
			#endif

			break;

		default:
			break;
			
	}
	
}
#endif

// TBD-LM
// LM_MOT Device Check
void devchk_lmMot_Control()
{
	// TBD-LM
#if (IO_LM_MOT)
		devCheckLmMotFlag = 0;

		// ERR 종료시,
		//devCheckFlagStep_AllClear();
		
		#if (FUNC_STBY_OFF_CONT)
			//== ERROR종료시 [STBY토크OFF제어]요구 Check필요
			stbyTorqOff_ReqCheck();
		#endif

#endif
	
}

// TBD-LM
// LM ENC Check Control
void devchk_lmEnc_Control()
{

}


//-----------------------------------------------
//	줄 끊어짐 검지
//-----------------------------------------------
#if (0)		// NOT_USE_CODE
void devchk_WireBroken_Start()
{
	devCheckWireBroken_Flag = 1;	
	devCheckWireBroken_Step = 10;
}

void devchk_WireBroken_Stop()
{
	devCheckWireBroken_Flag = 0;	
	devCheckWireBroken_Step = 0;
}


void devchk_WireBroken_Control()
{
	static unsigned long _to;
	static unsigned long _toErr;

	static unsigned short olddevCheckWireBroken_Step;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;
	
	String msg;

	
	switch(devCheckWireBroken_Step)
	{
		//IDLE
		case 0:
			break;

		// START
		case 10:
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			_oldSen = LVL01_BBNENCA;

			setTO(_to);
			devCheckWireBroken_Step = 11;
			break;

		// 모터정지 대기
		case 11:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				setTO(_to);
				devCheckWireBroken_Step = 20;
			}
			break;

		//토크모터 기동(최소값+5)
		case 20:
			_chgCnt = 0;
			_oldSen = LVL01_BBNENCA;

			torqMotor.setValue(defaultTorqueMotor+5);
			setTO(_to);
			setTO(_toErr);
			devCheckWireBroken_Step = 21;
			break;

		case 21:
			// ERR
			if( checkTO(_toErr, CHECK_TIME_10SEC_TO) )
			{
				//ERR
				errK = ERR_WIRE_BROKEN;
				setTO(_to);
				devCheckWireBroken_Step = 75;
			}
			// OK
			else if( checkTO(_to, CHECK_TIME_2SEC_TO) )	//ENC센서 안정후 3초 경과 => OK
			{
				setTO(_to);
				devCheckWireBroken_Step = 75;
			}
			else if(LVL01_BBNENCA != _oldSen)
			{
				_oldSen = LVL01_BBNENCA;
				_chgCnt++;
				setTO(_to);
			}
			break;

		case 75:
			// 모터 OFF
			bbnMotor.offBldc();
			torqOff();

			if(ERR_OK == errK)
			{
				devCheckWireBroken_Step = 90;	// END-OK
			}
			else
			{
				devCheckWireBroken_Step = 80;	//NG
			}
			break;
			
		// Error SET
		case 80:
			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devCheckWireBroken_Step = 0;
			devCheckFlagStep_AllClear();

			// Err MSG Send
			msg = STX_DEV_CONT_REQ + MAIN_WIRE_BROKEN_ERR;
			msg += "%";
			Resp2ApPrintln(msg);

			#if (LOG_DEVCHK_WIRE_BROKEN_ERR)
				LogPrintln(" LG] DevRq WIRE BROKEN ERR 6012");
			#endif

			#if (FUNC_STBY_OFF_CONT)
				// 장치장애검지 ERR 종료시, STBY토크OFF제어 요구 CHECK
				// ** 줄 끊어짐때는 OFF제어 안함!!!
				stbyTorqOff_ReqCheck();
			#endif
			
			break;

		// OK
		case 90:
			devCheckWireBroken_Flag = 0;	// CLR BLDC Flag
			devCheckWireBroken_Step = 0;		// Clr Step

		#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] DevCk TORQ MOT OK");
		#endif

			break;

		default:
			break;
			
	}

}
#endif

//-------- STEP CONTROL END -----------------


//----------------------------------------------
// Sensor & Motor Ready Check ( Home Position etc)
//	1. bldc Motor Check
//	2-1. Bobbin Enc Sensor A Check
//	2-2. Bobbin Enc Sensor B Check
//	3. BLDC 24V(FET) Check
//	4. Torque Motor Check
//	5. Wire Broken Check
//
//======= TBD ============
void devReadyCheck()
{
	//장애 검지 Start
	devReadychkFlag = 1;

	devChkErrOccure = 0;
	// 각 Dev Check Flag및 Control Step START

#if 1		// (V108) 장치 통합 체크 함수로 대체.	
	devchk_bbnAllStart();
#else		// (V108) 아래 함수 호출 제거
	//devchk_bldcStart();
	devchk_bbnMotStart();
	devchk_bbnEncStart();
	
	devchk_Bldc24vStart();	// BLDC_24V Check ADD
	
	devchk_TorqStart();
	//devchk_WireBroken_Start();		// 줄 끊어짐 검사 시작
#endif

#if (IO_LM_MOT)
	devchk_lmMotStart();
#endif	
	// stop 
	// Error발생시 Dev Check 중지
}

// 에러 CLR
void send_Error_Clr()
{
	String respMsg;
	respMsg = STX_DEV_CONT_REQ + "0000";	// OK응답 0000 (4자리)
	respMsg += "%";
	Resp2ApPrintln(respMsg);
}

//-------------------------------------------------------------------
// 장치 제어 명령
void anaDeviceContReq(String msg)
{
	unsigned int iKind = msg.substring(3).toInt();

	String respMsg ="--";
	
	switch(iKind)
	{
		// 모든 출력 정지 요구.
		case 0:			// Device All OFF
			motor_AllOff();		// MAIN ALL OFF
			
			reelOut_AllOff();	// REEL ALL OFF

			#if (0)	// "게임 처음 실행시, 정상으로 넘어감(코드는 6010 표시됨) " 문제 있음.
				respMsg = STX_DEV_CONT_REQ + "0000";	// OK응답 0000 (4자리)
				//respMsg = STX_DEV_CONT_REQ + "000";	// 응답 000 (3자리)
				//respMsg = STX_DEV_CONT_REQ + "00";	// 응답 00 (2자리)
				respMsg += "%";
				Resp2ApPrintln(respMsg);
			#endif
		
			LogPrintln(" LG] DevRq $1500 Device AllOff");
			break;

		// [장애검지] 요구
		case 1:			// Device Ready Check
			#if (DEV_CHECK_SKIP)	// TEST
				// Msg Test
				//devChkMsgSend(MAIN_DEV_CHK_OK);
				//devChkMsgSend(MAIN_ROD_COMM_ERR);
				//devChkMsgSend(ROD_IMU_COMM_ERR);
				devchk_OkEnd();
			#else	// REL

			// AP가 계속 반복 통지를 대응(처리중 요구는 무시)
			if(0 == devReadychkFlag)
			{
				// 1) 검지한 에러 클리어.
				if(devChkErrOccure)
				{
					devChkErrOccure = 0;
				}

				#if (FUNC_STBY_OFF_CONT)
					//2) STBY 토크OFF제어 중첩 처리
						// 대기OFF제어 중이면, 중지 & 예약
					if(stbyTorqOffControl_Flag) 	// [STBY토크OFF]제어 중?
					{
						stbyTorqOffControl_Req_Flag = 1;	// 장치장애검지 종료시, OFF재시작 SET
						stbyTorqOffControl_Stop(0);	// OFF제어 중지. (V108) 토크 모터 기본값 출력 안함
					}
						// 대기OFF제어 중이 아니고 && 현상태가 대기중이면, 예약
					else if( GMWAT_STBY == gmWatStatus)
					{
						stbyTorqOffControl_Req_Flag = 1;	// 예약				
					}
				#endif

				// Error CLR SEND
				#if (0)	// NOT SEND CLR
					send_Error_Clr();		// TBD
				#endif
			
				// 3) Device Ready Check
				devReadyCheck();
				// 4) 장치제어 종료(OK/ERR)때 , 대기OFF제어 Check는 각 ERR및 OK종료 함수에서 Check 
			}

			#endif

			//== FW자체 값 자발 출력 to AP ( TM Only )
			if( AP_IS_TM == apType) 		// TM Only
			{
				Resp2ApPrintln(STX_INFO_REQ + MAINMOT_INFO_READ + String(defaultTorqueMotor)+"%");	// 메인모터 최소값
 				Resp2ApPrintln(STX_INFO_REQ + BLDC_MOT_LIMIT_READ + String(bldcLimitVal)+"%");		// BLDC제한 값 출력
			}
			//
			#if (LOG_DEV_ERR_CHECK)
				LogPrintln(" LG] DevRq $1501 Device Ready Check Start");
			#endif

			break;

		// TBD 2~99 : Reserved

		default:
			break;
	}
	
}

void anaRestDistance(String msg)
{
	
	//Log
#if (NEW_IF)
	restPowerDist = msg.substring(5,(5+3)).toInt();
	if( 10 > restPowerDist ) { restPowerDist = 10; }
	if(200 < restPowerDist ) { restPowerDist = 200; }

#else
	restPowerDist = msg.substring(8).toInt();
	if( 10 > restPowerDist ) { restPowerDist = 10; }
	if(100 < restPowerDist ) { restPowerDist = 100; }
#endif

	LogPrintln(" AP] RestD Dist: ," + String(restPowerDist) + " [/M]");

  #if 0		// Delete , Randing IF Implemention
	// TBD temp
	if( (10 >= restPowerDist) && (GAME_FIGHT == gameStatus))		// Fight중 10미터 이하 = Randing
	{
		LogPrintln(" AP] ExRND Distance: ," + String(restPowerDist) + "/M");
		gameStatus = GAME_RANDING;
		exeRanding = 1;
		execRanding_SetVal();
		execRanding_Start();

		if(oldgameStatus != gameStatus)
		{
			oldgameStatus = gameStatus;
		}
	}
  #endif
}



//
// Angle(Servo) Control BY AP COMMAND
void ana_AngleControl( String msg )
{
#if IO_SERVO
	#if (NEW_IF)
	 	reqServoMotor = msg.substring(3).toInt();	  // Save Req Angle
	#else
	 	reqServoMotor = msg.substring(8).toInt();	  // Save Req Angle
	#endif
	  	absReqServoMotor = abs(reqServoMotor);
	  	if(oldAbsServoMotor != absReqServoMotor)
	  	{
		  #if RESP_IO
			//Resp2ApPrintln("$03APreq" + msg.substring(8,11) +"%");		// 변경이 있을때 응답(절대값)
			Resp2ApPrintln(msg +"%");		// 변경이 있을때 응답(절대값)
		  #endif
		  #if LOG_IO
			LogPrintln(" AP] SRVch " + msg + ", "+String(reqServoMotor) + ","+String(absReqServoMotor) + " R/A"); 	// LOG-SERIAL1
		  #endif
			oldAbsServoMotor = absReqServoMotor;
	  	}

  #if (AP_MOT_CONT)
	#if (IO_SERVO)
	  // output Direct
	  sMotor.setAngle(msg);
	#endif

  #else	// NOT AP_MOT_CONT

	#if (HAND_TEST_01)
	  // HUD Follow

	  	//if(!(exeBite || (exeResist && exeResistServoENB) || (exeHoldon && exeHoldonServoENB) ))		// test : Not Bite or Not Resist or Not HoldOn
	  	// 버티기 일떄만 서보 제어 안함. 그외는 AP가 제어함.
	  	// BLDC제어시는 서보제어 안함.
	  /*
	  	if(!( (exeHoldon && exeHoldonServoENB) || isExecBldc() )	)	// test : Not Bite or Not Resist or Not HoldOn
	  	{
		          sMotor.setAngle(msg);		// Not Bite, then noAction Servo
	  	}
	  */
	  //== 1) FW ANGLE CONTROL, at  HOLDON ,
		if( (exeHoldon && exeHoldonServoENB) ) //  || isExecBldc() )
		{
	  		// FW Self Control
		}
	  //==2) AP ANGLE CONTROL, at NOT HOLDON ,
	  	else
	  	{
		#if (IO_SERVO)
		  sMotor.setAngle(msg); 	// Not Bite, then noAction Servo
		#endif
	  	}
	#else
		// output Direct
		#if (IO_SERVO)
			sMotor.setAngle(msg);
		#endif
	#endif

  #endif	// AP_MOT_CONT : END
  
#endif

}

//
// MAIN(TORQ) by AP COMMAND
//	$02ddd%  ddd:duty 3자리
//		+TTTT%	TTTT: 없는 경우, 있는 경우 [ms]후 정지
//
void ana_MainMotControl( String msg )
{
	int recvTorq;
	unsigned int onTime = 0;
	#if (NEW_IF)
	  	//recvTorq = msg.substring(3).toInt();		// 
	  	recvTorq = msg.substring(3,(3+3)).toInt();		// Duty
	  	// TTTT[ms]있으면 시간제어 추가
		onTime = (unsigned int)msg.substring(6).toInt();
	  	if(0 != onTime)	// TBD ?
	  	{
			#if (LOG_MAIN_MOT_ONTIME)
				LogPrintln(" lg] TORQo newIF:" + String(onTime) + " ms");
			#endif
	  	}
		else
		{
			#if (LOG_MAIN_MOT_ONTIME)
				LogPrintln(" lg] TORQM oldIF:" + msg);
			#endif
		}
	#else
		recvTorq = msg.substring(8).toInt();		// 
	#endif

//	int torq = reqTorqueMotor;
	int torq = recvTorq;		// 출력Torq저장

	// 수신데이타가 "0"보다 큰 경우만 저장.
	if(0 < recvTorq)
	{
		reqTorqueMotor = recvTorq;
		recv_TorqMotor_Flag = 1;
	}
	// Default보다 작으면 DEFAULT저장
	//if(DEFAULT_TORQ > reqTorqueMotor) 	{ reqTorqueMotor = DEFAULT_TORQ; }
	if(defaultTorqueMotor > reqTorqueMotor) 	{ reqTorqueMotor = defaultTorqueMotor; }

	//--- AP_MOT_CON
	#if (AP_MOT_CONT)
		#if (FW_CONT_BITE_HOLD)
		  #if (IO_LM_MOT)
			if(exeBite || devReadychkFlag || lmHome_Flag || lmLeft_Flag || lmRight_Flag || lmWireControl_Flag)
		  #else
		  	// FW입질제어중, 장치장애확인중, 대기중토크OFF제어중
			if(exeBite || devReadychkFlag || holdOnPtnAll_Flag || stbyTorqOffControl_Flag)
		  #endif
			{
				// NA - FW Control
			}
			else	// Not Bite, AP Control
			{
				torqMotor.setValue(torq);
				if(onTime && (AP_IS_TM == apType))
				{
					torqMot_OffControlStart(onTime);
				}
			}
		#else
			torqMotor.setValue(torq);
			if(onTime && (AP_IS_TM == apType))
			{
			  torqMot_OffControlStart(onTime);
		  	}
		#endif
		
		#if (RESP_IO_MAIN_TORQ)
		  Resp2ApPrintln(msg+"%");
		#endif
	//--- NOT AP MOT CONT ( FW SELF MOT CONT LOGIC )
	#else	// OLD = NOT AP_MOT_CONT
	  #if (HAND_TEST_01)
	    // TBD, HoldOn TORQ CONT ( AP or FW )
	    
		//if( !( exeBite || exeResist )) // || exeHoldon) )	  // TEST :  Not ( Bite or Resist ) 
		if( !( exeBite || exeResist || exeHoldon) )	  // TEST :  Not ( Bite or Resist or HoldOn ) 
		//if( !(exeResist || exeHoldon) )	  // TEST :  Not ( Bite or Resist or HoldOn ) 
		{
			#if (0)	// NOT_USE, BBN_BELT_FOLLOW // // Proto1_#3 Only
				//int torq = (msg.substring(8).toInt() + BBNBELT_AP_TORQ_ADDVAL);
				if(30 < reqTorqueMotor)
				{
					torq = (reqTorqueMotor + BBNBELT_AP_TORQ_ADDVAL);
				}
				if(255 < torq) torq = 255;
		  		torqMotor.setValue(torq);		// == AP Cont Torq + ADD_TORQ
		  	#else
				//torqMotor.setValue(msg);		// ==  AP Cont Torq
				torqMotor.setValue(torq);		// ==  AP Cont Torq
			#endif

			#if (RESP_IO)
				Resp2ApPrintln(msg+"%");
			#endif
		}
		else
		{
			// == FW SELF CONTROL TORQ
		}
	  //#if !(HAND_TEST_01)
	  #else		// OLD
		torqMotor.setValue(msg);
		#if (RESP_IO)
		  Resp2ApPrintln(msg+"%");
		#endif
	  #endif
	  
	//-- END
	#endif		// AP_MOT_CONT : END
	
	#if (LOG_IO_TORQ)
	  //LogPrintln(" AP] torq_ " + String(torq) + " /255 Duty");
	#endif

}

#define AP_BBN_CMD_OFF 0
#define AP_BBN_CMD_CW 1
#define AP_BBN_CMD_CCW 2
#define AP_BBN_CMD_MAX 3

String reqBobbinMsg;
// Bite PTN_2 PreCheck Control
//	$04
void ana_BobbinControl_Check(String msg)
{
	reqBobbinMsg = msg;
	// TBD : Dir, Value Save
	
	#if (FW_CONT_BITE_HOLD)
	  #if (IO_LM_MOT)
		if(exeBite || devReadychkFlag || lmHome_Flag || lmLeft_Flag || lmRight_Flag || lmWireControl_Flag)
	  #else
		if(exeBite || devReadychkFlag )
	  #endif
		{
			// NA - FW Control
		}
		else	// Not Bite, AP Control
		{
			ana_BobbinControl(msg);
		}
	#else
		ana_BobbinControl(msg);
	#endif

}

#define AP_LM_CMD_OFF 	0
#define AP_LM_CMD_CW 	1
#define AP_LM_CMD_CCW 	2
#define AP_LM_CMD_CW_DD 	3
#define AP_LM_CMD_CCW_DD 	4
#define AP_LM_CMD_ACT 		5
#define AP_LM_CMD_MAX 	6



String reqLmMotMsg;
void ana_LineMotControl_Check(String msg)
{
#if (IO_LM_MOT)
	reqLmMotMsg = msg;
	
	ana_LmMotControl(msg);
#endif
}

//
// $04ADDDTTTT%
//
void ana_BobbinControl(String msg)
{

	String respMsg = "";
	
	int act = msg.substring(3,(3+1)).toInt();
	if(0 > act ) { act = 4; }		// Not Action
	int duty = msg.substring(4,(4+3)).toInt();
	// Range Check
	if(0 > duty) { duty = 0; }
	if(255 < duty) { duty = 255; }
	int onTime = msg.substring(7,(7+4)).toInt();
	if(0 > onTime ) { onTime = 0; }		// Not STOP

	// TBD Para NG Control
	// if(paraNgFlag)
	{
		// Error
		// return;
	}

#if (LOG_BLDC_AP_CMD)
	LogPrintln(" AP] BLDCo " + msg);
#endif

	if(0) { }	// dummy
	
	else if(AP_BBN_CMD_OFF == act)
	{
		bbnMotor.offBldc();
	}
	else if(AP_BBN_CMD_CW == act)	// CW
	{
		bbnMotor.onBldc(BBN_MOT_CW, duty);
		if(onTime) { bbnMotor_OffTime_Start(onTime); }
	}
	else if(AP_BBN_CMD_CCW == act)	// CCW
	{
		bbnMotor.onBldc(BBN_MOT_CCW, duty);
		if(onTime) { bbnMotor_OffTime_Start(onTime); }
	}
	else
	{
		// Error . NA
	}

#if RESP_IO_BBN_BLDC
	if(AP_BBN_CMD_MAX > act)
	{
		respMsg = msg.substring(0,(0+4)) + fillZero2String3Char(bbnMotor.sbbnOut) + msg.substring(7) + "%";
		Resp2ApPrintln(respMsg);	// Resp at Valid CMD only
	}
#endif
	
}


//-----------------------------------------------
//
//  Ana AP CMD, LM_MOT_CMD
//
//	=== LM MOT($07xx)
//-----------------------------------------------


#define END_TM_DUTY_OFF DEFAULT_TORQ	// End TM Duty  0 = OFF
#define DUTY2SPD_RATE	(1)		// 1 비율

#define DEFAULT_LM_PWR	35	// DEFAULT DUTY
//-----------------------------------------------
void ana_LmMotControl(String msg)
{
#if (IO_LM_MOT)
		int act = msg.substring(3,(3+1)).toInt();
		// TBD-LM, AP 연계 필요
		int fish;		// 2
		int pwr; 	// b
		
		int duty;
		int onTime;
		int actCmd;
		unsigned int para = 0xFFFFFFFF;
		int spd;
		
		switch(act)
		{
			case 0:	//OFF
				lmMotor.offBldc();
				break;
			
			case 1:		//CW
			case 2:		//CCW
				duty = msg.substring(4,(4+3)).toInt();
				if(  0 > duty) { duty = 0; }
				if(255 < duty) { duty = 255; }
				onTime = msg.substring(7,(7+4)).toInt();
				if(0 > onTime ) { onTime = 0; } 	// Not STOP

				lmMotor.onBldc((act-1)? BBN_MOT_CCW:BBN_MOT_CW, duty);
				if(onTime) { lmMotor_OffTime_Start(onTime); }
				
				break;

				// TBD-LM CMD처리
			case 3:		// CW-거리
			case 4:		// CCW 거리
				break;

			case 5:		// Action
				actCmd = msg.substring(4,(4+1)).toInt();
				//if(0 > kind || 4 < kind) { break; }
				//else
				{
					String paraStr = msg.substring(5,(5+1));
					if(NULL == paraStr)
					{
						LogPrintln(" lg] ParaNg NULL");
					}
					else
					{ 
						para = paraStr.toInt(); 
					}
					switch(actCmd)
					{
						case 0:		// Action ALL OFF
							lmHome_Control_Stop();		// Home
							lmLeft_Control_Stop();		// Left
							lmRight_Control_Stop();		// Right
							lmReturn_Control_Stop();	// Return
							// Motor OFF
							lmMotor.offBldc();
							bbnMotor.offBldc();
							torqOff();
							//
							lmPriority = LM_PRIORITY_NO;
							lmLeftStartReq = 0;
							lmRightStartReq = 0;
							//
							if(0) {}
							else if(LM_POSI_RIGHT <= lmPosi) { lmRight_Control_Return(); }
							else if(LM_POSI_LEFT <= lmPosi) { lmLeft_Control_Return(); }
							//else {lmHome_Control_Return(); }
							break;

						case 1:	// HOME  not-Start / 0-STOP
							//if(0 == para || 0xFFFFFFFF == para) { lmHome_Control_Stop(); }
							//else { lmHome_Control_Start(END_TM_DUTY_OFF); }		// Home Control
							lmHome_Control_Start(1, END_TM_DUTY_OFF);
							break;
							
						case 2:		// LEFT 구동요구
							if(0 == para) { lmLeft_Control_Return(); }
							else
							{
								lmRight_Control_Stop();	// Right STOP
								fish = para;
								pwr = msg.substring(6).toInt();
								if(0 == pwr) { pwr = DEFAULT_LM_PWR; }
								//spd = (pwr * DUTY2SPD_RATE);
								lmLeft_Control_Start(fish, pwr, reqTorqueMotor);		// Left	// TBD-LM, Fishi/Power형태
							}
							break;
						
						case 3:	// RIGHT 구동 요구
							if(0 == para) { lmRight_Control_Return(); }
							else
							{
								lmLeft_Control_Stop();	// LEFT STOP
								fish = para;
								pwr = msg.substring(6).toInt();
								if(0 == pwr) { pwr = DEFAULT_LM_PWR; }
								//spd = (pwr * DUTY2SPD_RATE);
								lmRight_Control_Start(fish, pwr, reqTorqueMotor);		// Right	// TBD-LM, Fishi/Power형태
							}
							break;
						
						case 4:	// Return
							para = msg.substring(5,(5+3)).toInt();		// para RE-SET
							if(0 == para) { lmReturn_Control_Stop(); }
							else
							{
								pwr = para;
								lmReturn_Control_Start(pwr);	// Return	// TBD-LM, Fishi/Power형태
								break;
							}

						default:
							// Error RESP
							break;
					}
				}
				break;
				
			default:
				// Error
 				break;
		}
		// Range Check
	#if RESP_IO_LMMOT
		if(AP_LM_CMD_MAX > act)
		{
			Resp2ApPrintln(msg+"%");	// Resp at Valid CMD only
		}
		else
		{
			Resp2ApPrintln(msg+"_ER%");	// Error
		}
	#endif

	#if LOG_LMMOT_AP_CMD
		LogPrintln(" AP] LMMOT " + msg);
	#endif

#endif
}



// TBD-LM
void lmHome_Control_Start(int _init, int tmReqDuty)
{
	endTmReqDuty = tmReqDuty;
	lmHomeStep = STEP_START;
	lmHome_Init = _init;
	lmHome_Flag = 1;
}
void lmHome_Control_Stop()
{
	lmHomeStep = STEP_IDLE;
	lmHome_Flag = 0;
}

// 화면 -우 =>, 구동 -좌측
void lmLeft_Control_Start(int fish, int pwr, int ReqTm)
{
	lmFishLevel = fish;
	//lmLeftDuty = (spd / DUTY2SPD_RATE);
	lmLeftDuty = (pwr);
	leftEndTmReqDuty = ReqTm;
	lmLeftStep = STEP_START;
	lmLeft_Flag = 1;
	if(LM_PRIORITY_NO == lmPriority) { lmPriority = LM_PRIORITY_LEFT; }
}
void lmLeft_Control_Return()
{
	lmLeftStep = LM_LEFT_HOME_RETURN;
	lmLeft_Flag = 1;
}
void lmLeft_Control_Stop()
{
	lmLeftStep = STEP_IDLE;
	lmLeft_Flag = 0;
}


// 화면 -좌, 구동 -우측
void lmRight_Control_Start(int fish, int pwr, int ReqTm)
{
	lmFishLevel = fish;
	//lmRightDuty = (spd / DUTY2SPD_RATE);
	lmRightDuty = (pwr);
	rightEndTmReqDuty = ReqTm;
	lmRightStep = STEP_START;
	lmRight_Flag = 1;
	if(LM_PRIORITY_NO == lmPriority) { lmPriority = LM_PRIORITY_RIGHT; }
}
void lmRight_Control_Return()
{
	lmRightStep = LM_RIGHT_HOME_RETURN;
	lmRight_Flag = 1;
}

void lmRight_Control_Stop()
{
	lmRightStep = STEP_IDLE;
	lmRight_Flag = 0;
}

// TBD-LM, CENTER CONTROL
void lmCenter_Control_Start(int fish, int pwr)
{
#if 0
	lmFishLevel = fish;
	//lmRightDuty = (spd / DUTY2SPD_RATE);
	lmRightDuty = (pwr);
	lmRightStep = STEP_START;
	lmRight_Flag = 1;
#endif
}
void lmCenter_Control_Stop()
{
#if 0
	lmRightStep = STEP_IDLE;
	lmRight_Flag = 0;
#endif
}


void lmReturn_Control_Start(int pwr)
{
#if 0
	//lmReturnDuty = (pwr);
	//lmReturnStep = STEP_START;
	//lmReturn_Flag = 1;
#endif
	stLmReturnTbl.lm = pwr;		// Return LM Duty SET
	if(0) {}
	else if(LM_POSI_RIGHT <= lmPosi) { lmRightStep = LM_RIGHT_HOME_RETURN; }
	else if(LM_POSI_LEFT <= lmPosi) { lmLeftStep = LM_LEFT_HOME_RETURN; }
	else { lmHome_Control_Start(0, reqTorqueMotor); }		
}
void lmReturn_Control_Stop()
{
	lmReturnStep = STEP_IDLE;
	lmReturn_Flag = 0;
}

void lmHoldOn_Stop()
{
	lmHold_Stop_Flag = 1;		// AP 버티기 종료 CMD수신
}


// TBD-LM, NOT-USE
void lmHome_Control_2()		//_ak : Action Kind
{
	static unsigned long _to;
	static unsigned int lmMoveDir = 0;
	static unsigned int lmHomeNextStep = 0;

	int lmDuty=0;
	int bmDuty=0;
	int tmDuty=0;

#if (0)
		static int old_step=0;
		if(old_step != lmHomeStep)
		{
			LogPrintln(" lg] step: " + String(old_step) +"->" + String(lmHomeStep));
			old_step = lmHomeStep;
		}
#endif
	
	switch(lmHomeStep)
	{
		case STEP_IDLE:	// IDLE
			break;
			
		case STEP_START:	// START (10)
			lmHomeRptCnt = 0;
			setTO(_to);
			lmHomeStep = 11;
			break;
			
		//상태 Check
		case 11:
			//Sensor Check
			if(LM_HOME_SEN_ON == lev10_lmHome)
			{
				lmHomeStep = 90;	// OK END
			}
			else
			{
				lmMoveDir = LM_MOT_CW;		// Move L
				if (LM_LEFT_SEN_ON == lev10_lmLeft)	//
				{
					lmMoveDir = LM_MOT_CCW;		// Move R
				}
			  #if 0
				else if (LM_RIGHT_SEN_ON == lev10_lmRight)
				{
					lmMoveDir = LM_MOT_CW;	// Move LEFT
				}
				else
				{
				}
			  #endif
				lmHomeStep = 12;
			}
			break;
			
		case 12:
			lmDuty = LM_HOME_MOVE_DUTY;		// LM_HOME_MOVE_DUTY
			lmMotor.onBldc(lmMoveDir, lmDuty);			// LM OFF
			setTO(_to);
			lmHomeStep = 13;
			break;

		case 13:
			if(checkTO(_to, LM_HOME_TO_TIME))	// TO 처리, 5SEC
			{
				//Fail
				lmHomeStep = 80;	// ERROR
			}
			else if (LM_HOME_SEN_ON == lev10_lmHome)
			{
				lmMotor.onBldc(LM_MOT_CW, 0);	// OFF
				lmHomeStep = 19;
				lmHomeNextStep = 20;	// Wire Home
			}
			else if (LM_LEFT_SEN_ON == lev10_lmLeft)
			{
				lmMotor.onBldc(LM_MOT_CW, 0);	// OFF
				lmHomeStep = 19;
				lmHomeNextStep = 11;	// Sensor Check
			}
			else if (LM_RIGHT_SEN_ON == lev10_lmRight)
			{
				// Error
				//lmHomeStep = 80;	// ERROR

				lmMotor.onBldc(LM_MOT_CW, 0);	// OFF
				lmHomeStep = 19;
				lmHomeNextStep = 11;	// Sensor Check
			}
			setTO(_to);			
			break;

		case 19:
			if(checkTO(_to, LM_HOME_DIR_CHANGE_TIME))	// TO 처리
			{
				lmHomeStep = lmHomeNextStep;
			}
			break;
			
		case 20:	// TIMEOUT Check
			//lmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].lm;
			//bmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
			//tmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
			lmDuty = stLmHomeTbl.lm;		// LM DUTY=0 : OFF
			bmDuty = stLmHomeTbl.bm;
			tmDuty = stLmHomeTbl.tm;
			
			lmMotor.onBldc(LM_MOT_CW, lmDuty);			// LM OFF
			bbnMotor.onBldc(BBN_MOT_CW, bmDuty);
			torqMotor.on(tmDuty);

			lmHomeRptCnt++;
			
			setTO(_to);
			lmHomeStep = 30;
			break;
			
		case 30:
			if(checkTO(_to, LM_HOME_TO_TIME))	// TO 처리
			{
				setTO(_to);
				if( LM_HOME_RETRY_CNT < lmHomeRptCnt)	{ lmHomeStep = 80; }	// goto ERROR_END
				else {	lmHomeStep = 20; }	// Retry
			}
			else if(LM_HOME_SEN_ON == lev10_lmHome)	// SENOR ON
			{
				setTO(_to);
				lmHomeStep = 40;	// goto STOP
			}
			break;
			
		case 40:	// MOT OFF WAIT
			if(checkTO(_to, nvm01_home_stopWait_time))	// NVM시간
			{
				lmHomeStep = 90;	//
			}
			break;

		case 50:	// DUMMY
			break;

		case 80:	// DUMMY
			lmHomeNG_Flag = 1;
			break;
			

		case 90:	// END
			lmMotor.onBldc(LM_MOT_CW, 0);
			bbnMotor.onBldc(BBN_MOT_CW, 0);
			//torqMotor.on(0);
			torqMotor.on(endTmReqDuty);		// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		

			// Flage CLR & VAR Clear
			lmHomeRptCnt = 0;
			lmHome_Control_Stop();

			lmPosition_Set(LM_POSI_HOME);
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}


#define STEP_OK_END 99
#define STEP_NG_END 98
//
//
//	CALL 10 ms
//
void lmHome_Control()		//_ak : Action Kind
{
#if (IO_LM_MOT)
	static unsigned long _to;

	int lmDuty=0;
	int bmDuty=0;
	int tmDuty=0;

	String logMsg;

#if (0)
	static int old_step=0;
	if(old_step != lmHomeStep)
	{
		LogPrintln(" lg] step: " + String(old_step) +"->" + String(lmHomeStep));
		old_step = lmHomeStep;
	}
#endif

	switch(lmHomeStep)
	{
		case STEP_IDLE:	// IDLE
			break;
			
		case STEP_START:	// START (10)
			// 값 초기화
			lmHomeNG_Flag = 0;
			lmHomeRptCnt = 0;
			lmHomeStep = 11;
			break;

			//상태 Check
		case 11:
			//Sensor Check
			if(0) {}
			else if(LM_HOME_SEN_ON == lev10_lmHome)		// HOME
			{
				if(lmHome_Init) { lmHomeStep = 12; }			// LM MOVE
				else            { lmHomeStep = STEP_OK_END; }	// LM NOT MOVE = OK END
			}
			else if (LM_RIGHT_SEN_ON == lev10_lmRight)	// LIMIT RIGHT
			{
				lmHomeStep = 70;	// MOVE-LEFT	
			}
			else
			{
				lmHomeStep = 20;	// MOVE _RIGHT
				//
				
			  #if 0
				lmMoveDir = LM_MOT_CW;		// Move L
				if (LM_LEFT_SEN_ON == lev10_lmLeft)	//
				{
					lmMoveDir = LM_MOT_CCW;		// Move R
				}
				else if (LM_RIGHT_SEN_ON == lev10_lmRight)
				{
					lmMoveDir = LM_MOT_CW;	// Move LEFT
				}
				else
				{
				}
			  #endif
			}
			break;
			

		// at HOME POSI - LM ONLY MOVE
		case 12:
			lmMotor.onBldc(LM_MOT_RIGHT, 20);			// LM  R
			setTO(_to);
			lmHomeStep = 13;			
			break;

		case 13:
			if(LM_HOME_SEN_OFF == lev10_lmHome)	// HOME OFF
			{
				setTO(_to);
				lmHomeStep = 14;	//
			}
			else if(checkTO(_to, 500))	// TO 처리 = HOME OFF ERROR
			{
				lmHomeStep = STEP_NG_END;	//ERROR
			}
			break;

		case 14:
			if(checkTO(_to, 10))	//추가 이동
			{
				lmMotor.offBldc(); 		// OFF
				setTO(_to);
				lmHomeStep = 15;	//ERROR
			}
			break;
			
		case 15:
			if(checkTO(_to, 30))	// 방향전환 대기시간
			{
				setTO(_to);
				lmHomeStep = 70;	//MOVE_LEFT
			}
			break;

		// HOME MOVE at OTHER CONDITION
		case 20:	// TIMEOUT Check
			//lmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].lm;
			//bmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
			//tmDuty = stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
			lmDuty = stLmHomeTbl.lm;
			bmDuty = stLmHomeTbl.bm;
			tmDuty = stLmHomeTbl.tm;
			
			lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);
			bbnMotor.onBldc(BBN_MOT_CW, bmDuty);
			torqMotor.on(tmDuty);

			lmHomeRptCnt++;
			
			setTO(_to);
			lmHomeStep = 30;
			break;
			
		case 30:
			if(0) {}
			else if(LM_HOME_SEN_ON == lev10_lmHome)	// HOME
			{
				setTO(_to);
				lmHomeStep = STEP_OK_END;	// goto STOP
			}
			else if(LM_RIGHT_SEN_ON == lev10_lmRight)	// RIGHT
			{
				// STOP
				lmMotor.offBldc();
				bbnMotor.offBldc();
				torqOff();
				//
				setTO(_to);
				lmHomeStep = 69;	// LEFT-MOVE
			}
			else if(checkTO(_to, LM_HOME_TO_TIME))	// TO 처리
			{
				if( LM_HOME_RETRY_CNT < lmHomeRptCnt)
				{ 
					lmHomeStep = STEP_NG_END;
				}	// goto ERROR_END
				else
				{
					lmHomeStep = 20;	// REPEAT
				}	// Retry
			}
			break;
			
		case 69:	// MOT OFF WAIT
			if(checkTO(_to, 30))	// 방향 전환 대기 30 ms
			{
				lmHomeStep = 70;	//
			}
			break;

		case 70:	// HOME MOVE -LEFT MOVE
			lmDuty = stLmHomeTbl.lm;
			bmDuty = stLmHomeTbl.bm;
			tmDuty = stLmHomeTbl.tm;			
			lmMotor.onBldc(LM_MOT_LEFT, lmDuty);
			bbnMotor.onBldc(BBN_MOT_CW, bmDuty);
			torqMotor.on(tmDuty);

			setTO(_to);
			lmHomeStep = 71;
			break;
			
		case 71:
			if(LM_HOME_SEN_ON == lev10_lmHome)
			{
				setTO(_to);
				lmHomeStep = 72;	//
			}
			else if(checkTO(_to, LM_MOVE_ERR_TO_TIME))
			{
				lmHomeStep = STEP_NG_END;	//				
			}
			break;

		case 72:
			if(checkTO(_to, nvm01_home_stopWait_time))	// NVM시간
			{				
				lmHomeStep = STEP_OK_END;	//
			}
			break;

		case STEP_NG_END:	// 98
			lmHomeNG_Flag = 1;
			lmHomeStep = STEP_OK_END;	//
			break;
			

		case STEP_OK_END:	// 99
			// 1) MOT OFF
			lmMotor.onBldc(LM_MOT_CW, 0);
			bbnMotor.onBldc(BBN_MOT_CW, 0);
			//torqMotor.on(0);
			if(lmHome_Init)
			{
				lmHome_Init = 0;
				torqMotor.on(reqTorqueMotor);
			}
			else
			{
				torqMotor.on(endTmReqDuty);
			}		// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		

			// 2) Flage CLR & VAR Clear
			lmHomeRptCnt = 0;
			lmHome_Control_Stop();

			// 3) ERR & SET POSI
			if(!lmHomeNG_Flag)
			{
				lmPosition_Set(LM_POSI_HOME);
				logMsg = " lg] LmHome OK";
			}
			else
			{
				lmHomeNG_Flag = 0;
				logMsg = " lg] LmHome NG";
			}
		  #if (LOG_LM_HOME_END)
			LogPrintln(logMsg);
		  #endif
		  
		  	// LM Priority SET, Home종료시 우선도 CLR
		  	lmPriority = LM_PRIORITY_NO;
			break;
			
		default:
			// ERROR (unknown Step)
			break;
	}
#endif
}

void lmPosition_Set(int posi)
{
	lmPosi = posi;
	// TBD-LM
	//센서가 변경되는 경우 자동 Home복귀 처리는?
}

//#define SEN_REPEAT_TIME 200	//50
#define SEN_REPEAT_TIME 300 // 200	//50

//#define LEFT_MOVE_PULSE	20	// 46
#define LEFT_MOVE_PULSE	 46
uint16_t nvm01_lm_return_stop = 1;
unsigned int lmLeftPulseCnt = LEFT_MOVE_PULSE;

// TBD-LM
// CONTROL - 좌측(CW)
void lmLeft_Control()
{
#if (IO_LM_MOT)
		static unsigned long _to;
	
		int lmDuty=0;
		int bmDuty=0;
		int tmDuty=0;
	
		String logMsg;
	
#if (LOG_LM_LEFT_STEP)
		static int old_step=0;
		if(old_step != lmLeftStep)
		{
			LogPrintln(" lg] step: " + String(old_step) +"->" + String(lmLeftStep));
			old_step = lmLeftStep;
		}
#endif
	
		switch(lmLeftStep)
		{
			case STEP_IDLE: // IDLE
				break;
				
			case STEP_START:	// START (10)
				lmPosition_Set(LM_POSI_LEFTMOVE);
				setTO(_to);
				lmLeftStep = 11;
				break;
			case 11:
				if(checkTO(_to,LM_START_WAIT_TIME))	// 300ms WAIT
				{
					lmLeftNG_Flag = 0;
					lmLeftPulseCnt = 0;
					//상태Check
					if(LM_HOME_SEN_ON == lev10_lmHome)
					{
						
					}
					lmLeftStep = 20;					
				}
		
			case 20:
				// LM-b우측, TM-a당김
				lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_b].lm;
				tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].tm;
				lmMotor.onBldc(LM_MOT_LEFT, lmDuty);
				torqMotor.on(tmDuty);
				//-- Count Start, TBD
				//lmLeftPulseCnt = lmEnc.lmCnt;
				setLmEnc(lmLeftPulseCnt);
				setTO(_to);
				lmLeftStep = 30;
				break;

			case 30:
				if(0) {}
				else if(checkLmEnc(lmLeftPulseCnt, LEFT_MOVE_PULSE))	// 46 Pulse WAIT
				{
					lmPosition_Set(LM_POSI_LEFT);
					lmLeftStep = 40;	// 	C-A반복
				}
				else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
				{
					lmLeftStep = 98;	// error
				}
				//else if(46 < (lmEnc.lmCnt - lmLeftPulseCnt))	// 46 Pulse WAIT
				else if(LM_LEFT_SEN_ON == lev10_lmLeft)	// 
				{
					lmPosition_Set(LM_POSI_LEFT);
					lmLeftStep = 60;	// Right 반복
				}
				break;

			// C - OUT
			case 40:
				lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lm;
				bmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].bm;
				tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].tm;
			
				lmMotor.onBldc(LM_MOT_LEFT, lmDuty);
				bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
				torqMotor.on(tmDuty);				
				setTO(_to);
				lmLeftStep = 41;	// 
				break;

			// WAIT TIME
			case 41:
				if(checkTO(_to, stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lmTime))	// LM TIME만 관리
				{
					setTO(_to);
					lmLeftStep = 42;	// 			
				}
				else if(LM_LEFT_SEN_ON == lev10_lmLeft)	// 
				{
					lmPosition_Set(LM_POSI_LEFT);
					lmLeftStep = 60;	// Right반복
				}
				break;

			// A - OUT
			case 42:
				lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].lm;
				bmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].bm;
				tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_b].tm;
			
				lmMotor.onBldc(LM_MOT_LEFT, lmDuty);
				bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
				torqMotor.on(tmDuty);				
				setTO(_to);
				lmLeftStep = 43;	// 
				break;

			case 43:
				if(checkTO(_to, stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].lmTime))	// LM TIME만 관리
				{
					setTO(_to);
					lmLeftStep = 40;	// 		goto C반복	
				}
				else if(LM_LEFT_SEN_ON == lev10_lmLeft)	// 
				{
					lmPosition_Set(LM_POSI_LEFT);
					lmLeftStep = 60;	// Right 반복 
				}
				break;

			// LEFT 반복, TBD
			case 60:
				lmMotor.offBldc();		// OFF
				setTO(_to);
				lmLeftStep = 61;
				break;
			case 61:
				if(0) {}
				else if(LM_LEFT_SEN_OFF == lev10_lmLeft)
				{
					setTO(_to);
					lmLeftStep = 62;
				}
				else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
				{
					lmLeftStep = 98;	// error
				}
				break;
			case 62:
				if(checkTO(_to, SEN_REPEAT_TIME))
				{
					lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lm;
					lmMotor.onBldc(LM_MOT_LEFT, lmDuty);
					setTO(_to);
					lmLeftStep = 63;
				}
				break;
				
			case 63:
				if(0) {}
				else if(LM_LEFT_SEN_ON == lev10_lmLeft)	// 
				{
					lmPosition_Set(LM_POSI_LEFT);
					lmLeftStep = 60;	// Right 반복 
				}
				else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
				{
					lmLeftStep = 98;	// error
				}
				break;

			// HOME RETURN, TBD
			case LM_LEFT_HOME_RETURN:
				lmDuty = stLmReturnTbl.lm;
				bmDuty = stLmReturnTbl.bm;
				tmDuty = stLmReturnTbl.tm;
				lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);		// 미사양 - KPS추가
				bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
				torqMotor.on(tmDuty);				
				lmPosition_Set(LM_POSI_HOMEMOVE);		// HOME
				setTO(_to);
				lmLeftStep = 81;	
				break;
				
			case 81:
				if(0) {}
				else if(LM_HOME_SEN_ON == lev10_lmHome)	// 
				{
					setTO(_to);
					lmLeftStep = 82;	// Right 반복
				}
				else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
				{
					lmLeftStep = 98;	// error
				}
				break;

			case 82:
				if(0) {}
				else if(checkTO(_to, nvm01_lm_return_stop))		// 
				{
					lmLeftStep = 99;
				}
				break;

			// FINISH ( ERROR )
			case 98:
				lmLeftNG_Flag = 1;
				lmLeftStep = 99;	// Error STOP 
				break;

			// FINISH ( OK )
			case 99:
				lmMotor.offBldc();		// OFF
				bbnMotor.offBldc();		// OFF
				//torqMotor.on(0);
				torqMotor.on(leftEndTmReqDuty);		// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		
				torqMotor.on(DEFAULT_TORQ);		// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		
				//torqMotor.on(reqTorqueMotor);		// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		

				// Flage CLR & VAR Clear
				lmLeft_Control_Stop();

				// Start_Wait Check
				lmPriority = LM_PRIORITY_NO;	// 우선도 CLEAR
				lmPosition_Set(LM_POSI_HOME);		// HOME
				
				if(lmRightStartReq)
				{
					lmRightStartReq = 0;
					lmRight_Control_Start(FISH_LVL_2, MOT_PWR_LVL_b, rightEndTmReqDuty); 
				}

				// LOG
				if(!lmLeftNG_Flag)
				{
					logMsg = " lg] LmLeft OK";
				}
				else
				{
					lmLeftNG_Flag = 0;
					logMsg = " lg] LmLeft NG";
				}
		  		#if (LOG_LM_HOME_END)
					LogPrintln(logMsg);
		  		#endif
			break;
				
			default:
				break;
		}

	//	버티기 종료때
	if(lmHold_Stop_Flag) // AP CMD 버티기 종료
	{
		lmHold_Stop_Flag = 0;
		lmLeftStep = LM_LEFT_HOME_RETURN;		// HOME RETURN
	}
#endif
}

//#define RIGHT_MOVE_PULSE		20	// 46
#define RIGHT_MOVE_PULSE		46
uint16_t nvm02_lm_return_stop = 1;
unsigned int lmRightPulseCnt = RIGHT_MOVE_PULSE;


void lmRight_Control()
{
#if (IO_LM_MOT)
			static unsigned long _to;
		
			int lmDuty=0;
			int bmDuty=0;
			int tmDuty=0;
		
			String logMsg;
		
#if (LOG_LM_RIGHT_STEP)
			static int old_step=0;
			if(old_step != lmRightStep)
			{
				LogPrintln(" lg] step: " + String(old_step) +"->" + String(lmRightStep));
				old_step = lmRightStep;
			}
#endif
		
			switch(lmRightStep)
			{
				case STEP_IDLE: // IDLE
					break;
					
				case STEP_START:	// START (10)
					setTO(_to);
					lmRightStep = 11;
					break;
				case 11:
					if(checkTO(_to,LM_START_WAIT_TIME))	// 300ms WAIT
					{
						lmRightNG_Flag = 0;
						lmRightPulseCnt = 0;
						//상태Check
						if(LM_HOME_SEN_ON == lev10_lmHome)
						{
							
						}
						lmRightStep = 20;					
					}
			
				case 20:
					// LM-b우측, TM-a당김
					lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_b].lm;
					tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].tm;
					lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);
					torqMotor.on(tmDuty);
					//-- Count Start, TBD
					//lmLeftPulseCnt = lmEnc.lmCnt;
					setLmEnc(lmRightPulseCnt);
					lmPosition_Set(LM_POSI_RIGHTMOVE);
					setTO(_to);
					lmRightStep = 30;
					break;
	
				case 30:
					if(0) {}
					//else if(46 < (lmEnc.lmCnt - lmLeftPulseCnt))	// 46 Pulse WAIT
					else if(checkLmEnc(lmRightPulseCnt, RIGHT_MOVE_PULSE))	// 46 Pulse WAIT
					{
						lmPosition_Set(LM_POSI_RIGHT);
						lmRightStep = 40;	//	C-A반복
					}
					else if(LM_RIGHT_SEN_ON == lev10_lmRight)	// LIMIT RIGHT
					{
						lmPosition_Set(LM_POSI_RIGHT);
						lmRightStep = 60;	// Right 반복
					}
					else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 3 SEC TBD
					{
						lmRightStep = 98;	// error
					}
					break;
	
				// C - OUT
				case 40:
					lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lm;
					bmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].bm;
					tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].tm;
				
					lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);
					bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
					torqMotor.on(tmDuty);				
					setTO(_to);
					lmRightStep = 41;	// 
					break;
	
				// WAIT TIME
				case 41:
					if(checkTO(_to, stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lmTime))	// LM TIME만 관리
					{
						setTO(_to);
						lmRightStep = 42;	//			
					}
					else if(LM_RIGHT_SEN_ON == lev10_lmRight)	// 
					{
						lmPosition_Set(LM_POSI_RIGHT);
						lmRightStep = 60;	// Right반복
					}
					break;
	
				// A - OUT
				case 42:
					lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].lm;
					bmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].bm;
					tmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_b].tm;
				
					lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);			// LM OFF
					bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
					torqMotor.on(tmDuty);				
					setTO(_to);
					lmRightStep = 43;	// 
					break;
	
				case 43:
					if(checkTO(_to, stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_a].lmTime))	// LM TIME만 관리
					{
						setTO(_to);
						lmRightStep = 40;	//		goto C반복	
					}
					else if(LM_RIGHT_SEN_ON == lev10_lmRight)	// 
					{
						lmPosition_Set(LM_POSI_RIGHT);
						lmRightStep = 60;	// Right 반복 
					}
					break;
	
				// RIGHT 반복, TBD
				case 60:
					lmMotor.offBldc();		// OFF
					setTO(_to);
					lmRightStep = 61;
					break;
				case 61:
					if(0) {}
					else if(LM_RIGHT_SEN_OFF == lev10_lmRight)
					{
						setTO(_to);
						lmRightStep = 62;
					}
					else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
					{
						lmRightStep = 98;	// error
					}
					break;
				case 62:
					if(checkTO(_to, SEN_REPEAT_TIME))
					{
						lmDuty = stLmDutyTbl[lmFishLevel][MOT_PWR_LVL_c].lm;
						lmMotor.onBldc(LM_MOT_RIGHT, lmDuty);
						setTO(_to);
						lmRightStep = 63;
					}
					break;
					
				case 63:
					if(0) {}
					else if(LM_RIGHT_SEN_ON == lev10_lmRight)	// 
					{
						lmPosition_Set(LM_POSI_RIGHT);
						lmRightStep = 60;	// Right 반복 
					}
					else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
					{
						lmRightStep = 98;	// error
					}
					break;
	
				// HOME RETURN, TBD
				case LM_RIGHT_HOME_RETURN:
					lmDuty = stLmReturnTbl.lm;
					bmDuty = stLmReturnTbl.bm;
					tmDuty = stLmReturnTbl.tm;
					lmMotor.onBldc(LM_MOT_LEFT, lmDuty);		// 미사양 - KPS추가
					bbnMotor.onBldc(BBN_MOT_PULL, bmDuty);
					torqMotor.on(tmDuty);				
					setTO(_to);
					lmRightStep = 81;	
					break;
					
				case 81:
					if(0) {}
					else if(LM_HOME_SEN_ON == lev10_lmHome) // 
					{
						setTO(_to);
						lmRightStep = 82;	// Right 반복
					}
					else if(checkTO(_to, LM_MOVE_TO_TIME))		// TO 4 SEC TBD
					{
						lmRightStep = 98;	// error
					}
					break;
	
				case 82:
					if(0) {}
					else if(checkTO(_to, nvm02_lm_return_stop)) 	// 
					{
						lmPosition_Set(LM_POSI_HOME);		// HOME
						lmRightStep = 99;
					}
					break;
	
				// FINISH ( ERROR )
				case 98:
					lmRightNG_Flag = 1;
					lmRightStep = 99;	// Error STOP 
					break;
	
				// FINISH ( OK )
				case 99:
					lmMotor.offBldc();		// OFF
					bbnMotor.offBldc(); 	// OFF
					//torqMotor.on(0);
					//torqMotor.on(rightEndTmReqDuty); 	// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		
					torqMotor.on(DEFAULT_TORQ); 	// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		
					//torqMotor.on(reqTorqueMotor); 	// Start시 요구된 Duty값 설정, PwrOn=0, WireReady=60, 그외는 AP요구값		
	
					// Flage CLR & VAR Clear
					lmRight_Control_Stop();
					
					// Start_Wait Check
					lmPriority = LM_PRIORITY_NO;	// 우선도 CLEAR
					lmPosition_Set(LM_POSI_HOMEMOVE);		// HOME

					if(lmLeftStartReq)
					{
						lmLeftStartReq = 0;
						lmLeft_Control_Start(FISH_LVL_2, MOT_PWR_LVL_b, leftEndTmReqDuty); 
					}

					// LOG
					if(!lmRightNG_Flag)
					{
						logMsg = " lg] LmRight OK";
					}
					else
					{
						lmRightNG_Flag = 0;
						logMsg = " lg] LmRight NG";
					}
		  		#if (LOG_LM_HOME_END)
						LogPrintln(logMsg);
		  		#endif
				break;
					
				default:
					break;
			}
	
		//	버티기 종료때
		if(lmHold_Stop_Flag) // AP CMD 버티기 종료
		{
			lmHold_Stop_Flag = 0;
			lmRightStep = LM_RIGHT_HOME_RETURN;		// HOME RETURN
		}
#endif	
}

void lmReturn_Control()
{
#if (IO_LM_MOT)
	static unsigned long _to;
		
	int lmDuty=0;
	int bmDuty=0;
	int tmDuty=0;
			
	String logMsg;
			
  #if (LOG_LM_RETURN_STEP)
	static int old_step=0;
	if(old_step != lmRightStep)
	{
		LogPrintln(" lg] step: " + String(old_step) +"->" + String(lmRightStep));
		old_step = lmRightStep;
	}
  #endif

  #if 0
	switch(lmRightStep)
	{
		case STEP_IDLE: // IDLE
			break;
						
		case STEP_START:	// START (10)
			setTO(_to);
			lmRightStep = 11;
			break;
		case 11:
			break;

		default:
			break;
	}
  #endif
#endif
}


 
//========================================
//
//
void anaMotOutRateSet(String msg)
{
	unsigned int act;
	unsigned int para1;
	unsigned int para2;

	act = msg.substring(3,(3+1)).toInt();
	if(0 == act )	// REL
	{
		//para1 = 0;
		//para2 = 0;
		para1 = 20;		// 100% SET
		para2 = 20;		// 100% SET
	}
	else			// TEST
	{
		para1 = msg.substring(4,(4+2)).toInt();
		if(20 < para1 ) { para1 = 20; }
		para2 = msg.substring(6,(6+2)).toInt();
		if(20 < para2 ) { para2 = 20; }		
	}

	motTestAct = act;	// 0 - Rel, 1- Test
	motTestMainIdx = para1; // 1 ~ 20
	motTestBbnIdx = para2;	// 1 ~ 20

  #if (LOG_MKT_TEST_23)
	//LogPrintln(" LG] MktT2 Test:" + String(motTestAct) +" Mi:"+String(motTestMainIdx) +" Bi:"+String(motTestBbnIdx));
	LogPrintln(" LG] MktT3 Test:" + String(motTestAct) +" Mi:"+String(motTestMainIdx) +" Bi:"+String(motTestBbnIdx));
  #endif
}


/*  =====================================
              Slave Check Timer, 
              If Only MAIN ENB , 
             CALL 500ms => 1 SEC
  ===================================== */

void checkRodTimer()
{

	String rodConnMsg = "-1";
	String imuConnMsg = "-1";
	String batMsg = "-1";


  	//=== 1) Slave NOT SLEEP REQ = Sleep Check, Onlky GAME_ENB 
  	// TBD, 불필요?
  	// Enable일 경우만 Slave Alive Check 보냄. 
 	if (isGameEnable)		  // if Main Enable
	{
	}
	else
	{
	}


	//=== 3) CHANGE ROD CONN, Send Only Change, Include SLEEP
	// 릴 연결 상태 변화 처리
	if (old_rod_conn_status != rod_conn_status)
	{
		//rod_conn_status = (new_rod_conn_status & ROD_STATUS_MASK) ;
		//rod_conn_status_reason = (new_rod_conn_status & ROD_DISCONN_REASON_MASK);

		//1) Rod 연결상태 변화 통지.
		if (ROD_CONN == rod_conn_status)		//모름,끊김 =>  ROD CONN
		{
			//sndMsg = RESP_STX_ROD_CONN + "11111" + battLvlStr + "%";		// SLAVE CONN, with BAT Level
			rodConnMsg = RESP_STX_ROD_CONN + STATE_CONNECTED;

		}
		else	// DISCONN ? or UNKNOWN			// ? / ? => 모름 혹은 끊김
		{
			rodConnMsg = RESP_STX_ROD_CONN + STATE_DISCONN;
		  
			// ROD연결 끊김시 배터리 "-1" 설정
			//ibatteryLvl = UNKNOWN_2;
			//battLvlStr = "-1";
		}
		rodConnMsg += "%";
		StsSendPrintln(rodConnMsg); 		// Send to PC : ROD_CONN
		LogPrintln(" LG] SLCN3 " + rodConnMsg + ", o,c:" + String(old_rod_conn_status) + "," + String(rod_conn_status));	  // LOG-SERIAL1

		// 2) Rod연결 상태 변화 , BAT LEVEL SEND at ROD CHANGE 
		if (ROD_CONN == rod_conn_status)		// => 연결로 변화, ROD CONN
		{
			batMsg = STX_BAT_LVL + battLvlStr + "%";	// 연결시 현재 잔량값(0~99) 통지
			StsSendPrintln(batMsg); 	// Send to PC : BAT LEVEL
		}
		else									// => 모름,끊김으로 변화
		{
			batMsg = STX_BAT_LVL + "-2%";				// 연결끊김 혹은 모름시 "-2" 통지
			StsSendPrintln(batMsg); 	// Send to PC : BAT LEVEL
		}
		LogPrintln(" LG] RODCN bat:" + batMsg);

		// ROD연결 변화시, 무조건 배터리 충전 : 변화없음 통지
		batMsg = STX_BAT_LVL + BAT_STR_CHAGER_NOCHANGE;
		StsSendPrintln(batMsg); // SEND to PC		

		old_rod_conn_status = rod_conn_status;	// Save Change Status
	}

	// === 4) NOT CHANGE BATT LEVEL
	if(oldibatteryLvl != ibatteryLvl)
	{
		String logStr;

		// ROD상태 변경, 배터리 값 변경 순서에 주의할 것
		if (ROD_UNKNOWN == rod_conn_status )
		{		
			batMsg = STX_BAT_LVL + "-1%";
			StsSendPrintln(batMsg); 	// Send to PC : BAT LEVEL
			logStr = " LG] ROD UNKNOWN";
			
			// 배터리 변화, ROD연결모름시, 변화없음 통지(POWER ON처리)
			batMsg = STX_BAT_LVL + BAT_STR_CHAGER_NOCHANGE;
			StsSendPrintln(batMsg); // SEND to PC		

		}
		else		// 연결 혹은 끊김 상태때.
		{
			//rodConnMsg = RESP_STX_ROD_CONN + "11111" + battLvlStr + "%";	// SLAVE CONN, with BAT Level
			batMsg = STX_BAT_LVL + battLvlStr + "%";
			StsSendPrintln(batMsg); 	// Send to PC : BAT LEVEL
			logStr = " LG] RODCN";

			// 배터리 변화, ROD연결끊김씨, 변화없음 통지(POWER ON처리)
			if(ROD_DISCONN == rod_conn_status) // 릴 연결시
			{
				batMsg = STX_BAT_LVL + BAT_STR_CHAGER_NOCHANGE;
				StsSendPrintln(batMsg); // SEND to PC
			}

			// 배터리 변화 , ROD연결중 만,  충전기 상태 : 충전(+), 사용(-) 판단 및 AP통지
			else if(ROD_CONN == rod_conn_status)	// 릴 연결시
			{
				//if((UNKNOWN != oldibatteryLvl) && ((oldibatteryLvl+1) < ibatteryLvl))	// + : 충전중 (+ 2이상 변화)
				if((UNKNOWN != ibatChargeLvl) && ((ibatChargeLvl+3) < ibatteryLvl))	// + : 충전중 (+ 4이상 변화)
				//if((oldibatteryLvl+2) < ibatteryLvl)	// + : 충전중 (+ 3이상 변화)
				{
					ibatChargeLvl = ibatteryLvl;		//충전표시용 잔량값 갱신
					//respMsg = STX_BAT_LVL + "-4%";	// 2 char
					batMsg = STX_BAT_LVL + BAT_STR_CHAGER_CHANGING;
					StsSendPrintln(batMsg);	// SEND to PC
				}
				//else if((UNKNOWN != oldibatteryLvl) && ((oldibatteryLvl-1) > ibatteryLvl))	// - 사용중 (-2이상 변화)
				else if((UNKNOWN != ibatChargeLvl) && ((ibatChargeLvl-1) > ibatteryLvl))	// - 사용중 (-2이상 변화)
				//else if((oldibatteryLvl-2) > ibatteryLvl)	// - 사용중 (-3이상 변화)
				{
					ibatChargeLvl = ibatteryLvl;		//충전표시용 잔량값 갱신
					//respMsg = STX_BAT_LVL + "-5%";	// 2 char
					batMsg = STX_BAT_LVL + BAT_STR_CHAGER_USE;
					StsSendPrintln(batMsg);	// SEND to PC
				}
			}	

		}

		#if (LOG_BAT_CHAGE)		
			LogPrintln(logStr +  " bat o/c:" + String(oldibatteryLvl)+","+String(ibatteryLvl) + ",RodSts=" + String(rod_conn_status));		// Slave Status LOG
			LogPrintln(" lg] BATST Charging: " + batMsg);
		#else
			LogPrintln(logStr +  ",bat o/c/c:" + String(oldibatteryLvl)+","+String(ibatteryLvl)+","+String(ibatChargeLvl));
		#endif
		oldibatteryLvl = ibatteryLvl;
		if(UNKNOWN == ibatChargeLvl) { ibatChargeLvl = ibatteryLvl; }		//충전표시용 잔량값 갱신
	}


	//== 5) Send IMU
	if(oldimu_conn_status != imu_conn_status)
	{
		imuConnMsg = RESP_STX_IMU_CONNECT;
		if(CONNECT == imu_conn_status) { imuConnMsg +=  STATE_CONNECTED; }
		else							{ imuConnMsg +=  STATE_DISCONN;   }
		imuConnMsg += "%";

		StsSendPrintln(imuConnMsg); 		// Send to PC : IMU_CONN
		// save old status
		oldimu_conn_status = imu_conn_status;
	}
	
}


void set_rod_conn_status(unsigned int sts)
{
	//rod_conn_status = (sts + reason);
	rod_conn_status = (sts);
}

//===========NOT USE============
/*-----------------------------------------------------------------
	Operation Condition Check
	: NOT USE
------------------------------------------------------------------*/
void condition_fw_check_10ms()
{

	// Condition Check
	// 1) Casting : Right Button 400ms ON
	if( 1 == cntRightButtonOn && 0 == cntRightButtonOff)
	{
		rightBtnOnCount++;
	}
	else if (0 == cntRightButtonOn && 1 == cntRightButtonOff )
	{
		if(40 < rightBtnOnCount)  // Upto 400ms
		{
			exeWave = 1;
			gameStatus = GAME_WAVE;
			waveType_TorqSet();
			
			LogPrintln(" AP] ExCst Wave TgTorq: " + String(stWaveTorqVal.tgTorq));	  // LOG-SERIAL1
		}
		rightBtnOnCount = 0;
	}
	else
	{
		rightBtnOnCount = 0;
	}
	
	//3) Hit : Break Change ( 0 => Other Data(243 or 230))
	if( reqBreakMotor != oldBreakMotor )
	{
		//recvBreak = 0;	// flag Clear
		if( 0 == oldBreakMotor )		// Break Change ( 0 => Other Value)
		{
			// 3-1) HIT
		  if( GAME_BITE == gameStatus)			// if ONLY Game BITE
		  {
			exeHit = 1;
			gameStatus = GAME_HIT;
		
/*		
// HUD Follow
			// HIt Angel SET 
			hitServoInterval = (hitServoAngle * 2 * 25 / 10);		// interval = Angle * [ 2.5ms/Degree ]
			if(100 > hitServoInterval) { hitServoInterval = 100; }	// min = 100 ms
*/
			// Hit Fish Type SET & Targ Torq
			if( fishTypeFixENB)		// Fix ?
			{
				if (!torqResistFix)		// Not FIX = Auto Caculation
				{
					stHitTorqVal.targetTorq = HitType_TorqSet(fishTypeFix);
					if ( LEVEL_NORMAL == gameLevel)		// Level 2
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 12 / 10) + 28 );		// Level 2 Resist = Target*1.2 +28
					}
					else if ( LEVEL_HARD == gameLevel)	// Level 3
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 14 / 10) + 21 );		// Level 3 Resist = Target*1.4 +21
					}
					else						// Level 1
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq *  9 / 10) + 41 );		// Level 3 Resist = Target*0.9 +41
					}
					stHitTorqVal.midTorq = (stHitTorqVal.holdonTorq + stHitTorqVal.lowTorq)/2; 
				}
			}
			else						// Auto ?
			{
				// 6) Find Fish Type AUTO from Break Val
				if (!torqResistFix)		// Not FIX = Auto Caculation
				{
					// [NOT USE] FW Self Condition Check
					fishTypeAuto = fishKind_autoFromBreak(reqBreakMotor);		// by Break Val
					stHitTorqVal.targetTorq = HitType_TorqSet(fishTypeAuto); 
					if ( LEVEL_NORMAL == gameLevel)		// Level 2
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 12 / 10) + 28 );		// Level 2 Resist = Target*1.2 +28
					}
					else if ( LEVEL_HARD == gameLevel)	// Level 3
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 14 / 10) + 21 );		// Level 3 Resist = Target*1.4 +21
					}
					else						// Level 1
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq *  9 / 10) + 41 );		// Level 3 Resist = Target*0.9 +41
					}
					stHitTorqVal.midTorq = (stHitTorqVal.holdonTorq + stHitTorqVal.lowTorq)/2; 
				}
			}
			execHit_ResistRandomStart();						// Start Resist, After 3000 ms

			// Beak Control ONCE HIT
			execHit_SetBreakOnceStart();
			
			
			LogPrintln(" AP] Exe_Hiting, FishKind = " + String(fishTypeAuto));   // LOG-SERIAL1
			
			LogPrintln(" AP] Exe_Hiting, Torq_FIX: " + String(torqResistFix) + ", Torq Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
		  }
		}

		//4) Success or Fail : Break Change ( Other Data(243 or 230) => 0 )
		else if ( 0 == reqBreakMotor )
		{
			exeSuccessFail = 1;
			gameStatus = GAME_SUCCESS;
			
			LogPrintln(" AP] Exe_Success_or_Fail");   // LOG-SERIAL1
		}
		oldBreakMotor = reqBreakMotor;
	}
	
	// 2) bite( touch a bait ) : Servo Change (+02 => Other Data(+50 or +70))
	if( reqServoMotor != oldServoMotor)
	{
		if(2 == oldServoMotor || 0 == oldServoMotor)		// Servo Change +02 => Other Value, PoweOn 1st ( old is 0)
		{
			exeBite = 1;
			gameStatus = GAME_BITE;
			biteType_TorqSet(fishTypeFix);
			
			LogPrintln(" AP] Exe_Biting, Fish Kind: " + String(fishTypeFix));
			
			LogPrintln(" AP] Exe_Biting, DeepTorq: " + String(stBiteTorqVal.deepTorq) + " sh_Torq: " + String(stBiteTorqVal.shallowTorq));   // LOG-SERIAL1
		}
	
		//5) End of Success or Fail : Servo Change Other => +02
		else if ( 2 == reqServoMotor )	// GameOver
		{
			exeGameOver = 1; // Flag Clear
			gameStatus = GAME_WAIT;
			// End Process : All Exeute Flag CEAR
			exeWave = 0;
			exeBite = 0;
			exeHit = 0;
			exeResist = 0;
			exeHoldon = 0;
			exeRanding = 0;
			exeSuccessFail = 0;
			LogPrintln(" AP] Exe_Ending");   // LOG-SERIAL1
			
		}
	
		oldServoMotor = reqServoMotor;
	}

	// 7) HoldOn Condition
	if( reqImuMeasFlag != oldImuMeasFlag)
	{
		if(exeHit)			// Only Whike HITING
		{
			if(0 == oldImuMeasFlag)		// Change 0 => OTHER Val: START
			{
				exeHoldon = 1;		// 버티기 시작
				gameStatus = GAME_HOLDON;
				if (!torqResistFix)		// Not FIX = Auto Caculation
				{
					stHitTorqVal.targetTorq = HitType_TorqSet(fishTypeAuto); 
					if ( LEVEL_NORMAL == gameLevel)		// Level 2
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 12 / 10) + 28 );		// Level 2 Resist = Target*1.2 +28
					}
					else if ( LEVEL_HARD == gameLevel)	// Level 3
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq * 14 / 10) + 21 );		// Level 3 Resist = Target*1.4 +21
					}
					else						// Level 1
					{
						stHitTorqVal.holdonTorq = ((stHitTorqVal.targetTorq *  9 / 10) + 41 );		// Level 3 Resist = Target*0.9 +41
					}
					stHitTorqVal.midTorq = (stHitTorqVal.holdonTorq + stHitTorqVal.lowTorq)/2; 
				}
				
				LogPrintln(" AP] Exe_Hoding ON, Fish Kind: " + String(fishTypeFix));   // LOG-SERIAL1
				
				LogPrintln(" AP] Exe_Hoding ON, Torq_FIX: " + String(torqResistFix) + ", Torq Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
			}
			else if(0 == reqImuMeasFlag)	// Change : OtherVal => 0
			{
				exeHoldOff = 1;		// 버티기 종료
				
				LogPrintln(" AP] Exe_Hoding OFF");	 // LOG-SERIAL1
			}
		}
		oldImuMeasFlag = reqImuMeasFlag;
	}
}

//--------------------------------------------
//	NEW BITE & NEW HOLD for FW Control by AP PTN CMD
//-------------------------------------------
void apPatternReq_Bite_Hold()
{
	if(0)	{ }		// dummy
	
	else if(exeBite)
	//else if(bitePtn_Flag)
	{
		// 순서 PTN1 -> PTN2 -> PTN3
		if(0)	{}	// Dummy
		
		else if(exeBitePtn1flag)
		{
			execBite_ptn1_Control();
		}
		else if(exeBitePtn2flag)
		{
			execBite_ptn2_Control();
		}
		else if(exeBitePtn3flag)
		{
			execBite_ptn3_Control();
		}
	}
	else if(holdOnPtnAll_Flag)
	{
		if(0)	{}	// Dummy
		
		else if(holdOnPtn1_Flag)
		{
			execHoldon_ptn1_BldcControl();
		}
	}
	
}

// 손맛개선
// 향후 개선 , Flag말고 gameStatus로 변경 개선/상태천이도 그리고!!!
//  CALL 10ms
void testHand()
{
	if(0) { }	// dummy
	
	// 1] Wave =  CAST_COMP
	else if(exeWave)			// CAST_COMP
	{
		switch(waveControlCase)			// (0) NO Wave Control
		{
			case 0:
				exeWave = 0;	// NOT CONTROL
				break;
			case 1:
			  #if IO_SERVO
		  		execWave_servo();
			  #endif
				break;
			case 2:
				execWave_torque();
				break;
			case 3:
				execWave_bbn();
				break;
			case 4:
				execWave_bbn_Torq();	// not use , too Strong
				break;
			case 5:
				execWave_torq_bbn();	// use , Torq OFF -> bldc ON : Smooth
				break;
			default:
				// Error
				break;
				
		}
	}

	// 2]  Bite
	else if(exeBite)
	{
		// BITE Servo Control
		//execBite_servo();

		if(biteLogOut)
		{
			biteLogOut = 0;
			
			LogPrintln(" LG] BITE_ Control: " + String(biteTorqCase));
		}

		if ( 0 == biteTorqCase)
		{ 
			exeBite = 0;	// Not Control, then FLAG CLR
		}
		// Torq Control Case
		else if( 1 == biteTorqCase ) { execBite_torque_justTarget(); }
		else if( 2 == biteTorqCase ) { execBite_torque_incTarget(); }
		else if( 3 == biteTorqCase ) { execBite_torquewf_Control(biteCnt); }
    	else
		{
			// ERROR
		}
	}
	// Hit Area = [ Hit / Resist / Hold On / Finish ]
	else if(exeHit)
	{
		execHiting();
	}

/*
	else if(exeFight)
	{
		//execHiting();
	}

	else if(exeHoldon)
	{
		//execHiting();
	}
	else if(exeRanding)
	{
		execRanding_Control();		// Include Fight ?
	}
*/
	// Hit는 Success or Fail시 CLEAR되므로, 별도 판단문으로 처리.
	if(exeSuccessFail)
	{
		execSuccessFail();
	}

	else if(exeContinue)
	{

	}
	
	else if(exeGameOver)
	{
		//All Flage Clear
		clear_ExecFlag();
		// All Out Off
	    motor_AllDefault();

/*
		// TBD Break OFF ?
//		String str = "000";
//		if(10 > reqBreakMotor)        { str = "00"+ String(reqBreakMotor); }
//		else if (100 > reqBreakMotor) { str = "0"+ String(reqBreakMotor); }
//		else                         { str = String(reqBreakMotor); }
//		str = "$0500000" + str;
//		eNow.write(PID_ROD_CTRL_NUM, str);	  // Break
*/
		//exeGameOver Clear
		exeGameOver = 0;
	}

	if(exeProgramEnd)
	{
		clear_ExecFlag();
		motor_AllOff();			// Main ALL OFF
		reelOut_AllOff();		// Reel ALL OFF
		exeProgramEnd = 0;
	}
}


// Diag INPUT Monitor 
//	10ms CALL
void diagInputMonitor()
{
	String logStr = "";
	int i = 0;
	
	//if(diagMode)		// Diag Mode
	{
	  // 1) 센서 읽기
		// MAIN보드 Input
		//if(diagInFlag_) { sensor = digitalRead(); }
		
		//if(diagInFlag_ResetSwc) { sensorResetSwc = digitalRead(RESET_PIN); }
		if(diagInFlag_BootSwc) { sensorBootSwc = (digitalRead(BOOT_MODE_PIN)? 0:1); }	// 반전
		if(diagInFlag_PcUsb5V) { sensorUsb5V = (digitalRead(PC_USB5V_PIN)? 1:0); }
		//if(diagInFlag_WdtLbSig) { sensor = digitalRead(WDTFB_PIN); }
		//if(diagInFlag_SelectBoard) { sensorSelBoard = digitalRead(SEL_BOARD_PIN); }
		if(diagInFlag_PonSwc) { sensorPonSwc = (digitalRead(PWROFF_SWC_PIN)? 0:1); }	//반전
		//if(diagInFlag_Interlock) { sensorDoorIlk = digitalRead(ILKDOOR_PIN); }

		//if(diagInFlag_AngleHomeSns) { sensorAngHomeSns = digitalRead(ANG_HOMESNS_PIN); }

		if(diagInFlag_BbnBldcFG) { sensorBbnFG = (digitalRead(BLDC_FG_PIN)? 1:0); }

		if(diagInFlag_BobbinEncA) { sensorBbnEncA = (digitalRead(BBN_ENC_A_CK_PIN)? 1:0); }
		if(diagInFlag_BobbinEncB) { sensorBbnEncB = (digitalRead(BBN_ENC_B_DT_PIN)? 1:0); }
		
		// 3-3 MAIN ENC DIR,CNT
		if(diagInFlag_BobbinEncDir)		// 03-03
		{
			sensorBbnEncCnt = mainEnc.bbnCnt;
			sensorBbnEncDir = mainEnc.bbnDir;
		}
		// LM DIR,CNT
		if(stDiagSen[DIAG_SEN_LMDIR].flag || stDiagSen[DIAG_SEN_LMPULSE].flag)	// 03-03, 03-12, 03-14
		{
			sensorBbnEncCnt = mainEnc.bbnCnt;
			sensorBbnEncDir = mainEnc.bbnDir;
		}
		
		// TBD : 보드2, 보드3, 보드4 의 통신으로 오는 센서변화 확인 필요.
#if (IO_LM_MOT)
	// LM DIAG SENSOR INPUT & LOG OUT, ONLY CHANGE
	if(stDiagSen[DIAG_SEN_LS01].flag)	{stDiagSen[DIAG_SEN_LS01].curr = (digitalRead(LM_HOME_SEN_PIN)? 1:0); }		// HOME
	if(stDiagSen[DIAG_SEN_LS02].flag)	{stDiagSen[DIAG_SEN_LS02].curr = (digitalRead(LM_RIGHT_SEN_PIN)? 1:0); }		// RIGHT
	if(stDiagSen[DIAG_SEN_LS03].flag)	{stDiagSen[DIAG_SEN_LS03].curr = (digitalRead(LM_LEFT_SEN_PIN)? 1:0); }		// LEFT
	if(stDiagSen[DIAG_SEN_LS04].flag)	{stDiagSen[DIAG_SEN_LS04].curr = (digitalRead(LM_ENC_SEN_PIN)? 1:0); }			// ENC
	if(stDiagSen[DIAG_SEN_LMFG].flag)	{stDiagSen[DIAG_SEN_LMFG].curr = (digitalRead(LM_MOT_FG_PIN)? 1:0); }
	if(stDiagSen[DIAG_SEN_LMDIR].flag)	{stDiagSen[DIAG_SEN_LMDIR].curr = sensorBbnEncDir; }	// 기존 ENC DIR (3-12)
	// LM
	for (i = DIAG_SEN_LS01; i < (DIAG_SEN__MAX); i++)
	{
	  //if((stDiagSen[i].flag && (stDiagSen[i].old != stDiagSen[i].curr)))
	  //if((stDiagSen[i].flag && (stDiagSen[i].old != stDiagSen[i].curr)) || (stDiagSen[i].flag && oldDiaglmEncPulse != mainEnc.lmCnt))
	  if(stDiagSen[i].flag && ((stDiagSen[i].old != stDiagSen[i].curr) || (oldDiaglmEncPulse != mainEnc.lmCnt)))
	  {
	  		// TBD - TM
		  //ApPrintln(String(STX_DIAG_INPUT) +"03-ii" + fillZero2StringSign4Char(stDiagSen[i].curr) + "%");
			if(0) {}
		  else if(DIAG_SEN_LMDIR == i)	// 03-12, TBD 삭제?
		  {
		  	LogPrintln(" LG] IO_in 03-" + String(i) + ":" + (sensorBbnEncDir? "CW_," : "CCW, ") + "->" + String(sensorBbnEncCnt));
			  oldsensorBbnEncDir = sensorBbnEncDir;	// TBD 삭제
		  }
		  else if(DIAG_SEN_LMPULSE == i)	// 03-13
		  {
			LogPrintln(" LG] IO_in 03-" + String(i) + ":" + (sensorBbnEncDir? "CW_," : "CCW, ") + String(oldDiaglmEncPulse) +"->"+String(mainEnc.lmCnt));
			  oldsensorBbnEncDir = sensorBbnEncDir;	// TBD 삭제
			  oldDiaglmEncPulse = mainEnc.lmCnt;
		  }
		  else
		  { 
		  	LogPrintln(" LG] IO_in 03-" + String(i) + ":" + fillZero2StringSign4Char(stDiagSen[i].old) +"->" + fillZero2StringSign4Char(stDiagSen[i].curr)); 
		  }
		  stDiagSen[i].old = stDiagSen[i].curr;
	  }
	}

	  //2)변화된 경우만 출력
	  /*
		if(diagInFlag_ && (oldsensor != sensor))
		{
			ApPrintln(String(STX_DIAG_INPUT) +"02-03:" + String(sensor) + "%");
			
			LogPrintln(" LG] " + String(STX_DIAG_INPUT) +"01-02: " + fillZero2StringSign4Char(sensor) + " <- old : " + fillZero2StringSign4Char(oldsensor)+ "%");
			oldsensor = sensor;
		}
	  */
#endif
		if(diagInFlag_BootSwc && (oldsensorBootSwc != sensorBootSwc))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"01-02," + String(sensorBootSwc) + "%");
			
			LogPrintln(" LG] IO_in 01-02:" + fillZero2StringSign4Char(oldsensorBootSwc) +"->" + fillZero2StringSign4Char(sensorBootSwc));
			oldsensorBootSwc = sensorBootSwc;
		}

/*
		// SEL M/S
		if(diagInFlag_SelectBoard && (oldsensorSelBoard != sensorSelBoard))
		{
			ApPrintln(String(STX_DIAG_INPUT) +"01-04" + fillZero2StringSign4Char(sensorSelBoard) + "%");
			
			LogPrintln(" LG]" + String(STX_DIAG_INPUT) +"01-04: " + fillZero2StringSign4Char(sensorSelBoard) + " <- old : " + fillZero2StringSign4Char(oldsensorSelBoard)+ "%");
			oldsensorSelBoard = sensorSelBoard;
		}
*/

		if(diagInFlag_PcUsb5V && (oldsensorUsb5V != sensorUsb5V))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"05-02," + String(sensorUsb5V) + "%");
			
			LogPrintln(" LG] IO_in 05-02(Usb#5vDect):" + fillZero2StringSign4Char(oldsensorUsb5V) + "->" + fillZero2StringSign4Char(sensorUsb5V));	
			oldsensorUsb5V = sensorUsb5V;
		}


		if(diagInFlag_PonSwc && (oldsensorPonSwc != sensorPonSwc))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"05-01," + String(sensorPonSwc) + "%");
			
			LogPrintln(" LG] IO_in 05-01(PwrSwc):" + fillZero2StringSign4Char(oldsensorPonSwc) + "->"+fillZero2StringSign4Char(sensorPonSwc));	
			oldsensorPonSwc = sensorPonSwc;
		}


		if(diagInFlag_Interlock && (oldsensorDoorIlk != sensorDoorIlk))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"05-03," + String(sensorDoorIlk) + "%");
			
			LogPrintln(" LG] IO_in 05-03:" + fillZero2StringSign4Char(oldsensorDoorIlk) + "->" +fillZero2StringSign4Char(sensorDoorIlk));	
			oldsensorDoorIlk = sensorDoorIlk;
		}


		// BBN Encoder
		if(diagInFlag_BobbinEncA && (oldsensorBbnEncA != sensorBbnEncA))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"03-01," + String(sensorBbnEncA) + "%");
			
			LogPrintln(" LG] IO_in 03-01:" + fillZero2StringSign4Char(oldsensorBbnEncA) + "->" + fillZero2StringSign4Char(sensorBbnEncA));
			oldsensorBbnEncA = sensorBbnEncA;
		}
		if(diagInFlag_BobbinEncB && (oldsensorBbnEncB != sensorBbnEncB))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"03-02," + String(sensorBbnEncB) + "%");
			
			LogPrintln(" LG] IO_in 03-02:" + fillZero2StringSign4Char(oldsensorBbnEncB) + "->" + fillZero2StringSign4Char(sensorBbnEncB));	
			oldsensorBbnEncB = sensorBbnEncB;
		}

		// Main Encoder Dir 먼저?
		if(diagInFlag_BobbinEncDir && ( (oldsensorBbnEncCnt != sensorBbnEncCnt) || (oldsensorBbnEncDir != sensorBbnEncDir)))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			//ApPrintln(stxMsg +"03-03," + String(mainEnc.bbnDir)+"," + fillZero2StringSign6Char(sensorBbnEncCnt) + "%");
			ApPrintln(stxMsg +"03-03," + String(mainEnc.bbnDir)+"," + String(sensorBbnEncCnt) + "%");

			logStr = " LG] IO_in 03-03:";
			logStr += (sensorBbnEncDir? "CW_," : "CCW,");
			LogPrintln(logStr + "bbnCnt:" + stringSign6Char(oldsensorBbnEncCnt) + "->" + stringSign6Char(sensorBbnEncCnt));
			oldsensorBbnEncCnt = sensorBbnEncCnt;
			oldsensorBbnEncDir = sensorBbnEncDir;
		}

		if(diagInFlag_AngleHomeSns && (oldsensorAngHomeSns != sensorAngHomeSns))
		{
			ApPrintln(String(STX_DIAG_INPUT) +"03-04" + fillZero2StringSign4Char(sensorAngHomeSns) + "%");
			
			LogPrintln(" LG] IO_in 03-04:" + fillZero2StringSign4Char(oldsensorAngHomeSns) + "->" + fillZero2StringSign4Char(sensorAngHomeSns));	
			oldsensorAngHomeSns = sensorAngHomeSns;
		}

		if(diagInFlag_BbnBldcFG && (oldsensorBbnFG != sensorBbnFG))
		{
			String stxMsg;
			if(AP_IS_TM == apType)	{ stxMsg = STX_TM_INPUT_REQ; }
			else					{ stxMsg = STX_DIAG_INPUT; }
			ApPrintln(stxMsg +"03-05," + String(sensorBbnFG) + "%");
			
			LogPrintln(" LG] IO_in 03-05:" + fillZero2StringSign4Char(oldsensorBbnFG) + "->" +fillZero2StringSign4Char(sensorBbnFG));
			oldsensorBbnFG = sensorBbnFG;
		}

		
	// TBD 보드2,3,4... 센서변화 확인,출력 추가
		if(1)
		{
		}


	}
	
}

// Clear All Input Exec Flag

void diagInputMotorSensorAllOff()
{
	diagInFlag_BobbinEncA = 0;
	diagInFlag_BobbinEncB = 0;

	diagInFlag_SelectBoard = 0;
}

/*
void diagInputAllOff()
{
	diagInputMotorSensorAllOff();
}
*/


/*-----------------------------------------------------------------
  Return Random Number(0 ~9)
    Retuen = min + 
------------------------------------------------------------------*/
int randomNum(int min, int max)
{
	int random = (curr_ms_tick % 10);		// random 0 ~9
	return (random*(max-min)/9 + min);		// return Between MIN ~ MAX
}



void biteType_TorqSet(int _fishKind)
{
	if( 1 > _fishKind ) { _fishKind = 1; }	// 1 ~7
	if( 8 < _fishKind ) { _fishKind = 8; }
	
	stBiteTorqVal.deepTorq = stBiteTorqVal.torqType[_fishKind];
}


/*-----------------------------------------------------------------
  Fish Kind , 8 case 
    (break defualt = 217, Add Min : +2, add Max : +30)  degree 4.6 = 28/6
------------------------------------------------------------------*/
int fishKind_autoFromBreak(int breakval)
{
	int fishkind = 1;
	
	if( (222) > breakval )      fishkind = 1;
	else if( (226) > breakval ) fishkind = 2;
	else if( (231) > breakval ) fishkind = 3;
	else if( (235) > breakval ) fishkind = 4;
	else if( (240) > breakval ) fishkind = 5;
	else if( (245) > breakval ) fishkind = 6;
	else if( (250) > breakval ) fishkind = 7;
	else                        fishkind = 8;

	return fishkind;
}

// Add Fish Kg by GameStatus(BITE)
int fishKind_FromKg(double fishKg)
{
	int fishkind = 1;
	
	if(      ( 3.0) > fishKg ) fishkind = 1;
	else if( ( 9.0) > fishKg ) fishkind = 2;
	else if( (18.0) > fishKg ) fishkind = 3;
	else if( (28.0) > fishKg ) fishkind = 4;
	else if( (43.0) > fishKg ) fishkind = 5;
	else if( (63.0) > fishKg ) fishkind = 6;
	else if( (83.0) > fishKg ) fishkind = 7;
	else                       fishkind = 8;

	return fishkind;
}


/*-----------------------------------------------------------
	Wave Control : Servo Only
	1. Drop WAIT 3 SEC
	2. REPEAT : Servo +10 Degree ~ -10 Degree / 1500 ms
------------------------------------------------------------*/
void execWave_servo()
{
	static unsigned long wave_s_to = 0;
	static unsigned int wave_s_step = 0x00;
	
	switch(wave_s_step)
	{
		case 0x00:			// Wait
			setTO(wave_s_to);
			wave_s_step = 0x01;
			break;
		case 0x01:			// Wait
			if(checkTO(wave_s_to,3000))	// wait Drop(2sec)
			{
				wave_s_step = 0x02;
			}
			break;
		case 0x02:			// Angle 2, & Repeat
			sMotor.setAngle(stWaveServoVal.angle2);		// angle2 = 30 degree(default)
			setTO(wave_s_to);
			wave_s_step = 0x10;
			break;

		case 0x10:			// time wait
			if(checkTO(wave_s_to, stWaveServoVal.interval))	// interval = 1500 ms (Default)
			{
				wave_s_step = 0x11;
			}
			break;
			
		case 0x11:			// Angle 1
			sMotor.setAngle(stWaveServoVal.angle1);		// angle1 = 0 (default)
			setTO(wave_s_to);
			wave_s_step = 0x20;
			break;

		case 0x20:			// time wait
			if(checkTO(wave_s_to, stWaveServoVal.interval))
			{
				wave_s_step = 0x02;		// Repeat
			}
			break;
		default:
			break;
	}
	
	if(exeBite || exeSuccessFail || exeContinue || exeGameOver)		// Hit or Ending
	{
		exeWave = 0;		// Finish Casting Exe
		wave_s_step = 0x00;
	}
}


unsigned int wave_tq_step = 0x00;
//--------------------------------------------------------------
//	Wave Control TORQ Only
//	1. WAIT Drop 1 SEC
//	2. REPEAT Torq +60 Duty(900ms)  ~  +40 Duty(1100ms)
//--------------------------------------------------------------
void execWave_torque()
{
	static unsigned int oldwave_t_step = 0x00;
	static unsigned long waveto = 0;
	
	#if LOG_FUNC_STEP
	if(oldwave_t_step != wave_t_step)
	{
		LogPrintln(" LG] FSTEP execWave_torque " + String(wave_t_step)+" /Step");
		oldwave_t_step = wave_t_step;
	}
	#endif
	
	switch(wave_tq_step)
	{
		case 0x00:			// Wait
			setTO(waveto);
			//wave_t_step = 0x01;
			wave_tq_step = 0x02;		// SKIP DROP
			break;
		case 0x01:			// Wait
			if(checkTO(waveto,1000))	// wait Drop(1sec)
			{
				wave_tq_step = 0x02;
			}
			break;
			
		case 0x02:			// Torg 2, & Repeat
			torqMotor.setValue(stWaveTorqVal.tgTorq);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_tq_step = 0x10;
			break;

		case 0x10:			// time wait
			if(checkTO(waveto, stWaveTorqVal.onInterval))		// interval = 2000ms(default)
			{
				wave_tq_step = 0x11;
			}
			break;
			
		case 0x11:			// Torq 1
			torqMotor.setValue(stWaveTorqVal.rtTorq);	// Torq1 = 60 (Default)
			setTO(waveto);
			wave_tq_step = 0x20;
			break;

		case 0x20:			// time wait
			if(checkTO(waveto, stWaveTorqVal.offInterval))
			{
				wave_tq_step = 0x02;		// Repeat
			}
			break;
		default:
			break;
	}

	if(exeBite || exeContinue || exeGameOver || exeProgramEnd)		// Hit or Ending
	{
		exeWave = 0;	// Finish Casting Exe
		oldwave_t_step = 0x00;
		wave_tq_step = 0x00;
		waveto = 0;
	}
}

unsigned int wave_bbn_tq_step = 0x01;
//----------------------------------------------------------------------
//	Wave Control , BLDC Only
//	1. TORQ = 0 Out
//	2. 처음 1/2지점으로 이동
//		-  CW(9), 1100/2 ms
//	3. REPEAT BLDC
//		-  CW(9), 1100 ms
//		- CCW(9), 1100ms
//-----------------------------------------------------------------------
void execWave_bbn()
{
	static unsigned long waveto = 0;
	
	switch(wave_bbn_tq_step)
	{
		case 0x00:			// IDLE
			break;

		case 0x01:			// TORQ SET
			torqMotor.setValue(00);		// Torq2 = 75 (Default)
			//torqMotor.setValue(10);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_tq_step = 0x02;
			break;
			
		case 0x02:			// 1st CW
			if(checkTO(waveto, 100)) 	// interval = 2000ms(default)
			{
				bbnMotor.onBldc(BBN_MOT_CW, stWaveBdutyVal.tgTorq);		// Torq2 = 75 (Default)
				setTO(waveto);
				wave_bbn_tq_step = 0x03;
			}
			break;

		case 0x03:			// time wait
			if(checkTO(waveto, (stWaveBdutyVal.onInterval)/2)) 	// interval = 2000ms(default)
			{
				wave_bbn_tq_step = 0x20;		// CW -> CCW
				//wave_t_step = 0x30;			// CW -> OFF
			}
			break;
			
		case 0x10:			// 2nd ~ CW
			bbnMotor.onBldc(BBN_MOT_CW, stWaveBdutyVal.tgTorq);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_tq_step = 0x11;
			break;

		case 0x11:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.onInterval)) 	// interval = 2000ms(default)
			{
				wave_bbn_tq_step = 0x20;		// CW -> CCW
				//wave_t_step = 0x30;			// CW -> OFF
			}
			break;
			
		case 0x20:			// CCW
			bbnMotor.onBldc(BBN_MOT_CCW, stWaveBdutyVal.tgTorq);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_tq_step = 0x21;
			break;

		case 0x21:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.onInterval))
			{
				wave_bbn_tq_step = 0x10; 	// Repeat
			}
			break;

		case 0x30:			// OFF 
			bbnMotor.offBldc();
			setTO(waveto);
			wave_bbn_tq_step = 0x31;
			break;

		case 0x31:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.offInterval))
			{
				wave_bbn_tq_step = 0x10; 	// Repeat
			}
			break;

		default:
			break;
	}

	if(exeBite || exeSuccessFail || exeContinue || exeGameOver)		// Hit or Ending
	{
		exeWave = 0;	// Finish Casting Exe
		wave_bbn_tq_step = 0x00;
		torqMotor.setValue(DEFAULT_TORQ); 	//Default
		bbnMotor.offBldc();
	}
}


#define WAVE_BBN_TORQ_START 0x03
unsigned int wave_bbn_step = WAVE_BBN_TORQ_START;

/*-------------------------------------------------------------------
	BBN(BLDC) CONTROL + MAIN(TORQ) OFF

	TORQ  ON & BLDC ON - NOT USE
---------------------------------------------------------------------*/
void execWave_bbn_Torq()
{
	static unsigned long waveto = 0;
	
	switch(wave_bbn_step)
	{
		case 0x00:			// IDLE
			break;

		case 0x01:			// TORQ SET
			torqMotor.setValue(00);		// Torq2 = 75 (Default)
			//torqMotor.setValue(10);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_step = 0x02;
			break;
			
		case 0x02:			// 1st CW
			if(checkTO(waveto, 100)) 	// interval = 2000ms(default)
			{
				wave_bbn_step = 0x03;
			}
			break;

		case WAVE_BBN_TORQ_START:
			torqMotor.setValue(0);		// OFF
			bbnMotor.onBldc(BBN_MOT_CW, stWaveBdutyVal.tgTorq); 	// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_step = 0x04;
			break;
		case 0x04:			// time wait
			//if(checkTO(waveto, (stWaveBdutyVal.onInterval)/2)) 	// interval = 2000ms(default)
			if(checkTO(waveto, (stWaveBdutyVal.onInterval))) 	// interval = 2000ms(default)
			{
				wave_bbn_step = 0x20;		// CW -> CCW
				//wave_bbn_step = 0x30;			// CW -> OFF
			}
			break;
			
		case 0x10:			// 2nd ~ CW
			torqMotor.setValue(DEFAULT_TORQ_30W_BELT);
			bbnMotor.onBldc(BBN_MOT_CW, stWaveBdutyVal.tgTorq);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_step = 0x11;
			break;

		case 0x11:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.onInterval)) 	// interval = 2000ms(default)
			{
				wave_bbn_step = 0x20;		// CW -> CCW
				//wave_bbn_step = 0x30;			// CW -> OFF
			}
			break;
			
		case 0x20:			// CCW
			torqMotor.setValue(40);
			bbnMotor.onBldc(BBN_MOT_CCW, stWaveBdutyVal.tgTorq);		// Torq2 = 75 (Default)
			setTO(waveto);
			wave_bbn_step = 0x21;
			break;

		case 0x21:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.onInterval))
			{
				wave_bbn_step = 0x10; 	// Repeat
			}
			break;

		case 0x30:			// OFF 
			bbnMotor.offBldc();
			setTO(waveto);
			wave_bbn_step = 0x31;
			break;

		case 0x31:			// time wait
			if(checkTO(waveto, stWaveBdutyVal.offInterval))
			{
				wave_bbn_step = 0x10; 	// Repeat
			}
			break;

		default:
			break;
	}

	if(exeBite || exeSuccessFail || exeContinue || exeGameOver)		// Hit or Ending
	{
		exeWave = 0;	// Finish Casting Exe
		wave_bbn_step = WAVE_BBN_TORQ_START;
		torqMotor.setValue(DEFAULT_TORQ); 	//Default
		bbnMotor.offBldc();
	}
}

unsigned short occureFault = 0;

unsigned int wave_torq_bbn_step = 0;

void execWave_torq_bbn_Start()
{
	wave_torq_bbn_step = 0x10;			// START
}
/*-------------------------------------------------------------------
	MAIN(TORQ) CONTROL + BBN(BLDC) Support

	TORQ =0 : OFF & BBN)BLDC) ON
---------------------------------------------------------------------*/
void execWave_torq_bbn()
{
	static unsigned int oldwave_t_step = 0x00;
	static unsigned long waveto = 0;
	
	#if LOG_FUNC_STEP
	if(oldwave_t_step != wave_torq_bbn_step)
	{
		LogPrintln(" LG] FSTEP execWave_torque " + String(wave_t_step)+" /Step");
		oldwave_t_step = wave_torq_bbn_step;
	}
	#endif
	
	switch(wave_torq_bbn_step)
	{
		case 0x00:			// IDLE
		  /*
			setTO(waveto);
			//wave_t_step = 0x01;
			wave_torq_bbn_step = 0x10;		// SKIP DROP
		  */
			break;

		case 0x10:			// START
			torqMotor.setValue(0);		// Torq2 = 0 = OFF
			bbnMotor.onBldc(BBN_MOT_CCW, stWavePwrBbn.pwr[waveType]);
			setTO(waveto);
			wave_torq_bbn_step = 0x11;
			break;
			
		case 0x11:			// time wait
			if(checkTO(waveto, stWaveTorqVal.offInterval))		// interval = 2000ms(default)
			{
				wave_torq_bbn_step = 0x20;
			}
			break;

		case 0x20:			// Torg 2, & Repeat
			torqMotor.setValue(stWaveTorqVal.pwr[waveType]);		// Torq2 = 75 (Default)
			bbnMotor.onBldc(BBN_MOT_CW, stWavePwrBbn.pwr[waveType]);
			setTO(waveto);
			wave_torq_bbn_step = 0x21;
			break;

		case 0x21:			// time wait
			if(checkTO(waveto, stWaveTorqVal.onInterval))		// interval = 2000ms(default)
			{
				wave_torq_bbn_step = 0x10;		// REPEAT
			}
			break;

		// NOT USE
		case 0x30:			// Torq 1
			torqMotor.setValue(stWaveTorqVal.rtTorq);	// Torq1 = 60 (Default)
			setTO(waveto);
			wave_torq_bbn_step = 0x31;
			break;

		case 0x31:			// time wait
			if(checkTO(waveto, stWaveTorqVal.offInterval))
			{
				wave_torq_bbn_step = 0x10;		// Repeat
			}
			break;
		default:
			break;
	}

	// Mode STATE()
	if(exeBite || exeHit || exeFight || exeResist || exeHoldon || exeRanding || exeSuccessFail || exeContinue || exeGameOver || exeProgramEnd)		// Hit or Ending
	// TBD , Occure FAULT
	{
		exeWave = 0;	// Finish Casting Exe
		oldwave_t_step = 0x00;

		wave_torq_bbn_step = 0x00;			// STEP CLR
		torqMotor.setValue(DEFAULT_TORQ);

		bbnMotor.offBldc();
		waveto = 0;

		if(occureFault)
		{
			wave_torq_bbn_step = 0x00;
			torqMotor.setValue(DEFAULT_TORQ);
		}

	}
}


//=================================================================
//		BITE CONTROL
//=================================================================

void execBite_servo()
{
}

unsigned int bite_jt_step = 0x00;
//
//
//
void execBite_torque_justTarget()
{

	static unsigned long bite_jt_to = 0;

	int outTorq;

	switch(bite_jt_step)
	{
		// START STEP
		case 0x00:			// Servo 0 Degree, Wait 300ms
			//sMotor.setAngle(0);	// "0" Degree		// TBD ?
			setTO(bite_jt_to);
			bite_jt_step = 0x01;
			break;
		case 0x01:			// time wait
			if(checkTO(bite_jt_to,BITE_WAIT_TIME))		// wait 1sec After BITE
			{
				bite_jt_step = 0x20;	// goto TARGET
			}
			break;

		// TARGET STEP
		case 0x20:			// Torg 2
			outTorq = stBiteTorqVal.targetTorq;
			torqMotor.setValue(outTorq);
			setTO(bite_jt_to);
			bite_jt_step = 0x21;
			break;
		case 0x21:			// time wait
			if(checkTO(bite_jt_to, stBiteTorqVal.targetInterval))
			{
				bite_jt_step = 0x30;			// goto DEC STEP
			}
			break;

		// DEC STEP
		case 0x30:			// Repeat - DEC TORQ 
			outTorq -=  stBiteTorqVal.decTorq;		// - dec Torq
			if( END_TORQ > outTorq ) { outTorq = END_TORQ; }	// if torq < 60, then SET 60
			torqMotor.setValue(outTorq);
			setTO(bite_jt_to);
			bite_jt_step = 0x31; 	// goto Repeat
			break;
		case 0x31:			// time wait
			if(checkTO(bite_jt_to, stBiteTorqVal.decInterval))
			{
				if(END_TORQ < outTorq)  { bite_jt_step = 0x30;	}	// goto Repeat 
				else                    { bite_jt_step = 0x40;	}	// goto END
			}
			break;

		// END STEP
		case 0x40:			// Torq 1
			torqMotor.setValue(END_TORQ);
			setTO(bite_jt_to);
			bite_jt_step = 0x41;
			break;
		case 0x41:			// time wait
			if(checkTO(bite_jt_to,BITE_WAIT_TIME))
			{
				bite_jt_step = 0x100;
			}
			break;

		// FINISH STEP
		case 0x100:			// Finshed
			exeBite = 0; 	// Finish Casting Exe
			bite_jt_step = 0x00;		// Repeat
			break;

		default:
			break;
			
	}
	
	if(exeHit || exeSuccessFail || exeContinue || exeGameOver)		// Hit or Ending
	{
		exeBite = 0;	// Finish Casting Exe
		bite_jt_step = 0x00;
	}

}


unsigned int bite_it_step = 0x00;
//
//
//
void execBite_torque_incTarget()
{

	static unsigned long bite_it_to = 0;

	int outTorq;

	switch(bite_it_to)
	{
		// START STEP
		case 0x00:			// Servo 0 Degree, Wait 300ms
			//sMotor.setAngle(0);	// "0" Degree	// TBD ?
			setTO(bite_it_to);
			bite_it_to = 0x01;
			break;
		case 0x01:			// time wait
			if(checkTO(bite_it_to, BITE_WAIT_TIME))		// wait 1sec After BITE
			{
				bite_it_to = 0x10;	// goto INC
				outTorq = START_TORQ;
			}
			break;

		// INC STEP
		case 0x10:			// Repeat : + INC TORQ 
			outTorq +=  stBiteTorqVal.incTorq;		// + inc Torq
			if( outTorq > stBiteTorqVal.targetTorq ) { outTorq = stBiteTorqVal.targetTorq; }	// if torq GE Target, then SET Target Torq
			torqMotor.setValue(outTorq);
			setTO(bite_it_to);
			bite_it_to = 0x11; 	// goto Repeat
			break;
		case 0x11:			// time wait
			if(checkTO(bite_it_to, stBiteTorqVal.incInterval))
			{
				if(outTorq < stBiteTorqVal.targetTorq)  { bite_it_to = 0x10;	}	// goto Repeat(INC)
				else                                    { bite_it_to = 0x20;	}	// goto Target
			}
			break;

		// TARGET STEP
		case 0x20:			// Torg 2
			outTorq = stBiteTorqVal.targetTorq;
			torqMotor.setValue(outTorq);
			setTO(bite_it_to);
			bite_it_to = 0x21;
			break;
		case 0x21:			// time wait
			if(checkTO(bite_it_to, stBiteTorqVal.targetInterval))
			{
				bite_it_to = 0x30;			// goto DEC STEP
			}
			break;

		// DEC STEP
		case 0x30:			// Repeat - DEC TORQ 
			outTorq -=  stBiteTorqVal.decTorq;		// - dec Torq
			if( END_TORQ > outTorq ) { outTorq = END_TORQ; }	// if torq < 60, then SET 60
			torqMotor.setValue(outTorq);
			setTO(bite_it_to);
			bite_it_to = 0x31; 	// goto Repeat
			break;
		case 0x31:			// time wait
			if(checkTO(bite_it_to, stBiteTorqVal.decInterval))
			{
				if(END_TORQ < outTorq)  { bite_it_to = 0x30;	}	// goto Repeat 
				else                    { bite_it_to = 0x40;	}	// goto END
			}
			break;

		// END STEP
		case 0x40:			// Torq 1
			torqMotor.setValue(END_TORQ);
			setTO(bite_it_to);
			bite_it_to = 0x41;
			break;
		case 0x41:			// time wait
			if(checkTO(bite_it_to, BITE_WAIT_TIME))
			{
				bite_it_to = 0x100;
			}
			break;

		// FINISH STEP
		case 0x100:			// Finshed
			exeBite = 0; 	// Finish Casting Exe
			bite_it_to = 0x00;		// Repeat
			break;

		default:
			break;
	}
	
	if(exeHit || exeSuccessFail || exeContinue || exeGameOver)		// Hit or Ending
	{
		exeBite = 0;	// Finish Casting Exe
		bite_it_to = 0x00;
	}

}


//
//
//
unsigned int rdy_incStep = 0x00;

void rdyincTorq_Stop()
{
	rdy_incStep = 0x00;
	rdy_outTorq = DEFAULT_TORQ; // 60;
	rdy_eachTime = 1000;
	rdy_cnt = 0;
}

//
void rdyincTorq_Start()
{
	rdy_incStep = 0x10;
}


// SET TagtTorq, IncTime, Repeat Cnt
void setReadyTorque_incTarget(int targetTorq, int incTime, int incTorq)
{
	rdy_outTorq = DEFAULT_TORQ; //60;
	rdy_eachTime = incTime;
	rdy_incTorq = incTorq;
	rdy_cnt = 0;
	rdy_reqCnt = (targetTorq - DEFAULT_TORQ)/incTorq;
}

//
//	READY POSITION CONTROL
//
void setReadyPosition_Start(int option)
{
	switch(option)
	{
		case 0:		// Title Ready
			break;
			
		case 1:		// Contibue Ready
			if(100 < continueTorq)
			{
				setReadyTorque_incTarget(continueTorq, 1000, 30); // 1000ms, + 30 Torq
			}
			else
			{
				setReadyTorque_incTarget(continueTorq, 1000, 20); // 1000ms, + 20 Torq
			}
			rdyincTorq_Start();
			execContinue2Ready = 1;			// Exec Flag SET
			//Serial.println(msg+"%");		// RESP
			break;
      
		default:
			break;
			
	}
}

//
//	READY POSITION Start & CONTROL

void execReady_TorqControl()
{

	static unsigned int oldrdy_incStep = 0xFF;
	static unsigned long _to = 0;
	static int outTorq;	// = DEFAULT_TORQ; //60; 

	String rtnMsg = "";
	
	switch(rdy_incStep)
	{
		// IDLE
		case 0x00:			// IDLE
			break;
			
		case 0x10:			// Start
			rdy_cnt = 0;
			rdy_outTorq = DEFAULT_TORQ; //60;
			torqMotor.setValue(rdy_outTorq);
			//Resp2ApPrintln("$02000" + fillZero2String5Char(rdy_outTorq) + "%");
				
			setTO(_to);
			rdy_incStep = 0x11;
			break;

		case 0x11:			// Start
			if(checkTO(_to,1000))		// 1 SEC
			{
				setTO(_to);
				rdy_incStep = 0x20;
			}
			break;
			
		case 0x20:			// time wait
			rdy_cnt++;
			if(rdy_cnt > rdy_reqCnt)
			{
				rdy_incStep = 0x30;
			}
			else if(checkTO(_to, rdy_eachTime))		// wait 1sec After BITE
			{
				rdy_outTorq += rdy_incTorq;			// + 10 torq
				torqMotor.setValue(rdy_outTorq);
			}
			break;


		case 0x30:
			rdy_incStep = 0x90;

		//send to AP Ready Finish
			
			break;
			/*
			torqMotor.setValue(END_TORQ);
			setTO(bite_it_to);
			bite_it_to = 0x41;
			break;
			*/
		// FINISH STEP
		case 0x90:			// Finshed
			execContinue2Ready = 0; 	// Finish
			rdy_incStep = 0x00;			// IDLE
			oldrdy_incStep = 0x00;		// IDLE
			rdy_cnt = 0;
			
			//Serial.println(rdyMsg+"%");		// RESP Ready Finish
		#if (NEW_IF)
			rtnMsg = STX_GAME_STS + "0401%";		// Finish CONTINUE_READY
		#else
			rtnMsg = STX_GAME_STS + "22010000%";
		#endif
			//Resp2ApPrintln("$0B22010000%");		// Ready Completed
			Resp2ApPrintln(rtnMsg);			// CONTINUE READY Completed
			
			LogPrintln(" LG] RDYCN END");
			break;

		default:
			break;
	}

}


unsigned int bite_bldc_step = 0x00;
unsigned int bite_bldcRptCnt = 0;
unsigned long bite_bldc_to = 0;

void execBite_BldcStart()
{
	bite_bldc_step = 0x10;
}

#define TO_CHG_DIR_STOP 10
//
//
//
void execBite_BldcControl()
{
	static unsigned int old_bite_wt_step = 0x00;
	
	//int outDuty;

#if LOG_FUNC_STEP
	if(old_bite_wt_step != bite_bldc_step)
	{
		
		LogPrintln(" LG] BiteC step: " + String(bite_bldc_step) + "<- " + String(old_bite_wt_step));
		old_bite_wt_step = bite_bldc_step;
	}
#endif	
	switch(bite_bldc_step)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			bite_bldcRptCnt = 0;
			setTO(bite_bldc_to);
			bite_bldc_step = 0x20;
			break;
			
		case 0x20:	// BLDC CW ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[BBNACT_BITE].onDuty); // MOT CW ON
			setTO(bite_bldc_to);
			bite_bldc_step = 0x21;
			break;
		case 0x21:
			if(checkTO(bite_bldc_to, sBbnActTbl[BBNACT_BITE].onTime))
			{
				#if 0	// NOT USE(CW <-> OFF)
					setTO(bite_bldc_to);
					bite_bldc_step = 0x30;	// [ CW - OFF ] Repeat
				#else
					bbnMotor.offBldc(); 			// MOT OFF
					setTO(bite_bldc_to);
					bite_bldc_step = 0x39;	// [ CW - CCW ] REPEAT
				#endif
			}
			break;

		// NOT USE
		case 0x30:		// OFF
			bbnMotor.offBldc(); 			// MOT OFF
			setTO(bite_bldc_to);
			bite_bldc_step = 0x31;
			break;

		case 0x31:	// MOT OFF WAIT
			if(checkTO(bite_bldc_to, sBbnActTbl[BBNACT_BITE].offTime))
			{
				bite_bldcRptCnt++;
				if( bbnReqCnt[BBNACT_BITE] < bite_bldcRptCnt)
				{
					bite_bldc_step = 0x70;		// END
				}
				else
				{
					bite_bldc_step = 0x20;			// REPEAT
				}
			}
			break;

		// CW => OFF => CCW
		case 0x39:		//OFF WAIT 
			if(checkTO(bite_bldc_to, TO_CHG_DIR_STOP))		// OFF 10ms Wait Change DIR
			{
				bite_bldc_step = 0x40;
			}
			break;
			
		case 0x40:		// CCW
			bbnMotor.onBldc(BBN_MOT_CCW, sBbnActTbl[BBNACT_BITE].onDuty); // MOT CW ON
			setTO(bite_bldc_to);
			bite_bldc_step = 0x41;
			break;

		case 0x41:	// MOT OFF WAIT
			if(checkTO(bite_bldc_to, sBbnActTbl[BBNACT_BITE].onTime))
			{
				bite_bldcRptCnt++;
				if( bbnReqCnt[BBNACT_BITE] < bite_bldcRptCnt)
				{
					bite_bldc_step = 0x70;		// END
				}
				else
				{
					bbnMotor.offBldc(); 			// MOT OFF
					setTO(bite_bldc_to);
					bite_bldc_step = 0x42;			// REPEAT
				}
			}
			break;

		case 0x42:
			if(checkTO(bite_bldc_to, TO_CHG_DIR_STOP))		// OFF 10ms Wait Change DIR
			{
				bite_bldc_step = 0x20;		// REPEAT
			}
			break;

		case 0x50:	// DUMMY
			break;

		case 0x60:	// DUMMY
			break;
			

		case 0x70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			// Flage CLR & VAR Clear
			bite_bldcRptCnt = 0;
			exeFlag_biteBldcTokTokEnd = 1;	// BITE BLDC_TOKTOK_END SET
			bite_bldc_step = 0x00;
			bite_bldc_to = 0;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}


//
//
//
unsigned int bite_wf_step = 0x00;
int biteReqCnt = 0;					//

void execBite_torquewf_Start(int reqCnt)
{
	biteReqCnt = reqCnt;			// notUse
	bite_wf_step = 0x10;
}

//----------------------------------------------------------
//	biteTorqCase
//---- Control Case : 3
//----------------------------------------------------------
void execBite_torquewf_Control(int reqCnt)
{
	static unsigned long _to = 0;
	static unsigned int oldbite_wf_step = 0x00;

	static int _cnt = 0;

	static int outTorq;

  #if (LOG_FUNC_STEP)		// Step LOG
	if( oldbite_wf_step != bite_wf_step )
	{
		
		LogPrint(" st] WFtorqCont, ");
		LogPrintf("%2x", oldbite_wf_step);
		LogPrint("/");
		LogPrintf("%2x", bite_wf_step);
		LogPrintln(" old/curr");
		oldbite_wf_step = bite_wf_step;
	}
  #endif

	switch(bite_wf_step)
	{
		// START STEP
		case 0x00:			//IDLE
			break;

		case 0x10:
			//sMotor.setAngle(0);	// Servo "0" Degree, Wait 300ms
			setTO(_to);
			_cnt = 0;
			//bite_jt_step = 0x01;
			bite_wf_step = 0x11;
			break;
		
		case 0x11:			// time wait
			if(checkTO(_to, BITE_WAIT_TIME))		// wait 300 ms After BITE
			{
				bite_wf_step = 0x20;	// goto INC
			}
			break;
		
		// REPEAT : SHALLOW STEP ( n Count = 1 ~ 3 Cnt )
		case 0x20:
			_cnt++;
			if(reqCnt < _cnt)	// Over
			{
				_cnt = 0;
				bite_wf_step = 0x30;	// goto DEEP
			}
			else
			{
				outTorq = stBiteTorqVal.shallowTorq;
				torqMotor.setValue(outTorq);		// shallow Torq
				setTO(_to);
				bite_wf_step = 0x21;
			}
			break;
		case 0x21:			// Repeat : + INC TORQ 
			if(checkTO(_to, stBiteTorqVal.shallowInterval))		// wait 500ms Bite Shallow Interval
			{
				bite_wf_step = 0x22;		//
			}
			break;
			
		case 0x22:
			torqMotor.setValue(START_TORQ);
			setTO(_to);
			bite_wf_step = 0x23;
			break;

		case 0x23:			// time wait
			if(checkTO(_to, stBiteTorqVal.shallowInterval))
			{
				bite_wf_step = 0x20; 		// goto Repeat
			}
			break;

		// TARGET STEP ( DEEP STEP )
		case 0x30:			// DEEP
			outTorq = stBiteTorqVal.torqType[fishTypeAuto];
			torqMotor.setValue(outTorq);			// deep Tprq
			setTO(_to);
			bite_wf_step = 0x31;
			break;
		case 0x31:			// time wait
			if(checkTO(_to, stBiteTorqVal.deepInterval))
			{
				bite_wf_step = 0x40;			// goto DEC STEP
			}
			break;

		// DEC STEP
		case 0x40:			// Repeat - DEC TORQ 
			outTorq -=  stBiteTorqVal.decTorq;		// - dec Torq
			if( END_TORQ > outTorq )
			{ 	// if torq < 60, then SET 60
				torqMotor.setValue(END_TORQ);
				bite_wf_step = 0x50;			// goto END
			}
			else
			{
				torqMotor.setValue(outTorq);
				setTO(_to);
				bite_wf_step = 0x41; 	// goto Repeat
			}
			break;
		case 0x41:			// time wait
			if(checkTO(_to, stBiteTorqVal.decInterval))
			{
				bite_wf_step = 0x40;	// goto Repeat
			}
			break;

		// END TORQ
		case 0x50:			// Torq 1
			torqMotor.setValue(WF_END_TORQ);	// Default +20
			if(1)		// TBD
			{
				execBite_BldcStart();
				bite_wf_step = 0x51;
			}	// With BLDC TOKTOK
			else
			{
				setTO(_to);
				bite_wf_step = 0x52;
			}	// Not BLDC
			break;

		case 0x51:			// BITE BLDC SEQ
			execBite_BldcControl();			// BITE BLDC CONTROL, if Finished END_Falg SET
			if(exeFlag_biteBldcTokTokEnd)		// IF BLDC END
			{
				exeFlag_biteBldcTokTokEnd = 0;
				setTO(_to);
				bite_wf_step = 0x52;
			}
			break;
			
		case 0x52:			// time wait
			if(checkTO(_to, BITE_WAIT_TIME))
			{
				bite_wf_step = 0x90;
			}
			break;

		// FINISH STEP
		case 0x90:			// Finshed
			exeBite = 0; 	// Finish Casting Exe
			bite_wf_step = 0x00;		// Repeat
			oldbite_wf_step = 0x00;
			_cnt = 0;
			break;

		default:
			break;
	}
	
	if(exeHit || exeSuccessFail || exeContinue || exeGameOver || exeProgramEnd)		// Hit or Ending
	{
		torqMotor.setValue(END_TORQ);
		bbnMotor.offBldc();
		exeBite = 0;				//

		oldbite_wf_step = 0x00;		// CLR TORQ STEP
		bite_wf_step = 0x00;		// CLR TORQ STEP

		exeFlag_biteBldcTokTokEnd = 0;
		bite_bldc_step = 0;			// CLR Bite BLDC STEP

		_cnt = 0;
		outTorq = 0;
	}

}

//== AP Control Bite Pattern_2, $1153Kxyz ==
int bitePtn1Cnt = 3;			// Shallow Cnt
int bitePtn2Cnt = 1;			// Deep Cnt (Note Use)
int bitePtn3Cnt = 4;			// TokTok Cnt

unsigned int bite_ptn1_step = 0;
void execBite_ptn1_Start()
{
	//bitePtn1Cnt = reqCnt;			// notUse
	bite_ptn1_step = 10;

	LogPrintln(" LG] EXEBT Start PTN_1 cnt=" + String(bitePtn1Cnt));
}

unsigned int bite_ptn2_step = 0;
void execBite_ptn2_Start()
{
	//bitePtn2Cnt = reqCnt;			// notUse
	bite_ptn2_step = 10;
	LogPrintln(" LG] EXEBT Start PTN_2 cnt=" + String(bitePtn2Cnt));
}

unsigned int bite_ptn3_step = 0;
void execBite_ptn3_Start()
{
	//bitePtn3Cnt = reqCnt;			// notUse
	bite_ptn3_step = 10;
	LogPrintln(" LG] EXEBT Start PTN_3 cnt=" + String(bitePtn3Cnt));
}

void execBite_ptn_Flag_Clear()
{
	exeBitePtn1flag = 0;
	bite_ptn1_step = 0;
	exeBitePtn2flag = 0;
	bite_ptn2_step = 0;
	exeBitePtn3flag = 0;
	bite_ptn3_step = 0;
}

void exeBiteFlag_Clr_check()		// Bite Normal ENND
{
	if(0 == exeBitePtn1flag && 0 == exeBitePtn2flag && 0 == exeBitePtn3flag )
	{
		// Flag CLR
		exeBite = 0; 	// Finish Casting Exe
		execBite_ptn_Stop();
	}

}

void execBite_ptn_Stop()
{
	execBite_ptn_Flag_Clear();
	
	//Motor Output
	bbnMotor.offBldc();
	torqMotor.setValue(reqTorqueMotor);
	send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, reqTorqueMotor);

	LogPrintln(" LG] EXEBT END PTN_ALL");
}

//----------------------------------
//	Ptn1 = Shallow Bite : Control Torq Only
//	- Shallow Torq, Shallow Interval Time
//	- cnt : bitePtn1Cnt
void execBite_ptn1_Control()
{
	static unsigned long _to = 0;
	static unsigned int oldbite_ptn1_step = 0x00;

	static int _cnt = 0;

	static int outTorq;

  #if LOG_FUNC_STEP_BITE		// Step LOG
	if( oldbite_ptn1_step != bite_ptn1_step )
	{
		
		LogPrint(" st] execBite_ptn1_Control, ");
		LogPrintf("%2d", oldbite_ptn1_step);
		LogPrint("->");
		LogPrintf("%2d", bite_ptn1_step);
		LogPrintln();
		oldbite_ptn1_step = bite_ptn1_step;
	}
  #endif

	switch(bite_ptn1_step)
	{
		case 0x00:			//IDLE
			break;

		// START STEP
		case 10:
			_cnt = 0;
			bite_ptn1_step = 20;
			break;
			
		// REPEAT : SHALLOW STEP ( n Count = 1 ~ 3 Cnt )
		case 20:
			_cnt++;
			if(bitePtn1Cnt < _cnt)	// Over
			{
				bite_ptn1_step = 90;	// goto PTN1 END
			}
			else
			{
				outTorq = stBiteTorqVal.shallowTorq;
				torqMotor.setValue(outTorq);		// shallow Torq
				setTO(_to);
				bite_ptn1_step = 21;
			}
			break;
		case 21:			//Wait TIME
			if(checkTO(_to, stBiteTorqVal.shallowInterval))		// wait 500ms Bite Shallow Interval
			{
				bite_ptn1_step = 30;		// 
			}
			break;
			
		case 30:
			//torqMotor.setValue(START_TORQ);
			torqMotor.setValue(OFF_TORQ);
			
			setTO(_to);
			bite_ptn1_step = 31;
			break;

		case 31:			// time wait
			if(checkTO(_to, stBiteTorqVal.shallowInterval))
			{
				bite_ptn1_step = 20; 		// goto Repeat
			}
			break;

		// FINISH STEP
		case 90:			// Finshed
			_cnt = 0;
			bite_ptn1_step = 0;		// Repeat
			oldbite_ptn1_step = 0;

			exeBitePtn1flag = 0;
			exeBiteFlag_Clr_check();
			break;

		default:
			break;
	}
	
}

//----------------------------------
//	Ptn2 = DEEP Bite : Control Torq Only
//	- Deep Torq(by FISH KIND), Deep Interval Time
//	- DEC Torq : -15, Dec Inteval(200ms)
//	- cnt : NOT USE(bitePtn2Cnt)
void execBite_ptn2_Control()
{
	static unsigned long _to = 0;
	static unsigned int oldbite_ptn2_step = 0x00;

	static int _cnt = 0;

	static int outTorq;

  #if LOG_FUNC_STEP_BITE		// Step LOG
	if( oldbite_ptn2_step != bite_ptn2_step )
	{
		
		LogPrint(" st] execBite_ptn2_Control, ");
		LogPrintf("%2d", oldbite_ptn2_step);
		LogPrint("->");
		LogPrintf("%2d", bite_ptn2_step);
		LogPrintln();
		oldbite_ptn2_step = bite_ptn2_step;
	}
  #endif

	switch(bite_ptn2_step)
	{
		case 0x00:			//IDLE
			break;

		// START STEP
		case 10:
			_cnt = 0;
			bite_ptn2_step = 20;
			break;
			
		// REPEAT : DEEP STEP ( n Count = 1 ~ 3 Cnt )
		case 20:			// DEEP
			outTorq = stBiteTorqVal.torqType[fishTypeAuto];
			torqMotor.setValue(outTorq);			// deep Tprq
			setTO(_to);
			bite_ptn2_step = 21;
			break;
		case 21:			// time wait
			if(checkTO(_to, stBiteTorqVal.deepInterval))
			{
				bite_ptn2_step = 30;			// goto DEC STEP
			}
			break;

		// DEC STEP
		case 30:			// Repeat - DEC TORQ 
			outTorq -=  stBiteTorqVal.decTorq;		// - dec Torq
			if( END_TORQ > outTorq )
			{ 	// if torq < 60, then SET 60
				torqMotor.setValue(END_TORQ);
				bite_ptn2_step = 40;			// goto END
			}
			else
			{
				torqMotor.setValue(outTorq);
				setTO(_to);
				bite_ptn2_step = 31; 	// goto Repeat
			}
			break;
		case 31:			// time wait
			if(checkTO(_to, stBiteTorqVal.decInterval))
			{
				bite_ptn2_step = 30;	// goto DEC Repeat
			}
			break;

		// END TORQ
		case 40:			// Torq 1
			torqMotor.setValue(WF_END_TORQ);	// Default +20
			bite_ptn2_step = 90;	// END STEP
			break;

		// FINISH STEP
		case 90:			// Finshed
			_cnt = 0;
			bite_ptn2_step = 0;		// Repeat
			oldbite_ptn2_step = 0;

			exeBitePtn2flag = 0;
			exeBiteFlag_Clr_check();
			break;

		default:
			break;
	}
	
}

//----------------------------------
//	Ptn3 = TokTok Bite : Control BLDC Only
//	- TokTok OnDuty, TokTok On  Time
//	- cnt :bitePtn3Cnt
void execBite_ptn3_Control()
{
	static unsigned long _to = 0;
	static unsigned int oldbite_ptn3_step = 0x00;

	static int _cnt = 0;

	static int outTorq;

  #if LOG_FUNC_STEP_BITE		// Step LOG
	if( oldbite_ptn3_step != bite_ptn3_step )
	{
		
		LogPrint(" st] execBite_ptn3_Control, ");
		LogPrintf("%2d", oldbite_ptn3_step);
		LogPrint("->");
		LogPrintf("%2d", bite_ptn3_step);
		LogPrintln();
		oldbite_ptn3_step = bite_ptn3_step;
	}
  #endif
		
	switch(bite_ptn3_step)
	{
		case 0:			//IDLE
			break;
		
		// START STEP
		case 10:	// START
			_cnt = 0;
			bite_ptn3_step = 20;
			break;

		// REPEAT
		case 20:	// BLDC CW ON
			_cnt++;
			if( bitePtn3Cnt < _cnt)
			{
				bite_ptn3_step = 90;		// END
			}
			else
			{
				bbnMotor.onBldc(BBN_MOT_CW, sBbnBiteVal.onDuty); // MOT CW ON
				setTO(_to);
				bite_ptn3_step = 21;
			}
			break;
		case 21:
			if(checkTO(_to, sBbnBiteVal.onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(_to);
				bite_ptn3_step = 22;
			}
			break;

		// CW => OFF => CCW
		case 22:		//OFF WAIT 
			if(checkTO(_to, TO_CHG_DIR_STOP))		// OFF 10ms Wait Change DIR
			{
				bite_ptn3_step = 40;
			}
			break;
			
		case 40:		// CCW
			bbnMotor.onBldc(BBN_MOT_CCW, sBbnBiteVal.onDuty); // MOT CW ON
			setTO(_to);
			bite_ptn3_step = 41;
			break;

		case 41:	// MOT OFF WAIT
			if(checkTO(_to, sBbnBiteVal.onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(_to);
				bite_ptn3_step = 42;			//
			}
			break;

		case 42:
			if(checkTO(_to, TO_CHG_DIR_STOP))		// OFF 10ms Wait Change DIR
			{
				bite_ptn3_step = 20;		// REPEAT
			}
			break;

		case 50:	// DUMMY
			break;

		case 60:	// DUMMY
			break;
			

		case 90:	// END
			
			_cnt = 0;
			bite_ptn3_step = 0;		// Repeat
			oldbite_ptn3_step = 0;

			exeBitePtn3flag = 0;
			exeBiteFlag_Clr_check();
			// Flage CLR & VAR Clear
			//exeFlag_biteBldcTokTokEnd = 1;	// BITE BLDC_TOKTOK_END SET
			break;
			
		default:
			// ERROR (unknown Step)
			break;
	}
}


//======HIT BLDC
unsigned int hit_BldcStep = 0x00;
unsigned long hit_bldc_to = 0;
unsigned int hitBldcRptCnt = 0;

void execHit_BldcStart()
{
	hit_BldcStep = 0x10;	// START
}

//
//	[ ]CW_ON / OFF ] 3 times Repeat
//
void execHit_1st_BldcControl()
{
	static unsigned int old_bldc_step = 0x00;
	
	//int outDuty;

#if LOG_FUNC_STEP
	if(old_bldc_step != hit_BldcStep)
	{
		
		LogPrintln(" LG] StepC Hit_1st: " + String(hit_BldcStep) + "<- " + String(old_bldc_step));
		old_bldc_step = hit_BldcStep;
	}
#endif	
	switch(hit_BldcStep)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			hitBldcRptCnt = 0;
			setTO(hit_bldc_to);
			hit_BldcStep = 0x20;
			break;
			
		case 0x20:	// BLDC ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[BBNACT_HIT].onDuty); // MOT CW ON
			setTO(hit_bldc_to);
			hit_BldcStep = 0x30;
			break;
		case 0x30:
			if(checkTO(hit_bldc_to, sBbnActTbl[BBNACT_HIT].onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(hit_bldc_to);
				hit_BldcStep = 0x40;
			}
			break;
		case 0x40:	// MOT OFF WAIT
			if(checkTO(hit_bldc_to, sBbnActTbl[BBNACT_HIT].offTime))
			{
				hitBldcRptCnt++;
				if( bbnReqCnt[BBNACT_HIT] < hitBldcRptCnt)
				{
					hit_BldcStep = 0x70;		// END
				}
				else
				{
					hit_BldcStep = 0x20;			// REPEAT
				}
			}
			break;

		case 0x50:	// DUMMY
			break;

		case 0x60:	// DUMMY
			break;
			

		case 0x70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			// Flage CLR & VAR Clear
			hitBldcRptCnt = 0;
			exeHit_1st = 0;	// HIT_1st BLDC CONTROL FLAG CLR
			hit_BldcStep = 0x00;		// IDLE
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}



//
// Hit Control
//
void execHiting()
{
	// SET BLDC Control at HIT
	if(exeHit_1st)
	{
		execHit_1st_BldcControl();
	}

	// HIT TORQ Control AP 
	{
		// NOT Control TORQ at HIT
	}

	// SET Resist ( Once SET after 3000ms )
	if(!exeResist && !exeResistOnceComp)
	{ 
		execHit_ResistRandom_ToCheck();
	}


	if(0) { }	// Dummy

	// Randing
	// 순서 주의!!!
	else if(exeRanding)
	{
		execRanding_Control();		// Include Fight ?
	}

	// Hold On Control
	else if(exeHoldon)			// 우선순위 HOLD가 먼저.
	{
		//
		if(exeHoldonServoENB)
		{
		  #if IO_SERVO
			execHit_HoldServo();
		  #endif
		}
		// Start Hold Control
		execHit_HoldOn_TorqControl();
		//exeHoldon = 0;
	}
	// Resist Control
	else if(exeResist)		// 우선순위 RESIST 나중,  FW에서 Random 1회만 실행.
	{
		if(exeResistServoENB)
		{
		  #if IO_SERVO
			execHit_HoldServo();
		  #endif
		}
		execHit_ResistTorqControl();
	}
	// Fight & Randing
	else		// Fight
	{
		//execHit_BreakControl();
		execHit_BreakDecControl();
		
		if(exeFight)
		{
			execHit_FightBldcControl();
		}
	}
}

unsigned int bbnFight_step = 0x00;
unsigned int oldbbnFight_step = 0x00;
unsigned long bbnFightTO = 0;
unsigned int bbnFightCnt = 0;
unsigned int bbnFight_ptn = 0;

void execHit_FightBldcStop()
{
	exeFight = 0;			// Fight UpDown Control STOP
	bbnMotor.offBldc(); 		// MOT OFF
	
	oldbbnFight_step = 0;
	bbnFight_step = 0;				
	bbnFightTO = 0;
	bbnFightCnt = 0;
	bbnFight_ptn = 0;
	
}

void execHit_FightBldcStart(unsigned _ptn)		//_ak : Action Kind
{
		bbnFight_step = 0x10; // Start
		bbnFight_ptn = _ptn;
}

// BLDC Only
// ptn : 0-CW/OFF, 1 - CW/CCW
void execHit_FightBldcControl()		//
{
	//static unsigned long to;
  /*
	if(oldbbnFight_step != bbnFight_step)
	{
		oldbbnFight_step = bbnFight_step;
		
		LogPrintln(" LG] StepC HFightBldc: " + String(oldbbnFight_step) +"," + String(bbnFight_step));
	}
  */
	
	switch(bbnFight_step)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			bbnFightCnt++;
			setTO(bbnFightTO);
			bbnFight_step = 0x20;
			break;
			
		case 0x20:	// CW ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[BBNACT_FIGHT].onDuty);	// MOT CW ON
			setTO(bbnFightTO);
			bbnFight_step = 0x21;
			break;
		case 0x21:	// TIME OUT
			if(checkTO(bbnFightTO, sBbnActTbl[BBNACT_FIGHT].onTime))
			{
				bbnMotor.offBldc();				// MOT OFF
				setTO(bbnFightTO);
				bbnFight_step = 0x22;
			}
			break;
		case 0x22:	// OFF TIME OUT
			if(checkTO(bbnFightTO,30))
			{
				bbnFight_step = 0x23;
			}
			break;

		case 0x23:
			if(FIGHT_PTN_CW_OFF == bbnFight_ptn)
			{
				bbnFight_step = 0x30;		// []CW - OFF] Repeat
			}
			else
			{
				bbnFight_step = 0x40;		// []CW - CCW] Repeat
			}
			break;

		case 0x30:	// CW-OFF PTN : MOT OFF WAIT
			if(checkTO(bbnFightTO, sBbnActTbl[BBNACT_FIGHT].offTime))
			{
				if ( 0 == bbnReqCnt[BBNACT_FIGHT] )		// NO LIMIT
				{
					bbnFight_step = 0x20;			// REPEAT
				}
				else
				{
					bbnRptCnt++;
					if( bbnReqCnt[BBNACT_FIGHT] < bbnRptCnt)
					{
						bbnFight_step = 0x70;		// END
					}
					else
					{
						bbnFight_step = 0x20;			// REPEAT
					}
				}
			}
			break;

		case 0x40:	// CW-CCW PTN : CCW ON
			bbnMotor.onBldc(BBN_MOT_CCW, sBbnActTbl[BBNACT_FIGHT].onDuty);	// MOT CCW ON
			setTO(bbnFightTO);
			bbnFight_step = 0x41;
			break;

		case 0x41:	// TIME OUT
			if(checkTO(bbnFightTO, sBbnActTbl[BBNACT_FIGHT].onTime))
			{
				bbnMotor.offBldc();				// MOT OFF
				setTO(bbnFightTO);
				bbnFight_step = 0x42;		// []CW - OFF] Repeat
			}
			break;
		case 0x42:	// OFF TIME OUT
			if(checkTO(bbnFightTO, 30))
			{
				bbnFight_step = 0x20;		// []CW - CCW] Repeat
			}
			break;
			
		case 0x50:	// DUMMY
			break;
		case 0x60:	// DUMMY
			break;

		case 0x70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			// Flage CLR & VAR Clear
			bbnFightTO = 0;
			bbnFightCnt = 0;			
			bbnFight_step = 0x00;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}

	// FIGHT END CHECK
	//	TBD Success or PointUp
	if(exeRanding || exeSuccessFail || exeContinue || exeGameOver || exeProgramEnd )
	{
		execHit_FightBldcStop();
	}
}


//
//
//
unsigned int hitResistRdStep = 0x10;	// Driect Start (Random Time)


void execHit_ResistRandomStart()
{
	hitResistRdStep = 0x10;	// START
}

//
//  3초후 무조건 resist(저항) 1회 Set  Random 1초~4초
//
void execHit_ResistRandom_ToCheck()
{
	static unsigned long to = 0;
	static int waittime = 0;
	//static unsigned int step = 0x00;
	
	switch(hitResistRdStep)
	{
		case 0x00:			// IDLE
			break;
		case 0x10:			// Start
			setTO(to);
			//waittime = randomNum(3000,6000);
			//waittime = randomNum(2000,5000);
			waittime = randomNum(1000,4000);
			hitResistRdStep = 0x20;		
			break;
		case 0x20:			// Wait
			if(checkTO(to,waittime)) // SET Resist, Once Only, After 3000 ms
			{
				hitResistRdStep = 0x00;			// IDLE

				exeResist = 1;	// SET Resist
				exeResistOnceComp = 1;
				execHit_ResistTorqStart();

			  #if (LOG_FISH_REGIST)
				//LogPrintln(" RT] RESIT rTime:" + String(waittime) + ",SrvE: " + String(exeResistServoENB) + "," + String(exeHoldonServoENB) + "," + String(hitServoAngle) +" RENB/HDENB/ANG" );		// Log
				
				LogPrintln(" RT] RESIT Fix:" + String(torqResistFix) + ",Tgt:" + String(stHitTorqVal.targetTorq) + ",Hold:" + String(stHitTorqVal.holdonTorq) + ",Low:" + String(stHitTorqVal.lowTorq) + ",mid" + String(stHitTorqVal.midTorq) );
			  #endif
			  
			}
			break;
		default:
			break;
	}
}

//--------------------------------------
// Resist Servo = HoldOn Servo
//-------------------------------------
void execHit_HoldServo()
{
#if IO_SERVO
	static unsigned long to = 0;
	static unsigned step = 0x00;
	int angle;
	
	switch(step)
	{

		case 0x00:			// Angle 2, & Repeat
			angle = hitServoAngle;
			sMotor.setAngle(angle); 	// angle2 = 30 degree(default)
			setTO(to);
			step = 0x01;
			break;

		case 0x01:			// time wait
			if(checkTO(to, hitServoInterval)) // interval = 1500 ms (Default)
			{
				step = 0x10;
			}
			break;
			
		case 0x10:			// Angle 1
			angle = hitServoAngle * (-1);
			sMotor.setAngle(angle); 	// angle1 = 0 (default)
			setTO(to);
			step = 0x11;
			break;

		case 0x11:			// time wait
			if(checkTO(to, hitServoInterval))
			{
				step = 0x00; 	// Repeat
			}
			break;
		default:
			break;
	}
	
	if( exeRanding || exeSuccessFail || exeContinue|| exeGameOver) 	// Biting or Ending
	{
		exeHit = 0;		// Finish Casting Exe
		exeResist = 0;
		exeHoldon = 0;
		exeHoldOff = 0;
		step = 0x00;
	}
#endif
}

unsigned int hit_resist_step = 0x00;

void execHit_ResistTorqStop()
{
	exeResist = 0;
	exeResistOnceComp = 1;		// Resist COMP
	hit_resist_step = 0x00;		// STOP
}

void execHit_ResistTorqStart()
{
	hit_resist_step = 0x10;		// START
}

void execHit_ResistTorqControl()
{
	// 변수명은 임시.
	  static unsigned long _to = 0;
	  static unsigned int old_hit_resist_step = 0x00;
	
	  static int resistCnt = 0;
	
	  static int outTorq;
	  static int outBreak;
	
#if LOG_FUNC_STEP	  // Step LOG
	  if( old_hit_resist_step != hit_resist_step )
	  {
		  
		  LogPrintln(" RT] RESIT " + String(old_hit_resist_step) + "/" + String(hit_resist_step));
		  old_hit_resist_step != hit_resist_step;
	  }
#endif
	
	  switch(hit_resist_step)
	  {
		  case 0x00:		  // IDLE
			  break;
		  
		  // START STEP
		  case 0x10:
			  outTorq = stHitTorqVal.holdonTorq;
			  torqMotor.setValue(outTorq);		  // 
			  outBreak = targetBreakVal + 10;
			  send_breakmot(outBreak);
			  setTO(_to);
			  hit_resist_step = 0x11;
			  break;
		  case 0x11:		  // Repeat : + INC TORQ 
			  if(checkTO(_to, stHitTorqVal.holdonInterval))	  // wait 500ms Bite Shallow Interval
			  {
				  hit_resist_step = 0x20;	  // goto DEEP
			  }
			  break;
			  
		  case 0x20:
			  torqMotor.setValue(stHitTorqVal.lowTorq);
			  send_breakmot(targetBreakVal);
			  setTO(_to);
			  hit_resist_step = 0x21;
			  break;	
		  case 0x21:		  // time wait
			  if(checkTO(_to, stHitTorqVal.lowInterval))
			  {
				  hit_resist_step = 0x30;		  // goto Repeat
			  }
			  break;
	
		  //
		  case 0x30:		  // Torg 2
			  outTorq = stHitTorqVal.midTorq;
			  torqMotor.setValue(outTorq);			  // deep Tprq
			  outBreak = targetBreakVal + 5;
			  send_breakmot(outBreak);
			  setTO(_to);
			  hit_resist_step = 0x31;
			  break;
		  case 0x31:		  // time wait
			  if(checkTO(_to, stHitTorqVal.midInterval))
			  {
				  hit_resist_step = 0x40;			  // goto DEC STEP
			  }
			  break;
	
		  // 
		  case 0x40:		  // Torq 1
			  torqMotor.setValue(reqTorqueMotor);
			  send_breakmot(outBreakVal);
			  setTO(_to);
			  hit_resist_step = 0x41;
			  break;
		  case 0x41:		  // time wait
			  if(checkTO(_to, HIT_WAIT_TIME))
			  {
				  hit_resist_step = 0x90;
			  }
			  break;
	
		  // FINISH STEP
		  case 0x90:		  // Finshed
			  exeResist = 0;	  // Finish Casting Exe
			  hit_resist_step = 0x00;		  // Repeat
			  old_hit_resist_step = 0x00;
			  resistCnt = 0;
			  break;
	
		  default:
			  break;
	  }
	  
	  if(exeRanding || exeHoldon || exeSuccessFail || exeContinue || exeGameOver)	  // Hit or Ending
	  {
		  torqMotor.setValue(reqTorqueMotor);
		  send_breakmot(outBreakVal);
		  exeResist = 0;	  // 
		  exeResistOnceComp = 1;		// Regist COMP
		  hit_resist_step = 0x00;
		  old_hit_resist_step = 0x00;
		  resistCnt = 0;
	  }
	

}

//
//
unsigned int holdon_bldc_step = 0x00;	// IDLE
unsigned long holdon_bldc_to = 0;
unsigned int holdon_bldcRptCnt = 0;

void execHit_Holdon_BldcStart()
{
	holdon_bldc_step = 0x10;
}

//
void execHit_Holdon_BldcControl()
{
	static unsigned int old_holdon_bldc_step = 0x00;
	
	//int outDuty;
  #if LOG_FUNC_STEP
	if(old_holdon_bldc_step != holdon_bldc_step)
	{
		
		LogPrintln(" LG] HoldC BLDC step: " + String(holdon_bldc_step) + "<- " + String(old_holdon_bldc_step));
		old_holdon_bldc_step = holdon_bldc_step;
	}
  #endif
  
	switch(holdon_bldc_step)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			holdon_bldcRptCnt = 0;
			setTO(holdon_bldc_to);
			holdon_bldc_step = 0x20;
			break;
			
		case 0x20:	// BLDC ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[BBNACT_HOLDON].onDuty); // MOT CW ON
			setTO(holdon_bldc_to);
			holdon_bldc_step = 0x30;
			break;
		case 0x30:
			if(checkTO(holdon_bldc_to, sBbnActTbl[BBNACT_HOLDON].onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(holdon_bldc_to);
				holdon_bldc_step = 0x40;
			}
			break;
		case 0x40:	// MOT OFF WAIT
			if(checkTO(holdon_bldc_to, sBbnActTbl[BBNACT_HOLDON].offTime))
			{
				holdon_bldcRptCnt++;
				if( bbnReqCnt[BBNACT_HOLDON] < holdon_bldcRptCnt)
				{
					holdon_bldc_step = 0x70;		// END
				}
				else
				{
					holdon_bldc_step = 0x20;			// REPEAT
				}
			}
			break;

		case 0x50:	// DUMMY
			break;

		case 0x60:	// DUMMY
			break;
			

		case 0x70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			// Flage CLR & VAR Clear
			holdon_bldcRptCnt = 0;
			exeFlag_holonBldcTokTokEnd = 1;	// BITE BLDC_TOKTOK_END SET
			holdon_bldc_step = 0x00;
			break;
			
		default:
			// ERROR (unknown Step)
			break;
	}
}

//----------------------------------------------------------
// NEW HOLDON IF for HOLDON FW Control
int HoldOnPtn1Cnt = 4;			// Tadadak Cnt

unsigned int Holdon_ptn1_step = 0;

void execHoldon_ptn1_Start()
{
	holdOnPtnAll_Flag = 1;			// PTN전체 FLAG
	
	holdOnPtn1_Flag = 1;
	Holdon_ptn1_step = 10;

	LogPrintln(" LG] EXEHD Start PTN_1 cnt=" + String(HoldOnPtn1Cnt));
}


// HOLD패턴 처리
//	 - BLDC모터만 제어.
void execHoldon_ptn1_BldcControl()
{
	static unsigned long _to = 0;
	static unsigned int oldHoldon_ptn1_step = 0x00;

	static unsigned int _cnt = 0;
	
	//int outDuty;
  #if LOG_FUNC_STEP
	if(oldHoldon_ptn1_step != Holdon_ptn1_step)
	{
		
		LogPrintln(" LG] HoldC BLDC step: " + String(oldHoldon_ptn1_step) + " -> " + String(Holdon_ptn1_step));
		oldHoldon_ptn1_step = Holdon_ptn1_step;
	}
  #endif
  
	switch(Holdon_ptn1_step)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// START
			_cnt = 0;
			Holdon_ptn1_step = 20;
			break;

		// REPEAT
		case 20:
			_cnt++;
			if(_cnt > HoldOnPtn1Cnt)
			{
				Holdon_ptn1_step = 90;
			}
			else
			{
				setTO(_to);
				Holdon_ptn1_step = 21;
			}
			break;
			
		case 21:	// BLDC ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnHoldVal[fishTypeAuto]); // MOT CW ON
			setTO(_to);
			Holdon_ptn1_step = 22;
			break;
			
		case 22:
			if(checkTO(_to, sBbnHoldVal_onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(_to);
				Holdon_ptn1_step = 40;
			}
			break;
			
		case 40:	// MOT OFF WAIT
			if(checkTO(_to, sBbnHoldVal_offTime))
			{
				Holdon_ptn1_step = 20;			// REPEAT
			}
			break;

		case 50:	// DUMMY
			break;
		case 60:	// DUMMY
			break;
			

		case 90:	// END
			_cnt = 0;
			Holdon_ptn1_step = 0;
			oldHoldon_ptn1_step = 0;

			holdOnPtn1_Flag = 0;
			//holdOnPtnAll_Flag = 0;		// 전체 PTN Flag CLR
			Check_HoldOnPtn_AllClr();
			break;
			
		default:
			break;
	}
}

// 버티기 패턴 모든 FLAG OFF Check, 모두OFF이면 전체FLAG도 CLR
void Check_HoldOnPtn_AllClr()
{
	// 0 = TBD, Flage ADD
	if(!holdOnPtn1_Flag && 0 && 0 && 0)
	{
		holdOnPtnAll_Flag = 0;
	}
}

//---------------------------------------------------
unsigned int hit_holdon_step = 0x00;

void execHit_HoldOn_TorqStop()
{
	torqMotor.setValue(reqTorqueMotor);
	send_breakmot(outBreakVal);
	hit_holdon_step = 0x10;
}

void execHit_HoldOn_TorqStart()
{
	hit_holdon_step = 0x10;
}

void execHit_HoldOn_TorqControl()
{
	// 변수명은 임시.
	  static unsigned int old_hit_holdon_step = 0x00;
	  static unsigned long hit_holdon_to = 0;
	
	  static int holdonCnt = 0;
	
	  static int outTorq;
	  static int outBreak;
	
#if LOG_FUNC_STEP	  // Step LOG
	  if( old_hit_holdon_step != hit_holdon_step )
	  {
		  
		  LogPrintln(" RT] HoldC Torq " + String(old_hit_holdon_step) + "/" + String(hit_holdon_step));
		  old_hit_holdon_step = hit_holdon_step;
	  }
#endif
	
	  switch(hit_holdon_step)
	  {
		  case 0x00:		  // IDLE
			  break;
		  
		  // START STEP
		  case 0x10:
			  outTorq = stHitTorqVal.holdonTorq;
			  torqMotor.setValue(outTorq);		  // 
			  outBreak = targetBreakVal + 10;
			  send_breakmot(outBreak);
			  setTO(hit_holdon_to);
			  hit_holdon_step = 0x11;
			  break;
		  case 0x11:		  // Repeat : + INC TORQ 
			  if(checkTO(hit_holdon_to, stHitTorqVal.holdonInterval))	  // wait 500ms Bite Shallow Interval
			  {
				  hit_holdon_step = 0x20;	  // goto DEEP
			  }
			  break;
			  
		  case 0x20:
			  torqMotor.setValue(stHitTorqVal.lowTorq);
			  send_breakmot(targetBreakVal);
			  setTO(hit_holdon_to);
			  hit_holdon_step = 0x21;
			  break;
	
		  case 0x21:		  // time wait
			  if(checkTO(hit_holdon_to, stHitTorqVal.lowInterval))
			  {
				  hit_holdon_step = 0x30;		  // goto Repeat
			  }
			  break;
	
		  //
		  case 0x30:		  // Torg 2
			  outTorq = stHitTorqVal.midTorq;
			  torqMotor.setValue(outTorq);			  // deep Tprq
			  outBreak = targetBreakVal + 5;
			  send_breakmot(outBreak);
			  setTO(hit_holdon_to);
			  hit_holdon_step = 0x31;
			  break;
		  case 0x31:		  // time wait
			  if(checkTO(hit_holdon_to, stHitTorqVal.midInterval))
			  {
			  	  //--- BLDC Not CONTROL
				  //hit_holdon_step = 0x40;			  // goto DEC STEP

				  //--- BLDC Control
			  	  execHit_Holdon_BldcStart();
			  	  hit_holdon_step = 0x32;			  // BLDC TOKTOK
			  }
			  break;

			// BLDC TOKTOK
		  case 0x32:
		  	execHit_Holdon_BldcControl();
		  	if(exeFlag_holonBldcTokTokEnd)
		  	{
		  		exeFlag_holonBldcTokTokEnd = 0;
				hit_holdon_step = 0x40;			// goto DEC STEP
			}
		  	break;
	
		  // 
		  case 0x40:		  // Torq 1
			  torqMotor.setValue(reqTorqueMotor);
			  send_breakmot(outBreakVal);
			  setTO(hit_holdon_to);
			  hit_holdon_step = 0x41;
			  break;
		  case 0x41:		  // time wait
			  if(checkTO(hit_holdon_to, HIT_WAIT_TIME))
			  {
				  hit_holdon_step = 0x90;
			  }
			  break;
	
		  // FINISH STEP
		  case 0x90:		  // Finshed
		  // TBD Error Without HOLD_ON FLAG CLR => OK :  Flag CLR at Hold Off, Control is IDLE
			  exeResist = 0;	  // Finish Casting Exe
			  hit_holdon_step = 0x00;		  // Repeat
			  old_hit_holdon_step = 0x00;
			  holdonCnt = 0;
			  break;
	
		  default:
			  break;
	  }
	  
	  if( exeRanding || exeHoldOff || exeSuccessFail || exeContinue || exeGameOver)	  // Hit or Ending
	  {
		#if IO_SERVO
		  sMotor.maxAdjAngle = 90;
		  sMotor.setAngle(reqServoMotor);
		#endif
		  bbnMotor.offBldc();
		  torqMotor.setValue(reqTorqueMotor);
		  send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, reqTorqueMotor);

		  send_breakmot(outBreakVal);
		  
		  exeResist = 0;	  // 
		  exeHoldon = 0;	  // 
		  exeHoldOff = 0;
		  exeFlag_holonBldcTokTokEnd = 0;
		  hit_holdon_step = 0x00;
		  old_hit_holdon_step = 0x00;
		  holdonCnt = 0;
	  }
	

}

//---- Beak Control ----

unsigned int hit_brk_step = 0x00;

void execHit_SetBreakOnceStart()
{
	// 1) SET
	if (fishTypeFixENB)	targetBreakVal = stBreakVal[breakType].val[fishTypeFix];
	else				targetBreakVal = stBreakVal[breakType].val[fishTypeAuto];
	send_breakmot(targetBreakVal);

	outBreakVal = targetBreakVal;	// SET Dec Break VAL
	decBreakCnt = 0;				// Count Clear

	// 2) START
	hit_brk_step = 0x10;			// Hit Break START	
}

void send_breakmot(int _val)
{
	String str = "000";

  #if (IO_BREAK_MOT)
	if( 255 < _val) _val = 255; 	// Max 255
	
	if ( 10 > _val) 	  str = "00" + String(_val);
	else if ( 100 > _val) str = "0" + String(_val);
	else				  str = String(_val);
	
	//str = STX_BREAK_MOTOR + "00000" + str;
	
	eNow.write(STR_PID_ROD_BREAK, str);	  // Control & LOG
  #else
	
	if(ROD_BOARD_V1 == rodBoardType)	// ROD OLD Board Only
	{
		if( 255 < _val)	_val = 255;		// Max 255
	
		if ( 10 > _val)	      str = "00" + String(_val);
		else if ( 100 > _val) str = "0" + String(_val);
		else                  str = String(_val);
	
		//str = STX_BREAK_MOTOR + "00000" + str;
	
		eNow.write(STR_PID_ROD_BREAK, str);	  // Control & LOG
	
	#if LOG_IO	
		LogPrintln(" LG] BRKmt " + str);	  // LOG-SERIAL1
	#endif

	}
  #endif
  
}


//
//	Break + 5 ~ - 5 Control
//
void execHit_BreakStop()
{
	hit_brk_step = 0x00;	// IDLE
}

void execHit_BreakStart()
{
	hit_brk_step = 0x10;
}

// Duaring HITING without Resist & Hold On
// 4 sec Break Out Tgt-10 / Tgt + 10
//
// NOT USE
void execHit_BreakControl()
{
	
	static unsigned long _to = 0;

	switch(hit_brk_step)
	{

		case 0x00:
			break;
		// START
		case 0x10:
			send_breakmot(targetBreakVal-5);
			setTO(_to);
			hit_brk_step = 0x11;	  //
			break;

		case 0x11:		  // Repeat : + INC TORQ 
			if(checkTO(_to, 4000))	  // wait 3 sec(4000ms)
			{
				hit_brk_step = 0x20;	//
			}
			break;

		case 0x20:
			send_breakmot(targetBreakVal+5);
			setTO(_to);
			hit_brk_step = 0x21;	  //
			break;

		case 0x21:		  // Repeat : + INC TORQ 
			if(checkTO(_to, 4000))	  // wait 3 sec(4000ms)
			{
				hit_brk_step = 0x10;	//
			}
			break;

		default:
			break;
	}

}

//
//  HIT시 일정간격으로  계속 Break -10 감소.
//
void execHit_BreakDecStop()
{
	hit_brk_step = 0x00;	// IDLE
	send_breakmot(0);
}

void execHit_BreakDecStart()
{
	hit_brk_step = 0x10;	// Start
}

void execHit_BreakDecControl()
{
	
	static unsigned long _to = 0;

	switch(hit_brk_step)
	{

		case 0x00:		// IDLE
			break;

		case 0x10:
			outBreakVal -= 10;				// Dec -10 Break VAL
			send_breakmot(outBreakVal);
			setTO(_to);
			hit_brk_step = 0x11;	  //
			break;

		case 0x11:		  // Repeat : -DEC TORQ 
			if(checkTO(_to, (1000)))	  // wait 10 sec(4000ms)
			{
				decBreakCnt++;
				if( 19 < decBreakCnt)	// 20 sec ( 2회 x 10 sec) 초과
				{
					hit_brk_step = 0x20;	//
				}
				else
				{
					hit_brk_step = 0x10;	//
				}
			}
			break;

		case 0x20:		// 
			send_breakmot(outBreakVal);
			setTO(_to);
			hit_brk_step = 0x21;	  //
			break;
			
		case 0x21:		  // Repeat : SAME TORQ 
			if(checkTO(_to, 10000))	  // wait 10 sec(4000ms)
			{
				hit_brk_step = 0x20;	//
			}
			break;

		default:
			break;
	}

}

//
//Fast Hold ON Control
//
void execFastHold_SetVal()
{

}

unsigned int fastHoldStep =0;
void execFastHold_Start()
{
	fastHoldStep = 10;
}
//
// 약한 토크 및 약한 bldc 흔들기
//

void execFastHold_Control()
{

	static unsigned int oldfastHoldStep =0;
	static unsigned long _to = 0;

#if LOG_FUNC_STEP	  // Step LOG
	if( oldfastHoldStep != fastHoldStep )
	{	  
		LogPrintln(" RT] RANDG " + String(oldfastHoldStep) + "/" + String(fastHoldStep));
		oldfastHoldStep = fastHoldStep;
	}
#endif
		

	switch(fastHoldStep)
	{

		case 0:		// IDLE
			break;

		case 10:	// START
			break;
			
		default:	// NOT DEFINE
			// Error
			break;
	}
	
	if( exeHoldOff || exeFastHoldOff || exeSuccessFail || exeContinue || exeGameOver)	// Hit or Ending
	{
		exeFastHoldOn = 0;		// Fast Hold ON END
		exeFastHoldOff = 0;
		fastHoldStep = 0;	// IDLE
		oldfastHoldStep = 0;	// IDLE
	}

}

//
// Fast Button Combo Control
//
void execButtonCombo_SetVal()
{

}

unsigned int btnComboStep =0;
void execButtonCombo_Start()
{
	btnComboStep = 10;
}
//
// 약한 토크 및 약한 bldc 흔들기
//

void execButtonCombo_Control()
{

	static unsigned int oldbtnComboStep =0;
	static unsigned long _to = 0;

#if LOG_FUNC_STEP	  // Step LOG
	if( oldbtnComboStep != btnComboStep )
	{	  
		LogPrintln(" RT] RANDG " + String(oldbtnComboStep) + "/" + String(btnComboStep));
		oldbtnComboStep != btnComboStep;
	}
#endif
		

	switch(btnComboStep)
	{

		case 0:		// IDLE
			break;

		case 10:	// START
			break;
			
		default:	// NOT DEFINE
			// Error
			break;
	}
	
	if( exeButtonComboOff || exeSuccessFail || exeContinue || exeGameOver)	// Hit or Ending
	{
		exeButtonCombo = 0;		// Button Combo END
		btnComboStep = 0;	// IDLE
		oldbtnComboStep = 0;
	}

}


//
//Randing Control
//
void execRanding_SetVal()
{

}

unsigned int randingStep = 0;
unsigned int randingCnt = 0;

void execRanding_Start()
{
	randingStep = 10;
}
//
// 약한 토크 및 약한 bldc 흔들기
//

void execRanding_Control()
{

	static unsigned int oldrandingStep =0;
	static unsigned long _to = 0;

#if LOG_FUNC_STEP	  // Step LOG
	if( oldrandingStep != randingStep )
	{	  
		LogPrintln(" RT] RANDG " + String(oldrandingStep) + "/" + String(randingStep));
		oldrandingStep = randingStep;
	}
#endif
		

	switch(randingStep)
	{

		case 0:		// IDLE
			break;

		case 10:	// START
			setTO(_to);
			randingStep = 20;
			break;
			
		case 20:	// BLDC ON
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[BBNACT_RANDING].onDuty); // MOT CW ON
			setTO(_to);
			randingStep = 30;
			break;
		case 30:
			if(checkTO(_to, sBbnActTbl[BBNACT_RANDING].onTime))
			{
				bbnMotor.offBldc(); 			// MOT OFF
				setTO(_to);
				randingStep = 40;
			}
			break;
		case 40:	// MOT OFF WAIT
			if(checkTO(_to, sBbnActTbl[BBNACT_RANDING].offTime))
			{
				randingCnt++;
				if( bbnReqCnt[BBNACT_RANDING] < randingCnt)
				{
					randingStep = 70;		// END
				}
				else
				{
					randingStep = 20;			// REPEAT
				}
			}
			break;

		case 50:	// DUMMY
			break;

		case 60:	// DUMMY
			break;
			

		case 70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			torqDefault();			// Torq DEFAULT
			// Flage CLR & VAR Clear
			randingCnt = 0;
			randingStep = 0x00;
			break;
						break;
			
		default:	// NOT DEFINE
			// Error
			break;
	}
	
	if( exeSuccessFail || exeContinue || exeGameOver)	// Hit or Ending
	{
		exeRanding = 0;		// Randing END
		randingCnt = 0;
		randingStep = 0;	// IDLE
		oldrandingStep = 0;	// IDLE
		torqDefault();
		bbnMotor.offBldc(); 			// MOT OFF
	}

}

unsigned int exeTo1secLog = 0;
unsigned int setLogToStep = 0;

//
//
void setAfter1sec_LogOut_Start()
{
	exeTo1secLog = 1;		// exe FLAG SET
	setLogToStep = 10;		// step START SET
}

//
// 1초후에 각 Control STEP을 LOG로 출력
void setAfterTo_LogOut_Control()
{
	static unsigned int _to;
	
	switch(setLogToStep)
	{
		case 0:			// IDLE
			break;

		case 10:	// START
			setTO(_to);
			setLogToStep = 11;
			break;
		case 11:
			if(checkTO(_to, 1000))
			{
				Log_Control_Step();
				setLogToStep = 90;
			}
			break;

		case 90:	// END
			exeTo1secLog = 0;	// Exe FLAG CLR
			setLogToStep = 0;	// STEP CLR
			break;

		default:
			break;
	}
}

//
//
//---------------------------------------------------------------------------------------------------------
void execSuccessFail()
{
	// Clear Flag
	clear_ExecFlag();

	// 24/10/15, TB_FOLLOW, TORQ_UP at FAIL or SUCCESS
#if 1
	execContinue2Ready = 0;
	motor_AllDefault();		// TBD
#endif

	// Log OUT(1st) & LogOut After 1SEC(2nd)
	Log_Control_Step();
	setAfter1sec_LogOut_Start();

}

// 성공후 점수 UP화면 시 처리,
//	성공의 Motor Action을 종료 처리
void execPointUp()
{
	motor_AllDefault();		// TBD
}

void execEnd()
{
}


//====================================
// DIAGNOSITC = UNIT TEST
//
//====================================

void diagInput_AllOff()
{
	diagInput_BoardInAllOff();
	DiagInput_MotInAllOff();
	DiagInput_MechaInAllOff();
	DiagInput_ElecInAllOff();
	
}

// Clear All Input Exec Flag

#define DIAG_CHAGE_VAL	3		// 값의 변화가 있도록 OLD값을 갱신

void diagInput_BoardInAllOff()
{
	// Board Input All Off
	diagInFlag_ResetSwc = 0; 	
	diagInFlag_BootSwc = 0; 	
	diagInFlag_WdtLbSig = 0; 	
	diagInFlag_SelectBoard = 0;
}
void diagInput_BoardInAllOn()
{
	// Board Input All Off
	//diagInFlag_ResetSwc = 1; 	
	diagInFlag_BootSwc = 1; 	
	oldsensorBootSwc = sensorBootSwc + DIAG_CHAGE_VAL;
	//diagInFlag_WdtLbSig = 1; 	
	//diagInFlag_SelectBoard = 1;
}

void DiagInput_MotInAllOff()
{
	diagInFlag_BobbinEncA = 0;
	diagInFlag_BobbinEncB = 0;
	diagInFlag_BobbinEncDir = 0;
	diagInFlag_AngleHomeSns = 0;
	diagInFlag_BbnBldcFG = 0;	
}


void DiagInput_MotInAllOn()
{

	diagInFlag_BbnBldcFG = 1;
	oldsensorBbnFG = sensorBbnFG + DIAG_CHAGE_VAL;
}

void DiagInput_MainEcoderInAllOn()
{
	diagInFlag_BobbinEncA = 1;
	oldsensorBbnEncA = sensorBbnEncA + DIAG_CHAGE_VAL;
	diagInFlag_BobbinEncB = 1;
	oldsensorBbnEncB = sensorBbnEncB + DIAG_CHAGE_VAL;

	if(AP_IS_TM == apType)
	{
		diagInFlag_BobbinEncDir = 1;
		oldsensorBbnEncCnt= sensorBbnEncCnt + DIAG_CHAGE_VAL;
	}

	//agInFlag_AngleHomeSns = 1;

}



void DiagInput_MechaInAllOff()
{
	// Reserve
}


void DiagInput_ElecInAllOff()
{
	diagInFlag_PonSwc = 0;
	diagInFlag_PcUsb5V = 0;
	//diagInFlag_Interlock = 0;
}
void DiagInput_ElecInAllOn()
{
	diagInFlag_PonSwc = 1;
	oldsensorPonSwc = sensorPonSwc + DIAG_CHAGE_VAL;
	diagInFlag_PcUsb5V = 1;
	oldsensorUsb5V = sensorUsb5V + DIAG_CHAGE_VAL;
	//diagInFlag_Interlock = 1;
	//oldsensorDoorIlk = sensorDoorIlk + DIAG_CHAGE_VAL;
}

void diagInput_Cat0(String _msg)
{
	// 5 th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();
	switch(iItemNo)
	{
		case 0:
			diagInput_AllOff();
			
			LogPrintln(" LG] IO_in All Input mon OFF");
			break;
		 default:
			 
			 LogPrintln(" LG] IO_in Can't Find Item No");
			 break;
	 }

}

// BOARD IN 
void diagIput_Cat1(String _msg)
{
	int iItemNo = _msg.substring(6,(6+2)).toInt();
		   // 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt();
		   // 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE

		   switch(iItemNo)
		   {
			   case 0:
				   if(DAIG_OFF == iAct)
				   {
					   diagInput_BoardInAllOff();
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInput_BoardInAllOn();
				   }
				   
				   LogPrintln(" LG] IO_in Board Input All OFF");
				   break;
			   case 1:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_ResetSwc = 0; 		   // Monitor OFF
					   
					   LogPrintln(" LG] IO_in Reset SWC mon ON");
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_ResetSwc = 1; 		   // Moniot ON
					   oldsensorResetSwc = sensorResetSwc + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Reset SWC mon ON");
				   }
				   else
				   {
					   
					   LogPrintln(" LG] IO_in Can't Find Act No");
				   }
				   break;
			   case 2:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_BootSwc = 0;			   // Monitor OFF
					   
					   LogPrintln(" LG] IO_in Boot SWC mon OFF");
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_BootSwc = 1;			   // Moniot ON
					   oldsensorBootSwc = sensorBootSwc + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Boot SWC mon ON");
				   }
				   else
				   {
				   }
				   break;
			   case 3:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_WdtLbSig = 0; 		   // Monitor OFF
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_WdtLbSig = 1; 		   // Moniot ON
					   oldsensorWdtFbSig = sensorWdtFbSig + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in WDT LB Signal mon ON");
				   }
				   else
				   {
				   }
				   break;
			   case 4: 
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_SelectBoard = 0;			   // Monitor CW Mon OFF
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_SelectBoard = 1; 
					   oldsensorSelBoard = sensorSelBoard + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Door Interlock mon ON");
				   }
				   break; // Input Monitor Start
			   default: 
				   
				   LogPrintln(" LG] IO_in Can't Find Item No");
				   break;
		   	}
}

// MOTOR IN
void diagInput_Cat3(String _msg)
{

	String logmsg;
	
	int iItemNo = _msg.substring(6,(6+2)).toInt();
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt();
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	 // NOT USE

	switch(iItemNo)
	{
		case 0:
			if(DAIG_OFF == iAct)
			{
				DiagInput_MotInAllOff();
			}
			else if(DAIG_ON == iAct)
			{
				DiagInput_MotInAllOn();
				DiagInput_MainEcoderInAllOn();
			}
			break;

		case 1: 
			if(DAIG_OFF == iAct)
			{
				 diagInFlag_BobbinEncA = 0; 		// BBN Enc A
				 
				 LogPrintln(" LG] IO_in Main Enc_A mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncA = 1; 
				oldsensorBbnEncA = sensorBbnEncA + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc_A mon ON");
			}
			break; // Input Monitor Start

		case 2: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BobbinEncB = 0;			// BBN ENC B
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncB = 1; 
				oldsensorBbnEncB= sensorBbnEncB+ DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc_B mon ON");
			}
			break; // Input Monitor Start

		case 3: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BobbinEncDir = 0;			// BBN ENC DIR
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncDir = 1; 
				oldsensorBbnEncCnt= sensorBbnEncCnt + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc Dir/Cnt ON");
			}
			break; // Input Monitor Start

		case 4: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_AngleHomeSns = 0;			// ANGLE HOME SNS
				LogPrintln(" LG] IO_in Angle HomeSensor mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_AngleHomeSns = 1; 
				oldsensorAngHomeSns = sensorAngHomeSns + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Angle HomeSensor mon ON");
			}
			break; // Input Monitor Start

		case 5: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BbnBldcFG = 0;			// BBN FG
				LogPrintln(" LG] IO_in BBN BLDC FG mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BbnBldcFG = 1; 
				oldsensorBbnFG = sensorBbnFG + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in BBN BLDC FG mon ON");
			}
			break; // Input Monitor Start
	
		 //case 6:		 // IN3-6 1ms 불필요(DIAG는 10로 변화만 CHECK)
		 //  break;
			 
 #if (IO_LM_MOT)		 
		case 7:
			 logmsg = " LG] IO_in";
			 logmsg += " LS01_Home"; 
			 if(DAIG_OFF == iAct)
			 {
				 stDiagSen[DIAG_SEN_LS01].flag = 0;
				 LogPrintln(logmsg +  " mon OFF");
			 }
			 else if(DAIG_ON == iAct)
			 {
				 stDiagSen[DIAG_SEN_LS01].flag = 1; 
				 stDiagSen[DIAG_SEN_LS01].old = stDiagSen[DIAG_SEN_LS01].curr + DIAG_CHAGE_VAL;
				 LogPrintln(logmsg +  " mon ON");
			 }
			 break; // Input Monitor Start
			 
		case 8: 	 // LS02(Right)
			 logmsg = " LG] IO_in";
			 logmsg += " LS02_Right";
			 if(DAIG_OFF == iAct)
			 {
				 stDiagSen[DIAG_SEN_LS02].flag = 0;
				 LogPrintln(logmsg +  " mon OFF");
			 }
			 else if(DAIG_ON == iAct)
			 {
				 stDiagSen[DIAG_SEN_LS02].flag = 1; 
				 stDiagSen[DIAG_SEN_LS02].old = stDiagSen[DIAG_SEN_LS02].curr + DIAG_CHAGE_VAL;
				 LogPrintln(logmsg +  " mon ON");
			 }
			 break;
			 
		case 9: 	 // LS03(Left)
			logmsg = " LG] IO_in";
			logmsg += " LS03_Left"; 
			if(DAIG_OFF == iAct)
			{
				stDiagSen[DIAG_SEN_LS03].flag = 0;
				LogPrintln(logmsg +  " mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				stDiagSen[DIAG_SEN_LS03].flag = 1; 
				stDiagSen[DIAG_SEN_LS03].old = stDiagSen[DIAG_SEN_LS03].curr + DIAG_CHAGE_VAL;
				LogPrintln(logmsg +  " mon ON");
			}
			break;
		case 10:	 // LS04(Lm Enc Pulse)
			logmsg = " LG] IO_in";
			logmsg += " LS04_Enc";
			if(DAIG_OFF == iAct)
			{
				stDiagSen[DIAG_SEN_LS04].flag = 0;
				LogPrintln(logmsg +  " mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				stDiagSen[DIAG_SEN_LS04].flag = 1; 
				stDiagSen[DIAG_SEN_LS04].old = stDiagSen[DIAG_SEN_LS04].curr + DIAG_CHAGE_VAL;
				LogPrintln(logmsg +  " mon ON");
			}
			break;
		case 11:	 // FG 10ms
			logmsg = " LG] IO_in";
			logmsg += " LM_FG"; 
			if(DAIG_OFF == iAct)
			{
				stDiagSen[DIAG_SEN_LMFG].flag = 0;
				LogPrintln(logmsg +  " mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				stDiagSen[DIAG_SEN_LMFG].flag = 1; 
				stDiagSen[DIAG_SEN_LMFG].old = stDiagSen[DIAG_SEN_LMFG].curr + DIAG_CHAGE_VAL;
				LogPrintln(logmsg +  " mon ON");
			}
			break;
		//case 12:		 // FG 1ms
		 //  break;
	
		case 12:	 // LM Dir
			logmsg = " LG] IO_in";
			logmsg += " BBN ENC DIR"; 
			if(DAIG_OFF == iAct)
			{
				stDiagSen[DIAG_SEN_LMDIR].flag = 0;
				LogPrintln(logmsg +  " mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				stDiagSen[DIAG_SEN_LMDIR].flag = 1; 
				oldsensorBbnEncCnt = sensorBbnEncCnt - 1;
				oldsensorBbnEncDir = sensorBbnEncDir - 1;
				//stDiagSen[DIAG_SEN_LMDIR].old = stDiagSen[DIAG_SEN_LMDIR].curr - 1;
				stDiagSen[DIAG_SEN_LMDIR].old = stDiagSen[DIAG_SEN_LMDIR].curr + DIAG_CHAGE_VAL;
				
				LogPrintln(logmsg +  " mon ON");
			}
			break;

		case 13:	 // LM Enc Pulse
			logmsg = " LG] IO_in";
			logmsg += " LM Enc Pulse"; 
			if(DAIG_OFF == iAct)
			{
				stDiagSen[DIAG_SEN_LMPULSE].flag = 0;
				LogPrintln(logmsg +  " mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				stDiagSen[DIAG_SEN_LMPULSE].flag = 1; 
				//oldDiaglmEncPulse = mainEnc.lmCnt - 1;
				oldDiaglmEncPulse = mainEnc.lmCnt  + DIAG_CHAGE_VAL;
				LogPrintln(logmsg +  " mon ON");
			}
			break;

 #endif
	
		default:
			LogPrintln(" LG] IO_in Can't Find Item No");
			break;
	}

}

// ELEC IN
void diagInput_Cat5(String _msg)
{
	   // 5 th : "-"
	   int iItemNo = _msg.substring(6,(6+2)).toInt();
	   // 8th : ","
	   int iAct = _msg.substring(9,(9+1)).toInt();
	   // 10th : ","
	   int iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE

	switch(iItemNo)
	{
		case 0:
			if(DAIG_OFF == iAct)
			{
				DiagInput_ElecInAllOff();
			}
			else if(DAIG_ON == iAct)
			{
				DiagInput_ElecInAllOn();
			}
			break;
		case 1: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_PonSwc = 0;			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_PonSwc = 1; 
				oldsensorPonSwc = sensorPonSwc + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Power On Switch mon ON");
			}
			break; // Input Monitor Start
		case 2: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_PcUsb5V = 0; 			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_PcUsb5V = 1; 
				oldsensorUsb5V = sensorUsb5V + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Usb 5V mon ON");
			}
			break; // Input Monitor Start
		case 3: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_Interlock = 0;			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_Interlock = 1;
				oldsensorDoorIlk = sensorDoorIlk + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Door Interlock mon ON");
			}
			break; // Input Monitor Start
			
		default: 
			LogPrintln(" LG] IO_in Can't Find Item No");
			break;
	}

}

/*-------------------------------------------------------------------
$DIxx-yy,A%
-------------------------------------------------------------------*/
void diagInputTest(String _msg)
{
   #if 0   //
		   rst = controlDiagInput(msg);
	   // TBD : Responce Input REsult
   #else
	   int iCatNo = 0;
	   int iItemNo = 0;
	   int iAct = 0;
	   int iPara = 0;

	   String logmsg = "";
	   
	   //int dir = 0;
	   //int dutyPercent = 0;

	   //------------------------------------------------------
	   //  $DIxx-xx,A%
	   //------------------------------------------------------
	   iCatNo = _msg.substring(3,(3+2)).toInt();	//xx-
	   // 5 th : "-"
	   iItemNo = _msg.substring(6,(6+2)).toInt();	//yy,
	   // 8th : ","
	   iAct = _msg.substring(9,(9+1)).toInt();		//A,
	   // 10th : ","
	   iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE
	   
	   if( 0 == iCatNo)
	   {
			diagInput_Cat0(_msg);
	   }
	   else if( 1 == iCatNo)	   // 보드1 Board In
	   {
	   		diagIput_Cat1(_msg);

	   }
	   else if( 2 == iCatNo)		   // Reserve (OP)
	   {
		   // RSV
		   LogPrintln(" LG] IO_in Can't Find Cat No");
	   }
	   else if( 3 == iCatNo)		   // Motor IN
	   {
	   		diagInput_Cat3(_msg);
	   }

	   // "04" - Reserve
	   else if( 4 == iCatNo)	   // 
	   {
	   	// RSV
	   }

	   // "05" - Board Input
	   else if( 5 == iCatNo)	   // 
	   {
	   		diagInput_Cat5(_msg);

	   }
	   else if( 11 == iCatNo)	   // 
	   {
	   }
	   else if( 12 == iCatNo)	   // 
	   {
	   }
	   else if( 14 == iCatNo)	   // 
	   {
	   }
	   else if( 15 == iCatNo)	   // 
	   {
	   }
	   else if( 20 == iCatNo)	   // 
	   {
	   }
	   else
	   {
		   
		   LogPrintln(" LG] IO_in Can't Find Cat No");
	   }
   #endif

   // TBD,	Responce - Return Value
   // Diag ON - Input - Chaned then Responce Always!!!
}

//===============================\
//  Diag Output Functins
//===============================

void diagOutput_AllOff()
{
	diagOutput_BoardOutAllOff();
}

void diagOutput_BoardOutAllOff()
{
	setRGBcolor(LED_IDX_CENT, COLOR_BLACK);
}


void setRGBcolor(int pos, int Color)
{
	if(COLOR_MAX_NO > Color)
	{
		sRGB[pos].r = sColorTBL[Color].r;	
		sRGB[pos].g = sColorTBL[Color].g;	
		sRGB[pos].b = sColorTBL[Color].b;
	}
	else
	{
		LogPrintln(" LG] LedRg Can't Find Color Number!!!");
	}
}

void setOutRGBcolor(int pos, int Color)
{
	if(COLOR_MAX_NO > Color)
	{
		outRGB[pos].r = sColorTBL[Color].r;	
		outRGB[pos].g = sColorTBL[Color].g;	
		outRGB[pos].b = sColorTBL[Color].b;
	}
	else
	{
		LogPrintln(" LG] LedRg Can't Find Color Number!!!");
	}
}

// LM MOT DIAG OUT
//
int lmMotTimeout_step = 0;
int diagOutflag_lmMotTimeOut = 0;
unsigned long lmMotTimeout_Val = 0;

void diagLmMotor_TimeoutStart(unsigned int _time)
{
	lmMotTimeout_Val = _time;
	lmMotTimeout_step = 10;			// Start
	diagOutflag_lmMotTimeOut = 1;		// monitor SET
}

void diagLmMotor_TimeOutStop()
{
	diagOutflag_lmMotTimeOut = 0;		// monitor SET
	lmMotTimeout_step = 0;			// Start
}

//
void diagLmMotor_TimeoutControl()
{
	static long _to;
	switch(lmMotTimeout_step)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// START
			setTO(_to);
			lmMotTimeout_step = 20;
			break;
		case 20:	// TIMEOUT Check
			if(checkTO(_to, lmMotTimeout_Val))
			{
				lmMotTimeout_step = 30;
			}
			break;
		case 30:	// MOT OFF
			lmMotOff();
			lmMotTimeout_step = 40;
			break;
		case 40:	// END
			_to = 0;
			lmMotTimeout_step = 0;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}


/*
==========================================================
	DIAG BBN Motor Control
		for DIAG
==========================================================
*/
// Auto Time Out  Control
void diagBbnMotor_TimeoutStart(unsigned int _time)
{
	bbnTimeout_Val = _time;
	bbnTimeout_step = 0x10;			// Start
	diagOutflag_bbnTimeOut = 1;		// monitor SET
}

// TBD, 
/*
	Diag_Exit함수에 아래 추가 필요.

	1. (조작) Diag나가기, Cat별 All Off하기등 
	 Step중에 종료되는 경우, Flag/Step처리 추가할 것.
*/


//
void diagBbnMotor_TimeoutControl()
{
	
	switch(bbnTimeout_step)
	{
		case 0x00:	// IDLE
			break;
		case 0x10:	// START
			setTO(bbnDiagTO);
			bbnTimeout_step = 0x20;
			break;
		case 0x20:	// TIMEOUT Check
			if(checkTO(bbnDiagTO, bbnTimeout_Val))
			{
				bbnTimeout_step = 0x30;
			}
			break;
		case 0x30:	// MOT OFF
			bbnBldcOff();
			bbnTimeout_step = 0x40;
			break;
		case 0x40:	// END
			diagOutflag_bbnTimeOut = 0;
			bbnTimeout_step = 0x00;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}

//=============================================
//	TOK TOK Control
//=============================================
void daigBbnMotor_TokTokEnd()
{
	diagBbnflag_TokTokControl = 0;
	bbnTokTok_step = 0x00;
	bbnMotor.offBldc();
}


void diagBbnMotor_TokTokStart(unsigned int _ackKind, unsigned int _cnt)
{

/*
	bbnTokTok_OnDuty = _onduty;
	bbnTokTok_OnTime = _ontime;
	bbnTokTok_OffDuty = _offduty;
	bbnTokTok_OffTime = _offtime;
*/
#if 1
	bbnActKind = _ackKind;
	// bbnOpKind = _opKind;
  	bbnReqCnt[bbnActKind] = _cnt;
#endif


	bbnTokTok_step = 0x10;
	diagBbnflag_TokTokControl = 1;		// Start
}


//
void diagBbnMotor_TokTokControl(unsigned _opKind)		//_ak : Action Kind
{
	//static unsigned long to;
  /*
	if(oldbbnTokStep != bbnTokTok_step)
	{
		oldbbnTokStep = bbnTokTok_step;
		
		LogPrintln(" LG] StepC bbn toktok step: " + String(bbnTokTok_step) );
	}
  */
	
	switch(bbnTokTok_step)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			bbnRptCnt = 0;
			setTO(bbnTokTO);
			bbnTokTok_step = 0x20;
			break;
			
		case 0x20:	// TIMEOUT Check
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[bbnActKind].onDuty);	// MOT CW ON
			setTO(bbnTokTO);
			bbnTokTok_step = 0x30;
			break;
		case 0x30:
			if(checkTO(bbnTokTO, sBbnActTbl[bbnActKind].onTime))
			{
				bbnMotor.offBldc();				// MOT OFF
				setTO(bbnTokTO);
				bbnTokTok_step = 0x40;
			}
			break;
		case 0x40:	// MOT OFF WAIT
			if(checkTO(bbnTokTO, sBbnActTbl[bbnActKind].offTime))
			{
				bbnRptCnt++;
				if( bbnReqCnt[bbnActKind] < bbnRptCnt)
				{
					bbnTokTok_step = 0x70;		// END
				}
				else
				{
					bbnTokTok_step = 0x20;			// REPEAT
				}
			}
			break;

		case 0x50:	// DUMMY
			break;

		case 0x60:	// DUMMY
			break;
			

		case 0x70:	// END
			bbnMotor.offBldc(); 			// MOT OFF
			// Flage CLR & VAR Clear
			bbnRptCnt = 0;
			
			bbnTokTok_step = 0x00;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}

//
void diagOut_Cat1(String _msg)
{
	int iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
	//5th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt(); 	// Act(1)
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
	//14th : ","
	//iPara2 = _msg.substring(15).toInt();	// para_2(4)
	int iPara2 = _msg.substring(15,(15+4)).toInt(); // para_2(4)
	// 19th : ","
	int iPara3 = _msg.substring(20,(20+5)).toInt(); // para_3(5)

	switch(iItemNo)
	{
		case 0:
			diagOutput_BoardOutAllOff();
			
			LogPrintln(" LG] IOout Board Output All OFF");
			break;
		case 1:
			diagOutFlag_WDToff = 1; 		// WDT OFF
			
			LogPrintln(" LG] IOout WDT OFF");
			break;
		case 2:
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED2_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED2 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED2_PIN, LED_ON);
				
				LogPrintln(" LG] IO_in	BD LED2 ON");
			}
			else
			{
			}
			break;
	
#if(!IO_LM_MOT)
		case 3:
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED3_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED3 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED3_PIN, LED_ON);
				
				LogPrintln(" LG] IOout BD LED3 ON");
			}
			else
			{
			}
			break;
		case 4: 
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED4_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED4 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED4_PIN, LED_ON);
				
				LogPrintln(" LG] IOout BD LED4 ON");
			}
			else
			{
			}
			break; // Input Monitor Start
		case 5 :	// DEBUG PORT
			if(DAIG_OFF == iAct)
			{
				debugPort_Off();
				LogPrintln(" LG] IOout BD DebugPort OFF");
			}
			else if(DAIG_ON == iAct)
			{
				debugPort_On();
				LogPrintln(" LG] IOout BD DebugPort ON");
			}
	
			else
			{
				// Error
			}
			break;
#endif
	
		default: 
			break;
	}

}

// DIAG OUT 03-XX
void diagOut_Cat3(String _msg)
{
	int iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
	//5th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt(); 	// Act(1)
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
	//14th : ","
	//iPara2 = _msg.substring(15).toInt();	// para_2(4)
	int iPara2 = _msg.substring(15,(15+4)).toInt(); // para_2(4)
	// 19th : ","
	int iPara3 = _msg.substring(20,(20+5)).toInt(); // para_3(5)

	switch(iItemNo)
	{
		case 0 :	// TBD Mot All Off
			torqMotor.setValue(0);
	  #if IO_SERVO
			sMotor.setAngle(0);
	  #endif
			bbnMotor.offBldc();
			
			LogPrintln(" LG] IOout Motot ALL OFF");
			break;
			
		case 1 :	// TORQ
			if(DAIG_MOT_OFF == iAct)
			{
				torqMotor.setValue(0);
				
				LogPrintln(" LG] IOout Main Mot(Torq) OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				torqMotor.setValue(iPara);
				
				LogPrintln(" LG] IOout Main Mot(Torq) ON, Duty : " + String(iPara));
			}
	
			else
			{
			}
			break;
			
		case 2 :	// TBD , Angle Motor
			break;
	
		case 3 :	// BBN BLDC
			// Para Check
			/*
			if((DAIG_MOT_OFF != iAct ) && ( 1 > iPara || 100 < iPara))
			{
				
				LogPrintln(" LG] Duty Range Over : " + String(iPara));
	
				// END
				//iPara = 5;
				break;
			}
			*/
			
			if(DAIG_MOT_OFF == iAct)
			{
				bbnMotor.offBldc();
				
				LogPrintln(" LG] IOout BBNMT OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CW, iPara);
				diagBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout BBNMT CW ON, Duty : " + String(iPara));
			}
			else if(DAIG_MOT_CCW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CCW, iPara);
				diagBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout BBNMT CCW ON, Duty : " + String(iPara));
			}
			else
			{
			}
			break;
			
			
		case 4 :	// TBD	BLDC_24V_OFF
			if(DAIG_MOT_OFF == iAct)
			{
				digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);	// BLDC_24V_ON OFF
				
				LogPrintln(" LG] IOout BBN24 OFF");
			}
			else			// ON
			{
				digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);	  // BLDC_24V_ON ON
				
				LogPrintln(" LG] IOout BBN24 ON");
			}
			break;
	
		case 5 :	// BBN BLDC TOKTOK
			// Para Check
			/*
			if((DAIG_MOT_OFF != iAct ) && ( 1 > iPara || 100 < iPara))
			{
				
				LogPrintln(" LG] Duty Range Over : " + String(iPara));
	
				// END
				//iPara = 5;
				break;
			}
			*/
			
			if(DAIG_MOT_OFF == iAct)
			{
				//bbnMotor.offBldc();
				daigBbnMotor_TokTokEnd();
				
				LogPrintln(" LG] IOout BBNTK OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				//bbnMotor.onBldc(BBN_MOT_CW, iPara);
				//diagBbnMotor_TokTokStart(bbnTokTok_OnDuty, bbnTokTok_OnTime, bbnTokTok_OffDuty, bbnTokTok_OffTime);
				//iPara = _msg.substring(9,(9+1)).toInt();	// // ActKind
				if ( (BBNACT_TBLMAX) <= iPara)	{ iPara = 0; }		// OP_kind (Wave,...)
				//iPara2 = _msg.substring(10,(10+1)).toInt();	// Repeat CNT
				//if (	<= iPara2)	{ iPara2 =	}
				//iPara2 = _msg.substring(10).toInt();	// Repeat CNT  (Repeat)
				if ( (10000) <= iPara2) { iPara2 = 10000; }
			//TBD Cnt or TIME
			/*
				iPara2 = _msg.substring(10).toInt();	// Total Time
				if ( (30000) <= iPara2) { iPara2 = 30000; }
			*/
				diagBbnMotor_TokTokStart(iPara, iPara2);			// (ActKind=OpKind, RPT CNT) 
				//String tokStr = String(bbnTokTok_OnDuty) + ", " + String(bbnTokTok_OnTime) + ", " + String(bbnTokTok_OffDuty) + ", " + String(bbnTokTok_OffTime);
				
				LogPrintln(" LG] IOout BBNTK ON, AckKind : " + String(iPara) + ", opKind: " + String(iPara2) + ", Repeat CNT: " + String(iPara3)+ ", On/Time/Off/Time: " + String(bbnTokTok_OnDuty) + ", " + String(bbnTokTok_OnTime) + ", " + String(bbnTokTok_OffDuty) + ", " + String(bbnTokTok_OffTime));
			}
		/*
			else if(DAIG_MOT_CCW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CCW, dutyPercent);
				//daigBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout Bobbin CCW ON, Duty : " + String(iPara));
			}
		*/
			else
			{
			}
			break;
			
#if (IO_LM_MOT)
		case 6:
			if(DAIG_MOT_OFF == iAct)
			{
				digitalWrite(LM_24VON_PIN, LM_24VOFF);	// BLDC_24V_ON OFF
				
				LogPrintln(" LG] IOout LM24 OFF");
			}
			else			// ON
			{
				digitalWrite(LM_24VON_PIN, LM_24VON);	  // BLDC_24V_ON ON
				
				LogPrintln(" LG] IOout LM24 ON");
			}			break;

		case 7:
			if(DAIG_MOT_OFF == iAct)
			{
				lmMotor.offBldc();
				
				LogPrintln(" LG] IOout LMMOT OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				lmMotor.onBldc(LM_MOT_CW, iPara);
				diagLmMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout LMMT CW ON, Duty : " + String(iPara));
			}
			else if(DAIG_MOT_CCW == iAct)
			{
				lmMotor.onBldc(LM_MOT_CCW, iPara);
				// diagBbnMotor_TimeoutStart(3000);
				diagLmMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout LMNMT CCW ON, Duty : " + String(iPara));
			}
			else
			{
			}
			break;			break;

		// TBD-LM	ACT
		case 8:
			break;

#endif
	
		default:
			break;
	}

}

/*
=================================================================
	DIAG OUTPUT Control
$DOxx-yy,A,pppp,PPPPP
=================================================================
*/
void diagOutputTest(String _msg)
{
	#if 0	//
		rst = controlDiagOutput(msg);
	// TBD : Responce Output REsult
	#else
		int iCatNo = 0;
		int iItemNo = 0;
		int iAct = 0;
		int iPara = 0;
		int iPara2 = 0;
		int iPara3 = 0;

		int dir = 0;
		int dutyPercent = 0;
		
		iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
		//5th : "-"
		iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
		// 8th : ","
		iAct = _msg.substring(9,(9+1)).toInt();		// Act(1)
		// 10th : ","
		iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
		//14th : ","
		//iPara2 = _msg.substring(15).toInt();	// para_2(4)
		iPara2 = _msg.substring(15,(15+4)).toInt();	// para_2(4)
		// 19th : ","
		iPara3 = _msg.substring(20,(20+5)).toInt();	// para_3(5)
		
	
		dutyPercent = iPara;

		//LogPrintln(" LG] IOout " );
		if( 0 == iCatNo && 0 == iItemNo )
		{
			// DiagInMonitor = 0;
			diagOutput_AllOff();
			
			LogPrintln(" LG] IOout All Output ALL OFF");
		}

		// " 1" - BD_OUT
		else if( 1 == iCatNo)		// 보드1 Board Out
		{
			diagOut_Cat1(_msg);
		}

		// "02"- Ext LED
		else if( 2 == iCatNo)			// Reserve (OP)
		{
			switch(iItemNo)
			{
				case 0:	// TBD
					//diagInput_BoardInAllOff();
					extLed_AllOff();					
					
					LogPrintln(" LG] IOout Ext LED All OFF");
					break;
				case 1:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_R = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_R);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_R OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_R = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_R, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_R ON");
					}
					else
					{
					}
					break;
				case 2:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_G = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_G);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_G OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_G = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_G, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_G ON");
					}
					else
					{
					}
					break;
				case 3:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_B = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_B);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_B OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_B = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_B, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_B ON");
					}
					else
					{
					}
					break;
				case 4: 
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_RGB = 0; 			// Monitor CW Mon OFF
						extLed.off(LEDPOS_CENT_R);		// Position, Duty
						extLed.off(LEDPOS_CENT_G);		// Position, Duty
						extLed.off(LEDPOS_CENT_B);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_RGB = 1;
						//setRGBcolor(sRGB, iPara);				// Color N -> R,G,B
						setRGBcolor(LEDPOS_CENT_RGB, iPara);				// Color N -> R,G,B
						extLed.RGBon(LEDPOS_CENT_RGB, sRGB[LEDPOS_CENT_RGB].r, sRGB[LEDPOS_CENT_RGB].g, sRGB[LEDPOS_CENT_RGB].b);
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else
					{
					}
					break; // Input Monitor Start

				// TBD Color item, Dimming
				case 5:
					break;
					
				case 6: 
					if(DAIG_OFF == iAct)
					{
						//diagOutFlag_LedCent_RGB = 0; 			// Monitor CW Mon OFF
						extLed.off(LEDPOS_BTM);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else if(DAIG_ON == iAct)
					{
						//diagOutFlag_LedCent_RGB = 1;
						//setRGBcolor(sRGB, iPara);				// Color N -> R,G,B
						//setRGBcolor(iPara);				// Color N -> R,G,B
						extLed.on(LEDPOS_BTM, iPara);
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else
					{
					}
					break; // Input Monitor Start
					
				default: 
					break;
			}
		}

		// "03"- MOTOR
		else if( 3 == iCatNo )
		{
			diagOut_Cat3(_msg);
		}

		// "04 - Reserved"
		else if( 4 == iCatNo )
		{
		}

		// "05" - Elec Out
		else if( 5 == iCatNo )
		{
			switch(iItemNo)
			{
				case 0 : 	// TBD Mot All Off
					// Board_1 Out ALL OFF					
					LogPrintln(" LG] IOout Board_1 ALL OFF");
					break;
					
				case 1 : 	// SUB_AC
					if(DAIG_OFF == iAct)
					{
						//subAC_Off();
						subAcOff_WithOtherOff();
					}
					else if(DAIG_ON == iAct)
					{
						subAC_On();
					}

					else
					{
						// Error
					}
					break;
					
				default:
					break;
			}
		}

		else
		{
			// Error
		}
	#endif
}

// NOT USE
void findDiagCode(String msg, stDiagCode code)
{
	code.catNo = msg.substring(3,5).toInt();	// 2 char	XX
												// 1char Skip "-" -
	code.itemNo = msg.substring(6,8).toInt(); 	// 2 char	YY
	
	code.act = msg.substring(8,9).toInt();		// 1 char	A
	
	code.para = msg.substring(9).toInt();		// 3 char ~	PPP ~ PPPPPPPPPPPPP
}

// NOT USE
int controlDiagInput(String msg)
{
	stDiagCode diag_code;		// item, actno
	findDiagCode(msg, diag_code);

}

// NOT USE
int controlDiagOutput(String msg)
{
	stDiagCode diag_code;		// item, actno
	findDiagCode(msg, diag_code);
	
	// execOutAct(diag_code);

}


#if 0
void execOutAct(stDiagCode dcode);
{
	stDiagOutTb[dcode.item][dcode.actNo].diagcallback(dcode.para);
}
#endif

void diagOutAllOff()
{
	torqMotor.setValue(0);
  #if IO_SERVO
	sMotor.setAngle("000");
  #endif
	bbnMotor.offBldc();
}



/*---------------------------------------------------------------------------------------------
	각 값들의 설정
	방법 - LOG SERIAL포트로 명령 송신
	처리 - 각 Control등의 설정값을 세팅
---------------------------------------------------------------------------------------------*/

// Wave Val SET ( $F0 )
void Set_WaveData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// Control SET
	{
		// Log OUT
	}
	else if( '1' == ch ) 		// Control Case
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 7 < val ) val = 7;
		waveControlCase = val;
		//
		//LogPrintln(" LG] waveControlCase: " + String(waveControlCase);
	}
	else if( '2' == ch )		// Angle SET
	{
		val = msg.substring(4,7).toInt();	// angle 1
		if( -90 > val ) val = -90;
		if( 90 < val ) val = 90;
		stWaveServoVal.angle1 = val;
	
		val = msg.substring(7,10).toInt();	// angle 2
		if( -90 > val ) val = -90;
		if( 90 < val ) val = 90;
		stWaveServoVal.angle2 = val;
	
		val = msg.substring(10).toInt();	// Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveServoVal.interval = val;
	}
	else if( '3' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Return Torq
		if( 20 > val ) val = 20;
		if( 80 < val ) val = 80;
		stWaveTorqVal.rtTorq = val;
	
		val = msg.substring(7,10).toInt();	// Target Torq
		if( 20 > val ) val = 20;
		if( 99 < val ) val = 99;
		stWaveTorqVal.tgTorq = val;
	
		val = msg.substring(10,(10+4)).toInt();	//on Time
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveTorqVal.onInterval = val;

		val = msg.substring(14,(14+4)).toInt();	//Off Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveTorqVal.offInterval = val;
	}
	
	else if( '5' == ch )		// Wave Type SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( (WAVE_PWR_MAX-1) < val ) val = (WAVE_PWR_MAX-1);
		waveType = val;
		waveType_TorqSet();
	}
	else if( '6' == ch )		// Wave Type  Torq SET
	{
		val = msg.substring(4,(4+3)).toInt();	// Torq 1
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[1] = val;
	
		val = msg.substring(7,(7+3)).toInt();	// Torq 2
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[2]  = val;
	
		val = msg.substring(10,(10+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[3]  = val;

		val = msg.substring(13,(13+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[4]  = val;

		val = msg.substring(16,(16+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[5]  = val;
		
	}
	else if( '7' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Return Torq
		if( 5 > val ) val = 5;
		if( 80 < val ) val = 80;
		stWaveBdutyVal.rtTorq = val;
	
		val = msg.substring(7,10).toInt();	// Target Torq
		if( 5 > val ) val = 5;
		if( 80 < val ) val = 80;
		stWaveBdutyVal.tgTorq = val;
	
		val = msg.substring(10,(10+4)).toInt();	//on Time
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveBdutyVal.onInterval = val;

		val = msg.substring(14,(14+4)).toInt();	//Off Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveBdutyVal.offInterval = val;
	}

	
	
	else if( '9' == ch )		// Wave Type  Torq SET
	{
		val = msg.substring(4,(4+1)).toInt();	// Torq 1
		if( 0 == val)			// WAVE FUNCTION STOP
		{
			exeWave = 0;
			wave_bbn_step = WAVE_BBN_TORQ_START;
			torqMotor.setValue(DEFAULT_TORQ);
			bbnMotor.offBldc();
		}
		else				// Wave Function Start
		{
			waveType_TorqSet();
			wave_bbn_step = WAVE_BBN_TORQ_START;
			exeWave = 1;
		}
	}
	
	if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) || ('9'==ch) )	//
	{
		
		LogPrintln(" LG] Wave_ContCase: " + String(waveControlCase));
		
		LogPrintln(" LG] Angle1: " + String(stWaveServoVal.angle1) + ", Angle2: " + String(stWaveServoVal.angle2) + ", sInterval: " + String(stWaveServoVal.interval));
		
		LogPrintln(" LG] Torq1: " + String(stWaveTorqVal.rtTorq) + ", Torq2: " + String(stWaveTorqVal.tgTorq) + ", onTime: " + String(stWaveTorqVal.onInterval)+ ", offTime: " + String(stWaveTorqVal.offInterval));
		
		LogPrintln(" LG] Wave_Type: " + String(waveType) + ", Type1: "+ String(stWaveTorqVal.pwr[waveType]) );
	}
	else
	{
		
		LogPrintln(" LG] UnCorrect Format!!");
	}

}

//
//	SET BITE VAL ($F1 K~)  K: Kind
//
void Set_BiteData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		biteLogOut = 1;
	}
	else if( '1' == ch )		// Bite Control Cobination SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 3 < val ) val = 3;
		biteTestCase = val;
	}
	else if( '2' == ch )		// Toq Control	SET ( NotControl / Just/ IncDec/ withFloats)
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 3 < val ) val = 3;
		biteTorqCase = val;
		biteLogOut = 1;
	}
	else if( '3' == ch )		// Control SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 1 > val ) val = 1;
		if( 3 < val ) val = 3;
		biteCnt = val;
	}
	else if( '4' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Torq 1
		if( 60 > val ) val = 60;
		if( 100 < val ) val = 100;
		stBiteTorqVal.shallowTorq = val;
	
		val = msg.substring(7,11).toInt();	// time
		if( 300 > val ) val = 300;
		if( 5000 < val ) val = 5000;
		stBiteTorqVal.shallowInterval = val;
	
		val = msg.substring(11,14).toInt(); // Torq 2
		if( 80 > val ) val = 80;
		if( 150 < val ) val = 150;
		stBiteTorqVal.deepTorq = val;
	
		val = msg.substring(14).toInt();	// Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stBiteTorqVal.deepInterval = val;
	}
	
	else if( '5' == ch )		// BITEStrong Type SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 5 < val ) val = 5;
		biteType = val;
	}
	else if( '6' == ch )		// BITE Type  Torq SET
	{
		val = msg.substring(4,7).toInt();	// Torq 1
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[1] = val;
	
		val = msg.substring(7,10).toInt();	// Torq 2
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[2] = val;
	
		val = msg.substring(10,13).toInt(); // Torq 3
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[3] = val;
	
		val = msg.substring(13,16).toInt(); // Torq 4
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[4] = val;
	
		val = msg.substring(16,19).toInt(); // Torq 5
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[5] = val;
	
		val = msg.substring(19,22).toInt(); // Torq 6
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[6] = val;
	
		val = msg.substring(22).toInt(); // Torq 7
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[7] = val;
		
	}
	
	else if( 'A' == ch )		// Servo DEC Angle
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 50 < val ) val = 50;
		
#if IO_SERVO
		//sMotor.saveDecAngle(val);
		sMotor.maxAdjAngle = val;
#endif

		LogPrintln(" LG] Servo - DEC Angle: " + String(val));
	}
	
	if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) )	//
	{
		
		LogPrintln(" LG] BiteConbi: " + String(biteTestCase) + ", BiteTorqControl: " + String(biteTorqCase) + ", BiteCount: " + String(biteCnt));
		//
		// LogPrintln(" LG] Angle1: " + String(stWaveServoVal.angle1) + ",Angle2: " + String(stWaveServoVal.angle2) + ",sInterval: " + String(stWaveServoVal.interval));
		
		LogPrintln(" LG] shallow_Torq: " + String(stBiteTorqVal.shallowTorq) + ", sh_Intval: " + String(stBiteTorqVal.shallowInterval) +", deep_Torq: " + String(stBiteTorqVal.deepTorq) + ", deep_Intval: " + String(stBiteTorqVal.deepInterval));
		
		LogPrintln(" LG] Bite_TorqType: " + String(biteType) + ", Type1: "+ String(stBiteTorqVal.torqType[1]) + ", Type2: "+ String(stBiteTorqVal.torqType[2])  + ", Type3: "+ String(stBiteTorqVal.torqType[3]) + ", Type4: "+ String(stBiteTorqVal.torqType[4])	+ ", Type5: "+ String(stBiteTorqVal.torqType[5])  + ", Type6: "+ String(stBiteTorqVal.torqType[6])	+ ", Type7: "+ String(stBiteTorqVal.torqType[7]) );
	}
	else
	{
		
		LogPrintln(" LG] UnCorrect Format!!");
	}

}


//
void Set_HitData(String msg)
{

			int val = 0;
			char ch = msg.charAt(3);
		
			int i = 0;
		
			if( '0' == ch ) 		// View Set Value [ LOG ]
			{
				// LOG OUT
			}
  #if 0
			else if( '1' == ch )		// Bite Control Cobination SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 3 < val ) val = 3;
				biteTestCase = val;
			}
  #endif
			else if( '2' == ch )		//Servo Angle
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 90 < val ) val = 90;
				hitServoAngle = val;
			}
  #if 0
			else if( '3' == ch )		// Control SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 1 > val ) val = 1;
				if( 3 < val ) val = 3;
				biteCnt = val;
			}
  #endif
			// TARGET & RESIST
			else if( '3' == ch )		// Target SET
			{
				val = msg.substring(4,7).toInt();	// Taget Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.targetTorq = val;
				stHitTorqVal.holdonTorq = val;
		
				val = msg.substring(7,11).toInt();	// Target Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.targetInterval = val;
				stHitTorqVal.holdonInterval = val;
		
			}
			else if( '4' == ch )		// Low & Mid SET
			{
				val = msg.substring(4,7).toInt();	// low Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.lowTorq = val;
		
				val = msg.substring(7,11).toInt();	// Low Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.lowInterval = val;
		
				val = msg.substring(11,14).toInt(); //Mid Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.midTorq = val;
		
				val = msg.substring(14).toInt();	// Mid Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.midInterval = val;
			}
			
  #if 0
		
			else if( '5' == ch )		// BITEStrong Type SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 5 < val ) val = 5;
				biteType = val;
			}
  #endif
		  
			else if( '6' == ch )		// BITE Type  Torq SET
			{
				for(i = 0; i < 12; i++)
				{
					val = msg.substring((3*i)+4,(3*i)+7).toInt();	// Torq 
					if( 60 > val ) val = 60;
					if( 250 < val ) val = 250;
					stHitTorqVal.torqType[i+1] = val;
				}
				for(i = 0; i < 12; i++)
				{
					
					LogPrintln(" LG] Hit Tg_Torq " + String(i+1) + " : " + String(stHitTorqVal.torqType[i+1]) );
				}
			}
  #if 0 
			else if( 'A' == ch )		// Servo DEC Angle
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 50 < val ) val = 50;
				
			  #if IO_SERVO
				//sMotor.saveDecAngle(val);
				sMotor.maxAdjAngle = val;
			  #endif
				
				LogPrintln(" LG] Servo - DEC Angle: " + String(val));
			}
#endif
		
			else if( 'S' == ch )		// Resist & HoldOn Servo Control ENB/DIS
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				//sMotor.saveDecAngle(val);
				exeResistServoENB = val;
				val = msg.substring(6).toInt(); // 6th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				exeHoldonServoENB = val;
				
				LogPrintln(" LG] Servo - Resist ServoENB: " + String(exeResistServoENB) + ", HoldOn ServoENB: " + String(exeHoldonServoENB) );
			}
			
			else if( 'K' == ch )		// Finsh KIND SET ( AUTO or SEL)
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(Auto), else =1(Fix)
				//sMotor.saveDecAngle(val);
				fishTypeFixENB = val;
				if(fishTypeFixENB)		// if Not Auto = FIX SEL
				{
					val = msg.substring(6).toInt(); // 6th 2 char
					if( 1 > val ) val = 1;
					if( 15 < val ) val = 15;
					fishTypeFix = val;
				}
				
				LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
			}
		
			if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) )	//
			{
				
				LogPrintln(" LG] HitAngle: " + String(hitServoAngle) );
				
				
				LogPrintln(" LG] hit_tg_Torq: " + String(stHitTorqVal.targetTorq) + ", hit_tg_Intval: " + String(stHitTorqVal.targetInterval) );
				
				LogPrintln(" LG] hit_low_Torq: " + String(stHitTorqVal.lowTorq) + ", hit_low_Intval: " + String(stHitTorqVal.lowInterval) +  ", hit_mid_Torq: " + String(stHitTorqVal.midTorq) + ", hit_mid_Intval: " + String(stHitTorqVal.midInterval) );
			}
			else
			{
				
				LogPrintln(" LG] UnCorrect Format!!");
			}
}


//
void Set_BreakData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		
		LogPrintln(" LG] WaveType(1~3): " + String(waveType));
		
		LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
		
		LogPrintln(" LG] Hit Servo - Resist ENB: " + String(exeResistServoENB) + ", HoldOn ENB: " + String(exeHoldonServoENB) + ", Hit_Angle: " + String(hitServoAngle));
		
		LogPrintln(" LG] Hit_Resist_FIX: " + String(torqResistFix) + ", Torq Target: " + String(stHitTorqVal.targetTorq) + ", Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
		
		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}
	else if( '1' == ch )		// SET Break Type
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 1 > val ) val = 1;
		if( 3 < val ) val = 3;
		breakType = val;

		
		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}

/*
	else if( 'A' == ch )		// Resist Auto & FIX
	{
		val = msg.substring(4).toInt(); //
		if( 0 > val ) val = 0;
		if( 1 < val ) val = 1;
		torqResistFix = val;	// Auto(0), Fix(1)
	}
*/

}

//
// $F5 : STX_EXTLED_SET
//	$F5 1x - LED CONT ENB/DIS
//	$F5 2xxxx - Dimmming Time
//	$F5 3xxxx - HoldCount
//
void Set_ExtledData(String msg)
{
			int val = 0;
			int para1 = 0;
			char ch = msg.charAt(3);
	
			int i = 0;
		
			if( '0' == ch ) 		// View Set Value [ LOG ]
			{
				// LogPrintln();
			}
		//
		//
			else if( '1' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;		
				 ledControlMode = val;	// LED CONTROL ENB/DIS
			}
			else if( '2' == ch )		// 
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 1 > val ) val = 1;
				if( 60 < val ) val = 60;
				dimming_time = val;			// [sec]
			}
			else if( '3' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 100 > val ) val = 100;
				if( 5000 < val ) val = 5000;
				ledoffHoldTimeCnt = (unsigned int)(val/10);
			}
		/*
			else if( '4' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
				if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
				sBbnActTbl[bbnActKind].offTime = (unsigned long)val;
			}
	
			else if( '5' == ch )		// TOKTOK KIND
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( BBNACT_TBLMAX <= val ) val = 0; 	// default (SETTING)
				bbnActKind = (unsigned long)val;
			}
	
			else if( '6' == ch )		// RPT CNT
			{
				para1 = msg.substring(4,(4+1)).toInt(); // 4th 2 char
				if((BBNACT_TBLMAX-1) < para1) { para1 = (BBNACT_TBLMAX-1); }
				val = msg.substring(5).toInt(); // 5th 3 char
				if( BBN_RPT_CNT_MIN > val ) { val = BBN_RPT_CNT_MIN; }
				if( BBN_RPT_CNT_MAX < val ) { val = BBN_RPT_CNT_MAX; }
				bbnReqCnt[para1] = (unsigned long)val;
			}
			else if( '7' == ch )		// Motor WAT SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if(1 > val ) val = 1;
				if( 6 < val ) val = 6;
				torqMotor.torqWattAdj = val;
			}
			else
			{
				// Error
			}
		*/
  #if 0
		
			else if( '5' == ch )		// BITEStrong Type SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 5 < val ) val = 5;
				biteType = val;
			}
		  
			else if( '6' == ch )		// BITE Type  Torq SET
			{
				for(i = 0; i < 12; i++)
				{
					val = msg.substring((3*i)+4,(3*i)+7).toInt();	// Torq 
					if( 60 > val ) val = 60;
					if( 250 < val ) val = 250;
					stHitTorqVal.torqType[i+1] = val;
				}
				for(i = 0; i < 12; i++)
				{
					
					LogPrintln(" LG] Hit Tg_Torq " + String(i+1) + " : " + String(stHitTorqVal.torqType[i+1]) );
				}
			}
#endif
#if 0 
			else if( 'A' == ch )		// Servo DEC Angle
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 50 < val ) val = 50;
#if IO_SERVO
				//sMotor.saveDecAngle(val);
				sMotor.maxAdjAngle = val;
#endif				
				LogPrintln(" LG] Servo - DEC Angle: " + String(val));
			}
		
			else if( 'S' == ch )		// Resist & HoldOn Servo Control ENB/DIS
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				//sMotor.saveDecAngle(val);
				exeResistServoENB = val;
				val = msg.substring(6).toInt(); // 6th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				exeHoldonServoENB = val;
				
				LogPrintln(" LG] Servo - Resist ServoENB: " + String(exeResistServoENB) + ", HoldOn ServoENB: " + String(exeHoldonServoENB) );
			}
			
			else if( 'K' == ch )		// Finsh KIND SET ( AUTO or SEL)
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(Auto), else =1(Fix)
				//sMotor.saveDecAngle(val);
				fishTypeFixENB = val;
				if(fishTypeFixENB)		// if Not Auto = FIX SEL
				{
					val = msg.substring(6).toInt(); // 6th 2 char
					if( 1 > val ) val = 1;
					if( 15 < val ) val = 15;
					fishTypeFix = val;
				}
				
				LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
			}
	
#endif
		
			if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) ) //
			{
				
				LogPrintln(" LG] EXTLED_SET ENB : " + String(ledControlMode) + " [sec]");
				LogPrintln(" LG] EXTLED_SET dimm : " + String(dimming_time) + " [sec]");
				LogPrintln(" LG] EXTLED_SET hold : " + String(ledoffHoldTimeCnt*10) + " [ms]");
			/*	
				LogPrintln(" LG] BBN TokTok ACT KIND: " + String(bbnActKind) + " [ 0~6 ]");
				
				LogPrintln(" LG] BBN TokTok Rpt REQ CNT: " + String(bbnReqCnt[bbnActKind]) + " [ 1 ~ 100 ] Times");
	
				
				LogPrintln(" LG] BLDC TokTok On Duty: " + String(sBbnActTbl[bbnActKind].onDuty) + " / 255 Max");
				
				LogPrintln(" LG] BLDC TokTok On Time: " + String(sBbnActTbl[bbnActKind].onTime) + " ms");
				
				
				LogPrintln(" LG] BLDC TokTok Off Duty: " + String(sBbnActTbl[bbnActKind].offDuty) + "  / 255 Max");
				
				LogPrintln(" LG] BLDC TokTok Off Time: " + String(sBbnActTbl[bbnActKind].offTime) + " ms");
			*/
			}
			else
			{
				
				LogPrintln(" LG] UnCorrect Format!!");
			}
	
}

//
void Set_BbnTokTokData( String msg)
{
		int val = 0;
		int para1 = 0;
		char ch = msg.charAt(3);

		int i = 0;
	
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			// LogPrintln();
		}
	//
		else if( '5' == ch )		// TOKTOK KIND
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( 0 > val ) val = 0;
			if( BBNACT_TBLMAX <= val ) val = 0;		// default (SETTING)
			bbnActKind = (unsigned long)val;
		}

		else if( '6' == ch )		// RPT CNT
		{
			para1 = msg.substring(4,(4+1)).toInt(); // 4th 2 char
			if((BBNACT_TBLMAX-1) < para1) { para1 = (BBNACT_TBLMAX-1); }
			val = msg.substring(5).toInt(); // 5th 3 char
			if( BBN_RPT_CNT_MIN > val ) { val = BBN_RPT_CNT_MIN; }
			if( BBN_RPT_CNT_MAX < val ) { val = BBN_RPT_CNT_MAX; }
			bbnReqCnt[para1] = (unsigned long)val;
		}
	//
		else if( '1' == ch )		// 
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_PWM_DUTY_MIN > val ) val = BBN_PWM_DUTY_MIN;
			if( BBN_PWM_DUTY_MAX < val ) val = BBN_PWM_DUTY_MAX;
			sBbnActTbl[bbnActKind].onDuty = val;
		}
		else if( '2' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
			if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
			sBbnActTbl[bbnActKind].onTime = (unsigned long)val;
		}
		else if( '3' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_PWM_DUTY_MIN > val ) val = BBN_PWM_DUTY_MIN;
			if( BBN_PWM_DUTY_MAX < val ) val = BBN_PWM_DUTY_MAX;
			sBbnActTbl[bbnActKind].offDuty = val;
		}
		else if( '4' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
			if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
			sBbnActTbl[bbnActKind].offTime = (unsigned long)val;
		}

		else if( '7' == ch )		// Motor WAT SET
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if(1 > val ) val = 1;
			if( 6 < val ) val = 6;
			torqMotor.torqWattAdj = val;
		}
		else
		{
			// Error
		}
  #if 0
	
		else if( '5' == ch )		// BITEStrong Type SET
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( 0 > val ) val = 0;
			if( 5 < val ) val = 5;
			biteType = val;
		}
 	  
		else if( '6' == ch )		// BITE Type  Torq SET
		{
			for(i = 0; i < 12; i++)
			{
				val = msg.substring((3*i)+4,(3*i)+7).toInt();	// Torq 
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.torqType[i+1] = val;
			}
			for(i = 0; i < 12; i++)
			{
				
				LogPrintln(" LG] Hit Tg_Torq " + String(i+1) + " : " + String(stHitTorqVal.torqType[i+1]) );
			}
		}
#endif
#if 0 
		else if( 'A' == ch )		// Servo DEC Angle
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( 0 > val ) val = 0;
			if( 50 < val ) val = 50;
#if IO_SERVO
			//sMotor.saveDecAngle(val);
			sMotor.maxAdjAngle = val;
#endif

			LogPrintln(" LG] Servo - DEC Angle: " + String(val));
		}
	
		else if( 'S' == ch )		// Resist & HoldOn Servo Control ENB/DIS
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
			//sMotor.saveDecAngle(val);
			exeResistServoENB = val;
			val = msg.substring(6).toInt(); // 6th 2 char
			if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
			exeHoldonServoENB = val;
			
			LogPrintln(" LG] Servo - Resist ServoENB: " + String(exeResistServoENB) + ", HoldOn ServoENB: " + String(exeHoldonServoENB) );
		}
		
		else if( 'K' == ch )		// Finsh KIND SET ( AUTO or SEL)
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 0 != val ) val = 1; 	// 0 = 0(Auto), else =1(Fix)
			//sMotor.saveDecAngle(val);
			fishTypeFixENB = val;
			if(fishTypeFixENB)		// if Not Auto = FIX SEL
			{
				val = msg.substring(6).toInt(); // 6th 2 char
				if( 1 > val ) val = 1;
				if( 15 < val ) val = 15;
				fishTypeFix = val;
			}
			
			LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
		}

#endif
	
		if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) )	//
		{
			LogPrintln(" LG] TORQ MOT WATT : " + String(torqMotor.torqWattAdj) + " [ 1:10W,2:20W,3:30W]");
			LogPrintln(" LG] BBN TokTok ACT KIND: " + String(bbnActKind) + " [ 0~6 ]");
			LogPrintln(" LG] BBN TokTok Rpt REQ CNT: " + String(bbnReqCnt[bbnActKind]) + " [ 1 ~ 100 ] Times");
			LogPrintln(" LG] BLDC TokTok On Duty: " + String(sBbnActTbl[bbnActKind].onDuty) + " / 255 Max");
			LogPrintln(" LG] BLDC TokTok On Time: " + String(sBbnActTbl[bbnActKind].onTime) + " ms");
			LogPrintln(" LG] BLDC TokTok Off Duty: " + String(sBbnActTbl[bbnActKind].offDuty) + "  / 255 Max");
			LogPrintln(" LG] BLDC TokTok Off Time: " + String(sBbnActTbl[bbnActKind].offTime) + " ms");
		}
		else
		{
			LogPrintln(" LG] UnCorrect Format!!");
		}
}

/*

*/
void Set_TorqAdjValData( String msg)
{
		int val = 0;
		char ch = msg.charAt(3);
		
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			
			LogPrintln(" LG] Type: " + String(ch) + ",	");
			
			LogPrintln(" LG] *Torq Adj Curr Val: " + String(torqMotor.setAdjTorq));
		}
		else if( '1' == ch )		// SET Torq Default Val
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 30 > val ) val = 30;
			if( 80 < val ) val = 80;
			torqMotor.defaultVal = val;
			
			LogPrintln(" LG] Set Torq Default Val, *ADJ Curr Val : " + String(torqMotor.setAdjTorq) );
			
			LogPrintln(" LG] *ADJ Set Val : " + String(val) );
		}
	
		else if( '2' == ch )		// SET Torq Adj(Dec) Val
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 1 > val ) val = 1;
			if( 50 < val ) val = 50;
			
			LogPrintln(" LG] Set Torq ADJ Val, *ADJ Curr Val : " + String(torqMotor.setAdjTorq) );
			
			LogPrintln(" LG] *ADJ Set Val : " + String(val) );
			torqMotor.setAdjTorq = val;
		}
	/*
		else if( 'A' == ch )		// Resist Auto & FIX
		{
			val = msg.substring(4).toInt(); //
			if( 0 > val ) val = 0;
			if( 1 < val ) val = 1;
			torqResistFix = val;	// Auto(0), Fix(1)
		}
	*/
		else
		{
			
			LogPrintln(" LG] Set Torq Undefine Val, " + String(msg) );
		}

}


//
//	$FAA~
//	A - Action
//		0 - Log View
//		1 - Light
//			~ : xxx : Light 10~100 [%]
//
void Set_LedValue( String msg)
{
		unsigned int val = 0;
		int pos = 0;
		char ch = msg.charAt(3);
		
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			
		}
		else if( '1' == ch )		// SET Led Light
		{
			val = (unsigned int)msg.substring(5,(5+3)).toInt(); // 4th 2 char
			if( 1 > val ) val = 1;
			if( 100 < val ) val = 100;
			ledLight = val;
		}

		
		else if( '2' == ch )		// SET HoldTIme $FA2xZZZZ
		{
			pos = (unsigned int)msg.substring(4,(4+1)).toInt(); // 4th 1 char, led Pos
			if( 0> pos) { pos=0; } if( 4<pos ) { pos =0; }
			
			val = (unsigned int)msg.substring(5,(5+4)).toInt();	// 5th~9th 4_Char = Hold TIme
			if( 10 > val ) val = 10;
			if( 9999 < val ) val = 9999;

			ledDimmHoldTime[pos] = val;
			
		}
	/*
		else if( 'A' == ch )		// Resist Auto & FIX
		{
			val = msg.substring(4).toInt(); //
			if( 0 > val ) val = 0;
			if( 1 < val ) val = 1;
			torqResistFix = val;	// Auto(0), Fix(1)
		}
	*/
		else
		{
			
			LogPrintln(" LG] Set Torq Undefine Val, " + String(msg) );
		}

	if( '0' == ch || '1'==ch || '2'==ch || '3'==ch) 		// View Set Value [ LOG ]
	{
		LogPrintln(" LG] Set LED Light= " + String(ledLight) + "/%" );
		LogPrintln(" LG] Set LED HoldTime 1234: " + String(ledDimmHoldTime[1]) + "," + String(ledDimmHoldTime[2]) + "," + String(ledDimmHoldTime[3]) + "," + String(ledDimmHoldTime[4]) + ","  );
	}
}

// MKT TUN HIT & HOLD
// $F9aXXXyy
//	a : 0- Val view, 1-hit, 2-hold, 3-Test Stop
// XXX : Main Mot Power ( 60 ~ 255)
// yy : Bobbin Mot Power (10 ~ 70)
void Set_TorqHitHoldOn( String msg)
{

#if (MKT_TEST_1)

	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{

	}
	else if( '1' == ch )		// SET HIT
	{
		f9setFlag = 1; // TEST SET

		val = msg.substring(5,(5+3)).toInt(); // Hit MAIN
		if( 60 > val ) val = 60;
		if( 180 < val ) val = 180;
		HitMainTorq = val;

		val = msg.substring(9,(9+2)).toInt(); // Hit BBN
		if( 10 > val ) val = 10;
		if( 70 < val ) val = 70;
		sBbnActTbl[BBNACT_HIT].onDuty = val;
	}

	else if( '2' == ch )		// SET HOLDON
	{
		f9setFlag = 1; // TEST SET

		val = msg.substring(5,(5+3)).toInt(); // HOLD MAIN
		if( 60 > val ) val = 60;
		if( 180 < val ) val = 180;
		HoldMainTorq = val;

		val = msg.substring(9,(9+2)).toInt(); // HOLD BBN
		if( 10 > val ) val = 10;
		if( 70 < val ) val = 70;
		sBbnActTbl[BBNACT_HOLDON].onDuty = val;		
	}
	else if( '3' == ch )		// TEST CLR
	{
		f9setFlag = 0; // TEST CLR
		HitMainTorq = 100;
		HoldMainTorq = 120;
		sBbnActTbl[BBNACT_HIT].onDuty = 40;
		sBbnActTbl[BBNACT_HOLDON].onDuty = 46;
	}
	else
	{
		LogPrintln(" LG] F_SET Not Find CMD " + String(msg) );
	}

	if( '0' == ch || '1'==ch || '2'==ch || '3'==ch) 		// View Set Value [ LOG ]
	{
		LogPrintln(" LG] MKT_TEST_1 Test: " + String(f9setFlag));	
		LogPrintln(" LG] F_SET HIT_> MAIN:" + String(HitMainTorq) + " BBN:" + String(sBbnActTbl[BBNACT_HIT].onDuty));
		LogPrintln(" LG] F_SET HOLD> MAIN:" + String(HoldMainTorq) + " BBN:" + String(sBbnActTbl[BBNACT_HOLDON].onDuty));
	}

#endif

}

void Set_EnbData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		LogPrintln(" LG] WaveType(1~3): " + String(waveType));
		LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
		LogPrintln(" LG] Hit Servo - Resist ENB: " + String(exeResistServoENB) + ", HoldOn ENB: " + String(exeHoldonServoENB) + ", Hit_Angle: " + String(hitServoAngle));
		LogPrintln(" LG] Hit_Resist_FIX: " + String(torqResistFix) + ", Torq Target: " + String(stHitTorqVal.targetTorq) + ", Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
//		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}
	else if( '1' == ch )		// SET Level (Easy/Normal/Hard)
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( LEVEL_MIN > val ) val = LEVEL_MIN;
		if( LEVEL_MAX < val ) val = LEVEL_MAX;
		gameLevel = val;
		
		LogPrintln(" LG] game Level, gameLevel: " + String(gameLevel) );
	}

	else if( 'A' == ch )		// Resist Auto & FIX
	{
		val = msg.substring(4).toInt(); //
		if( 0 > val ) val = 0;
		if( 1 < val ) val = 1;
		torqResistFix = val;	// Auto(0), Fix(1)
	}
}


//=== TEST Seral Event ===
#if 0		// NG
void serialEvent()
{
    
	LogPrintln(" LG] Serial Event 000000 ~");
}

void serialEvent1()
{
    
	LogPrintln(" LG] Serial Event 111111 ~");
}
#endif

#if 0
//
//
//
unsigned short pwrSwcOnCnt = 0;
unsigned short pwrSwcOffCnt = 0;
unsigned short pwrSwcTECnt = 0;
unsigned short pwrSwcLECnt = 0;

unsigned short subAcOffSeq = 0;

int usb5V_LVL_OFFSTART = -1;	// unknown

#define TO_OVER_FILTER	5	// 100ms * Continue 5 Times
#define TO_EDGE_FILTER_CNT	5	// 50 ms ( 20ms + 30 ms After LE/TE Occure ]

int pwrSafyTimeSet = 0;
int pwrSafyTimeCnt = 0;

//
//
//
void check_PowerSwitch_Change_10ms()
{

	//1) Check LE/TE ( Change Level )
	if(LE10_PWROFF_SWC && LVL10_PWROFF_SWC)			// [ LE & HIGH ] = PWR OFF ?
	{
		pwrSwcLECnt = 1;
		pwrSwcTECnt = 0;		// CLR
	  #if LOG_PWRSWC_OFFON
		LogPrintln(" LG] PWR_C SWC OFF");
	  #endif
	}
	else if(TE10_PWROFF_SWC && !(LVL10_PWROFF_SWC))	// [ TE & LOW ] = PWR ON ?
	{
		pwrSwcLECnt = 0;		// LE CLR
		pwrSwcTECnt = 1;
	  #if LOG_PWRSWC_OFFON
	  	LogPrintln(" LG] PWR_C SWC ON");
	  #endif
	}

	//2-1) Check Filter Counter [LE] - PWR_OFF
	if(0 < pwrSwcLECnt)
	{
	  // SWC ON => OFF
	  if((LVL10_PWROFF_SWC))	// HIGH-OFF
	  {
		if(TO_EDGE_FILTER_CNT > pwrSwcLECnt)		// 50ms Filter : HIGH 유지
		{
			pwrSwcLECnt++;	// Count Up
		}
		else
		{
		  pwrSwcLECnt = 0;
		  pwrSwcTECnt = 0;
		  //TE EDGE = [ ]AC OFF SEQ ]Control
		  subAcOffSeq = 1;
		  // 1) PC OFF Command SEND to AP(FG)
		  
		  StsSendPrintln(STS_APSTS_REQ+"00%");		  // PC OFF REQ
		  //StsSendPrintln("$1700%");		  // PC OFF REQ
		  
		  recvPwrOffExec_Resp = 0;	  // CLR
		  subAcOff_Control_Start();
		#if LOG_PWRSWC_OFFON
		  LogPrintln(" LG] PWR_C SWC_OFF Start");
		#endif
		  // USB 5V ON/OFF Status Check at AC OFF SEQ Start
		  if(LVL10_USB5V)
		  {
			  usb5V_LVL_OFFSTART = 1;	  // ON save
		  }
		  else
		  {
			  usb5V_LVL_OFFSTART = 0;	  // OFF save
		  }

		}
	  }
	  // SWC_ON
	  else
	  {
	  	pwrSwcLECnt = 0;		// CLR COUNTER
	  }
	}


	// 2-2) Check Filter Counter [TE] - PWR_ON
	if(0 < pwrSwcTECnt)
	{
		// PWR SWITCH OFF => ON
	  if(!LVL10_PWROFF_SWC)		// LOW-ON
	  {
		if(TO_EDGE_FILTER_CNT > pwrSwcTECnt)		// 50ms LOW유지 FILTER
		{
			pwrSwcTECnt++;	// Count Up
		}
		else
		{
			//LE EDGE  =	AC ON Control l
			clrPwrOffSeq();
			subAC_On();
		#if LOG_PWRSWC_OFFON
			LogPrintln(" LG] PWR_C SWC_ON Once:Off Cancel");
		#endif

			// 전원 Switch ON (하루에 아침에 한번 ON할때 마다, 메인보드 리셋) - 하루에 한번은 리셋한다.
			//= 전원스위치를 OFF => ON으로 변화될떄, 메인 보드를 SOFT 리셋한다.
			// 25/5/21
			#if (PWR_SW_OFF_CPU_SOFT_RESET)		// 모든CASE검토후 도입예정
				delay(10);
				esp_soft_reset();
			#endif
		
		}
	  }
	  else
	  {
	  	pwrSwcTECnt = 0;
	  }
	}

	// SUB AC OFF SEQ Statrt
	
}

// TO 10SEC Force AC OFF
unsigned int pwr_off_step = 0;
#define FORCE_OFF_15SEC	(15*1000)		// 15 SEC
#define FORCE_OFF_2MIN	(2*60*1000)		// 2 MIN ( with Windows Update)
#define ACOFF_500MS_AFTER_USB5VOFF	(500)		// 500ms After USB5VOFF

//
void subAcOff_WithOtherOff()
{
	subAC_Off();		// OFF Output
	pwr_off_step = 0; 	// SET IDLE
	clrPwrOffSeq();
	// LED OFF
	ledOff_MbCent();
	ledOff_MbBtm();
	ledOff_OptLeft();
	ledOff_OptRight();
}
//
//
void clrPwrOffSeq()
{
	subAcOffSeq = 0;
	
	pwr_off_step = 0;
	
	usb5V_LVL_OFFSTART = -1;
	recvPwrOffExec_Resp = 0;
	
	pwrSwcLECnt = 0; 	// CLR
	pwrSwcTECnt = 0; 	// CLR
}



void subAcOff_Control_Start()
{
	
	pwr_off_step = 10; 	// START
}

//void forceOff_Control()
//	CALL 10ms
//
void subAcOff_Control()
{
	static int _to;

		switch(pwr_off_step)
		{
			// IDLE
			case 0:
				break;

			// START
			case 10:
				setTO(_to);
				pwr_off_step = 20;
				break;

			// CHECK Normal OFF or TO Force OFF
			case 20:
				//if(checkTO(_to, FORCE_OFF_15SEC))
				if(checkTO(_to, FORCE_OFF_2MIN))
				{
					subAcOff_WithOtherOff();
					#if LOG_POWEROFF_FUNC
						LogPrintln(" LG] PWROF Force Off 2 MIN");
					#endif
					pwr_off_step = 0;
					
				}
				else if(recvPwrOffExec_Resp && (LOW == LVL10_USB5V))
				{
					recvPwrOffExec_Resp = 0;
					setTO(_to);
					pwr_off_step = 30;
				}
				break;
				
			case 30:
				if(checkTO(_to, ACOFF_500MS_AFTER_USB5VOFF))
				{
					subAcOff_WithOtherOff();
					#if LOG_POWEROFF_FUNC
						LogPrintln(" LG] PWROF Recv OFF & USB 5V OFF");
					#endif

					pwr_off_step = 0;
				}
				break;
				
			default:
				break;

		}
}
#endif

// TO 10SEC Force AC OFF
//#define POWER_ON_WAIT		(g_PwrOnDelayTime*1000)	// 10 SEC
//#define BOOTING_WAIT		(g_UsbSafeTime*1000)	// 28 SEC
#define POWER_ON_WAIT		(DEF_PWRON_DELAY_TIME*1000)	// 10 SEC
#define BOOTING_WAIT		(DEF_USB_SAFE_TIME*1000)	// 28 SEC
#define FORCE_OFF_2MIN		(2*60*1000)				// 2 MIN ( with Windows Update)
#define RESP_WAIT_TIME		(500)					// 500ms After USB5VOFF
#define AUTO_REBOOT_TIME	(14*1000)				// 14초 After AP Running
#define APP_CHECK_TIME		(18*1000)				// APP 커맨드 체크 타임(18초동안 폴커맨드등 없을 때) 
#define ACOFF_500MS_AFTER_USB5VOFF	(500)			// 500ms After USB5VOFF
#define TO_EDGE_FILTER_CNT	5						// 50 ms ( 20ms + 30 ms After LE/TE Occure]
#define	PWR_SW_ON		1
#define	PWR_SW_OFF		2

enum {
	PS_INITIAL = 0,				// 보드 초기화 상태
	PS_ON_WAIT,					// 전원스위치 켜짐 & 전원 오프 타임 아웃 10초 대기
	PS_BOOTING,					// 부팅중
	PS_RUNNING,					// AP 실행중
//	PS_STOPPING,				// AP 종료(인위적 종료)
	PS_RSPWAIT,					// 전원오프 커맨드 AP에 전송후 응답 대기
	PS_USBWAIT,					// USB 5V OFF 대기
	PS_OFFWAIT					// USB 5V OFF 후 500ms 대기
};
int	g_ProcStatus = 0;					// INIT
unsigned long	g_TmRTM = 0;
unsigned long	g_TmNTM = 0;
unsigned long	g_TmFTM = 0;
unsigned long	g_TmBTM = 0;
unsigned int	g_PSON_Count = 0;
unsigned int	g_PSOF_Count = 0;
unsigned int	g_USB5V_Count = 0;
//
void subAcOff_WithOtherOff()
{
	subAC_Off();		// OFF Output
	// LED OFF
	ledOff_MbCent();
	ledOff_MbBtm();
	ledOff_OptLeft();
	ledOff_OptRight();
	g_RecvCmdCount = 0;
//	g_FlagCmdStop = 0;
}

int CheckAppCmd()
{
//	if (g_FlagCmdStop && (g_RecvCmdCount < 2))							// 종료 커맨드 수신이고 커맨드 개수가 2미만이면
//		return PS_STOPPING;												// AP 종료중 리턴
//	else
	{
		if ((g_RecvCmdCount > 2) && !checkTO(g_LastCmdTick, 4000)) {	// 커맨드 수신 카운트가 2보다 크고 최종수신 시간이 4초이내이면
//			g_FlagCmdStop = 0;											// AP 종료중 플래그 클리어
			return 1;													// AP 실행중 리턴
		}
	}
	return 0;
}

int CheckPowerSwitch()
{
	if (LVL10_PWROFF_SWC)						// 전원스위치 OFF(HIGH-OFF) 이면
	{
		g_PSON_Count=0;							// On Count clear
		g_PSOF_Count++;							// Off Count +1
		if (g_PSOF_Count > TO_EDGE_FILTER_CNT)	// Off Count가 5보다 큰가
			return PWR_SW_OFF;					// 전원 스위치 OFF 리턴
	}
	else
	{
		g_PSON_Count++;							// On Count +1
		g_PSOF_Count=0;							// Off Count clear
		if (g_PSON_Count > TO_EDGE_FILTER_CNT)	// On Count가 5보다 큰가
			return PWR_SW_ON;					// 전원 스위치 ON 리턴
	}
	return 0;
}

String StsStr[] = {  "PS_INITIAL", "PS_ON_WAIT", "PS_BOOTING", "PS_RUNNING",
					 "PS_RSPWAIT", "PS_USBWAIT", "PS_OFFWAIT" };
#define	SnsStr(x)	("ACOFF("+String((x >> SNS_AcOff) & 1) + ") USB5V("+String((x >> SNS_usb5V) & 1) + ") PWOFF("+String((x >> SNS_PwrOffSwc) & 1)+")")

void check_PowerSwitch_Change_10ms()
{
	static int	SaveStatus = -1;
	static	int	oldSns = 0;
	int newSns = LVL10_PWRALL;
	int chgSns = oldSns ^ newSns;
	if (chgSns) {
		LogPrintln(" LG] CHKSNS : " + SnsStr(newSns));
		oldSns = newSns;
	}
	switch (g_ProcStatus)
	{
	case PS_INITIAL :		// 초기 상태
		if (CheckPowerSwitch() == PWR_SW_ON) {		// 전원 스위치 ON이면
			initSet_LedContVal(1);					// LED VAL INIT
			ledOn_MbBtm();							//
			pwrMode = PWR_ON;						// LED를 켠다.
			g_ProcStatus = PS_ON_WAIT;				// PC 안정화 시간(10초) 보장 PC ON 대기로 상태 전환
		}
		else {
			if ((g_SerialEnable == 0) && 			// USB CDC가 disable 이면서
				checkTO(g_TmBTM, BOOTING_WAIT)) {	// 부팅 안정화 시간이 지났으면
				SerialPortEnable();					// USB CDC Enable
			}
		}
		break;
	case PS_ON_WAIT :		// PC ON 대기
		if (CheckPowerSwitch() == PWR_SW_OFF) {		// 전원 스위치 OFF이면
		//	initSet_LedContVal();					// LED VAL INIT
			extLed_AllOff();
			extLed_controlOff();
			pwrMode = PWR_OFF;						// LED를 끈다.
			g_ProcStatus = PS_INITIAL;				// PC 안정화 시간(10초) 보장 PC ON 대기로 상태 전환
		}
		else if (checkTO(g_TmNTM, POWER_ON_WAIT)) {	// 안정화 시간이 지났으면
			if (g_SerialEnable) {
				SerialPortDisable();				// USB CDC Disable
			}
			subAC_On();								// PC 등 AC 전원 ON
			setTO(g_TmBTM);							// PC 부팅 타이머 설정
			g_ProcStatus = PS_BOOTING;				// PC 부팅중으로 상태 전한
		}
		break;
	case PS_BOOTING :		// PC 부팅중
		if (g_SerialEnable || checkTO(g_TmBTM, BOOTING_WAIT)) {		// 부팅 안정화 시간이 지났으면
			if (g_SerialEnable == 0) {
				SerialPortEnable();					// USB CDC Enable
			}
			g_ProcStatus = PS_RUNNING;				// PC 부팅중으로 상태 전한
		}
		break;
	case PS_RUNNING :		// AP 실행중
		if ((CheckPowerSwitch() == PWR_SW_OFF) ||	// 
			(g_AutoReboot && CheckAppCmd() && checkTO(g_TmFTM, AUTO_REBOOT_TIME))) {	// TEST AUTO REBOOT
			recvPwrOffExec_Resp = 0;				// 수신 플래그 클리어
			StsSendPrintln(STS_APSTS_REQ+"00%");	// PC OFF REQ 커맨드 송신
			setTO(g_TmFTM);							// FTM(2분) 설정(강제종료 타이머)
			setTO(g_TmRTM);							// RTM(500ms) 설정(응답 타이머)
			g_ProcStatus = PS_RSPWAIT;				// 응답 대기로 상태 전환
		}
		break;
	case PS_RSPWAIT :		// 응답 대기
		if(recvPwrOffExec_Resp)						// PC off 커맨드 응답 수신이면
		{
			recvPwrOffExec_Resp = 0;				// 수신 플래그 클리어
			g_ProcStatus = PS_USBWAIT;				// USB 5V LOW 대기
		}
		else if (checkTO(g_TmFTM, FORCE_OFF_2MIN))	// 강제 종료 시간(2분) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		else if (checkTO(g_TmRTM, RESP_WAIT_TIME))	// 응답 시간(500ms) 경과이면
		{
			if ((CheckPowerSwitch() == PWR_SW_ON)	// 전원 스위치가 다시 On이고
				&& (CheckAppCmd() == 0)) {			// AP가 실행 안되고 있으면
				g_ProcStatus = PS_RUNNING;			// 초기 상태로 전환
			}
			else {
				StsSendPrintln(STS_APSTS_REQ+"00%");// PC OFF REQ 재전송
				setTO(g_TmRTM);						// RTM(500ms) 설정(응답 타이머)
			}
		}
		break;
	case PS_USBWAIT :		// USB 5V LOW 대기
		if(LOW == LVL10_USB5V)						// USB 5V가 LOW 인가
		{
			setTO(g_TmRTM);							// RTM(500ms) 설정(전원OFF 대기 시간)
			g_ProcStatus = PS_OFFWAIT;				// 전원 OFF 대기 상태로 전환
		}
		else if (checkTO(g_TmFTM, FORCE_OFF_2MIN))	// 강제 종료 시간(2분) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		break;
	case PS_OFFWAIT :		// 전원 OFF 대기
		if (checkTO(g_TmRTM, RESP_WAIT_TIME))		// 전원OFF 대기 시간(500ms) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		break;
	default :				// 디폴트(정의 안된 상태의 경우)
		LogPrintln(" LG] CHKPWR : PS_UNKNOWN("+ String(g_ProcStatus) + ") -> PS_INITIAL");
		g_ProcStatus = PS_INITIAL;					// 초기 상태로 전환
		break;
	}
	if (SaveStatus != g_ProcStatus) {
		LogPrintln(" LG] CHKPWR : " + ((SaveStatus==-1) ? "PS_UNKNOWN" : StsStr[SaveStatus]) + " -> " + StsStr[g_ProcStatus]);
		SaveStatus = g_ProcStatus;
	}
}


// IMU Data START/STOP Cmd Send to ROD
void imuDataOutCmdSend(int onOff)
{
	String msg = String(onOff);
	//if(IMU_STOP == stop) { 	msg = STX_IMU_SET + "00000000"; }
	//else{ 					msg = STX_IMU_SET + "11111111"; }
	eNow.write(STR_PID_IMU_DATA_OUT, msg);		// Control & LOG
}

//
// PC-MAIN간 USB케이블이 빠지는 경우, 출력(모터)를 모두 OFF
void usb5vChange_Check()
{
	if(TE10_USB5V)
	{
		// MOTOR ALL OFF
		motor_AllOff();		// MAIN ALL OFF

		reelOut_AllOff();	// REEL ALL OFF

		LogPrintln(" LG] InTST usb5v TE");

		#if (NOT_USE_CODE)
			// 대기중에 텐션 유지
			//stbyTorqOffControl_Start();
		#endif
	}
}
//----------------------------------------------------------------------
//
//---------------------------------------------------------------------
#define DICSIZ	    4096
#define HASHSIZ     1001U
#define MAXLEN	    18

int	DecodeLZ(unsigned char* src, unsigned char* dst, int opt)
{
	short   lst_r, i, mask, flag;
	unsigned short  pos, len, pos1, len1;
	unsigned char	Dic[DICSIZ + MAXLEN];
	unsigned int	incount, outcount, filesize, posi=0;

	lst_r = outcount = mask = 0;

	filesize = src[0] * 0x100 + src[1];
	if (opt && (filesize == opt))
		goto NoCompress;
	incount = 2;

	while (outcount < filesize) {
		if (mask == 0) {
			flag = src[incount++];
			mask = 0x80;
		}
		if (mask & flag) {
			Dic[lst_r] = src[incount++];
			dst[outcount++] = Dic[lst_r];
			if (++lst_r == DICSIZ) lst_r = 0;
		}
		else {
			pos = src[incount++];
			len = src[incount++];
			pos = ((pos << 8) + len) >> 4;
			len = (len & 0x0f) + 3;
			if (outcount >= filesize)
				goto NoCompress;
			for (i = 0; i < len; i++, outcount++) {
				if (outcount >= filesize)
					goto NoCompress;
				dst[outcount] = Dic[pos];
				Dic[lst_r] = Dic[pos];
				if (++pos == DICSIZ) pos = 0;
				if (++lst_r == DICSIZ) lst_r = 0;
			}
		}
		mask >>= 1;
	}
	return (outcount);
NoCompress:
//DbgPrintf(" DN] DecodeLZ: NoCompress(%d) opt(%d) filesize(%d) incount(%d) outcount(%d)\r\n", posi, opt, filesize, incount, outcount);
	memcpy(dst, src, filesize);
	return filesize;
}

// $DN+d(1)+xx(2)+nn(2)+data(nn)+csum(2)+%
//  d : 'M'=Main 비압축, 'm'=Main 압축, 'R'=Rod 비압축, 'r'=Rod 압축
//  xx: "ED"=End, "00"~"FF"=Seq No
//  nn: data length(hexa-big endian)
//  data: binary data(hexa)
//  csum: data check sum(hexa-big endian)

// (V1081) 다운로드 실행 함수
void Exec_Download(String msg)
{
	int act;
	int retc = 0;
	int paraNG = 0;		// 0-OK
	int	nSeqNo;
	int DnDataLen = g_DnSaveLen[g_DnExecIdx];
	unsigned char sChr;
	String respMsg;
	
	LogPrintf(" DN] Exec_Download() : nDataLen(%d) RecvSeq(%d) DataSum(%x)\r\n", DnDataLen, g_DnRecvSeq, g_DnDataSum);
	if (g_DnRecvSeq == 999) {					// Stop Packet
		Update.end(true);							// 시스템 OTA 세션 정상 종료
		LogPrintln(" DN] Update.end(true), Statck 여유:" + String(uxTaskGetStackHighWaterMark(NULL)));
		delay(20);
		esp_soft_reset();
		g_DnExecFlag = 0;
		return;
	}
	else if (g_DnRecvSeq == 998) {				// Stop Packet
		LogPrintln(" DN] Update.end(false)");
		Update.end(false);							// 시스템 OTA 세션 이상 종료
		for (int i=0; i < DN_MAX_BUF; i++)
			g_DnSaveLen[i] = 0;
		g_DnExecIdx = g_DnSaveIdx;
		g_DnExecFlag = 0;
		return;
	}
	if (g_DnRecvSeq == 0) {
		if (g_DnExecFlag || Update.hasError()) {
			LogPrintln(" DN] Update.end(false)");
			Update.end(false);							// 시스템 OTA 세션 정리
		}
		LogPrintln(" DN] Update.begin(UPDATE_SIZE_UNKNOWN), Statck 여유:" + String(uxTaskGetStackHighWaterMark(NULL)));
		if (Update.begin(UPDATE_SIZE_UNKNOWN)) {		// 시스템 OTA start
		//if (true) {
			g_DnExecFlag = 1;
			g_DnRecvNxt = 0;
		}
		else {
			paraNG = 1;
		}
	}
	if ((g_DnRecvNxt != g_DnRecvSeq) || g_DnDataSum) {
		paraNG = 2;
		LogPrintf(" DN] g_DnRecvNxt(%d) != g_DnRecvSeq(%d) or g_DnDataSum(%d)\r\n", g_DnRecvNxt, g_DnRecvSeq, g_DnDataSum);
	}
	else {
		g_DnRecvNxt++;
	}
	respMsg = msg.substring(0,(0+7)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);
	Serial.flush();
//	LogPrintln(" DN] ApRsp : " + respMsg);
	if (g_DnExecFlag && (g_DnDataSum == 0)) {					//
		if (msg[3] == 'm' || msg[3] == 'r') {
			DnDataLen = DecodeLZ(g_DnSaveBuf[g_DnExecIdx], g_DnExecBuf, g_DnSaveLen[g_DnExecIdx]);
		//	LogPrintf(" DN] DecodeLZ(%d) : %d->%d\r\n", g_DnRecvSeq, g_DnSaveLen[g_DnExecIdx], DnDataLen);
		}
		else {
			DnDataLen = g_DnSaveLen[g_DnExecIdx];
			memcpy(g_DnExecBuf, g_DnSaveBuf[g_DnExecIdx], DnDataLen);
		}
		g_DnSaveLen[g_DnExecIdx] = 0;
		g_DnExecIdx = (g_DnExecIdx+1) & (DN_MAX_BUF-1);
		//LogPrintln(" DN] Update.write(" + String(DnDataLen) + ")");
		if (DnDataLen) {
			Update.write(g_DnExecBuf, DnDataLen);
		}
	}
}

//---------------------------------------------------------
//	CONTROL STEP LOG CURR_STEP
//--------------------------------------------------------
void Log_Control_Step()
{
	
#if LOG_CONT_STEP
		// LED Step
		LogPrintln(" LG] ext_led_dim_step " + String(ext_led_dim_step));
		// bldc Mot Stop
		LogPrintln(" LG] bbnMotorStop_Step " + String(bbnMotorStop_Step));
	
		// Wave
		LogPrintln(" LG] wave_tq_step " + String(wave_tq_step));
		LogPrintln(" LG] wave_bbn_tq_step " + String(wave_bbn_tq_step));

		LogPrintln(" LG] wave_torq_bbn_step " + String(wave_torq_bbn_step));
		
	
		// Bite Step
		LogPrintln(" LG] bite_wf_step " + String(bite_wf_step));
	
		// Bite BLDC
		LogPrintln(" LG] bite_justTorq_step " + String(bite_jt_step));
		LogPrintln(" LG] bite_incTorq_step " + String(bite_it_step));
		LogPrintln(" LG] bite_bldc_step " + String(bite_bldc_step));
	
			// Resist
			LogPrintln(" LG] hitResistRdStep " + String(hitResistRdStep));
			LogPrintln(" LG] hit_resist_step " + String(hit_resist_step));
			
			// HIT
			LogPrintln(" LG] hit_BldcStep " + String(hit_BldcStep));
			LogPrintln(" LG] hit_brk_step " + String(hit_brk_step));
	
		// Fight
		LogPrintln(" LG] bbnFight_step " + String(bbnFight_step));
	
			// HOLD ON
			LogPrintln(" LG] hit_holdon_step " + String(hit_holdon_step));
			LogPrintln(" LG] holdon_bldc_step " + String(holdon_bldc_step));
			
		// Randing
		LogPrintln(" LG] randingStep " + String(randingStep));
		// Fast Hold On
		LogPrintln(" LG] fastHoldStep " + String(fastHoldStep));
		// Button Combo
		LogPrintln(" LG] btnComboStep " + String(btnComboStep));
	
	
		// Ready INC
		LogPrintln(" LG] rdy_incStep " + String(rdy_incStep)); 
		
		
		// Pwr_Off Step
	//	LogPrintln(" LG] pwr_off_step " + String(pwr_off_step));		// (V108)
				
#endif
}

//
//
//	CALL 10ms
void reelAllOff_SendControl()
{
	static int _to;

	switch(reelAlloffStep)
	{
		// IDLE
		case 0:
			break;

		// START
		case 10:
			setTO(_to);
			reelAlloffStep = 20;	// NA
			break;
		
		case 20:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				imuDataOutCmdSend(IMU_DATA_OFF);	// IMU OUT OFF
				setTO(_to);
				reelAlloffStep = 30;
			}
			break;
		
		case 30:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				sendRodVrtCmd(0,0,0,0); 	// Vrt Mot OFF
				setTO(_to);
				reelAlloffStep = 40;
			}
			break;
		
		case 40:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				sendRodBtnLedCmd(0,0,0,0,0);	// BTN LED ALL OFF
				setTO(_to);
				reelAlloffStep = 50;
			}
			break;

		case 50:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				breakMotorOff();				// BREAK MOT OFF
				setTO(_to);
				reelAlloffStep = 90;	// END
			}
			break;
		
		case 90:
			// END
			reelAlloffStep = 0;			// STEP IDLE
			reelOut_AllOff_Flag = 0;	// FLAG CLR
			break;
			
		default:
			break;
	}
}


//--------------------------------------------------------------
//WIRE_HOME_BBN_CNT
//--------------------------------------------------------------

// 후킹, 랜딩시 개시
void lmLongWireCheck_Start()
{
	wireLongCheck_Flag = 1;
	currWirePulse = mainEnc.bbnCnt;	// 현재 Pulse Count저장

	#if (LOG_LM_WIRE_CONTROL)
		LogPrintln(" lg] WIRE Check START:" + String(currWirePulse));
	#endif
}

// Lm Wire Check , Hit=>버티기, Fail=>Casting대기
void lmLongWireCheck_Stop()
{
	if(wireLongCheck_Flag)
	{
		wireLongCheck_Flag = 0;
		#if (LOG_LM_WIRE_CONTROL)
			LogPrintln(" lg] WIRE Check STOP:" + String(currWirePulse)+","+String(mainEnc.bbnCnt));
		#endif
	}
}

#define LM_WIRE_ADD_TARGET	5		// 10 pulse
volatile short targetAddPulse = LM_WIRE_ADD_TARGET;		// 10 Pulse , 10*10 mm = 10 cm
short lmWire_Step = 0;

// Hooking & Randing중 BBN Encoder CCW/CW Check
void checkBbnCcwCheck()
{
	if(!lmWireControl_Flag)
	{
		if(BBN_MOT_CCW == mainEnc.bbnDir)	// 상태 무관
		{
			//CCW 전환후 + 10 Pulse 이동	// = 10 * 10 mm
			if( (currWirePulse + targetAddPulse) < mainEnc.bbnCnt)
			{
				targetWirePulse = mainEnc.bbnCnt;	// []Hooking 시작/Randing시작 ] + 10Pulse
				lmWireControl_Start();
			}
		}
	}
}

void lmWireControl_Start()
{
	lmWireControl_Flag = 1; 	// Control Start
	lmWire_Step = 10;
	#if (LOG_LM_WIRE_CONTROL)
		LogPrintln(" lg] WIRE_CONTROL_START:" + String(mainEnc.bbnCnt));
	#endif
}

void lmWireControl_End()
{
	lmWire_Step = 90;
}

void lmWireControl_Stop()
{

	lmWireControl_Flag = 0;
	lmWire_Step = 0;

	wireLongCheck_Flag = 0;

	#if (LOG_LM_WIRE_CONTROL)
		LogPrintln(" lg] WIRE_CONTROL_STOP:" + String(mainEnc.bbnCnt));
	#endif
	
}

#define LM_WIRE_CONTROL_TO	(15*1000)
//
//	1) Taget 갱신
//	2) 현재+10 Pulse위치 검지(CW) => 1a 출력
//	3) 1a<->1c 반복제어
//
void lmWireControl()
{
	// 최대치를 항상 갱신
  #if (NOT_USE_CODE)
	if(targetWirePulse < mainEnc.bbnCnt)
	{
		targetWirePulse = mainEnc.bbnCnt;		// 최대치 목표Pulse설정
	}
  #endif
  
	// CW로 전환, 1a 출력 조건 확인	
	if(19 < lmWire_Step && 40 > lmWire_Step && BBN_MOT_CW == mainEnc.bbnDir)
	{
		// CW전환후, Current + 10 Pulse이동 //
		if((currWirePulse + targetAddPulse) > mainEnc.bbnCnt)
		{
			// 모터 1a 출력
			lmWire_Step = 40;	// A출력 고정
			#if (LOG_LM_WIRE_CONTROL)
				LogPrintln(" lg] WIRE A OUT FIX:" + String(mainEnc.bbnCnt));
			#endif
		}
	}

	static unsigned long _endto=0;
	static unsigned long acCnt=0;
	static unsigned long _to;
	int tmVal = 0;
	int bbnVal = 0;
	switch(lmWire_Step)
	{
		case 0:
			break;

		case 10:
			acCnt=0;
			setTO(_endto);
			lmWire_Step = 20;
			break;

		// c<-a 반복
		case 20:
			// C출력
			tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].tm;
			bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].bm;
			torqMotor.on(tmVal);
			bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
			acCnt++;
			#if (LOG_LM_WIRE_CONTROL)
				if(1==acCnt%3) { LogPrint("lg] WireL LM Rept Cnt:" + String(acCnt) + "," + String(mainEnc.bbnCnt)); }
			#endif
			setTO(_to);
			lmWire_Step = 21;
			break;
			
		case 21:
			// C출력 대기
			if(checkTO(_to,stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].bmTime ))
			{
				// A출력
				tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
				bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
				torqMotor.on(tmVal);
				bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
				setTO(_to);
				lmWire_Step = 22;
			}
			break;

		case 22:
			// A출력 대기
			if(checkTO(_to,stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bmTime ))
			{
				lmWire_Step = 20;	// REPEAT
			}
			break;

		//1a출력
		case 40:
			// A출력
			tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
			bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
			torqMotor.on(tmVal);
			bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
			#if (LOG_LM_WIRE_CONTROL)
				LogPrint("lg] WireL LM Rept Cnt:" + String(acCnt));
			#endif
			setTO(_to);
			lmWire_Step = 41;
			break;

		case 41:
			if(checkTO(_to,stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tmTime ))
			{
				lmWire_Step = 90;	// END
			}
			if(currWirePulse > mainEnc.bbnCnt)
			{
				lmWire_Step = 90;	// END
			}
			break;

		case 90:
			if(68 > reqTorqueMotor) { tmVal = 68; }					// #1호기 UNIQ
			else					{ tmVal = reqTorqueMotor; }
			torqMotor.on(tmVal);
			bbnMotor.offBldc();
			// CLR Var & Flag
			lmWireControl_Stop();
			break;
			
		default:
			break;
	}

	//ALL TO (15 초)
	if(19 <= lmWire_Step && checkTO(_endto, LM_WIRE_CONTROL_TO))	// Start ~ 30sec
	{
		lmWire_Step = 90;	// END
		#if (LOG_LM_WIRE_CONTROL)
			LogPrintln(" lg] WireL LM TIME_OUT:" + String(LM_WIRE_CONTROL_TO) + "," + String(mainEnc.bbnCnt));
		#endif
	}
	
	// END처리
	
}

//-----------------------------------------
//	인터럽트에서 저장한 데이타를 LOG출력
//
//-----------------------------------------
void wat_isrLogOut()
{
	// 외부INT 테스트
	bool encFlagCheck;
	int encOutLog;
	unsigned int target;

	// INT내 변수 취득
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		encFlagCheck = bIsrEncInterruptFlag;
		if(encFlagCheck) { bIsrEncInterruptFlag = 0; }
		encOutLog = pidExecOutLog;
		if(encOutLog) { pidExecOutLog = 0; target = iTargetDist; }
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료

	if(encFlagCheck)
	{
		unsigned int isrCnt;
		unsigned int interval;
		unsigned short sEncDir;
		int intEncCnt;
		unsigned int currDist;
		double dCalVal;
		short sOutDuty;
	
		portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
			isrCnt = iIsrEncInterruptCnt;
			interval = iEncIntrIntervalTime;
			sEncDir = sIsrBbnEncDir;
			intEncCnt = iIsrbbnEncCnt;
			currDist = iWireDistance;
			dCalVal = dIsrOutDuty;
			sOutDuty = sMotorSpeed;
	
		portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
	
		LogPrintln(" lg] WireC EncINT c:" + String(isrCnt)
				+ ", invt:" + String(interval)
				+ ", dir:" + String(sEncDir)
				+ ", enc:" + String(intEncCnt)
				+ ", dist:" + String(currDist)
				+ ", out:" + String(sOutDuty)
				+ ", calVal:" + String(dCalVal)
				);
	}
	
	if(encOutLog)
	{
		LogPrintln(" lg] WireC Target:" + String(target)+ ", GmSts:" + String(gmWatStatus));
	}
}

/*----------------------------------------------------------------
	주기적 처리
----------------------------------------------------------------*/
void t1ms_Process_Exec()
{

	// Input Process  
	mainEnc.rotate();	// BBN Encoder Reotation

	  
	// Device Check
	if(devReadychkFlag)   { devChkReady_Control(); }				// 장치 장애 검지
	
	if(AP_IS_TM == apType)
	{
		if(mainMotAutoSet_Flag)
		{
			//mainMot_MinOut_AutoSet_Control(); 	// 메인모터 최소출력 자동 설정
			//mainMot_MinOut_AutoSet_Control2_Time();	// 메인모터 최소출력 자동 설정
			mainMot_MinOut_AutoSet_Control3_36PulseTime();
		}
	}

	
#if (LOG_INTR_ENC_A)
	wat_isrLogOut();
#endif


	// 1MS STBY OFF CONTROL

}

/*------------------------------------------------------------------
   10ms EXEC Peocess
   1) casting
   2) biting
   3) Hiting
   4) Success or Fail
   5) Ending
------------------------------------------------------------------*/
void t10ms_Process_Exec()
{

#if 0
	  // Slave Connection Check
	  if(5 > pwrOn1stCnt)
	  {
		  pwrOn1stCnt++;
		  eNow.write(STR_PID_ROD_ALIVE_CHK, "");
	  }
#endif

	// opreation Condition Check
#if (FW_CONDITION_CHK)
	  	//condition_fw_check_10ms();
#endif

// AP MOT Control, by Pattern포함
#if (AP_MOT_CONT)
	// FW자제 로직 (손맛 개선) 무시. 

	//Bite ONLY Pattern2 Control From AP
	#if (FW_CONT_BITE_HOLD)
		apPatternReq_Bite_Hold();		// AP Control BITE PATTERN_2
	#endif
	
#else	// Hand Test , FW 조건판단 자체 모터 제어
  #if HAND_TEST_01
	testHand();		// // FW자제 로직 (손맛 개선)
  #endif
#endif

	// Diag Input test (10ms)
	diagInputMonitor();

#if (AP_MOT_CONT)
	// NA
#else
	if(execContinue2Ready)	{ execReady_TorqControl(); }
#endif
	
	// Diag BBN Motor T/O Count
	if(diagOutflag_bbnTimeOut) { diagBbnMotor_TimeoutControl(); }
	// Diag BLDC TOKTOK Control
	if(diagBbnflag_TokTokControl) { diagBbnMotor_TokTokControl(bbnActKind); }
	// Normal BLDC STOP T/O Control
	if(exeFlag_bbnMotorStop) { bbnMotor_OffTime_Control(); }

	if(exeFlag_torqMotorStop) { torqMotor_OffTime_Control(); }

	// Diag BBN Motor T/O Count
	if(diagOutflag_lmMotTimeOut) { diagLmMotor_TimeoutControl(); }


	// Sub AC On/Off Control
#if (IO_PC_PWR_OFF)

	#if 0
	if(3 > pwrSafyTimeCnt)	{	pwrSafyTimeCnt++;	}
	else					{	pwrSafyTimeSet = 1; }
	#endif
	//if(pwrSafyTimeSet)	// 2SEC
	{
		check_PowerSwitch_Change_10ms();
	//	if(subAcOffSeq) { subAcOff_Control(); }					// (V108) delete
	}
#endif

	//-- External LED Control
	extLed_Control();

	// PC_Main USB Cable DisConnection Check
	usb5vChange_Check();

	// Reel now SEND step
	//if(reelOut_AllOff_Flag)	{ reelAllOff_SendControl(); }	// 10ms Interval NOW SEND to ROD

	//LM-JIG
#if (IO_LM_MOT)
	if(0) {}
	else if(lmHome_Flag) {  lmHome_Control(); }
	//else if(lmHome_Flag) {  lmHome_Control_2(); }		// LM MOVE => BM/TM
	else if(lmRight_Flag || lmLeft_Flag)
	{
		if(0) {}
		else if(LM_PRIORITY_LEFT == lmPriority)	// (1)=LEFT
		{
			lmLeft_Control();
		}
		else if(LM_PRIORITY_RIGHT == lmPriority)	// (2)= Right
		{
			lmRight_Control();
		}
		else		// Priority NOT(0)
		{
			if(0) {}
			else if(lmRight_Flag) { lmRight_Control(); }
			else if(lmLeft_Flag) { lmLeft_Control(); }
		}
	}
	//else if(lmReturn_Flag) { lmReturn_Control(); }
	else
	{
	}

	// LM-JIG Wire Control
	if(0) {}
	else if(lmWireControl_Flag)
	{
		lmWireControl();	// Wire Control
	}
	else if(wireLongCheck_Flag)	// LM Wire Control
	{
		checkBbnCcwCheck();	// Wire Long Check
	}
#endif

	// 낚시줄 에이징 테스트
	if(wireAgingTest_Flag)
	{
		if(AP_IS_TM == apType)	// TM의 경우만 실시.
		{
			wireAging_Test_Control(); 
		}
		else
		{
			wireAging_Test_Stop("$98010");	// Flag CLR 및 동작 정지, $98,01-WireAging,0-OFF
		}
	}

	// 10ms STBY OFF CONTROL
	#if (FUNC_STBY_OFF_CONT)
		// 대기중 텐션유지
		if(stbyTorqOffControl_Flag) { stbyTorqOff_Control(); }	// 메인모터 최소출력 자동 설정
	#endif


}


//=== 100ms Process
//
//-------------------------------------------------
void t100ms_Process_Exec()
{
	if(exeTo1secLog)	{ setAfterTo_LogOut_Control(); }
	
	if(rodRegistMode)	{ rodRegistToControl();		}	// Regist TO Check

}

// Call 500MS
void t500ms_Process_Exec()
{
	//===== MOVE to T500ms Proc => Move to 1SEC
	//if (isGameEnable) {		// if Main Enable
	//	checkSlaveTimer();	// 500ms Slave Check Timer
	//}

	// board Led Blinking for 500ms
	boardLed_Control();
}

//
//
//
void t1sec_Process_Exec()
{
	// Power On(reset) SEND to ROD, PowerOn1st CMD
	sendCmdSts_MainPwrOn();
	
	// AP-MAIN Connection (Alive Recv) Check
	apAlive_Check();
	
	if(!rodRegistMode)		//통상, ROD등록mode시 Alive송부 금지
	{
		// Send ROD_Alive CHK
		send2RodAlive_CntCheck();		// Send Alive ROD Check
		//
		checkRodTimer();  // ROD Slave Check Timer
	}
}

int pwrOn1stCnt = 0;
int pwrOn1stCmdSendFlag = 1;
//------------------------------------------------------------
//	SEND CMD(PWRON_1stCMD) , Only Once  MAIN=>ROD
//------------------------------------------------------------
void sendCmdSts_MainPwrOn()
{
	// PWR_1ST_CMD Send , First
	if(0 == pwrOn1stCmdSendFlag)			// After 1SEC, at PowerON 
	{
		return;
	}
	// CLR SEND FLAG
	pwrOn1stCmdSendFlag = 0;

	String respMsg = "";

	//=== 1) SEND PWR_ON MAIN=>ROD
	eNow.write(STR_PID_PWR_1STCMD, respMsg);

	//=== 2) ROD Regist Wait T/O SEC
	respMsg = STX_INFO_REQ + ROD_REGIST_TO_SEC;
	respMsg += String(ROD_REGI_CANCEL_TO/1000);
	respMsg += "%";
	Resp2ApPrintln(respMsg);
	LogPrintln(" FW] INFOR wto:" + respMsg);

	//=== 3) SEND AP TYPE
	eNow.write(STR_PID_AP_INFO_SEND, "0100");	// AP TYPE = 00(DF), apType

	//===X) TBD

		
}


//----------------------------------------------------------------------
//
//---------------------------------------------------------------------
volatile int logRcved_flag = 0;
volatile int logRcved_Seq = 0;
String logRcvApMsg = "";
String logRcvApMsg_Save = "";

// LOG UART_SERIAL 1 DATA Parsing : NOT BREAK
void logUartRecvParsing()
{

	// 1) Rcv Msg Parsing
	int msglength = LogSerial.available();
	if (msglength)						// Length of Serial Buffer from PC
	{
		// NOT DELAY(BREAK) CONTROL
		for(int i=0; i<msglength; i++)
		{
			char recvCh = LogSerial.read();
			if(0) { }
			else if( 0x24 == recvCh)		// '$'
			{
				logRcved_Seq = 50;	// Rcv Start Seq
				logRcvApMsg = "$";
			}
			else if( 0x0A == recvCh)		// LF (Not SAVE)
			{
				logRcved_Seq = 53; // Recv Seq( Remove MSG)
				//LogPrint("N");
			}
			else if( 0x0D == recvCh)		// CR (Not SAVE)
			{
				logRcved_Seq = 52; // Recv Seq( Remove MSG)
				//LogPrint("R");
			}
			else if( 0x25 == recvCh)		// "%" (Not SAVE)
			{
				if(50 == logRcved_Seq)
				{
					logRcved_Seq = 51;	// Recv Seq(STOP)
					// Data Save
					logRcvApMsg_Save = logRcvApMsg;
					logRcved_flag = 1;				// SET AP MSG RCVED
					break;
				}
			}
			else			// Data Save
			{
				if(50 == logRcved_Seq)
				{
					logRcvApMsg += recvCh;
				}
			}
		}

	  #if (LOG_UART1_RCV_MSG_PARSING)
		if(1 == logRcved_flag)
		{
			LogPrintln(" AL] rcvMg Parsing" + logRcvApMsg_Save);
		}
	  #endif
	}

}

/*
=================================================================
	COMMAND ANAYSIS & CONTROL EXEC with RECVED  LOG SERIAL PORT
	
=================================================================
*/
void logRecvHandler()
{

#if 0
	if (LogSerial.available())						// Length of Serial Buffer from PC
	{
		String msg = LogSerial.readStringUntil('%');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
#endif

	// MSG미수신시 처리하지 않음
	if (0 == logRcved_flag)					//Not Recved
	{
		return;
	}

	String msg = logRcvApMsg_Save;
	logRcvApMsg_Save = "";
	logRcved_flag = 0; // MSG ANA FINISH

#if (LOG_UART1_RCV_MSG)
	LogPrintln(" AL] rcvMg Handler" + msg);
#endif
	msg.trim(); 								// Delete Space in String

	// MSG가 없는 경우 처리하지 않음
	if (msg == "")			// Return, if not Recv
	{
		return;
	}

#if (TBD)		// 추후 개선
	int stx = msg.substring(1,(1+2)).toInt();
	switch(stx)
	{
		case STX_CMD_ROD_REGIST:
			break;
			
		default:
			break;
	}
#endif
	// Recv STX String Switch
		if (0) { }	// Dummy

		else if (msg.startsWith(STX_ROD_REGIST))	// ROD REGIST
		{
			anaRodRegist(msg);
		}
		
		// 1) $11 - Mac Addr READ - ack : M + my Addr + Targ Addr+%
		else if (msg.startsWith(STX_GET_ADDR))
		{
		}
		//2) $12 - Target Addr WRITE  - ack : void
		else if (msg.startsWith(STX_SET_ADDR))
		{
		}
#if (NEW_IF)
		// $01 - INFO_REQ
		else if (msg.startsWith(STX_INFO_REQ))
		{
			LogPrintln( "LG] InfRq IF_ver: " + IfVer);
		}
#else
		// 3) $01 - Main Board ENB/DIS Setting
		else if (msg.startsWith(STX_GAMEENB))
		{
			LogPrintln( "LG] GameS ENB/DIS: " + msg);
		}
#endif
		// $03 - Servo Motor SET
		else if (msg.startsWith(STX_SERVO_MOTOR))
		{
		}
		// $02 - Torque Motor SET
		else if (msg.startsWith(STX_TORQUE_MOTOR))
		{
		}

		// $04 - Bldc Motor SET <=  $40
		else if (msg.startsWith(STX_BBN_MOTOR))
		{
		}

#if (NEW_IF)
		// $06 - LED CONT CMD
		else if (msg.startsWith(STX_LED_CONT))
		{
			analedContCmd(msg);
		}
#endif		

		// $10 <= $0A - Version Read
		else if (msg.startsWith(STX_VER_READ))
		{
			if ((ROD_CONN == rod_conn_status) && (gRodSleepStat == 0))
			{
				eNow.write(STR_PID_ROD_VER_READ, ""); 	// Send Version READ
				//String str = STX_VER_READ + mainVer + "," + eNow.getVersion() + "," + eNow.getTargetAddress() + "%";
			}
			else
			{
				String str;
				if (g_RespVersion[0] == '$')
					str = g_RespVersion;
				else
					str = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + "Vs99.99.99" + "," + "Vi99.99.99" + "%";
			
				LogPrintln(" FR] Ver: " + str);		// Log
			}
		}

#if HAND_TEST_01	
		// $F0 - Wave Val SET
		else if (msg.startsWith(STX_WAVE_SET))		// $F0
		{
			Set_WaveData(msg);
		}

		// $F1 - BITE Val SET
		else if (msg.startsWith(STX_BITE_SET))
		{
			Set_BiteData(msg);
		}

		// $F2 - HIT Val SET
		else if (msg.startsWith(STX_HIT_SET))
		{
			Set_HitData(msg);
		}

		// $F3 - Break Val SET
		else if (msg.startsWith(STX_BREAK_SET))
		{
			Set_BreakData(msg);
		}

		// $F4 - BLDC TOKTOK Val SET
		else if (msg.startsWith(STX_TOKTOK_SET))
		{
			Set_BbnTokTokData(msg);
		}
		// $F5 - EXT LED SET
		else if (msg.startsWith(STX_EXTLED_SET))
		{
			Set_ExtledData(msg);
		}
		

		// $FF - Set ENB/Kind
		else if (msg.startsWith(STX_ENB_SET))
		{
			Set_EnbData(msg);
		}
		else if (msg.startsWith(STX_TORQ_ADJ_SET))
		{
			Set_TorqAdjValData(msg);
		}

		// TEST RANDING ($F6)
		else if (msg.startsWith(STX_RANDING_START))
		{
			anaRanding(msg);
		}

	#if (MKT_TEST_1)
		// MKT TUN HIT & HOLD
		// $F9aXXXyy
		//  a : 0- Val view, 1-hit, 2-hold
		// XXX : Main Mot Power ( 60 ~ 255)
		// yy : Bobbin Mot Power (10 ~ 70)
		else if (msg.startsWith(STX_TORQ_TUN_SET))
		{
			Set_TorqHitHoldOn(msg);
		}
	#endif

		else if (msg.startsWith(STX_LED_SET))
		{
			Set_LedValue(msg);
		}

#endif
		// $DI - Diag Input
		else if (msg.startsWith(STX_DIAG_INPUT))
		{
			mDiag_entry();
			diagInputTest(msg);
		}
		// $D0 - Diag Output
		else if (msg.startsWith(STX_DIAG_OUTPUT))
		{
			mDiag_entry();
		 	diagOutputTest(msg);
		}
		else if (msg.startsWith(STX_DIAG_EXIT))
		{
			mDiag_exit();
		}

		// Data Set/Read ($99)
		else if (msg.startsWith(STX_DATA_PARA_SET))
		{
			ana_SetDataSave(msg);
		}

		// Test Execute ($TE)
		else if (msg.startsWith(STX_TEST_EXEC))			// (V108) TEST
		{
			ana_TestExecution(msg);
		}
		//	else Slave Connection ( Send Other STX to Slave )
		// $05(Break) or $08(IMU Pass to SLAVE)
		else if (ROD_CONN == rod_conn_status)
		{
			// Send to ROD
		}
	  
}


//-------------------------------------------------------------
#define	EX_STX_LED_OFF	"$10"
#define	EX_STX_LED_ON	"$11"
#define	EX_STX_OTHER	"$12"

//====================================================
//
//
//====================================================
void extIOuartRecvHandler()
{
#if	(FUNC_IO_EXT_UART)		// FOR COMPILE, PIN & MACRO DEFINE

	if (ExtSerial.available())						// Length of Serial Buffer from PC
	{
		String msg = ExtSerial.readStringUntil('!');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
		//String msg = ExtSerial.readStringUntil('#');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
    	msg.trim();									// Delete Space in String
    
    	if (msg == "")			// Return, if not Recv
    	{
      		return;
    	}
		
		char stx = msg.charAt(0);
		if (stx != '$')
		{
			LogPrintln(" XR] noStx " + msg);	  // LOG-SERIAL1
			return;
		}
		
//======================================================================
		// LOG OUT : AP CMD
		if(!(msg.startsWith(EX_STX_OTHER)))
		{
			send2extPrintln(msg+"!");	  // Send to EXT_IO_BOARD
	  	#if LOG_EXTIO
			LogPrintln(" XR] ExtRSP " + msg + "!");	  // LOG-SERIAL1
	  	#endif
		}
	
//========================================================================
		// ANA EXT_IO_BOARD RESP & EXEC
		// Recv STX String Switch

		int respInt = msg.substring(1,(1+2)).toInt();
		if(0) {  }	// dummy

		else if(12 > respInt)
		{
			send2extPrintln(msg+"!");	  // Send to EXT_IO_BOARD
			#if LOG_EXTIO
			LogPrintln(" XR] ExtRSP " + msg + "!");	// LOG-SERIAL1
			#endif
		}
		else
		{
			#if LOG_EXTIO
			LogPrintln(" XR] ExtRSP Not Define Msg" + msg + "!!!");   // LOG-SERIAL1
			#endif
		}

	#if 0
		// $00 - PC & MAIN Connection Check
    	else if (msg.startsWith(EX_STX_LED_OFF))	// MAIN ALIVE
    	{
			send2extPrintln(msg+"!");	  // Send to EXT_IO_BOARD
	  	#if LOG_EXTIO
			LogPrintln(" XR] ExtRSP " + msg + "!");	  // LOG-SERIAL1
	  	#endif
		}
	#endif
	
  	}
#endif	// NOT_IO_EXT_UART
	
}


short imuIntervalCheckCnt =0;
unsigned long measureCnt=0;
unsigned long imuIntervalTime=0;

#define IMU_MEASURE_CNT	5
//-----------------------------------------------
//	IMU Interval 간격 측정
//		-- Only TM
//-----------------------------------------------
void imuInterval_Measure()
{
  	if(imuIntervalCheckCnt)
  	{
  		imuIntervalCheckCnt++;
		if(0) {}
  		else if(2 == imuIntervalCheckCnt) { measureCnt = curr_ms_tick;	}// 1st
  		else if(3 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 2nd
  		else if(4 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 3rd
  		else if(5 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 4th
  		else if(6 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 5th
		else
		{
			imuIntervalTime += (int)(curr_ms_tick - measureCnt);		// 5th
			imuIntervalTime = imuIntervalTime/IMU_MEASURE_CNT;
			// Send to PC
			strImuInterval = fillZero4Char((unsigned int)imuIntervalTime);
			Resp2ApPrintln(STX_INFO_REQ+ IMU_INTVAL_TIME + strImuInterval +"%");	// ms
			
			imuIntervalCheckCnt = 0;	// 측정 종료
			imuIntervalTime = 0;
			measureCnt = 0;
		}
  	}

}

//---- Delay Check
unsigned short imuRecv1stTime = 0;

String rodVer = "Vr99.99.99";


//-------------------------------------------------------------------
//
int handEncCnt = 0;
int oldhandEncCnt = -99;

/*  =====================================
       From SLAVE, Check Unlimit Loop, For (ESP Now) Recv Data Hanlder From Slave
  ===================================== */
void nowRecvHandler()
{

	String respMsg;
	//String sendBtnMsg = "99";
	//String logMsg = " FS] BTNNG 99";

// Not Recv Pid, then Return
	//if (now_msg.pid == -1)
	if (0 == now_rcv_id_cb)
	{
		return;
	}

#if 0	// CB의 처리시간 최소화
	// == Address Check ==
	// 1. 통상 : 송신한 낚시대가 타켓주소와 일치하지 않는 경우, MSG는 버림.
	// 2. Rod 등록 : 모두 받아 처리함
	if(!rodRegistMode)
	{
		for(int i = 5; i > -1; i--)
		{ 
			if(slave_board_addr[i] != *(rcv_src_addr_cb+i))
			{
				now_msg.pid = -1;
				return;
			}
		}
	}
	//-----------------------------
#endif

	//--SAVE CALLBACK DATA(ID+MSG)---------
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		now_rcv_id = now_rcv_id_cb; 	// 추후 개선
		now_msg_str = now_msg_str_cb;
		//--CLR CALLBACK DATA
		now_rcv_id_cb = 0;
		now_msg_str_cb = "";
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료

//====== ROAD RESP/STS ANALISIS===
	int notDefineCmdFlag = 0;
	String now_cmd_data = now_msg_str.substring(2); // Msg_Data저장, ID(2 char) 제외

#if 1
	if (now_rcv_id != PID_ROD_ALIVE_RESP) {
		if (now_msg_str.length() > 18) {
			LogPrintln(" RD] MSG:  " + now_msg_str.substring(0, 18) + "#");	  // LOG-SERIAL1
		}
		else {
			LogPrintln(" RD] MSG:  " + now_msg_str + "%");	  // LOG-SERIAL1
		}
	}
#endif
	
// *1. ANA CMD : Switch Recv Pid
	//1) Recv Slave Control ( 11 )
	if(0) { }	// Dummy

	// ROD교체 ADDR등록 진입(L버튼 3초이상 누름)
	else if (PID_ROD_ADDR_WRITE == now_rcv_id)	// ROD -> MAIN (BroadCast Addr)
	{
		memcpy(rcv_src_addr_back, rcv_src_addr_cb, 6); // CB에서 저장한 값을 최대한 빨리 퇴피 시킴
		// Addr Mode인 경우만 처리
		//String recvRodAddr = now_cmd_data.substring(0,(0+17));  // NG
		LogPrintln(" LG] RodRg BC RcvRodaddr:" + now_cmd_data);
		if(1 == rodRegistMode)		// REGIST 진입상태 ( 개시이후는 처리 안함)
		{
			rodRegistExec(now_cmd_data);
		}
  	}

#if (0)	// NOT_USE
	// ($01) ROD_INFO, // TM인경우만 IMU_INTERVAL 응답 ($01,03,yyyy)
	else if (PID_ROD_INFO_RESP == now_rcv_id && (!rodRegistMode))	  // Button & Encoder
	{
		short kind = now_cmd_data.substring(0,2).toInt();
		short val = now_cmd_data.substring(2,(2+4)).toInt();

		switch(kind)
		{
			//case 1:		// (자발) 보드Type(PID=2 )
			//	break;
			//case 2:		// (응답) 릴등록 대기시간 (PID=)
			//	break;
			case 3:			// (자발) IMU INTERVAL 
				//imuOutIntervalTime = val;
				strImuInterval = now_cmd_data.substring(2);		// Val값만 문자열 저장
				if(AP_IS_TM == apType)
				{
					respMsg = STX_INFO_REQ + now_cmd_data + "%";		// KIND=03, IMU_INTVAL_TIME
					StsSendPrintln(respMsg);	  // NEW IF 
				}
				break;
				
			default:
				break;
		}
	}
#endif

	// Rod Button
  else if (PID_ROD_BUTTON == now_rcv_id && (!rodRegistMode))	// Button & Encoder
  {
  		respMsg = RESP_STX_BUTTON + now_cmd_data + "%";		// PASS
		StsSendPrintln(respMsg);	  // NEW IF
		
		#if LOG_ROD_BTN
			int btn = now_cmd_data.substring(0).toInt();
			String logMsg;
			switch(btn)
			{
				case 10: logMsg = "BT_LD"; break;
				case 11: logMsg = "BT_LU"; break;
				case 12: logMsg = "BT_LF"; break;
				case 16: logMsg = "BT_LL"; break;
				case 20: logMsg = "BT_RD"; break;
				case 21: logMsg = "BT_RU"; break;
				case 22: logMsg = "BT_RF"; break;
				case 26: logMsg = "BT_RL"; break;
				//
				case 80: logMsg = "BT_SD"; break;
				case 81: logMsg = "BT_SU"; break;
				case 90: logMsg = "BT_BU"; break;
				case 91: logMsg = "BT_BD"; break;
				
				default: logMsg = "BT_UN"; break;
			}
			LogPrintln(" FS] BTN__ " + logMsg);
		#endif
  }

	// Reel ECN COUNTER
  else if (PID_ROD_ENC_CNT == now_rcv_id && (!rodRegistMode))	// Button & Encoder
  {
  		respMsg = RESP_STX_ENCODER + now_cmd_data + "%";
		StsSendPrintln(respMsg);	  // NEW IF

	  #if LOG_ROD_ENC
		handEncCnt = now_cmd_data.substring(0).toInt();
		if(oldhandEncCnt != handEncCnt)
	  	{
			LogPrintln(" FS] HDENC " + now_cmd_data);
	  		oldhandEncCnt != handEncCnt;
		}
	  #endif
  }

#if 0	// NOT-USE  
  //==== SLAVE CTRL (=PASS CMD)
  else if (PID_ROD_CTRL_NUM == now_rcv_id && (!rodRegistMode))	// Button & Encoder
  {
  	// MSG BYPASS
	respMsg = now_msg_str;

	// Recv Button($06) LOG
	if(0) { /* Dummy */ }

#if (NEW_IF)
	// TBD : OLD($06) or NEW($13)
 	else if(respMsg.startsWith(OLD_RESP_STX_BUTTON))   // Button
#else
 	else if(respMsg.startsWith(RESP_STX_BUTTON))   // Button
#endif
 	{

		int btn;
		// TBD : Edit ( Exchange OLD =>  NEW)
 		btn = respMsg.substring(3,11).toInt();
 		if(1 == btn)			// LEFT Btn DOWN(=Push)
 		{
 			sendBtnMsg = RESP_STX_BUTTON + "10";
			sendBtnMsg += "%"; 
		  logMsg = (" FS] BT_LU " + respMsg); // Log
		}
		else if(10 == btn)  	// LEFT Btn UP(=Off)
 		{
		  sendBtnMsg = RESP_STX_BUTTON + "11";
		  sendBtnMsg += "%"; 
		  logMsg = (" FS] BT_LD " + respMsg); // Log
		}
 		else if(100 == btn)			// Right Btn DOWN(=Push)
 		{
		  sendBtnMsg = RESP_STX_BUTTON + "20";
		  sendBtnMsg += "%"; 
	      cntRightButtonOn = 1;
		  cntRightButtonOff = 0;
		  logMsg = (" FS] BT_RU " + respMsg); // Log
		}
		else if(1000 == btn)  	// Right Btn UP(=Off)
 		{
		  sendBtnMsg = RESP_STX_BUTTON + "21";
		  sendBtnMsg += "%"; 
		  cntRightButtonOn = 0;
		  cntRightButtonOff = 1;
		  logMsg = (" FS] BT_RD " + respMsg); // Log
		}
		// NOT Define KEY
		else
		{
			sendBtnMsg = RESP_STX_BUTTON + "99";
			sendBtnMsg += "%"; 
			logMsg = (" FS] BT_NG " + respMsg); // Log
		}
		//LogPrintln(btn);

	  #if (NEW_IF)
		StsSendPrintln(sendBtnMsg);		// NEW IF
	  #endif
	  
	  #if LOG_ROD_BTN
		LogPrintln(logMsg);
	  #endif
	  
 	}

  	// Encoder($07) LOG ( Not Out LOG)
#if (NEW_IF)
// TBD : OLD($07) or NEW($14)
	else if(respMsg.startsWith(OLD_RESP_STX_ENCODER))	// Handling Encoder 
#else
	else if(respMsg.startsWith(RESP_STX_ENCODER))	// Handling Encoder
#endif
	{
		String encMsg;
	  #if (NEW_IF)
		// TBD : Edit ( Exchange OLD =>  NEW)
	    encMsg = respMsg.substring(8,(8+3));
		StsSendPrintln(RESP_STX_ENCODER + encMsg +"%");	// pass to PC
	  #endif
		
		handEncCnt = encMsg.substring(0).toInt();
		if(oldhandEncCnt != handEncCnt)
	  	{
	  		#if LOG_ROD_ENC
				LogPrintln(" FS] HDENC " + encMsg);
			#endif
	  		oldhandEncCnt != handEncCnt;
		}
	}

	// NOT Define CMD
	else
	{
		// Error
	}

  }
#endif

  // 2) IMU EULER Data( 12 )
#if 0
  else if (now_rcv_id == PID_IMU_EULER_NUM && (!rodRegistMode))
  {
    eulerStr = now_msg_str;			// Save Jyro Data String
	//#if LOG_IO
	#if (LOG_IMU_RCV_1ST_TIME)
		if(1 == imuRecv1stTime)
		{
			imuRecv1stTime = 2;
			LogPrintln(" FS] EULER " + eulerStr);
		}
	#endif
  }
#endif

 	// ( 14 ) IMU Data RECV
  else if (now_rcv_id == PID_IMU_DATA_NUM && (!rodRegistMode))
  {
    //if (eulerStr.length() != 0)	// EULER Data exist
    {
    	// send tp PC ($09 + EulerData + ',' + message String + %)
      //String imuStr = STX_IMU_DATA_STS + eulerStr + "," + now_msg_str + "%";
      
      String imuStr = STX_IMU_DATA_STS + now_cmd_data + "%";
      //String imuStr = STX_IMU_DATA_STS + "#__" + now_msg_str + "__#";	// TEST
      ApPrintln(imuStr);		// Send to PC

	  //#if LOG_IO
	  #if (LOG_IMU_RCV_1ST_TIME)
		//if(2 == imuRecv1stTime)
		if(1 == imuRecv1stTime)
		{
			imuRecv1stTime = 0;
			//LogPrintln(" FS] GYRO " + eulerStr + "//" + now_msg_str);
			LogPrintln(" FS] Euler+Gyro: " + now_msg_str);
		}
	  #endif

	  //== IMU Interval 간격 측정
	  if(AP_IS_TM == apType && imuIntervalCheckCnt)
	  {
		imuInterval_Measure();
	  }
	  //-------------------------------------------------------
    }
  }
  // 4) Recv ROD_ALIVE( 10 )
  else if (now_rcv_id == PID_ROD_ALIVE_RESP && (!rodRegistMode))
  {
		//slave_resp_over_cnt = 0;			// CLR OverCNT at RECVED
		//battLevelSet(now_msg_str);
		setRod_Connected();
	#if LOG_ROD_ALV_RESP
		LogPrintln(" LG] RDALV Cnt:"+ String(rodAlive_SendCnt) + ", msg:" + now_msg_str);
	#endif
  }

  #if 0
	// 5) Recv ROD_VER_READ( 19 / 20 )
	//else if (now_rcv_id == PID_ROD_VER_RESP_ROD)	//(19)
	//{
		//String respVer = STX_VER_READ + mainVer + "," + now_msg_str + "%";
	//	rodVer = now_msg_str;
		//Resp2ApPrintln(respVer);
		//LogPrintln(" FM] Ver__ " + respVer);	// Log
	//}
  #endif

	// (22) 버전 RESP
	else if (PID_ROD_VER_RESP == now_rcv_id)	//(20)
	{
	//	String respVer = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + now_cmd_data + "%";	// Main,Rod,Imu
		g_RespVersion = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + now_cmd_data + "%";	// Main,Rod,Imu
		Resp2ApPrintln(g_RespVersion);
		
		LogPrintln(" FM] Ver__ " + g_RespVersion);	  // Log
	}


	// 배터리 잔량 수신.
  else if (PID_BATT_LVL == now_rcv_id && (!rodRegistMode))	//BAT LEVEL
  {
		// Batt Level Set [ INT / STRING ]
		battLevelSet(now_cmd_data);
		
		//slave_resp_over_cnt = 0;			// CLR Over CNT at RECVED
		respMsg = STX_BAT_LVL + battLvlStr + "%";	// 2 char
		StsSendPrintln(respMsg);	// pass to PC

		// 배터리 잔량 경고( 7020: 15%미만) 통지
		if(BAT_LVL_LOW > ibatteryLvl)
		{
			respMsg = STX_DEV_CONT_REQ + ROD_BAT_LOW_WARN;
			respMsg += "%";
			StsSendPrintln(respMsg);	// pass to PC	
		}

	  #if LOG_IO
		LogPrintln(" FS] ALIVE " + battLvlStr + " /BattLVL_Per");
	  #endif
  }

  // 6) IMU Connection(PID "21")
  else if (PID_STS_IMU_CONNECT == now_rcv_id)	// IMU CONN
  {
	// TBD : Edit ( OLD => NEW)
	int recvIMUconn = now_cmd_data.substring(0).toInt();
	if(IMU_CONN_2CHAR_INT == recvIMUconn)	// "11"
	{
		imu_conn_status = CONNECT;		// IMU CONN ( 0-CONN, 1-DISCONN, -1 : UNKNOWN)
	}
	else
	{
		imu_conn_status = DISCONNECT;		// IMU CONN ( 0-CONN, 1-DISCONN, -1 : UNKNOWN)
	}

		respMsg = STX_IMU_CONN;
		if(CONNECT == imu_conn_status)	{ respMsg += STATE_CONNECTED; }
		else							{ respMsg += STATE_DISCONN; }
		respMsg += "%";
		StsSendPrintln(respMsg);	// bypass to PC

	LogPrintln(" FM] IMUCN " + respMsg + ", " + String(imu_conn_status) );	// Log
  }

  else if (PID_ROD_BOARD_TYPE == now_rcv_id) // Board TYPE
  {
  	// 1) ROD보드 Type SAVE
  	strRodBoardType = now_cmd_data;	// 3 Char (OLD-000, NEW-001)

	rodBoardType = strRodBoardType.substring(0).toInt();

	// 2) SEND to AP , ROD보드 TYPE
	respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;		// Board TYPE
  	respMsg += strRodBoardType;		// 3 Char (OLD-000, NEW-001)
	respMsg += "%";
	StsSendPrintln(respMsg);	  // SEND ROD BOARD TYPE to AP
	
  	LogPrintln(" FS] INFOR " + respMsg + ",type:" + String(rodBoardType) );  // Log
  }
  else if (PID_ROD_SLPRSP == now_rcv_id) // Rod wakeup
  {
	int stm;
	// Batt Level Set [ INT / STRING ]
	battLevelSet(now_cmd_data);
	
	//slave_resp_over_cnt = 0;			// CLR Over CNT at RECVED
	respMsg = STX_BAT_LVL + battLvlStr + "%";	// 2 char
	StsSendPrintln(respMsg);	// pass to PC

	if (now_cmd_data[3] == 'S') {
		stm = now_cmd_data.substring(4,4+4).toInt();
 	 	LogPrintf(" LG] SLEEP[r]: gRodSleepMode(%d) gRodSleepTime(%d->%d) gRodSleepStat(%d->1)\r\n", gRodSleepMode, gRodSleepTime, stm, gRodSleepStat);  // Log
		gRodSleepTime = stm;
		gRodSleepStat = 1;
	}
	else {
		if (gRodSleepMode) {
			if (gRodSleepMode == SS_EXIT)
				stm = TIME_SLEEP_DEEP;
			else
				stm = TIME_SLEEP_LIGHT;
		}
		else {
			stm = 0;
		}
 	 	LogPrintf(" LG] SLEEP[R]: gRodSleepMode(%d) gRodSleepTime(%d->%d) gRodSleepStat(%d->0)\r\n", gRodSleepMode, gRodSleepTime, stm, gRodSleepStat);  // Log
		gRodSleepStat = 0;
		sendSleepEnable(gRodSleepMode, stm, 3);
	}
  }

#if 0	// Delete SLEEP
  // 7) SLAVE SLEEP ENTRY
  else if (now_msg.pid == PID_ROD_RESP_SLEEP_ENT)
  {
	//fSlaveSleep = 1;					// Set Sleep Entry
	// new_rod_conn_status = ROD_DISCONN_SLEEP;
	respMsg = now_msg_str;

  	LogPrintln(" FM] SLCN1 " + String(now_msg.pid) + " " + respMsg);	// Log
  }
#endif

	// NOT DEFINE CMD
  else
  {
  	notDefineCmdFlag = 1;
  }

	// * 2. Check Not Define & Error SET & CLR
  if(notDefineCmdFlag)
  {
  		notDefineCmdFlag = 0;
		//Error
  }
  else				// Valid SLAVE STS
  {
	  //check_slave_dis_conn_cnt = 0; 			  // Check Cnt CLR( Check Restart )
	  // TEST, Alive NG
	  rodAlive_SendCnt = 0;
  }

  #if (LOG_NOW_RCV_ANA)
  	LogPrintln(" LG] nowRcv ANA Pid:" + String(now_rcv_id) +", rcvSeq:" + String(nowRecvSeqNo));
  #endif
  
  // * 3.  Set Not Recv(Slave PID)
  //now_msg.pid = -1;
  now_rcv_id = 0;
  now_msg_str = "";

}

volatile int rcved_flag = 0;
int rcved_Seq = 0;
String rcvApMsg = "";
String rcvApMsg_Save = "";

// AP_SERIAL DATA Parsing : NOT BREAK
void uartRecvParsing()
{

#if 0	// OLD Control : BREAK = UNTIL %
	if (Serial.available()) 					  // Length of Serial Buffer from PC
	{
    	String msg = Serial.readStringUntil('%');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
    	msg.trim();									// Delete Space in String
    
    	if (msg == "")			// Return, if not Recv
    	{
    		return;
    	}
#endif

	// 1) Rcv Msg Parsing
	int msglength = Serial.available();
	if (msglength > 0)						// Length of Serial Buffer from PC
	{
	//	Serial1.printf("%10d",curr_ms_tick);Serial1.printf(" DN] Serial.available() = %d\r\n", msglength);
		// NOT DELAY(BREAK) CONTROL
		for(int i=0; i<msglength; i++)
		{
			if(g_DnExecStep)
			{
				int idx;
				unsigned short cSum;
				unsigned char rChr;
				if (g_DnExecStep < 100) {
					rChr = (unsigned char)Serial.read();
				//	g_readBuf[g_SavePtr] = rChr;
				//	g_SavePtr = (g_SavePtr + 1) & (MAX_SAVE_LEN-1);
				}
				switch (g_DnExecStep) {
				case 1 :
				case 2 :
				case 3 :
				case 4 :
					rcvApMsg += (char)rChr;
					g_DnExecStep++;
					break;
				case 5 :
					g_DnRecvSeq = rcvApMsg.substring(4).toInt();
				//	if (rcvApMsg[3] == 'm' || rcvApMsg[3] == 'r') {
					g_DnRecvBuf = g_DnSaveBuf[g_DnSaveIdx];				// 
					g_DnRecvLen = rChr << 8;
					g_DnExecStep = 6;
					break;
				case 6 :
					g_DnRecvLen += rChr;
					g_DnRecvIdx = 0;
					g_DnCalcSum = 0;
					g_DnExecStep = 100;
				//	DbgPrintf(" DN] g_DnRecvSeq(%d) DnRecvLen(0x%x, %d)\r\n", g_DnRecvSeq, g_DnRecvLen, g_DnRecvLen);
					break;
				case 7 :
					g_DnDataSum = rChr << 8;
					g_DnExecStep = 8;
					break;
				case 8 :
					g_DnDataSum += rChr;
					if (g_DnDataSum == g_DnCalcSum) {
						g_DnDataSum = 0;
						g_DnSaveLen[g_DnSaveIdx] = g_DnRecvLen;
						g_DnSaveIdx = (g_DnSaveIdx+1) & (DN_MAX_BUF-1);
					}
					else {
						LogPrintf(" DN] DataSum(%x), DnCalcSum(%x)\r\n", g_DnDataSum, g_DnCalcSum);
						g_DnDataSum = 1;
					}
					g_DnExecStep = 0;
					break;
				case 100 :
					if (g_DnRecvIdx < g_DnRecvLen) {
						int rbytes = min(g_DnRecvLen-g_DnRecvIdx, msglength-i);
						Serial.readBytes(&g_DnRecvBuf[g_DnRecvIdx], rbytes);
					//	rbytes = ApReadBytes(&g_DnRecvBuf[g_DnRecvIdx], rbytes);
						for (int jdx=0; jdx < rbytes; jdx++)
							g_DnCalcSum += g_DnRecvBuf[g_DnRecvIdx++];
						i += rbytes - 1;
					}
					if (g_DnRecvIdx >= g_DnRecvLen)
						g_DnExecStep = 7;
					break;
				}
			}
			else
			{
				char recvCh = Serial.read();
				if( 0x24 == recvCh)		// '$'
				{
					rcved_Seq = 50;	// Rcv Start Seq
					rcvApMsg = "$";
				}
				else if( 0x0A == recvCh)		// LF (Not SAVE)
				{
					rcved_Seq = 53; // Recv Seq( Remove MSG)
					//LogPrint("N");
				}
				else if( 0x0D == recvCh)		// CR (Not SAVE)
				{
					rcved_Seq = 52; // Recv Seq( Remove MSG)
					//LogPrint("R");
				}
				else if( 0x25 == recvCh)		// "%" (Not SAVE)
				{
					if(50 == rcved_Seq)
					{
						rcved_Seq = 51;	// Recv Seq(STOP)
						// Data Save
						if (rcved_flag == 0) {
							rcvApMsg_Save = rcvApMsg;
							rcved_flag = 1;				// SET AP MSG RCVED
						}
						break;
					}
				}
				else			// Data Save
				{
					if(50 == rcved_Seq)
					{
						rcvApMsg += recvCh;
						if ((rcvApMsg.length() == 3) && (rcvApMsg == STX_DNLD_EXEC)) {
							g_DnExecStep = 1;
						}
					}
				}
			}
		}

	  #if (LOG_AP_RCV_MSG_PARSING)
		if(1 == rcved_flag)
		{
			LogPrintln(" LG] rcvMg " + rcvApMsg_Save);
		}
	  #endif
	}
}

	
/*  =====================================
       From PC, (Uart) Recv Data Hanlder From PC
  ===================================== */
void uartRecvHandler()
{
/*
	if(!Serial)
	{
		return;
	}
*/
#if (0)
  if (Serial.available())						// Length of Serial Buffer from PC
  {
    String msg = Serial.readStringUntil('%');	//Wait Recv "%", timeout 1sec, EndChar(%) is NOT INCLUDE
#endif

	if (0 == rcved_flag) 	// Length of Serial Buffer from PC
	{
		return;
  	}	// 처리안함

	// 수신때만 처리
	String msg = rcvApMsg_Save;
	rcvApMsg_Save = "";
	rcved_flag = 0; // MSG ANA FINISH

	msg.trim();				// Delete Space in String

	// msg있을때만 처리    
    if (msg == "")			// Return, if not Recv
    {
    	return;
    }
//	LogPrintln(" AP] ApCmd Recved len:" + String(g_SavePtr));
//	HexDump(0, g_readBuf, g_SavePtr);
//	g_SavePtr = 0;
//======================================================================
#if (LOG_AP_CMD)	//처리시간 개선,   LOG_AP = 0)
	// LOG OUT : AP CMD
	if(!(msg.startsWith(STX_SERVO_MOTOR)) && !(msg.startsWith(STX_MAINALIVE)))
	{
		if (msg.length() > 10) {
			LogPrintln(" AP] ApCmd " + msg.substring(0, 10) + "%");	  // LOG-SERIAL1
		}
		else {
			LogPrintln(" AP] ApCmd " + msg + "%");	  // LOG-SERIAL1
		}
	}
#endif
//========================================================================
	setTO(g_LastCmdTick);						// (V108)
	g_RecvCmdCount++;							// (V108)

	// ANA AP CMD & EXEC
	// Recv STX String Switch
	if(0) { /* Dummy */ }

	//($30) Rod Regist
    else if (msg.startsWith(STX_ROD_REGIST))	// ROD REGIST
	{
		anaRodRegist(msg);
    }

	// $00 - PC & MAIN Connection Check
    else if (msg.startsWith(STX_MAINALIVE))	// MAIN ALIVE
	{
		anaMainAlive(msg);
    }

	// $29 - AP INFO SEND
    else if (msg.startsWith(STX_AP_INFO_SEND))	// AP SET
	{
		anaApInfo(msg);
    }
	
	// $27 - TM INPUT TEST REQ
    else if (msg.startsWith(STX_TM_INPUT_REQ))	// INPUT TEST
	{
		if(AP_IS_TM == apType)
		{
			anaTmInputTestReq(msg);
		}
    }

	// $28 - TM OUTPUT TEST REQ
    else if (msg.startsWith(STX_TM_OUTPUT_REQ))	// OUTPUT TEST
	{
		if(AP_IS_TM == apType)
		{
			anaTmOutputTestReq(msg);
		}
    }
	

#if (NEW_IF)
	// ($01) DEVICE INFO
	else if (msg.startsWith(STX_INFO_REQ))
	{
		anaDeviceInfoReq(msg);
	}
#else
	// 3) $01 - Main Board ENB/DIS Setting
	else if (msg.startsWith(STX_GAMEENB))
	{
		anaGameEnbOver(msg);
	}
#endif
	
	
	// 1) $11 - Mac Addr READ - ack : M + my Addr + Targ Addr+%
    else if (msg.startsWith(STX_GET_ADDR))
	{
      String str = STX_GET_ADDR + "M" + eNow.getMyAddress() + "," + eNow.getTargetAddress() + "%";
      Resp2ApPrintln(str);
    }

	//2) $12 - Target Addr WRITE  - ack : void
    else if (msg.startsWith(STX_SET_ADDR))
	{
      msg = msg.substring(3, msg.length());		// Catch 3rd ~ end String
	  int len = msg.length();
      if((MAC_ADDR_OK_STR_LEN - 1) < len)
      {
      	// TBD , Return at FAIL
      	eNow.setTargetAddress(msg);
  		#if 1
	  		LogPrintln(" LG] T_ADDR Len:" + String(len) + ",msg:" + msg);
  		#endif
      }
	  else
	  {
	  	LogPrintln(" LG] ERROR T_ADR Len:" + String(len) + ",msg:" + msg);
	  }
    }

	// $02 - Main ( Torque Motor ) SET
    else if (msg.startsWith(STX_TORQUE_MOTOR))
    {
 		ana_MainMotControl(msg);
	}

	// $03 - Angle (Servo Motor) SET
    else if (msg.startsWith(STX_SERVO_MOTOR))
    {
		ana_AngleControl(msg);
    }

	// $04 - BOBBIN(BLDC) CONTROL by AP
    else if (msg.startsWith(STX_BBN_MOTOR))
    {
		//ana_BobbinControl(msg);
		ana_BobbinControl_Check(msg);
    }

#if (IO_LM_MOT)
	// $07 - LM(BLDC) CONTROL by AP
    else if (msg.startsWith(STX_LM_MOTOR))
    {
		//ana_BobbinControl(msg);
		ana_LineMotControl_Check(msg);
    }
#endif

#if (NEW_IF)
	// $06 - LED CONT CMD
	else if (msg.startsWith(STX_LED_CONT))
	{
		analedContCmd(msg);
	}
#endif		

	// $0A - Version Read
    else if (msg.startsWith(STX_VER_READ))
    {
    
    #if (NEW_IF)
      ApFgVer = msg.substring(3);
		#if (LOG_VER)
			LogPrintln(" LG] VerRq " + msg + ",FgVer: " + ApFgVer);		// LOG-SERIAL1
		#endif
	#endif

		if ((ROD_CONN == rod_conn_status) && (gRodSleepStat == 0))
		{
			eNow.write(STR_PID_ROD_VER_READ, "");		// Send Version READ to SLAVE
			// Responce is Sended When Recived Slave Version
		}
		else
		{
			String str;
			if (g_RespVersion[0] == '$')
				str = g_RespVersion;
			else
				str = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + "Vs99.99.99" + "," + "Vi99.99.99" + "%";
			Resp2ApPrintln(str);
			LogPrintln(" FM] Ver__ " + str);		// LOG-SERIAL1
		}
	}

	// $11 - Game Status
	else if (msg.startsWith(STX_GAME_STS))
    {
		anaGameStatus(msg);
	}

//#if (IO_LM_MOT || AUTO_TENSION_PID)
	// $25 - Game Status WTA
	else if (msg.startsWith(STX_GAME_STS_WAT))
    {
      #if (LOG_GAME_WAT_CMD)
	  	LogPrintln(" lg] GmWAT " + msg);
	  #endif
		anaGameStatusWAT(msg);
	}
//#endif

#if (!NEW_IF)
	// $07 - Rest Distance
	else if (msg.startsWith(STX_REST_DIST))
    {
		anaRestDistance(msg);
		
	}
#endif

	// [장치장애검지] 요구
	else if (msg.startsWith(STX_DEV_CONT_REQ))
    {
		anaDeviceContReq(msg);
		
	}
	
	// Recved RESP POWER OFF EXEC(by AP)
	
	else if (msg.startsWith(STX_APSTS_RESP))
	//else if (msg.startsWith("$17"))
	{
		int i = -1; 
		// TBD  toInt  is return " 0 "	- Need IF Edit
		i = msg.substring(3).toInt();
		if(0 == i)		// PC OFF
		{ 
			recvPwrOffExec_Resp = 1;
			//recvPwrOffExec_Resp = 1;
		  #if LOG_POWEROFF_FUNC
	  		LogPrintln(" LG] PWR_C RecvPowerOffExec from AP opt= " + String(i));
		  #endif
		}
	}

#if (NEW_IF)
	// MKT_TEST_2
	else if (msg.startsWith(STX_MOT_OUT_RATE_SET))
	{
		anaMotOutRateSet(msg);
	}
#endif

//==== Diag Input & Output
	// $DI - Diag Input
	else if (msg.startsWith(STX_DIAG_INPUT))
	{
		Resp2ApPrintln(msg);
		mDiag_entry();
		diagInputTest(msg);
	}
	// $D0 - Diag Output
	else if (msg.startsWith(STX_DIAG_OUTPUT))
	{
		Resp2ApPrintln(msg);
		mDiag_entry();
		diagOutputTest(msg);
	}
	else if (msg.startsWith(STX_DIAG_EXIT))
	{
		mDiag_exit();
	}


//====else Slave Connection ( Send Other STX to Slave ),  $05(Break) or $08(IMU Pass to SLAVE)
    //else if (isSlaveConn)
    //else if (ROD_CONN == rod_conn_status)
    
    //if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
    //{

	//test Beake Change
	else if (msg.startsWith(STX_BREAK_MOTOR))
	{
		// ROD연결 & 낚시대등록 아닌경우만 전송
		if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
		{
			// ROD보드Type(브레이크 유무)에 따라, 없는 경우는 전송하지 않음
	  		if(ROD_BOARD_V1 == rodBoardType)	// OLD BOARD Only
	  		{
		 		reqBreakMotor = msg.substring(3).toInt();	//
				String sendmsg = String(reqBreakMotor);		// For AP(3 char) & SETUP (8 char)
		 	  #if (AP_MOT_CONT)
				//msg = STX_BREAK_MOTOR + "00000" + msg.substring(3);
				eNow.write(STR_PID_ROD_BREAK, sendmsg);
		 	  #else
	     		if(!exeHit)
      	 		{
		   			//msg = STX_BREAK_MOTOR + "00000" + msg.substring(3);
		   			String sendmsg = msg.substring(3);
		   			eNow.write(STR_PID_ROD_BREAK, sendmsg);
      	 		}
		 	  #endif
	  		}
		}
	}

	// ($08) IMU DATA/ IMU SET
	else if (msg.startsWith(STX_IMU_SET))
	{
	//FW자체 Condition 확인을 위해 분석 추가
		// ROD연결 & 낚시대등록 아닌경우만 전송
		if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
		{	
		#if (NEW_IF)
			// IMPROVE TBD
			int act = msg.substring(3,(3+1)).toInt();		//Test
			if( (IMU_DATA_OFF == act) || (IMU_DATA_ON  == act) )
			{
				String sendmsg = msg.substring(3,(3+1));
				eNow.write(STR_PID_IMU_DATA_OUT, sendmsg);		// Control & LOG
				reqImuMeasFlag = act;

				// Send to ROD(OK) or to AP(NG)
				#if (RESP_IMU_DATA)
					String respStr = msg + "%";
					Resp2ApPrintln(respStr);	// 받은값 그대로 RETURN
					#if (LOG_IMU_RCV_1ST_TIME)
						if(1==reqImuMeasFlag)
						{
							imuRecv1stTime = 1;	//	1st Time Measure START
							LogPrintln(" LG] $0801");
						}
						else
						{
							imuRecv1stTime = 0;		//1st Time Measure END
							LogPrintln(" LG] $0800");
						}
					#endif
				#endif

				// IMU데이타 간격 측정
				if(AP_IS_TM == apType)	// Only TM
				{
					if(IMU_DATA_ON  == act) { imuIntervalCheckCnt = 1; }		// Interval측정 Start
					else					{ imuIntervalCheckCnt = 0; }		// Interval측정 Stop
				}
			}
			//else if(3==act)	{ msg = STX_IMU_SET + "10101010"; }
			//else if(4==act)	{ msg = STX_IMU_SET + "10100000"; }
			//else if(20000 < act) { msg = STX_IMU_SET + "0000" + String(reqImuMeasFlag-20000); }
			else	// Error
			{
				#if (RESP_IMU_DATA)
					String respStr = msg + "_ER%";
					Resp2ApPrintln(respStr);
				#endif
			}
		
		#else	// OLD_IF
			reqImuMeasFlag = msg.substring(8).toInt(); 	  //Test
			String sendmsg = msg.substring(8);
	    	eNow.write(STR_PID_IMU_DATA_OUT, sendmsg);		// Control & LOG
		#endif	// NEW_IF_END
		}
	}


	// VRT MOT CONT(ON/OFF)
	else if (msg.startsWith(STX_VRT_MOT_CONT))
	{
		String logMsg = "";
		if(ROD_BOARD_V2 == rodBoardType)
		{
			logMsg = anaVrtMotCmd(msg);
		}
		Resp2ApPrintln(msg + "%");
		
		#if (LOG_VRT_AP_CMD)
			LogPrintln(" LG] VrtMot " + logMsg);
	  	#endif
	}

	// ($24) 버튼 LED
	else if (msg.startsWith(STX_BTN_LED_CONT))
	{
		String logMsg = "";
		if(ROD_BOARD_V2 == rodBoardType)
		{
			logMsg = anaBtnLedCmd(msg);
		}
		Resp2ApPrintln(msg + "%");
	  
		#if (LOG_BTN_LED_AP_CMD)
	  		LogPrintln(" LG] BtnLed " + logMsg);
		#endif
	}

	// STX_SETUP_REQ
	else if(msg.startsWith(STX_DEV_SET_REQ))	// 설정 요구
	{
		String	rtsMsg = "";
		short errK = ERR_OK;
		
		rtsMsg = anaDevSetReq(msg);
		if(0 > rtsMsg.toInt())	// -1:실패, -2:범위오버, -3: 파라NG
		{
			// NG, 곧바로 응답
			errK = ERR_NG;
			Resp2ApPrintln(msg + rtsMsg + "%");
		}
		else
		{
			// OK, 실행후 응답
			//Resp2ApPrintln(msg + "00" + "%");
		}
	
		#if (LOG_MAINMOT_AUTOSET_ERR)
			if(ERR_OK > errK)
			{
				LogPrintln(" LG] DevFN " + msg + rtsMsg);
			}
		#endif
	}

	// DEV_FUNCTION_REQ($98)
	else if (msg.startsWith(STX_DEV_FUNCTION_REQ))
	{
		String	rtsMsg = STR_ERR_OK;
		short errK = ERR_OK;
		
		rtsMsg = anaDevFunctionReq(msg);
		//바로 응답
		Resp2ApPrintln(msg.substring(0,(0+6)) + rtsMsg + "%");	// OK , NG

		#if (LOG_MAINMOT_AUTOSET_ERR)
			errK = rtsMsg.toInt();
			if(ERR_OK > errK)
			{
				LogPrintln(" LG] DevFN " + msg + rtsMsg);
			}
		#endif
	}

	
//#if (IO_LM_MOT || AUTO_TENSION_PID)
	// LM DATA SAVE
	else if (msg.startsWith(STX_DATA_PARA_SET))
	{
		ana_SetDataSave(msg);
	}
//#endif

	// Download Main B/D ($DN)
	else if (msg.startsWith(STX_DNLD_EXEC))			// (V1081) 
	{
		Exec_Download(msg);
	}

	else if (msg.startsWith(STX_TEST_EXEC))			// (V108) 엔코더 값 콘솔에 출력
	{
		ana_TestExecution(msg);
	}
	
//==== Not Define AP_CMD, in ROD NOT_CONNECTED
	else
	{
  		// NOT DEFINE & SLAVE NOT CONN
	}

}

//
//
//	$23, AccNNNFFF%
String anaVrtMotCmd(String msg)
{
	String logMsg = " ok";

	int act = msg.substring(3,(3+1)).toInt();
	if(5 < act)
	{
		return " Ng act";;
	}

	switch(act)
	{
			case 0: // off
			case 1: // on
			case 2: // cnt
			case 3: // time
			case 4: // c+ time
			case 5: // Pattern, TBD
				//sendRodVrtCmd(act, cnt, ontime, offtime);
				eNow.write(STR_PID_REEL_VRT_CONT, msg.substring(3));		// Control & LOG
				break;

			default:
				break;
				
	}
	#if (LOG_VRT_AP_CMD)
		LogPrintln(" lg] VrtMot act:" + String(act) +",m:" + msg);
	#endif
	
	return logMsg;
}

//
// $99kk	// kk-구분
//	kk=xy (11~33)
//		: xydt(3)	x =1,2,3  /  y=1,2,3(a,b,c)  / dt (3개 SET lm,bm,tm)
//
//	kk = 40 : Bbn Enc 인터벌 Buff LOG출력
//	
void ana_SetDataSave(String msg)
{
	int act;
	int kind;
	int iPara;
	double dPara;
	int paraNG = 0;		// 0-OK
	String respMsg;
	String logMsg;

	int fish,pwr,d1,t1,d2,t2,d3,t3;
	
	act = msg.substring(3,(3+2)).toInt();	// 1
	switch(act)
	{
		case 0:		// TBD
			paraNG=1;
			break;
			
	  #if(IO_LM_MOT)
		case 11:		// lm Fish Level 1
		case 12:		// lm Fish Level 1
		case 13:		// lm Fish Level 1
		case 21:		// lm Fish Level 2
		case 22:		// lm Fish Level 2
		case 23:		// lm Fish Level 2
		case 31:		// lm Fish Level 3
		case 32:		// lm Fish Level 3
		case 33:		// lm Fish Level 3
			fish = act; // 1
			pwr = msg.substring(4,(4+1)).toInt();	// a
			if(1>pwr || 3<pwr) { paraNG =1; }
			else
			{
				d1 = msg.substring(5,(5+3)).toInt();
				t1 = msg.substring(8,(8+3)).toInt();
				d2 = msg.substring(11,(11+3)).toInt();
				t2 = msg.substring(14,(14+3)).toInt();
				d3 = msg.substring(17,(17+3)).toInt();
				t3 = msg.substring(20,(20+3)).toInt();
			
				// TBD Data Range Check
				stLmDutyTbl[fish][pwr].lm = d1;
				stLmDutyTbl[fish][pwr].lmTime = t1;
			
				stLmDutyTbl[fish][pwr].bm = d2;
				stLmDutyTbl[fish][pwr].bmTime = t2;
			
				stLmDutyTbl[fish][pwr].tm = d3;
				stLmDutyTbl[fish][pwr].tmTime = t3;

				#if (LOG_LM_SET_PARA_OUT)
					LogPrintln(" lg] LmParaSet " + String(fish) + "," + String(pwr) + ":"\
					+ String(stLmDutyTbl[fish][pwr].lm)+","+ String(stLmDutyTbl[fish][pwr].lmTime)+","\
					+ String(stLmDutyTbl[fish][pwr].bm)+","+ String(stLmDutyTbl[fish][pwr].bmTime)+","\
					+ String(stLmDutyTbl[fish][pwr].tm)+","+ String(stLmDutyTbl[fish][pwr].tmTime));
				#endif
			}
			break;
		  #endif		// IO_LM_MOT

		case 40:		// Enc INV LOG OUT
			// 조건문
			for(int i=0; i < ENC_INTV_BUFF_SIZE; i++)
			{
				LogPrintln(" lg] EnInt Interval:" +String(i)
					+ ","+ String(stEncIntvBuff[i].tick)
					+ ","+ String(stEncIntvBuff[i].width)
					+ ","+ String(stEncIntvBuff[i].dir)
					+ ","+ String(stEncIntvBuff[i].enc)
					+ ","+ String(stEncIntvBuff[i].dist)
					);
			}

			break;

		case 41:		// PID ParaSet
			wat_PidParaSet(msg);
			break;
			
		default:
			paraNG = 1;
			break;
	}
	
	// Response
	respMsg = msg.substring(0,(0+5)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);

}

//
// ROD BTN LED Control
//	$24,XA ccNNNFFF
String anaBtnLedCmd(String msg)
{
		String logMsg = " ok";
		//if(ROD_BOARD_V2 == rodBoardType)
		{
			int act = msg.substring(4,(4+1)).toInt();		// 2nd CHAR, ACT
			if(5 < act)
			{
				return " Ng act";;
			}
			
			int posi = msg.substring(3,(3+1)).toInt();		// 1st CHAR , LED POSITION 1=LEFT, 2=RIGHT
			if(BTN_LED_ALL_POSI == posi && (0==act||1==act))	// ALL OFF,ON
			{
				// NA, PASS to ROD
			}
			else if(BTN_LED_LAST_POSI < posi)		// MAX이상이면 처리 안함
			{
				return " Ng posi";
			}
			
			switch(act)
			{
				case 0:	// off
				case 1:	// on
				case 2: // cnt
				case 3: // time
				case 4: // cnt+time
				case 5:	// pattern
					eNow.write(STR_PID_BTN_LED_CONT, msg.substring(3));		// Control & LOG
					break;
		  
				default:
					break;
			}
			#if(LOG_BTN_LED_AP_CMD)
				LogPrintln(" lg] BtnLED act:" + String(act) +",m:" + msg);
			#endif
		}
	return logMsg;	
}


#define STR_RTS_OK			"00"
#define STR_RTS_NG  		"-1"
#define STR_RTS_RANGE_OVER	"-2"
#define STR_RTS_PARANG  	"-3"

//-----------------------------------------------
// 장치 설정 수행 요구
//-----------------------------------------------
String anaDevSetReq(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iKind = msg.substring(3,(3+2)).toInt();		// 2nd CHAR, kk
	switch(iKind)
	{
		case 0:
			rts = STR_RTS_PARANG;
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			if(AP_IS_TM == apType)
			{
				rts = anaDevSet_MainMotor_AutoSet(msg);
			}
			else
			{
				rts = STR_ERR_FAIL;
			}			
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}

// 메인모터 최소출력값 자동설정
String anaDevSet_MainMotor_AutoSet(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iAct = msg.substring(5,(5+1)).toInt();		// 1 CHAR, ACT
	switch(iAct)
	{
		case 0:
			mainMot_MinOut_AutoSet_Stop(msg);	// void
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			mainMot_MinOut_AutoSet_Start(msg);
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


//-----------------------------------------------
// 장치 기능 수행 요구
//-----------------------------------------------
String anaDevFunctionReq(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iKind = msg.substring(3,(3+2)).toInt();		// 2nd CHAR, kk
	switch(iKind)
	{
		case 0:
			rts = STR_RTS_PARANG;
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			if(AP_IS_TM == apType)
			{
				rts = anaDevFunc_wireAging(msg);
			}
			else
			{
				rts = STR_ERR_FAIL;
			}
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


// 에이징 테스트 , 낚시줄
String anaDevFunc_wireAging(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iAct = msg.substring(5,(5+1)).toInt();		// 1 CHAR, ACT
	switch(iAct)
	{
		case 0:
			wireAging_Test_Stop(msg);
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			wireAging_Test_Start(msg);
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


//---------------------------------------------------------------------
// Start
void mainMot_MinOut_AutoSet_Start(String msg)
{
	mainMotAutoSet_Flag = 1;
	mainMotAutoSet_Step = 10;
}

void mainMot_MinOut_AutoSet_Stop(String msg)
{
	mainMotAutoSet_Flag = 0;
	mainMotAutoSet_Step = 0;
	torqMotor_DefaultOut();		// 토크 DEFAULT출력
	bbnMotor.offBldc();			// BLDC OFF
}

// 토크모터 초기값 출력 혹은 AP요구값 출력
void torqMotor_DefaultOut()
{
	if(recv_TorqMotor_Flag)
	{
		torqMotor.setValue(reqTorqueMotor);
	}
	else
	{
		torqMotor.setValue(defaultTorqueMotor);
	}
}

//-------------------------------------------------------------
#define MAINMOT_MIN_RETRY_CNT	(1)	// Retry 5회(총 5회)


// 1ms => 100us
//#define MAINENCA_SPEC_PERIOD		(10)		// 10.5ms
//#define MAINENCA_SPEC_PERIOD		(109)		// 10.5ms = [10.5 *1000/100]

//#define MAINENCA_SPEC_ADD 2
//#define MAINENCA_SPEC_PERIOD_SP_HIGH	(100 + MAINENCA_SPEC_ADD)		// 10.0 ms
//#define MAINENCA_SPEC_PERIOD_SP_MID		(110 + MAINENCA_SPEC_ADD)		//11.0 ms
//#define MAINENCA_SPEC_PERIOD_SP_LOW		(120 + MAINENCA_SPEC_ADD)		//12.0 ms

#define MAINENCA_SPEC_PERIOD_SP_HIGH	102 // (100 + MAINENCA_SPEC_ADD)		// 10.0 ms
#define MAINENCA_SPEC_PERIOD_SP_MID		112 // (110 + MAINENCA_SPEC_ADD)		//11.0 ms
#define MAINENCA_SPEC_PERIOD_SP_LOW		122 // (120 + MAINENCA_SPEC_ADD)		//12.0 ms


// 보빈 회전 카운트
#define BBN_REV_1TIME_SIGCHG_CNT	(18*2*1)
#define BBN_REV_2TIME_SIGCHG_CNT	(18*2*2)
#define BBN_REV_3TIME_SIGCHG_CNT	(18*2*3)
#define BBN_REV_4TIME_SIGCHG_CNT	(18*2*4)

#define BBN_REV_READY_SIGCHG_CNT	(70*2*1)
#define BBN_REV_TIMEOUT_SIGCHG_CNT	(80*2*1)

#define MAINMOT_MIN_AUTOSET_TO_1S		1000 //
#define MAINMOT_MIN_AUTOSET_TO_2S		2000 //
#define MAINMOT_MIN_AUTOSET_TO_2p5S		2500 //
#define MAINMOT_MIN_AUTOSET_TO_3S		3000 //
#define MAINMOT_MIN_AUTOSET_TO_3p2S		3200 //


#define MAINMOT_MIN_AUTOSET_REV_1ST_TIME	500 // 200 //500 //300	//
#define MAINMOT_MIN_AUTOSET_REV_TIME	1200 // 1500 

#define MAINMOT_MIN_AUTOSET_BREAK_TO		50

unsigned long lastPulseWidth = 9999;
unsigned long pulseWidthMs = 9999;

#define PULSE_RANGE_OK (MAINENCA_SPEC_PERIOD_SP_HIGH < pulseWidthMs) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs)

#define PULSE_MEASURE_CNT	4	// 4부터 측정( 4이상)
#define PULSE_OK_MIN_CNT	6
#define PULSE_OK_SPEC_CNT	36	// 2회전

//--------------------------------------------------------------
//	1) 실행
//	2) 결과
//		- 성공 :
//		- 실패 : 엔코더 나오지 않음
//		- 실패2 : 성공했지만 범위(60~80)을 넘음.
//	3) 결과를 "반환용문자(strMainMot_AutoSet_Result)"에 저장
//--------------------------------------------------------------

// NOT USE
void mainMot_MinOut_AutoSet_Control1_Period()
{
		static unsigned long _to;
	
		static unsigned long _toSenChg;

		static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;
	
		static int _out;
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
	
	
#if ( LOG_MMOT_AUTOSET_STEP)
		// Step Change LOG OUT
		if(oldmainMotAutoSet_Step != mainMotAutoSet_Step )
		{
			LogPrintln(" lg] MinSt : " + String(mainMotAutoSet_Step));
			oldmainMotAutoSet_Step = mainMotAutoSet_Step;
		}
#endif
		
		switch(mainMotAutoSet_Step)
		{
	
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				_toReverseTime = 0;
				
				_chgCnt = 0;
				_toSenChg = 0;
				speed_high = 0;
				errK = ERR_OK;
				_out = (70+1);		// 최소값 70보다 크게.
				_deltaOut = 0;

				setTO(_to);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				if(0 == _retryCnt)	{ _toReverseTime = MAINMOT_MIN_AUTOSET_REV_1ST_TIME; }
				else				{ _toReverseTime = MAINMOT_MIN_AUTOSET_REV_TIME; }
				//
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 5회 OVER
				{
					errK = ERR_RETRY_OVER;		// REPEAT OVER
			
					setTO(_to);
					mainMotAutoSet_Step = 75; // ERROR
				}
				else	// OK
				{
					// 반복 1 ]  BLDC CCW로 돌려 줄을 풀어줌
					bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );	// 줄 풀기(Duty 50)

					setTO(_to);
					mainMotAutoSet_Step = 21;		
				}
				break;
				
			case 21:
				if(checkTO(_to, _toReverseTime) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					torqMotor.setValue(70);	//DUTY = 70 START
					setTO(_to);
					mainMotAutoSet_Step = 22;	// REPEAT
				}
				break;
				
			case 22:
				//if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
				if(checkTO(_to, 80) )	//WAIT 100 ms TO
				{
					//bbnMotor.offBldc();
					torqOff();
					setTO(_to);
					mainMotAutoSet_Step = 23;
				}
				break;

			case 23:
				//if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
				if(checkTO(_to, 70) )	//WAIT 100 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 30;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				_chgCnt = 0;
				_oldSen = LVL01_BBNENCA;
				pulseWidthMs = 9999;				
			//
				// == 펄스측정 START SET ???
			//
				torqMotor.setValue(_out);	//DUTY = 70 START
				setTO(_to);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) SEN CHANGE L=>H or H=>L
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy
				// 3-1) 3초 경과, 펄스OVER 혹은 T/O 체크
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					// 회전을 못함 = 반복안함
					if(PULSE_MEASURE_CNT > _chgCnt)	//  센서 변화가 없음( 4번 미만)
					{
						errK= ERR_FAIL;
						setTO(_to);
						mainMotAutoSet_Step = 75;
					}

					//else if(!PULSE_RANGE_OK)
					else
					{
						// 정지 & 반복
						_deltaOut = ( (pulseWidthMs - MAINENCA_SPEC_PERIOD_SP_MID)*10/15 );
						if(0 == _deltaOut)
						{
							if(MAINENCA_SPEC_PERIOD_SP_MID < pulseWidthMs)	// SP_LOW
								_deltaOut++;
							else
								_deltaOut--;
						}
						_out += _deltaOut;
						if(60 > _out) {_out = 60;}
						else if(80 < _out) {_out = 80;}
						/*
						if(MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs)
						{
							_out--; 		// 감소는 -1 씩
						}
						else
						{
							_out++;
						}
						*/
						setTO(_to);
						mainMotAutoSet_Step = 32;		// REPEAT			
					}
				}

			
				// 3-2) 2초 경과, 조건 만족?
				//else if( speed_high && PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2p5S) )
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2S) )
				{
					// 2nd 조건 만족?
					if(speed_high && PULSE_RANGE_OK )
					{
						speed_high = 0;
						//_out++;
					 	if( 59 < _out && 81 > _out) // 60 ~ 80
						{
							errK = ERR_OK;		//성공
							// 설정값 저장, _out
						}
						else
						{
							errK = ERR_RANGE_OVER; //RANGE OVER?
						}
						setTO(_to);
						mainMotAutoSet_Step = 75;					
					}
					// 조건 불만족, 1st조건 만족?
					else if(!speed_high && (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs))
					{
						speed_high = 1;
						// OVER SPEED = OK
						_out--; 		// 감소는 -1 씩
						setTO(_to);
						mainMotAutoSet_Step = 32;		
					}
				}

				// 2초 전, 1st조건 만족?
				else if( !speed_high && (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs))	// (속도빠름) = SPEC 10.5 ms
				{
					speed_high = 1;
					// OVER SPEED = OK
					_out--; 		// 감소는 -1 씩
					setTO(_to);
					mainMotAutoSet_Step = 32;
				}
				break;

			// TORQ OFF
			case 32:
				// Last Pulse Width 저장
				lastPulseWidth = pulseWidthMs;
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;					

				#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
					LogPrintln(" lg] MMINS last Pulse Width:" + String(lastPulseWidth));
				#endif

				break;

			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// TORQ STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{
					_retryCnt++;		// 반복회수 증가
					setTO(_to);
					mainMotAutoSet_Step = 20;	// REPEAT
				}
				break;


			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				lastPulseWidth = pulseWidthMs;
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(_out));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = (int)_out;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FAIL;
						mainMotAutoSet_Step = 80; //NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}
				#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
					LogPrintln(" lg] MMINS last Pulse Width:" + String(lastPulseWidth));
				#endif
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				#if (LOG_DEVCHK_TORQ_ERR)
					LogPrintln(" LG] DevRq BLDC_24V ERR 6050");
				#endif
				
				break;
			
			case 90:
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 91;					
				break;

			case 91:
				if(checkTO(_to, 50) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 93;	// REPEAT
				}
				break;
			// OK
			case 93:

				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
				#if (LOG_DEV_CHECK_OK)
					LogPrintln(" LG] SetRQ Mmot MINSET REQ: " + String(defaultTorqueMotor));
				#endif
		
				break;
			
			default:
				break;
				

		}

}

int minOut = 0;
//---------------------------------------------------------------
// 일정속도 도달까지의 시간을 측정, 최소출력값으로 환산
//---------------------------------------------------------------
void mainMot_MinOut_AutoSet_Control2_Time()
{

		static unsigned long _to;
		static unsigned long _toErr;
		static long okTime;
		static long logTime;
	
		static unsigned long _toSenChg;

		static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;

		static int _out;
		
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
		String logMsg;
	
	
#if ( LOG_MMOT_AUTOSET_STEP)
		// Step Change LOG OUT
		if(oldmainMotAutoSet_Step != mainMotAutoSet_Step )
		{
			LogPrintln(" lg] MinSt : " + String(mainMotAutoSet_Step) );
			oldmainMotAutoSet_Step = mainMotAutoSet_Step;
		}
#endif
		
		switch(mainMotAutoSet_Step)
		{
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				_toReverseTime = 0;
				
				_chgCnt = 0;
				_toSenChg = 0;
				speed_high = 0;
				errK = ERR_OK;
				
				_out = (0);		// 최소값 70보다 크게.
				minOut = 0;
				_deltaOut = 0;

				okTime = 1060;		// 계산오류 방지
				logTime = 5555;		// 계산오류 방지

				setTO(_to);
				setTO(_toErr);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				_retryCnt++;
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 2회 OVER
				{
					if(ERR_OK == errK) { errK = ERR_RETRY_OVER; }		//반복오버  ( -1 )
					setTO(_to);
					mainMotAutoSet_Step = 75; //반복회수 오버 => 장애 처리
				}
				else	// OK
				{
					torqMotor.setValue(70);	//DUTY = 70 START
					setTO(_to);
					mainMotAutoSet_Step = 30;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				_chgCnt = 0;
				_oldSen = LVL01_BBNENCA;
				pulseWidthMs = 9999;

				setTO(_to);
				setTO(_toErr);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) 엔코더 신호 변화 검지.
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy

				// 		3-1) 3초 경과, 펄스OVER 혹은 T/O 체크
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					logTime = getTO(_to);
					// 회전을 못함 = 반복안함
					setTO(_to);
					if(PULSE_OK_MIN_CNT > _chgCnt)
					{
						errK= ERR_NOT_ROTATE;	//( -6 )
						mainMotAutoSet_Step = 75;		//회전 안됨 => 바로 장애
					}
					else
					{
						errK= ERR_TO_OVER;	// ( -4 )
						mainMotAutoSet_Step = 32;		//시간경과 => 반복
					}
				}
				
				//		3-2) 일정속도(펄스폭) 도달
				else if( (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs) )
				{
					lastPulseWidth = pulseWidthMs;
					okTime = getTO(_to);
					logTime = okTime;
					if(0 > okTime)
					{
						errK = ERR_FW_CAL_NG; //RANGE OVER? ( - 2 )
						setTO(_to);
						mainMotAutoSet_Step = 75;		// 급정지
					}
					else
					{
						_out = MAIN_MOT_DEFAULT_MIN_DUTY + ((okTime - 1060)/88);
						//_out = 50 + ((okTime - 1060)/176);	// TEST
						//_out = 50 + ((okTime - 1060)/27);		// TEST
						// 계산 결과 범위 밖 : 60 ~80 외
						//if(60 > _out && 80 < _out)
						//if(60 > _out || 85 < _out)
						if(MAIN_MOT_DEFAULT_MIN_DUTY > _out)
						{
							// Error
							minOut = MAIN_MOT_DEFAULT_MIN_DUTY;
						}
						else if (MAIN_MOT_DEFAULT_MAX_DUTY < _out)
						{
							minOut = MAIN_MOT_DEFAULT_MAX_DUTY;
						}
						else
						{
							minOut = _out;
						}				
						errK= ERR_OK;	// 성공
						setTO(_to);
						mainMotAutoSet_Step = 32;		// 급정지
					}
				}
				break;

			
			// 반복 , TORQ OFF
			case 32:
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;
				break;

			// 급정지 대기
			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// BLDC STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{
					#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
						LogPrintln(" lg] MMINS last Pulse Width:" + String(lastPulseWidth));
					#endif

					if(ERR_OK == errK)
					{
						setTO(_to);
						mainMotAutoSet_Step = 75; 	//성공 => 종료
					}
					else
					{
						setTO(_to);
						mainMotAutoSet_Step = 20;	// REPEAT
					}
				}
				break;

			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(minOut));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = minOut;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FS_WRITE;		// FS쓰기 에러( -7 )
						mainMotAutoSet_Step = 80;	//NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}

				// 검출결과, AP에 출력(확인용)
				Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG") + ",try:" + String(_retryCnt) + ",Out:" + String(_out)  + ",Save:" + String(minOut) +",ms:" + String(logTime) );
				
				#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
					LogPrintln(" lg] MINST Result,Time,Width:" + String(errK) + "," +  String(logTime)+ "," + String(lastPulseWidth));
				#endif
				
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				#if (LOG_DEVCHK_TORQ_ERR)
					LogPrintln(" LG] DevRq BLDC_24V ERR 6050");
				#endif
				
				break;

				// 성공
			case 90:
				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
				#if (LOG_DEV_CHECK_OK)
					LogPrintln(" LG] SetRQ Mmot MINSET REQ: " + String(defaultTorqueMotor));
				#endif
		
				break;

			default:
				break;
	}
}

#define MMOT_AUTOSET_OUT_DUTY	80	// 80 Duty
//---------------------------------------------------------------
//  측정시 출력 : 80 Duty
//	측정 : 36 펄스(2회전)까지의 경과 시간 측정
// 	에러: 3.2초(완전율 2배)경과해도 36펄스 도달못하면 장애.
//---------------------------------------------------------------

void mainMot_MinOut_AutoSet_Control3_36PulseTime()
{

		static unsigned long _to;
		
		static long okTime = 0;
	
		//static unsigned long _toSenChg;

		//static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;

		static int _out;
		//static int _hz;
		
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		//static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
		String logMsg;
	
	
#if ( LOG_MMOT_AUTOSET_STEP)
		// Step Change LOG OUT
		if(oldmainMotAutoSet_Step != mainMotAutoSet_Step )
		{
			LogPrintln(" lg] MinSt : " + String(mainMotAutoSet_Step) );
			oldmainMotAutoSet_Step = mainMotAutoSet_Step;
		}
#endif
		
		switch(mainMotAutoSet_Step)
		{
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				//_toReverseTime = 0;
				
				_chgCnt = 0;
				//_toSenChg = 0;
				//speed_high = 0;
				errK = ERR_OK;
				
				_out = (0);		//
				//_hz = 2000;
				minOut = 0;
				//_deltaOut = 0;

				okTime = 0;		// 계산오류 방지
				//logTime = 0;		// 계산오류 방지

				setTO(_to);
				//setTO(_toErr);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				_retryCnt++;
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 2회 OVER
				{
					if(ERR_OK == errK) { errK = ERR_RETRY_OVER; }		//반복오버  ( -1 )
					setTO(_to);
					mainMotAutoSet_Step = 75; //반복회수 오버 => 장애 처리
				}
				else	// OK
				{
					_chgCnt = 0;
					_oldSen = LVL01_BBNENCA;
					pulseWidthMs = 9999;
					
					torqMotor.setValue(MMOT_AUTOSET_OUT_DUTY);	//DUTY = 70 START
					setTO(_to);
					//mainMotAutoSet_Step = 30;
					mainMotAutoSet_Step = 31;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				//_chgCnt = 0;
				//_oldSen = LVL01_BBNENCA;

				setTO(_to);
				//setTO(_toErr);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) 엔코더 신호 변화 검지.
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy

				// 		3-1) 3.2초 경과, 36펄스 미도달
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3p2S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					okTime = getTO(_to);
					lastPulseWidth = pulseWidthMs;
					// 회전을 못함 = 반복안함
					setTO(_to);
					if(PULSE_OK_MIN_CNT > _chgCnt)
					{
						errK= ERR_NOT_ROTATE;	//( -6 )
						mainMotAutoSet_Step = 75;		//회전 안됨 => 바로 장애
					}
					else
					{
						errK= ERR_TO_OVER;	// ( -4 )
						mainMotAutoSet_Step = 32;		//시간경과 => 반복
					}
				}
				
				//		3-2) 36펄스(2회전) 도달
				else if( (PULSE_OK_SPEC_CNT*2) < _chgCnt )
				{
					lastPulseWidth = pulseWidthMs;
					okTime = getTO(_to);
					//Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG") + ",ms:" + String(okTime));
					//logTime = okTime;
					if(0 > okTime)
					{
						errK = ERR_FW_CAL_NG; //RANGE OVER? ( - 2 )
						setTO(_to);
						mainMotAutoSet_Step = 75;		// 급정지
					}
					else
					{
						if(900 > okTime)
						{
							_out = 10 + (0.0667*okTime);
							//_out = 0 + (0.0667*okTime);		// TEST
						}
						else
						{
							_out = 53 + ( 0.0175*okTime);
						}

						if(MAIN_MOT_DEFAULT_MIN_DUTY > _out)
						{
							// Error
							minOut = MAIN_MOT_DEFAULT_MIN_DUTY;
						}
						else if (MAIN_MOT_DEFAULT_MAX_DUTY < _out)
						{
							minOut = MAIN_MOT_DEFAULT_MAX_DUTY;
						}
						else
						{
							minOut = _out;
						}				
						errK= ERR_OK;	// 성공
						setTO(_to);
						mainMotAutoSet_Step = 32;		// 급정지
					}
				}
				break;

			
			// 반복 , TORQ OFF
			case 32:
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;
				break;

			// 급정지 대기
			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// BLDC STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{
					#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
						LogPrintln(" lg] MMINS last Pulse Width:" + String(lastPulseWidth));
					#endif

					if(ERR_OK == errK)
					{
						setTO(_to);
						mainMotAutoSet_Step = 75; 	//성공 => 종료
					}
					else
					{
						setTO(_to);
						mainMotAutoSet_Step = 20;	// REPEAT
					}
				}
				break;

			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(minOut));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = minOut;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FS_WRITE;		// FS쓰기 에러( -7 )
						mainMotAutoSet_Step = 80;	//NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}

				// 검출결과, AP에 출력(확인용)
				Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG")
					+ ",try:" + String(_retryCnt) + ",Out:" + String(_out)  + ",Save:" + String(minOut) 
					+",cnt:" + String(_chgCnt) +",ms:" + String(okTime) +",plsWid:" + String(lastPulseWidth));
				
				#if (LOG_MAINMOT_AUTOSET_LAST_PULSE)
					LogPrintln(" lg] MINST Result,Time,Width:" + String(errK) + "," +  String(okTime)+ "," + String(lastPulseWidth));
				#endif
				
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				#if (LOG_DEVCHK_TORQ_ERR)
					LogPrintln(" LG] DevRq BLDC_24V ERR 6050");
				#endif
				
				break;

				// 성공
			case 90:
				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
				#if (LOG_DEV_CHECK_OK)
					LogPrintln(" LG] SetRQ Mmot MINSET REQ: " + String(defaultTorqueMotor));
				#endif
		
				break;

			default:
				break;
	}
}


//-------------------------------------------------------------------------
//--- TORQ OFF CONTROL for STBY

void stbyTorqOffControl_Start()
{
	// [장치장애검지] 중이면, 요구Flage세트
	if(devReadychkFlag)
	{
		stbyTorqOffControl_Req_Flag = 1;	// []장치장애검지]처리 종료까지 대기.
	}
	else
	{
		stbyTorqOffControl_Req_Flag = 0;
		stbyTorqOffControl_Flag = 1;		//Control ENB
		stbyTorqOffControlStep = STEP_START;
	}
}

// (V108) flag 추가함 - 0:토크모터 동작 안함. 이외 기본값 출력
void stbyTorqOffControl_Stop(int flag)
{
	int outTorq=0;
	//if(stbyTorqOffControl_Flag)
	{
		stbyTorqOffControl_Flag = 0;		// Control DIS
		stbyTorqOffControlStep = STEP_IDLE;

		if (flag)
		{
			if(recv_TorqMotor_Flag)
			{
				torqMotor.setValue(reqTorqueMotor);	//토크 =  AP 요구값
				outTorq = reqTorqueMotor;
			}
			else
			{
				torqMotor.setValue(defaultTorqueMotor); //토크 = 최소출력값 OUT
				outTorq = defaultTorqueMotor;
			}
		}
		bbnMotor.offBldc();						// BLDC OFF
		// FW자체 출력 응답
		//Resp2ApPrintln(STX_CMD_TORQUE_MOTOR + fillZero2String3Char(defaultTorqueMotor) + "%");
		send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);		//FW자체출력응답
		//Resp2ApPrintln(STX_CMD_BLDC_MOTOR + "00000000%");	// $04AdddTTTT%
		Resp2ApPrintln(STX_BBN_MOTOR + "00000000%");	// $04AdddTTTT%
	}
}

// [STBY 토크OFF제어] 요구가 있는 경우, OFF제어 시작
void stbyTorqOff_ReqCheck()
{
	if(stbyTorqOffControl_Req_Flag)
	{
		// TBD : 일정시간후에 시작 필요(조작감)

		// 예약이면, "대기OFF제어"를 시작
		stbyTorqOffControl_Req_Flag = 0;
		stbyTorqOffControl_Start();	
	}
}

//------------
#define ENC_STOP_WAIT_TO		CHECK_TIME_1SEC_TO //3000	// 2 SEC
#define TENSION_HOLD_TIME_TO	7000 // 5000	// 5SEC

//#define STBY_OFF_CONT_SEN	LVL01_BBNENCA	// 1ms 센서 데이타
#define STBY_OFF_CONT_SEN	LVL10_BBNENCA	// 10ms 센서 데이타
#define STBY_OFF_CONT_ENCAB	LVL10_ENCAB		// 10ms 센서 데이타

#define	STBY_BBN_MIN_DUTY		10
#define	BBN_MOVE_DETECT_CNT		10
//---------------------------------------------
// 1) 대기중에 OFF제어 개시.
//  -- STBY시 토크OFF
// 	-- 엔코더 감시중 변화 검지 => 최소토크 출력
// 	--엔코더 감시중 정지 검지 => 토크 OFF
// 2) 대기중에서 상태가 변경되면
//  -- 토크 최소출력값 출력후
//  --OFF제어 중지
// 3) STBY 토크 OFF중에는 AP명령 무시
//----------------------------------------------
// CALL 10ms
//----------------------------------------------
#if 0
void stbyTorqOff_Control()
{
	static unsigned long _to;
	static unsigned long _toErr;
	static unsigned long _toChk;

	static unsigned short oldstbyTorqOffControlStep;
	static short errK = ERR_OK;

	static int _checkFlg = 0;
	static int _retryCnt = 0;
	static unsigned int	  _EncCnt = 0;
	static unsigned short _oldSen;
	unsigned short _newSen, _chgSen;

	static unsigned short _TotalCntA = 0;
	static unsigned short _TotalCntB = 0;

//	static int _nBldcDuty[] = { 10, 20, 30, 20, 10, 0 };
//	static int _nBldcDuty[] = { 10, 20, 30, 20, 10, 7, 4, 2, 1, 0 };
//	static int _nBldcDuty[] = { 10, 20, 30, 20, 10, 8, 4, 2, 1, 0 };
//	static int _nBldcTime[] = { 1000, 1000, 1000, 1000, 1000, 200, 100, 50, 25, 0 };
	static int _nBldcDutyUp[] = {   10,   18,   24,   30, 0 };
	static int _nBldcTimeUp[] = { 1000, 1000, 1000, 1000, 0 };
	static int _nBldcDuty[] = {  14,  10,    8,   4,   2,   1, 0 };
	static int _nBldcTime[] = { 100, 800, 1400, 800, 400, 100, 0 };

	static unsigned int		_chgSaveA;
	static unsigned int		_chgProcA;
	static unsigned int		_chgSaveB;
	static unsigned int		_chgProcB;
	#define	_CHG_SAVE_CNT	16
	#define	_CHG_SAVE_MSK	(_CHG_SAVE_CNT-1)
	static unsigned long	_TimeOutA[_CHG_SAVE_CNT] = {0};
	static unsigned long	_TimeOutB[_CHG_SAVE_CNT] = {0};
	String msg;

#if (LOG_STBY_TORQ_OFF_STEP)
	// Step Change LOG OUT
	if(oldstbyTorqOffControlStep != stbyTorqOffControlStep )
	{
		LogPrintln(" lg] ChgST Stby Torq OFF : " + String(stbyTorqOffControlStep));
		oldstbyTorqOffControlStep = stbyTorqOffControlStep;
	}
#endif		
	switch(stbyTorqOffControlStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			torqOff();
			bbnMotor.offBldc();

			//== 먼저 "줄끊어짐 검지"==
			_checkFlg = 0;
			errK = ERR_OK;
			setTO(_to);
			stbyTorqOffControlStep = 11;
			break;

		// 모터정지 대기
		case 11:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				stbyTorqOffControlStep = 12;
			}
			break;

		//BLDC 모터 기동(최소값:10) 줄 서서히 감기
		case 12:
			_retryCnt = 0;
			_EncCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;

			stbyTorqOffControlStep = 14;
			break;

		// 1초 마다 줄을 조금씩 세게 당겼다 서서히 힘 풀기(10->30->10->20->10)
		case 14:
			if(_nBldcDutyUp[_retryCnt] == 0)
			{
				bbnMotor.offBldc();
				stbyTorqOffControlStep = 17;
			}
			else
			{
				bbnMotor.onBldc(BBN_MOT_CW, _nBldcDutyUp[_retryCnt]);			// BBN 출력
				_toChk = _nBldcTimeUp[_retryCnt];
				setTO(_to);
				setTO(_toErr);
				_retryCnt++;
				stbyTorqOffControlStep = 15;
			}
			break;

		// 10초간 센서가 계속 변하면 줄 끊어짐. 1초간 센서 유지되면 다음 duty(case4)로 넘어감
		case 15:
			if( (_checkFlg == 0) && checkTO(_toErr, CHECK_TIME_10SEC_TO) )
			{
				//ERR
				errK = ERR_WIRE_BROKEN;		// 10초 경과 줄 끊어짐
				setTO(_to);
				stbyTorqOffControlStep = 75;
			}
			// OK
			else if( checkTO(_to, _toChk) )			//ENC센서 안정후 1초 경과 => OK
			{
				_checkFlg = 1;
				stbyTorqOffControlStep = 14;		// 대기OFF제어 시작
			}
			else if(STBY_OFF_CONT_SEN != _oldSen)
			{
				_oldSen = STBY_OFF_CONT_SEN;
				_EncCnt++;
				setTO(_to);
				if (_EncCnt >= BBN_MOVE_DETECT_CNT) {
					_retryCnt = 0;
					stbyTorqOffControlStep = 17;		// 대기OFF제어 시작
				}
			}
			break;

		// 1초 마다 줄을 조금씩 세게 당겼다 서서히 힘 풀기(10->30->10->20->10)
		case 17:
			if(_nBldcDuty[_retryCnt] == 0)
			{
				bbnMotor.offBldc();
				stbyTorqOffControlStep = 20;
			}
			else
			{
				bbnMotor.onBldc(BBN_MOT_CW, _nBldcDuty[_retryCnt]);			// BBN 출력
				_toChk = _nBldcTime[_retryCnt];
				setTO(_to);
				setTO(_toErr);
				_retryCnt++;
				stbyTorqOffControlStep = 18;
			}
			break;

		// 10초간 센서가 계속 변하면 줄 끊어짐. 1초간 센서 유지되면 다음 duty(case4)로 넘어감
		case 18:
			if( (_checkFlg == 0) && checkTO(_toErr, CHECK_TIME_10SEC_TO) )
			{
				//ERR
				errK = ERR_WIRE_BROKEN;		// 10초 경과 줄 끊어짐
				setTO(_to);
				stbyTorqOffControlStep = 75;
			}
			// OK
//			else if( checkTO(_to, CHECK_TIME_1SEC_TO) )	//ENC센서 안정후 1초 경과 => OK
			else if( checkTO(_to, _toChk) )				//ENC센서 안정후 1초 경과 => OK
			{
				_checkFlg = 1;
				stbyTorqOffControlStep = 17;		// 대기OFF제어 시작
			}
			else if(STBY_OFF_CONT_SEN != _oldSen)
			{
				_oldSen = STBY_OFF_CONT_SEN;
				// TBD, ENC방향 검지(유저가 당겼다 놓았다는 건 방향이 계속 바뀌니까)
				setTO(_to);
			}
			break;

		// OFF제어 개시 & 반복
		case 20:
			_chgSaveA = 0;
			_chgProcA = 0;
			_TotalCntA = 0;
			_chgSaveB = 0;
			_chgProcB = 0;
			_TotalCntB = 0;
			_oldSen = STBY_OFF_CONT_ENCAB;	// 센서상태 저장
			_EncCnt = iIsrbbnEncCnt;

			stbyTorqOffControlStep = 22;
			break;
			
		//엔코더 변화 check (at OFF상태)
		case 22:
			// 센서변화 CHECK
			_newSen = STBY_OFF_CONT_ENCAB;		// 엔코더 센서 AB를 _newSen에 저장
			_chgSen = _oldSen ^ _newSen;		// 변경된 센서 Bit를 _chgSen에 저장
			if(_chgSen)							// 변경된 센서가 있나
			{
				int chgCntA, chgCntB;
				unsigned long chkTimeA, chkTimeB;
				_oldSen = _newSen;				// 센서상태 저장
				if (_chgSen & BIT_SNS_BBNENCA)
				{
					_TotalCntA++;
					setTO(_TimeOutA[_chgSaveA]);
					_chgSaveA = (_chgSaveA+1) & _CHG_SAVE_MSK;			// 신호변화 save index 증가
				}
				if (_chgSen & BIT_SNS_BBNENCB)
				{
					_TotalCntB++;
					setTO(_TimeOutB[_chgSaveB]);
					_chgSaveB = (_chgSaveB+1) & _CHG_SAVE_MSK;			// 신호변화 save index 증가
				}
				chgCntA = (unsigned int)(_chgSaveA-_chgProcA) & _CHG_SAVE_MSK;
				chgCntB = (unsigned int)(_chgSaveB-_chgProcB) & _CHG_SAVE_MSK;
				chkTimeA = (unsigned long)(curr_ms_tick - _TimeOutA[_chgProcA]);
				chkTimeB = (unsigned long)(curr_ms_tick - _TimeOutB[_chgProcB]);
				if((( 6 < chgCntA) && ( 6 < chgCntB) && (chkTimeA <= CHECK_TIME_1SEC_TO) && (chkTimeB <= CHECK_TIME_1SEC_TO)) ||	// 순간 센서 변화량이 6보다 크거나
				   ((10 < _TotalCntA) && (10 < _TotalCntB)))																		// 총 변화량이 10보다 큰가
				{
					LogPrintln(" LG] stbyTorqOffCtrl: chgCnt("+String(chgCntA)+","+String(chgCntB)+"), _TotalCnt("+String(_TotalCntA)+","+String(_TotalCntB)+"), _EncCnt(" + String(_EncCnt) + "->" + String(iIsrbbnEncCnt) + ")");
					stbyTorqOffControlStep = 30;
				}
				else
				{
					while ((_chgProcA != _chgSaveA) && ((unsigned long)(curr_ms_tick - _TimeOutA[_chgProcA]) > CHECK_TIME_1SEC_TO))		// 1초보다 큰 센서 변화 기록 삭제
					{
						_chgProcA = (_chgProcA+1) & _CHG_SAVE_MSK;		// 신호변화 proc index 증가
					}
					while ((_chgProcB != _chgSaveB) && ((unsigned long)(curr_ms_tick - _TimeOutB[_chgProcB]) > CHECK_TIME_1SEC_TO))		// 1초보다 큰 센서 변화 기록 삭제
					{
						_chgProcB = (_chgProcB+1) & _CHG_SAVE_MSK;		// 신호변화 proc index 증가
					}
				}
			}
			break;

		//텐션 유지 재개시
		case 30:
			stbyTorqOffControlStep = 12;
			break;

		// ERR | OK 판별
		case 75:
			torqOff();		// 모터 OFF
			bbnMotor.offBldc();

			// OK,ERR판별
			if(ERR_OK == errK)
			{
				stbyTorqOffControlStep = 90;	// END-OK
			}
			else
			{
				stbyTorqOffControlStep = 80;	//NG
			}
			break;

		// Error SET
		case 80:
			stbyTorqOffControl_Flag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			stbyTorqOffControlStep = 0;
	
			msg = STX_DEV_CONT_REQ + MAIN_WIRE_BROKEN_ERR + "%";
			Resp2ApPrintln(msg);
	
			#if (LOG_STBY_TORQ_OFF_ERR)
				LogPrintln(" LG] StbyF WIRE BROKEN/NOT_STABLE ERR " + MAIN_WIRE_BROKEN_ERR);
			#endif
				
			break;
	
		// OK
		case 90:
			stbyTorqOffControl_Flag = 0;	// CLR BLDC Flag
			stbyTorqOffControlStep = 0;		// Clr Step
	
			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] StbyF OK");
			#endif
	
			break;

		default:
			break;
		
	}

}
#else
void stbyTorqOff_Control()
{

	static unsigned long _to;
	static unsigned long _toErr;

	static unsigned short oldstbyTorqOffControlStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;

	static unsigned short _senErrK = ENC_SEN_OK;

	int outTorq;
	
	String msg;
	String logMsg;

#if (LOG_STBY_TORQ_OFF_STEP)
	// Step Change LOG OUT
	if(oldstbyTorqOffControlStep != stbyTorqOffControlStep )
	{
		LogPrintln(" lg] ChgST Stby Torq OFF : " + String(stbyTorqOffControlStep));
		oldstbyTorqOffControlStep = stbyTorqOffControlStep;
	}
#endif		
	switch(stbyTorqOffControlStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;

			//== 먼저 "줄끊어짐 검지"==
			setTO(_to);
			stbyTorqOffControlStep = 1;
			break;

			break;

		// 모터정지 대기
		case 1:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				setTO(_to);
				stbyTorqOffControlStep = 2;
			}
			break;

		//토크모터 기동(최소값+5)
		case 2:
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;

			if(recv_TorqMotor_Flag)
				torqMotor.setValue(reqTorqueMotor);	// 최소값 출력
			else
				torqMotor.setValue(defaultTorqueMotor); // 최소값 출력
			setTO(_to);
			setTO(_toErr);
			stbyTorqOffControlStep = 3;
			break;

		case 3:
			// ERR
			if( checkTO(_toErr, CHECK_TIME_10SEC_TO) )
			{
				//ERR
				errK = ERR_WIRE_BROKEN;		// 10초 경과 줄 끊어짐
				setTO(_to);
				stbyTorqOffControlStep = 75;
			}
			// OK
			else if( checkTO(_to, CHECK_TIME_1SEC_TO) )	//ENC센서 안정후 2초 경과 => OK
			{
				_retryCnt = 0;
				setTO(_to);
				stbyTorqOffControlStep = 11;		// 대기OFF제어 시작
			}
			else if(STBY_OFF_CONT_SEN != _oldSen)
			{
				_oldSen = STBY_OFF_CONT_SEN;
				// TBD, ENC방향 검지(유저가 당겼다 놓았다는 건 방향이 계속 바뀌니까)
				_chgCnt++;
				setTO(_to);
			}
			break;


			
		// 줄 서서히 풀기(0.3마다 4번)
		case 11:
			_retryCnt++;
			if(12 < _retryCnt)
			{
				torqOff();
				setTO(_to);
				stbyTorqOffControlStep = 20;
			}
			else
			{
				// 1.2 SEC간 토크를 서서히 줄임!
				if(recv_TorqMotor_Flag)
					torqMotor.setValue( reqTorqueMotor + 0 - (_retryCnt*2) );					// 낚시줄 텐센 제거
				else
					torqMotor.setValue( defaultTorqueMotor + 0 - (_retryCnt*2) );					// 낚시줄 텐센 제거
				setTO(_to);
				stbyTorqOffControlStep = 12;
			}
			break;

			// 0.3초 마다 * 4번 반복
		case 12:
			if(checkTO(_to, 100))
			{
				setTO(_to);
				stbyTorqOffControlStep = 11;
			}
			break;


		// OFF제어 개시  & 반복
		case 20:
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장

			setTO(_to);
			setTO(_toErr);
			stbyTorqOffControlStep = 21;
			break;
			
		// OFF시, ENC정지 대기 = 소비전력 SAVE
		case 21:
			// 20초간 ENC정지 안함
			if(checkTO(_toErr, CHECK_TIME_20SEC_TO))
			{
				// Error : ???
				#if (LOG_STBY_TORQ_OFF_ERR)
					LogPrintln(" lg] ErrSB Error Stby Torq OFF, not Stable ENC ");
				#endif

				errK = ERR_ENC_NOT_STABLE;
				// 상태 변경
				setTO(_to);
				stbyTorqOffControlStep = 75;	// TBD
			}
			else
			{
				// 센서 변화 없이,  3초 경과
				//if(checkTO(_to, ENC_STOP_WAIT_TO))		// 3초
				if(checkTO(_to, ENC_STOP_WAIT_TO))		// 3초
				{
					// 변수 CLR
					_chgCnt = 0;
					_oldSen = STBY_OFF_CONT_SEN;
					// 상태 변경
					setTO(_to);
					stbyTorqOffControlStep = 22;
				}
				// 센서 변화 유?
				else if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
				{
					_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
					_chgCnt++;		// 신호변화 cnt
					setTO(_to);		// 변화시 TO CLR
				}
			}
			break;
			
		//엔코더 변화 check (at OFF상태)
		case 22:
			// 센서변화 CHECK
			if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
			{
				_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}

			// 1초내, 4회 이상 변화
			if( (4 < _chgCnt) && checkTO(_to, CHECK_TIME_1SEC_TO))
			{
				setTO(_to);
				stbyTorqOffControlStep = 30;
			}
			break;

		//텐션 유지 개시
		case 30:
			_chgCnt = 0;
			if(recv_TorqMotor_Flag)
			{
				torqMotor.setValue(reqTorqueMotor);
				outTorq =reqTorqueMotor;
			}
			else
			{
				torqMotor.setValue(defaultTorqueMotor);
				outTorq = defaultTorqueMotor;
			}
			//Resp2ApPrintln(STX_CMD_TORQUE_MOTOR + fillZero2String3Char(defaultTorqueMotor) + "%");	// FW자체출력 응답
			send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);
		
			setTO(_to);
			stbyTorqOffControlStep = 31;
			break;

			
		// ON시, ENC정지 대기 = 텐션유지
		case 31:
			// 센서 변화 없이,  7초 경과
			if(checkTO(_to, TENSION_HOLD_TIME_TO))
			{
				// 변수 CLR
				_chgCnt = 0;
				_oldSen = STBY_OFF_CONT_SEN;
				// 상태 변경
				setTO(_to);
				stbyTorqOffControlStep = 32;
			}
			// 센서 변화 유?
			else if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
			{
				_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
				setTO(_to);		// 변화시 TO CLR
			}
			break;

		case 32:
			//torqOff();		// 모터 OFF
			bbnMotor.offBldc();
		
			//stbyTorqOffControlStep = 20; 	// 반복
			_retryCnt = 0;
			stbyTorqOffControlStep = 11; 	// 반복 (서서히 풀기)
			break;

		//
		case 40: 
			break;

		// NOT_USE
		case 60:
			break;

		// ERR | OK 판별
		case 75:
			torqOff();		// 모터 OFF
			bbnMotor.offBldc();

			// OK,ERR판별
			if(ERR_OK == errK)
			{
				stbyTorqOffControlStep = 90;	// END-OK
			}
			else
			{
				stbyTorqOffControlStep = 80;	//NG
			}
			break;

		// Error SET
		case 80:
			stbyTorqOffControl_Flag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			stbyTorqOffControlStep = 0;
	
			// Err MSG Send
			if(ERR_WIRE_BROKEN == errK)
			{
				msg = STX_DEV_CONT_REQ + MAIN_WIRE_BROKEN_ERR;
				logMsg = MAIN_WIRE_BROKEN_ERR;
			}
			else
			{
				msg = STX_DEV_CONT_REQ + MAIN_WIRE_NOTSTABLE_ERR;
				logMsg = MAIN_WIRE_NOTSTABLE_ERR;
			}
			msg += "%";
			Resp2ApPrintln(msg);
	
			#if (LOG_STBY_TORQ_OFF_ERR)
				LogPrintln(" LG] StbyF WIRE BROKEN/NOT_STABLE ERR " + logMsg);
			#endif
				
			break;
	
		// OK
		case 90:
			stbyTorqOffControl_Flag = 0;	// CLR BLDC Flag
			stbyTorqOffControlStep = 0;		// Clr Step
	
			#if (LOG_DEV_CHECK_OK)
				LogPrintln(" LG] StbyF OK");
			#endif
	
			break;

		default:
			break;
		
	}

}
#endif

//-------------------------------------------------------------------------
typedef struct _stAgingPara
{
	unsigned short cw_duty1;
	unsigned short bldc_duty1;		// BLDC CW
	unsigned long cw_time1;
	unsigned short cw_duty2;
	unsigned short bldc_duty2;	
	unsigned long cw_time2;
} stAgingPara;
typedef stAgingPara* ptrstAgingPara;

stAgingPara wireAging_Mmot =
{
	100, 20, 2000,			// duty1, time1
	 70,  0, 1000,			// duty2, time2
};


//---------- WIRE AGING TEST------------------------
String strData[10] =
{
	"",
};

//------- WIRE AGING TEST -----------------------------
// 설정값 저장 & START
void wireAging_Test_Start(String msg)
{
	unsigned short length;

	unsigned short i = 0;

	// == 1) PARA SET
	//ptrstAgingPara ptrSt = wireAging;
	length = splitTextToStrings(msg.substring(7), strData);

	for(i=0; i<length; i++)
	{
		if(0==i) wireAging_Mmot.cw_duty1 = strData[i].toInt();
		if(1==i) wireAging_Mmot.bldc_duty1 = strData[i].toInt();
		if(2==i) wireAging_Mmot.cw_time1 = strData[i].toInt();
		if(3==i) wireAging_Mmot.cw_duty2 = strData[i].toInt();
		if(4==i) wireAging_Mmot.bldc_duty2 = strData[i].toInt();
		if(5==i) wireAging_Mmot.cw_time2 = strData[i].toInt();
	}

	#if (LOG_WIRE_AGING_TEST)
		LogPrintln(" lg] WireA Val: " + String(wireAging_Mmot.cw_duty1) + "," + String(wireAging_Mmot.bldc_duty1) + ","+ String(wireAging_Mmot.cw_time1)+ ","
									  + String(wireAging_Mmot.cw_duty2) + "," + String(wireAging_Mmot.bldc_duty2) + ","+ String(wireAging_Mmot.cw_time2) );
	#endif

	// == 2) START
	wireAgingTest_Flag = 1;
	wireAgingTest_Step = 10;
	
}

unsigned short splitTextToStrings(String msg, String *s)
{
	unsigned short  idx = 0;
	int startIdx = 0;
	int commaIdx = msg.indexOf(',');

	while (commaIdx != -1) {
		s[idx++] = msg.substring(startIdx, commaIdx);
		startIdx = commaIdx + 1;
		commaIdx = msg.indexOf(',', startIdx);
	}

	// 마지막 토큰
	s[idx++] = msg.substring(startIdx);

	return idx;
}

// 정지
unsigned long agingPulseWidth = 9999;

void wireAging_Test_Stop(String msg)
{
	wireAgingTest_Flag = 0;
	wireAgingTest_Step = 0;
	torqOff();
}

// 동작 제어(반복)
void wireAging_Test_Control()
{
			static unsigned long _to;
		
			static unsigned long _toSenChg;
		
			static unsigned short oldwireAgingTest_Step;
			static short errK;
		
			static int _out;
			static unsigned short _oldSen;
		
			static unsigned short _retryCnt;
			static unsigned short _chgCnt;
	
			static unsigned short speed_high;	// 속도OVER 만족 FLAG
	
			static int _deltaOut;
			
			short result;
			
			String msg;
		
		
#if (LOG_WIRE_AGING_STEP)
			// Step Change LOG OUT
			if(oldwireAgingTest_Step != wireAgingTest_Step )
			{
				LogPrintln(" lg] WireA : " + String(wireAgingTest_Step));
				oldwireAgingTest_Step = wireAgingTest_Step;
			}
#endif
			
			switch(wireAgingTest_Step)
			{
		
				//IDLE
				case 0:
					break;
				
				// === START
				case 10:
					torqOff();					// 낚시줄 텐센 제거
					bbnMotor.offBldc();
	
					_retryCnt = 0;
					_chgCnt = 0;
					//_toSenChg = 0;
					//speed_high = 0;
					errK = ERR_OK;
					//_out = (70+1);		// 최소값 70보다 크게.
					///_deltaOut = 0;
	
					setTO(_to);
					wireAgingTest_Step = 11;
					break;
				
				// 정지 50ms 대기
				case 11:
					if(checkTO(_to, 50))	//WAIT 50 ms TO
					{
						setTO(_to);
						wireAgingTest_Step = 20;
					}
					break;
	
	
				// === 반복 회수 체크
				case 20:
					_retryCnt++;		// 반복회수 증가
					// 동작 개시
					{
						// 반복 1 ]  BLDC CCW로 돌려 줄을 풀어줌
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt = 0;
						torqMotor.setValue(wireAging_Mmot.cw_duty1);	// 줄 감기1(세게)
						bbnMotor.onBldc(BBN_MOT_CW, wireAging_Mmot.bldc_duty1);	// 줄 감기1(세게)
						setTO(_to);
						wireAgingTest_Step = 21;
					}
					break;
					
				case 21:
					if( checkTO(_to, wireAging_Mmot.cw_time1) ) //WAIT 500 ms TO
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt = 0;
						torqMotor.setValue(wireAging_Mmot.cw_duty2);	// 줄 감기2 (약하게)
						bbnMotor.onBldc(BBN_MOT_CW, wireAging_Mmot.bldc_duty2);	// 줄 감기1(세게)
						setTO(_to);
						wireAgingTest_Step = 22;	//
					}
					// 안도는 경우
				#if (WIRE_TEST_DESK)
					else if( (2 > _chgCnt) && checkTO(_to,(10*1000)) )	// 1st Change times
				#else
					else if( (2 > _chgCnt) && checkTO(_to,(wireAging_Mmot.cw_time1 - 50)) )	// 1st Change times
				#endif
					{
						// ERR
						errK = ERR_NG;
						setTO(_to);
						wireAgingTest_Step = 75;	//		
					}
					// 너무 빨리 도는 경우(줄 끊어짐) = 검지 어려움
					//{
					//}
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
					
				case 22:
					if(checkTO(_to, wireAging_Mmot.cw_time2) ) //WAIT 500 ms TO
					{
						setTO(_to);
						wireAgingTest_Step = 23;	// 
					}
					// 안도는 경우
				#if (WIRE_TEST_DESK)
					else if( (2 > _chgCnt) && checkTO(_to,(10*1000)) )	// 1st Change times
				#else
					else if( (2 > _chgCnt) && checkTO(_to,(wireAging_Mmot.cw_time2-50)) )	// 1st Change times
				#endif
					{
						// ERR
						errK = ERR_NG;
						setTO(_to);
						wireAgingTest_Step = 75;	//
					}
					// 너무 빨리 도는 경우(줄 끊어짐) = 검지 어려움
					//{
					//}
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
					
				case 23:
					// 에이징 카운트 up통지
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += "9+1";
					msg += "%";
					Resp2ApPrintln(msg);

					setTO(_to);
					wireAgingTest_Step = 20;	// REPEAT
					break;


				// === NOT USE ===
				
				// === CW, 메인모터 출력, 펄스주기 측정
				case 30:
					// 반복시 변수 CLR
					_chgCnt = 0;
					_oldSen = LVL01_BBNENCA;
					pulseWidthMs = 9999;				
				//
					// == 펄스측정 START SET ???
				//
					torqMotor.setValue(_out);	//DUTY = 70 START
					setTO(_to);
					wireAgingTest_Step = 31;
					break;
	
				case 31:
					// 회전해서 4번 이후(Pulse2번)
					if(6 < _chgCnt)
					{
						//펄스 HIHG때 펄스주기 저장!!
						portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
							pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
						portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
					}
	
					// 판단 3가지
					//if( !speed_high && (4 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs)) // (속도빠름) = SPEC 10.5 ms
					//if( !speed_high && (6 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs)) // (속도빠름) = SPEC 10.5 ms
					if( !speed_high && (6 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs) )	// (속도빠름) = SPEC 10.5 ms
					{
						speed_high = 1;
						// OVER SPEED = OK
						_out--; 		// 감소는 -1 씩
						setTO(_to);
						wireAgingTest_Step = 32;			
	
					}
	
					else if( (speed_high) && PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2S) )
					{
						//if(speed_high)
						{
							speed_high = 0;
							//_out++;
							if( 59 < _out && 81 > _out) // 60 ~ 80
							{
								errK = ERR_OK;		//성공
								// 설정값 저장, _out
							}
							else
							{
								errK = ERR_RANGE_OVER; //RANGE OVER?
							}
							setTO(_to);
							wireAgingTest_Step = 75;					
						}

					}
	
					//펄스OVER 혹은 T/O 체크
					//if( ((BBN_2REV_SIGCHG_CNT < _chgCnt) || checkTO(_to, MAINMOT_MIN_AUTOSET_TO) ) && (HIGH == LVL01_BBNENCA) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
					else if( !PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
					{
						// 회전을 못함 = 반복안함
						if(6 > _chgCnt) //	센서 변화가 없음
						{
							errK= ERR_FAIL;
							setTO(_to);
							wireAgingTest_Step = 75;
						}
						else
						{
							// 정지 & 반복
							_deltaOut = ( (pulseWidthMs - MAINENCA_SPEC_PERIOD_SP_MID)*10/15 );
							if(0 == _deltaOut)
							{
								if(MAINENCA_SPEC_PERIOD_SP_MID < lastPulseWidth)	// SP_LOW
									_deltaOut = 1;
								else
									_deltaOut = -1;
							}
							_out += _deltaOut;

							setTO(_to);
							wireAgingTest_Step = 32;					
						}
					}
	
			
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
	
	
				// === NOT USE
				case 50:
					break;
	
	
				// === OK,ERR 판별
				case 75:
					// 모터 OFF
					agingPulseWidth = pulseWidthMs;
					torqOff();
					bbnMotor.offBldc();
				//
					// 펄스측정 정지.
				//
				
					if(ERR_OK == errK)
					{
						wireAgingTest_Step = 90; //OK
					}
					else
					{
						//setTO(_to);
						wireAgingTest_Step = 80; //NG
					}
					#if (LOG_WIRE_AGING_TEST)
						LogPrintln(" lg] WireA last Pulse Width:" + String(agingPulseWidth));
					#endif
					break;
					
				// Error SET
				case 80:
					wireAgingTest_Flag = 0;	//에러 => DEVICE CHK종료
					wireAgingTest_Step = 0;
					//devCheckFlagStep_AllClear();
	
					// 1) 실패 응답
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += "1" + String(errK);		// Act=1
					msg += "%";
					Resp2ApPrintln(msg);
					
					#if (LOG_WIRE_AGING_TEST_ERR)
						LogPrintln(" LG] WireA *** Not ROTATION NG");
					#endif
					
					break;

				// OK
				case 90:
	
					wireAgingTest_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
					wireAgingTest_Step = 0;	// Clr Step
	
					// 1) 성공 응답
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += STR_ERR_OK;
					msg += "%";
					Resp2ApPrintln(msg);
	
					#if (LOG_WIRE_AGING_TEST)
						LogPrintln(" LG] SetRQ Mmot MINSET REQ: " + String(defaultTorqueMotor));
					#endif
			
					break;
				
				default:
					break;
					
	
			}

}
//-------------------------------------------------------


//--------------------------------------------
// 보드 LED_2 Control
//	AP-O + ROD -O : 0.5sec
//	AP-O + ROD -X : 2 sec
//	AP-X + ROD-O : ON
//	AP-X + ROD-X : OFF
//	CALL 500ms
//--------------------------------------------
volatile short bdLed2_out = 0;
//--------------------------------------------
void boardLed_Control()
{

//#if(!IO_LM_MOT)

	// 보드 LED Control
	if(DIAG_MODE != mainMode)
	{
		if(PROG_END == gameStatus)		// NOT AP EXEC = LED3 OFF
		{
			bdLed2_out = LOW;
			digitalWrite(BD_LED2_PIN, bdLed2_out); 	// LED3 OFF
		}
		else
		{

		#if (AP_MOT_CONT)
			//if (ROD_CONN == rod_conn_status && CONNECT == imu_conn_status)		  // ROD & IMU Conected
			if (CONNECT == ap_conn_status)	  // ROD & IMU Conected
			{
				if(ROD_CONN == rod_conn_status)
				{
					// 0.5 SEC
					bdLed2_out = !bdLed2_out;
					digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 500ms
				}
				else
				{
					// 2.5 SEC
					if(!(count_500ms % 5))	// 2.5 SEC
					{
						bdLed2_out = !bdLed2_out;
						digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 2 SEC
					}
				}
			}
			// AP NOT CONN
			else
			{
				if(ROD_CONN == rod_conn_status)
				{
					bdLed2_out = HIGH;
					digitalWrite(BD_LED2_PIN, bdLed2_out); 	// LED 3 ON
				}
				else
				{
					bdLed2_out = LOW;
					digitalWrite(BD_LED2_PIN, bdLed2_out);		// LED 3 OFF
				}
			}
		#else
			if (isGameEnable)		  // if Main Enable = 500ms Blinking
			{
				bdLed2_out = !bdLed2_out;
				digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 500ms
			}
			else		
			{
				if(!(count_500ms % 5))
				{
					bdLed2_out = !bdLed2_out;
					digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 2.5 SEC
				}
			}
		#endif
		}
	}
//#endif

}


//-----------------------------------------------------------------
//  READ INPUT
//	1ms 정도가 필요한 센서만 1ms READ
//	Encoder, FG
//
void read_input_1ms()
{
	// 아래를 센서 Map개수 만큼 For문 돌릴 것. 현재는  Sensor Map 1개(16bit)만 있음!
	{
	// 1) Input Read
	
	sensor1ms[SENSOR_1].curr =  (digitalRead(BBN_ENC_B_DT_PIN) << SNS_bbnEncB) |
								(digitalRead(BBN_ENC_A_CK_PIN) << SNS_bbnEncA) |
								(digitalRead(BLDC_FG_PIN) << SNS_bbnMotFg);
	//sensor1ms[SENSOR_1].curr |= ((digitalRead(PWROFF_SWC_PIN)) << SNS_PwrOffSwc);	// Low Active
	//sensor1ms[SENSOR_1].curr |= (digitalRead(PC_USB5V_PIN) << SNS_usb5V);
	//sensor1ms[SENSOR_1].curr |= ((!digitalRead(BOOT_MODE_PIN)) << SNS_boot);		// Low Active

#if (IO_LM_MOT)	
	//-- LM-JIG
	sensor1ms[SENSOR_1].curr |= (digitalRead(LM_HOME_SEN_PIN) << SNS_LM_HOME) |	// LOW
								(digitalRead(LM_LEFT_SEN_PIN) << SNS_LM_LEFT) |	// LOW
								(digitalRead(LM_RIGHT_SEN_PIN) << SNS_LM_RIGHT) |	//LOW
								(digitalRead(LM_ENC_SEN_PIN) << SNS_LM_ENC) |
								(digitalRead(LM_MOT_FG_PIN) << SNS_LMMOT_FG);
#endif
	// 2) 연산
	sensor1ms[SENSOR_1].lvl = ( (sensor1ms[SENSOR_1].lvl & sensor1ms[SENSOR_1].old) | (sensor1ms[SENSOR_1].curr & (sensor1ms[SENSOR_1].lvl ^ sensor1ms[SENSOR_1].old)));		// (V108)
	// 3) LE/TE검출
	sensor1ms[SENSOR_1].le = 0;
	sensor1ms[SENSOR_1].te = 0;
	
	sensor1ms[SENSOR_1].le = ((~sensor1ms[SENSOR_1].old2) & sensor1ms[SENSOR_1].old & sensor1ms[SENSOR_1].curr);
	sensor1ms[SENSOR_1].te = ( sensor1ms[SENSOR_1].old2 & (~sensor1ms[SENSOR_1].old) & (~sensor1ms[SENSOR_1].curr) );
	}

#if LOG_LE_TE_1MS
	if(sensor1ms[SENSOR_1].le || sensor1ms[SENSOR_1].te)
	{
		LogPrintTime;
		LogPrint(" LG] RD1MS LE: ");
		LogPrintf("%2X", sensor1ms[SENSOR_1].le);
		LogPrint(" ,TE: ");
		LogPrintf("%2X", sensor1ms[SENSOR_1].te);
		LogPrint(" ,LVL: ");
		LogPrintf("%2X", sensor1ms[SENSOR_1].lvl);
		//LogPrintTimeln();
		//LogPrintln(" LG] RD01M DATA old2/old/curr " + String(sensor1ms[SENSOR_1].old2) + ", " + String(sensor1ms[SENSOR_1].old) + ", " + String(sensor1ms[SENSOR_1].curr));
	}
#endif
	// save data
	sensor1ms[SENSOR_1].old2 = sensor1ms[SENSOR_1].old;
	sensor1ms[SENSOR_1].old  = sensor1ms[SENSOR_1].curr;

}

//----------------------------------------------------------------------------
//  10ms마다 센서확정level, LE/TE를 검출
//	Diag IN Test : LVL_10MS
//

//#define FREQ_CHANGE_BIT_MASK 0xFF1F		// BbnEncA+B+BbnFG
#define FREQ_CHANGE_BIT_MASK 0x5F1F	 //0xFF1F		// BbnEncA+B+BbnFG

int lc10msCnt = 0;
//
//
//
void read_input_10ms()
{
	// 아래를 센서 Map개수 만큼 For문 돌릴 것. 현재는  Sensor Map 1개(16bit)만 있음!
	{
	// 1) Input Read
	sensor10ms[SENSOR_1].curr  = sensor1ms[SENSOR_1].lvl |
								(digitalRead(PWROFF_SWC_PIN) << SNS_PwrOffSwc) |	// Low Active
								(digitalRead(PC_USB5V_PIN) << SNS_usb5V) |
								(digitalRead(BOOT_MODE_PIN) << SNS_boot) |			// Low Active
								(digitalRead(SUB_ACOFF_PIN) << SNS_AcOff);			// Low Active(V108)

#if (IO_LM_MOT)	
	//-- LM-JIG
	sensor10ms[SENSOR_1].curr |= (digitalRead(LM_HOME_SEN_PIN) << SNS_LM_HOME) |		// LOW
								(digitalRead(LM_LEFT_SEN_PIN) << SNS_LM_LEFT) |		// LOW
								(digitalRead(LM_RIGHT_SEN_PIN) << SNS_LM_RIGHT) |	// LOW
								(digitalRead(LM_ENC_SEN_PIN) << SNS_LM_ENC) |
								(digitalRead(LM_MOT_FG_PIN) << SNS_LMMOT_FG);
#endif

	// 2) 연산
	sensor10ms[SENSOR_1].lvl = ( (sensor10ms[SENSOR_1].lvl & sensor10ms[SENSOR_1].old) | (sensor10ms[SENSOR_1].curr & (sensor10ms[SENSOR_1].lvl ^ sensor10ms[SENSOR_1].old)));		// (V108)

	// 3) LE/TE검출
	sensor10ms[SENSOR_1].le = 0;
	sensor10ms[SENSOR_1].te = 0;
	
	sensor10ms[SENSOR_1].le = ( (~sensor10ms[SENSOR_1].old2) & sensor10ms[SENSOR_1].old & sensor10ms[SENSOR_1].curr );
	sensor10ms[SENSOR_1].te = ( sensor10ms[SENSOR_1].old2 & (~sensor10ms[SENSOR_1].old) & (~sensor10ms[SENSOR_1].curr) );
	}

#if LOG_LE_TE_10MS
	//if(sensor10ms[SENSOR_1].le || sensor10ms[SENSOR_1].te)
	if((sensor10ms[SENSOR_1].le & FREQ_CHANGE_BIT_MASK) || (sensor10ms[SENSOR_1].te & FREQ_CHANGE_BIT_MASK) )		// withput Bbn_Enc & Bbn_FG
	{
		LogPrintTime;
		LogPrint(" LG] RD10M LE: ");
		LogPrintf("%2X", sensor10ms[SENSOR_1].le);
		LogPrint(" ,TE: ");
		LogPrintf("%2X", sensor10ms[SENSOR_1].te);
		LogPrint(" ,LVL: ");
		LogPrintf("%2X", sensor10ms[SENSOR_1].lvl);
		//LogPrintTimeln();
		//LogPrintln(" LG] RD10M DATA lvl/old2/old/curr "  + String(sensor10ms[SENSOR_1].lvl) + ", " + String(sensor10ms[SENSOR_1].old2) + ", " + String(sensor10ms[SENSOR_1].old) + ", " + String(sensor10ms[SENSOR_1].curr));
	}
#endif

	// save data
	sensor10ms[SENSOR_1].old2 = sensor10ms[SENSOR_1].old;
	sensor10ms[SENSOR_1].old  = sensor10ms[SENSOR_1].curr;

#if LOG_READ_IN_10MS
	lc10msCnt++;
	if(50000 < lc10msCnt) { lc10msCnt=0; }
	if(0 == (lc10msCnt%1000))
	{
		LogPrintln(" LG] SYS_T 10ms Count: " + String(lc10msCnt));
	}
#endif


}

void read_input_100ms()
{
	// TBD
}



//===========================================================
//		SETUP Part
//
//===========================================================

#define eepromLogPrintln	Resp2ApPrintln
//#define eepromLogPrintln	LogPrintln
//-----------------------------------------------------
//	eeprom Test
//-----------------------------------------------------
void eeprom_test()
{

	unsigned int reqlen = 6;
	/*
		int getTo = Wire.getTimeOut();
		eepromLogPrintln (" LG] NvmBF TO default:" + String(getTo) + " /ms");
		Wire.setTimeOut(50);		// default 1000ms => 50 ms
		getTo = eNvm.getTimeOut();
		eepromLogPrintln (" LG] NvmBF TO set:" + String(getTo) + " /ms");
	*/
		unsigned int len;
	
		byte readTempBuff[128];
		byte writeTempBuff[128];
	
		// 1) READ Offset:0 addr, 6 Length
		eNvm.readSend(0, reqlen);
		delay(100);
	
		len = eNvm.read(&readTempBuff[0], 0, reqlen);
	#if LOG_I2C
		LogPrintln(" LG] I2C__ Read Len: " + String(len));
	#endif
		eNvm.grazingGeddong(&readTempBuff[0], 0, reqlen);
		delay(100);
	
		// 2) WRITE READ Val + 1,  6 Length
		writeTempBuff[0]=readTempBuff[0]+1;   
		writeTempBuff[1]=readTempBuff[1]+1;   
		writeTempBuff[2]=readTempBuff[2]+1;
		writeTempBuff[3]=readTempBuff[3]+1;
		writeTempBuff[4]=0x89;
		eNvm.write(&writeTempBuff[0], 0, 5);
		delay(100);

		// 3) reREAD Data, 6 Length
		eNvm.readSend(0, reqlen);
		delay(100);
		len = eNvm.read(&readTempBuff[0], 0, reqlen);
	#if LOG_I2C
		LogPrintln(" LG] I2C__ Write&Read Len: " + String(len));
	#endif
		eNvm.grazingGeddong(&readTempBuff[0], 0, reqlen);
		
	/*	
		rdLen = eNvm.rReadSend(10, 3);
		eepromLogPrintln(" LG] I2C__ ReadSend OK: " + String(rdLen));
		delay(100);
		
		bfLen = Wire.available();
		if(bfLen)
		{
			eNvm.buffSave(10,bfLen);		// addr=0, lenth = 10 byte
			eNvm.grazingGeddong(10, bfLen);
		}
		else
		{
			eepromLogPrintln(" LG] I2C__ RxBuffer Len: " + String(bfLen));
		}
	
	
	//	1) Buffer Write
		wrtLen = 20;
		int i=0;
		for(i=0; i<wrtLen; i++)
		{
			if(i%2) { eNvm.buff[i] = 0x55+i; }
			else	{ eNvm.buff[i] = 0xAA+i; }
		}
	
	
		bfLen = eNvm.write(0,wrtLen);
	
		rdLen = eNvm.currentRead(bfLen);
		delay(50);
		bfLen = Wire.available();
		if(bfLen)
		{
			eNvm.read(0,bfLen);   // addr=0, lenth = 10 byte
			eNvm.grazingGeddong(0, bfLen);
		}
		else
		{
			eepromLogPrintln(" LG] I2C__ RxBuffer bgLen: " + String(bfLen) +", wrtLen: "+String(wrtLen));
		}
	
		Wire.beginTransmission(0x50);
		Wire.write(0x55);
		Wire.endTransmission();
	
		eNvm.read(0,2);
		eNvm.grazingGeddong(0, 2);
	
		//eepromLogPrintln(" LG] I2C__ RxBuffer bgLen: " + String(bfLen) +", wrtLen: "+String(wrtLen));
	*/
	

}

//-----------------------------------------------------------
// PowerOn SUB_AC ON/OFF, by PWR_SW_STS
//-----------------------------------------------------------
void subAc_Control_atReset()
{
	LogPrintln(" LG] PWRAC SUB_AC On");
	
	if(LVL10_PWROFF_SWC)		// POWER SWC OFF?
	{
		//subAC_Off();

	// 24.11.7 Delete AC OFF at PWR_SWC_OFF
		//subAcOff_WithOtherOff();
	}
	else
	{
		subAC_On();
		motor_AllOff();		// MAIN ALL OFF
		ledOn_MbBtm();
		reelOut_AllOff();	// REEL ALL OFF
	}
}

//--- CONFIG Define
#define CONFIG_AD_MAX	4096	// ADC 12 BIT
#define CONFIG_AD_EA	2 // 8		// Dipsw 3개 = 8개 조합
#define CONFIG_AD_DIVS  (CONFIG_AD_MAX/CONFIG_AD_EA)
#define CONFIG_AD_GAP	(CONFIG_AD_DIVS/2)

#define CONF_READ_CNT	5
//-------------------------------------------------------------
//		CONFIG (Normal / LM-JIG) SET
//-------------------------------------------------------------
short setConfig()
{
	short i;
	short conf = 0;
	short rts;
	short confAvg = -1;
	int dummy;
	int confAd[CONF_READ_CNT];

	//==[ 1 ] INPUT - CONFIG [ ]보드 타입: 노멀(0) /LM보드(1) 구분] ,  only POWER ON
	pinMode(DF_CONFIG_PIN, INPUT_PULLDOWN);	// INPUT Check

// GPIO입력 (LOW/HIGH)
#if(IO_CONF_DIGITAL)
	for(i=0; i<CONF_READ_CNT; i++)
	{
		conf |= ((digitalRead(DF_CONFIG_PIN)? 1:0 ) << i);
		delay(10);
	}
// ADC입력
#else	// ADC
	//delay(100);	//OK
	delay(50);
	//delay(30);	//OK
	//delay(10);
	//delay(1);	// NG
	// Dummy, 3번
	for(i=0; i<CONF_READ_CNT; i++)
	{
		dummy= analogRead(DF_CONFIG_PIN);
		delay(10);
	}
	// READ
	for(i=0; i<CONF_READ_CNT; i++)
	{
		confAd[i] = analogRead(DF_CONFIG_PIN);
		conf += confAd[i];
		delay(10);
	}
	confAvg = conf/CONF_READ_CNT;
#endif

	//== [ 2 ] OUT : UART_RXD or LM_MOT_24VON
	#if (IO_LM_MOT)
		//pinMode(DF_CONFIG_PIN, OUTPUT);	// GPIO-1, OUT SET
		pinMode(LM_24VON_PIN, OUTPUT);		// GPIO-1, OUT SET
		digitalWrite(LM_24VON_PIN, LM_24VOFF);	// LM BLDC_24V_ON OFF
	#endif
	
	//== [ 3 ] CONFIG 판단
#if(IO_CONF_DIGITAL)
	if(0x1F == conf)
	{
		rts = DF_CONFIG_LMJIG;
	}
	else
	{
		rts = DF_CONFIG_STAND_ALONE;
	}
#else	// ADC
	//curr_ms_tick = millis();
	if(0) {}
	else if( (CONFIG_AD_DIVS*(CONFIG_AD_EA-1)) + CONFIG_AD_GAP < conf) { rts = DF_CONFIG_LMJIG; }
	//else if( conf) { }
	//else if( conf) { }
	//else if( 0 + CONFIG_AD_GAP < conf) { rts = DF_CONFIG_STAND_ALONE; }
	else			{ rts = DF_CONFIG_STAND_ALONE; }	
#endif

	#if(LOG_CONFIG)
		LogPrintln(" lg] Config:" + String(rts) +"," + String(conf) +"," + String(confAvg));
	#endif
	return rts;
}

//-----------------------------------------------------------------
//	PORT(PIN) SETTING
//-----------------------------------------------------------------
void ioLedOffSetting()
{
//
#if IO_DLED
	// 중앙 LED
	pinMode(LED_CENT_R_PIN, OUTPUT);
	pinMode(LED_CENT_G_PIN, OUTPUT);
	pinMode(LED_CENT_B_PIN, OUTPUT);

	// 하단 LED
	pinMode(LED_BTM_PIN, OUTPUT);

	extLed.init();
	
	// 옵션 LED
  #if (!IO_LM_MOT)
  	#if (!IO_EXTIO_UART_PIN)		// NOT IO_EXT_UART
		//pinMode(LED_OPT_L_PIN, OUTPUT);	// GPIO-1
		//pinMode(LED_OPT_R_PIN, OUTPUT);	// GPIO-2
  	#endif
  #endif
  
#endif

}

void ioPinSetting()
{

	//=== 1) input
	//pinMode(, INPUT);

	//pinMode(BTMODE_PIN, INPUT);	// BOOT MODE PIN not PULL_UP
	
#if IO_SERVO
	//pinMode(ANG_HOMESNS_PIN, INPUT_PULLUP);			// 예비 - Servo Home Sensor
#endif

#if (IO_LM_MOT)
	pinMode(LM_ENC_SEN_PIN, INPUT);		// IO-47 , WDT
	pinMode(LM_HOME_SEN_PIN, INPUT);
	pinMode(LM_LEFT_SEN_PIN, INPUT);
	pinMode(LM_RIGHT_SEN_PIN, INPUT);
#endif	

	pinMode(PC_USB5V_PIN, INPUT_PULLUP);
	pinMode(BLDC_FG_PIN, INPUT_PULLUP);
	//pinMode(PWROFF_SWC_PIN, INPUT_PULLUP);			// NG, LOW ACTIVE
	//pinMode(PWROFF_SWC_PIN, INPUT_PULLDOWN);		// NG, LOW ACTIVE
	pinMode(PWROFF_SWC_PIN, INPUT);			// LOW ACTIVE

	pinMode(BBN_ENC_A_CK_PIN, INPUT_PULLUP);
	pinMode(BBN_ENC_B_DT_PIN, INPUT_PULLUP);

	//pinMode(ILKDOOR_PIN, INPUT_PULLUP);				// 예비 - Interlock Door
	//pinMode(SEL_BOARD_PIN, INPUT_PULLUP);

	  
	// === 2) output
	//pinMode(, OUTPUT);

#if (!IO_LM_MOT)
	// 0) - Power Control & WDT SET

	// TBD 	Boot(Download)시 WDT출력 안됨. Reeset됨!!!
	//	WDT

	// 보드타입(IN)
	#if (DEF_NOT_USE)
		pinMode(BD_TYPE_PIN, INPUT_PULLUP);		// TBD,  HIGH-V10(구보드),  LOW-V11(신보드-TBD)=외부에 풀다운 저항 추가 필요.
	#endif
		
	//#if (DEF_NOT_USE)
	#if (1)			// PWR ON시만 1번 토글 출력함.
		// WDT(OUT)
		pinMode(WDT_OUT_PIN, OUTPUT);
		digitalWrite(WDT_OUT_PIN, HIGH);	// 
		digitalWrite(WDT_OUT_PIN, LOW); 	// 
		digitalWrite(WDT_OUT_PIN, HIGH);	//	미사용포트, 출력, HIGH유지
	#endif
#endif
	

	//	보드 LED
	pinMode(BD_LED2_PIN, OUTPUT);	  // SET Pin OUT
	
#if(IO_LM_MOT)
	pinMode(LM_MOT_ON_PIN, OUTPUT);		// IO-40
	pinMode(LM_MOT_DIR_PIN, OUTPUT);	// IO_41
	
	pinMode(LM_24VON_PIN, OUTPUT);		// GPIO-1
	digitalWrite(LM_24VON_PIN, LM_24VOFF);		  // LM BLDC_24V_ON OFF
	
	pinMode(LM_MOT_PWM_PIN, OUTPUT);	// GPIO -2
#else
	pinMode(BD_LED3_PIN, OUTPUT);		// IO-40
	pinMode(BD_LED4_PIN, OUTPUT);		// IO-41
#endif

	// TORQ PORT
	pinMode(TORQ_SS, OUTPUT);				   // PIN SET to SS =OUT

	//pinMode(SERVOPWM_PIN, OUTPUT);

#if IO_BBN
	// BLDC PORT
	pinMode(BBN_MOT_ON_PIN, OUTPUT);		// BLDC_ON_PIN
	pinMode(BBN_MOT_DIR_PIN, OUTPUT);
	pinMode(BBN_MOT_PWM_PIN, OUTPUT);
	
	pinMode(BBN_BLDC_24VON_PIN, OUTPUT);
	digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);	// BBN BLDC_24V_ON OFF
#endif

//
#if IO_DLED
	// 중앙 LED
	pinMode(LED_CENT_R_PIN, OUTPUT);
	pinMode(LED_CENT_G_PIN, OUTPUT);
	pinMode(LED_CENT_B_PIN, OUTPUT);

	// 하단 LED
	pinMode(LED_BTM_PIN, OUTPUT);
	
	// 옵션 LED
  #if (!IO_LM_MOT)
  	#if (!IO_EXTIO_UART_PIN)		// NOT IO_EXT_UART
		//pinMode(LED_OPT_L_PIN, OUTPUT);	// GPIO-1
		//pinMode(LED_OPT_R_PIN, OUTPUT);	// GPIO-2
  	#endif
  #endif
  
#endif

}

//--------------------------------------------------
//	PID제어
//--------------------------------------------------


//--------------------------------------------------
//	PID제어 종료
//--------------------------------------------------

void wat_PidParaSet(String msg)
{
	// 시작전에 설정하므로, Mutex사용안 함.
	int iPara = msg.substring(5,(5+1)).toInt(); // 
	double dPara = msg.substring(6).toDouble();
	switch(iPara)
	{
		case 0: // View
			LogPrintln(" lg] PidPr:" + String(Kp)+","+String(Ki)+","+String(Kd));
			break;
	#if (PID_PARA_SET_ENB)
		case 1: // Kp
			Kp = dPara;
			LogPrintln(" lg] PidPr:" + String(Kp)+","+String(Ki)+","+String(Kd));
			break;
		case 2: // Ki
			Ki = dPara;
			LogPrintln(" lg] PidPr:" + String(Kp)+","+String(Ki)+","+String(Kd));
			break;
		case 3: // Kd
			Kd = dPara;
			LogPrintln(" lg] PidPr:" + String(Kp)+","+String(Ki)+","+String(Kd));
			break;
	#endif
		default:
			break;
	}

}

void wat_HookRandCheckExec()
{
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		bHookRandingCheckOnce = true;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
}

void wat_PidStart_TargetSet(short _targetDist)
{
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		iTargetDist = (_targetDist);	// 목표거리로 설정
		pidExecFlag = 1;
		pidExecOutLog = 1;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
}


void wat_PidStop()
{	
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		pidExecFlag = 0;
		iWireDistance = 0;		// INIT
		sMotorSpeed = 0;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료

	int iOut;
	// 모터 멈춤
	#if (PID_OUT_BLDC)
		bbnMotor.offBldc();
	#endif
	
	#if (PID_OUT_TORQ)
		if(DEFAULT_TORQ > reqTorqueMotor)	{ iOut = DEFAULT_TORQ; }
		else								{ iOut = reqTorqueMotor; }
		torqMotor.on(reqTorqueMotor);
	#endif
}



//------------------------------------------------------------------------------
//	1ms SYSTEM TIMER COUNT, FREE RUN : 0 ~ 0xFFFFFFFF
//------------------------------------------------------------------------------
void IRAM_ATTR ISR_onTimerHandler() {
  // 타이머 인터럽트 내에서 current_time 증가
  //portENTER_CRITICAL();  // 임계 영역 시작
  //lSys1MsTime++;        // current_time 값 증가
  ///portEXIT_CRITICAL();   // 임계 영역 종료

  portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작 (인터럽트 내에서)
  
	lSys100UsTime++;		// 100us FREE RUN
	
	sSys100UsCnt++;			// 100us CNT
	if((10-1) < sSys100UsCnt)
	{
  		sSys100UsCnt = 0;
		
		lSys1MsTime++;         // 1ms SYS TIME : FREE RUN 값 증가
	}
  portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
}


#define INTERVAL_2MS_1MS	(2 * 1)
#define INTERVAL_2MS_100US	(2 * 1000/100)
//-------------------------------------------------------------------------
// PID제어 : ENC TE 인터럽트 서비스 루틴 (ISR)
//
//
//-------------------------------------------------------------------------
void IRAM_ATTR ISR_encIntrHandle()
{
	unsigned long _lSys1MsTimer;
#if 1
	// 현재 1ms 시간을 취득
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		// [1] - 속도 : 인터벌 측정, 
    	// GPIO 인터럽트 내에서 current_time 값 읽기
    	_lSys1MsTimer = lSys1MsTime;
    	#if(SYS_1MS_INTR)
			lCurrentTime = lSys1MsTime;   // 1MS current_time 값 읽기
		#else
			lCurrentTime = lSys100UsTime;	//100US current_time 값 읽기
		#endif
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
#else
	//lCurrentTime = millis();		// ms
	lCurrentTime = micros();		// us
#endif
	//iEncIntrIntervalTime = lCurrentTime - lastInterruptTime;
	//lastInterruptTime = lCurrentTime;  // 마지막 시간을 현재 값으로 갱신

	// [1]인터벌 타임 [MS] 계산
	if (lCurrentTime < lLastTime) {  // currentTime이 lastTime보다 작으면 오버플로우 발생
	  iEncIntrIntervalTime = ((ULONG_MAX - lLastTime) + lCurrentTime + 1);
	} else {
	  iEncIntrIntervalTime = (lCurrentTime - lLastTime);  // 일반적인 시간 차이 계산
	}

//=== 신호에 문제 있음. TBD TBD =========
	//if(2 > iEncIntrIntervalTime)	// 2ms미만은 INT 무시
	if(INTERVAL_2MS_100US > iEncIntrIntervalTime)	// 2ms미만은 INT 무시
	{
		return;		// 0ms, 1ms간격의 INT는 오동작(CPU특성) => INT무시
	}
	// 갱신은 누적해서 처리.
	lLastTime = lCurrentTime;  // 마지막 시간을 갱신

	//-- ENC 방향 판별
	// CW 	(TE & LOW)
	sIsrEncBdata = (digitalRead(BBN_ENC_B_DT_PIN)? 1:0);
	if(!sIsrEncBdata)
	{
		iIsrbbnEncCnt--;
		if(10 > iIsrbbnEncCnt) { iIsrbbnEncCnt = 10; }	// 신호문제 = 잠정 조치
		sIsrBbnEncDir = BBN_ENC_CW;
	}
	// CCW	(TE & HIGH)
	else
	{
		iIsrbbnEncCnt++;
		sIsrBbnEncDir = BBN_ENC_CCW;
	}
	
	// [2] 거리 계산
	//iWireDistance = (mainEnc.bbnCnt*BBN_ENC_1PULSE_DIST)/1000;
	if(0 > iIsrbbnEncCnt) { iIsrbbnEncCnt = 1; }	// 잠정조치
	iWireDistance = (iIsrbbnEncCnt*BBN_ENC_1PULSE_DIST)/1000;

	// [2]-2 : Data기록, 인터벌 타임 기록
	//stEncIntvBuff[sEncBuffIdx].tick = lCurrentTime;
	stEncIntvBuff[sEncBuffIdx].tick = _lSys1MsTimer;			// 1ms
	stEncIntvBuff[sEncBuffIdx].width = iEncIntrIntervalTime;	// 100us
	stEncIntvBuff[sEncBuffIdx].dir = sIsrBbnEncDir;
	stEncIntvBuff[sEncBuffIdx].enc = iIsrbbnEncCnt;
	stEncIntvBuff[sEncBuffIdx].dist = iWireDistance;
	sEncBuffIdx++;
	sEncBuffIdx = (sEncBuffIdx & (ENC_INTV_BUFF_SIZE-1));

	// INT=>TASK처리, 전개(Flag)
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		bIsrEncInterruptFlag = true;  // 인터럽트 발생 시 플래그(LOG출력용)를 true로 설정
		iIsrEncInterruptCnt++;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료



// PID제어 실시 여부
#if (GPIO_INTR_PID_CONT)	// PID CONT

	// [3] PID실시 조건 확인_1
	// 10ms 이하 & [ 후킹 or 랜딩]
	if( (0 == pidExecFlag)
		&& (BBN_ENC_CCW == sIsrBbnEncDir)
		//&& ((PID_EXEC_ENC_MIN_TIME_1MS + 1) > iEncIntrIntervalTime)					// Enc펄스폭이 10ms이하일때
		&& ((PID_EXEC_ENC_MIN_TIME_100US + 1) > iEncIntrIntervalTime)					// Enc펄스폭이 15ms이하일때
		&& ( (GMWAT_HOOK == gmWatStatus) || (GMWAT_RANDING == gmWatStatus) )	// 후킹 혹은 랜딩시
		&& (bHookRandingCheckOnce)
	)
	{
		//iTargetDist = (iWireDistance -100);	// []현재거리 - 10cm ]를 목표거리로 설정
		//iTargetDist = (iWireDistance -50);	// []현재거리 - 10cm ]를 목표거리로 설정
		iTargetDist = (iWireDistance);	// [현재거리 - 10cm ]를 목표거리로 설정
		pidExecFlag = 1;
		pidExecOutLog = 1;
		bHookRandingCheckOnce = false;		// 1회만 검지
	}

	// [4] PID실시 조건 확인_2
	// 거리 4미터 이상 &  [버티기]
	if( (0 == pidExecFlag) 
		&& (BBN_ENC_CCW == sIsrBbnEncDir)
		&& (WRIE_MAX_DIST < iWireDistance)		// 4미터 Over
		&& ( (GMWAT_HOLDON == gmWatStatus) || (GMWAT_FIGHT == gmWatStatus) || (GMWAT_HARDACT == gmWatStatus)  )		// 버티기 /파이팅 때
		//&& ( (GMWAT_SELECT < gmWatStatus) )		// 선택이후, 게임중
		)
	{
		iTargetDist = (WRIE_MAX_DIST - TARGET_SUB_VAL);	// [3.6M-10cm] 값을 목표로 설정
		pidExecFlag = 1;
		pidExecOutLog = 1;
	}

	// [5] PID제어
	if(pidExecFlag)
	{
		if(iTargetDist > iWireDistance)	// 목표도달 = PID정지.
		{
			// 모터 OFF
			#if (PID_OUT_BLDC)
				#if 0
					bbnMotor.offBldc();
				#else
					digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);		// BLDC_24V_ON OFF
					digitalWrite(BBN_MOT_ON_PIN, BBN_MOT_OFF);
					ledcWrite(BOBBIN_CH, BOBBIN_MAX_DUTY);
				#endif
			#endif
			// 모터 OFF
			#if (PID_OUT_TORQ)
				digitalWrite(TORQ_SS, LOW);
				SPI.transfer(B00010001);		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
				SPI.transfer(0);			// VAL Send
				digitalWrite(TORQ_SS, HIGH);
			#endif
			pidExecFlag = 0;
			sMotorSpeed = 0;
		}
		else				// 목표도달 전 = PID제어 필요.
		{
			// PID제어
			  // 목표 거리와 현재 위치의 오차 계산
  			double diff = (iWireDistance - iTargetDist); 		// 현거리 - 목표거리 (우리는 Over거리를 주로 제어하므로)

			// PID 제어: 비례, 적분, 미분 계산
  			dIntegral += diff;  // 누적 오차 (적분)
  			double derivative = diff - dLastDiff;  // 오차 변화량 (미분)
  
  			// PID 출력 계산
  			dIsrOutDuty = (Kp * diff) + (Ki * dIntegral) + (Kd * derivative);

  			// 출력값을 이용한 모터 속도 조절
			#if (PID_OUT_TORQ)
  				sMotorSpeed = constrain(dIsrOutDuty, -255, 255);  // 출력값을 -255에서 255 사이로 제한
  				//sMotorSpeed = map(dIsrOutDuty,-1023, 1023, -255, 255);  // 출력값을 -255에서 255 범위로 매팅
  			#endif
  			#if(PID_OUT_BLDC)
				sMotorSpeed = constrain(dIsrOutDuty, -80, 80);  // 출력값을 -80에서 80 사이로 제한
				//sMotorSpeed = map(dIsrOutDuty,-1023, 1023, -255, 255);  // 출력값을 -255에서 255 범위로 매팅
			#endif
			// 모터 제어
			if (0 < sMotorSpeed)
			{
	  			// CW 회전
	  			#if (PID_OUT_BLDC)
					#if 0
	  					bbnMotor.onBldc(BBN_MOT_CW, sMotorSpeed);
					#else
						digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);		// BLDC_24V_ON ON
						digitalWrite(BBN_MOT_DIR_PIN, BBN_MOT_CW);			// SET DIR
						ledcWrite(BOBBIN_CH, sMotorSpeed);
					#endif
				#endif
			
				#if (PID_OUT_TORQ)
			  		#if 0
						torqMotor.on(sMotorSpeed);
			  		#else
						digitalWrite(TORQ_SS, LOW);
						SPI.transfer(B00010001);  		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
						SPI.transfer(sMotorSpeed);       		// VAL Send
						digitalWrite(TORQ_SS, HIGH);
			  		#endif
				#endif
			}
			//== CCW로 푸는 것은 의미 없음.
			//else if (0 > sMotorSpeed)
			//{
			//	// CCW 회전
		  	//	bbnMotor.onBldc(BBN_MOT_CCW, -sMotorSpeed);
			//} 
			else
			{
			  // 모터 멈춤
				#if (PID_OUT_BLDC)
			  		bbnMotor.offBldc();
				#endif
			
				#if (PID_OUT_TORQ)
				  #if 0
					torqOff();
				  #else
					digitalWrite(TORQ_SS, LOW);
					SPI.transfer(B00010001);		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
					SPI.transfer(0);				// VAL Send
					digitalWrite(TORQ_SS, HIGH);
				  #endif
				#endif
			}
			// PID 제어에서의 이전 오차 업데이트
			dLastDiff = diff;
		}
	}
#endif

}

// _pin : TORQ_SS
void spi_out(int _pin, int _val)
{
	digitalWrite(_pin, LOW);
	SPI.transfer(B00010001);  		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
	SPI.transfer(_val);       		// VAL Send
	digitalWrite(_pin, HIGH);
}

unsigned int system_delay_cnt = 0;
/*------------------------------------------------------------------
   Create System Counter
     1ms * 10 => 10ms
     10ms *10 => 100ms
 Ca;; : Unlimit Loop
------------------------------------------------------------------*/
void system_counter()
{
	unsigned long add_tick = 0;

#if (SYS_TIMER_INTR_ENB)
	// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
	portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
		curr_ms_tick = lSys1MsTime;	   // current_time 값 읽기
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
#else
	// Reset ~ 500ms wait AT POWER ON
	curr_ms_tick = millis(); 	// Read Current Tick
#endif
	
	if(0)
	{
	}
/*
	else if(!isPowerOnReady)				// Once Only
	{
		if(999 < curr_ms_tick)
		{
			isPowerOnReady = true;
			//old_ms_tick = (curr_ms_tick + 0xFFFFF800); // test
			old_ms_tick = curr_ms_tick;
		}
	}
*/	
	//if(isPowerOnReady)
	else
	{
		// 1) count Up check
		//curr_ms_tick = (millis()+ 0xFFFFF800); // test
		if(curr_ms_tick != old_ms_tick)
		{
			if( curr_ms_tick > old_ms_tick )
			{
				add_tick = curr_ms_tick - old_ms_tick;
			}
			else
			{
				add_tick = (0xFFFFFFFF - old_ms_tick) + curr_ms_tick + 1;
			  /*
				Serial1.println();
				Serial1.println();
				Serial1.print("old:");
				Serial1.print(old_ms_tick, HEX);
				Serial1.print("curr:");
				Serial1.print(curr_ms_tick, HEX);
				Serial1.print("add:");
				Serial1.print(add_tick, HEX);
				Serial1.println();
				Serial1.println();
			  */
			}
			sys_count_1ms += add_tick;
			count_1ms += add_tick;
			
gotoSysTimer:
			if( 9 < count_1ms )		// 10ms?
			{
				count_1ms -= 10;
				
				sys_count_10ms++;
				count_10ms++;
				if( 9 < count_10ms)		// 100ms?
				{
					count_10ms -= 10;
					
					sys_count_100ms++;
					count_100ms++;
 					if( 9 < count_100ms) 	// 1 SEC?
					{
						count_100ms -= 10;
						
						sys_count_1sec++;
						count_1sec++;
						if( 9 < count_1sec) 	// 1 SEC?
						{
							count_1sec -= 10;
						}
 					}
						
					// 500ms , 5 SEC
					if(!(count_100ms % 5)) 	// 500ms? [ 0, 5 ]
					{
						sys_count_500ms++;

						count_500ms++;
						if( 9 < count_500ms )	// 500ms
						{
							count_500ms -= 10;
					
							sys_count_5sec++;
							//count_5sec++;
						}
					}
					//---
				}
			}

			if(9 < count_1ms)	//
			{
				goto gotoSysTimer;
			}
			
		  #if LOG_SYSTICK_DELAY
			//if(9 < add_tick)
			//if(10 < add_tick)
			//if(5 < add_tick)
			if(2 < add_tick)
			{
				// 10ms Over Delay
				system_delay_cnt++;
				LogPrintln(" LG] SYSTK MN_DLY:" + String(add_tick) + " ms,cnt:" + String(system_delay_cnt));
			}
		  #endif
		  
			old_ms_tick = curr_ms_tick;
		}
	}
}

//---------------------------------------------------------
// 메인보드 타입 검지
//	구보드(V1) - NC : Open(HIGH)
//	신보드(V1.1) - GND : LOW
//---------------------------------------------------------
int getBoardType()
{

#if (DEF_NOT_USE)

	int rts = MAIN_BOARD_V10;

	String _strMsg;

	mainBoardTypedData = 0;

	short i =0;
	for(i=0; i<5; i++)
	{
		mainBoardTypedData += ((unsigned int)digitalRead(BD_TYPE_PIN) ) << i;
		delay(10);		// 10 ms마다 읽기
	}
	// 판별값으로 반환값(보트타입) 저장
	if(0x1F == mainBoardTypedData)	rts = MAIN_BOARD_V10; 	// 구보드 연속 50ms(10ms * 5번) HIGH
	else							rts = MAIN_BOARD_V11;	// 신보드

	#if (LOG_MAIN_BOARD_TYPE)
		_strMsg = (MAIN_BOARD_V10 == rts? "MBD_V10":"MBD_V11");
		LogPrintln(" LG] INFO_ BoardType: " + _strMsg + ",data: " + String(mainBoardTypedData));	
	#endif

	return rts;
	
#else

	return MAIN_BOARD_V10;

#endif

}



/*------------------------------------------------------------------

   SETUP Arduino
------------------------------------------------------------------*/
void setup()
{
	curr_ms_tick = millis();
	old_ms_tick = curr_ms_tick;
		
	//== 1) Communiction
		// SERIAL 0 = USB(CDC) - ARDUIO IDE /Tool / CDC Enable [v]
	//Serial.begin(115200);		  // begin 
	//Serial.setTimeout(10);		  // wait 10ms, Default 1SEC
	//Serial.setTimeout(20);		  // wait 10ms, Default 1SEC
	//Serial.setTimeout(5);		  // wait 10ms, Default 1SEC
	//Serial.println();			  // print an Empty Line
	
	//== 2) SERIAL_1 : LOG Serial SET
	Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);		// TXD_0 / RXD_0
	Serial1.setTimeout(10);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(30);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(15);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(5);		  // wait 10ms, Default 1SEC
	//Serial1.println();				// print an Empty Line

	//-------------------------------------------------------------------------------
	// == 메인모터 MainMot Default VAL읽기, FILE없으면 FILE쓰기(70)
	//== 3) FILE SYS :: MANUF INFO
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON FS INIT");
	// 순서 중요: Enow Init보다 우선 실행될 것.
	fsInfo.init();

	String strBootInfo = fsInfo.getInfo(BOOTING_INFO);
	if (strBootInfo.substring(0,1) == "-") {
		strBootInfo = STR_BOOTINFO_DEFAULT;
	}
	g_AutoReboot  = strBootInfo.substring(0,1).toInt();

	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON MAIN Ver:" + mainVer + (g_AutoReboot ? "-R" : "") + ",mode:" + String(mainMode));
	LogPrintln(" LG] BOOT INFO:" + strBootInfo + ",dlytime:"+String(DEF_PWRON_DELAY_TIME) + ",sftime:"+String(DEF_USB_SAFE_TIME) + ",Max_Priority:"+String(configMAX_PRIORITIES-1));


	//== 4) SUB_AC ON Cont
	curr_ms_tick = millis();
	pinMode(SUB_ACOFF_PIN, OUTPUT);
	pinMode(PC_USB5V_PIN, INPUT_PULLUP);
	pinMode(PWROFF_SWC_PIN, INPUT);			// LOW ACTIVE

	int usb5v = digitalRead(PC_USB5V_PIN);
	int pwrSW = digitalRead(PWROFF_SWC_PIN);
	int	ledMd = 1;

	// (V108) 변경 - 공장 부팅불 장애 대책
	g_ResetReason = esp_reset_reason();		// (V108) 1:PWRON Reset, 0:USB_UART_CHIP_RESET, 3:RTC_SW_CPU_RST
	LogPrintln(" LG] SUB_ACOFF_PIN:" + String(digitalRead(SUB_ACOFF_PIN)) + ",PC_USB5V_PIN:" + String(usb5v) + ",PWROFF_SWC_PIN:" + String(pwrSW) + ",esp_reset_reason:" + String(g_ResetReason));
	if ((g_ResetReason == 0) || (g_ResetReason == 3))	{	// COM Port Reset or ESP.restart() reset
		SerialPortEnable();				// (V108)
		g_ProcStatus = PS_RUNNING;
	}
	else {
		if (pwrSW == LOW) {				// 전원스위치 On?
			SerialPortPinInput();		// (V108)
			g_ProcStatus = PS_BOOTING;
		}
		else {
			subAC_Off();				// SUB AC Off
			ioLedOffSetting();
			SerialPortEnable();			// (V108)
			ledMd = 0;
			g_ProcStatus = PS_INITIAL;
		}
	}

	//== 5) SET CONFIG (AD PORT)
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON IO SET");
	
	//== [ 0 ] CONFIG READ : CONFIG에 따라 포트 출력/입력 설정 다름.
	dfConfig = setConfig();
	
	// == 6) IO PIN SETTING
	curr_ms_tick = millis();
	ioPinSetting();

	// 보드타입 구분 판단.
	#if (DEF_NOT_USE)
		mainBoardType = getBoardType();
	#endif

#if (FUNC_IO_EXT_UART)
	//== 7) SERIAL_2 :  IO_Extention UART
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON LOG Serail 2 SET");
	ExtSerial.begin(115200, SERIAL_8N1, UART2_RXD_PIN, UART2_TXD_PIN);	  // Return Thing
	ExtSerial.setTimeout(10);		// wait 10ms, Default 1SEC
	//Serial2.setTimeout(20);		// wait 10ms, Default 1SEC
#endif
/*	(V108) 앞쪽으로 옮김
	//== 7) FILE SYS :: MANUF INFO
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON FS INIT");
	// 순서 중요: Enow Init보다 우선 실행될 것.
	fsInfo.init();
*/	

#if IO_I2C
	//== 8) EEPROM
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON nvm Init");
	//Wire.begin(I2C_DT_PIN, I2C_CLK_PIN);				  // SDA, SCL
	eNvm.init();
	delay(50);

	#if IO_NVM_TEST
		curr_ms_tick = millis();
		eeprom_test();
	#endif
#endif

	if (Update.hasError()) {			// (V1081)
		Update.end(false);  // 시스템 OTA 세션 정리
		LogPrintln(" lg] System: Previous OTA error cleared");
	}

//-------------------------------------------------------------------------------
	// == 메인모터 MainMot Default VAL읽기, FILE없으면 FILE쓰기(70)
	String strMmotVal = fsInfo.getInfo(MAINMOT_INFO);
	short mMotVal = strMmotVal.toInt();
	//if( -3 == mMotVal)	// Para NG
	if( 0 > mMotVal)	// -1: Not Val, -2: No File: -3 : Para NG
	{
		defaultTorqueMotor = AUTOSET_DEFAULT_TORQ;
	}
	else if(MAIN_MOT_FS_MIN_DUTY > mMotVal || MAIN_MOT_FS_MAX_DUTY < mMotVal)	//40 ~ 100 범위 밖
	{
		fsInfo.saveInfo(MAINMOT_INFO, STR_AUTOSET_DEFAULT_TORQ);
		defaultTorqueMotor = AUTOSET_DEFAULT_TORQ;
	}
	else	// OK, 값저장
	{
		defaultTorqueMotor = mMotVal;
		// AP에 값통지는 장치에러 CHECK시 통지(TM일 경우)
	}

	#if (LOG_MAINMOT_INFO)
		LogPrintln(" lg] MmotV VAL : " + strMmotVal + "," + String(defaultTorqueMotor));
	#endif
//-------------------------------------------------------------------------------

	// == BLDC제한값 Mot Default VAL읽기, FILE없으면 FILE쓰기( 기본값 100 )
	String strBldcVal = fsInfo.getInfo(BLDC_LIMIT_INFO);
	short iBmotVal = strBldcVal.toInt();

	// TBD, 조건문은 정사 필요.
	// Error , 기본값 = 100 제한
	if( 0 > iBmotVal)	// -1: Not Val, -2: No File: -3 : Para NG
	{
		bldcLimitVal = BLDC_LIMIT_MIN;
	}
	else if(BLDC_LIMIT_MIN > iBmotVal || BLDC_LIMIT_MAX < iBmotVal)	//100 ~ 255 범위 밖
	{
		// 파일 저장 , 기본갓 (100)
		fsInfo.saveInfo(BLDC_LIMIT_INFO, STR_BLDC_LIMIT_MIN);
		bldcLimitVal = BLDC_LIMIT_MIN;
	}
	else	// OK, 값할당
	{
		bldcLimitVal = iBmotVal;
		// AP에 값통지는 장치에러 CHECK시 통지(TM일 경우)
	}
	
	#if (LOG_BLDC_LIMIT_INFO)
		LogPrintln(" lg] BmotV VAL : " + strBldcVal + "," + String(bldcLimitVal));
	#endif

//-------------------------------------------------------------------------------


#if 1
	//== 9) eNow RF 2.4G
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON enow INIT");
	eNow.init(recv_cb_esp_now_msg);  // esp_now
	//--- now Callback Func ( Recv from Slave Message ),  ESP-NOW 데이터 수신시 콜백 함수를 등록.
	if(ESP_OK != esp_now_register_send_cb(sent_cb_esp_now_sts))		// Data송신완료 콜백함수
	{
		LogPrintln(" LG] Error nowSent CallBack Function");	
	}
#endif

//-------------------------------------------------------------------------------


  	//== 10) Object INIT ( Servo, Torque, Bobbin, MainEncoder )
#if IO_SERVO
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON SERVO MOT INIT");
	sMotor.init();					// servo Motor
#endif

	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON MAIN MOT INIT");
	torqMotor.init();				// torque Motor

#if IO_BBN
	curr_ms_tick = millis();
	bbnMotor.init();				// Bobin Motor
#endif

	curr_ms_tick = millis();
	mainEnc.init();
    //--- Defien Callback
	curr_ms_tick = millis();
    mainEnc.setRotateCallback(rotateChangeCallback);

#if (IO_LM_MOT)
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON LM MOT INIT");
	lmMotor.init();				// Bobin Motor
    //----) Defien Callback
	curr_ms_tick = millis();
    //mainEnc.setRotateCallback(rotateChangeCallback);
#endif

#if IO_DLED
	curr_ms_tick = millis();
	extLed.init();
#endif
  
	//== 20) Output All Off
	// Out ALL OFF
	curr_ms_tick = millis();
	motor_AllOff();
	// PowerOn, All LED OFF
	curr_ms_tick = millis();
	extLed_AllOff();

	curr_ms_tick = millis();
	ledoff_BoardLedAllOff();

	//--- External LED ON
//	curr_ms_tick = millis();
//	ledOn_MbBtm();

	// REEL ALL OFF (타이밍 검토 필요)
	//reelOut_AllOff_ForceStep();	// REEL OUT ALL OFF
	curr_ms_tick = millis();
	reelOut_AllOff();	// REEL OUT ALL OFF
		
	// == 21) Power On LED2 Blinking / LED4 1 times BLNK
	curr_ms_tick = millis();
	delay(200);		// LED Blink
  	int cnt = 1;
  	while (cnt < 10)		// 400ms = 50ms * 8 times(2~9)
  	{
    	cnt++;
    	digitalWrite(BD_LED2_PIN, (cnt%2));		// off,on~off,on
    	delay(50);
  	}
	//digitalWrite(BD_LED2_PIN, LOW);
	
#if (!IO_LM_MOT)
	digitalWrite(BD_LED4_PIN, HIGH);	// MAIN BOARD POWER ON
	delay(200);		// LED Blink
	digitalWrite(BD_LED4_PIN, LOW);	// MAIN BOARD POWER OFF
#endif
	
	//== 22) Input Filtering at Power ON (3 times)
	// 1ms Input처리
	curr_ms_tick = millis();
	sensor1ms[0].old2 = 0;
	sensor1ms[0].old = 0;
	sensor1ms[0].curr = 0;
	sensor1ms[0].lvl = 0;
	sensor1ms[0].le = 0;
	sensor1ms[0].te = 0;
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);

	
	curr_ms_tick = millis();
	// 10ms Input처리
	sensor10ms[0].old2 = 0;
	sensor10ms[0].old = 0;
	sensor10ms[0].curr = 0;
	sensor10ms[0].lvl = 0;
	sensor10ms[0].le = 0;
	sensor10ms[0].te = 0;
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);

	//== 23) 미사용(삭제함:무조건 ON) : Reset시 전원스위치의 상태에 따라  AC On/OFF 삭제(다운로드시 전원 OFF됨)
//#if (IO_PC_PWR_OFF)
#if (0)	// NOT_USE
	curr_ms_tick = millis();
	subAc_Control_atReset();
#endif

  //== 24) 초기 변수값 설정 

  //-- NOW통신 변수 초기 설정
  curr_ms_tick = millis();
  //now_msg.pid = -1;					// now mesage -1( not Recv Pid )
  now_rcv_id_cb = 0;
  now_rcv_id = 0;
  now_msg_str_cb ="";
  now_msg_str ="";


  curr_ms_tick = millis();
  //--- LED변수 초기화
  initSet_LedContVal(ledMd);		// LED VAL INIT

	if (ledMd == 0) {
		pwrMode = PWR_OFF;			//PWR_OFF
		extLed_AllOff();
	}

  //--- CONT메인 모드 설정
  mainMode = NORMAL_MODE;	// NORMAL MODE
  LogPrintln(" LG] PWRON **MAIN Mode= " + String(mainMode) + ",time:" + String(millis()-old_ms_tick)+"ms");

	// --- BBN ENC BUFF CLEAR
	for(int i=0; i<ENC_INTV_BUFF_SIZE; i++)
	{
		stEncIntvBuff[i].tick = 9999;
		stEncIntvBuff[i].width = 9999;
		stEncIntvBuff[i].dir = 0;
		stEncIntvBuff[i].enc = 9999;
		stEncIntvBuff[i].dist = 9999;
	}
	
	//--- SYSTEM TIMER STAMP 클리어
#if (SYS_TIMER_INTR_ENB)
	curr_ms_tick = lSys1MsTime;
#else
	curr_ms_tick = millis();
#endif
	old_ms_tick = curr_ms_tick;		//

	//--- Power On 1St Send CMD Req Flag SET
	pwrOn1stCmdSendFlag = 1;


	//== 30) 단독 Reset CMD송신
	ApPrintln(STX_PWRON_STS + "11%");		  // CONT MAIN BOARD RESET


	//== 31) GPIO INTR : Enc 인터럽트 추가, REL
#if (GPIO_INTR_ENCA_ENB)
	//attachInterrupt(digitalPinToInterrupt(ENC_INTR_PIN), encIntrHandle, RISING);	//RISING -NG : INT 2번 발생되는 경우 있음
	attachInterrupt(digitalPinToInterrupt(ENC_INTR_PIN), ISR_encIntrHandle, FALLING);
#endif


	//== 32) 하드웨어 타이머 설정 (타이머0 사용)
#if (SYS_TIMER_INTR_ENB)
	timer = timerBegin(0, 80, true);  // 타이머 0, 80분주
	timerAttachInterrupt(timer, &ISR_onTimerHandler, true);  // 타이머 인터럽트 핸들러 연결
	//timerAlarmWrite(timer, 1000, true);  // 1ms마다 인터럽트 발생 (1 초 = 1000000 ticks)
	timerAlarmWrite(timer, 100, true);  // 100 US 마다 인터럽트 발생 (100 us = 100000 ticks)
	timerAlarmEnable(timer);  // 타이머 인터럽트 활성화
#endif

}


/*------------------------------------------------------------------
   LOOP Arduino
------------------------------------------------------------------*/
void loop()
{

  system_counter();  // Create system Timer(1ms / 10ms /100ms)

  if(0 < sys_count_1ms)
  {
  
#if (SYS_TIMER_INTR_ENB)
	// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
	portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
	chk_ms_tick = lSys1MsTime;	   // current_time 값 읽기
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
#else
  	chk_ms_tick = millis();		// Read Current Tick
#endif
	sys_count_1ms--;

	//=== 1) Input Control
	read_input_1ms();
	if(0 < sys_count_10ms)
	{
		read_input_10ms();
	}
	if(0 < sys_count_100ms)
	{
		read_input_100ms();
	}

	//=== 2) RECV CMD from AP, ROD, ExtUART, LOG
	uartRecvParsing();	// NOT DELAY(BREAK) Pasing
	uartRecvHandler();	// PC(APP) - Main UART
	
#if 1
	nowRecvHandler();		// MAIN <-> ROD NOW(wifi RF 2.4G)
#endif

#if (FUNC_IO_EXT_UART)
	extIOuartRecvHandler();	// extIO_Board
#endif

#if	(FUNC_LOG_UART_RCV_ENB)
	logUartRecvParsing();
	logRecvHandler();		// PC(LOG) <-> MAIN UART
#endif

	// 3) Process Control
	// 3-1) 1ms Process
	t1ms_Process_Exec();

	// 3-2) 10ms Process
	if(0 < sys_count_10ms)
	{
		sys_count_10ms--;

		// Execution Process
		t10ms_Process_Exec();

	}

	// 3-3) 100ms Process 
	if(0 < sys_count_100ms)
	{
		sys_count_100ms--;

		t100ms_Process_Exec();

	  #if (!IO_LM_MOT)
		//*** WDT OUT,, MIN 900ms
		if(0 == diagOutFlag_WDToff)	// Not Diag WDT_OFF Test
		{
			#if (DEF_NOT_USE)		// 출력 삭제
				digitalWrite(WDT_OUT_PIN, !digitalRead(WDT_OUT_PIN));	// Toggle OUT
			#endif
		}
	  #endif
	  
  	}

	// 3-4) 500ms Process 
	if(0 < sys_count_500ms)
	{
		sys_count_500ms--;

		t500ms_Process_Exec();

	  #if (!IO_LM_MOT)
		//*** WDT OUT,, MIN 900ms
		if(0 == diagOutFlag_WDToff)	// Not Diag WDT_OFF Test
		{
			#if (DEF_NOT_USE)
				digitalWrite(WDT_OUT_PIN, !digitalRead(WDT_OUT_PIN));
			#endif
		}
	  #endif
	  
  	}


	// 3-5) 1 SEC Process
	if(0 < sys_count_1sec)
	{
		sys_count_1sec--;

		t1sec_Process_Exec();
	
	}

	// input LE/TE CLR
	//sensor1ms[0].le = 0;
	//sensor1ms[0].te = 0;
	//sensor10ms[0].le = 0;
	//sensor10ms[0].te = 0;

#if (LOG_1MS_EXE_DELAY)

	unsigned long end_ms_tick;

	#if (SYS_TIMER_INTR_ENB)
		// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
		portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
		end_ms_tick = lSys1MsTime;	   // current_time 값 읽기
		portEXIT_CRITICAL(&mux);   // 임계 영역 종료
	#else
		end_ms_tick = millis(); 	// Read Current Tick
	#endif

//	if (2 < (end_ms_tick-chk_ms_tick))	// 4 ms Over
	if (1 < (end_ms_tick-chk_ms_tick))	// 4 ms Over
	{
		LogPrintln(" LG] 1msExe Delay: " + String(end_ms_tick-chk_ms_tick) + "ms");
	}
#endif

  }

/*=== Test Exec Time (Serila Wait) ===
	lpCnt++;
	if( (0 < lpCnt) && (1000001 > lpCnt) )
	{
		if( !(lpCnt % 100) )
  		{
			LogPrintln(String(curr_ms_tick)+"    lp:"+ String(lpCnt));
  		}
  	}
 */

}
