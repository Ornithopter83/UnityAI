//
//  Break Motor ( =SPI PotentioMeter )
//

#include "Common.h"
#include "Potentiometer.h"

Potentiometer::Potentiometer()
{

}

void Potentiometer::init()
{
	//1) SET SPI
  SPI.begin(SPI_SCK, -1, SPI_MOSI, SPI_SS);
  pinMode (SPI_SS, OUTPUT);
  digitalPortWrite(SPI_SS, DEFAULT_VALUE);
  //digitalPortWrite(SPI_SS, DEFAULT_VALUE_255);			// Out 0 = (255 - Default255) 
  
	// 2) PWM SET - not use  
#if (0)		// not USE, Delete
  ledcSetup(0, 5000, 8);					// PWM Set ( Channel no, Freq, Resolution bit)
  ledcAttachPin(PWM_PIN, 0);				// PWM Set ( Pin No, Channel no)
#endif
}

/*--------------------------------------------------------------------------------
	Output SPI Value to BreakMotor Potentio Value
--------------------------------------------------------------------------------*/
void Potentiometer::digitalPortWrite(int _pinNum, int _val)
{
  digitalWrite(_pinNum, LOW);
  SPI.transfer(B00010001);
  SPI.transfer(_val);
  digitalWrite(_pinNum, HIGH);
}

/*--------------------------------------------------------------------------------
	Output Break Motor Value (STRING)
--------------------------------------------------------------------------------*/
void Potentiometer::setValue(String _str)
{
	//int _val = _str.substring(8).toInt();		// 9th ~11th Char (0~255)
	int _val = _str.substring(0).toInt();		// 9th ~11th Char (0~255)

	outValue(_val);
}

void Potentiometer::setValue(int _val)
{
	outValue(_val);
}

/*--------------------------------------------------------------------------------
	Output Break Motor Value (INT)
--------------------------------------------------------------------------------*/
void Potentiometer::outValue(int _val)
{
  if (_val < RECV_MIN)
  {
    _val = RECV_MIN;
  }

  if (_val > RECV_MAX)
  {
    _val = RECV_MAX;
  }

	//1) NOT_USE : Out PWM
#if (0)		// Delete , Not USE
	ledcWrite(0, val);
#endif
  
  //2) Out SPI   Output Value = 255 - Req Val
  _val = 255 - _val;
  digitalPortWrite(SPI_SS, _val);
}

