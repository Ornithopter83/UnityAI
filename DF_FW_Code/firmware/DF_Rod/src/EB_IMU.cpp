//
//
//

#include "Common.h"
#include "EB_IMU.h"
//#include "Version.h"

extern String rodVer;
extern unsigned short flagIMUSTOP;

extern unsigned short imuIntervalCheckCnt;
extern unsigned short apType;		// is ApType

extern void setRecvImuData();
extern int isSlaveSetupMode();


// Define IMU Measure CMD Data
// OLD SETUP프로그램(v2.2 이전)
int M_ENABLE = 11111111;
int M_DISABLE = 0;
int M_JYRO_INIT = 10100000;
int M_JYRO_ENABLE = 10101010;

// Recv Buffer 64 Byte
char mBuf[MPU_BUF_SIZE];			// 64 Byte => 128 byte
signed int mBuf_cnt = 0;

// IMU Auto SETUP, 인터벌 저장 파일 이름
static const char *IMU_FILE = "/imu.txt";			// imu Setup Text


// Call Back Function
void (*_imuCallback)(String, String);

EB_IMU::EB_IMU()
{

}

//=== Resp Check, Improved IMU SERIAL, NOT USE
typedef struct _respPack
{
	unsigned int ok;
	char str[10];
} stRespPack;
typedef stRespPack *stRespPackPtr;

stRespPack imuRespPack;

stRespPackPtr *stPtr;

#define RESP_SUCESS 0
#define RESP_NG		1

#define RECV_DATA_NO	0
#define RECV_DATA_YES	1
//
//
//
int EB_IMU::respCheck()
{
	
	char chData;
	int i = 0;
	int idx = 0;

	int rts = 0;

/*
	stPtr = &imuRespPack;

	stPtr->ok = RECV_DATA_NO;
	
	int rbytes = imuSerial.available();			// Return Length of RECV DATA
	if(0 != rbytes)
	{
		for(i=0; i<rbytes; i++)
		{
			char chData = imuSerial.read();
			if(0)	{  }	// dummy
			else if('<' == chData)	// Start
			{
				idx = 0;
				stPtr->str[0] = chData;
			}
			else if('>' == chData)	// End
			{
				stPtr->str[i] = chData;
				stPtr->ok = RESP_SUCESS;
				break;
			}
			else			// Data
			{
				stPtr->str[i] = chData;
			}
        }
	}
*/
	return rts;
}

/*
------------------------------------------------------------------
   Check IMU Recv Data for 4Sec, return String(resData)
------------------------------------------------------------------
*/
String EB_IMU::reply() {
  String resData = "";
  bool isTimeout = false;
  long timeoutDelay = millis() + MPU_RES_TIMEOUT;	// wait 4 sec

  // WAIT until Recv Data, or T/O MAX 3SEC WAIT
  while (!imuSerial.available()) {		// Wait 3SEC Without Recv Data
    if (millis() > timeoutDelay) {
      isTimeout = true;				// TO SET
      break;
    }
  }

  // Recv Data Save to STRING & Return
  if (!isTimeout) {					// NOT T/O
    while (imuSerial.available()) {		// Data Read, 1 CHAR each 1ms
      char data = imuSerial.read();
      resData += data;
      delay(1);
    }
  }
  return resData;
}

// Set Callback Funtion Pointer, Only Normal Mode
//callback Function Name :  imuDataCallback
void EB_IMU::setStateCallback(void (*imuCallback)(String, String)) {
  _imuCallback = imuCallback;
}

// SERIAL_1 = IMU UART
void EB_IMU::init()
{
	//imuSerial.begin(115200, SERIAL_8N1, IMU_RX, IMU_TX);	// Return Thing

	// Data CLR
	isMeasure = false;
	imuDataStr = "";

  	// IMU Serial NG Check
  	//imuVer = getversion();

//== LOG
/* 
  LogPrintln(" LG] IMU SOD : ");
  send2Imuln("<sod0>");
  LogPrintln(" LG] " + reply());
  delay(50);
  LogPrintln(" LG] IMU SOG : ");
  send2Imuln("<sog1>");
  LogPrintln(" LG] " + reply());
  delay(50);
  LogPrintln(" LG] IMU SOR : ");
  send2Imuln("<sor100>");
  LogPrintln(" LG] " + reply());
*/

}

