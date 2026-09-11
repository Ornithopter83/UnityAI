//
//
//

#include "Common.h"
#include "Diag.h"

//=================================
void mDiag_entry()
{
	mainMode = DIAG_MODE;
	//ledControlMode = 0;

	if( oldmainMode != mainMode)
	{
		oldmainMode = mainMode;

		LogPrintln(" LG] DIAG_ Entry");
		// Motor OFF
		motor_AllOff();
		// LED OFF
		//extLed_AllOff();
		extLed_controlOff();
		
		reelOut_AllOff();	// REEL ALL OFF
	}

}

// TBD , 어디서 어떻게 Call처리할까(IF사양?)  <DIAG_MODE>의  {ENTRY] [EXIT] 버튼 작성

void mDiag_exit()
{
	diagInput_AllOff();
	diagOutput_AllOff();

	initSet_LedContVal();		  // LED VAL INIT
	//ledControlMode = 1;
	ledOn_MbBtm();

	mainMode = NORMAL_MODE;

	LogPrintln(" LG] DIAG_ EXIT");

	// DIAG FUNC 중지 처리 필요(?)
}

//=================================


