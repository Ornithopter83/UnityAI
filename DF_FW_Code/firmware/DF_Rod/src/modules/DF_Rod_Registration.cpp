// Rod registration implementation.
#include "../DF_Rod_Internal.h"
#include "DF_Rod_Registration.h"
void t1ms_Process()
{
	// 1) AD Read

}


/*---------------------------------------------------------
----------------------------------------------------------*/
void t10ms_Process()
{
	if(vrt_cont_flag) { rodVrtControl(); }			// VRT MOT 
	if(btn_led_flag[0])	{ rodLfBtnLedControl();	}	// 0 = LEFT BTN LED
	if(btn_led_flag[1])	{ rodRtBtnLedControl();	}	// 1 = RIGHT BTN LED
	
}

//--------------------------------------------------
//	1) ALIVE SEND WAIT ( 100ms )
//	2) Send IMU CONN INFO (100ms)
//	3) Send BATT LEVEL (100ms)
//	4) Send ROD BOARD TYPE (100ms)
//
//	CALL for 100MS
void sendRodInfo_forStep()
{


	switch(recvInfoSeqStep)
	{
		//case 0:		// 0인경우 들어오지 않음
		//	break;
		
		case 1:			// Wait ALIVE SEND
			recvInfoSeqStep ++;
			break;
		
		case 2:		// IMU SEND
			recvInfoSeqStep++;
			imuConnResp();
				LogPrintln(" LG] IMUCON o,c:" + String(oldimuStatus)+","+String(imuStatus));
				//LogPrintlnus(" LG] IMUCON odl->curr: " + String(oldimuStatus)+","+String(imuStatus));
			break;

		case 3:		// Send BATT LVL
			recvInfoSeqStep++;
			
			eNow.write(DF_Protocol_RodToMain_Battery, battery.restLvlStr);
				LogPrintln(" LG] BATLVL o,c:" + String(battery.rest_lvl)+","+String(battery.old_restLvl));
				//LogPrintlnus(" LG] BATLVL odl->curr: " + String(battery.rest_lvl)+","+String(battery.old_restLvl));
			break;
			
		case 4:		// Send TYPE
			recvInfoSeqStep++;
			sendInfo_boardType();
			break;
			
		default:
			recvInfoSeqStep = 0;	// END
			break;
	}

}

//---------------------------------------------
// 낚시대(REEL) 교체 CONTROL
//---------------------------------------------

static unsigned int rodRegistToStep = 0;	// TO control Step

//TO정지
void rodRegistToStop()
{
	rodRegistToStep = 0;	// Start
}

// TO 확인 개시
void rodRegistToStart()
{
	rodRegistToStep = 10;	// 10SEC T/O Start	
}

void rodRegi3secToStart()
{
	rodRegistToStep = 30;	// 3SEC T/O  Start
}


#define ROD_REGI_CANCEL_TO	(15*1000)	// 15 SEC
#define ROD_REGI_END_TO		(3500)	// 3.5 SEC
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
			}
			break;

		case 30:	//Start
			rodRegistToStep = 31;
			setTO(_to);
			break;
		case 31:
			if(checkTO(_to, ROD_REGI_END_TO))
			{
				rodRegistCancel();		// MODE CLR
				rodRegistToStep = 0;		// to IDLE
				//ESP.restart();
				ESP_Soft_Reset();
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


// 버튼 Check Counter
// L버튼 3초 이상 누른 경우, BroadCast로 보냄.
//
unsigned int btn_L_DownCnt = 0;
unsigned int AddrWrite_LedBlink = 0;


/*---------------------------------------------------------
//
----------------------------------------------------------*/
