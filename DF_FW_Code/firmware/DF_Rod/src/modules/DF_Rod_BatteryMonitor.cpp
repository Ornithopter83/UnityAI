// Rod battery monitoring implementation.
#include "../DF_Rod_Internal.h"
#include "DF_Rod_BatteryMonitor.h"
void battChangeCheckSend()
{
	// 1) Range : + 2 / -2 Change
	int	bat_gap = abs(battery.rest_lvl - battery.old_restLvl);
	int	bat_dir = bat_gap ? (battery.rest_lvl - battery.old_restLvl) / bat_gap : 0;
	int bat_chk = checkTO(battery.last_tick, BAT_UPDATE_INTVL) || (battery.chk_cnt && (bat_gap > BAT_UPDATE_GAP*4));
//	if ((bat_chk && ((bat_gap >= BAT_UPDATE_GAP) || (bat_gap && (battery.rest_lvl==BATTERY_PER_MAX)))) || 
//		(bat_dir && (bat_dir != battery.old_dir)))
	if (bat_chk && ((bat_gap >= BAT_UPDATE_GAP) || (bat_gap && (battery.rest_lvl==BATTERY_PER_MAX))))
 	{
		int	notify_Level;
		if ((battery.old_restLvl < 0) || (bat_gap < BAT_UPDATE_GAP)) {
			notify_Level = battery.rest_lvl;
		}
		else {
			notify_Level = battery.old_restLvl + BAT_UPDATE_GAP * bat_dir;
		}
		String str = battery.getLevel3Char(notify_Level);	// rest_lvl : 01~99
		battery.restLvlStr = str;

		eNow.write(DF_Protocol_RodToMain_Battery, battery.restLvlStr);	// Send to MAIN, Baterry Levle : 3 char 001~009%
		battery.old_restLvl = notify_Level;
		if (bat_dir && (battery.old_dir != bat_dir)) {
			battery.old_dir = bat_dir;
//			battery.chk_cnt = 1;
			str[0] = (bat_dir > 0) ? '+' : '-';
		}
		else if (battery.chk_cnt > 0)
			battery.chk_cnt--;
		setTO(battery.last_tick);

	}
	else
	{
		// NA
		// str = battery.getLevel3Char(battery.old_restLvl);
	}
}


//----------------------------------------------------------------
//
//	CALL 1SEC
