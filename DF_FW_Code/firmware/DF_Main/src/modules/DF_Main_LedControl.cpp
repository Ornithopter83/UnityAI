// Main LED control implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_LedControl.h"
void initSet_LedContVal(int Act)
{
	int i=0;

	// SET VAL
	memset(ledCont, 0, sizeof(ledCont));
	for(i=0; i<LED_IDX_MAX; i++)
	{
		if (Act) {
			ledCont[i].oldcont = 	initledCont[i].oldcont;
			ledCont[i].cont = 		initledCont[i].cont;
		}
		ledCont[i].oldcont = 	initledCont[i].oldcont;
		ledCont[i].cont = 		initledCont[i].cont;
		ledCont[i].colorNo = 	initledCont[i].colorNo;
		ledCont[i].colorMany = 	initledCont[i].colorMany;
		ledCont[i].bTime = 		initledCont[i].bTime;
		ledCont[i].dTime = 		initledCont[i].dTime;
		ledCont[i].bStep = 		initledCont[i].bStep;
		ledCont[i].dStep = 		initledCont[i].dStep;
		ledCont[i].dColorCnt = 	initledCont[i].dColorCnt;
		ledCont[i].dReqCnt = 	initledCont[i].dReqCnt;
		ledCont[i].rsv_e = 		initledCont[i].rsv_e;
	}

	// LED START
	for(i=LED_IDX_CENT; i<LED_IDX_MAX; i++)
	{
		switch(ledCont[i].cont)
		{
			case 0: //OFF
				ledCont_OffStart(i);
				break;
			case 1: //ON
				ledCont_OnStart(i);
				break;
			case 2: //Blinking
				ledCont_BlinkStart(i);
				break;
			case 3: //Dimming
				ledCont_DimmStart(i);
				break;
		}
	}

	// LED DISPLAY ENB
	ledControlMode = 1;
	// LED Light
	ledLight = 100;	// 100% at POWER ON & DIAG OUT
	
}

//--- LED Control Start
void ledCont_OffStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK & DIMM Step CLR
	ledCont[pos].bStep = 0;
	ledCont[pos].dStep = 0;
	// Off 처리
	extLed.off(pos+2);
}

void ledCont_OnStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK & DIMM Step CLR
	ledCont[pos].bStep = 0;
	ledCont[pos].dStep = 0;

	// Color Set
	if(LED_IDX_CENT == pos)
	{
		setRGBcolor(pos, (int)ledCont[pos].colorNo);
		extLed.RGBon(LEDPOS_CENT_RGB, sRGB[pos].r, sRGB[pos].g, sRGB[pos].b );	
	}
	else
	{
		extLed.on(pos+2, (int)LED_MAX_DUTY);
	}
	// On 처리
}

void ledCont_BlinkStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// DIMM Step CLR
	ledCont[pos].dStep = 0;

	// Color Set,	TBD
	setRGBcolor((int)pos, (int)ledCont[pos].colorNo );

	// Blink Start
	ledCont[pos].bStep = 10;	// Start,	// execledCont_Blink
}


//
// Call 10ms, LED BLINK CONTROL
//
void execledCont_Blink(int pos)
{
	static unsigned long _to[LED_IDX_MAX] ={0,0,0,0,0};
	static unsigned int _oldStep[LED_IDX_MAX] = {0,0,0,0,0};
	//

	switch(ledCont[pos].bStep)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// Start , LED ON
			// 
			if(LED_IDX_CENT == pos)
			{
				extLed.RGBon((pos+2), sRGB[pos].r, sRGB[pos].g, sRGB[pos].b );
			}
			else
			{
				extLed.on((pos+2), (int)LED_MAX_DUTY);
			}

			setTO(_to[pos]);
			ledCont[pos].bStep = 11;	// On Wait
			break;
		case 11:			// ON WAIT
			if(checkTO(_to[pos], ledCont[pos].bTime))
			{
				ledCont[pos].bStep = 20;
			}
			break;
			
		case 20:		// OFF
			extLed.off(pos+2);
			
			setTO(_to[pos]);
			ledCont[pos].bStep = 21;	// OFF Wait
			
			break;
			
		case 21:	// OFF WAIT
			if(checkTO(_to[pos], ledCont[pos].bTime))
			{
				ledCont[pos].bStep = 10;
			}
			break;
			

		case 30:
			break;
		case 40:
			break;

		case 90:
			break;
			
		default:
			break;
	}
}

//
void ledCont_DimmStart(unsigned int pos)
{
	if(0==pos) { return;}
	
	// BLINK Step CLR
	ledCont[pos].bStep = 0;

	// Color Set, 
	//Start Color CNT
	if(7 > ledCont[pos].colorMany)
	{
		ledCont[pos].dColorCnt = COLOR_RED;				// Color Oder Oder = 1st( red) 
	}
	else
	{
		ledCont[pos].dColorCnt = COLOR_WHITE; 			// Color Oder Oder = 1st( White)
	}
	ledCont[pos].dReqCnt = (ledCont[pos].dTime/LED_CONTROL_10MS/2);	// ReqCnt = Cal CNT
	setRGBcolor((int)pos, (int)ledCont[pos].dColorCnt );

	// Dimm Start
	ledCont[pos].dStep = 10;	// Start,	// execledCont_Dimm
}


