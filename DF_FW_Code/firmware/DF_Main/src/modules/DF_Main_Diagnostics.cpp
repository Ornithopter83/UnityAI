// Main diagnostics implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_Diagnostics.h"
void diagInput_AllOff()
{
	diagInput_BoardInAllOff();
	DiagInput_MotInAllOff();
	DiagInput_MechaInAllOff();
	DiagInput_ElecInAllOff();
	
}

// Clear All Input Exec Flag

#define DIAG_CHAGE_VAL	3		// 값의 변화가 있도록 OLD값을 갱신

void diagInput_BoardInAllOff()
{
	// Board Input All Off
	diagInFlag_ResetSwc = 0; 	
	diagInFlag_BootSwc = 0; 	
	diagInFlag_WdtLbSig = 0; 	
	diagInFlag_SelectBoard = 0;
}
void diagInput_BoardInAllOn()
{
	// Board Input All Off
	//diagInFlag_ResetSwc = 1; 	
	diagInFlag_BootSwc = 1; 	
	oldsensorBootSwc = sensorBootSwc + DIAG_CHAGE_VAL;
	//diagInFlag_WdtLbSig = 1; 	
	//diagInFlag_SelectBoard = 1;
}

void DiagInput_MotInAllOff()
{
	diagInFlag_BobbinEncA = 0;
	diagInFlag_BobbinEncB = 0;
	diagInFlag_BobbinEncDir = 0;
	diagInFlag_AngleHomeSns = 0;
	diagInFlag_BbnBldcFG = 0;	
}


void DiagInput_MotInAllOn()
{

	diagInFlag_BbnBldcFG = 1;
	oldsensorBbnFG = sensorBbnFG + DIAG_CHAGE_VAL;
}

void DiagInput_MainEcoderInAllOn()
{
	diagInFlag_BobbinEncA = 1;
	oldsensorBbnEncA = sensorBbnEncA + DIAG_CHAGE_VAL;
	diagInFlag_BobbinEncB = 1;
	oldsensorBbnEncB = sensorBbnEncB + DIAG_CHAGE_VAL;

	if(AP_IS_TM == apType)
	{
		diagInFlag_BobbinEncDir = 1;
		oldsensorBbnEncCnt= sensorBbnEncCnt + DIAG_CHAGE_VAL;
	}

	//agInFlag_AngleHomeSns = 1;

}



void DiagInput_MechaInAllOff()
{
	// Reserve
}


void DiagInput_ElecInAllOff()
{
	diagInFlag_PonSwc = 0;
	diagInFlag_PcUsb5V = 0;
	//diagInFlag_Interlock = 0;
}
void DiagInput_ElecInAllOn()
{
	diagInFlag_PonSwc = 1;
	oldsensorPonSwc = sensorPonSwc + DIAG_CHAGE_VAL;
	diagInFlag_PcUsb5V = 1;
	oldsensorUsb5V = sensorUsb5V + DIAG_CHAGE_VAL;
	//diagInFlag_Interlock = 1;
	//oldsensorDoorIlk = sensorDoorIlk + DIAG_CHAGE_VAL;
}

void diagInput_Cat0(String _msg)
{
	// 5 th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();
	switch(iItemNo)
	{
		case 0:
			diagInput_AllOff();
			
			LogPrintln(" LG] IO_in All Input mon OFF");
			break;
		 default:
			 
			 LogPrintln(" LG] IO_in Can't Find Item No");
			 break;
	 }

}

// BOARD IN 
void diagIput_Cat1(String _msg)
{
	int iItemNo = _msg.substring(6,(6+2)).toInt();
		   // 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt();
		   // 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE

		   switch(iItemNo)
		   {
			   case 0:
				   if(DAIG_OFF == iAct)
				   {
					   diagInput_BoardInAllOff();
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInput_BoardInAllOn();
				   }
				   
				   LogPrintln(" LG] IO_in Board Input All OFF");
				   break;
			   case 1:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_ResetSwc = 0; 		   // Monitor OFF
					   
					   LogPrintln(" LG] IO_in Reset SWC mon ON");
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_ResetSwc = 1; 		   // Moniot ON
					   oldsensorResetSwc = sensorResetSwc + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Reset SWC mon ON");
				   }
				   else
				   {
					   
					   LogPrintln(" LG] IO_in Can't Find Act No");
				   }
				   break;
			   case 2:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_BootSwc = 0;			   // Monitor OFF
					   
					   LogPrintln(" LG] IO_in Boot SWC mon OFF");
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_BootSwc = 1;			   // Moniot ON
					   oldsensorBootSwc = sensorBootSwc + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Boot SWC mon ON");
				   }
				   else
				   {
				   }
				   break;
			   case 3:
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_WdtLbSig = 0; 		   // Monitor OFF
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_WdtLbSig = 1; 		   // Moniot ON
					   oldsensorWdtFbSig = sensorWdtFbSig + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in WDT LB Signal mon ON");
				   }
				   else
				   {
				   }
				   break;
			   case 4: 
				   if(DAIG_OFF == iAct)
				   {
					   diagInFlag_SelectBoard = 0;			   // Monitor CW Mon OFF
				   }
				   else if(DAIG_ON == iAct)
				   {
					   diagInFlag_SelectBoard = 1; 
					   oldsensorSelBoard = sensorSelBoard + DIAG_CHAGE_VAL;
					   
					   LogPrintln(" LG] IO_in Door Interlock mon ON");
				   }
				   break; // Input Monitor Start
			   default: 
				   
				   LogPrintln(" LG] IO_in Can't Find Item No");
				   break;
		   	}
}

// MOTOR IN
void diagInput_Cat3(String _msg)
{

	String logmsg;
	
	int iItemNo = _msg.substring(6,(6+2)).toInt();
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt();
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	 // NOT USE

	switch(iItemNo)
	{
		case 0:
			if(DAIG_OFF == iAct)
			{
				DiagInput_MotInAllOff();
			}
			else if(DAIG_ON == iAct)
			{
				DiagInput_MotInAllOn();
				DiagInput_MainEcoderInAllOn();
			}
			break;

		case 1: 
			if(DAIG_OFF == iAct)
			{
				 diagInFlag_BobbinEncA = 0; 		// BBN Enc A
				 
				 LogPrintln(" LG] IO_in Main Enc_A mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncA = 1; 
				oldsensorBbnEncA = sensorBbnEncA + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc_A mon ON");
			}
			break; // Input Monitor Start

		case 2: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BobbinEncB = 0;			// BBN ENC B
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncB = 1; 
				oldsensorBbnEncB= sensorBbnEncB+ DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc_B mon ON");
			}
			break; // Input Monitor Start

		case 3: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BobbinEncDir = 0;			// BBN ENC DIR
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BobbinEncDir = 1; 
				oldsensorBbnEncCnt= sensorBbnEncCnt + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Main Enc Dir/Cnt ON");
			}
			break; // Input Monitor Start

		case 4: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_AngleHomeSns = 0;			// ANGLE HOME SNS
				LogPrintln(" LG] IO_in Angle HomeSensor mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_AngleHomeSns = 1; 
				oldsensorAngHomeSns = sensorAngHomeSns + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Angle HomeSensor mon ON");
			}
			break; // Input Monitor Start

		case 5: 					// Main Encoder Direction
			if(DAIG_OFF == iAct)
			{
				diagInFlag_BbnBldcFG = 0;			// BBN FG
				LogPrintln(" LG] IO_in BBN BLDC FG mon OFF");
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_BbnBldcFG = 1; 
				oldsensorBbnFG = sensorBbnFG + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in BBN BLDC FG mon ON");
			}
			break; // Input Monitor Start
	
		 //case 6:		 // IN3-6 1ms 불필요(DIAG는 10로 변화만 CHECK)
		 //  break;
			 
	
		default:
			LogPrintln(" LG] IO_in Can't Find Item No");
			break;
	}

}