void EB_IMU::disable()
{
  isMeasure = false;
}

// old SETUP프로그램 대응 ( $080000xxxx% - Interval )
String EB_IMU::convert_oldSetup(String msg)
{
	String rtsMsg ="";
	int val = msg.substring(3).toInt();
	if (M_DISABLE == val)	  // 2. IMU  DATA OFF( Flag Control )
	{
		rtsMsg = "0";
	}
	else if (M_ENABLE == val)		  //1.	IMU DATA ON (Flag Control)
	{
		rtsMsg = "1";
	}
	else if (M_JYRO_ENABLE == val) //4. IMU OUT ENB SET (EULER =always, GYRO = out, DISTANCE = not out)
	{
		rtsMsg = "3";
	}
	else if (M_JYRO_INIT == val)  //3. IMU INIT( JYRO CALIBRATUON)
	{
		rtsMsg = "4";
	}
	else	// Interval
	{
		if ( ((IMU_INTV_MIN_TIME) > val) || ((IMU_INTV_MAX_TIME) < val) )		//NG: 10미만 ot 1000초과
		{
			rtsMsg = "20100";	// Default 100ms
		}
		else		// OK : 10 ~ 1000
		{
			rtsMsg = "2" + msg.substring(7,(7+4));	//0000XXXX%
		}
	}

	return rtsMsg;

}

/*-------------------------------------------------------------
  Set IMU Config ( $08 ) , att Setup Mode & Normal Mode
-------------------------------------------------------------*/
void EB_IMU::setConfig(String msg)
{
	// TBD, Improve String :  val -> String msg
  //char act = msg.charAt(0);
  int act = msg.substring(0,1).toInt();
  int val = 100;
  String respMsg= "";

	// SETUP 프로그램 변경 필요.
  switch(act)
  {
  	case IMU_DATA_OFF:		// DATA_OFF
 	  	isMeasure = false;
		break;

	case IMU_DATA_ON:		// DATA ON
    	isMeasure = true;
		if (flagIMUSTOP)					// IMU STOP SET?
		{
			flagIMUSTOP = 0;				// flag CLR
			setStartStop(IMU_CONF_START);		// IMU START
		}
		
		if (AP_IS_TM == apType)
		{
		  imuIntervalCheckCnt = 1;
		}
		break;

	// PC-SETUP 프로그램에서만 사용.
	case IMU_SET_INTEVAL:		// INTERVAL SET
		//val = msg.substring(1,(1+4)).toInt();	// 2xxxx [ms]
		val = msg.substring(1).toInt();	// 2xxxx [ms]
		if ((IMU_INTV_MIN_TIME) > val || (IMU_INTV_MAX_TIME) < val)		// NG:범위 밖
		{
			RespPrintln(RESP_IMU_INTV_RNG_OVER);
		}
		else	//OK : 10 ~ 1000 [ms]
		{
			setInterval(val);		// 인터벌 설정
			saveInterval(val);		// FILE저장
		}
		break;

	case IMU_SET_OUT:		// IMU OUT ENB ( JYRO ENB)
		//send2Imuln("<stop>");
		// <sog = GYRO, 1= ENB Output ENB
  		// send to IMU
		send2Imuln("<sog1>");
		// Responde to PC
    	respMsg = reply().equals("<ok>") ? RESP_IMU_OUTG_OK : RESP_IMU_OUTG_NG;
    	delay(50);		// TBD : Delay 삭제
    	RespPrintln(respMsg);

		// <sod = Distanse, 0 = DIS output
		// send to IMU
		send2Imuln("<sod0>");
 		// Response to PC
    	respMsg = reply().equals("<ok>") ? RESP_IMU_OUTD_OK : RESP_IMU_OUTD_OK;
    	//delay(50);
    	RespPrintln(respMsg);
		break;

	case IMU_SET_CALBRATION:		// JYRO INIT(CALIBRATION)
		// <cg> = JYRO Calibration & Response to PC
		jyro_calibration();
		break;
		
	default:
		break;
		
  }



}

