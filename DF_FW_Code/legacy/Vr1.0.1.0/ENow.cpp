//
//
//
#include "Common.h"
#include "ENow.h"

static const char *ADDR_FILE = "/addr.txt";			// enow Target Addr
uint8_t main_board_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broad_cast_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

ENow::ENow()
{

}

void ENow::fsFormat()
{
		#if (LOG_FS_FORMAT)
			LogPrintln(" LG] SPIFFS Format Start");
		#endif
		
    	SPIFFS.format();	// About 30sec
		
		#if (LOG_FS_FORMAT)
			LogPrintln(" LG] SPIFFS Format END");
		#endif
		
}
	
/*-----------------------------------------------------------------------
	Save [ main_board_addr[0]~[5], Change Mac Address to Hex Data  from STRING with ':'
-----------------------------------------------------------------------*/
void ENow::bindTargetAddress()
{

#if 0	// NOT_USE => Move Setup Func CALL
	// FILE System INIT with FORMAT & Reset
	if (!SPIFFS.begin())		// without FORMAT
	{
		#if (LOG_FS_FORMAT)
			LogPrintln(" LG] FS Format Start");
		#endif
		
    	SPIFFS.format();	// About 30sec
		
		#if (LOG_FS_FORMAT)
			LogPrintln(" LG] SPIFFS NG, FORMAT, then EPS Restart");
		#endif
		
		//delay(2000);
		delay(10);		// Format후 Reset 안정화(?) 시간
		
    	//ESP.restart();
		ESP_Soft_Reset();
	}

	// OK시만 처리됨
	#if (LOG_SPIFFS_OK)
		LogPrintln(" LG] SPIFFS OK");
	#endif
#endif

	// Load Target Address to FILES
	// TBD : 타켓주소를 읽지 못하는 경우, 주소 처리및 NOW PEER처리는 추후
	String addr = getTargetAddress();

#if (1)		// FIX
	// Target Addr : 17 har ~ 30 Char String
	//if( MAC_ADDR_OK_LEN == addr.length())	// 꼭 17자리 문자?
	if( (16 < addr.length()) && (31 > addr.length()) )
	{
  		// Change Target Addess to  main_board_sddr[0]~[5] from STRING
		char str[32];
		addr.toCharArray(str, addr.length() + 1);
		char* ptr;

		char* stPtr;
		int i = 0;

		// Improved, Reset Trouble Shooting, ADD Check NULL POINT
		stPtr = strtok(str, ":");
		if(NULL != stPtr)
		{
			for (i = 0; i < 6; i++ )
			{
				//main_board_addr[0] = strtol( strtok(str, ":"), &ptr, HEX );
				main_board_addr[i] = strtol( stPtr, &ptr, HEX );
		
				stPtr = strtok( NULL, ":");
				if(NULL == stPtr)
				{
					break;	// Finish FOR
				}
				else
				{
					//main_board_addr[i] = strtol( strtok( NULL, ":"), &ptr, HEX );
					//main_board_addr[i] = strtol( stPtr, &ptr, HEX );
				}
			}
		}
	  #if (LOG_TARGET_LOAD)
		LogPrintln(" LG] T_ADDR Load OK, addr= " + addr);
	  #endif
	}
	else
	{
	  #if (LOG_TARGET_LOAD)
		LogPrintln(" LG] T_ADDR Load NG, addr= " + addr);
	  #endif
	}
	
// OLD -NG(RESET at without NULL Pointer Check)	
#else
	if( 16 < addr.length())
	{
  		// Change Target Addess to  main_board_sddr[0]~[5] from STRING
		char str[32];
		addr.toCharArray(str, addr.length() + 1);
		char* ptr;

		main_board_addr[0] = strtol( strtok(str, ":"), &ptr, HEX );
		for ( uint8_t i = 1; i < 6; i++ )
		{
			main_board_addr[i] = strtol( strtok( NULL, ":"), &ptr, HEX );
		}
	  #if (LOG_TARGET_LOAD)
		LogPrintln(" LG] T_ADDR Load OK, addr= " + addr);
	  #endif
	}
	else
	{
	  #if (LOG_TARGET_LOAD)
		LogPrintln(" LG] T_ADDR Load NG, addr= " + addr);
	  #endif
	}

#endif

}