// ELEC IN
void diagInput_Cat5(String _msg)
{
	   // 5 th : "-"
	   int iItemNo = _msg.substring(6,(6+2)).toInt();
	   // 8th : ","
	   int iAct = _msg.substring(9,(9+1)).toInt();
	   // 10th : ","
	   int iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE

	switch(iItemNo)
	{
		case 0:
			if(DAIG_OFF == iAct)
			{
				DiagInput_ElecInAllOff();
			}
			else if(DAIG_ON == iAct)
			{
				DiagInput_ElecInAllOn();
			}
			break;
		case 1: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_PonSwc = 0;			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_PonSwc = 1; 
				oldsensorPonSwc = sensorPonSwc + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Power On Switch mon ON");
			}
			break; // Input Monitor Start
		case 2: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_PcUsb5V = 0; 			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_PcUsb5V = 1; 
				oldsensorUsb5V = sensorUsb5V + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Usb 5V mon ON");
			}
			break; // Input Monitor Start
		case 3: 
			if(DAIG_OFF == iAct)
			{
				diagInFlag_Interlock = 0;			// Monitor CW Mon OFF
			}
			else if(DAIG_ON == iAct)
			{
				diagInFlag_Interlock = 1;
				oldsensorDoorIlk = sensorDoorIlk + DIAG_CHAGE_VAL;
				
				LogPrintln(" LG] IO_in Door Interlock mon ON");
			}
			break; // Input Monitor Start
			
		default: 
			LogPrintln(" LG] IO_in Can't Find Item No");
			break;
	}

}

/*-------------------------------------------------------------------
$DIxx-yy,A%
-------------------------------------------------------------------*/
void diagInputTest(String _msg)
{
	   int iCatNo = 0;
	   int iItemNo = 0;
	   int iAct = 0;
	   int iPara = 0;

	   String logmsg = "";
	   
	   //int dir = 0;
	   //int dutyPercent = 0;

	   //------------------------------------------------------
	   //  $DIxx-xx,A%
	   //------------------------------------------------------
	   iCatNo = _msg.substring(3,(3+2)).toInt();	//xx-
	   // 5 th : "-"
	   iItemNo = _msg.substring(6,(6+2)).toInt();	//yy,
	   // 8th : ","
	   iAct = _msg.substring(9,(9+1)).toInt();		//A,
	   // 10th : ","
	   iPara = _msg.substring(11,(11+3)).toInt();	// NOT USE
	   
	   if( 0 == iCatNo)
	   {
			diagInput_Cat0(_msg);
	   }
	   else if( 1 == iCatNo)	   // 보드1 Board In
	   {
	   		diagIput_Cat1(_msg);

	   }
	   else if( 2 == iCatNo)		   // Reserve (OP)
	   {
		   // RSV
		   LogPrintln(" LG] IO_in Can't Find Cat No");
	   }
	   else if( 3 == iCatNo)		   // Motor IN
	   {
	   		diagInput_Cat3(_msg);
	   }

	   // "04" - Reserve
	   else if( 4 == iCatNo)	   // 
	   {
	   	// RSV
	   }

	   // "05" - Board Input
	   else if( 5 == iCatNo)	   // 
	   {
	   		diagInput_Cat5(_msg);

	   }
	   else if( 11 == iCatNo)	   // 
	   {
	   }
	   else if( 12 == iCatNo)	   // 
	   {
	   }
	   else if( 14 == iCatNo)	   // 
	   {
	   }
	   else if( 15 == iCatNo)	   // 
	   {
	   }
	   else if( 20 == iCatNo)	   // 
	   {
	   }
	   else
	   {
		   
		   LogPrintln(" LG] IO_in Can't Find Cat No");
	   }

   // TBD,	Responce - Return Value
   // Diag ON - Input - Chaned then Responce Always!!!
}

//===============================\
//  Diag Output Functins
//===============================

void diagOutput_AllOff()
{
	diagOutput_BoardOutAllOff();
}

void diagOutput_BoardOutAllOff()
{
	setRGBcolor(LED_IDX_CENT, COLOR_BLACK);
}


void setRGBcolor(int pos, int Color)
{
	if(COLOR_MAX_NO > Color)
	{
		sRGB[pos].r = sColorTBL[Color].r;	
		sRGB[pos].g = sColorTBL[Color].g;	
		sRGB[pos].b = sColorTBL[Color].b;
	}
	else
	{
		LogPrintln(" LG] LedRg Can't Find Color Number!!!");
	}
}

void setOutRGBcolor(int pos, int Color)
{
	if(COLOR_MAX_NO > Color)
	{
		outRGB[pos].r = sColorTBL[Color].r;	
		outRGB[pos].g = sColorTBL[Color].g;	
		outRGB[pos].b = sColorTBL[Color].b;
	}
	else
	{
		LogPrintln(" LG] LedRg Can't Find Color Number!!!");
	}
}

// LM MOT DIAG OUT
//
static int lmMotTimeout_step = 0;
int diagOutflag_lmMotTimeOut = 0;
static unsigned long lmMotTimeout_Val = 0;

void diagLmMotor_TimeoutStart(unsigned int _time)
{
	lmMotTimeout_Val = _time;
	lmMotTimeout_step = 10;			// Start
	diagOutflag_lmMotTimeOut = 1;		// monitor SET
}

void diagLmMotor_TimeOutStop()
{
	diagOutflag_lmMotTimeOut = 0;		// monitor SET
	lmMotTimeout_step = 0;			// Start
}

//
void diagLmMotor_TimeoutControl()
{
	static long _to;
	switch(lmMotTimeout_step)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// START
			setTO(_to);
			lmMotTimeout_step = 20;
			break;
		case 20:	// TIMEOUT Check
			if(checkTO(_to, lmMotTimeout_Val))
			{
				lmMotTimeout_step = 30;
			}
			break;
		case 30:	// MOT OFF
			lmMotOff();
			lmMotTimeout_step = 40;
			break;
		case 40:	// END
			_to = 0;
			lmMotTimeout_step = 0;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}


/*
==========================================================
	DIAG BBN Motor Control
		for DIAG
==========================================================
*/
// Auto Time Out  Control
void diagBbnMotor_TimeoutStart(unsigned int _time)
{
	bbnTimeout_Val = _time;
	bbnTimeout_step = 0x10;			// Start
	diagOutflag_bbnTimeOut = 1;		// monitor SET
}

// TBD, 
/*
	Diag_Exit함수에 아래 추가 필요.

	1. (조작) Diag나가기, Cat별 All Off하기등 
	 Step중에 종료되는 경우, Flag/Step처리 추가할 것.
*/


