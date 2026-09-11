
#pragma once



// Shared low-level state and compatibility API. Feature-owned declarations live in module headers.



#include <Arduino.h>

#include "Common.h"

#include "SwitchButton.h"

#include "Encoder.h"

#include "ENow.h"

#include "Potentiometer.h"

#include "EB_IMU.h"

#include "Battery.h"

#include "Version.h"

extern ENow eNow;

extern Encoder encoder;

extern SwitchButton sBtn;

extern Potentiometer brkMotor;

extern EB_IMU imu;

extern Battery battery;

#define STX_ENB_DIS     	"$01"

#define STX_DEV_INFO_REQ    "$01"

#define STX_BREAK_MOTOR 	"$05"

#define STX_SWITCH      	"$06"

#define STX_ROTARY      	"$07"

#define DEF_STX_IMU_SETUP   "$08"

extern String STX_IMU_SETUP;

extern String STX_VERSION_READ;

extern String STX_GET_ADDR;

extern String STX_SET_ADDR;

extern String STX_IMU_SETSTRING;

extern String STX_SETUP_ENTRY;

extern String STX_SETUP_EXIT;

extern String STX_IMU_INTVAL_CHK;

extern String STX_SET_FILE;

extern String STX_SET_DATA;

#define RESP_STX_IMU_CONN "$05"

#define RESP_STX_IMU_CONN_OK "$0511111111%"

#define RESP_STX_IMU_CONN_NG "$0500000000%"

#define RESP_STX_IMU_CONN_NG_SLEEP "$0500000009%"

#define RESP_STX_SLEEP_ENT "009"

#define GAME_ENABLE     "11111111"

#define GAME_DISABLE    "00000000"

#define RUN_TIMEOUT_ALIVE_CNT   10

#define RUN_TIMEOUT_SLEEP_CNT   15

extern unsigned short apType;

extern unsigned long curr_ms_tick;

extern unsigned long oldcurr_ms_tick;

extern unsigned long curr_us_tick;

extern unsigned long chk_ms_tick;

extern unsigned int run_time_1ms;

extern unsigned int run_time_10ms;

extern unsigned int run_time_100ms;

extern unsigned int run_time_1sec;

extern unsigned int sys_1ms_cnt;

extern unsigned int sys_10ms_cnt;

extern unsigned int sys_100ms_cnt;

extern unsigned int sys_500ms_cnt;

extern unsigned int sys_1sec_cnt;

#define RUN_TIME_10MS_TICK		10

#define RUN_TIME_100MS_TICK		100

#define RUN_TIME_1SEC_TICK		1000

extern unsigned short led1Grn_OutSts;

extern unsigned short led2Red_OutSts;

extern unsigned short btnLFLedRed_OutSts;

extern unsigned short btnRTLedBlue_OutSts;

extern unsigned int rodPowerOnFlag;

extern int rod_Alive_recv_over_cnt;

extern int run_time_sleep_over_cnt;

extern now_message now_msg;

extern bool isEnable;

extern bool isSetupMode;

#define MAIN_STS_UNKNOWN	-1

#define MAIN_STS_CONN		0

#define MAIN_STS_DISCONN	1

#define MAIN_STS_PWRON		2

extern int mainStatus;

extern int oldmainStatus;

extern int mainPollingTOcnt;

extern int rodSelfStatus;

extern int oldrodSelfStatus;

extern int rodPollingTOcnt;

#define IMU_UNKNOWN_2  -2

#define IMU_UNKNOWN  -1

#define IMU_CONN      0

#define IMU_DISCONN   1

extern int imuStatus;

extern int oldimuStatus;

extern int imuPollingTOcnt;

#define IMU_CONN_TIMEOUT_CNT   5

extern unsigned short fRecvImuData;

extern unsigned short flagIMUSTOP;

#define NORMAL_MODE 	0

#define SETUP_MODE  	1

extern int rodMode;

extern String L_BTN_UP;

extern String L_BTN_DN;

extern String R_BTN_UP;

extern String R_BTN_DN;

extern String ROD_ENC_CNT;

#define TESTROD_START_STEP	9

extern unsigned short iocStep;

extern unsigned long rod_ImuCycleCnt;

extern unsigned short recvInfoSeqStep;

extern unsigned short imuIntervalCheckCnt;

extern unsigned long measureCnt;

extern const int rodBoardType;

extern const int buttonType;

extern const int breakType;

extern const int battType;

extern const int reelEncType;

extern unsigned int rodRegistMode;

extern unsigned int vrt_cont_flag;

extern unsigned int vrt_ap_req_cnt;

extern unsigned int vrt_ap_req_on_time;

extern unsigned int vrt_ap_req_off_time;

#ifndef BTN_LED_MAX_SIZE
#define BTN_LED_MAX_SIZE 2
#endif

extern unsigned int btn_led_flag[BTN_LED_MAX_SIZE];

extern unsigned int btn_led_ap_req_cnt[BTN_LED_MAX_SIZE];

extern unsigned int btn_led_ap_req_on_time[BTN_LED_MAX_SIZE];

extern unsigned int btn_led_ap_req_off_time[BTN_LED_MAX_SIZE];

extern uint8_t main_board_addr[];

extern uint8_t broad_cast_addr[];

void execRod_ImuOutCycle();

String zeroFill4Char(unsigned int _val);
