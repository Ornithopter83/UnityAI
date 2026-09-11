// Rod power and IMU supervision implementation.
#include "../DF_Rod_Internal.h"
#include "DF_Rod_PowerAndImu.h"
void rodAliveRecvTOCheck()
{
	// CLEAR at Main Alive Recved
	// Main에서 ROD_ALIVE(10)을 수신하면 CNT를 CLR함
	// MAIN에서 연속 10초간 안오는 경우, 낚시대 LED 점멸 정지 
    if (RUN_TIMEOUT_ALIVE_CNT < rod_Alive_recv_over_cnt)		// Over 10 SEC ( 10 * 1sec)
	{
		// COUNT CLR : Recved From MAIN [ALive Check] 1 SEC
		rodAliveRcv_LedBlinkFlag = 0;		// GREEN LED BLINK 1 sec
		mainStatus = MAIN_STS_DISCONN;
    }
	else
	{
		// 2 - rin time Check
  		rod_Alive_recv_over_cnt++;
	}
}

//----------------------------------------------------------------
//
//
//	CALL 1SEC
void sleepEnteranceCheck()
{
		//1 ) Slave Sleep Time Check
	if (RUN_TIMEOUT_SLEEP_CNT < run_time_sleep_over_cnt )	// Over 7 Sec ?
	{
	  /*
		eNow.write(DF_Protocol_RodToMain_Sleep, RESP_STX_SLEEP_ENT);		// Send to MAIN , Slave Sleep EnNTERANCE
		//if (isRodNewBoard())
    	if(BREAK_NO == getBreakType())	// NEW ROD
		{
			// N.A
		}
		else
		{
			brkMotor.setValue(0);
			digitalWrite(DC24VON_PIN, LOW);			// 24V_DC OFF
		}
		delay(100);
		esp_deep_sleep_start();
	  */
	}

}

/*---------------------------------------------------------
//
	Call 1sec
-----------------------------------------------------------*/
void imuConnectCheck()
{
	String respMsg;

	//respMsg = RESP_STX_IMU_CONN_OK;
	//respMsg = RESP_STX_IMU_CONN_OK;
	if(fRecvImuData)		// IMU Data RECVED?
	{
		fRecvImuData = 0;		// CLEAR FLAG of IMU RECV DATA
		imuPollingTOcnt = 0;			// Counter CLR
		//respMsg = RESP_STX_IMU_CONN_OK;
		respMsg = "11";
		imuStatus = IMU_CONN;
	}
	else			// NOT RECV?
	{
		imuPollingTOcnt++;
		if( IMU_CONN_TIMEOUT_CNT < imuPollingTOcnt) // Over ( 6 sec )?
		{
			imuPollingTOcnt = IMU_CONN_TIMEOUT_CNT+1;
			//respMsg = RESP_STX_IMU_CONN_NG;
			respMsg = "00";
			imuStatus = IMU_DISCONN;
		}
	}
	
	// Send Msg IMU CONN, Only Change
	if(oldimuStatus != imuStatus)
	{
		eNow.write(DF_Protocol_RodToMain_ImuConnection, respMsg);		// Send to MAIN
			LogPrintln(" LG] IMUCON " + respMsg + " o,c:" + String(oldimuStatus)+","+String(imuStatus));		// LOG
		
		oldimuStatus = imuStatus;
	}

}


/*---------------------------------------------------------
----------------------------------------------------------*/