//
void diagBbnMotor_TimeoutControl()
{
	
	switch(bbnTimeout_step)
	{
		case 0x00:	// IDLE
			break;
		case 0x10:	// START
			setTO(bbnDiagTO);
			bbnTimeout_step = 0x20;
			break;
		case 0x20:	// TIMEOUT Check
			if(checkTO(bbnDiagTO, bbnTimeout_Val))
			{
				bbnTimeout_step = 0x30;
			}
			break;
		case 0x30:	// MOT OFF
			bbnBldcOff();
			bbnTimeout_step = 0x40;
			break;
		case 0x40:	// END
			diagOutflag_bbnTimeOut = 0;
			bbnTimeout_step = 0x00;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}

//=============================================
//	TOK TOK Control
//=============================================
void daigBbnMotor_TokTokEnd()
{
	diagBbnflag_TokTokControl = 0;
	bbnTokTok_step = 0x00;
	bbnMotor.offBldc();
}


void diagBbnMotor_TokTokStart(unsigned int _ackKind, unsigned int _cnt)
{

/*
	bbnTokTok_OnDuty = _onduty;
	bbnTokTok_OnTime = _ontime;
	bbnTokTok_OffDuty = _offduty;
	bbnTokTok_OffTime = _offtime;
*/
	bbnActKind = _ackKind;
	// bbnOpKind = _opKind;
  	bbnReqCnt[bbnActKind] = _cnt;


	bbnTokTok_step = 0x10;
	diagBbnflag_TokTokControl = 1;		// Start
}


//
void diagBbnMotor_TokTokControl(unsigned _opKind)		//_ak : Action Kind
{
	//static unsigned long to;
  /*
	if(oldbbnTokStep != bbnTokTok_step)
	{
		oldbbnTokStep = bbnTokTok_step;
		
		LogPrintln(" LG] StepC bbn toktok step: " + String(bbnTokTok_step) );
	}
  */
	
	switch(bbnTokTok_step)
	{
		case 0x00:	// IDLE
			break;
			
		case 0x10:	// START
			bbnRptCnt = 0;
			setTO(bbnTokTO);
			bbnTokTok_step = 0x20;
			break;
			
		case 0x20:	// TIMEOUT Check
			bbnMotor.onBldc(BBN_MOT_CW, sBbnActTbl[bbnActKind].onDuty);	// MOT CW ON
			setTO(bbnTokTO);
			bbnTokTok_step = 0x30;
			break;
		case 0x30:
			if(checkTO(bbnTokTO, sBbnActTbl[bbnActKind].onTime))
			{
				bbnMotor.offBldc();				// MOT OFF
				setTO(bbnTokTO);
				bbnTokTok_step = 0x40;
			}
			break;
		case 0x40:	// MOT OFF WAIT
			if(checkTO(bbnTokTO, sBbnActTbl[bbnActKind].offTime))
			{
				bbnRptCnt++;
				if( bbnReqCnt[bbnActKind] < bbnRptCnt)
				{
					bbnTokTok_step = 0x70;		// END
				}
				else
				{
					bbnTokTok_step = 0x20;			// REPEAT
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
			bbnRptCnt = 0;
			
			bbnTokTok_step = 0x00;
			break;
		default:
			// ERROR (unknown Step)
			break;
	}
}

//
void diagOut_Cat1(String _msg)
{
	int iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
	//5th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt(); 	// Act(1)
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
	//14th : ","
	//iPara2 = _msg.substring(15).toInt();	// para_2(4)
	int iPara2 = _msg.substring(15,(15+4)).toInt(); // para_2(4)
	// 19th : ","
	int iPara3 = _msg.substring(20,(20+5)).toInt(); // para_3(5)

	switch(iItemNo)
	{
		case 0:
			diagOutput_BoardOutAllOff();
			
			LogPrintln(" LG] IOout Board Output All OFF");
			break;
		case 1:
			diagOutFlag_WDToff = 1; 		// WDT OFF
			
			LogPrintln(" LG] IOout WDT OFF");
			break;
		case 2:
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED2_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED2 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED2_PIN, LED_ON);
				
				LogPrintln(" LG] IO_in	BD LED2 ON");
			}
			else
			{
			}
			break;
	
		case 3:
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED3_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED3 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED3_PIN, LED_ON);
				
				LogPrintln(" LG] IOout BD LED3 ON");
			}
			else
			{
			}
			break;
		case 4: 
			if(DAIG_OFF == iAct)
			{
				digitalWrite(BD_LED4_PIN, LED_OFF);
				
				LogPrintln(" LG] IOout BD LED4 OFF");
			}
			else if(DAIG_ON == iAct)
			{
				digitalWrite(BD_LED4_PIN, LED_ON);
				
				LogPrintln(" LG] IOout BD LED4 ON");
			}
			else
			{
			}
			break; // Input Monitor Start
		case 5 :	// DEBUG PORT
			if(DAIG_OFF == iAct)
			{
				debugPort_Off();
				LogPrintln(" LG] IOout BD DebugPort OFF");
			}
			else if(DAIG_ON == iAct)
			{
				debugPort_On();
				LogPrintln(" LG] IOout BD DebugPort ON");
			}
	
			else
			{
				// Error
			}
			break;
	
		default: 
			break;
	}

}

// DIAG OUT 03-XX
void diagOut_Cat3(String _msg)
{
	int iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
	//5th : "-"
	int iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
	// 8th : ","
	int iAct = _msg.substring(9,(9+1)).toInt(); 	// Act(1)
	// 10th : ","
	int iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
	//14th : ","
	//iPara2 = _msg.substring(15).toInt();	// para_2(4)
	int iPara2 = _msg.substring(15,(15+4)).toInt(); // para_2(4)
	// 19th : ","
	int iPara3 = _msg.substring(20,(20+5)).toInt(); // para_3(5)

	switch(iItemNo)
	{
		case 0 :	// TBD Mot All Off
			torqMotor.setValue(0);
			bbnMotor.offBldc();
			
			LogPrintln(" LG] IOout Motot ALL OFF");
			break;
			
		case 1 :	// TORQ
			if(DAIG_MOT_OFF == iAct)
			{
				torqMotor.setValue(0);
				
				LogPrintln(" LG] IOout Main Mot(Torq) OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				torqMotor.setValue(iPara);
				
				LogPrintln(" LG] IOout Main Mot(Torq) ON, Duty : " + String(iPara));
			}
	
			else
			{
			}
			break;
			
		case 2 :	// TBD , Angle Motor
			break;
	
		case 3 :	// BBN BLDC
			// Para Check
			/*
			if((DAIG_MOT_OFF != iAct ) && ( 1 > iPara || 100 < iPara))
			{
				
				LogPrintln(" LG] Duty Range Over : " + String(iPara));
	
				// END
				//iPara = 5;
				break;
			}
			*/
			
			if(DAIG_MOT_OFF == iAct)
			{
				bbnMotor.offBldc();
				
				LogPrintln(" LG] IOout BBNMT OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CW, iPara);
				diagBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout BBNMT CW ON, Duty : " + String(iPara));
			}
			else if(DAIG_MOT_CCW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CCW, iPara);
				diagBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout BBNMT CCW ON, Duty : " + String(iPara));
			}
			else
			{
			}
			break;
			
			
		case 4 :	// TBD	BLDC_24V_OFF
			if(DAIG_MOT_OFF == iAct)
			{
				digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);	// BLDC_24V_ON OFF
				
				LogPrintln(" LG] IOout BBN24 OFF");
			}
			else			// ON
			{
				digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VON);	  // BLDC_24V_ON ON
				
				LogPrintln(" LG] IOout BBN24 ON");
			}
			break;
	
		case 5 :	// BBN BLDC TOKTOK
			// Para Check
			/*
			if((DAIG_MOT_OFF != iAct ) && ( 1 > iPara || 100 < iPara))
			{
				
				LogPrintln(" LG] Duty Range Over : " + String(iPara));
	
				// END
				//iPara = 5;
				break;
			}
			*/
			
			if(DAIG_MOT_OFF == iAct)
			{
				//bbnMotor.offBldc();
				daigBbnMotor_TokTokEnd();
				
				LogPrintln(" LG] IOout BBNTK OFF");
			}
			else if(DAIG_MOT_CW == iAct)
			{
				//bbnMotor.onBldc(BBN_MOT_CW, iPara);
				//diagBbnMotor_TokTokStart(bbnTokTok_OnDuty, bbnTokTok_OnTime, bbnTokTok_OffDuty, bbnTokTok_OffTime);
				//iPara = _msg.substring(9,(9+1)).toInt();	// // ActKind
				if ( (BBNACT_TBLMAX) <= iPara)	{ iPara = 0; }		// OP_kind (Wave,...)
				//iPara2 = _msg.substring(10,(10+1)).toInt();	// Repeat CNT
				//if (	<= iPara2)	{ iPara2 =	}
				//iPara2 = _msg.substring(10).toInt();	// Repeat CNT  (Repeat)
				if ( (10000) <= iPara2) { iPara2 = 10000; }
			//TBD Cnt or TIME
			/*
				iPara2 = _msg.substring(10).toInt();	// Total Time
				if ( (30000) <= iPara2) { iPara2 = 30000; }
			*/
				diagBbnMotor_TokTokStart(iPara, iPara2);			// (ActKind=OpKind, RPT CNT) 
				//String tokStr = String(bbnTokTok_OnDuty) + ", " + String(bbnTokTok_OnTime) + ", " + String(bbnTokTok_OffDuty) + ", " + String(bbnTokTok_OffTime);
				
				LogPrintln(" LG] IOout BBNTK ON, AckKind : " + String(iPara) + ", opKind: " + String(iPara2) + ", Repeat CNT: " + String(iPara3)+ ", On/Time/Off/Time: " + String(bbnTokTok_OnDuty) + ", " + String(bbnTokTok_OnTime) + ", " + String(bbnTokTok_OffDuty) + ", " + String(bbnTokTok_OffTime));
			}
		/*
			else if(DAIG_MOT_CCW == iAct)
			{
				bbnMotor.onBldc(BBN_MOT_CCW, dutyPercent);
				//daigBbnMotor_TimeoutStart(3000);
				
				LogPrintln(" LG] IOout Bobbin CCW ON, Duty : " + String(iPara));
			}
		*/
			else
			{
			}
			break;
			
	
		default:
			break;
	}

}