/*------------------------------------------------------------------------
  SET IMU CONFIG with RECV MSG(String)
------------------------------------------------------------------------*/

void EB_IMU::setConfigString(String msg)
{
    String str;
    
	// 1) STOP
    send2Imuln("<stop>");	// 1st : IMU Stop
		// Responde to PC
    str = reply();
    RespPrintln(str);

	//if (str.equals("<ok>"))
	{
		// 2) SEND msg
    	send2Imuln(msg);		// 2nd : Send Msg to IMU
		// Responde to PC
    	str = reply();
    	RespPrintln(str);

		// 3) START
    	send2Imuln("<start>");	// 3rd : IMU Start
		// Responde to PC
    	str = reply();
    	RespPrintln(str);
	}


}

#define IMU_PARSER_OK	0
#define IMU_PARSER_NG	1
/*------------------------------------------------------------------------
	Change IMU Data to Double(Float) from Recv Data : STRING_FLOAT (Imu Data)
	Result : OK(1), NG(0)
	Save Value : item[0]~[5] = Double (Imu Data), Upto number_of_item
------------------------------------------------------------------------*/
int EB_IMU::ebimu_parser(float *item, int number_of_item)
{
  int n, i;
  int rbytes;
  char *addr;
  int result = IMU_PARSER_NG;

	char str[128];


rbytes = imuSerial.available();			// Return Length of RECV DATA
if(rbytes)
{
  for (n = 0; n < rbytes; n++)
  {
    mBuf[mBuf_cnt] = imuSerial.read();

	// END CHAR ( CR: 0x0D / LF: 0x0A )

	// TBD if( 0x0D == data)  CR(0x0D)처리는 미정
    if (mBuf[mBuf_cnt] == 0x0A)			// End Char [ LF(0x0A) ] of IMU OutData ? 
    {
    	result = IMU_PARSER_OK;
		
		//imustr = String(mBuf);
    	memcpy(&str[0], &mBuf[0], (mBuf_cnt));		// Without LF
		str[mBuf_cnt] = NULL;
		imuDataStr = String(str);

	  
	  mBuf_cnt = -1;
    }
    else if (mBuf[mBuf_cnt] == '*')		// First Char(*) of IMU OutData ?
    {
      mBuf_cnt = -1;
    }
	// TBD if( 0x0D == data)  CR(0x0D)처리는 미정
    else if (mBuf[mBuf_cnt] == 0x0D)			// End Char [ LF(0x0A) ] of IMU OutData ? 
    {
    	mBuf_cnt--;			// NOT SAVE
    }	
	
    mBuf_cnt++;
    if (mBuf_cnt >= MPU_BUF_SIZE)
    {
      mBuf_cnt = 0;
    }
  }

}
return result;
}

#define IMU_RESP_OK		0
#define IMU_RESP_NOT_MS		1
#define IMU_RESP_NOT_RECV	2

