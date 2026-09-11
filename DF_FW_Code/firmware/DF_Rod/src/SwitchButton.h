// SwitchButton.h

#pragma once

#include "arduino.h"


class SwitchButton
{
  public:
    SwitchButton();
    void init();
    void toggle();

    void setSwitchCallback(void (*switchCallback)(String));
    
  private:
    bool redBtnToggle;		// BT1 = RIGHT Button Value if its Changed
    bool blueBtnToggle;		// BT2 = LEFT Button Value if its Changed
    //
    bool oldswSetup;
    bool oldswBoot;
};

// Extern Define
//---------------------------------------------------
extern unsigned int btn_L_DownCnt;

extern unsigned short btnLFLedRed_OutSts;
extern unsigned short btnRTLedBlue_OutSts;

extern unsigned short apType;

//---------------------------------------------------
//extern int isRodNewBoard();
extern int getButtonType();

