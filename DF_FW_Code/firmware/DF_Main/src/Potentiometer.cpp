//
// Torque Motor (Potentio Motor) Control
//

#include "Common.h"
#include "Potentiometer.h"

Potentiometer::Potentiometer()
{

}

/*------------------------------------------------------------------
   Potentio Initialize (SPI IF)
------------------------------------------------------------------*/
void Potentiometer::init()
{
  SPI.begin(TORQ_SCK, -1, TORQ_MOSI, TORQ_SS);  // SPI SET (SCK,SO,SI,SS  Freq=1MHz(default))
  digitalPortWrite(TORQ_SS, TORQ_OFF_VALUE);    // Torque Motor OFF (Default  Val = 0)
}

/*------------------------------------------------------------------
    Output Value of Torque Motor, SEND SPI  IF
------------------------------------------------------------------*/
void Potentiometer::digitalPortWrite(int _pinNum, int _val)
{
  digitalWrite(_pinNum, LOW);
  SPI.transfer(B00010001);  		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
  SPI.transfer(_val);       		// VAL Send
  digitalWrite(_pinNum, HIGH);
}

/*------------------------------------------------------------------
   Output Torque Motor for Set Value STRING(0~255)
      ( Incllude Change to INT from STRING)
------------------------------------------------------------------*/
void Potentiometer::setValue(String _val)
{
	// Catch INT Val inside of STRING
	int val = _val.substring(3,(3+3)).toInt();

	outValue(val);
}

/*------------------------------------------------------------------
   Output Torque Motor for Set Value INT(0~255)
------------------------------------------------------------------*/

void Potentiometer::setValue(int val)
{
	outValue(val);
}

void Potentiometer::on(int val)
{
	outValue(val);
}

void Potentiometer::outValue(int val)
{

  //if(0< val && 60 > val) val = defaultVal;
  //else					   val = val - setAdjTorq;

  if (val < RECV_MIN) { val = RECV_MIN; }
  if (val > RECV_MAX) { val = RECV_MAX; }

  // 30W ADJ to 150 SET
  if(TORQ_30W_LIMIT == torqWattAdj)
  {
  	if ( 35 < val)
  	{
  		//val = (val*68/195) + 39;		// Limit
  		//val = (val*58/195) + 39;		  // Limit
  		val = (val*73/200) + 35;		  // Limit
  	}
  }

  


	if (val < RECV_MIN) { val = RECV_MIN; }
	if (val > RECV_MAX) { val = RECV_MAX; }
  
	digitalPortWrite(TORQ_SS, val);

		LogPrintln(" LG] TORQo " + String(val));
}

// NOT-USE
void Potentiometer:: respTorqOutVal(int val)
{
}

