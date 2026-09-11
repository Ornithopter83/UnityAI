// Rod shared state and low-level compatibility implementation.
#include <Arduino.h>
#include "Common.h"
#include "SwitchButton.h"
#include "Encoder.h"
#include "ENow.h"
#include "Potentiometer.h"
#include "EB_IMU.h"
#include "Battery.h"
#include "Version.h"

// Create Object
ENow eNow;
Encoder encoder;
SwitchButton sBtn;
Potentiometer brkMotor;
EB_IMU imu;
Battery battery;

// Define PID of CMD(Main-Slave)

// Define STX of CMD
#define STX_ENB_DIS     	"$01"
#define STX_DEV_INFO_REQ    "$01"	// Add New_ROD_Board
#define STX_BREAK_MOTOR 	"$05"
#define STX_SWITCH      	"$06"
#define STX_ROTARY      	"$07"
#define DEF_STX_IMU_SETUP   "$08"

String STX_IMU_SETUP =  	"$08";

	String STX_VERSION_READ = "$10";

	String STX_GET_ADDR   = "$16";
	String STX_SET_ADDR   = "$17";
	String STX_IMU_SETSTRING   = "$91";

// SetUp Cmd $90 [ USE SETUP Progam ] 
String STX_SETUP_ENTRY 	  = "$9001";		// Setup Entry
String STX_SETUP_EXIT 	  = "$9002";		// Setup Exit

String STX_IMU_INTVAL_CHK = "$9010";		// IMU INTERVAL LOG

String STX_SET_FILE	= "$F0";		// FILE SYSTEM Control

String STX_SET_DATA	= "$F6";		// ROD SET DATA

// Define RESP_STX
#define RESP_STX_IMU_CONN "$05"
#define RESP_STX_IMU_CONN_OK "$0511111111%"
#define RESP_STX_IMU_CONN_NG "$0500000000%"
#define RESP_STX_IMU_CONN_NG_SLEEP "$0500000009%"

#define RESP_STX_SLEEP_ENT "009"


// Define ENB/DIS
#define GAME_ENABLE     "11111111"
#define GAME_DISABLE    "00000000"

// 
#define RUN_TIMEOUT_ALIVE_CNT   10 		// 10 Sec
#define RUN_TIMEOUT_SLEEP_CNT   15 		// 15 Sec

unsigned short apType = AP_IS_DF;


//-------------------------------------------------------
//
unsigned long curr_ms_tick = 0;
unsigned long oldcurr_ms_tick = 0;

unsigned long curr_us_tick = 0;

// Test Delay Time
unsigned long chk_ms_tick = 0;			// 1ms Check Curr tick

//
unsigned int run_time_1ms = 0;
unsigned int run_time_10ms = 0;
unsigned int run_time_100ms = 0;
//unsigned int run_time_500ms = 0;
unsigned int run_time_1sec = 0;

unsigned int sys_1ms_cnt = 0;				// 1ms	// TBD
unsigned int sys_10ms_cnt = 0;				// 10ms
unsigned int sys_100ms_cnt = 0;				// 100ms
unsigned int sys_500ms_cnt = 0;				// 500ms
unsigned int sys_1sec_cnt = 0;				// Sys 1sec Counter

//unsigned int local_10secCnt = 0;


//
#define RUN_TIME_10MS_TICK		10		//
#define RUN_TIME_100MS_TICK		100		//
#define RUN_TIME_1SEC_TICK		1000		// 1 Sec = 1000ms

//-------------------------------------------------------

// === Out Port Sts 
unsigned short led1Grn_OutSts = 0;
unsigned short led2Red_OutSts = 0;

unsigned short btnLFLedRed_OutSts = BTN_LED_OFF;
unsigned short btnRTLedBlue_OutSts = BTN_LED_OFF;

//-------------------------------------------------------

// Rod PowerOn Flag
unsigned int rodPowerOnFlag = 0;		// Power On Once Only

// Send SLAVE_ALIVE at main ENB
int rod_Alive_recv_over_cnt = 0;				// Slave Alive 1sec Counter
int run_time_sleep_over_cnt = 0;

now_message now_msg;

//
bool isEnable = false;
bool isSetupMode = false;

//Define MAIN_STATUS
#define MAIN_STS_UNKNOWN	-1
#define MAIN_STS_CONN		0
#define MAIN_STS_DISCONN	1
#define MAIN_STS_PWRON		2	// PowerOn (?)


int mainStatus = UNKNOWN;
int oldmainStatus = UNKNOWN;
int mainPollingTOcnt = 0;

int rodSelfStatus = UNKNOWN;
int oldrodSelfStatus = UNKNOWN;
int rodPollingTOcnt = 0;

// IMU Connetion, Polling
#define IMU_UNKNOWN_2  -2
#define IMU_UNKNOWN  -1
#define IMU_CONN      0		// 0- ok = Conection
#define IMU_DISCONN   1		// not 0


