#pragma once

// Cross-module compatibility constants retained during the first conversion.
#define PWR_ON 1
#define PWR_OFF 0
#define SEND_NOW_STEP_TIME 10
#define GMWAT_STBY 2
#define FIGHT_PTN_CW_OFF 0
#define WAVE_BBN_TORQ_START 0x03
#define DEVCHK_BBN_WIRE_REL_DUTY 50
#define ENC_SEN_OK 0

enum
{
    PS_INITIAL = 0,
    PS_ON_WAIT,
    PS_BOOTING,
    PS_RUNNING,
    PS_RSPWAIT,
    PS_USBWAIT,
    PS_OFFWAIT
};

extern int pwrMode;
extern volatile int reelOut_AllOff_Flag;
extern volatile int reelAlloffStep;



// Shared low-level state and compatibility API. Feature-owned declarations live in module headers.



#include <Arduino.h>

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

extern String strMainBoard[10];

typedef struct _ledCont
{
	unsigned int oldcont;			               
	unsigned int cont;			               
	unsigned int colorNo;		           
	unsigned int colorMany;		             
	unsigned int bTime;			                
	unsigned int dTime;			              
	unsigned int bStep;			              
	unsigned int dStep;			            
	unsigned int dColorCnt;		                          
	unsigned int dReqCnt;		  
	unsigned int rsv_e;
}stLedCont;

typedef stLedCont *ptrLedCont;

#define LED_CONTROL_10MS	10

#define LED_CONTROL_20MS	20

#define LED_CONTROL_50MS	50

extern const stLedCont initledCont[LED_IDX_MAX];

extern stLedCont ledCont[LED_IDX_MAX];

extern unsigned int ledLight;

extern unsigned int ledDimmHoldTime[LED_IDX_MAX];

#define COLOR_ORDER	10

extern unsigned int ledColorOrder[LED_IDX_MAX][COLOR_ORDER];

typedef struct _diagCode
{
	int catNo;			      
	int itemNo;			       
	int act;			         
	int para;			             
}stDiagCode;

typedef stDiagCode *ptrDiagCode;

extern ENow eNow;

extern Potentiometer torqMotor;

extern ServoMotor sMotor;

extern BobbinMotor bbnMotor;

extern Encoder mainEnc;

extern LmMotor lmMotor;

extern Led extLed;

extern Eeprom eNvm;

extern FileSys fsInfo;

extern String eulerStr;

extern String battLvlStr;

extern int ibatteryLvl;

extern int oldibatteryLvl;

extern int ibatChargeLvl;

#define ERR_OK		0

#define ERR_CW		-1

#define ERR_CCW		-2

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

#define STR_ERR_OK			"00"

#define STR_ERR_NG			"-1"

#define STR_ERR_FAIL		STR_ERR_NG

#define STR_ERR_RANGE_OVER	"-2"

#define STR_ERR_PARA_NG		"-3"

extern String STX_MAINALIVE;

extern String STX_INFO_REQ;

#define MAIN_IF_VER			"00"

#define NUM_MAIN_IF_VER		0

#define ROD_BOARD_TYPE		"01"

#define NUM_ROD_BOARD_TYPE 	1

#define ROD_REGIST_TO_SEC	"02"

#define NUM_ROD_REGIST_TO_SEC	2

#define IMU_INTVAL_TIME		"03"

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

#define MAINMOT_INFO_READ		"10"

#define NUM_MAINMOT_INFO_READ		10

#define MAINMOT_INFO_SAVE		"11"

#define NUM_MAINMOT_INFO_SAVE		11

#define MAIN_BOARD_TYPE		"12"

#define NUM_MAIN_BOARD_TYPE 	12

#define BLDC_MOT_LIMIT_READ			"13"

#define NUM_BLDC_MOT_LIMIT_READ		13

#define BLDC_MOT_LIMIT_WRITE		"14"

#define NUM_BLDC_MOT_LIMIT_WRITE	14

extern String STX_TORQUE_MOTOR;

extern String STX_SERVO_MOTOR;

extern String STX_BBN_MOTOR;

extern String STX_BREAK_MOTOR;

extern String STX_LED_CONT;

extern String STX_RESERVE07;

extern String STX_IMU_SET;

extern String STX_IMU_DATA_STS;

extern String STX_VER_READ;

extern String STX_GAME_STS;

extern String OLD_STX_BAT_LVL;

extern String STX_BAT_LVL;