/*
=================================================================
	DIAG OUTPUT Control
$DOxx-yy,A,pppp,PPPPP
=================================================================
*/
void diagOutputTest(String _msg)
{
		int iCatNo = 0;
		int iItemNo = 0;
		int iAct = 0;
		int iPara = 0;
		int iPara2 = 0;
		int iPara3 = 0;

		int dir = 0;
		int dutyPercent = 0;
		
		iCatNo = _msg.substring(3,(3+2)).toInt();	// cat(2)
		//5th : "-"
		iItemNo = _msg.substring(6,(6+2)).toInt();	// item(2)
		// 8th : ","
		iAct = _msg.substring(9,(9+1)).toInt();		// Act(1)
		// 10th : ","
		iPara = _msg.substring(11,(11+3)).toInt();	// para_1(3)
		//14th : ","
		//iPara2 = _msg.substring(15).toInt();	// para_2(4)
		iPara2 = _msg.substring(15,(15+4)).toInt();	// para_2(4)
		// 19th : ","
		iPara3 = _msg.substring(20,(20+5)).toInt();	// para_3(5)
		
	
		dutyPercent = iPara;

		//LogPrintln(" LG] IOout " );
		if( 0 == iCatNo && 0 == iItemNo )
		{
			// DiagInMonitor = 0;
			diagOutput_AllOff();
			
			LogPrintln(" LG] IOout All Output ALL OFF");
		}

		// " 1" - BD_OUT
		else if( 1 == iCatNo)		// 보드1 Board Out
		{
			diagOut_Cat1(_msg);
		}

		// "02"- Ext LED
		else if( 2 == iCatNo)			// Reserve (OP)
		{
			switch(iItemNo)
			{
				case 0:	// TBD
					//diagInput_BoardInAllOff();
					extLed_AllOff();					
					
					LogPrintln(" LG] IOout Ext LED All OFF");
					break;
				case 1:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_R = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_R);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_R OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_R = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_R, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_R ON");
					}
					else
					{
					}
					break;
				case 2:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_G = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_G);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_G OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_G = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_G, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_G ON");
					}
					else
					{
					}
					break;
				case 3:
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_B = 0;			// Monitor OFF
						extLed.off(LEDPOS_CENT_B);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_B OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_B = 1;			// Moniot ON
						extLed.on(LEDPOS_CENT_B, iPara);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_B ON");
					}
					else
					{
					}
					break;
				case 4: 
					if(DAIG_OFF == iAct)
					{
						diagOutFlag_LedCent_RGB = 0; 			// Monitor CW Mon OFF
						extLed.off(LEDPOS_CENT_R);		// Position, Duty
						extLed.off(LEDPOS_CENT_G);		// Position, Duty
						extLed.off(LEDPOS_CENT_B);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB OFF");
					}
					else if(DAIG_ON == iAct)
					{
						diagOutFlag_LedCent_RGB = 1;
						//setRGBcolor(sRGB, iPara);				// Color N -> R,G,B
						setRGBcolor(LEDPOS_CENT_RGB, iPara);				// Color N -> R,G,B
						extLed.RGBon(LEDPOS_CENT_RGB, sRGB[LEDPOS_CENT_RGB].r, sRGB[LEDPOS_CENT_RGB].g, sRGB[LEDPOS_CENT_RGB].b);
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else
					{
					}
					break; // Input Monitor Start

				// TBD Color item, Dimming
				case 5:
					break;
					
				case 6: 
					if(DAIG_OFF == iAct)
					{
						//diagOutFlag_LedCent_RGB = 0; 			// Monitor CW Mon OFF
						extLed.off(LEDPOS_BTM);		// Position, Duty
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else if(DAIG_ON == iAct)
					{
						//diagOutFlag_LedCent_RGB = 1;
						//setRGBcolor(sRGB, iPara);				// Color N -> R,G,B
						//setRGBcolor(iPara);				// Color N -> R,G,B
						extLed.on(LEDPOS_BTM, iPara);
						
						LogPrintln(" LG] IOout diagOutFlag_LedCent_RGB ON");
					}
					else
					{
					}
					break; // Input Monitor Start
					
				default: 
					break;
			}
		}

		// "03"- MOTOR
		else if( 3 == iCatNo )
		{
			diagOut_Cat3(_msg);
		}

		// "04 - Reserved"
		else if( 4 == iCatNo )
		{
		}

		// "05" - Elec Out
		else if( 5 == iCatNo )
		{
			switch(iItemNo)
			{
				case 0 : 	// TBD Mot All Off
					// Board_1 Out ALL OFF					
					LogPrintln(" LG] IOout Board_1 ALL OFF");
					break;
					
				case 1 : 	// SUB_AC
					if(DAIG_OFF == iAct)
					{
						//subAC_Off();
						subAcOff_WithOtherOff();
					}
					else if(DAIG_ON == iAct)
					{
						subAC_On();
					}

					else
					{
						// Error
					}
					break;
					
				default:
					break;
			}
		}

		else
		{
			// Error
		}
}

// NOT USE
void findDiagCode(String msg, stDiagCode code)
{
	code.catNo = msg.substring(3,5).toInt();	// 2 char	XX
												// 1char Skip "-" -
	code.itemNo = msg.substring(6,8).toInt(); 	// 2 char	YY
	
	code.act = msg.substring(8,9).toInt();		// 1 char	A
	
	code.para = msg.substring(9).toInt();		// 3 char ~	PPP ~ PPPPPPPPPPPPP
}

// NOT USE
int controlDiagInput(String msg)
{
	stDiagCode diag_code;		// item, actno
	findDiagCode(msg, diag_code);

}

// NOT USE
int controlDiagOutput(String msg)
{
	stDiagCode diag_code;		// item, actno
	findDiagCode(msg, diag_code);
	
	// execOutAct(diag_code);

}



void diagOutAllOff()
{
	torqMotor.setValue(0);
	bbnMotor.offBldc();
}



/*---------------------------------------------------------------------------------------------
	각 값들의 설정
	방법 - LOG SERIAL포트로 명령 송신
	처리 - 각 Control등의 설정값을 세팅
---------------------------------------------------------------------------------------------*/