/*------------------------------------------------------------------------
  Send to MAIN (Normal Mode), IMU Data($09) + imuData1, imuData2, ...imuData6 + %
     ( imuData is STRING float : ex, -45.78 )
------------------------------------------------------------------------*/
int EB_IMU::uartRecv_Normal()
{
  //https://www.e2box.co.kr/entry/%EC%95%84%EB%91%90%EC%9D%B4%EB%85%B8-%EC%98%88%EC%A0%9C
  int rts = IMU_RESP_NOT_RECV;
  
  //float euler[IMU_DATA_SIZE];				// 6 items
  //float euler[IMU_DATA_SIZE] = { -9.99, -9.99, -9.99, -9.99, -9.99, -9.99 };				// 6 items
  float euler[IMU_DATA_SIZE] = { -0.01, -0.01, -0.01, -0.01, -0.01, -0.01 };				// 6 items
  if (IMU_PARSER_OK == ebimu_parser(euler, IMU_DATA_SIZE))
  {
    setRecvImuData();			// IMU Alive SET
	
	rts = IMU_RESP_OK;

    if (true == isMeasure)		// Measure
    {
		// Send to MAIN(WIFI) , 2 times MESSAGE Only Measure!!!
    	if (_imuCallback != NULL)
    	{
				_imuCallback(DF_Protocol_RodToMain_ImuData, imuDataStr);
    	}
    }
	else
	{
  		rts = IMU_RESP_NOT_MS;
  	}
  }
  
  return rts;
}

/*------------------------------------------------------------------------
  Send to PC (setup Mode), IMU Data($09) + imuData1, imuData2, ...imuData6 + %
     ( imuData is STRING float : ex, -45.78 )
------------------------------------------------------------------------*/
int EB_IMU::uartRecv_Setup()
{
	int rts = IMU_RESP_NOT_RECV;
	
  //https://www.e2box.co.kr/entry/%EC%95%84%EB%91%90%EC%9D%B4%EB%85%B8-%EC%98%88%EC%A0%9C
  //float euler[IMU_DATA_SIZE];
  //float euler[IMU_DATA_SIZE] = { -9.99, -9.99, -9.99, -9.99, -9.99, -9.99 };				// 6 items
  float euler[IMU_DATA_SIZE] = { -0.01, -0.01, -0.01, -0.01, -0.01, -0.01 };				// 6 items
  if (IMU_PARSER_OK == ebimu_parser(euler, IMU_DATA_SIZE))		// Save euler[0]~[5] 6 items IMU Data = IMU Recv Data
  {
    setRecvImuData();		// IMU Alive SET

	rts = IMU_RESP_OK;

    if (isMeasure)			// IMU DATA OUT ON?
    {
		// Send to PC(SERIAL), IMU Data, 1 time Message  ( $09 imu1,imu2,imu3,imu4,imu5,imu6 % ) Only Measure ENB
		String str = "$09" + imuDataStr + "%";
		
    	StsPrintln(str);
    }
	else
	{
  		rts = IMU_RESP_NOT_MS;
	}

  }
  
  return rts;
}



/*------------------------------------------------------------------------
  Execution JYRO Calibration
------------------------------------------------------------------------*/
void EB_IMU::jyro_calibration()
{
  String str;
  // <cg> = execute IMU Calibration
  send2Imuln("<cg>");
	// Response Result to PC
  str = reply().equals("<ok>") ? RESP_IMU_CALB_OK : RESP_IMU_CALB_NG;
  // Response to PC(SETUP AP)
  RespPrintln(str);


}

/*------------------------------------------------------------------------
  SET IMU Output Rate ( interval Time : data(ms) * 1ms
------------------------------------------------------------------------*/
void EB_IMU::setInterval(int ms)
{
  String str;
  
	// <sor = Set Interval ms=*ms, Send to IMU
  str = "<sor" + String(ms) + ">";
  send2Imuln(str);

  str = reply().equals("<ok>") ? RESP_IMU_INTV_OK : RESP_IMU_INTV_NG;
  // Response to PC(SETUP AP)
  RespPrintln(str);

	
}

/*------------------------------------------------------------------------
  Get IMU VERSION
------------------------------------------------------------------------*/
String EB_IMU::getversion()
{
  String str;
	
  // <ver> = get IMU Version
  send2Imuln("<ver>");

	// TBD Improve [REPLY]
  str = reply();
  
  str = str.substring(str.indexOf('<'), (str.indexOf('>')+1));
  //str = rodVer + "," + str;

  return str;

}

