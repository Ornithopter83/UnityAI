// Rod setup protocol implementation.
#include "../DF_Rod_Internal.h"
#include "DF_Rod_SetupProtocol.h"
void uartPcHandler_NormalMode()
{

  if (Serial.available())	// if Recv Recv Length
  {
    String msg = Serial.readStringUntil('%');	// Wait ETX Char(%)
    msg.trim();									// Delete [ Space Char ]
    if (msg == "")
    {
      return;
    }

  //== Recv Cmd Ana & Exec
	if(0) {}	// dummy

	// 
	else if (msg.startsWith(STX_IMU_SETUP) )
	{
		//msg = msg.substring(3);	// from PC Normal
		String setMsg = imu.convert_oldSetup(msg);
		imu.setConfig(setMsg);		// From PC _NORMAL
	}
	
	// ($11)-Get Address
    else if (msg.startsWith(STX_GET_ADDR) || msg.startsWith(STX_SETUP_ENTRY) )
	{
		// [Get Address ] 1st Button Click
		String str = STX_GET_ADDR + "S" + eNow.getMyAddress() + "," + eNow.getTargetAddress() + "%";
		RespPrintln(str);

		// ENTRY SETUP MODE
		imu.setStartStop(IMU_CONF_STOP);
		flagIMUSTOP = 1;

		Entry_SetupMode();		// 위치 이동하지 말것, 마지막 처리
    }
	
	// ($9002) SETUP EXIT, NA - Normal Mode
	else if (msg.startsWith(STX_SETUP_EXIT) )
  	{
  		// NA - Normal Exit No Effetive
	}
	
  	else if (msg.startsWith(STX_IMU_INTVAL_CHK))
	{
	  imuIntervalCheckCnt = 1;
    }
	else if (msg.startsWith(STX_VERSION_READ))// 2. VER at NORMAL
	{
	 	if(IMU_CONN == imuStatus)
		{
			msg = imu.getversion();
	  	}
	  	else
	  	{
			msg = rodVer + ",Vi99.99.99";
	  	}
		
		LogPrintln(" LG] Ver__ " + rodVer + "," + msg + "%");
	}
	
	else if (msg.startsWith(STX_DEV_INFO_REQ))	// 
	{
		int reqKind = msg.substring(3,(3+2)).toInt();
		switch(reqKind)
		{
			case 0:		// IF_VER
				break;
			case 1:		// BOARD TYPE
				LogPrintln(" LG] INFO__ Board Type: " + String(rodBoardType));
				break;
			default:
				break;
		}
	}

	// ($F0) - FILE CONTROL 
  	else if (msg.startsWith(STX_SET_FILE))
	{
		//fileControl_test(msg);
		eNow.fsFormat();
  	}
	

	//($F6) - POWER AGEING TEST PROGRAM
  	else if (msg.startsWith(STX_SET_DATA))
	{
	  msg = msg.substring(3, msg.length());
	  setData_RodCycleTest(msg);
	  //LogPrintln(" LG] IMUSET" + msg);
    }
	
	//=== NOT DEFINE CMD
	else
	{
		LogPrintln(" lg] ERROR_ NOT DEFINE CMD");		
	}
	
  }
}

//-------------------------------------------------
//	타겟주소 FS에 저장
//	WRITE_OK 1000
//	WRITE_FAIL 1001
//	LENGTH_ERR  0~16
//-------------------------------------------------
int targetAddrWrite(String msg)
{
	int rts = 1000;	// OK
	
	  int len = msg.length();
	  //if(16 < len)
      if((DF_Protocol_MacAddressTextLength - 1) < len)
	  //if((MAC_ADDR_OK_LEN) != len)
	  {
		  // TBD  at FAIL ( SET NG )
      	if(false == eNow.setTargetAddress(msg))
      	{
      		rts = 1001;	//Write Error
      		LogPrintln(" LG] Error_ T_ADDR Write NG");
      	}
		else
		{
			LogPrintln(" LG] T_ADDR Set Ok Len:" + String(len) + ",m:" + msg);
		}
	  }
	  else
	  {
	  	rts = len;		// ㅣLength Error : 0 ~ 16
	  	LogPrintln(" LG] T_ADDR Set Address NG, LESS 17 ");
	  }

	  return rts;
}

/*  =====================================
        (Uart) Recv Data Hanlder, from PC, Only SetUp Mode
  ===================================== */
