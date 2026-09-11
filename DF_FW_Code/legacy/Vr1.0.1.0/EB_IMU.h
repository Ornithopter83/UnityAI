// RotaryEncoder.h

#ifndef _EB_IMU_h
#define _EB_IMU_h

#include "Arduino.h"

#include "SPIFFS.h"

//#define MPU_BUF_SIZE      64		// IMU Recv Buffer Size 64 Byte
#define MPU_BUF_SIZE      128		// IMU Recv Buffer Size 64 Byte
//#define MPU_RES_TIMEOUT   4000		// IMU Response Wait Time 4 Sec(4000ms)
#define MPU_RES_TIMEOUT   3000		// IMU Response Wait Time 4 Sec(4000ms)

#define IMU_DATA_SIZE   6

// Define IMU START, STOP
#define IMU_CONF_STOP		0		// IMU CONFIG STOP
#define IMU_CONF_START 		1		// IMU CONFIG START

// Define IMU_RESP_OK/NG
// TBD Resp String, INTV, OUT, CALB, Inteval Measure
#define STX_IMU_RESP_OK		"$0800000001%"
#define STX_IMU_RESP_NG		"$0800000000%"

#define RESP_IMU_STOP_OK		"$08_stop_ok%"	
#define RESP_IMU_STOP_NG		"$08_stop_ng%"	

#define RESP_IMU_START_OK		"$08_start_ok%"	
#define RESP_IMU_START_NG		"$08_start_ng%"	

#define RESP_IMU_INTV_OK		"$08_intv_ok%"
#define RESP_IMU_INTV_NG		"$08_intv_ng%"
#define RESP_IMU_INTV_RNG_OVER	"$08_intv_rg%"

#define RESP_IMU_OUTG_OK		"$08_outG_ok%"	
#define RESP_IMU_OUTG_NG		"$08_outG_ng%"	
#define RESP_IMU_OUTD_OK		"$08_outD_ok%"	
#define RESP_IMU_OUTD_NG		"$08_outD_ng%"	

#define RESP_IMU_CALB_OK		"$08_Calb_ok%"	
#define RESP_IMU_CALB_NG		"$08_Calb_ng%"	

class EB_IMU
{
  public:
    EB_IMU();

    void init();
    int  uartRecv_Normal();
    void setConfig(String msg);
    void setConfigString(String msg);
	String convert_oldSetup(String msg);		// OLD SETUP

	String getversion();

    void disable();
    void jyro_calibration();
    void setInterval(int ms);
	void setStartStop(int set);
    //void write(String pid, String str);

    void setStateCallback(void (*imuCallback)(String, String));
    int  uartRecv_Setup();

	int  polling();		//Connection Check

	short checkSetupFileVal();
	String getIntervalFileStr();	
	void autoSetup();
	short saveInterval(int intval);
	short saveInterval(String intvalStr);

	bool isMeasure = false;
	String imuDataStr = "";
	String imuVer = "";

  private:
    int ebimu_parser(float *item, int number_of_item);
    String reply();
	int respCheck();
	short saveIntval(String intvalStr);
};


#endif
