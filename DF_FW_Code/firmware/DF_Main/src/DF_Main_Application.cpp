#include "DF_Main_Application.h"
#include "DF_Main_Internal.h"
#include "modules/DF_Main_Scheduler.h"

void DF_Main_Application_Setup()
{
	curr_ms_tick = millis();
	old_ms_tick = curr_ms_tick;

	//== 1) Communiction
		// SERIAL 0 = USB(CDC) - ARDUIO IDE /Tool / CDC Enable [v]
	//Serial.begin(115200);		  // begin
	//Serial.setTimeout(10);		  // wait 10ms, Default 1SEC
	//Serial.setTimeout(20);		  // wait 10ms, Default 1SEC
	//Serial.setTimeout(5);		  // wait 10ms, Default 1SEC
	//Serial.println();			  // print an Empty Line

	//== 2) SERIAL_1 : LOG Serial SET
	Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);		// TXD_0 / RXD_0
	Serial1.setTimeout(10);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(30);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(15);		  // wait 10ms, Default 1SEC
	//Serial1.setTimeout(5);		  // wait 10ms, Default 1SEC
	//Serial1.println();				// print an Empty Line

	//-------------------------------------------------------------------------------
	// == 메인모터 MainMot Default VAL읽기, FILE없으면 FILE쓰기(70)
	//== 3) FILE SYS :: MANUF INFO
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON FS INIT");
	// 순서 중요: Enow Init보다 우선 실행될 것.
	fsInfo.init();

	String strBootInfo = fsInfo.getInfo(BOOTING_INFO);
	if (strBootInfo.substring(0,1) == "-") {
		strBootInfo = STR_BOOTINFO_DEFAULT;
	}
	g_AutoReboot  = strBootInfo.substring(0,1).toInt();

	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON MAIN Ver:" + mainVer + (g_AutoReboot ? "-R" : "") + ",mode:" + String(mainMode));
	LogPrintln(" LG] BOOT INFO:" + strBootInfo + ",dlytime:"+String(DEF_PWRON_DELAY_TIME) + ",sftime:"+String(DEF_USB_SAFE_TIME) + ",Max_Priority:"+String(configMAX_PRIORITIES-1));


	//== 4) SUB_AC ON Cont
	curr_ms_tick = millis();
	pinMode(SUB_ACOFF_PIN, OUTPUT);
	pinMode(PC_USB5V_PIN, INPUT_PULLUP);
	pinMode(PWROFF_SWC_PIN, INPUT);			// LOW ACTIVE

	int usb5v = digitalRead(PC_USB5V_PIN);
	int pwrSW = digitalRead(PWROFF_SWC_PIN);
	int	ledMd = 1;

	// (V108) 변경 - 공장 부팅불 장애 대책
	g_ResetReason = esp_reset_reason();		// (V108) 1:PWRON Reset, 0:USB_UART_CHIP_RESET, 3:RTC_SW_CPU_RST
	LogPrintln(" LG] SUB_ACOFF_PIN:" + String(digitalRead(SUB_ACOFF_PIN)) + ",PC_USB5V_PIN:" + String(usb5v) + ",PWROFF_SWC_PIN:" + String(pwrSW) + ",esp_reset_reason:" + String(g_ResetReason));
	if ((g_ResetReason == 0) || (g_ResetReason == 3))	{	// COM Port Reset or ESP.restart() reset
		SerialPortEnable();				// (V108)
		g_ProcStatus = PS_RUNNING;
	}
	else {
		if (pwrSW == LOW) {				// 전원스위치 On?
			SerialPortPinInput();		// (V108)
			g_ProcStatus = PS_BOOTING;
		}
		else {
			subAC_Off();				// SUB AC Off
			ioLedOffSetting();
			SerialPortEnable();			// (V108)
			ledMd = 0;
			g_ProcStatus = PS_INITIAL;
		}
	}

	//== 5) SET CONFIG (AD PORT)
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON IO SET");

	//== [ 0 ] CONFIG READ : CONFIG에 따라 포트 출력/입력 설정 다름.
	dfConfig = setConfig();

	// == 6) IO PIN SETTING
	curr_ms_tick = millis();
	ioPinSetting();

	// 보드타입 구분 판단.