extern String STX_BUTT_SWC;

extern String STX_HAND_ENC;

extern String STX_DEV_CONT_REQ;

extern String STX_GET_ADDR;

extern String STX_SET_ADDR;

extern String STX_APSTS_RESP;

extern String STX_MOT_OUT_RATE_SET;

extern String STX_ROD_CONN;

extern String STX_IMU_CONN;

extern String STX_PWRON_STS;

#define CONT_MAIN_BOARD	1

#define REEL_MAIN_BOARD 2

extern String STX_VRT_MOT_CONT;

extern String STX_BTN_LED_CONT;

#define BTN_LED_ALL_POSI	0

#define BTN_LED_LEFT_POSI	1

#define BTN_LED_RIGHT_POSI	2

#define BTN_LED_LAST_POSI	BTN_LED_RIGHT_POSI

extern String STX_GAME_STS_WAT;

#define GAME_xxx	0

extern String STX_DEV_SET_REQ;

#define STR_MAINMOT_MINSET_REQ		"01"

extern String STX_TM_INPUT_REQ;

extern String STX_TM_OUTPUT_REQ;

extern String STX_AP_INFO_SEND;

#define DEF_AP_TYPE		1

extern String STX_ROD_REGIST;

#define ROD_REGI_AP_ENTRY	1

#define ROD_REGI_AP_CANCEL	2

#define ROD_REGI_FW_CANCEL		"03%"

#define ROD_REGI_WRITE_START	"04%"

#define ROD_REGI_FINISH			"05%"

extern String STX_DEV_CHK_ERR_LOG;

extern String STX_DEV_FUNCTION_REQ;

#define STR_WIRE_AGING_TEST		"01"

#define NUM_WIRE_AGING_TEST		01

extern String STX_DATA_PARA_SET;

extern String STX_DIAG_INPUT;

extern String STX_DIAG_OUTPUT;

extern String STX_DIAG_FUNC;

extern String STX_DIAG_SET;

extern String STX_DIAG_EXIT;

extern String STX_DNLD_EXEC;

extern String STX_WAVE_SET;

extern String STX_BITE_SET;

extern String STX_HIT_SET;

extern String STX_BREAK_SET;

extern String STX_TOKTOK_SET;

extern String STX_EXTLED_SET;

extern String STX_RANDING_START;

extern String STX_TORQ_ADJ_SET;

extern String STX_TORQ_TUN_SET;

extern String STX_LED_SET;

extern String STX_ENB_SET;

extern String STX_TEST_EXEC;

extern String OLD_RESP_STX_BUTTON;

extern String OLD_RESP_STX_ENCODER;

extern String RESP_STX_BUTTON;

extern String RESP_STX_ENCODER;

extern String STS_APSTS_REQ;

extern String RESP_STX_ROD_CONN;

extern String RESP_STX_IMU_CONNECT;

#define RESP_IMU_FULLSTR_CONN		"$2101%"

#define RESP_IMU_FULLSTR_DISCONN	"$2100%"

extern String STS_PWRON_STS;

#define STX_CMD_MAINALIVE			0

#define STX_CMD_INFO_REQ			1

#define STX_CMD_TORQUE_MOTOR		2

#define STX_CMD_SERVO_MOTOR			3

#define STX_CMD_BLDC_MOTOR			4

#define STX_CMD_LED_CONT			6

#define STX_CMD_IMU_SET				8

#define STX_STS_IMU_DATA_STS		9

#define STX_CMD_VER_READ			10

#define STX_CMD_GAME_STS			11

#define STX_STS_BAT_LVL				12

#define STX_STS_BUTT_SWC			13

#define STX_STS_HAND_ENC			14

#define STX_CMD_DEV_CONT_REQ		15

#define STX_CMD_GET_ADDR			16

#define STX_CMD_SET_ADDR			17

#define STX_CMD_APSTS_RESP			18

#define STS_CMD_APSTS_REQ			18

#define STX_CMD_MOT_OUT_RATE_SET	19

#define STX_STS_ROD_CONN			20

#define STX_STS_IMU_CONN			21

#define STX_STS_PWRON_STS			22

#define STX_CMD_VRT_MOT_CONT		23

#define STX_CMD_BTN_LED_CONT		24

#define STX_CMD_ROD_REGIST			30

#define STX_STS_ROD_REGIST			30

#define ROD_REGI_CANCEL_TO		(25*1000)