int imuStatus = UNKNOWN;			// Default (1 = CONN )
int oldimuStatus = UNKNOWN;		// Not Send IMU_DIS at 1st PowerON
int imuPollingTOcnt = 0;

// define FLAG
#define IMU_CONN_TIMEOUT_CNT   5 // 5 // 3 // 5	// 5 Sec -> 6Sec

unsigned short fRecvImuData = 0;			// flag RECV IMU DATA, for IMU CONNECT
unsigned short flagIMUSTOP = 0;			// flag IMU STOP, for SETUP MODE

// SLAVE MODE
#define NORMAL_MODE 	0
#define SETUP_MODE  	1		// include !=0
int rodMode = NORMAL_MODE;


String L_BTN_UP = "$0600000001%";		// LEFT_UP
String L_BTN_DN = "$0600000010%";		// LEFT_DOWN
String R_BTN_UP = "$0600000100%";		// RIGHT_UP
String R_BTN_DN = "$0600001000%";		// RIGHT_DOWN

String ROD_ENC_CNT = "$0700000+";

#define TESTROD_START_STEP	9 // 10	// START STEP
unsigned short iocStep = TESTROD_START_STEP;			// START_STEP
unsigned long rod_ImuCycleCnt = 0;	// Log Out

// 
unsigned short recvInfoSeqStep = 0;	// nowSend Step


// IMU Interval Check & LOG OUT
unsigned short imuIntervalCheckCnt = 0;
unsigned long measureCnt = 0;


	extern const int rodBoardType = NEW_BOARD_2;		//NEW(1) FIX

extern const int buttonType = BUTTON_V2;
extern const int breakType = BREAK_NO;
extern const int battType = BATT_800_V2;
extern const int reelEncType = REEL_ENC_V3;

//---REGI ROD
unsigned int rodRegistMode = 0;		// 0- Normal, 1-Addr Setup

//-- VRT MOT 
unsigned int vrt_cont_flag = 0;
unsigned int vrt_ap_req_cnt = 6;
unsigned int vrt_ap_req_on_time = 100;
unsigned int vrt_ap_req_off_time = 50;
//-- BTN LED
#define BTN_LED_MAX_SIZE 2
unsigned int btn_led_flag[BTN_LED_MAX_SIZE] = {0,0};
unsigned int btn_led_ap_req_cnt[BTN_LED_MAX_SIZE] = {6,6};
unsigned int btn_led_ap_req_on_time[BTN_LED_MAX_SIZE] = {50,50};
unsigned int btn_led_ap_req_off_time[BTN_LED_MAX_SIZE] = {50,50};


//===============EXTERN=======================
extern uint8_t main_board_addr[];
extern uint8_t broad_cast_addr[];