/*	(V108) 앞쪽으로 옮김
	//== 7) FILE SYS :: MANUF INFO
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON FS INIT");
	// 순서 중요: Enow Init보다 우선 실행될 것.
	fsInfo.init();
*/

	//== 8) EEPROM
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON nvm Init");
	//Wire.begin(I2C_DT_PIN, I2C_CLK_PIN);				  // SDA, SCL
	eNvm.init();
	delay(50);


	if (Update.hasError()) {			// (V1081)
		Update.end(false);  // 시스템 OTA 세션 정리
		LogPrintln(" lg] System: Previous OTA error cleared");
	}

//-------------------------------------------------------------------------------
	// == 메인모터 MainMot Default VAL읽기, FILE없으면 FILE쓰기(70)
	String strMmotVal = fsInfo.getInfo(MAINMOT_INFO);
	short mMotVal = strMmotVal.toInt();
	//if( -3 == mMotVal)	// Para NG
	if( 0 > mMotVal)	// -1: Not Val, -2: No File: -3 : Para NG
	{
		defaultTorqueMotor = AUTOSET_DEFAULT_TORQ;
	}
	else if(MAIN_MOT_FS_MIN_DUTY > mMotVal || MAIN_MOT_FS_MAX_DUTY < mMotVal)	//40 ~ 100 범위 밖
	{
		fsInfo.saveInfo(MAINMOT_INFO, STR_AUTOSET_DEFAULT_TORQ);
		defaultTorqueMotor = AUTOSET_DEFAULT_TORQ;
	}
	else	// OK, 값저장
	{
		defaultTorqueMotor = mMotVal;
		// AP에 값통지는 장치에러 CHECK시 통지(TM일 경우)
	}

//-------------------------------------------------------------------------------

	// == BLDC제한값 Mot Default VAL읽기, FILE없으면 FILE쓰기( 기본값 100 )
	String strBldcVal = fsInfo.getInfo(BLDC_LIMIT_INFO);
	short iBmotVal = strBldcVal.toInt();

	// TBD, 조건문은 정사 필요.
	// Error , 기본값 = 100 제한
	if( 0 > iBmotVal)	// -1: Not Val, -2: No File: -3 : Para NG
	{
		bldcLimitVal = BLDC_LIMIT_MIN;
	}
	else if(BLDC_LIMIT_MIN > iBmotVal || BLDC_LIMIT_MAX < iBmotVal)	//100 ~ 255 범위 밖
	{
		// 파일 저장 , 기본갓 (100)
		fsInfo.saveInfo(BLDC_LIMIT_INFO, STR_BLDC_LIMIT_MIN);
		bldcLimitVal = BLDC_LIMIT_MIN;
	}
	else	// OK, 값할당
	{
		bldcLimitVal = iBmotVal;
		// AP에 값통지는 장치에러 CHECK시 통지(TM일 경우)
	}


//-------------------------------------------------------------------------------


	//== 9) eNow RF 2.4G
	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON enow INIT");
	eNow.init(recv_cb_esp_now_msg);  // esp_now
	//--- now Callback Func ( Recv from Slave Message ),  ESP-NOW 데이터 수신시 콜백 함수를 등록.
	if(ESP_OK != esp_now_register_send_cb(sent_cb_esp_now_sts))		// Data송신완료 콜백함수
	{
		LogPrintln(" LG] Error nowSent CallBack Function");
	}

//-------------------------------------------------------------------------------


	//== 10) Object INIT ( Servo, Torque, Bobbin, MainEncoder )

	curr_ms_tick = millis();
	LogPrintln(" LG] PWRON MAIN MOT INIT");
	torqMotor.init();				// torque Motor

	curr_ms_tick = millis();
	bbnMotor.init();				// Bobin Motor

	curr_ms_tick = millis();
	mainEnc.init();
	if(DF_CONFIG_LMJIG == dfConfig)
	{
		lmMotor.init();
	}
    //--- Defien Callback
	curr_ms_tick = millis();
    mainEnc.setRotateCallback(rotateChangeCallback);


	curr_ms_tick = millis();
	extLed.init();

	//== 20) Output All Off
	// Out ALL OFF
	curr_ms_tick = millis();
	motor_AllOff();
	// PowerOn, All LED OFF
	curr_ms_tick = millis();
	extLed_AllOff();

	curr_ms_tick = millis();
	ledoff_BoardLedAllOff();

	//--- External LED ON