//
//
void execledCont_Dimm(int pos)
{
	static unsigned long _to[LED_IDX_MAX] ={ 0,0,0,0,0 };
	static unsigned int _oldStep[LED_IDX_MAX] = { 0,0,0,0,0 };
	static unsigned int _cnt[LED_IDX_MAX] = { 0,0,0,0,0 };
	//
	
	switch(ledCont[pos].dStep)
	{
		case 0: // IDLE
			break;
				
		case 10:	// Start , Color CNT
			if(LED_IDX_BTM == pos)
			{
				setRGBcolor(pos, COLOR_BLUE);	// BTM is BLUE ONLY
			}
			else
			{
				setRGBcolor(pos, ledCont[pos].dColorCnt);	// FIX ORDER, outRGB is Dimming OUT VAL
				//setRGBcolor(pos, ledColorOrder[pos][ledCont[pos].dColorCnt]); //SET ORDER, outRGB is Dimming OUT VAL
			}
			//setOutRGBcolor(pos, ledCont[pos].dColorCnt);	// outRGB is Dimming OUT VAL
			_cnt[pos] = 0;
			ledCont[pos].dStep = 20;
			// 
			break;
			
		case 20:		// INC
			_cnt[pos]++;
			if(_cnt[pos] > ledCont[pos].dReqCnt)
			{
				extLed.RGBon((pos+2), sRGB[pos].r, sRGB[pos].g, sRGB[pos].b);	// FULL COLOR ON
				setTO(_to[pos]);
				ledCont[pos].dStep = 21;
			}
			else
			{
				outRGB[pos].r = (sRGB[pos].r) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].g = (sRGB[pos].g) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].b = (sRGB[pos].b) * _cnt[pos] / ledCont[pos].dReqCnt;
				extLed.RGBon((pos+2), outRGB[pos].r, outRGB[pos].g, outRGB[pos].b);
			}
			break;
		case 21:
			if(checkTO(_to[pos], ledDimmHoldTime[pos]))		// 200ms HOLD
			{
				ledCont[pos].dStep = 30;
			}
			break;

		case 30:		// DEC
			_cnt[pos]--;
			if(_cnt[pos] <= 0)		// Count  = 0;
			{
				extLed.RGBon((pos+2), 0,0,0);	// OFF
				setTO(_to[pos]);
				ledCont[pos].dStep = 31;
			}
			else
			{
				outRGB[pos].r=(sRGB[pos].r) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].g=(sRGB[pos].g) * _cnt[pos] / ledCont[pos].dReqCnt;
				outRGB[pos].b=(sRGB[pos].b) * _cnt[pos] / ledCont[pos].dReqCnt;
				extLed.RGBon((pos+2), outRGB[pos].r, outRGB[pos].g, outRGB[pos].b);
			}
			break;
		case 31:
			if(checkTO(_to[pos], ledDimmHoldTime[pos]))		// 200ms HOLD
			{
				ledCont[pos].dStep = 40;
			}
			break;

		case 40:
			ledCont[pos].dColorCnt++;
			if(ledCont[pos].dColorCnt > ledCont[pos].colorMany)
			{
				if(7 > ledCont[pos].colorMany)
				{
					ledCont[pos].dColorCnt = COLOR_RED;
				}
				else
				{
					ledCont[pos].dColorCnt = COLOR_WHITE;
				}
			}
			else
			{
			}
			ledCont[pos].dStep = 10;
			break;

		case 50:
			break;
			
		default:
			break;
	}

}

// Move to UPPER
//---10ms Call Ext Led Dimming Control

unsigned int ext_led_dim_step = 10;
static unsigned int old_ext_led_dim_step = 0x00;
//static unsigned long _to = 0;
static unsigned long tot_time = 0;
unsigned int cnt = 0;
unsigned int reqCnt = 0;
static unsigned int loopCnt = 0;
static unsigned int outCnt = 0;

static unsigned int bDuty = (LED_MAX_DUTY*10);
static unsigned int updownStep = (1*10);

