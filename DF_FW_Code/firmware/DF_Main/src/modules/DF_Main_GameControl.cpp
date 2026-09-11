// Main game control implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_GameControl.h"
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

static unsigned short occureFault = 0;

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
			rtnMsg = STX_GAME_STS + "0401%";		// Finish CONTINUE_READY
			//Resp2ApPrintln("$0B22010000%");		// Ready Completed
			Resp2ApPrintln(rtnMsg);			// CONTINUE READY Completed
			
			LogPrintln(" LG] RDYCN END");
			break;

		default:
			break;
	}

}


unsigned int bite_bldc_step = 0x00;
static unsigned int bite_bldcRptCnt = 0;
static unsigned long bite_bldc_to = 0;

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
					bbnMotor.offBldc(); 			// MOT OFF
					setTO(bite_bldc_to);
					bite_bldc_step = 0x39;	// [ CW - CCW ] REPEAT
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
static int biteReqCnt = 0;					//

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
static int bitePtn1Cnt = 3;			// Shallow Cnt
static int bitePtn2Cnt = 1;			// Deep Cnt (Note Use)
static int bitePtn3Cnt = 4;			// TokTok Cnt

static unsigned int bite_ptn1_step = 0;
void execBite_ptn1_Start()
{
	//bitePtn1Cnt = reqCnt;			// notUse
	bite_ptn1_step = 10;

	LogPrintln(" LG] EXEBT Start PTN_1 cnt=" + String(bitePtn1Cnt));
}

static unsigned int bite_ptn2_step = 0;
void execBite_ptn2_Start()
{
	//bitePtn2Cnt = reqCnt;			// notUse
	bite_ptn2_step = 10;
	LogPrintln(" LG] EXEBT Start PTN_2 cnt=" + String(bitePtn2Cnt));
}

static unsigned int bite_ptn3_step = 0;
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
static unsigned long hit_bldc_to = 0;
static unsigned int hitBldcRptCnt = 0;

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
static unsigned int oldbbnFight_step = 0x00;
static unsigned long bbnFightTO = 0;
static unsigned int bbnFightCnt = 0;
static unsigned int bbnFight_ptn = 0;

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

				//LogPrintln(" RT] RESIT rTime:" + String(waittime) + ",SrvE: " + String(exeResistServoENB) + "," + String(exeHoldonServoENB) + "," + String(hitServoAngle) +" RENB/HDENB/ANG" );		// Log
				
				LogPrintln(" RT] RESIT Fix:" + String(torqResistFix) + ",Tgt:" + String(stHitTorqVal.targetTorq) + ",Hold:" + String(stHitTorqVal.holdonTorq) + ",Low:" + String(stHitTorqVal.lowTorq) + ",mid" + String(stHitTorqVal.midTorq) );
			  
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
static unsigned long holdon_bldc_to = 0;
static unsigned int holdon_bldcRptCnt = 0;

void execHit_Holdon_BldcStart()
{
	holdon_bldc_step = 0x10;
}

//
void execHit_Holdon_BldcControl()
{
	static unsigned int old_holdon_bldc_step = 0x00;
	
	//int outDuty;
  
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
static int HoldOnPtn1Cnt = 4;			// Tadadak Cnt

static unsigned int Holdon_ptn1_step = 0;

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

	
	if(ROD_BOARD_V1 == rodBoardType)	// ROD OLD Board Only
	{
		if( 255 < _val)	_val = 255;		// Max 255
	
		if ( 10 > _val)	      str = "00" + String(_val);
		else if ( 100 > _val) str = "0" + String(_val);
		else                  str = String(_val);
	
		//str = STX_BREAK_MOTOR + "00000" + str;
	
		eNow.write(DF_Protocol_MainToRod_Break, str);	  // Control & LOG
	

	}
  
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
static unsigned int randingCnt = 0;

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
static unsigned int setLogToStep = 0;

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
	execContinue2Ready = 0;
	motor_AllDefault();		// TBD

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
