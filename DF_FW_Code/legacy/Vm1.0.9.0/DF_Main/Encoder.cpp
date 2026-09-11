//
//
//

#include "Common.h"
#include "Encoder.h"


void (*_rotateCallback)(int _dir, int _cnt);

Encoder::Encoder()
{

}

void Encoder::init()
{
  last_ACLK = (digitalRead(BBN_ENC_A_CK_PIN)? 1:0);
  last_BDT = (digitalRead(BBN_ENC_B_DT_PIN)? 1:0);
  #if (IO_LM_MOT)
  last_lmEnc = (digitalRead(LM_ENC_SEN_PIN)? 1:0);
  #endif
}

//callback Function Name : rotateChangeCallback
void Encoder::setRotateCallback(void (*rotateCallback)(int _dir, int _cnt))
{
  _rotateCallback = rotateCallback;
}

//=== Define BBN DIR 
//#define BBN_CW_DIR HIGH
//#define BBN_CW_DIR LOW
#define BBN_CW_DIR HIGH

/*
// DIR - CW/CCW는 엔코더의 회전방향이 아니라 보빈의 회전방향임.
//	call - 1ms 모니터
*/
void Encoder::rotate()
{
	int changeEnc = 0;
	//String _str;

//***  TBD Input Filtering Control  
	//====1) READ PORT INPUT
	current_ACLK = (digitalRead(BBN_ENC_A_CK_PIN)? 1:0);
	current_BDT = (digitalRead(BBN_ENC_B_DT_PIN)? 1:0);
#if (IO_LM_MOT)
	current_lmEnc = (digitalRead(LM_ENC_SEN_PIN)? 1:0);
#endif

	//[1] =====BBN Encoder WITH DIR=============
	// Change CLK ,  CLK:H DT:H - CW 
	if(current_ACLK != last_ACLK)
  	{
		// Update Old Data
  		last_ACLK = current_ACLK;

		// 1) CW(당김 돌림)
  		if( ((HIGH == current_ACLK) && ( BBN_CW_DIR == current_BDT ))		// LEAD EDGE & LOW
		//|| 	(( LOW == current_ACLK) && ( BBN_CW_DIR != current_BDT ))		// TAIL EDGE & HIGH
		)	
		{
			// Change Encoder Direction
		#if ( BBN_ENC_DIR_INVERT)
			bbnDir = BBN_ENC_CCW;
	    	count++;
			bbnCnt++;
			//bbnDir = BBN_ENC_CW;
			if (MAX_COUNT < bbnCnt  )
	   		{
	        	count = MAX_COUNT;
				bbnCnt = MAX_COUNT;
			}
		#else
			bbnDir = BBN_ENC_CW;
 	    	count--;
			bbnCnt--;
 	  		if (MIN_COUNT > bbnCnt)
    		{
   		    	count = MIN_COUNT;
				bbnCnt = MIN_COUNT;
    		}
		#endif
			changeEnc = 1;
	  	}

		// 2) CCW (풀림)
	    else if ( ((HIGH == current_ACLK) && ( BBN_CW_DIR != current_BDT ))		// LEAD EDGE &  CCW
    	//||	  (( LOW == current_ACLK) && ( BBN_CW_DIR == current_BDT ))		// TAIL EDGE & LOW
		)
		{
		  // Change Encoder Direction
		#if ( BBN_ENC_DIR_INVERT)
			//bbnDir = BBN_ENC_CCW;
			bbnDir = BBN_ENC_CW;
 	    	count--;
			bbnCnt--;
   			if (MIN_COUNT > bbnCnt)
 	   		{
 	    		count = MIN_COUNT;
				bbnCnt = MIN_COUNT;
 	    	}
		#else
			bbnDir = BBN_ENC_CCW;
    		count++;
	 		bbnCnt++;
			if (MAX_COUNT < bbnCnt  )
 	   		{
 	    		count = MAX_COUNT;
				bbnCnt = MAX_COUNT;
 	   		}
		#endif
			changeEnc = 1;
    	}
	
	}
  
	//--- 3) 변화확인 : BBN DIR JUSGEMENT
	if(changeEnc)  // Sned Only Changed, VAL = PULSE/2
	{
		changeEnc = 0;
		String dirStr = (BBN_ENC_CW == bbnDir)? "CW_" : "CCW";

		#if (LOG_BBN_ENC_LOGOUT)	// LOG OUT
			LogPrintln(" lg] bbnENC: " + dirStr + "," + String(bbnCnt));
		#endif
	}

	//---4) Not Use
	if(last_BDT != current_BDT)
	{
    	last_BDT = current_BDT;
	}

#if (IO_LM_MOT)
	// [2] ========= LM ENC COUNT ONLY=========
	if(last_lmEnc != current_lmEnc) 	// LE ONLY (1체배)
	{
		last_lmEnc = current_lmEnc;
		if(HIGH == current_lmEnc)		// LE ONLY
		{
			lmCnt++;	// FREE RUN( 4 byte)
		  #if (LOG_LM_ENC_LOGOUT)	// LOG OUT
			LogPrintln(String(lmCnt));
		  #endif
		}
	}
	//==
#endif
}


String Encoder::fillZero(unsigned int _count)
{
  String str;
  if (_count < 10)
  {
    str = "0000" + String(_count);
  }
  else if (_count < 100)
  {
    str = "000" + String(_count);
  }
  else if (_count < 1000)
  {
    str = "00" + String(_count);
  }
  else if (_count < 10000)
  {
    str = "0" + String(_count);
  }  else
  {
    str = String(_count);
  }
  return str;
}

/*------------------------------------------------------------
  1. VAL = Pulse /2
  2. Send Only Change
------------------------------------------------------------*/
#if (NEW_IF)
#define STX_BBN_ENC_RESP	"$FF00"
#endif

#if 0
String Encoder::sendCount(int _dir, unsigned int _count)
{
  String str = "";

  str = "+" + fillZero(_count);


  if (!str.equals(oldStr))			// if Change String(CNT)
  {
    oldStr = str;
	// TBD Not Define STS
  #if (NEW_IF)
    str = STX_BBN_ENC_RESP + str + "%";
  #else
    str = "$0B00" + str + "%";
  #endif
    
    if (_rotateCallback != nullptr)
    {
      _rotateCallback(_dir, _count);
    }
  }
  return str;
}
#endif


void Encoder::setBbnCnt(unsigned int _cnt) 
{
	bbnCnt = _cnt;
	LogPrintln(" lg] bnEnc Cnt:" + String(bbnCnt));
}

void Encoder::setLmCnt(unsigned int _cnt) 
{
	lmCnt = _cnt;
	LogPrintln(" lg] LmEnc Cnt:" + String(lmCnt));
}