// Wave Val SET ( $F0 )
void Set_WaveData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// Control SET
	{
		// Log OUT
	}
	else if( '1' == ch ) 		// Control Case
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 7 < val ) val = 7;
		waveControlCase = val;
		//
		//LogPrintln(" LG] waveControlCase: " + String(waveControlCase);
	}
	else if( '2' == ch )		// Angle SET
	{
		val = msg.substring(4,7).toInt();	// angle 1
		if( -90 > val ) val = -90;
		if( 90 < val ) val = 90;
		stWaveServoVal.angle1 = val;
	
		val = msg.substring(7,10).toInt();	// angle 2
		if( -90 > val ) val = -90;
		if( 90 < val ) val = 90;
		stWaveServoVal.angle2 = val;
	
		val = msg.substring(10).toInt();	// Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveServoVal.interval = val;
	}
	else if( '3' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Return Torq
		if( 20 > val ) val = 20;
		if( 80 < val ) val = 80;
		stWaveTorqVal.rtTorq = val;
	
		val = msg.substring(7,10).toInt();	// Target Torq
		if( 20 > val ) val = 20;
		if( 99 < val ) val = 99;
		stWaveTorqVal.tgTorq = val;
	
		val = msg.substring(10,(10+4)).toInt();	//on Time
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveTorqVal.onInterval = val;

		val = msg.substring(14,(14+4)).toInt();	//Off Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveTorqVal.offInterval = val;
	}
	
	else if( '5' == ch )		// Wave Type SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( (WAVE_PWR_MAX-1) < val ) val = (WAVE_PWR_MAX-1);
		waveType = val;
		waveType_TorqSet();
	}
	else if( '6' == ch )		// Wave Type  Torq SET
	{
		val = msg.substring(4,(4+3)).toInt();	// Torq 1
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[1] = val;
	
		val = msg.substring(7,(7+3)).toInt();	// Torq 2
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[2]  = val;
	
		val = msg.substring(10,(10+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[3]  = val;

		val = msg.substring(13,(13+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[4]  = val;

		val = msg.substring(16,(16+3)).toInt();	// Tprq 3
		if( 5 > val ) val = 5;
		if( 200 < val ) val = 200;
		stWaveTorqVal.pwr[5]  = val;
		
	}
	else if( '7' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Return Torq
		if( 5 > val ) val = 5;
		if( 80 < val ) val = 80;
		stWaveBdutyVal.rtTorq = val;
	
		val = msg.substring(7,10).toInt();	// Target Torq
		if( 5 > val ) val = 5;
		if( 80 < val ) val = 80;
		stWaveBdutyVal.tgTorq = val;
	
		val = msg.substring(10,(10+4)).toInt();	//on Time
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveBdutyVal.onInterval = val;

		val = msg.substring(14,(14+4)).toInt();	//Off Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stWaveBdutyVal.offInterval = val;
	}

	
	
	else if( '9' == ch )		// Wave Type  Torq SET
	{
		val = msg.substring(4,(4+1)).toInt();	// Torq 1
		if( 0 == val)			// WAVE FUNCTION STOP
		{
			exeWave = 0;
			wave_bbn_step = WAVE_BBN_TORQ_START;
			torqMotor.setValue(DEFAULT_TORQ);
			bbnMotor.offBldc();
		}
		else				// Wave Function Start
		{
			waveType_TorqSet();
			wave_bbn_step = WAVE_BBN_TORQ_START;
			exeWave = 1;
		}
	}
	
	if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) || ('9'==ch) )	//
	{
		
		LogPrintln(" LG] Wave_ContCase: " + String(waveControlCase));
		
		LogPrintln(" LG] Angle1: " + String(stWaveServoVal.angle1) + ", Angle2: " + String(stWaveServoVal.angle2) + ", sInterval: " + String(stWaveServoVal.interval));
		
		LogPrintln(" LG] Torq1: " + String(stWaveTorqVal.rtTorq) + ", Torq2: " + String(stWaveTorqVal.tgTorq) + ", onTime: " + String(stWaveTorqVal.onInterval)+ ", offTime: " + String(stWaveTorqVal.offInterval));
		
		LogPrintln(" LG] Wave_Type: " + String(waveType) + ", Type1: "+ String(stWaveTorqVal.pwr[waveType]) );
	}
	else
	{
		
		LogPrintln(" LG] UnCorrect Format!!");
	}

}

//
//	SET BITE VAL ($F1 K~)  K: Kind
//
void Set_BiteData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		biteLogOut = 1;
	}
	else if( '1' == ch )		// Bite Control Cobination SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 3 < val ) val = 3;
		biteTestCase = val;
	}
	else if( '2' == ch )		// Toq Control	SET ( NotControl / Just/ IncDec/ withFloats)
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 3 < val ) val = 3;
		biteTorqCase = val;
		biteLogOut = 1;
	}
	else if( '3' == ch )		// Control SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 1 > val ) val = 1;
		if( 3 < val ) val = 3;
		biteCnt = val;
	}
	else if( '4' == ch )		// Torq SET
	{
		val = msg.substring(4,7).toInt();	// Torq 1
		if( 60 > val ) val = 60;
		if( 100 < val ) val = 100;
		stBiteTorqVal.shallowTorq = val;
	
		val = msg.substring(7,11).toInt();	// time
		if( 300 > val ) val = 300;
		if( 5000 < val ) val = 5000;
		stBiteTorqVal.shallowInterval = val;
	
		val = msg.substring(11,14).toInt(); // Torq 2
		if( 80 > val ) val = 80;
		if( 150 < val ) val = 150;
		stBiteTorqVal.deepTorq = val;
	
		val = msg.substring(14).toInt();	// Interval
		if( 500 > val ) val = 500;
		if( 5000 < val ) val = 5000;
		stBiteTorqVal.deepInterval = val;
	}
	
	else if( '5' == ch )		// BITEStrong Type SET
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 5 < val ) val = 5;
		biteType = val;
	}
	else if( '6' == ch )		// BITE Type  Torq SET
	{
		val = msg.substring(4,7).toInt();	// Torq 1
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[1] = val;
	
		val = msg.substring(7,10).toInt();	// Torq 2
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[2] = val;
	
		val = msg.substring(10,13).toInt(); // Torq 3
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[3] = val;
	
		val = msg.substring(13,16).toInt(); // Torq 4
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[4] = val;
	
		val = msg.substring(16,19).toInt(); // Torq 5
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[5] = val;
	
		val = msg.substring(19,22).toInt(); // Torq 6
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[6] = val;
	
		val = msg.substring(22).toInt(); // Torq 7
		if( 60 > val ) val = 60;
		if( 150 < val ) val = 150;
		stBiteTorqVal.torqType[7] = val;
		
	}
	
	else if( 'A' == ch )		// Servo DEC Angle
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 0 > val ) val = 0;
		if( 50 < val ) val = 50;
		

		LogPrintln(" LG] Servo - DEC Angle: " + String(val));
	}
	
	if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) )	//
	{
		
		LogPrintln(" LG] BiteConbi: " + String(biteTestCase) + ", BiteTorqControl: " + String(biteTorqCase) + ", BiteCount: " + String(biteCnt));
		//
		// LogPrintln(" LG] Angle1: " + String(stWaveServoVal.angle1) + ",Angle2: " + String(stWaveServoVal.angle2) + ",sInterval: " + String(stWaveServoVal.interval));
		
		LogPrintln(" LG] shallow_Torq: " + String(stBiteTorqVal.shallowTorq) + ", sh_Intval: " + String(stBiteTorqVal.shallowInterval) +", deep_Torq: " + String(stBiteTorqVal.deepTorq) + ", deep_Intval: " + String(stBiteTorqVal.deepInterval));
		
		LogPrintln(" LG] Bite_TorqType: " + String(biteType) + ", Type1: "+ String(stBiteTorqVal.torqType[1]) + ", Type2: "+ String(stBiteTorqVal.torqType[2])  + ", Type3: "+ String(stBiteTorqVal.torqType[3]) + ", Type4: "+ String(stBiteTorqVal.torqType[4])	+ ", Type5: "+ String(stBiteTorqVal.torqType[5])  + ", Type6: "+ String(stBiteTorqVal.torqType[6])	+ ", Type7: "+ String(stBiteTorqVal.torqType[7]) );
	}
	else
	{
		
		LogPrintln(" LG] UnCorrect Format!!");
	}

}


//
void Set_HitData(String msg)
{

			int val = 0;
			char ch = msg.charAt(3);
		
			int i = 0;
		
			if( '0' == ch ) 		// View Set Value [ LOG ]
			{
				// LOG OUT
			}
			else if( '2' == ch )		//Servo Angle
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( 90 < val ) val = 90;
				hitServoAngle = val;
			}
			// TARGET & RESIST
			else if( '3' == ch )		// Target SET
			{
				val = msg.substring(4,7).toInt();	// Taget Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.targetTorq = val;
				stHitTorqVal.holdonTorq = val;
		
				val = msg.substring(7,11).toInt();	// Target Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.targetInterval = val;
				stHitTorqVal.holdonInterval = val;
		
			}
			else if( '4' == ch )		// Low & Mid SET
			{
				val = msg.substring(4,7).toInt();	// low Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.lowTorq = val;
		
				val = msg.substring(7,11).toInt();	// Low Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.lowInterval = val;
		
				val = msg.substring(11,14).toInt(); //Mid Torq
				if( 60 > val ) val = 60;
				if( 250 < val ) val = 250;
				stHitTorqVal.midTorq = val;
		
				val = msg.substring(14).toInt();	// Mid Interval
				if( 100 > val ) val = 100;
				if( 9000 < val ) val = 9000;
				stHitTorqVal.midInterval = val;
			}
			
		  
			else if( '6' == ch )		// BITE Type  Torq SET
			{
				for(i = 0; i < 12; i++)
				{
					val = msg.substring((3*i)+4,(3*i)+7).toInt();	// Torq 
					if( 60 > val ) val = 60;
					if( 250 < val ) val = 250;
					stHitTorqVal.torqType[i+1] = val;
				}
				for(i = 0; i < 12; i++)
				{
					
					LogPrintln(" LG] Hit Tg_Torq " + String(i+1) + " : " + String(stHitTorqVal.torqType[i+1]) );
				}
			}
		
			else if( 'S' == ch )		// Resist & HoldOn Servo Control ENB/DIS
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				//sMotor.saveDecAngle(val);
				exeResistServoENB = val;
				val = msg.substring(6).toInt(); // 6th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(DIS), else =1(ENB)
				exeHoldonServoENB = val;
				
				LogPrintln(" LG] Servo - Resist ServoENB: " + String(exeResistServoENB) + ", HoldOn ServoENB: " + String(exeHoldonServoENB) );
			}
			
			else if( 'K' == ch )		// Finsh KIND SET ( AUTO or SEL)
			{
				val = msg.substring(4,6).toInt(); // 4th 2 char
				if( 0 != val ) val = 1; 	// 0 = 0(Auto), else =1(Fix)
				//sMotor.saveDecAngle(val);
				fishTypeFixENB = val;
				if(fishTypeFixENB)		// if Not Auto = FIX SEL
				{
					val = msg.substring(6).toInt(); // 6th 2 char
					if( 1 > val ) val = 1;
					if( 15 < val ) val = 15;
					fishTypeFix = val;
				}
				
				LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
			}
		
			if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) )	//
			{
				
				LogPrintln(" LG] HitAngle: " + String(hitServoAngle) );
				
				
				LogPrintln(" LG] hit_tg_Torq: " + String(stHitTorqVal.targetTorq) + ", hit_tg_Intval: " + String(stHitTorqVal.targetInterval) );
				
				LogPrintln(" LG] hit_low_Torq: " + String(stHitTorqVal.lowTorq) + ", hit_low_Intval: " + String(stHitTorqVal.lowInterval) +  ", hit_mid_Torq: " + String(stHitTorqVal.midTorq) + ", hit_mid_Intval: " + String(stHitTorqVal.midInterval) );
			}
			else
			{
				
				LogPrintln(" LG] UnCorrect Format!!");
			}
}