#define ROD_REGI_END_TO_1ST		(1500)

#define ROD_REGI_END_TO_2ND		(4000)

#define DEF_STX_GET_ADDR     16

extern String STATE_DISABLE;

extern String STATE_WAITING;

extern String STATE_ENABLE;

extern String STATE_READY;

#define STATE_CONN_UNKNOWN     "-1";

#define STATE_DISCONN          "00";

#define STATE_CONNECTED        "01";

#define STATE_DISCONN_NOTINIT  "11";

#define STATE_DISCONN_NOTPEER  "12";

#define STATE_DISCONN_SLEEP    "88";

#define STATE_DISCONN_MAC      "99";

#define IMU_CONN_2CHAR_INT 11

extern bool isGameEnable;

#define STR_BOOTINFO_DEFAULT	"06028"

#define	DEF_PWRON_DELAY_TIME	60

#define	DEF_PWRON_DELAY_TMAX	99

#define	DEF_PWRON_DELAY_TMIN	10

#define	DEF_USB_SAFE_TIME		28

extern unsigned long g_RecvCmdCount;

extern unsigned long g_LastCmdTick;

extern int g_SerialEnable;

extern int g_ResetReason;

extern int g_AutoReboot;

extern unsigned short mainBoardType;

extern unsigned short mainBoardTypedData;

#define ROD_BOARD_V1	0

#define ROD_BOARD_V2	1

extern unsigned short rodBoardType;

extern String strRodBoardType;

extern int ap_conn_status;

extern int old_ap_conn_status;

extern unsigned int apAliveConn_NotRecvCnt;

extern unsigned int apAliveElse_NotRecvCnt;

extern int rod_conn_status;

extern int old_rod_conn_status;

extern String g_RespVersion;

extern bool isPowerOnReady;

extern unsigned long curr_ms_tick;

extern unsigned long old_ms_tick;

extern unsigned long chk_ms_tick;

extern unsigned int count_1ms;

extern unsigned int count_10ms;

extern unsigned int count_100ms;

extern unsigned int count_500ms;

extern unsigned int count_1sec;

extern unsigned int sys_count_1ms;

extern unsigned int sys_count_10ms;

extern unsigned int sys_count_100ms;

extern unsigned int sys_count_500ms;

extern unsigned int sys_count_1sec;

extern unsigned int sys_count_5sec;

#define TO_1MS_TICK 1

#define TO_10MS_TICK 10

#define TO_100MS_TICK 100

#define setTO(var) (var = curr_ms_tick)

#define getTO(var) ( (curr_ms_tick < var)? (0xFFFFFFFF - var + curr_ms_tick + 1 ) :  (curr_ms_tick - var) )

#define checkTO(var,TO) ( getTO(var) > TO)

#define setLmEnc(var) (var = mainEnc.lmCnt)

#define checkLmEnc(var,TO) (TO < (mainEnc.lmCnt - var))

extern int reqServoMotor;

extern int oldServoMotor;

extern int absReqServoMotor;

extern int oldAbsServoMotor;

extern short recv_TorqMotor_Flag;

extern int reqTorqueMotor;

extern int oldTorqueMotor;

extern int reqBreakMotor;

extern int oldBreakMotor;

extern long reqImuMeasFlag;

extern long oldImuMeasFlag;

extern short defaultTorqueMotor;

extern short bldcLimitVal;

extern int cntRightButtonOn;

extern int cntRightButtonOff;

extern int rightBtnOnCount;

extern int exeWave;

extern int exeBite;

extern int exeHit;

extern int exeHit_1st;

extern int exeResist;

extern int exeFight;

extern int exeHoldon;

extern int exeHoldOff;

extern int bitePtn_Flag;

extern int exeBitePtn1flag;

extern int exeBitePtn2flag;

extern int exeBitePtn3flag;

extern short holdOnPtnAll_Flag;

extern short holdOnPtn1_Flag;

extern int exeFastHoldOn;

extern int exeFastHoldOff;

extern int exeButtonCombo;

extern int exeButtonComboOff;

extern int exeRanding;

extern int exeSuccessFail;

extern int exeContinue;

extern int exeGameOver;

extern int execContinue2Ready;

extern int exeProgramEnd;

extern int exeResistServoENB;

extern int exeHoldonServoENB;

extern int biteLogOut;

extern int gameLevel;

extern int gameStatus;

extern int oldgameStatus;

extern int fishTypeFixENB;

