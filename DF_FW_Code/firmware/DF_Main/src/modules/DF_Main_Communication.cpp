// Main communication implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_Communication.h"
void rotateChangeCallback(int _dir, int _cnt)
{
  //eNow.write(PID_CTRL_NUM, str);
  String dir = (_dir)? "CW_":"CCW";
  LogPrintln(" LG] BBENC " +  dir + ", cnt: " + String(_cnt));
}

enum
{
	DF_Main_Communication_ReceiveBufferSize = 128,
	DF_Main_Communication_AddressSize = 6
};

static char DF_Main_Communication_ReceiveBuffer[DF_Main_Communication_ReceiveBufferSize];
static uint8_t DF_Main_Communication_SourceAddress[DF_Main_Communication_AddressSize];
static volatile unsigned int DF_Main_Communication_ReceiveLength = 0;
static volatile unsigned int DF_Main_Communication_ReceivePending = 0;
static volatile unsigned long DF_Main_Communication_OverwrittenCount = 0;
static volatile unsigned long DF_Main_Communication_InvalidCount = 0;
static volatile unsigned int DF_Main_Communication_SendPending = 0;
static volatile int DF_Main_Communication_SendStatus = ESP_NOW_SEND_SUCCESS;
static portMUX_TYPE DF_Main_Communication_Mux = portMUX_INITIALIZER_UNLOCKED;

void sent_cb_esp_now_sts(const uint8_t* mac_addr, esp_now_send_status_t status)
{
	(void)mac_addr;
	portENTER_CRITICAL(&DF_Main_Communication_Mux);
	DF_Main_Communication_SendStatus = (int)status;
	DF_Main_Communication_SendPending = 1;
	portEXIT_CRITICAL(&DF_Main_Communication_Mux);
}

void DF_Main_Communication_ProcessSendResult()
{
	int status = ESP_NOW_SEND_SUCCESS;
	unsigned int pending = 0;

	portENTER_CRITICAL(&DF_Main_Communication_Mux);
	pending = DF_Main_Communication_SendPending;
	if (pending)
	{
		status = DF_Main_Communication_SendStatus;
		DF_Main_Communication_SendPending = 0;
	}
	portEXIT_CRITICAL(&DF_Main_Communication_Mux);

	if (pending && ((ESP_NOW_SEND_SUCCESS != status) || rodRegistMode))
	{
		LogPrintln(String(" LG] nowSent ") + ((ESP_NOW_SEND_SUCCESS == status) ? "Ok" : "Fail"));
	}
}

void recv_cb_esp_now_msg(const uint8_t *mac_info, const uint8_t *data, int data_len)
{
	int i;

	if ((NULL == mac_info) || (NULL == data) || (2 > data_len) || (DF_Main_Communication_ReceiveBufferSize <= data_len))
	{
		portENTER_CRITICAL(&DF_Main_Communication_Mux);
		DF_Main_Communication_InvalidCount++;
		portEXIT_CRITICAL(&DF_Main_Communication_Mux);
		return;
	}

	if (!rodRegistMode)
	{
		for (i = 0; i < DF_Main_Communication_AddressSize; i++)
		{
			if (slave_board_addr[i] != mac_info[i])
			{
				return;
			}
		}
	}

	portENTER_CRITICAL(&DF_Main_Communication_Mux);
	if (DF_Main_Communication_ReceivePending)
	{
		DF_Main_Communication_OverwrittenCount++;
	}
	memcpy(DF_Main_Communication_ReceiveBuffer, data, data_len);
	DF_Main_Communication_ReceiveBuffer[data_len] = '\0';
	memcpy(DF_Main_Communication_SourceAddress, mac_info, DF_Main_Communication_AddressSize);
	DF_Main_Communication_ReceiveLength = (unsigned int)data_len;
	DF_Main_Communication_ReceivePending = 1;
	portEXIT_CRITICAL(&DF_Main_Communication_Mux);
}

int DF_Main_Communication_TakeReceived(char *data, unsigned int capacity, unsigned int *length, uint8_t *sourceAddress, unsigned long *overwrittenCount, unsigned long *invalidCount)
{
	unsigned int receivedLength;

	if ((NULL == data) || (NULL == length) || (NULL == sourceAddress) || (NULL == overwrittenCount) || (NULL == invalidCount))
	{
		return 0;
	}

	portENTER_CRITICAL(&DF_Main_Communication_Mux);
	if (!DF_Main_Communication_ReceivePending)
	{
		portEXIT_CRITICAL(&DF_Main_Communication_Mux);
		return 0;
	}
	receivedLength = DF_Main_Communication_ReceiveLength;
	if ((receivedLength + 1) > capacity)
	{
		DF_Main_Communication_InvalidCount++;
		DF_Main_Communication_ReceivePending = 0;
		portEXIT_CRITICAL(&DF_Main_Communication_Mux);
		return 0;
	}
	memcpy(data, DF_Main_Communication_ReceiveBuffer, receivedLength + 1);
	memcpy(sourceAddress, DF_Main_Communication_SourceAddress, DF_Main_Communication_AddressSize);
	*length = receivedLength;
	*overwrittenCount = DF_Main_Communication_OverwrittenCount;
	*invalidCount = DF_Main_Communication_InvalidCount;
	DF_Main_Communication_OverwrittenCount = 0;
	DF_Main_Communication_InvalidCount = 0;
	DF_Main_Communication_ReceivePending = 0;
	portEXIT_CRITICAL(&DF_Main_Communication_Mux);
	return 1;
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
static unsigned int torqMotorStop_Step = 0x00;
static unsigned long torqMotor_StopTimeOut = 0;
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
					Resp2ApPrintln(STX_TORQUE_MOTOR + "000%");	// OFF RESP
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
static unsigned long bbnMotor_StopTimeOut = 0;
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
				Resp2ApPrintln(STX_BBN_MOTOR + "00000000%");	// OFF RESP
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
}

void lmMotor_OffTime_Control()
{
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

	
	motor_AllOff();

	// End Process : All Exeute Flag CEAR
	clear_ExecFlag();
	
	reelOut_AllOff();

}

void setVal_Hit_MainPwr(String msg)
{
	
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
	eNow.write(DF_Protocol_MainToRod_ImuDataControl, msg);		// Control & LOG , Send TO ROD
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
	eNow.write(DF_Protocol_MainToRod_Sleep, strMsg);	// ROD Sleep enable
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
		  
		  LogPrintln(" LG] RODCN "+respMsg);
		
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
		  
		  LogPrintln(" FW] IMUCN " + respMsg);		// LOG-SERIAL1

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
	  	
	  	
		  LogPrintln(" FW] BATLV " + respMsg); 	  // LOG-SERIAL1

		// ===4) ROD Board Type SEND
		respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;
		respMsg += strRodBoardType;		// Old/New
		respMsg += "%";
		Resp2ApPrintln(respMsg); 	  // Send to PC

			LogPrintln(" FW] INFOR " + respMsg);	  // LOG-SERIAL1

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
