//
//
//
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include <esp_sleep.h>

#include "Common.h"
#include "ENow.h"

// Define EXTERN
extern void set_slave_status(unsigned int sts);

//--- end Extern

static const char *ADDR_FILE = "/addr.txt";
//static const char *MANUF_FILE = "/manufInfo.txt";

uint8_t slave_board_addr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};	// REL
//uint8_t slave_board_addr[] = {0x48, 0x27, 0xE2, 0x13, 0xB5, 0x70};	// TEST

esp_now_peer_info_t peerInfo;

ENow::ENow()
{

}

/*-------------------------------------------------------------
  INIT now Commucation( RF 2.4GHz )
-------------------------------------------------------------*/
void ENow::init(esp_now_recv_cb_t cbfunc)
{
	//== 1) File시스템 마운트& 포맷
	//    => 이동 FILESYSTEM.cpp

  	//==2) 타겟주소 읽어 설정
	bindTargetAddress();		// SAVE Slave Address[0~5] by HEX DATA

	//== 2-1) Check Target Address is Not SET ( FF:FF:FF:FF:FF:FF)
	if(!isTargetAddressOk())
	{
  		//new_slave_status = SLAVE_DISCONN_MAC;		// Mac Not Set
  		//set_slave_status(SLAVE_R_DIS_MAC);
  		LogPrintln(" lg] Error eNOW ADDR NG:" + strRodAddr);
  	}
 
	//==3) Set Mode : ESP_NOW
	  /* WIFI_STA : station mode: the ESP32 connects to an access point 
	    WIFI_AP : access point mode: stations can connect to the 
	    ESP32 WIFI_AP_STA : access point and a station connected to another access point
	    출처: https://cocoastudy.tistory.com/353 [EIGHTBOX:티스토리]
	  */
	WiFi.mode(WIFI_STA);

	WiFi.disconnect();

	// 채널 고정
	esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

	//== 4) INIT esp_now
		// NG - Unlimit Loop,OK-next
	if (ESP_OK != esp_now_init())		// ESP-NOW를 초기화
	{
  		//new_slave_status = SLAVE_DISCONN_NOWINITNG;		//NOW_INIT_NG
  		// set_slave_status( SLAVE_DISCONN_NOTINIT);
    	LogPrintln("(Err) ESP NOW Init Error.");
		// 무한루프 개선 필요
		/*
    	while (1)
    	{
      		delay(1000);
    	}
    	*/
    	return;
	}

	//== 5) Add peer :페어링된 장치 목록에 장치를 추가
	memcpy(peerInfo.peer_addr, slave_board_addr, 6);
	peerInfo.channel = 0;			// AUTO CHANNEL
	peerInfo.encrypt = false;		// 멀티캐스트 공급업체별 작업 프레임 암호화는 지원
	if (ESP_OK != esp_now_add_peer(&peerInfo))
	{
  		//new_slave_status = SLAVE_DISCONN_ADDRPEERNG;		//PEER ADDR NG
  		//set_slave_status(SLAVE_DISCONN + SLAVE_DISCONN_NOTPEER);
		LogPrintln(" LG] PwrOn (Err) ESP NOW Add Peer Error.");

		// 무한루프 개선 필요
		/*
    	while (1)
    	{
      		delay(1000);
    	}
		*/
		return;
	}

	//== 6) now Callback Func ( Recv from Slave Message ),  ESP-NOW 데이터 수신시 콜백 함수를 등록.
	//esp_now_register_recv_cb(cbfunc);
	if(ESP_OK != esp_now_register_recv_cb(cbfunc))	// Data수신 콜백함수
	{
		LogPrintln(" LG] Error nowRecv CallBack Function");
	}
	LogPrintln(" LG] PwrOn eNOW INIT OK");



  
}

/*-----------------------------------------------------------------------
	Save [ slave_board_addr[0]~[5], Change Mac Address to Hex Data  from STRING with ':'
-----------------------------------------------------------------------*/
void ENow::bindTargetAddress()
{

	String addr = getTargetAddress();
	strRodAddr = addr;

	// IMP TB
	if( (16 < addr.length()) && (31 > addr.length()) )
	{
		char str[32];
		addr.toCharArray(str, addr.length() + 1);			//insert char array[32] from String
		char* ptr;

		char* stPtr;

		// Improved, RESET Trouble Shooting, ADD Check NULL POINT
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
		LogPrintln(" LG] T_ADR rod: " + String(slave_board_addr[0]) + "," + String(slave_board_addr[1]) + "," + String(slave_board_addr[2]) + ","+ String(slave_board_addr[3]) + ","+ String(slave_board_addr[4]) + ","+ String(slave_board_addr[5]) );
	}
	else
	{
		LogPrintln(" LG] T_ADR Load NG, addr= " + addr);
	}

}