/*-------------------------------------------------------------
  INIT now Commucation( RF 2.4GHz )
-------------------------------------------------------------*/
void ENow::init(esp_now_recv_cb_t cb)
{
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    //Serial.println("(Err) ESP NOW Init Error.");
    LogPrintln(" LG] Error NOW INIT NG");
    /*
    while (1)
    {
      delay(1000);
    }
    */
    return;
  }

  // Register peer
  memcpy(peerInfo.peer_addr, main_board_addr, 6);
  peerInfo.channel = 0;			// Auto Channel
  //peerInfo.channel = 6;			// FIX Channel 6 => NG, 통신 안됨
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    //    Serial.println("(Err) ESP NOW Add Peer Error.");
    LogPrintln(" LG] Error NOW PEER NG");
	/*
    while (1)
    {
      delay(1000);
    }
    */
    return;
  }

  // Regist CALL BACK Func (  recv_cb_esp_now_msg )
  if( ESP_OK != esp_now_register_recv_cb(cb))
  {
	LogPrintln(" LG] Error NOW RCV_CB_REGIST NG");
  }

#if (LOG_NOW_GET_INFO)
	// now Version
	uint32_t nowVer;
	esp_now_get_version(&nowVer);		//*** Result Ver = 1
	// now Channel
	uint8_t now1stCh = WiFi.channel();	// Result CH =1
	int now2ndCh = 255;
	// now Power
	int nowPower = WiFi.getTxPower();	// Result PWR = 80
  	// Log PEER(Device) LIST
  	esp_now_peer_num_t rodPeer;
  	esp_now_get_peer_num(&rodPeer);
	LogPrintln(" LG] nowGET Ver:" + String(nowVer) +",Ch:" +String(now1stCh) +",Pwr:" + String(nowPower)+",PeerNum:" + String(rodPeer.total_num));	

	//WiFi.setTxPower(20);		//Max 20dBm = 100W
	//WiFi.setTxPower(WIFI_POWER_19_5dBm);		// Test 19dBm = 80mW
	//nowPower = WiFi.getTxPower();
	//LogPrintln(" LG] eNow set Power = " + String(nowPower));

#endif

}


/*------------------------------------------------------------------------
    SEND to MAIN, NOW Message ( pid + String )
------------------------------------------------------------------------*/
void ENow::write(String pid, String str)
{
  //now_message msg;
  //strcpy(msg.str, str.c_str());
  //msg.pid = pid;

  char msg[128];

  str = pid + str;			  // String ID+MSG
 // str = "$"+ pid + str + "#";			  // String ID+MSG

  strcpy(msg, str.c_str());
  int len = str.length();

	esp_err_t errRet = esp_now_send(main_board_addr, (uint8_t *) &msg, len);
	#if (LOG_NOW_SEND_ERR)
		if(ESP_OK != errRet)
		{
			LogPrintln(" lg] Error now Send:" + pid +",m:" + str);
		 }
	#endif
  
	#if (LOG_NOW_SEND_SEQNO)
	  if(pid.equals(STR_PID_ALIVE_RESP)) { sendSeqNo++; } 	// test Send SeqNo
	  LogPrintln(" LG] nowSend id:" + pid + ",Seq:" + String(sendSeqNo));
	#endif
  
	  // 연속으로 NOW송신의 경우는 아래 변수가 OverWrite됨(LOG확인시 주의 요망)
	  sendPid = pid;  //LOG용
}