//
void Set_BreakData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		
		LogPrintln(" LG] WaveType(1~3): " + String(waveType));
		
		LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
		
		LogPrintln(" LG] Hit Servo - Resist ENB: " + String(exeResistServoENB) + ", HoldOn ENB: " + String(exeHoldonServoENB) + ", Hit_Angle: " + String(hitServoAngle));
		
		LogPrintln(" LG] Hit_Resist_FIX: " + String(torqResistFix) + ", Torq Target: " + String(stHitTorqVal.targetTorq) + ", Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
		
		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}
	else if( '1' == ch )		// SET Break Type
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( 1 > val ) val = 1;
		if( 3 < val ) val = 3;
		breakType = val;

		
		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}

/*
	else if( 'A' == ch )		// Resist Auto & FIX
	{
		val = msg.substring(4).toInt(); //
		if( 0 > val ) val = 0;
		if( 1 < val ) val = 1;
		torqResistFix = val;	// Auto(0), Fix(1)
	}
*/

}

//
// $F5 : STX_EXTLED_SET
//	$F5 1x - LED CONT ENB/DIS
//	$F5 2xxxx - Dimmming Time
//	$F5 3xxxx - HoldCount
//
void Set_ExtledData(String msg)
{
			int val = 0;
			int para1 = 0;
			char ch = msg.charAt(3);
	
			int i = 0;
		
			if( '0' == ch ) 		// View Set Value [ LOG ]
			{
				// LogPrintln();
			}
		//
		//
			else if( '1' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;		
				 ledControlMode = val;	// LED CONTROL ENB/DIS
			}
			else if( '2' == ch )		// 
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 1 > val ) val = 1;
				if( 60 < val ) val = 60;
				dimming_time = val;			// [sec]
			}
			else if( '3' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 100 > val ) val = 100;
				if( 5000 < val ) val = 5000;
				ledoffHoldTimeCnt = (unsigned int)(val/10);
			}
		/*
			else if( '4' == ch )		//
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
				if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
				sBbnActTbl[bbnActKind].offTime = (unsigned long)val;
			}
	
			else if( '5' == ch )		// TOKTOK KIND
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if( 0 > val ) val = 0;
				if( BBNACT_TBLMAX <= val ) val = 0; 	// default (SETTING)
				bbnActKind = (unsigned long)val;
			}
	
			else if( '6' == ch )		// RPT CNT
			{
				para1 = msg.substring(4,(4+1)).toInt(); // 4th 2 char
				if((BBNACT_TBLMAX-1) < para1) { para1 = (BBNACT_TBLMAX-1); }
				val = msg.substring(5).toInt(); // 5th 3 char
				if( BBN_RPT_CNT_MIN > val ) { val = BBN_RPT_CNT_MIN; }
				if( BBN_RPT_CNT_MAX < val ) { val = BBN_RPT_CNT_MAX; }
				bbnReqCnt[para1] = (unsigned long)val;
			}
			else if( '7' == ch )		// Motor WAT SET
			{
				val = msg.substring(4).toInt(); // 4th 2 char
				if(1 > val ) val = 1;
				if( 6 < val ) val = 6;
				torqMotor.torqWattAdj = val;
			}
			else
			{
				// Error
			}
		*/
		
			if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) ) //
			{
				
				LogPrintln(" LG] EXTLED_SET ENB : " + String(ledControlMode) + " [sec]");
				LogPrintln(" LG] EXTLED_SET dimm : " + String(dimming_time) + " [sec]");
				LogPrintln(" LG] EXTLED_SET hold : " + String(ledoffHoldTimeCnt*10) + " [ms]");
			/*	
				LogPrintln(" LG] BBN TokTok ACT KIND: " + String(bbnActKind) + " [ 0~6 ]");
				
				LogPrintln(" LG] BBN TokTok Rpt REQ CNT: " + String(bbnReqCnt[bbnActKind]) + " [ 1 ~ 100 ] Times");
	
				
				LogPrintln(" LG] BLDC TokTok On Duty: " + String(sBbnActTbl[bbnActKind].onDuty) + " / 255 Max");
				
				LogPrintln(" LG] BLDC TokTok On Time: " + String(sBbnActTbl[bbnActKind].onTime) + " ms");
				
				
				LogPrintln(" LG] BLDC TokTok Off Duty: " + String(sBbnActTbl[bbnActKind].offDuty) + "  / 255 Max");
				
				LogPrintln(" LG] BLDC TokTok Off Time: " + String(sBbnActTbl[bbnActKind].offTime) + " ms");
			*/
			}
			else
			{
				
				LogPrintln(" LG] UnCorrect Format!!");
			}
	
}

//
void Set_BbnTokTokData( String msg)
{
		int val = 0;
		int para1 = 0;
		char ch = msg.charAt(3);

		int i = 0;
	
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			// LogPrintln();
		}
	//
		else if( '5' == ch )		// TOKTOK KIND
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( 0 > val ) val = 0;
			if( BBNACT_TBLMAX <= val ) val = 0;		// default (SETTING)
			bbnActKind = (unsigned long)val;
		}

		else if( '6' == ch )		// RPT CNT
		{
			para1 = msg.substring(4,(4+1)).toInt(); // 4th 2 char
			if((BBNACT_TBLMAX-1) < para1) { para1 = (BBNACT_TBLMAX-1); }
			val = msg.substring(5).toInt(); // 5th 3 char
			if( BBN_RPT_CNT_MIN > val ) { val = BBN_RPT_CNT_MIN; }
			if( BBN_RPT_CNT_MAX < val ) { val = BBN_RPT_CNT_MAX; }
			bbnReqCnt[para1] = (unsigned long)val;
		}
	//
		else if( '1' == ch )		// 
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_PWM_DUTY_MIN > val ) val = BBN_PWM_DUTY_MIN;
			if( BBN_PWM_DUTY_MAX < val ) val = BBN_PWM_DUTY_MAX;
			sBbnActTbl[bbnActKind].onDuty = val;
		}
		else if( '2' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
			if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
			sBbnActTbl[bbnActKind].onTime = (unsigned long)val;
		}
		else if( '3' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_PWM_DUTY_MIN > val ) val = BBN_PWM_DUTY_MIN;
			if( BBN_PWM_DUTY_MAX < val ) val = BBN_PWM_DUTY_MAX;
			sBbnActTbl[bbnActKind].offDuty = val;
		}
		else if( '4' == ch )		//
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if( BBN_WAIT_TIME_MIN > val ) val = BBN_WAIT_TIME_MIN;
			if( BBN_WAIT_TIME_MAX < val ) val = BBN_WAIT_TIME_MAX;
			sBbnActTbl[bbnActKind].offTime = (unsigned long)val;
		}

		else if( '7' == ch )		// Motor WAT SET
		{
			val = msg.substring(4).toInt(); // 4th 2 char
			if(1 > val ) val = 1;
			if( 6 < val ) val = 6;
			torqMotor.torqWattAdj = val;
		}
		else
		{
			// Error
		}
	
		if( ('0'==ch) || ('1'==ch)	|| ('2'==ch) || ('3'==ch) || ('4'==ch) || ('5'==ch) || ('6'==ch) || ('7'==ch) )	//
		{
			LogPrintln(" LG] TORQ MOT WATT : " + String(torqMotor.torqWattAdj) + " [ 1:10W,2:20W,3:30W]");
			LogPrintln(" LG] BBN TokTok ACT KIND: " + String(bbnActKind) + " [ 0~6 ]");
			LogPrintln(" LG] BBN TokTok Rpt REQ CNT: " + String(bbnReqCnt[bbnActKind]) + " [ 1 ~ 100 ] Times");
			LogPrintln(" LG] BLDC TokTok On Duty: " + String(sBbnActTbl[bbnActKind].onDuty) + " / 255 Max");
			LogPrintln(" LG] BLDC TokTok On Time: " + String(sBbnActTbl[bbnActKind].onTime) + " ms");
			LogPrintln(" LG] BLDC TokTok Off Duty: " + String(sBbnActTbl[bbnActKind].offDuty) + "  / 255 Max");
			LogPrintln(" LG] BLDC TokTok Off Time: " + String(sBbnActTbl[bbnActKind].offTime) + " ms");
		}
		else
		{
			LogPrintln(" LG] UnCorrect Format!!");
		}
}