extern int fishTypeAuto;

extern int fishTypeFix;

extern int waveControlCase;

#define WAVE_NOT_CONT	0

#define WAVE_WEAK		1

#define WAVE_NORMAL		2

#define WAVE_STRONG		3

#define WAVE_MAX		4

extern int waveType;

typedef struct _stWaveServoSet
{
	int angle1;		               
	int angle2;		               
	int interval;
}stWaveServoSet;

extern stWaveServoSet stWaveServoVal;

#define WAVE_PWR_MAX	(1+12)

typedef struct _stWaveTorqSet
{
	int rtTorq;			              
	int tgTorq;			              
	int offInterval;
	int onInterval;

	unsigned int pwr[WAVE_PWR_MAX];
}stWaveTorqSet;

typedef stWaveTorqSet *stWavePtr;

extern stWaveTorqSet stWaveTorqVal;

#define DEFAULT_BBN		9

extern stWaveTorqSet stWaveBdutyVal;

typedef struct _stWavePower
{
	unsigned int pwr[WAVE_PWR_MAX];
}stWavePower;

typedef stWavePower *stWavePwrPtr;

extern stWavePower stWavePwrBbn;

extern int biteTestCase;

extern int biteTorqCase;

extern int biteCnt;

extern int biteType;

typedef struct _biteTorqSet
{
	int targetTorq;
	int targetInterval;
	int incTorq;
	int incInterval;
	int decTorq;
	int decInterval;
	  
	int shallowTorq;
	int shallowInterval;
	int deepTorq;
	int deepInterval;

  	int torqType[1+8];	                
}biteTorqSet;

#define DEEP_ADD_TORQ	25

#define DEEP_STEP_TORQ	6

extern biteTorqSet stBiteTorqVal;

#define START_TORQ  DEFAULT_TORQ+10

#define END_TORQ    DEFAULT_TORQ

#define OFF_TORQ	(DEFAULT_TORQ-20)

#define WF_END_TORQ    DEFAULT_TORQ+20

#define HIT_LOW_TORQ    DEFAULT_TORQ+20

#define HIT_END_TORQ    DEFAULT_TORQ+20

#define BITE_WAIT_TIME	300

extern int hitServoAngle;

extern int hitServoInterval;

extern int torqResistFix;

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

	int torqType[1+12];		                         
} hitTorqSet;

#define HIT_TORQ_ADD_VAL 20

#define HIT_TORQ_STEP_VAL 3

extern hitTorqSet stHitTorqVal;

#define HIT_WAIT_TIME	50

typedef struct _stBreakSet
{
	int val[9];
}stBreakSet;

extern stBreakSet stBreakVal[1+3];

extern int breakType;

extern int targetBreakVal;

extern int outBreakVal;

extern int decBreakCnt;

extern long lpCnt;

extern int DiagInMonitor;

extern int diagInFlag_ResetSwc;

extern int diagInFlag_BootSwc;

extern int diagInFlag_WdtLbSig;

extern int diagInFlag_SelectBoard;

extern int diagInFlag_BobbinEncA;

extern int diagInFlag_BobbinEncB;

extern int diagInFlag_BobbinEncDir;

extern int diagInFlag_AngleHomeSns;

extern int diagInFlag_BbnBldcFG;

extern int diagInFlag_PonSwc;

extern int diagInFlag_PcUsb5V;

extern int diagInFlag_Interlock;

extern int sensorResetSwc;

extern int oldsensorResetSwc;

extern int sensorBootSwc;

extern int oldsensorBootSwc;

extern int sensorWdtFbSig;

extern int oldsensorWdtFbSig;

extern int sensorSelBoard;

extern int oldsensorSelBoard;

extern int sensorBbnEncA;

extern int oldsensorBbnEncA;

extern int sensorBbnEncB;

extern int oldsensorBbnEncB;

extern unsigned int sensorBbnEncCnt;

extern unsigned int oldsensorBbnEncCnt;

extern int sensorBbnEncDir;

extern int oldsensorBbnEncDir;

extern int sensorAngHomeSns;

extern int oldsensorAngHomeSns;

extern int sensorBbnFG;

extern int oldsensorBbnFG;

extern int sensorPonSwc;

extern int oldsensorPonSwc;

extern int sensorUsb5V;

extern int oldsensorUsb5V;

extern int sensorDoorIlk;

extern int oldsensorDoorIlk;

#define DIAG_SEN_XXX	0

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

