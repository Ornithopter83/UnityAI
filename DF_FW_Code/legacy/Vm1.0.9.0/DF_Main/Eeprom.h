//
// EEPROM.H
// I2C
//	kps
//

#ifndef _EEPROM_h
#define _EEPROM_h

#include "arduino.h"
#include "Wire.h"


#define EEP_TOT_SIZE 32*1024	// 32K byte
#define EEP_PAGE_SIZE 1024	// 32K byte


#define EEP_PAGE1	0
#define EEP_PAGE2	1

#define EEP_PAGGE32	31

#define NVM_ADDR_1ST	(EEP_PAGE1 * 1)

class Eeprom
{
  public:
    Eeprom();

    void init();
	int cReadSend(unsigned int len);

	int readSend(unsigned int offsetAddr, unsigned int reqLen);
	int buffSave(unsigned int offsetAddr, unsigned int reqLen);
	int read(byte *buffPtr, unsigned int offsetAddr, unsigned int reqLen);

	int writeSend(unsigned int offsetAddr, unsigned int reqLen);
	int write(byte *buffPtr, unsigned int offsetAddr, unsigned int reqLen);
	
	void grazingGeddong(byte* buffPtr, unsigned int reqAddr, unsigned int reqLen);
	
    void checksum();
	
    //void setReadCallback(void (*rotateCallback)(int _dir, int _cnt));
    
    int page = 0;
	int addr = 0;
	int length = 0;
	int value = 0;

	byte buff[EEP_PAGE_SIZE];

  private:
    int count;
	String nib2char(byte _nib);
};

// Extern VAR
extern unsigned long curr_ms_tick;


#endif
