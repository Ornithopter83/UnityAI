//
// ENow.h
//

#pragma once

#include "arduino.h"
#include "WiFi.h"
#include "esp_now.h"

#include "SPIFFS.h"				// File System


#define NOW_MSG_CH_SIZE	24 //28		// max 21 + Dummy(3)
typedef struct now_message {
  //int pid;
  short pid;
  //char chStr[32];
  char chStr[NOW_MSG_CH_SIZE];
} now_message;


class ENow
{
  public:
    ENow();
    void init(esp_now_recv_cb_t cbfunc);
    //void write(short pid, String str);
    void write(String pid, String str);
	void writeBC(String pid, String str);
	bool writeRaw(const unsigned char *data, unsigned int length);

    String getMyAddress();
    String getTargetAddress();
    bool setTargetAddress(String addr);
	bool isTargetAddressOk();
	int addPeerAddress(uint8_t * prAdr);
	
	int sendSeqNo = 0;
  	String sendPid;


  private:
    void bindTargetAddress();
	String strRodAddr;
	
};


//extern unsigned int new_slave_status;
extern unsigned long curr_ms_tick;
//extern const uint8_t broad_cast_addr[];
extern uint8_t broad_cast_addr[];