//----------------------------------------------------------------
// Send BROAD CAST
void ENow::writeBC(String pid, String str)
{
#if 0
  memcpy(peerInfo.peer_addr, broad_cast_addr, 6);
  peerRet = esp_now_add_peer(&peerInfo);
  if (ESP_OK != peerRet)
  {
	LogPrintln(" LG] ERROR nowPeer ADD: " + String(peerRet));
  #if 0
  	// Log PEER(Device) LIST
  	esp_now_get_peer_num(&rodPeer);
	LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
	return;
  #endif
  }

  // Log PEER(Device) LIST
  esp_now_get_peer_num(&rodPeer);
  LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
#endif

  //now_message msg;
  ///strcpy(msg.str, str.c_str());
  //msg.pid = pid;

  char msg[128];

  str = pid + str;			  // String ID+MSG
  //str = "$"+ pid + str + "#";			  // String ID+MSG

  strcpy(msg, str.c_str());
  int len = str.length();
  
	esp_err_t errRet = esp_now_send(broad_cast_addr, (uint8_t *) &msg, len);		// Send NG
	#if (LOG_NOW_SEND_ERR)
		if(ESP_OK != errRet)
		{
			LogPrintln(" lg] Error now Send:" + pid +",m:" + str);
		 }
	#endif
  
	#if (LOG_NOW_SEND_SEQNO)
		if(pid.equals(STR_PID_ALIVE_RESP)) { sendSeqNo++; }  // test Send SeqNo
		LogPrintln(" LG] nowSend id:" + pid + ",Seq:" + String(sendSeqNo));
	#endif
	// 연속으로 NOW송신의 경우는 아래 변수가 OverWrite됨(LOG확인시 주의 요망)
	sendPid = pid;	//LOG용

	//-----------------------------------------------------------
	// BC PEER삭제
	esp_err_t peerRet;
	esp_now_peer_num_t rodPeer;
#if 1		// BC PEER 삭제
	peerRet = esp_now_del_peer(broad_cast_addr);
	if (ESP_OK != peerRet)
	{
	  LogPrintln(" LG] ERROR nowPeer DEL: " + String(peerRet));
	}
	esp_now_get_peer_num(&rodPeer);
	LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
#endif

}



/*-------------------------------------------------------------
  READ my Mac Address
-------------------------------------------------------------*/
String ENow::getMyAddress() {
  return WiFi.macAddress();
}

/*-------------------------------------------------------------
  READ target Address from FILE
-------------------------------------------------------------*/
String ENow::getTargetAddress() {
	String addr = "FF:FF:FF:FF:FF:FF";
	//String addr = "FF:FF:FF:FF:FF:F";
	int seq;
	File file = SPIFFS.open(ADDR_FILE, "r");
	if (!file || file.isDirectory())
	{
		// FILE장애, FF~FF 주소 반환
		// 장애처리 = 처리못함 : 통신주소가 없어 CONT에 장애코드를 보낼수 없음(Reel연결안됨 장애로 대체)
  		return	addr;
	}

	// FILE정상
    addr = "";
    while (file.available())
    {
      addr += (char)file.read();
    }
	file.close();
	return addr;
}

/*-------------------------------------------------------------
  Write FILE to Target Mac Address
-------------------------------------------------------------*/
bool ENow::setTargetAddress(String addr)
{
	File file = SPIFFS.open(ADDR_FILE, "w");
	if (!file || file.isDirectory())
	{
		// FILE장애
		// 장애처리 : FILE 쓰기 NG -> TBD, LOG출력
		return false;
	}

	// FILE정상
	bool rts = file.print(addr.c_str());
    file.close();

	return rts;
}

int ENow::addPeerAddress(uint8_t * prAdr)
{
	int ret;
	// channel,encrypt is alredy set
	memcpy(peerInfo.peer_addr, prAdr, 6);
	ret = esp_now_add_peer(&peerInfo);
	if (ESP_OK != ret)
	{
		LogPrintln(" LG] Error NOW PEER ADD NG" + String(ret));
	}
	
	// Log PEER(Device) LIST
	esp_now_peer_num_t rodPeer;
	esp_now_get_peer_num(&rodPeer);
	LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
	
	return ret;
}

