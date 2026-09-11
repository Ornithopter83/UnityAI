// Potentiometer.h

#pragma once

#include "arduino.h"
#include "SPI.h"

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

