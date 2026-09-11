//
//
//

#include "Common.h"
#include "Eeprom.h"


//-- Define ContByte
#define CONTWRITE 0xA0
#define CONTREAD  0xA1

//---
#define I2C_SLAVE_ADDR	0x50


Eeprom::Eeprom()
{

}

void Eeprom::init()
{
	bool rts;

	rts = Wire.begin();		// Master

	//
  #if LOG_I2C
	if(true == rts)
	{
		LogPrintln(" LG] I2C__ OK i2c begin");
	}
	else
	{
		LogPrintln(" LG] I2C__ NG i2c begin");
	}
  #endif

}


int Eeprom::cReadSend(unsigned int len)
{
	return (int)Wire.requestFrom(I2C_SLAVE_ADDR, len);
	// Read For Loop
}


//int Eeprom::read(unsigned int reqOffset, unsigned int reqLen, unsigned short* bufPtr)

int Eeprom::readSend(unsigned int reqOffset, unsigned int reqLen)
{
	
	unsigned int max = (reqOffset + reqLen);
	if( !reqLen || (0x8000 < reqLen) || (0x7FFF < reqOffset))
	{
		return (-1);	// NG
	}

	else
	{
		Wire.beginTransmission(I2C_SLAVE_ADDR);
		Wire.write( (byte)(reqOffset & 0x7F00 >> 8) );	// MSB
		Wire.write( (byte)(reqOffset & 0x00FF) ); // LSB
		//int rts = Wire.endTransmission();
		int rts = Wire.endTransmission(false);				// Do not release the bus after transmission

		Wire.requestFrom(I2C_SLAVE_ADDR, reqLen);
		
		return (rts);	//OK
	}

}

int Eeprom::read(byte* buffPtr, unsigned int reqOffset, unsigned int reqLen)
{
	int rdLen = Wire.available();
	if(rdLen)
	{
		unsigned int i;
		for(i=reqOffset; i< rdLen; i++)
		{
			buffPtr[i]= Wire.read();
		}
	}
	// TBD (rdlen <==> reqLen)
	return rdLen;
}

// TBD
int Eeprom::buffSave(unsigned int reqOffset, unsigned int reqLen)
{
	
	unsigned int max = (reqOffset+reqLen);
	if( !reqLen || (0x8000 < reqLen) || (0x7FFF < reqOffset))
	{
		return (-1);	// NG
	}

	else
	{
		unsigned int i,j;
		
		if(0x7FFF < max)
		{
			for(j=reqOffset; j<0x8000; j++)
			{
				buff[j] = Wire.read();
			}
 			max -= 0x8000;
			for(i=0; i<max; i++)
			{
				buff[i] = Wire.read();
			}
			i= i+(j-reqOffset+1);
		}
		else
		{
 			for(i=reqOffset; i<max; i++)
			{
				buff[i] = Wire.read();
			}
			i=(i-reqOffset+1);
		}

		return (i);	//OK
	}

}

//----------------------------------------
//	eeprom Data Display (LOG OUT)
//----------------------------------------
void Eeprom::grazingGeddong(byte* buffPtr, unsigned int reqAddr, unsigned int reqLen)
{

	unsigned int max = (reqAddr+reqLen);
	String buffStr="";
	int i = 0;

	//ubyte* bufPtr = buffPtr;
	for(i=reqAddr; i<max; i++)
	{
		buffStr += nib2char(buffPtr[i]);
	}

	#if LOG_I2C
	LogPrintln(" LG] NvmBF grazingView:" + String(reqLen) + ", str: " + buffStr);
	#endif
}

String Eeprom::nib2char(byte _nib)
{
	String str="";
	char nibChar;
	byte nib2;
	
	nib2 = (_nib & 0xF0) >> 4;
	if(10 > nib2 ) { nibChar = nib2 + '0'; }
	else		   { nibChar = nib2 - 10 + 'A'; }
	str += nibChar;
	
	nib2 = (_nib & 0x0F);
	if(10 > nib2 ) { nibChar = nib2 + '0'; }
	else		   { nibChar = nib2 - 10 + 'A'; }
	str += nibChar;

	str += ",";

	return str;	
}
//callback Function Name : rotateChangeCallback


int Eeprom::writeSend(unsigned int reqOffset, unsigned int reqLen)
{

}

int Eeprom::write(byte *buffPtr, unsigned int reqOffset, unsigned int reqLen)
{
	
	unsigned int max;
	max = (reqOffset + reqLen);
	if( !reqLen || (0x7FFF < reqOffset) || (0x8000 < max) )
	{
		return (-1);	// NG
	}
	else
	{
		Wire.beginTransmission(I2C_SLAVE_ADDR);
		Wire.write( (byte)(reqOffset & 0x7F00 >> 8) );
		Wire.write( (byte)(reqOffset & 0x00FF) );
		unsigned int i;
		for(i=reqOffset; i<max; i++)
		{
			Wire.write(buffPtr[i]);
		}
		return (int)Wire.endTransmission();
	}
}


/*

void Eeprom::setReadCallback(void (*rotateCallback)(int _dir, int _cnt))
{
  _rotateCallback = rotateCallback;
}

*/

/*
//
//
*/
void Eeprom::checksum()
{


 
}

/*
String Encoder::fillZero(int _count)
{
  String str;
  if (_count < 10)
  {
    str = "0000" + String(_count);
  }
  else if (_count < 100)
  {
    str = "000" + String(_count);
  }
  else if (_count < 1000)
  {
    str = "00" + String(_count);
  }
  else if (_count < 10000)
  {
    str = "0" + String(_count);
  }  else
  {
    str = String(_count);
  }
  return str;
}
*/