//
// 10ms Call, OLD Dimming
//
void old_execCentLed_Dimming_Control(int color, int sec)
{

	//

	switch(ext_led_dim_step)
	{
		case 0:	// IDLE
			break;
			
		case 10:	// Start , Calculation
			reqCnt = (sec * 1000) / 2 / 10;	// 10ms Count
			loopCnt = (reqCnt+128)/256; // (256*2);
			if(1 > loopCnt)
			{
				loopCnt = 1;
			}
			
			if(256 > reqCnt)
			{
				updownStep = (256*10)/reqCnt;
			}
			else
			{
				updownStep = (1*10);
			}
			
			bDuty = (0*10);

			// TBD Color, int color

			ledOut(LED_CENT_R_PIN, 0);
			ledOut(LED_CENT_G_PIN, 0);
			ledOut(LED_CENT_B_PIN, bDuty);	// Blue
			ext_led_dim_step = 20;
			break;
			
		case 20:		// UP
			bDuty += updownStep ;
			if((LED_MAX_DUTY*10) < bDuty)		// Over
			{
				outCnt = 0;
				bDuty = 0;
				ext_led_dim_step = 29;
			}
			else
			{
				ext_led_dim_step = 21;
			}
			ledOut(LED_CENT_B_PIN, (bDuty/10)); // Blue
			break;
			
		case 21:
			outCnt++;
			if( !(outCnt % loopCnt) )
			{
				if(outCnt > reqCnt)			// Up END?
				{
					outCnt = 0;
					if( LED_MAX_DUTY*10 < bDuty) bDuty = (LED_MAX_DUTY*10);
					ledOut(LED_CENT_B_PIN, (bDuty/10));	// Blue
					ext_led_dim_step = 29;
				}
				else
				{
					ext_led_dim_step = 20;
				}
			}
			break;
			
		case 29:
			outCnt++;
			if(outCnt > ledoffHoldTimeCnt)
			{
				if( LED_MAX_DUTY*10 < bDuty) bDuty = (LED_MAX_DUTY*10);
				outCnt = 0;
				ext_led_dim_step = 30;
			}
			break;
			
		case 30:		// Down
			bDuty -= updownStep;
			if((0*10) > bDuty)		// Down Over
			{
				bDuty = (0*10);
				outCnt = 0;
				ext_led_dim_step = 32;		// HoldTime
			}
			else
			{
				ext_led_dim_step = 31;
			}
			ledOut(LED_CENT_B_PIN, (bDuty/10));	// Blue
			break;
			
		case 31:
			outCnt++;
			if( !(outCnt % loopCnt) )
			{
				if(outCnt > reqCnt)		// Down Over
				{
					bDuty = (0*10);
					outCnt = 0;
					ext_led_dim_step = 32;	// Hoild
				}
				else
				{
					ext_led_dim_step = 30;
				}
			}
			break;

		case 32:		// DOWN HOLD
			outCnt++;
			if(outCnt > ledoffHoldTimeCnt)
			{
				if( 0*10 > bDuty) bDuty = (0*10);
				outCnt = 0;
				ext_led_dim_step = 20;
			}
			break;
			
		default:
			break;
	}
}



// IF idx Position : 1~4  =>  innerPosition:3~6 [ Inner  other: 0 ~ 2 = RGB)
void extLed_Off(unsigned int idx)
{
	switch(idx)
	{
		case LED_IDX_CENT:
		case LED_IDX_BTM:
			extLed.off((idx+2));
			break;

		case LED_IDX_LEFT:
		case LED_IDX_RIGHT:
			break;
		
		default:
			break;
	}
}

//
//
void extLed_On(unsigned int idx)
{
	unsigned int color;
	
	switch(idx)
	{
		case LED_IDX_CENT:
			// Color Set
			color = ledCont[idx].colorNo;
			if(COLOR_MAGENTA < color) { color = COLOR_WHITE; }	// 6(Magenta) Over White SET
			setRGBcolor((int)idx, (int)color);
			extLed.RGBon((idx+2), sRGB[idx].r, sRGB[idx].g, sRGB[idx].b);
			break;
		
		case LED_IDX_BTM:
			extLed.on((idx+2), (int)LED_MAX_DUTY);	// LED_MAX
			break;
		
		case LED_IDX_LEFT:
		case LED_IDX_RIGHT:
			break;
			
		default:
			break;
	}
}

/*
void extLed_Blink(int idx)
{
}

void extLed_Dimm(int idx)
{
	if(LED_IDX_CENT == idx)
	{
		old_execCentLed_Dimming_Control(COLOR_BLUE, dimming_time);
	}
}
*/

//
//	Call 10ms Control
//
void extLed_Control() 
{
	unsigned int pos=0;
	// 10ms, EXT LED CONTROL
	if(PWR_ON == pwrMode)
	{
		
		//if(DIAG_MODE != mainMode)
		if(ledControlMode)
		{

			for(pos=LED_IDX_CENT; pos<LED_IDX_MAX; pos++)
			{
				if(LED_CON_BLINK == ledCont[pos].cont)		// 2 - blinking
				{
					//extLed_Blink(pos);
					execledCont_Blink(pos);
				}
				else if(LED_CON_DIMM == ledCont[pos].cont)	// 3 - Dimming
				{
					//if(LED_IDX_CENT == pos)		// TBD , Position & RGB Port MATCHING
					{
						//extLed_Dimm(pos);
						execledCont_Dimm(pos);		// RGB Port 
					}
				}
				else	// OFF or ON
				{
					if(ledCont[pos].oldcont != ledCont[pos].cont)
					{
						if(LED_CON_OFF == ledCont[pos].cont)	// 0 - OFF
						{
							extLed_Off(pos);
						}
						else if(LED_CON_ON == ledCont[pos].cont)	// 1 - ON(100%)
						{
							extLed_On(pos);
						}
						ledCont[pos].oldcont = ledCont[pos].cont;
					}
					else
					{
						// NA
					}
				}
			}
		}
	}

}

//====== Uart CMD ANA===
//
//
//

// (V108) 엔코더 센서 변화 내역 콘솔에 출력
