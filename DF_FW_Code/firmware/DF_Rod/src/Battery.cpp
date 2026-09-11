//
//
//

#include "Common.h"
#include "Battery.h"


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

	// LOG to INIT AD VAL
}


//
//	BAT LEVEL READ , 100ms
//
void Battery::scanAvgInteg()
{
  int i = 0;
  int rest_ad;
	
  adc_value = analogRead(BATTERY_LVL_PIN);					// (Vr1.0.1.0) 멤버 변수로 공유함.

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

	rest_lvl = map(rest_ad, BATTERY_MIN_VALUE_NEW, BATTERY_MAX_VALUE_NEW, 0, 100);

  if( BATTERY_PER_MIN_0 > rest_lvl) rest_lvl = BATTERY_PER_MIN_0;	// 0 %
  //if( BATTERY_PER_MIN_1 > rest_lvl) rest_lvl = BATTERY_PER_MIN_1;	// 1 %
  if( BATTERY_PER_MAX < rest_lvl) rest_lvl = BATTERY_PER_MAX;	// 99 %
  
  lowState = ( BATTERY_PER_LOW > rest_lvl );				// 20% under

}


void Battery::scanKalFilter()
{

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