//	curr_ms_tick = millis();
//	ledOn_MbBtm();

	// REEL ALL OFF (타이밍 검토 필요)
	//reelOut_AllOff_ForceStep();	// REEL OUT ALL OFF
	curr_ms_tick = millis();
	reelOut_AllOff();	// REEL OUT ALL OFF

	// == 21) Power On LED2 Blinking / LED4 1 times BLNK
	curr_ms_tick = millis();
	delay(200);		// LED Blink
	int cnt = 1;
	while (cnt < 10)		// 400ms = 50ms * 8 times(2~9)
	{
	cnt++;
	digitalWrite(BD_LED2_PIN, (cnt%2));		// off,on~off,on
	delay(50);
	}
	//digitalWrite(BD_LED2_PIN, LOW);

	if(DF_CONFIG_LMJIG != dfConfig)
	{
		digitalWrite(BD_LED4_PIN, HIGH);	// MAIN BOARD POWER ON
		delay(200);		// LED Blink
		digitalWrite(BD_LED4_PIN, LOW);	// MAIN BOARD POWER OFF
	}

	//== 22) Input Filtering at Power ON (3 times)
	// 1ms Input처리
	curr_ms_tick = millis();
	sensor1ms[0].old2 = 0;
	sensor1ms[0].old = 0;
	sensor1ms[0].curr = 0;
	sensor1ms[0].lvl = 0;
	sensor1ms[0].le = 0;
	sensor1ms[0].te = 0;
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);
	read_input_1ms();
	delay(1);


	curr_ms_tick = millis();
	// 10ms Input처리
	sensor10ms[0].old2 = 0;
	sensor10ms[0].old = 0;
	sensor10ms[0].curr = 0;
	sensor10ms[0].lvl = 0;
	sensor10ms[0].le = 0;
	sensor10ms[0].te = 0;
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);
	read_input_10ms();
	delay(10);

	//== 23) 미사용(삭제함:무조건 ON) : Reset시 전원스위치의 상태에 따라  AC On/OFF 삭제(다운로드시 전원 OFF됨)

  //== 24) 초기 변수값 설정

  //-- NOW통신 변수 초기 설정
  curr_ms_tick = millis();
  //now_msg.pid = -1;					// now mesage -1( not Recv Pid )
  curr_ms_tick = millis();
  //--- LED변수 초기화
  initSet_LedContVal(ledMd);		// LED VAL INIT

	if (ledMd == 0) {
		pwrMode = PWR_OFF;			//PWR_OFF
		extLed_AllOff();
	}

  //--- CONT메인 모드 설정
  mainMode = NORMAL_MODE;	// NORMAL MODE
  LogPrintln(" LG] PWRON **MAIN Mode= " + String(mainMode) + ",time:" + String(millis()-old_ms_tick)+"ms");

	// --- BBN ENC BUFF CLEAR
	for(int i=0; i<ENC_INTV_BUFF_SIZE; i++)
	{
		stEncIntvBuff[i].tick = 9999;
		stEncIntvBuff[i].width = 9999;
		stEncIntvBuff[i].dir = 0;
		stEncIntvBuff[i].enc = 9999;
		stEncIntvBuff[i].dist = 9999;
	}

	//--- SYSTEM TIMER STAMP 클리어
	curr_ms_tick = lSys1MsTime;
	old_ms_tick = curr_ms_tick;		//

	//--- Power On 1St Send CMD Req Flag SET
	pwrOn1stCmdSendFlag = 1;


	//== 30) 단독 Reset CMD송신
	ApPrintln(STX_PWRON_STS + "11%");		  // CONT MAIN BOARD RESET


	//== 31) GPIO INTR : Enc 인터럽트 추가, REL
	//attachInterrupt(digitalPinToInterrupt(ENC_INTR_PIN), encIntrHandle, RISING);	//RISING -NG : INT 2번 발생되는 경우 있음
	attachInterrupt(digitalPinToInterrupt(ENC_INTR_PIN), ISR_encIntrHandle, FALLING);


	//== 32) 하드웨어 타이머 설정 (타이머0 사용)
	timer = timerBegin(0, 80, true);  // 타이머 0, 80분주
	timerAttachInterrupt(timer, &ISR_onTimerHandler, true);  // 타이머 인터럽트 핸들러 연결
	//timerAlarmWrite(timer, 1000, true);  // 1ms마다 인터럽트 발생 (1 초 = 1000000 ticks)
	timerAlarmWrite(timer, 100, true);  // 100 US 마다 인터럽트 발생 (100 us = 100000 ticks)
	timerAlarmEnable(timer);  // 타이머 인터럽트 활성화

}

