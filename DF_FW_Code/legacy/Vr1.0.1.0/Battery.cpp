//
//
//

#include "Common.h"
#include "Battery.h"

#if 0
Kalman myFilter(0.125, 32, 4095, 0);
#endif

Battery::Battery()
{

}

//void Battery::init()
//{
//}

void Battery::init()
{
	int i = 0;
	adc_value = analogRead(BATTERY_LVL_PIN);					// (Vr1.0.1.0) 멤버 변수로 공유함.

	integ_ad = adc_value;		// 초기 누적 Val저장.

	sum_ad = 0;
	for(i=0; i<BAT_BUF_SIZE; i++)
	{
		ad_buff[i] = adc_value;
		sum_ad += adc_value;
	}
	
	avg_ad = (int)(sum_ad / BAT_BUF_SIZE);
	ad_idx = 0;

	// update rest Level
	scanAvgInteg();

	// Add STRING INIT_VAL
	String str = getLevel3Char(rest_lvl);	// rest_lvl : 00~99
	restLvlStr = str;

#ifdef	DbgSerial
	#if LOG_BAT
	DbgPrintln(" LG] BatLvl:" + restLvlStr + ", ADC:" + String(adc_value));
	#endif
#endif
	// LOG to INIT AD VAL
	#if (LOG_BATT_INIT_STR)
		LogPrintln(" LG] BatLvl: " + restLvlStr);
	#endif
}

/*
void Battery::scanAvg()
{
  int i = 0;
	
  int adc_value = analogRead(BATTERY_LVL_PIN);
  int rest_ad;

// Imporve Delete Filter, Average AD 100ms * 10회
#if BAT_KAL_FILTER
  int map_val = map(avg_ad, BATTERY_MIN_VALUE, BATTERY_MAX_VALUE, 0, 100);
  double lvl = myFilter.getFilteredValue((double)map_val);
  rest_lvl = floor(lvl);									// delete Under float point
#else
  ad_buff[ad_idx] = adc_value;
  ad_idx++;
  if((BAT_BUF_SIZE-1) < ad_idx) ad_idx = 0;  
  
  sum_ad = 0;
  for(i=0; i<BAT_BUF_SIZE; i++)
  {
	sum_ad += ad_buff[i];
  }
  avg_ad = (int)(sum_ad/BAT_BUF_SIZE);
  rest_lvl = map(avg_ad, BATTERY_MIN_VALUE, BATTERY_MAX_VALUE, 0, 100);
#endif


  if(  1 > rest_lvl) rest_lvl = 1;
  if(100 < rest_lvl) rest_lvl = 100;
  
  if( BATTERY_LOW_PERCENT > rest_lvl ) battLowState = true;				// 20% under

}
*/

//
//	BAT LEVEL READ , 100ms
//
void Battery::scanAvgInteg()
{
  int i = 0;
  int rest_ad;
	
  adc_value = analogRead(BATTERY_LVL_PIN);					// (Vr1.0.1.0) 멤버 변수로 공유함.

	#if BAT_AD_AVG_INTEG	// INTEG=1
		integ_ad = (integ_ad + adc_value)/2;
		
		rest_ad = integ_ad;
	#else					// AVG = 0
		ad_buff[ad_idx] = adc_value;
  		ad_idx++;
  		if((BAT_BUF_SIZE-1) < ad_idx) ad_idx = 0;  
	
  		sum_ad = 0;
  		for(i=0; i<BAT_BUF_SIZE; i++)
  		{
			sum_ad += ad_buff[i];
  		}
  		avg_ad = (int)(sum_ad/BAT_BUF_SIZE);
		
		rest_ad = avg_ad;	
	#endif

	//if(isRodNewBoard())	// New ROD
	if(BATT_800_V2 == getBattType())	// New ROD
	{
		rest_lvl = map(rest_ad, BATTERY_MIN_VALUE_NEW, BATTERY_MAX_VALUE_NEW, 0, 100);
	}
	else
	{
		rest_lvl = map(rest_ad, BATTERY_MIN_VALUE, BATTERY_MAX_VALUE, 0, 100);
	}

  if( BATTERY_PER_MIN_0 > rest_lvl) rest_lvl = BATTERY_PER_MIN_0;	// 0 %
  //if( BATTERY_PER_MIN_1 > rest_lvl) rest_lvl = BATTERY_PER_MIN_1;	// 1 %
  if( BATTERY_PER_MAX < rest_lvl) rest_lvl = BATTERY_PER_MAX;	// 99 %
  
  lowState = ( BATTERY_PER_LOW > rest_lvl );				// 20% under

}


void Battery::scanKalFilter()
{

#if 0
  int i = 0;
	
  int adc_value = analogRead(BATTERY_LVL_PIN);

  int map_value = map(adc_value, BATTERY_MIN_VALUE, BATTERY_MAX_VALUE, 0, 100);

// Imporve Delete Filter, Average AD 100ms * 10회
#if BAT_KAL_FILTER
  double lvl = myFilter.getFilteredValue((double)map_value);
  rest_lvl = floor(lvl);									// delete Under float point
#endif
  
  if( BATTERY_PER_MIN > rest_lvl) rest_lvl = BATTERY_PER_MIN;
  if( BATTERY_PER_MAX < rest_lvl) rest_lvl = BATTERY_PER_MAX;
  
  if( BATTERY_PER_LOW > rest_lvl ) lowState = true;				// 20% under
#endif
}


String Battery::getLevel3Char(int _lvl)
{
  //if(rest_lvl < 0)	rest_lvl = 0;	// test
  
  if (10 > _lvl) {
    return "00" + String(_lvl);
  } else if (100 > _lvl) {
    return "0" + String(_lvl);
  } else {
    return String(_lvl);
  }
}

bool Battery::isLowLevel()
{
  return lowState;
}
