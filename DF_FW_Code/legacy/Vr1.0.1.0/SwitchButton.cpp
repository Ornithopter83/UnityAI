//
//
//

#include "Common.h"
#include "SwitchButton.h"


//---------------------------------------------------
// Define PID(Button)
String PID_SWITCH = "$06";

// Define CMD Data
String BLUE_BUTTON_UP = "00000001%";		// LEFT_UP
String BLUE_BUTTON_DOWN = "00000010%";		// LEFT_DOWN
String RED_BUTTON_UP = "00000100%";			// RIGHT_UP
String RED_BUTTON_DOWN = "00001000%";		// RIGHT_DOWN

// Declare Switch Callback Funtionn
void (*_switchCallback)(String);

//
SwitchButton::SwitchButton()
{

}

// INIT SwitchButton OBJ
void SwitchButton::init()
{
	//-- Button Input
	pinMode(BTN_RED_PIN, INPUT);
	//if(isRodNewBoard())
	if(BUTTON_V2 == getButtonType())	// New
	{
		pinMode(BTN_BLUE_PIN_NEW, INPUT);
	}
	else
	{
		pinMode(BTN_BLUE_PIN, INPUT);
	}

	// READ Button of INIT Value
	redBtnToggle = digitalRead(BTN_RED_PIN);
	//if(isRodNewBoard())
	if(BUTTON_V2 == getButtonType())	// New
	{
		blueBtnToggle = digitalRead(BTN_BLUE_PIN_NEW);
	}
	else
	{
		blueBtnToggle = digitalRead(BTN_BLUE_PIN);
	}
}

//callback Function Neme : btnChangeCallback
// 	encoder.setRotateCallback(rotateChangeCallback);
void SwitchButton::setSwitchCallback(void (*switchCallback)(String))
{
  _switchCallback = switchCallback;
}

// Button Check
void SwitchButton::toggle()
{
  String str = "";

  bool redBtnState = digitalRead(BTN_RED_PIN);
  if (redBtnState != redBtnToggle)
  {
    if (redBtnState)
    {
      //str = PID_SWITCH + RED_BUTTON_DOWN;		// HIGH -> BTN PUSH
      str = "21";	// RIGHT UP
	  btnRTLedBlue_OutSts = BTN_LED_OFF;
	  digitalWrite(BTN_LED_RT_BLUE_PIN, btnRTLedBlue_OutSts);
    }
    else
    {
      //str = PID_SWITCH + RED_BUTTON_UP;			// LOW -> BTN OFF
      str = "20";	// RIGHT DOWN      
	  btnRTLedBlue_OutSts = BTN_LED_ON;
	  digitalWrite(BTN_LED_RT_BLUE_PIN, btnRTLedBlue_OutSts);

	#if (TEST_VRT_MOT)
	  	testVrtMot_Start();
	#endif

	}

    if (_switchCallback != nullptr)
    {
      _switchCallback(str);
    }
    redBtnToggle = redBtnState;
	#if LOG_BTN
		LogPrintln(" LG] R_BTN_" + str);	// PC Log
	#endif

  }

	//--- Blue Buuton
	bool blueBtnState;
	//if(isRodNewBoard())
	if(BUTTON_V2 == getButtonType())	// New
	{
		blueBtnState = digitalRead(BTN_BLUE_PIN_NEW);
	}
	else
	{
		blueBtnState = digitalRead(BTN_BLUE_PIN);
	}
  if (blueBtnState != blueBtnToggle)
  {
    if (blueBtnState)
    {
      //str = PID_SWITCH + BLUE_BUTTON_DOWN;		// HIGH -> BTN PUSH
      str = "11";	// LEFT UP
      btn_L_DownCnt = 0;	// Time Check CLEAR
	  btnLFLedRed_OutSts = BTN_LED_OFF;
	  digitalWrite(BTN_LED_LF_RED_PIN, btnLFLedRed_OutSts);
    }
    else
    {
      //str = PID_SWITCH + BLUE_BUTTON_UP;		// LOW -> BTN OFF
      str = "10";	// LEFT DOWN
	  btn_L_DownCnt = 1;	//Time Check START
	  btnLFLedRed_OutSts = BTN_LED_ON;
	  digitalWrite(BTN_LED_LF_RED_PIN, btnLFLedRed_OutSts);

	#if (TEST_VRT_MOT)
		testVrtMot_Start();
	#endif

	}

    if (_switchCallback != nullptr)
    {
      _switchCallback(str);
    }
	
    blueBtnToggle = blueBtnState;
	
	#if LOG_BTN
		LogPrintln(" LG] L_BTN_" + str);		// PC Log
	#endif
  }

  //== TM, Send SETUP Switch Level Change
  if(AP_IS_TM == apType)
  {
  	//---------------------------------
	// SETUP Switch
	bool swSetup = digitalRead(SETUP_MODE_PIN);
	if (oldswSetup != swSetup)
	{
	  if (swSetup)	{ str = "80"; } // SETUP DOWN (RED)
	  else			{ str = "81"; } // SETUP UP (YELLOW) 	 
	  if (_switchCallback != nullptr)
	  {
		_switchCallback(str);
	  }
	  oldswSetup = swSetup;
	}
	// BOOT Switch
	bool swBoot = digitalRead(BOOT_MODE_PIN);
	if (oldswBoot != swBoot)
	{
	  if (swBoot)	{ str = "91"; } // BOOT UP	(YELLOW)
	  else			{ str = "90"; } // BOOT DOWN  (RED)	 
	  if (_switchCallback != nullptr)
	  {
		_switchCallback(str);
	  }
	  oldswBoot = swBoot;
	}
	//----------------------------
  }
}