extern STDIAGSEN stDiagSen[SEN_MAX_SIZE];

extern unsigned int oldDiaglmEncPulse;

extern int diagOutFlag_WDToff;

extern int diagOutFlag_LedCent_R;

extern int diagOutFlag_LedCent_G;

extern int diagOutFlag_LedCent_B;

extern int diagOutFlag_LedCent_RGB;

extern int diagOutflag_bbnTimeOut;

extern unsigned int bbnTimeout_step;

extern unsigned long bbnTimeout_Val;

extern int diagBbnflag_TokTokControl;

extern unsigned int bbnTokTok_step;

extern int bbnTokTok_OnDuty;

extern unsigned long bbnTokTok_OnTime;

extern int bbnTokTok_OffDuty;

extern unsigned long bbnTokTok_OffTime;

extern int continueTorq;

extern int currTorq;

extern int oldTorq;

extern int rdy_outTorq;

extern int rdy_eachTime;

extern int rdy_cnt;

extern int rdy_incTorq;

extern int rdy_reqCnt;

extern int gLevel;

extern int gZone;

extern int gStage;

extern String rdyMsg;

extern int recvMainConn_Pwr1stCnt;

typedef struct _paraCheck
{
	int idx;
	int min;
	int max;
}stParaCheck;

#define GAME_STS_LVL	11

#define GAME_STS_LVL	11

#define MAX_PARA_CNT	20

extern stParaCheck paraCheck[MAX_PARA_CNT];

extern int mainMode;

extern int oldmainMode;

extern int ledControlMode;

extern unsigned int bbnActKind;

extern unsigned int bbnOpKind;

extern unsigned int bbnTokCnt;

extern int exeFlag_bldcWave;

extern int exeFlag_bldcBite;

extern int exeFlag_bldcHit;

extern int exeFlag_bldcFight;

extern int exeFlag_bldcHoldOn;

extern int exeFlag_bldcRanding;

extern int exeFlag_bldcSuccess;

extern int exeFlag_bbnMotorStop;

extern int exeFlag_biteBldcTokTokEnd;

extern int exeFlag_holonBldcTokTokEnd;

extern int exeFlag_torqMotorStop;

extern int restPowerDist;

extern unsigned long bbnTokTO;

extern unsigned int oldbbnTokStep;

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
	BBNACT_WAVE,			   
	BBNACT_BITE,			   
	BBNACT_HIT,				   
	BBNACT_FIGHT,			   
	BBNACT_HOLDON,			   
	BBNACT_RANDING,			   

	BBNACT_TBLMAX		    
}eBBNACTKIND;

#define MUL_BBN_PWR 2

extern BBN_ACT sBbnActTbl[BBNACT_TBLMAX];

extern int bbnReqCnt[BBNACT_TBLMAX];

extern int bbnRptCnt;

extern String strFishKind;

extern float fishKg_flt;

extern double fishKg_dbl;

extern int fishSize;

extern int fishRare;

extern int holdDir;

extern int startEnd;

extern int randingPtn;

#define LEVEL_MIN 1

#define LEVEL_MAX 3

extern BBN_ACT sBbnBiteVal;

extern unsigned long sBbnHoldVal_onTime;

extern unsigned long sBbnHoldVal_offTime;

#define FISH_MAX (1+6+1)

extern int sBbnHoldVal[FISH_MAX];

typedef struct _RGB
{
	int r;
	int g;
	int b;
}sLEDRGB;

typedef sLEDRGB *ptrRGB;

extern sLEDRGB sRGB[LED_IDX_MAX];

extern sLEDRGB outRGB[LED_IDX_MAX];

#define COLOR_MAX_NO 16

extern const sLEDRGB sColorTBL[COLOR_MAX_NO];

extern unsigned long bbnDiagTO;

#define BBN_PWM_DUTY_MIN 1

#define BBN_PWM_DUTY_MAX 255

#define BBN_WAIT_TIME_MIN 1

#define BBN_WAIT_TIME_MAX 30000

#define BBN_RPT_CNT_MIN	1

#define BBN_RPT_CNT_MAX	255

#define ROD_RESP_OVER_CNT 		(5)

extern unsigned int rodAlive_SendCnt;

extern int imu_conn_status;

extern int oldimu_conn_status;

extern int exeResistOnceComp;

extern int dimming_time;

extern int olddimming_time;

extern unsigned int ledoffHoldTimeCnt;

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