/*

*/
void Set_TorqAdjValData( String msg)
{
		int val = 0;
		char ch = msg.charAt(3);
		
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			
			LogPrintln(" LG] Type: " + String(ch) + ",	");
			
			LogPrintln(" LG] *Torq Adj Curr Val: " + String(torqMotor.setAdjTorq));
		}
		else if( '1' == ch )		// SET Torq Default Val
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 30 > val ) val = 30;
			if( 80 < val ) val = 80;
			torqMotor.defaultVal = val;
			
			LogPrintln(" LG] Set Torq Default Val, *ADJ Curr Val : " + String(torqMotor.setAdjTorq) );
			
			LogPrintln(" LG] *ADJ Set Val : " + String(val) );
		}
	
		else if( '2' == ch )		// SET Torq Adj(Dec) Val
		{
			val = msg.substring(4,6).toInt(); // 4th 2 char
			if( 1 > val ) val = 1;
			if( 50 < val ) val = 50;
			
			LogPrintln(" LG] Set Torq ADJ Val, *ADJ Curr Val : " + String(torqMotor.setAdjTorq) );
			
			LogPrintln(" LG] *ADJ Set Val : " + String(val) );
			torqMotor.setAdjTorq = val;
		}
	/*
		else if( 'A' == ch )		// Resist Auto & FIX
		{
			val = msg.substring(4).toInt(); //
			if( 0 > val ) val = 0;
			if( 1 < val ) val = 1;
			torqResistFix = val;	// Auto(0), Fix(1)
		}
	*/
		else
		{
			
			LogPrintln(" LG] Set Torq Undefine Val, " + String(msg) );
		}

}


//
//	$FAA~
//	A - Action
//		0 - Log View
//		1 - Light
//			~ : xxx : Light 10~100 [%]
//
void Set_LedValue( String msg)
{
		unsigned int val = 0;
		int pos = 0;
		char ch = msg.charAt(3);
		
		if( '0' == ch ) 		// View Set Value [ LOG ]
		{
			
		}
		else if( '1' == ch )		// SET Led Light
		{
			val = (unsigned int)msg.substring(5,(5+3)).toInt(); // 4th 2 char
			if( 1 > val ) val = 1;
			if( 100 < val ) val = 100;
			ledLight = val;
		}

		
		else if( '2' == ch )		// SET HoldTIme $FA2xZZZZ
		{
			pos = (unsigned int)msg.substring(4,(4+1)).toInt(); // 4th 1 char, led Pos
			if( 0> pos) { pos=0; } if( 4<pos ) { pos =0; }
			
			val = (unsigned int)msg.substring(5,(5+4)).toInt();	// 5th~9th 4_Char = Hold TIme
			if( 10 > val ) val = 10;
			if( 9999 < val ) val = 9999;

			ledDimmHoldTime[pos] = val;
			
		}
	/*
		else if( 'A' == ch )		// Resist Auto & FIX
		{
			val = msg.substring(4).toInt(); //
			if( 0 > val ) val = 0;
			if( 1 < val ) val = 1;
			torqResistFix = val;	// Auto(0), Fix(1)
		}
	*/
		else
		{
			
			LogPrintln(" LG] Set Torq Undefine Val, " + String(msg) );
		}

	if( '0' == ch || '1'==ch || '2'==ch || '3'==ch) 		// View Set Value [ LOG ]
	{
		LogPrintln(" LG] Set LED Light= " + String(ledLight) + "/%" );
		LogPrintln(" LG] Set LED HoldTime 1234: " + String(ledDimmHoldTime[1]) + "," + String(ledDimmHoldTime[2]) + "," + String(ledDimmHoldTime[3]) + "," + String(ledDimmHoldTime[4]) + ","  );
	}
}

// MKT TUN HIT & HOLD
// $F9aXXXyy
//	a : 0- Val view, 1-hit, 2-hold, 3-Test Stop
// XXX : Main Mot Power ( 60 ~ 255)
// yy : Bobbin Mot Power (10 ~ 70)
void Set_TorqHitHoldOn( String msg)
{


}

void Set_EnbData( String msg)
{
	int val = 0;
	char ch = msg.charAt(3);
	
	if( '0' == ch ) 		// View Set Value [ LOG ]
	{
		LogPrintln(" LG] WaveType(1~3): " + String(waveType));
		LogPrintln(" LG] Fish Auto(0)Fix(1): " + String(fishTypeFixENB) + ", FishType(Fix): " + String(fishTypeFix) + ", FishType(Auto): " + String(fishTypeAuto) );
		LogPrintln(" LG] Hit Servo - Resist ENB: " + String(exeResistServoENB) + ", HoldOn ENB: " + String(exeHoldonServoENB) + ", Hit_Angle: " + String(hitServoAngle));
		LogPrintln(" LG] Hit_Resist_FIX: " + String(torqResistFix) + ", Torq Target: " + String(stHitTorqVal.targetTorq) + ", Resist: " + String(stHitTorqVal.holdonTorq) + ", low: " + String(stHitTorqVal.lowTorq) + ", Mid: " + String(stHitTorqVal.midTorq) );
//		LogPrintln(" LG] Hit_Break, Break_Type: " + String(breakType) );
	}
	else if( '1' == ch )		// SET Level (Easy/Normal/Hard)
	{
		val = msg.substring(4).toInt(); // 4th 2 char
		if( LEVEL_MIN > val ) val = LEVEL_MIN;
		if( LEVEL_MAX < val ) val = LEVEL_MAX;
		gameLevel = val;
		
		LogPrintln(" LG] game Level, gameLevel: " + String(gameLevel) );
	}

	else if( 'A' == ch )		// Resist Auto & FIX
	{
		val = msg.substring(4).toInt(); //
		if( 0 > val ) val = 0;
		if( 1 < val ) val = 1;
		torqResistFix = val;	// Auto(0), Fix(1)
	}
}


//=== TEST Seral Event ===


// TO 10SEC Force AC OFF
#define POWER_ON_WAIT		(DEF_PWRON_DELAY_TIME*1000)	// 10 SEC
#define BOOTING_WAIT		(DEF_USB_SAFE_TIME*1000)	// 28 SEC
#define FORCE_OFF_2MIN		(2*60*1000)				// 2 MIN ( with Windows Update)
#define RESP_WAIT_TIME		(500)					// 500ms After USB5VOFF
#define AUTO_REBOOT_TIME	(14*1000)				// 14초 After AP Running
#define APP_CHECK_TIME		(18*1000)				// APP 커맨드 체크 타임(18초동안 폴커맨드등 없을 때) 
#define ACOFF_500MS_AFTER_USB5VOFF	(500)			// 500ms After USB5VOFF
#define TO_EDGE_FILTER_CNT	5						// 50 ms ( 20ms + 30 ms After LE/TE Occure]
#define	PWR_SW_ON		1
#define	PWR_SW_OFF		2