void uartPcHandler_Setup()
{

  if (Serial.available())	// if Recv Recv Length
  {
    String msg = Serial.readStringUntil('%');	// Wait ETX Char(%)
    msg.trim();									// Delete [ Space Char ]
    if (msg == "")
    {
      return;
    }

	String respMsg;
	
//=== Recv CMD ANA & EXEC
	if(0) {}	// dummy
	
	// ADD SEUP EXIT by Command PC
	else if (msg.startsWith(STX_SETUP_EXIT))
	{
		Exit_SetupMode();
	}

	// ($11)-Get Address
    else if (msg.startsWith(STX_GET_ADDR))
	{
      respMsg = STX_GET_ADDR + "S" + eNow.getMyAddress() + "," + eNow.getTargetAddress() + "%";
      RespPrintln(respMsg);
    }
	
	//($12)-Set Address
    else if (msg.startsWith(STX_SET_ADDR))
	{
		msg = msg.substring(3);		// Address Only(17개 문자)
		//targetAddrWrite(msg);
  		int ret = targetAddrWrite(msg);
	  	if((1000 != ret) && (!isSetupMode) )	// NG & NORMAL = LOG출력, SETUP = LOG X
	  	{
			String logMsg = "";
	  		if(1001 == ret)	logMsg = "Write Fail";
			else			logMsg = "Len Error";
			LogPrintln(" LG] Error_ TART ADDR " + logMsg +",m"+ msg + ",len:" + String(msg.length()));
	  	}


    }
	//($08Axxxx)-IMU Setting
    else if (msg.startsWith(STX_IMU_SETUP))
	{
		//msg = msg.substring(3);
		respMsg = imu.convert_oldSetup(msg);
      	imu.setConfig(respMsg);	// from PC SETUP Progam
    }

  	//($91)-Set IMU config With RECV STRING
  	else if (msg.startsWith(STX_IMU_SETSTRING))
	{
	  respMsg = msg.substring(3, msg.length());
	  //LogPrintln(msg);
	  imu.setConfigString(respMsg);
    }

  	else if (msg.startsWith(STX_IMU_INTVAL_CHK))
	{
	  imuIntervalCheckCnt = 1;
    }
	// NOT DEFINE CMD
	else
	{
		// Error
	}
	
  }
}



static int pwrOn_1st_imuRecv =0;
static int imuIntervalTime = 0;
/*-------------------------------------------------------------------
	Set Flag IMU Data Recved, for IMU Connetion
--------------------------------------------------------------------*/
void setRecvImuData()
{
	//-- 1)  IMU RCV FLAG SET
	fRecvImuData = 1;		// SET Flag IMU Data RECVED

	//-- 2) IMU Interval측정,  IMU Interval LOG OUT, CMD=$92
	// 3번 측정하여 평균 시간을 응답
  	if(imuIntervalCheckCnt)
  	{
  		imuIntervalCheckCnt++;
		if(0) {}
  		else if(2 == imuIntervalCheckCnt) { measureCnt = curr_ms_tick;	}// 1st
  		else if(3 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 2nd
  		else if(4 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 3rd
		else						// 2nd
		{
			imuIntervalTime += (int)(curr_ms_tick - measureCnt);		// 4th
			imuIntervalTime = imuIntervalTime/3;
			String strImuIntv = zeroFill4Char((unsigned int)imuIntervalTime);
			// Send to PC
			//if(AP_IS_TM == apType)
			//{
			//	eNow.write(DF_Protocol_RodToMain_Sleep, (IMU_INTERVAL_TIME + strImuIntv));
			//}
			RespPrintln(STX_IMU_INTVAL_CHK + strImuIntv +"%");	// ms
			
			imuIntervalCheckCnt = 0;
			imuIntervalTime = 0;
			measureCnt = 0;
		}
  	}
}

//---------------------------------------------------------
//
int isRodNewBoard()
{
	return (rodBoardType);		// 0-OLD ROD, 1-NEW ROD
}

//---------------------------------------------------------
//	PowerOn후 제일먼저 실행할 것.
//	- PinMode설정전에 실행할 것.
//---------------------------------------------------------
//	구보드(V1) -NC : Open(HIGH)
//	신보드(V2) - GND : LOW
//---------------------------------------------------------
int setBoardType()
{
	return NEW_BOARD_2;
}


// Button Type
int getButtonType()
{
	return (buttonType);
}

// Reel ENCODERType
int getBreakType()
{
	return (breakType);
}

// Reel ENCODERType
int getEncType()
{
	return (reelEncType);
}


// BATT Type
int getBattType()
{
	return (battType);
}

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
void settingIOtype()
{
	// Current Rod configuration is fixed in DF_Rod_Foundation.inc.
}

//----------------------------------------------------------------
//
void setSlaveMode()
{
	int i =0;
	unsigned int readData = 0x00;
	for(i=0; i<5; i++)
	{
		readData += ((unsigned int)digitalRead(SETUP_MODE_PIN) ) << i;
		delay(10);
	}	
	if(0x1F == readData)	rodMode = SETUP_MODE;
	else					rodMode = NORMAL_MODE;
}

int readSlaveMode()
{
	return rodMode;
}
int isSlaveSetupMode()
{
	return rodMode;
}

// SET PC CMD SET
void Entry_SetupMode()
{
	rodMode = SETUP_MODE;
	led1Grn_OutSts = LOW;
	digitalWrite(LED1_GRN_PIN, led1Grn_OutSts);
	led2Red_OutSts = HIGH;
	digitalWrite(LED2_RED_PIN, led2Red_OutSts);
}
void Exit_SetupMode()
{
	//rodMode = NORMAL_MODE;
	//digitalWrite(LED2_RED_PIN, LOW);

	// Setup -> Reset(=Normal) 
	//ESP.restart();
	ESP_Soft_Reset();
}

void ESP_Soft_Reset()
{
	// Setup -> Reset(=Normal) 
	ESP.restart();
	//esp_restart();
}

//-------------------------------------------------------
//	1. Change Check ( -2 ~ +2 )
//	2. Send to MAIN
//
//	CALL, 1 SEC =>, 100ms =>  1 SEC
//-------------------------------------------------------
