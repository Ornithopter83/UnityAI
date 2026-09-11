// Potentiometer.h

#ifndef _POTENTIOMETER_h
#define _POTENTIOMETER_h

#include "arduino.h"
#include "SPI.h"

#if defined(ESP32)
#define ESP32_BOARD
#endif

#define RECV_MIN  0
#define RECV_MAX  255

#define DEFAULT_VALUE  255
#define DEFAULT_VALUE_255  255
#define DEFAULT_VALUE_0      0

class Potentiometer
{
  public:
    Potentiometer();

    void init();
    void setValue(String _str);
    void setValue(int _val);
	void outValue(int _val);

  private:
    void digitalPortWrite(int _pinNum, int _val);
};

#endif
