//
//		Common Header
//
#pragma once

#include <DFProtocol.h>

//================================================= 
//===  [1] DEFINE CONFIG ===

//=== 1-1) IO CONFIG & SWITCHING


//=== 1-2) IO ENB ======
#define IO_IMU	(1)

//=== Board Type Define

//=== 1-3) DEFINE FUNCTION ENB =================
#define BAT_KAL_FILTER		(0)			// 24/7/10 Delete Kal Filter
	#define BAT_AD_AVG_INTEG	(0)			// 0-AVG, 1-INTERGRATION

#define NEW_IF		(1)			// IF K20

#define SLEEP_SET	(0)
#define TEST_ROD_POWER_CONSUMPTION_TEST (0)		// TEST_FW

#define ROD_BDTYPE_DETECT	(0)		// REEL BOARD판별 안함. FIX(V2=NEW)

#define TEST_VRT_MOT	(0)			//REL=0, TEST ROM (xx.xx.xx.60)

//================================================= 
// === [2]  PIN DEFINE ====

// IMU PIN Number
#define IMU_RX  			1
#define IMU_TX  			2

//Button PIN
#define BTN_RED_PIN			5			// BT1 = RIGHT Button

#define BTN_BLUE_PIN_NEW	4		// NEW ROD, BT2 = LEFT Button

#define BTN_LED_LF_RED_PIN		8		// SPARE_1 BUTTON
#define BTN_LED_RT_BLUE_PIN		9		// SPARE_12BUTTON


// MAIN Pin Number
#define LED1_GRN_PIN  		6 		//GREEN LED ( Switch 보드)
#define LED2_RED_PIN  		7 		//RED LED( Rod Main보드)

// Potentio, Break motor Control SPI
#define SPI_SS    			10
#define SPI_SCK   			11
#define SPI_MOSI  			12
//--- NEW2보드
#define VRT_MOT_ON_PIN		12		//

// Batt PIN
#define BATTERY_LVL_PIN		14

// DC24V On/Off for Break Motor
#define DC24VON_PIN  		15			// DC24V On/Off at Sleep Mode

// Magnetic Hall Encoder Sensor
#define ENC_HALL_SEN_PIN	18	// Magnetic Hall Sensor Pin 

#define PWM_PIN   			38		// PWM, NOT USE

// Board Type Detect PIN
#define BOARD_TYPE_PIN		39		// Low-NEW, HIGH-old

#define DEBUG_TX_PIN		43		// LOG_TX : TXD_0 (Vr1.0.1.0)
#define DEBUG_RX_PIN		44		// LOG_RX : RXD_0 (Vr1.0.1.0)
#define SETUP_MODE_PIN  	48	// Low:Normal Mode, high:Address SetUp Mode
#define BOOT_MODE_PIN		0	//

//======= ACTIVE DEFINE======
//-- button LED on/off : NOT ACTIVE
#define BTN_LED_ON		LOW
#define BTN_LED_OFF		HIGH
//--- VRT MOT ON/OFF
#define VRT_MOT_ON		HIGH
#define VRT_MOT_OFF		LOW



//=== Define IO Type=======================
//--- Board Type
// ROD BOARD TYPE
#define NEW_BOARD_2		1
#define BREAK_NO 	1	// no Break Motor
#define BUTTON_V2	1	// BOARD V2 : GPIO5,4
#define REEL_ENC_V3	2	// BOARD V3 : 1 SIGNAL - Magnetic Hall Sensor	(Vr1.0.1.0)
#define BATT_800_V2		1	// BOARD V2 : 1 SIGNAL - Magnetic Hall Sensor

//----ROD REGI
//----------------------------------

//=========2) STX/RESP DEFINE=================================================================
//----- RCV PID (INT)
#define PID_NOT_RECVED		0		// 미사용 ID, Rcv Check용도

//---- SEND PID(STRING)


// Define PID of CMD
#define IMU_SETUP     11


#define PID_IMU_VERSION   20

//----IMU CMD _ DATA DEfine
// 신규 SETUP프로그램 (V3.0)
#define IMU_DATA_OFF 		0
#define IMU_DATA_ON			1
//----------------------------------
#define IMU_SET_INTEVAL		2
#define IMU_SET_OUT			3
#define IMU_SET_CALBRATION	4

//============================================
//	DEFINE AP TYPE
#define AP_IS_DF	0	// DF - 게임프로그램
#define AP_IS_TM	1	// TM - 테스트 프로그램


//==========================================
//=== DEFINE 2. Macro

//--- PC SERIAL Define Macro : SERIAL _0 : USB CDC 
#define RespPrintln Serial.println
#define StsPrintln  Serial.println

#define LogPrint Serial.print
#define LogPrintln Serial.printf("%10d",curr_ms_tick);Serial.println

//--- IMU SERIAL Macro : SERIAL_1 : UART_1(?) :  ---
#define imuSerial Serial1
#define send2Imuln Serial1.println
//--- DEBUG SERIAL Macro : SERIAL_2 : DEBUG :  --- (Vr1.0.1.0)
#define DbgSerial Serial2
#define DbgPrintf Serial2.printf("%10d",curr_ms_tick);Serial2.printf
#define DbgPrintln Serial2.printf("%10d",curr_ms_tick);Serial2.println
#define send2DBGln Serial2.println
//--------------------------------------------------
// Define Macro
#define setTO(var) (var = curr_ms_tick)				// a = old time;
#define checkTO(var,TO) ((curr_ms_tick - var) > TO)	// if ( (curr - old) > TO )

