//
//
//

#include "Common.h"
#include "FileSys.h"

//--- Define EXTERN
extern void esp_soft_reset();
//extern unsigned int new_slave_status;
extern unsigned long curr_ms_tick;

//--- END Extern--------------------------------------------------

//--- Define VAR
//static const char *ptrMANUFAC_FILE = "/manufInfo.txt";
//static const char *ptrINSTALL_FILE = "/installInfo.txt";
static char *ptrMANUFAC_FILE = "/manufactInfo.txt";
static char *ptrINSTALL_FILE = "/installInfo.txt";
static char *ptrMAINMOT_FILE = "/mainMotInfo.txt";
static char *ptrBLDC_MOT_FILE = "/bldcMotInfo.txt";
static char *ptrBOOTINFO_FILE = "/bootingInfo.txt";			// (V108)


//--- Define Function
FileSys::FileSys()
{

}

/*-------------------------------------------------------------
  INIT FILE System MOUNT & FORMAT
-------------------------------------------------------------*/
void FileSys::init()
{

	// 1) Begin FILE System
	if (!SPIFFS.begin())
	{
		SPIFFS.format();			// FILE Format
	
		//== TBD Improve Reset
		//ESP.restart();			// ESP Reset
		esp_soft_reset();
	}

#if (LOG_MANUF_INFO)
	LogPrintln(" lg] ManuF Init Mount & Format Ok");	
#endif
  
}


/*-------------------------------------------------------------
  READ to ManuFatory INFO
//-------------------------------------------------------------
    FS 종류
   1 - 제조 정보
   2- 설치 정보
   	3 - 메인모터 토크 기본값(40~100)
-------------------------------------------------------------*/
String FileSys::getInfo(short fsKind)
{

	String strReturn;
	char *ptrFileName;

	switch(fsKind)
	{
		case MANUF_INFO :
			ptrFileName = ptrMANUFAC_FILE;
			strReturn = NO_FILE_MANUF_INFO;		// 파일없음(9999)
			break;
		case INSTALL_INFO:
			ptrFileName = ptrINSTALL_FILE;
			strReturn = NO_FILE_INSTALL_INFO;		// 파일없음(9999)
			break;

		case MAINMOT_INFO:
			ptrFileName = ptrMAINMOT_FILE;
			strReturn = NO_FILE_FS_INFO;	// Not File (-2)
			break;

		case BLDC_LIMIT_INFO:
			ptrFileName = ptrBLDC_MOT_FILE;
			strReturn = NO_FILE_FS_INFO;	// Not File (-2)
			break;

		case BOOTING_INFO:									// (V108)
			ptrFileName = ptrBOOTINFO_FILE;
			strReturn = NO_FILE_FS_INFO;	// Not File (-2)
			break;

		default:
			return PARA_NG_FS_INFO;		// Para NG(-3)
			break;
	}

	//File file = SPIFFS.open(MANUF_FILE, "r");
	File file = SPIFFS.open(ptrFileName, "r");
	
	//--- NG: 파일이 아니거나 디렉토리이면 NG => 바로 REETURN, 처리 중단
	if (!file || file.isDirectory())
	{
		return strReturn;	// NotFile or ERR
	}

	//---- FILE정상시 --------------------
	if(file.available())	// File내 내용이 있으면?
	{
		strReturn = "";
		while (file.available())
		{
  			strReturn += (char)file.read();	// 값이 숫자,문자 혼합이면?
		}
	}
	else					// File내 내용이 없으면? (NULL?)
	{
		strReturn = NO_VAL_FS_INFO;	// Data없음. NG	
	}
	//---------------------------------
	
	if(MAINMOT_INFO == fsKind)		// 숫자 철리할때 만.
	{
		if(false == isNumeric(strReturn))
		{
			strReturn = NO_VAL_FS_INFO;	// 숫자+문자 혼합. NG
		}
	}
  
#if (LOG_MANUF_INFO)
	  LogPrintln(" LG] ManuF get:" + strReturn);
#endif

  file.close();
  return strReturn;
}

// 문자열 숫자 판별
/*
bool FileSys::isNumeric(char* str)
{
  while (*str) {
    if (!isdigit(*str)) return false;
    str++;
  }
  return true;
}
*/

bool FileSys::isNumeric(String str)
{
  for (size_t i = 0; i < str.length(); i++)
  {
    if (!isDigit(str.charAt(i)))
	{
      return false;
    }
  }
  return true;
}

