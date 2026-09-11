//
//
//

#include "Common.h"
#include "Led.h"


void (*_ledCallback)(String);

Led::Led()
{

}

void Led::init()
{

	// Cent LED R,G,B
  #if IO_DLED
  	analogWrite(LED_CENT_R_PIN,LED_OFF_DUTY);			// 0% = LOW
  	analogWrite(LED_CENT_G_PIN,LED_OFF_DUTY);			// 0% = LOW
  	analogWrite(LED_CENT_B_PIN,LED_OFF_DUTY);			// 0% = LOW
  	analogWrite(LED_BTM_PIN,LED_OFF_DUTY);			// 0% = LOW

	#if(!IO_LM_MOT)
  	  #if (!IO_EXTIO_UART_PIN)	// 확장UART를 안쓰면
  		//analogWrite(LED_OPT_L_PIN,LED_OFF_DUTY);			// 0% = LOW
  		//analogWrite(LED_OPT_R_PIN,LED_OFF_DUTY);			// 0% = LOW
  	  #endif
	#endif
	
  #endif

	int ledoff = mapled(LED_OFF_DUTY);

	LogPrintln(" LG] LEDOFF LedOffDuty: " + String(ledoff) );

}


//callback Function Name : rotateChangeCallback
void Led::LedCallback(void (*ledCallback)(String))
{
  _ledCallback = ledCallback;
}


/*
========Led ON,
	Position : 
	Duty : 0 ~ 255
*/
void Led::on(unsigned int ledPos, int _duty)
{

#if (IO_DLED)

	if(LEDPOS_MAX <= ledPos)
	{
		LogPrintln(" LG] NGLED " + String(ledPos));
		return;
	}
	else
	{
		switch(ledPos)
		{
			case LEDPOS_CENT_R:
				ledOut(LED_CENT_R_PIN, _duty);
				break;
			
			case LEDPOS_CENT_G:
				ledOut(LED_CENT_G_PIN, _duty);
				break;
			
			case LEDPOS_CENT_B:
				ledOut(LED_CENT_B_PIN, _duty);
				break;
			
			case LEDPOS_BTM:
				ledOut(LED_BTM_PIN, _duty);
				break;
			
			case LEDPOS_LEFT:

			  #if(!IO_LM_MOT)
			  	#if (!IO_EXTIO_UART_PIN)
					//ledOut(LED_OPT_L_PIN, _duty);
			  	#endif
			  #endif

			    break;
			  
			case LEDPOS_RIGHT:

			  #if(!IO_LM_MOT)
			    #if (!IO_EXTIO_UART_PIN)
				  //ledOut(LED_OPT_R_PIN, _duty);
			    #endif
			  #endif

			    break;
			  
			default:
				//error
				break;
		}
	}

#endif

}

void Led::RGBon(unsigned int ledPos, int rVal, int gVal, int bVal)
{
	
#if (IO_DLED)
	
	if(LEDPOS_MAX <= ledPos)
	{
		LogPrintln(" LG] NGLED " + String(ledPos));
		return;
	}
	else
	{
		switch(ledPos)
		{
			case LEDPOS_CENT_RGB:
				ledOut(LED_CENT_R_PIN, rVal);
				ledOut(LED_CENT_G_PIN, gVal);
				ledOut(LED_CENT_B_PIN, bVal);
				break;
				
			case LEDPOS_BTM:
				ledOut(LED_BTM_PIN, bVal);
				break;
			
			default:
				// error
				break;
		}
	}

#endif

}

void Led::off(unsigned int ledPos)
{
	
#if (IO_DLED)
	
	if(LEDPOS_MAX <= ledPos)
	{
		LogPrintln(" LG] LEDNG " + String(ledPos));
		return;
	}
	else
	{
		switch(ledPos)
		{
			case LEDPOS_CENT_RGB:
				ledOut(LED_CENT_R_PIN, LED_OFF_DUTY);
				ledOut(LED_CENT_G_PIN, LED_OFF_DUTY);
				ledOut(LED_CENT_B_PIN, LED_OFF_DUTY);
				break;
			case LEDPOS_BTM:
				ledOut(LED_BTM_PIN, LED_OFF_DUTY);
				break;
			
			case LEDPOS_LEFT:

			  #if(!IO_LM_MOT)
			    #if (!IO_EXTIO_UART_PIN)
				  //ledOut(LED_OPT_L_PIN, LED_OFF_DUTY);
			    #endif
			  #endif
			  
				break;
			  
			case LEDPOS_RIGHT:
				
			  #if(!IO_LM_MOT)
			    #if (!IO_EXTIO_UART_PIN)
				  //ledOut(LED_OPT_R_PIN, LED_OFF_DUTY);
			    #endif
			  #endif

				break;
			  
			default:
				// error
				break;
		}
	}

#endif

}

void Led::blink(unsigned int ledPos, int time, int bCnt)
{
	// TBD
}