void DF_Main_Application_Loop()
{

  system_counter();  // Create system Timer(1ms / 10ms /100ms)

  if(0 < sys_count_1ms)
  {

	// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
	portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
	chk_ms_tick = lSys1MsTime;	   // current_time 값 읽기
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
	sys_count_1ms--;

	//=== 1) Input Control
	read_input_1ms();
	if(0 < sys_count_10ms)
	{
		read_input_10ms();
	}
	if(0 < sys_count_100ms)
	{
		read_input_100ms();
	}

	//=== 2) RECV CMD from AP, ROD, ExtUART, LOG
	uartRecvParsing();	// NOT DELAY(BREAK) Pasing
	uartRecvHandler();	// PC(APP) - Main UART

	nowRecvHandler();		// MAIN <-> ROD NOW(wifi RF 2.4G)


	logUartRecvParsing();
	logRecvHandler();		// PC(LOG) <-> MAIN UART

	// 3) Process Control
	// 3-1) 1ms Process
	t1ms_Process_Exec();

	// 3-2) 10ms Process
	if(0 < sys_count_10ms)
	{
		sys_count_10ms--;

		// Execution Process
		t10ms_Process_Exec();

	}

	// 3-3) 100ms Process
	if(0 < sys_count_100ms)
	{
		sys_count_100ms--;

		t100ms_Process_Exec();

		//*** WDT OUT,, MIN 900ms
		if(0 == diagOutFlag_WDToff)	// Not Diag WDT_OFF Test
		{
		}

	}

	// 3-4) 500ms Process
	if(0 < sys_count_500ms)
	{
		sys_count_500ms--;

		t500ms_Process_Exec();

		//*** WDT OUT,, MIN 900ms
		if(0 == diagOutFlag_WDToff)	// Not Diag WDT_OFF Test
		{
		}

	}


	// 3-5) 1 SEC Process
	if(0 < sys_count_1sec)
	{
		sys_count_1sec--;

		t1sec_Process_Exec();

	}

	// input LE/TE CLR
	//sensor1ms[0].le = 0;
	//sensor1ms[0].te = 0;
	//sensor10ms[0].le = 0;
	//sensor10ms[0].te = 0;


	unsigned long end_ms_tick;

		// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
		portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
		end_ms_tick = lSys1MsTime;	   // current_time 값 읽기
		portEXIT_CRITICAL(&mux);   // 임계 영역 종료

//	if (2 < (end_ms_tick-chk_ms_tick))	// 4 ms Over
	if (1 < (end_ms_tick-chk_ms_tick))	// 4 ms Over
	{
		LogPrintln(" LG] 1msExe Delay: " + String(end_ms_tick-chk_ms_tick) + "ms");
	}

  }

/*=== Test Exec Time (Serila Wait) ===
	lpCnt++;
	if( (0 < lpCnt) && (1000001 > lpCnt) )
	{
		if( !(lpCnt % 100) )
		{
			LogPrintln(String(curr_ms_tick)+"    lp:"+ String(lpCnt));
		}
	}
 */

}