#define SENSOR_1	0

#define SENSOR_2	1

#define SENSOR_3	2

#define SENSOR_4	3

#define SENSOR_MAX	4

extern sensor sensor1ms[SENSOR_MAX];

extern sensor sensor10ms[SENSOR_MAX];

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

#define SNS_boot		BIT_0

#define SNS_usb5V		BIT_1

#define SNS_PwrOffSwc	BIT_2

#define SNS_AcOff		BIT_3

#define SNS_RSV04		BIT_4

#define SNS_bbnMotFg	BIT_5

#define SNS_bbnEncA		BIT_6

#define SNS_bbnEncB		BIT_7

#define SNS_LM_HOME		BIT_8

#define SNS_LM_LEFT		BIT_9

#define SNS_LM_RIGHT	BIT_10

#define SNS_RSV11		BIT_11

#define SNS_RSV12		BIT_12

#define SNS_LMMOT_FG	BIT_13

#define SNS_RSV14		BIT_14

#define SNS_LM_ENC		BIT_15

#define BIT_SNS_BOOT	0x0001

#define BIT_SNS_USB5V	0x0002

#define BIT_SNS_PWRSWC	0x0004

#define BIT_SNS_ACOFF	0x0008

#define	BIT_SNS_PWRALL	(BIT_SNS_USB5V|BIT_SNS_PWRSWC|BIT_SNS_ACOFF)

#define BIT_SNS_RSV4	0x0010

#define BIT_SNS_BBNFG	0x0020

#define BIT_SNS_BBNENCA	0x0040

#define BIT_SNS_BBNENCB	0x0080

#define BIT_SNS_BBNALL	(BIT_SNS_BBNFG|BIT_SNS_BBNENCA|BIT_SNS_BBNENCB)

#define BIT_SNS_ENCAB	(BIT_SNS_BBNENCA|BIT_SNS_BBNENCB)

#define BIT_SNS_LMHOME_BIT8		0x0100

#define BIT_SNS_LMLEFT_BIT9		0x0200

#define BIT_SNS_LMRIGHT_BIT10	0x0400

#define BIT_SNS_LMMOTFG			0x2000

#define BIT_SNS_LMENC			0x8000

#define SNS_ON		1

#define SNS_OFF		0

#define	LVL01_BBNALL	(sensor1ms[SENSOR_1].lvl & (BIT_SNS_BBNALL))

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

#define LVL10_BOOT		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BOOT))

#define LVL10_USB5V		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_USB5V))

#define LVL10_PWROFF_SWC (sensor10ms[SENSOR_1].lvl & (BIT_SNS_PWRSWC))

#define LVL10_ACOFF		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_ACOFF))

#define	LVL10_PWRALL	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_PWRALL))

#define LVL10_BBNFG		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNFG))

#define LVL10_BBNENCA	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNENCA))

#define LVL10_BBNENCB	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_BBNENCB))

#define	LVL10_ENCAB		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_ENCAB))

#define lev10_lmHome	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMHOME_BIT8))? 1:0)

#define lev10_lmLeft	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMLEFT_BIT9))? 1:0)

#define lev10_lmRight	((sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMRIGHT_BIT10))? 1:0)

#define LVL10_LMMOTFG	(sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMMOTFG))

#define LVL10_LMENC		(sensor10ms[SENSOR_1].lvl & (BIT_SNS_LMENC))

#define LE10_BBNFG		(sensor10ms[SENSOR_1].le & (BIT_SNS_BBNFG))

#define LE10_USB5V		(sensor10ms[SENSOR_1].le & (BIT_SNS_USB5V))

#define LE10_PWROFF_SWC	(sensor10ms[SENSOR_1].le & (BIT_SNS_PWRSWC))

#define LE10_LMHOME		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMHOME_BIT8))

#define LE10_LMLEFT		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMLEFT_BIT9))

#define LE10_LMRIGHT	(sensor10ms[SENSOR_1].le & (BIT_SNS_LMRIGHT_BIT10))

#define LE10_LMMOTFG	(sensor10ms[SENSOR_1].le & (BIT_SNS_LMMOTFG))

#define LE10_LMENC		(sensor10ms[SENSOR_1].le & (BIT_SNS_LMENC))

#define TE10_BBNFG		(sensor10ms[SENSOR_1].te & (BIT_SNS_BBNFG))

#define TE10_USB5V		(sensor10ms[SENSOR_1].te & (BIT_SNS_USB5V))