int	g_ProcStatus = 0;					// INIT
static unsigned long	g_TmRTM = 0;
static unsigned long	g_TmNTM = 0;
static unsigned long	g_TmFTM = 0;
static unsigned long	g_TmBTM = 0;
static unsigned int	g_PSON_Count = 0;
static unsigned int	g_PSOF_Count = 0;
static unsigned int	g_USB5V_Count = 0;
//
void subAcOff_WithOtherOff()
{
	subAC_Off();		// OFF Output
	// LED OFF
	ledOff_MbCent();
	ledOff_MbBtm();
	ledOff_OptLeft();
	ledOff_OptRight();
	g_RecvCmdCount = 0;
//	g_FlagCmdStop = 0;
}

int CheckAppCmd()
{
//	if (g_FlagCmdStop && (g_RecvCmdCount < 2))							// 종료 커맨드 수신이고 커맨드 개수가 2미만이면
//		return PS_STOPPING;												// AP 종료중 리턴
//	else
	{
		if ((g_RecvCmdCount > 2) && !checkTO(g_LastCmdTick, 4000)) {	// 커맨드 수신 카운트가 2보다 크고 최종수신 시간이 4초이내이면
//			g_FlagCmdStop = 0;											// AP 종료중 플래그 클리어
			return 1;													// AP 실행중 리턴
		}
	}
	return 0;
}

int CheckPowerSwitch()
{
	if (LVL10_PWROFF_SWC)						// 전원스위치 OFF(HIGH-OFF) 이면
	{
		g_PSON_Count=0;							// On Count clear
		g_PSOF_Count++;							// Off Count +1
		if (g_PSOF_Count > TO_EDGE_FILTER_CNT)	// Off Count가 5보다 큰가
			return PWR_SW_OFF;					// 전원 스위치 OFF 리턴
	}
	else
	{
		g_PSON_Count++;							// On Count +1
		g_PSOF_Count=0;							// Off Count clear
		if (g_PSON_Count > TO_EDGE_FILTER_CNT)	// On Count가 5보다 큰가
			return PWR_SW_ON;					// 전원 스위치 ON 리턴
	}
	return 0;
}

static String StsStr[] = {  "PS_INITIAL", "PS_ON_WAIT", "PS_BOOTING", "PS_RUNNING",
					 "PS_RSPWAIT", "PS_USBWAIT", "PS_OFFWAIT" };
#define	SnsStr(x)	("ACOFF("+String((x >> SNS_AcOff) & 1) + ") USB5V("+String((x >> SNS_usb5V) & 1) + ") PWOFF("+String((x >> SNS_PwrOffSwc) & 1)+")")

void check_PowerSwitch_Change_10ms()
{
	static int	SaveStatus = -1;
	static	int	oldSns = 0;
	int newSns = LVL10_PWRALL;
	int chgSns = oldSns ^ newSns;
	if (chgSns) {
		LogPrintln(" LG] CHKSNS : " + SnsStr(newSns));
		oldSns = newSns;
	}
	switch (g_ProcStatus)
	{
	case PS_INITIAL :		// 초기 상태
		if (CheckPowerSwitch() == PWR_SW_ON) {		// 전원 스위치 ON이면
			initSet_LedContVal(1);					// LED VAL INIT
			ledOn_MbBtm();							//
			pwrMode = PWR_ON;						// LED를 켠다.
			g_ProcStatus = PS_ON_WAIT;				// PC 안정화 시간(10초) 보장 PC ON 대기로 상태 전환
		}
		else {
			if ((g_SerialEnable == 0) && 			// USB CDC가 disable 이면서
				checkTO(g_TmBTM, BOOTING_WAIT)) {	// 부팅 안정화 시간이 지났으면
				SerialPortEnable();					// USB CDC Enable
			}
		}
		break;
	case PS_ON_WAIT :		// PC ON 대기
		if (CheckPowerSwitch() == PWR_SW_OFF) {		// 전원 스위치 OFF이면
		//	initSet_LedContVal();					// LED VAL INIT
			extLed_AllOff();
			extLed_controlOff();
			pwrMode = PWR_OFF;						// LED를 끈다.
			g_ProcStatus = PS_INITIAL;				// PC 안정화 시간(10초) 보장 PC ON 대기로 상태 전환
		}
		else if (checkTO(g_TmNTM, POWER_ON_WAIT)) {	// 안정화 시간이 지났으면
			if (g_SerialEnable) {
				SerialPortDisable();				// USB CDC Disable
			}
			subAC_On();								// PC 등 AC 전원 ON
			setTO(g_TmBTM);							// PC 부팅 타이머 설정
			g_ProcStatus = PS_BOOTING;				// PC 부팅중으로 상태 전한
		}
		break;
	case PS_BOOTING :		// PC 부팅중
		if (g_SerialEnable || checkTO(g_TmBTM, BOOTING_WAIT)) {		// 부팅 안정화 시간이 지났으면
			if (g_SerialEnable == 0) {
				SerialPortEnable();					// USB CDC Enable
			}
			g_ProcStatus = PS_RUNNING;				// PC 부팅중으로 상태 전한
		}
		break;
	case PS_RUNNING :		// AP 실행중
		if ((CheckPowerSwitch() == PWR_SW_OFF) ||	// 
			(g_AutoReboot && CheckAppCmd() && checkTO(g_TmFTM, AUTO_REBOOT_TIME))) {	// TEST AUTO REBOOT
			recvPwrOffExec_Resp = 0;				// 수신 플래그 클리어
			StsSendPrintln(STS_APSTS_REQ+"00%");	// PC OFF REQ 커맨드 송신
			setTO(g_TmFTM);							// FTM(2분) 설정(강제종료 타이머)
			setTO(g_TmRTM);							// RTM(500ms) 설정(응답 타이머)
			g_ProcStatus = PS_RSPWAIT;				// 응답 대기로 상태 전환
		}
		break;
	case PS_RSPWAIT :		// 응답 대기
		if(recvPwrOffExec_Resp)						// PC off 커맨드 응답 수신이면
		{
			recvPwrOffExec_Resp = 0;				// 수신 플래그 클리어
			g_ProcStatus = PS_USBWAIT;				// USB 5V LOW 대기
		}
		else if (checkTO(g_TmFTM, FORCE_OFF_2MIN))	// 강제 종료 시간(2분) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		else if (checkTO(g_TmRTM, RESP_WAIT_TIME))	// 응답 시간(500ms) 경과이면
		{
			if ((CheckPowerSwitch() == PWR_SW_ON)	// 전원 스위치가 다시 On이고
				&& (CheckAppCmd() == 0)) {			// AP가 실행 안되고 있으면
				g_ProcStatus = PS_RUNNING;			// 초기 상태로 전환
			}
			else {
				StsSendPrintln(STS_APSTS_REQ+"00%");// PC OFF REQ 재전송
				setTO(g_TmRTM);						// RTM(500ms) 설정(응답 타이머)
			}
		}
		break;
	case PS_USBWAIT :		// USB 5V LOW 대기
		if(LOW == LVL10_USB5V)						// USB 5V가 LOW 인가
		{
			setTO(g_TmRTM);							// RTM(500ms) 설정(전원OFF 대기 시간)
			g_ProcStatus = PS_OFFWAIT;				// 전원 OFF 대기 상태로 전환
		}
		else if (checkTO(g_TmFTM, FORCE_OFF_2MIN))	// 강제 종료 시간(2분) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		break;
	case PS_OFFWAIT :		// 전원 OFF 대기
		if (checkTO(g_TmRTM, RESP_WAIT_TIME))		// 전원OFF 대기 시간(500ms) 경과이면
		{
			subAcOff_WithOtherOff();				// AC 전원등 전부 Off 처리
			setTO(g_TmNTM);							// PC 안정화 타이머 설정
			g_ProcStatus = PS_INITIAL;				// 초기 상태로 전환
		}
		break;
	default :				// 디폴트(정의 안된 상태의 경우)
		LogPrintln(" LG] CHKPWR : PS_UNKNOWN("+ String(g_ProcStatus) + ") -> PS_INITIAL");
		g_ProcStatus = PS_INITIAL;					// 초기 상태로 전환
		break;
	}
	if (SaveStatus != g_ProcStatus) {
		LogPrintln(" LG] CHKPWR : " + ((SaveStatus==-1) ? "PS_UNKNOWN" : StsStr[SaveStatus]) + " -> " + StsStr[g_ProcStatus]);
		SaveStatus = g_ProcStatus;
	}
}


// IMU Data START/STOP Cmd Send to ROD