/*-------------------------------------------------------------
  SAVE FILE to ManuFatory INFO
-------------------------------------------------------------*/
short FileSys::saveInfo(short fsKind, String _strInfo)
{
	short length = 0;
	char *ptrFileName;

	switch(fsKind)
	{
		case MANUF_INFO:
			ptrFileName = ptrMANUFAC_FILE;
			break;

		case INSTALL_INFO:
			ptrFileName = ptrINSTALL_FILE;
			break;

		case MAINMOT_INFO:
			ptrFileName = ptrMAINMOT_FILE;
			break;
		
		case BLDC_LIMIT_INFO:
			ptrFileName = ptrBLDC_MOT_FILE;
			break;
		
		case BOOTING_INFO:									// (V108)
			ptrFileName = ptrBOOTINFO_FILE;
			break;
		
		default:
//			return (-2);		//ERR : CMD PARA NG
			return (NUM_PARA_NG_FS_INFO);		//ERR : CMD PARA NG
			break;
	
	}


	File file = SPIFFS.open(ptrFileName, "w");
	// -- NG?
	if (!file || file.isDirectory())
	{
//		  return (-1);			// ERR : FILE OPEN NG
		  return (NUM_OPEN_FAIL_FS_INFO);			// ERR : FILE OPEN NG
	}

	//-- OK?
	length = file.print(_strInfo.c_str());		// return Write Byte Length
	#if (LOG_MANUF_INFO)
		LogPrintln(" LG] Info FS save:" + String(length));
	#endif

	file.close();
	return length;
}

/*-------------------------------------------------------------
  DELETE File of Manuf Info
   리턴값 
   	- 성공(TRUE :1) 	0 : ERR_OK
   	- 실패(FALSE:0)	-1 : ERR_NG
  
-------------------------------------------------------------*/
short FileSys::deleteInfo(short fsKind)
{
	//saveInfo("");	//

	char *ptrFileName;

	switch(fsKind)
	{
		case MANUF_INFO:
			ptrFileName = ptrMANUFAC_FILE;
			break;

		case INSTALL_INFO:
			ptrFileName = ptrINSTALL_FILE;
			break;

		case MAINMOT_INFO:
			ptrFileName = ptrMAINMOT_FILE;
			break;

		case BLDC_LIMIT_INFO:
			ptrFileName = ptrBLDC_MOT_FILE;
			break;

		case BOOTING_INFO:									// (V108)
			ptrFileName = ptrBOOTINFO_FILE;
			break;

		default:
			return NUM_PARA_NG_FS_INFO;	// PARA NG
			break;
				
	}
	return (SPIFFS.remove(ptrFileName)? 0:-1);
}

/*
short FileSys::checkValue(short fsKind)
{

	String strVal = "";
	
	if(MAINMOT_INFO != fsKind)
	{
		return 0;
	}

	short rts = 0;
	// MAIN_MOT_DEFAULT DUTY일때만 처리
	
	strVal = getInfo(fsKind);
	if(strVal.equals("") || strVal.equals(NOT_DEF_MAINMOT_INFO) || strVal.equals(NO_VAL_MAINMOT_INFO))	// Not File -NG
	{
		rts = 1;	// NG
	}
	else
	{
		int intval = strVal.toInt();
		if((MAIN_MOT_DEFAULT_MIN_DUTY) > intval || (MAIN_MOT_DEFAULT_MAX_DUTY) < intval)	//값이 : 10ms미만~1000ms 초과  NG
		{
			rts = 1;	//NG
		}
	}
	
	return rts;

}
*/

//-----------------------------------------------------------------------
//	Manu Info Binding
//-----------------------------------------------------------------------
#if 0
void FileSys::binding()
{

	String addr = getTargetAddress();

	// IMP TB
	if( (16 < addr.length()) && (31 > addr.length()) )
	{
		char str[32];
		addr.toCharArray(str, addr.length() + 1);			//insert char array[32] from String
		char* ptr;

		char* stPtr;

		// Improved, Reset Trouble Shooting, ADD Check NULL POINT
		stPtr = strtok(str, ":");		// load 1st Addr STRING
		if(NULL != stPtr)
		{
			for ( uint8_t i = 0; i < 6; i++ )
			{
				//slave_board_addr[i] = strtol( strtok( NULL, ":"), &ptr, HEX );
				slave_board_addr[i] = strtol( stPtr, &ptr, HEX );	// Save Addr HEX
				
				stPtr = strtok( NULL, ":");	// Load 2nd ~ 6th Addr STRING
				if(NULL == stPtr)
				{
					break;	// STOP, FOR LOOP
				}
				else
				{
					// NA
				}
			}
  		}
		#if (LOG_TART_ADDR_LOD)
		LogPrintln(" LG] T_ADR rod: " + String(slave_board_addr[0]) + "," + String(slave_board_addr[1]) + "," + String(slave_board_addr[2]) + ","+ String(slave_board_addr[3]) + ","+ String(slave_board_addr[4]) + ","+ String(slave_board_addr[5]) );
		#endif
	}
	else
	{
		LogPrintln(" LG] T_ADR Load NG, addr= " + addr);
	}

}
#endif