/*------------------------------------------------------------------------
  SET IMU Output STOP
   set  (0)-STOP, (!=0)-START
------------------------------------------------------------------------*/
void EB_IMU::setStartStop( int set)
{
	// <start>: IMU Output Enable, <stop>: IMU Output Disable
  String str;
	
  if(0 == set) str = "<stop>";
  else		   str = "<start>";
  send2Imuln(str);

  str = reply().equals("<ok>") ? STX_IMU_RESP_OK : STX_IMU_RESP_NG;

	// Response to PC
  if(isSlaveSetupMode()) {
    RespPrintln(str);
  }
}


//-----------------------------------------------------------------------
//	1) File여부 - SFFIS FORMAT - enow
//	2) File없음/디렉토리의 경우 - IMU셋업
//	3) File값이 100이 아니면 -IMU셋업
//	4-1) IMU셋업후, FILE "100" Wite
//	4-2) 문제없는경우, NA
//-----------------------------------------------------------------------
short EB_IMU::checkSetupFileVal()
{
	short rts = 0;			// 0-ok

	//1) File Read
	String intvalStr = getIntervalFileStr();	// FILE Check포함
	if(intvalStr.equals(""))	// Not File -NG
	{
		rts = 1;	// NG
	}
	else
	{
		int intval = intvalStr.toInt();
		if((IMU_INTV_MIN_TIME) > intval || (IMU_INTV_MAX_TIME) < intval)	//값이 : 10ms미만~1000ms 초과  NG
		{
			rts = 1;	//NG
		}
	}
	
	return rts;
}

// File 인터벌 문자열 Return, "" - NG
String EB_IMU::getIntervalFileStr()
{
	String intvalStr = "";
	File file = SPIFFS.open(IMU_FILE, "r");
	if (!file || file.isDirectory())
	{
		return intvalStr;
	}

	// File 정상
    while (file.available())
    {
    	intvalStr += (char)file.read();
    }
	file.close();

	return intvalStr;
}

// File Save결과 Return
short EB_IMU::saveInterval(int intval)		// [ms]
{
	String intvalStr = String(intval);
	return saveIntval(intvalStr);
}

short EB_IMU::saveInterval(String intvalStr)		// [ms]
{
	return saveIntval(intvalStr);
}

// private, Return Byte수
short EB_IMU::saveIntval(String intvalStr)		// [ms]
{
	File file = SPIFFS.open(IMU_FILE, "w");
	if (!file || file.isDirectory())
	{
		return (0);
	}

	// File 정상
	short rts = file.print(intvalStr.c_str());
    file.close();

	return rts;
}

//-----------------------------------------
//	1) Interval : 100ms
//	2) out : 오일러(기본) /  GYRO : O / Distance : X
//----------------------------------------
void EB_IMU::autoSetup()
{
	String respMsg;
	send2Imuln("<stop>");
	respMsg = "stop: ";
	if(-1 == reply().indexOf("<ok>"))	{ respMsg += RESP_IMU_STOP_NG; }
	else								{ respMsg += RESP_IMU_STOP_OK; }
    delay(5);		// TBD : Delay 삭제
  
	send2Imuln("<sor100>");
	respMsg = "intval_100: ";
    respMsg += reply().equals("<ok>") ? RESP_IMU_OUTG_OK : RESP_IMU_OUTG_NG;
	delay(5);		// TBD : Delay 삭제

	// <sog = GYRO, 1= ENB Output ENB
	send2Imuln("<sog1>");
	respMsg = "gyro: ";
    respMsg += reply().equals("<ok>") ? RESP_IMU_OUTG_OK : RESP_IMU_OUTG_NG;
    delay(5);		// TBD : Delay 삭제

	// <sod = Distanse, 0 = DIS output
	send2Imuln("<sod0>");
	respMsg = "Distance: ";
    respMsg += reply().equals("<ok>") ? RESP_IMU_OUTD_OK : RESP_IMU_OUTD_OK;
    delay(5);

	send2Imuln("<start>");
	respMsg = "start: ";
    respMsg += reply().equals("<ok>") ? RESP_IMU_START_OK : RESP_IMU_START_NG;
    delay(5);

}