//===========================================================
//
//	
//	Call 100 MS
void execRod_ImuOutCycle();
String zeroFill4Char(unsigned int _val);
void btnChangeCallback(String str);
void rotateChangeCallback(String str);
void imuDataCallback(String pid, String str);
void setData_RodCycleTest(String msg);
void sendInfo_boardType();
void imuConnResp();
void nowRecvHandler();
void anaReelDeviceControl(int act, String msg);
void anaVrtMotMainCmd(String msg);
void anaBtnLedMainCmd(String msg);
void rodVrtControl_Start(int cnt, unsigned int ontime, unsigned int offtime);
void rodVrtControl_Stop();
void rodVrtControl();
void rodBtnLedControl_Start(int idx, unsigned int cnt, unsigned int ontime, unsigned int offtime);
void rodBtnLedControl_Stop(int idx);
void rodLfBtnLedControl();
void rodRtBtnLedControl();
void uartPcHandler_NormalMode();
int targetAddrWrite(String msg);
void uartPcHandler_Setup();
void setRecvImuData();
int isRodNewBoard();
int setBoardType();
int getButtonType();
int getBreakType();
int getEncType();
int getBattType();
void settingIOtype();
void setSlaveMode();
int readSlaveMode();
int isSlaveSetupMode();
void Entry_SetupMode();
void Exit_SetupMode();
void ESP_Soft_Reset();
void battChangeCheckSend();
void rodAliveRecvTOCheck();
void sleepEnteranceCheck();
void imuConnectCheck();
void t1ms_Process();
void t10ms_Process();
void sendRodInfo_forStep();
void rodRegistToStop();
void rodRegistToStart();
void rodRegi3secToStart();
void rodRegistToControl();
void rodRegistCancel();
void t100ms_Process();
void t500ms_Process();
void t1sec_Process();
void rodLed1SecControl();
void run_timer();
void ioPinSetting();
void checkFileSystem();
void DF_Rod_Application_Setup();
void DF_Rod_Application_Loop();
void execRod_ImuOutCycle()
{

	static unsigned short oldiocStep = 0;
	static unsigned long _to;
	static unsigned int _rptCnt=0;
	static unsigned int _imuCnt=0;
	static unsigned int _encCnt=0;
	String sndStr;
	
	
	switch(iocStep)
	{
		case 0:		// IDLE
			break;
			
		case TESTROD_START_STEP:	// START, R BTN
			led2Red_OutSts = HIGH;
			digitalWrite(LED2_RED_PIN, led2Red_OutSts);
			rod_ImuCycleCnt++;
			if(1) { LogPrintln(" LG] RODCYC, " + String(rod_ImuCycleCnt) + " cnt"); }
			_rptCnt = 0;
			_imuCnt = 0;
			iocStep = 10;
			break;
			
		//case TESTROD_START_STEP:	// START, R BTN
		case 10:	// START, R BTN
			_rptCnt++;
			if((3000/500) < _rptCnt)
			//if((1000/500) < _rptCnt)
			{
				led2Red_OutSts = LOW;
				digitalWrite(LED2_RED_PIN, led2Red_OutSts);
				_rptCnt = 0;
				iocStep = 20;	// Next STEP
			}
			else
			{
				if(_rptCnt % 2) btnChangeCallback(R_BTN_UP);		// R Button
				else			btnChangeCallback(R_BTN_DN);		// R Button
				setTO(_to);
				iocStep = 11;	
			}
			break;
		case 11:
			if(checkTO(_to, 500))
			//if(checkTO(_to, 200))
			{
				iocStep = 10;					
			}
			break;

		case 20:			// IMU 10 Sec ( ON - 5sec + OFF - 5 sec )
			//Set IMU OUT
			imu.isMeasure =  true;		// IMU ON
			setTO(_to);			
			iocStep = 21;					
			break;
		case 21:
			if(checkTO(_to, 67*100))	// 6.7 SEC
			//if(checkTO(_to, 200))
			{
				imu.isMeasure =  false;		// IMU OFF
				setTO(_to); 		
				iocStep = 22;					
			}
			break;
		case 22:
			if(checkTO(_to, 33*100))	// 3.3 SEC
			//if(checkTO(_to, 10*20))	//
			{
				imu.isMeasure = false;
				_rptCnt = 0;
				_imuCnt++;
				iocStep = 30;	// default
				if(0) {}
				else if(1 == _imuCnt) { iocStep = 30; }	// L BTN				
				else if(2 == _imuCnt) { iocStep = 40; } 	// Hit_Encoder		
				else if(3 == _imuCnt) { iocStep = 40; } 	// Encoder		
				else if(4 == _imuCnt) { iocStep = 40; } 	// Encoder		
				else if(5 == _imuCnt) { iocStep = 40; } 	// Encoder		
				else if(6 == _imuCnt) { iocStep = 40; }		// Encoder		
				else if(7 == _imuCnt) { iocStep = 10; }	// R BTN		
				else if(7 < _imuCnt) { iocStep = 50; }	// END
				//LogPrintln(" LG] _step 10 sec");
			}
			break;

		case 30:	// L BTN
			_rptCnt++;
			if((3000/500) < _rptCnt)
			//if((1000/500) < _rptCnt)
			{
				_rptCnt = 0;
				iocStep = 20;	// Next IMU
			}
			else
			{
				if(_rptCnt % 2) btnChangeCallback(L_BTN_UP);		// R Button
				else			btnChangeCallback(L_BTN_DN);		// R Button
				setTO(_to);
				iocStep = 31;	
			}
			break;
		case 31:
			if(checkTO(_to, 500))
			{
				iocStep = 30;					
			}
			break;

		case 40:	// Encoder
			_rptCnt++;
			if((3000/500) < _rptCnt)		// 3sec
			//if((1000/500) < _rptCnt)		// 3sec
			{
				_rptCnt = 0;
				iocStep = 20;	// Next STEP
			}
			else
			{ 
				_encCnt = encoder.count;
				_encCnt++;
				if(20 < _encCnt) { _encCnt = 0; }
				encoder.count = _encCnt;
				sndStr = ROD_ENC_CNT + encoder.fillZero2char(_encCnt) + "%";
				rotateChangeCallback(sndStr);		// Enc Count UP
				setTO(_to);
				iocStep = 41;	
			}
			break;
		case 41:
			if(checkTO(_to, 500))
			{
				iocStep = 40;					
			}
			break;

			
		case 50:
			// CLEAR VAR
			_imuCnt = 0;
			_rptCnt = 0;
			iocStep = 9;	// Start
			break;
			
		case 60:
			break;

		case 90:
			break;
			
		default:
			break;
	}
}



String zeroFill4Char(unsigned int _val)
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

//== Button, Encoder, Imu Call back
/*  =====================================
      Sensor & Module Status Callback
      ; Send to MAIN by eNow
  ===================================== */
