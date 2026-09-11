// ENow.h

#pragma once

#include "arduino.h"
#include "WiFi.h"
#include "esp_now.h"

#include "SPIFFS.h"

#define NOW_MSG_CH_SIZE	24 //28		// max 21 + Dummy(3)
typedef struct now_message {
  //int pid;
  short pid;
  char str[NOW_MSG_CH_SIZE];
} now_message;

class ENow
{
  public:
    ENow();
    void init(esp_now_recv_cb_t cb);
    void write(String pid, String str);
    void writeBC(String pid, String str);
	bool writeRaw(const unsigned char *data, unsigned int length);

    void bindTargetAddress();
    String getMyAddress();
    String getTargetAddress();
    bool setTargetAddress(String addr);
	void fsFormat();
	int addPeerAddress(uint8_t * prAdr);

	int sendSeqNo = 0;
	String sendPid;
  
  private:
};

//----------------------------------------
extern void	ESP_Soft_Reset();