/*-------------------------------------------------------------
  SEND to Slave NOW Message ( pid + String )
-------------------------------------------------------------*/
//void ENow::write(short pid, String str)
void ENow::write(String pid, String str)
{
	char msg[128];
	String cmd = pid + str;			// String ID+MSG
	strcpy(msg, cmd.c_str());
	int len = cmd.length();

	//esp_err_t ret = esp_now_send(slave_board_addr, (uint8_t *) &msg, sizeof(msg));
	esp_err_t errRet = esp_now_send(slave_board_addr, (uint8_t *) &msg[0], len);
		if(ESP_OK != errRet)
		{
			LogPrintln(" lg] Error now Send:" + pid +",m:" + str);
		}

	
	// 연속으로 NOW송신의 경우는 아래 변수가 OverWrite됨(LOG확인시 주의 요망)
	sendPid = pid;	//LOG용

}

bool ENow::writeRaw(const unsigned char *data, unsigned int length)
{
	if ((NULL == data) || (0 == length) || (250 < length))
	{
		return false;
	}
	return ESP_OK == esp_now_send(slave_board_addr, data, length);
}

void ENow::writeBC(String pid, String str)
{
  //now_message msg;
  //strcpy(msg.chStr, str.c_str());
  //msg.pid = pid;

  char msg[128];
  str = pid + str;			  // String ID+MSG
  strcpy(msg, str.c_str());
  int len = str.length();

	esp_err_t errRet = esp_now_send(broad_cast_addr, (uint8_t *) &msg[0], len);
		if(ESP_OK != errRet)
		{
			LogPrintln(" lg] Error now Send:" + pid +",m:" + str);
		}


	// 연속으로 NOW송신의 경우는 아래 변수가 OverWrite됨(LOG확인시 주의 요망)
	sendPid = pid;	//LOG용
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
String ENow::getTargetAddress()
{
  String addr = "FF:FF:FF:FF:FF:FF";

  int seq, i;
  int length = 0;

  File file = SPIFFS.open(ADDR_FILE, "r");

  // OK ? TBD 조건문 개선 (객체없는 것과 디렉토리인거 구분 처리)
  if (file && !file.isDirectory())
  //if (file && file.isDirectory())
  {

	// TB-Main Reboot At MAC Addr("00"")
	//---------------------------------
    addr = "";
    while (file.available())
    {
      addr += (char)file.read();
    }
	//---------------------------------


  }

  file.close();
  
  return addr;
}

/*-------------------------------------------------------------
  Write FILE to Target Mac Address
-------------------------------------------------------------*/
bool ENow::setTargetAddress(String addr)
{
	bool rts = true;
	File file = SPIFFS.open(ADDR_FILE, "w");
	//-- FS NG?
	if (!file)
	{
		return false;
	}

	//-- FS OK?
	rts = file.print(addr.c_str());		// return Write Byte Length

	file.close();

	return rts;
}

/*-------------------------------------------------------------
  if Address = FF:FF:FF:FF:FF:FF : false(0), ELSE : true(1)
-------------------------------------------------------------*/
bool ENow::isTargetAddressOk()
{
  bool bRtn = false;			// (0) NG: MAC NOT SET(모두 FF면 false)
  for (int i = 0; i < 6; i++ )
  {
    if ( 0xFF != slave_board_addr[i])
	{
		bRtn = true;	//(1) OK: ADDR SET, 한개라도 FF가 아니면 OK 
		break;
	} 
  }
  return bRtn;
}

int ENow::addPeerAddress(uint8_t * prAdr)
{
	memcpy(peerInfo.peer_addr, prAdr, 6);
	peerInfo.channel = 0;		  // AUTO CHANNEL
	peerInfo.encrypt = false;	  // 멀티캐스트 공급업체별 작업 프레임 암호화는 지원

	int ret;
	ret = esp_now_add_peer(&peerInfo);
	if (ESP_OK != ret)
	{
		LogPrintln(" LG] Error Func PEER ADD" + String(ret));
	}
	LogPrintln(" LG] NowPeer ADD OK");
	
	// Log PEER(Device) LIST
	esp_now_peer_num_t rodPeer;
	esp_now_get_peer_num(&rodPeer);
	LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
	
	return ret;
}