#define TE10_PWROFF_SWC	(sensor10ms[SENSOR_1].te & (BIT_SNS_PWRSWC))

#define TE10_LMHOME		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMHOME_BIT8))

#define TE10_LMLEFT		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMLEFT_BIT9))

#define TE10_LMRIGHT	(sensor10ms[SENSOR_1].te & (BIT_SNS_LMRIGHT_BIT10))

#define TE10_LMMOTFG	(sensor10ms[SENSOR_1].te & (BIT_SNS_LMMOTFG))

#define TE10_LMENC		(sensor10ms[SENSOR_1].te & (BIT_SNS_LMENC))

extern int recvPwrOffExec_Resp;

extern unsigned int motTestAct;

extern unsigned int motTestMainIdx;

extern unsigned int motTestBbnIdx;

extern uint8_t rcv_src_addr_back[];

extern uint8_t rcv_dest_addr[];

extern uint8_t broad_cast_addr[];

extern volatile int rodRegistMode;

#define LM_POSI_UNKNOWN		0

#define LM_POSI_HOME		1

#define LM_POSI_HOMEMOVE	2

#define LM_POSI_LEFT		10

#define LM_POSI_LEFTEND		11

#define LM_POSI_LEFTMOVE	12

#define LM_POSI_RIGHT		20

#define LM_POSI_RIGHTEND	21

#define LM_POSI_RIGHTMOVE	22

#define LM_POSI_MAX		30

extern int lmPosi;

extern int oldlmPosi;

#define LM_HOME_SEN_ON		HIGH

#define LM_HOME_SEN_OFF		LOW

#define LM_LEFT_SEN_ON		LOW

#define LM_LEFT_SEN_OFF		HIGH

#define LM_RIGHT_SEN_ON		LOW

#define LM_RIGHT_SEN_OFF	HIGH

extern short lmHomeNG_Flag;

extern short lmLeftNG_Flag;

extern short lmRightNG_Flag;

extern short lmFishLevel;

extern short lmLeftDuty;

extern short lmRightDuty;

extern short lmReturnDuty;

extern short endTmReqDuty;

extern short leftEndTmReqDuty;

extern short rightEndTmReqDuty;

extern short lmHome_Init;

extern short lmHome_Flag;

extern short lmLeft_Flag;

extern short lmRight_Flag;

extern short lmReturn_Flag;

extern short lmHold_Stop_Flag;

extern short lmHomeRptCnt;

extern short lmHomeStep;

extern short lmLeftStep;

extern short lmRightStep;

extern short lmReturnStep;

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

#define FISH_LVL_AUTO	0

#define FISH_LVL_1	1

#define FISH_LVL_2	2

#define FISH_LVL_3	3

#define FISH_LVL_MAX	(1 + FISH_LVL_3)

#define MOT_PWR_LVL_a	1

#define MOT_PWR_LVL_b	2

#define MOT_PWR_LVL_c	3

#define MOT_PWR_LVL_MAX	(1 + MOT_PWR_LVL_c)

extern STLMOTDUTYTBL stLmWireDutyTbl[2][MOT_PWR_LVL_MAX];

extern STLMOTDUTYTBL stLmDutyTbl[FISH_LVL_MAX][MOT_PWR_LVL_MAX];

extern STLMOTDUTYTBL stLmHomeTbl;

extern STLMOTDUTYTBL stLmReturnTbl;

extern volatile int nvm01_home_stopWait_time;

#define LM_HOME_RETRY_CNT	3

#define LM_HOME_STOP_WAIT_TIME	0

#define LM_HOME_MOVE_DUTY	40

#define LM_HOME_DIR_CHANGE_TIME		100

#define LM_START_WAIT_TIME 10

#define LM_HOME_TO_TIME			3000

#define LM_MOVE_ERR_TO_TIME		3000

#define LM_MOVE_TO_TIME	3000

extern short fishLevel;

extern short rcvlmPwr;

extern short lmLeftStartReq;

extern short lmRightStartReq;

#define LM_PRIORITY_NO		0

#define LM_PRIORITY_LEFT	1

#define LM_PRIORITY_RIGHT	2

extern short lmPriority;

extern short fishPwr;

extern short gmWatStatus;

extern short oldGmWatStatus;

#define	TIME_SLEEP_LIGHT		60

#define	TIME_SLEEP_DEEP			120

#define	SS_NONE			0

