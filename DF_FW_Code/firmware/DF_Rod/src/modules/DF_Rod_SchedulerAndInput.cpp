// Rod scheduler and input implementation.
#include "../DF_Rod_Internal.h"
#include "DF_Rod_SchedulerAndInput.h"
void t100ms_Process()
{

	// Power On Process
	if(rodPowerOnFlag)
	{
		rodPowerOnFlag = 0;	// CLR , Exec Once Only
		sendInfo_boardType();
	}
	
	//=== INFO REQ STEP
	if(recvInfoSeqStep)	// Req SEQ (not 0, Step Process)
	{
		sendRodInfo_forStep();
	}
	//===

// == IMU AGING TEST( IMU data출력)

//== ROD등록 "L"길게 누름 CHECK
	#define L_BTN_LONG_TIME	2500	// 2.5초
//== ROD L버튼 누른 시간 Check
	if(btn_L_DownCnt)
	{
		btn_L_DownCnt++;
		if((L_BTN_LONG_TIME/100) < btn_L_DownCnt)	// 2.5초 Over
		{
			
			rodRegistMode = 55;		// ROD REGIST ENTRY

			btn_L_DownCnt = 0;
			

			// 2) BC로 주소 송부
			eNow.addPeerAddress(broad_cast_addr);

			String rodAddr = eNow.getMyAddress();
			//eNow.write(STR_PID_CTRL_NUM, "$0600000016%");		// L버튼 LONG DOWN
			eNow.writeBC(DF_Protocol_RodToMain_RodAddress, rodAddr); 	// L버튼 LONG DOWN , ROD=>MAIN

			// TBD_, 메인에서 명령이 안오는 경우는 CANCEL처리 필요.(20초?)
			rodRegistToStart();
			
			// LED Blink SET
			AddrWrite_LedBlink = 1;	// LED Blink Set

			// TBD, Peer DisConnection?

			//----
		}
	}

//== ROD등록 LED점멸 Control
	#define ROD_REGI_LED_BLINK_TIME	1000	// 1 SEC
	// led Blink
	if(AddrWrite_LedBlink)
	{
		led1Grn_OutSts = !led1Grn_OutSts;
		digitalWrite(LED1_GRN_PIN, led1Grn_OutSts); 	// NORMAL : Green(스위치부) LED Blink 1SEC
		AddrWrite_LedBlink++;
 		if((ROD_REGI_LED_BLINK_TIME/100) < AddrWrite_LedBlink)
		{
			// 1) STOP BLINK
			AddrWrite_LedBlink = 0;
		}		// BLINK STOP (1초간만 Blinking)

	}

//== ROD등록 T/O Control
	if(rodRegistMode)
	{
		rodRegistToControl();
	}
	
}

/*---------------------------------------------------------
//
----------------------------------------------------------*/
void t500ms_Process()
{

	// 1) AD Read, Batt Level
	//unsigned bat_chk_us = micros();
	
	battery.scanAvgInteg();

	//LogPrintlnus(" LG] BatRead Time us:" + String(micros()-bat_chk_us));

}

/*---------------------------------------------------------
//
----------------------------------------------------------*/
void t1sec_Process()
{
	// 1 - Local 1sec Count
  /*
	local_10secCnt++;
	if(9 < local_10secCnt) { local_10secCnt = 0; }		// Count  1 ~ 10
  */

	// 2) ROD_ALIVE_CHK Recved Check
	rodAliveRecvTOCheck();


	// 4) IMU Connection Polling
	imuConnectCheck();

	// 5) Send BAT LEVEL (each 1 SEC)
	battChangeCheckSend();

	// 6) LED Display OUT
	rodLed1SecControl();

}