//------------------------------------------
//	IMU 인터벌  [ms]
//------------------------------------------
#define IMU_INTV_MIN_TIME	20	//10ms
#define IMU_INTV_MAX_TIME	1000	//1000 ms
#define IMU_INTV_DEFAULT_TIME	100	//100 ms



//==========================================
//===  3.  DEFINE COMMON Status
#define UNKNOWN		(-1)
#define CONNECTED	(0)
#define DISCONN		(1)
#define CONNELSE	(2)
//------
#define STS_OK		(0)
#define STS_NG		(1)

//====================================
//	4. Enable Funtion Define
#define FUNC_EQ_TARGET_ADDR		(1)		// 타켓주소와 다른 수신 Now MSG는 버림
#define FUNC_NOW_DIS_AT_SETUP	(1)		// SETUP시 NOW수신 CB처리 안함
#define FUNC_SLEEP_ENTRY		(0)		// SLEEP지원 안함

//==========================================
//===  4. LOG DEFINE
#define LOG_PWRON_TIME	(0)

#define LOG_BTN			(0)
#define LOG_BAT			(0)

#define LOG_FUNC_STEP	(0)

#define LOG_CONN		(1) //(1)		//관련Device연결시 LOG출력

#define LOG_MAIN_RECV	(0) //(1)		// REL(0), Recv now ID OUT for DEBUG, 

#define LOG_ALIVE_CHK_CNT	(0) //(0)	// Alive수신 Chk및 수신시 LOG(CNT) 출력

#define LOG_POWER_ON_VER	(1)
#define LOG_POWER_ON_MODE	(0)

#define LOG_IMU_CONFIG	(0)

#define LOG_TARGET_LOAD		(0)		// test
#define LOG_BRK_MOT			(0)		// for Debug

#define LOG_BATT_INIT_STR	(0) //(0)		// For Debug, Batt_Init BATT STR Log Out

#define LOG_MAIN_PWRON		(1)		// For Debug, Main Power On CMD RECV Log Out
#define LOG_MAIN_IMU_ONOFF	(0)		// For Debug, IMU Data ON/OFF Log Out

#define LOG_BOARD_TYPE		(0)		// PowerOn Board Type Log Out
#define LOG_IO_TYPE			(0)		// Power On, IO Type Log Out

#define LOG_IMU_RECV_DATA_OUT	(0)	// REL(0) 

#define LOG_ADDR_NG		(1)			// For Debug, Log Out At Addr Length LessThen 17

#define LOG_FS_FORMAT_START	(1)			// for Debug
#define LOG_FS_FORMAT	(0)			// for Debug
#define LOG_SPIFFS_OK	(0)			// For Debug, Power ON SPI FFS OK Log Out

#define LOG_SYSTICK_DELAY	(1)			//System Run Time Delay LOG
#define LOG_1MS_EXE_DELAY	(1)			// 1ms EXEC Delay

// NOW SEND/SENT CHECK
#define LOG_NOW_SEND_ERR		(1)	//(1)		// Send NG Error 
#define LOG_NOW_SEND_SEQNO		(0)			// Send Result
#define LOG_NOW_CB_SENT_FAIL	(1)
#define LOG_NOW_CB_SENT_REGI	(1)	//(1)		// SENT CB LOG

// now Recv Call back
#define LOG_NOW_RCV_OVER_WRITE	(1)		// Now RCV msg OverWrite여부 출력 LOG
#define LOG_NOW_RCV_CB_US_CHK	(0)	//(1)	// NOW수신 처리 시간 확인 LOG
#define LOG_NOW_CB_RCV_MSG		(0)
#define LOG_NOW_NOT_DEF_CMD		(1)		// NOW수신 미정의 CMD(Pid,msg)

//--- system Timer us Measure
#define LOG_SYS_US_CHK	(0)

// NOW정보 로그 출력 , at Power ON
#define LOG_NOW_GET_INFO	(0) //(1)		// log Now Info

// REEL REGIST
#define LOG_BTN_LONG			(0)			// for Debug
#define LOG_MAIN_ADDR_WRITE		(0)		//
#define LOG_ROD_REGI_TO_STEP	(0)	// LOG
#define LOG_OTHER_ROD_NOT_EXEC	(1)		// Other ROD NOT EXEC

#define LOG_RCV_BREAK_EXE		(0)	//(1)			// BREAK RCV LOG출력

// VRT MOT & BTN LED
#define LOG_VRT_MOT_MAIN_CMD_ERR	(1)
#define LOG_VRT_MOT_MAIN_CMD		(0)
#define LOG_BTN_LED_MAIN_CMD		(0)
#define LOG_BTN_LED_MAIN_CMD_ERR	(1)

//
#define LOG_IMU_OUT_MAIN_CMD_ERR	(1)		// IMU OUT CMD ERROR, LOG OUT
//
#define LOG_OUT_OFF	(0) //(1)		//(REL=0) Reel OFF Main CMF LOG OUT for Debug

#define LOG_IMU_AUTOSET_START	(1)		//
#define LOG_IMU_AUTOSET			(0)		//

//==========================================
//==== DEFINE 5. EXTERN FUNCTION

//==========================================
//==== DEFINE 6. EXTERN VAR
extern unsigned long curr_ms_tick;
extern unsigned long curr_us_tick;




