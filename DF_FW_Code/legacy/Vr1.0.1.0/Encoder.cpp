//
//
//

#include "Common.h"
#include "Encoder.h"


void (*_rotateCallback)(String);

Encoder::Encoder()
{

}

void Encoder::init()
{
	//--Encoder INPUT
	//#if (ROD_BOARD_V2)
	//if(isRodNewBoard())
	if(REEL_ENC_V2 <= getEncType())		// NEW(Vr1.0.1.0)
	{
		pinMode(ENC_HALL_SEN_PIN, INPUT_PULLUP);
		lastCLK = digitalRead(ENC_HALL_SEN_PIN);
	}
	else		// V1 = OLD OD
	{
		pinMode(CLK_PIN, INPUT_PULLUP);
		pinMode(DT_PIN, INPUT_PULLUP);
		pinMode(BTN_PIN, INPUT_PULLUP);
		lastCLK = digitalRead(CLK_PIN);
	}
}

//callback Function Name : rotateChangeCallback
void Encoder::setRotateCallback(void (*rotateCallback)(String))
{
  _rotateCallback = rotateCallback;
}


//
//
void Encoder::rotate()
{

// V2 = NEW ROD
//#if (ROD_BOARD_V2)
//if(isRodNewBoard())
if(REEL_ENC_V2 <= getEncType()) 	// NEW(Vr1.0.1.0)
{
	currentCLK = digitalRead(ENC_HALL_SEN_PIN);

	// 2체배, LE and TE ,  6.3 Pulse / Rev =>  12.6 Pulse/Rev (*2) = 25.2 Pulse (Count * 2 ) => 12.6 Pulse/Rev ( Count 1/2)
	if (lastCLK != currentCLK)
	{
		// Change Encoder Direction
		count++;
		//count++;		// SendCount = 1/2
		//if (count > MAX_COUNT)
		//if (count > MAX_COUNT_V2)
		if (count > MAX_COUNT_VX)			// (Vr1.0.1.0)
		{
			count = 0;
		}
		// Sned Only Changed, VAL = PULSE/2
    	sendCount();
  	}
	lastCLK = currentCLK;
}

// V1 = OLD ROD
else
{
  currentCLK = digitalRead(CLK_PIN);
  currentDT = digitalRead(DT_PIN);
  // 1체배, LE Only, => 20 pulse/Rev => 10 Pulse / Rev ( Count 1/2 )
  if ((lastCLK != currentCLK)  && (HIGH == currentCLK))
  {
    if (currentDT == currentCLK)
    {
	  // Change Encoder Direction
      count++;
      if (count > MAX_COUNT)
      {
        count = 0;
      }
    }
    else
    {
	  // Change Encoder Direction
		count++;
		if (count > MAX_COUNT)
		{
	  		count = 0;
		}
    }
	// Sned Only Changed, VAL = PULSE/2
    sendCount();
  }
  lastCLK = currentCLK;
}

}


String Encoder::fillZero2char(int _count)
{

  String str;
  if(0) {}
  
  else if (10 > _count)		//  0 + 1char(1~9)
  {
    str = "0" + String(_count);
  }
  else					//  2char (10 ~ 99)
  {
    str = String(_count);
  }
  return str;
}


/*------------------------------------------------------------
  1. VAL = Pulse /2
  2. Send Only Change
------------------------------------------------------------*/
String Encoder::sendCount()
{
	String str = "";
	int divCount;

	//if(isRodNewBoard())
	if(REEL_ENC_V2 <= getEncType())					// NEW(Vr1.0.1.0)
	{
		divCount = count * ENC_MUL_VX / ENC_DIV_VX;	// count = Pulse * 10/12 (/1.2)		// (Vr1.0.1.0)
		divCount = divCount / DIVISION_VAL;		// Pulse /2
	}
	else
	{
		divCount = count / DIVISION_VAL;			// Pulse /2
	}

	if ( 0 < divCount)			// +
	{
		str = "+" + fillZero2char(divCount);
	}
	else if ( 0 > divCount)		// -
	{
		str = "-" + fillZero2char(abs(divCount));
	}
	else	// if 0
	{
		str = "000";				// else 000
	}

	// Send MSG, Only Change
	if (!str.equals(oldStr))			// if Change String(CNT)
	{
		oldStr = str;
		//str = "$0700000" + str + "%";

		if (_rotateCallback != nullptr)
		{
			_rotateCallback(str);
		}
	}
	return str;
}