//-----------------------------------------------------
//  낚시대 LED 제어
//	Call 1 SEC
void rodLed1SecControl()
{
	if(rodAliveRcv_LedBlinkFlag)
	{
		if(!AddrWrite_LedBlink)
		{
			led1Grn_OutSts = !led1Grn_OutSts;
			digitalWrite(LED1_GRN_PIN, led1Grn_OutSts); 	// NORMAL : Green(스위치부) LED Blink 1SEC
		}
		led2Red_OutSts = !led2Red_OutSts;
		digitalWrite(LED2_RED_PIN, led2Red_OutSts); 	// NORMAL : Red(CPU보드부) LED Blink 1SEC
	}
	else
	{
		if(!AddrWrite_LedBlink)
		{
			led1Grn_OutSts = HIGH;
			digitalWrite(LED1_GRN_PIN, led1Grn_OutSts); 	// MAIN Not OP :  GREEN ON
		}
		led2Red_OutSts = LOW;
		digitalWrite(LED2_RED_PIN, led2Red_OutSts);	// MAIN Not OP :  BLUE OFF
	}
}

/*
=====================================
              Running Timer
=====================================
 */
unsigned int system_delay_cnt = 0;

void run_timer()
{
	unsigned int add_tick = 0;

	curr_ms_tick = millis();
	if(oldcurr_ms_tick != curr_ms_tick)
	{
		// TBD System Run TImer & System Count(1ms,10ms,100ms,1sec )
		if(curr_ms_tick > oldcurr_ms_tick)
		{
			add_tick = (int)(curr_ms_tick - oldcurr_ms_tick);
		}
		else
		{
			add_tick = (int)((0xFFFFFFFF - oldcurr_ms_tick) + 1 + curr_ms_tick);
		}

		sys_1ms_cnt += add_tick;		// system 1ms ++

		run_time_1ms += add_tick;
		
gotoSystemTimer:
	
		if( 9 < run_time_1ms)	// 10ms
		{
			//run_time_1ms = (run_time_1ms - 10);
			run_time_1ms -= 10;
			
			sys_10ms_cnt++;

			run_time_10ms++;
			if( 9 < run_time_10ms)	// 100ms
			{
				run_time_10ms -= 10;
				
				sys_100ms_cnt++;

				run_time_100ms++;

				if(!(run_time_100ms % 5))		// 500ms
				{
					sys_500ms_cnt++;
				}

				if( 9 < run_time_100ms)	// 1000ms
				{
					run_time_100ms -= 10;
					
					sys_1sec_cnt++;		// 1 SEC
					// run_time_1sec++;
				}
				
			}
		}

		if(9 < run_time_1ms)	// 
		{
			goto gotoSystemTimer;
		}
		
		//if(10 < add_tick)
		//if(5 < add_tick)
		if(2 < add_tick)
		{
			// TBD
			// error = sys delay = not good performance !!!
			system_delay_cnt++;
			LogPrintln(" LG] SYSTIK ROD Delay " + String(add_tick) + "ms ,cnt:" + String(system_delay_cnt));
		}

		oldcurr_ms_tick = curr_ms_tick;

	}

}

//------------------------------------------------------------------
//
//
//------------------------------------------------------------------
void ioPinSetting()
{

	//== SET Pin Mode (INPUT)

	//--Setup Mode Switch
	pinMode(SETUP_MODE_PIN, INPUT);

	//=== SET Pin Mode ( OUT )
	// BRD LED
	pinMode(LED1_GRN_PIN, OUTPUT);
	pinMode(LED2_RED_PIN, OUTPUT);

	// BTN LED
	pinMode(BTN_LED_LF_RED_PIN, OUTPUT);
	pinMode(BTN_LED_RT_BLUE_PIN, OUTPUT);

	pinMode(VRT_MOT_ON_PIN, OUTPUT);
	digitalWrite(VRT_MOT_ON_PIN, VRT_MOT_OFF);

}

//---------------------------------------------------------------
// File System Mount Check
//---------------------------------------------------------------
void checkFileSystem()
{
	// FILE System INIT with FORMAT & Reset
	if (!SPIFFS.begin())		// without FORMAT
	{
			LogPrintln(" LG] FS Format Start");
		
    	SPIFFS.format();	// About 30sec
		
		
		//delay(2000);
		delay(10);		// Format후 Reset 안정화(?) 시간
		
    	//ESP.restart();
		ESP_Soft_Reset();
	}

	// OK시만 처리됨
}


/*------------------------------------------------------------------
   SETUP Arduino
------------------------------------------------------------------*/
static int pwrOn1st_loop = 0;
int resp_imu = 0;
/*------------------------------------------------------------------------
  Arduino Loop
------------------------------------------------------------------------*/