#define	SS_WAIT			1

#define	SS_EXIT			2

#define	SS_DOWN			3

extern int gRodSleepStat;

extern int gRodSleepMode;

extern int gRodSleepTime;

extern volatile unsigned short wireLongCheck_Flag;

extern volatile unsigned short lmWireControl_Flag;

extern volatile unsigned int targetWirePulse;

extern volatile unsigned int currWirePulse;

extern volatile unsigned int oldcurrWirePulse;

#define DF_CONFIG_UNKNOWN		0

#define DF_CONFIG_STAND_ALONE	1

#define DF_CONFIG_LMJIG			99

extern short dfConfig;

extern unsigned short apType;

extern String strImuInterval;

extern unsigned short devReadychkFlag;

extern unsigned short devCheckAllFlag;

extern unsigned short devCheckBbnMotFlag;

extern unsigned short devCheckBbnEncFlag;

extern unsigned short devCheckTorqFlag;

extern unsigned short devCheckLmMotFlag;

extern unsigned short devCheckBldc24VFlag;

extern unsigned short mainMotAutoSet_Flag;

extern unsigned short wireAgingTest_Flag;

extern unsigned short devBbnAllChkStep;

extern unsigned short devBbnMotChkStep;

extern unsigned short devBbnEncChkStep;

extern unsigned short devTorqChkStep;

extern unsigned short devLmMotChkStep;

extern unsigned short devBldc24VChkStep;

extern unsigned short mainMotAutoSet_Step;

extern unsigned short wireAgingTest_Step;

extern unsigned short devChkErrOccure;

extern unsigned short stbyTorqOffControl_Req_Flag;

extern unsigned short stbyTorqOffControl_Flag;

extern unsigned short stbyTorqOffControlStep;

extern volatile unsigned short sSys100UsCnt;

extern volatile unsigned long lSys100UsTime;

extern volatile unsigned long lSys1MsTime;

extern hw_timer_t *timer;

extern volatile unsigned long lLastTime;

extern volatile unsigned long lCurrentTime;

extern volatile unsigned long iEncIntrIntervalTime;

#define ENC_INTV_BUFF_SIZE		256

typedef struct _encIsrDataBuff
{
	unsigned long tick;
	unsigned long width;
	unsigned short dir;	
	unsigned short enc;	
	unsigned short dist;
}encIsrDataBuff;

extern encIsrDataBuff stEncIntvBuff[ENC_INTV_BUFF_SIZE];

extern volatile unsigned short sEncBuffIdx;

extern volatile bool bIsrEncInterruptFlag;

extern volatile unsigned int iIsrEncInterruptCnt;

extern portMUX_TYPE mux;

extern double dLastDiff;

extern double dIntegral;

extern double dIsrOutDuty;

#define PID_EXEC_ENC_MIN_TIME_1MS	15

#define PID_EXEC_ENC_MIN_TIME_100US	(15*10)

extern unsigned int iWireDistance;

extern unsigned int iTargetDist;

extern bool bHookRandingCheckOnce;

extern unsigned short pidExecFlag;

extern unsigned short pidExecOutLog;

extern double Kp;

extern double Ki;

extern double Kd;

extern short sMotorSpeed;

extern int iIsrbbnEncCnt;

extern unsigned short sIsrEncBdata;

extern unsigned short sIsrBbnEncDir;

#define	MAX_SAVE_LEN	4096

extern int g_SavePtr;

extern int g_ProcPtr;

extern unsigned char g_readBuf[MAX_SAVE_LEN];

#define	DN_MAX_BUF	2

#define	DN_MAX_LEN	0x8000

#define	DN_RCV_PTR	0x1000

extern int g_DnExecFlag;

extern int g_DnExecStep;

extern int g_DnRecvNxt;

extern int g_DnRecvSeq;

extern unsigned short g_DnDataSum;

extern unsigned short g_DnCalcSum;

extern int g_DnRecvLen;

extern int g_DnRecvIdx;

extern int g_DnExecIdx;

extern int g_DnSaveIdx;

extern int g_DnSaveLen[DN_MAX_BUF];

extern unsigned char g_DnExecBuf[DN_MAX_LEN];

extern unsigned char g_DnSaveBuf[DN_MAX_BUF][DN_MAX_LEN];

extern unsigned char* g_DnRecvBuf;

extern volatile unsigned int g_idleCountCore0;

#define	MAX_IDLE_CORE	11893125

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
