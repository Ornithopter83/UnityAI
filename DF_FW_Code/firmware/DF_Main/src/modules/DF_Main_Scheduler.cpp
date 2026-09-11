// Main scheduler and application implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_Scheduler.h"
void imuDataOutCmdSend(int onOff)
{
	String msg = String(onOff);
	//if(IMU_STOP == stop) { 	msg = STX_IMU_SET + "00000000"; }
	//else{ 					msg = STX_IMU_SET + "11111111"; }
	eNow.write(DF_Protocol_MainToRod_ImuDataControl, msg);		// Control & LOG
}

//
// PC-MAIN간 USB케이블이 빠지는 경우, 출력(모터)를 모두 OFF
void usb5vChange_Check()
{
	if(TE10_USB5V)
	{
		// MOTOR ALL OFF
		motor_AllOff();		// MAIN ALL OFF

		reelOut_AllOff();	// REEL ALL OFF

		LogPrintln(" LG] InTST usb5v TE");

	}
}
//----------------------------------------------------------------------
//
//---------------------------------------------------------------------
#define DICSIZ	    4096
#define HASHSIZ     1001U
#define MAXLEN	    18

int	DecodeLZ(unsigned char* src, unsigned char* dst, int opt)
{
	short   lst_r, i, mask, flag;
	unsigned short  pos, len, pos1, len1;
	unsigned char	Dic[DICSIZ + MAXLEN];
	unsigned int	incount, outcount, filesize, posi=0;

	lst_r = outcount = mask = 0;

	filesize = src[0] * 0x100 + src[1];
	if (opt && (filesize == opt))
		goto NoCompress;
	incount = 2;

	while (outcount < filesize) {
		if (mask == 0) {
			flag = src[incount++];
			mask = 0x80;
		}
		if (mask & flag) {
			Dic[lst_r] = src[incount++];
			dst[outcount++] = Dic[lst_r];
			if (++lst_r == DICSIZ) lst_r = 0;
		}
		else {
			pos = src[incount++];
			len = src[incount++];
			pos = ((pos << 8) + len) >> 4;
			len = (len & 0x0f) + 3;
			if (outcount >= filesize)
				goto NoCompress;
			for (i = 0; i < len; i++, outcount++) {
				if (outcount >= filesize)
					goto NoCompress;
				dst[outcount] = Dic[pos];
				Dic[lst_r] = Dic[pos];
				if (++pos == DICSIZ) pos = 0;
				if (++lst_r == DICSIZ) lst_r = 0;
			}
		}
		mask >>= 1;
	}
	return (outcount);
NoCompress:
//DbgPrintf(" DN] DecodeLZ: NoCompress(%d) opt(%d) filesize(%d) incount(%d) outcount(%d)\r\n", posi, opt, filesize, incount, outcount);
	memcpy(dst, src, filesize);
	return filesize;
}

// $DN+d(1)+xx(2)+nn(2)+data(nn)+csum(2)+%
//  d : 'M'=Main 비압축, 'm'=Main 압축, 'R'=Rod 비압축, 'r'=Rod 압축
//  xx: "ED"=End, "00"~"FF"=Seq No
//  nn: data length(hexa-big endian)
//  data: binary data(hexa)
//  csum: data check sum(hexa-big endian)

// (V1081) 다운로드 실행 함수
void Exec_Download(String msg)
{
	enum { DF_Main_Download_RodTargetUnsupported = 3 };
	int act;
	int retc = 0;
	int paraNG = 0;		// 0-OK
	int	nSeqNo;
	int DnDataLen = g_DnSaveLen[g_DnExecIdx];
	unsigned char sChr;
	String respMsg;

	if ((3 < msg.length()) && (('R' == msg[3]) || ('r' == msg[3])))
	{
		if (g_DnExecFlag || Update.hasError())
		{
			Update.end(false);
		}
		for (int i = 0; i < DN_MAX_BUF; i++)
		{
			g_DnSaveLen[i] = 0;
		}
		g_DnExecIdx = g_DnSaveIdx;
		g_DnExecFlag = 0;
		respMsg = msg.substring(0, (0 + 7)) + String(DF_Main_Download_RodTargetUnsupported) + "%";
		Resp2ApPrintln(respMsg);
		Serial.flush();
		LogPrintln(" DN] Rod target rejected: use Main-to-Rod OTA relay");
		return;
	}
	
	LogPrintf(" DN] Exec_Download() : nDataLen(%d) RecvSeq(%d) DataSum(%x)\r\n", DnDataLen, g_DnRecvSeq, g_DnDataSum);
	if (g_DnRecvSeq == 999) {					// Stop Packet
		Update.end(true);							// 시스템 OTA 세션 정상 종료
		LogPrintln(" DN] Update.end(true), Statck 여유:" + String(uxTaskGetStackHighWaterMark(NULL)));
		delay(20);
		esp_soft_reset();
		g_DnExecFlag = 0;
		return;
	}
	else if (g_DnRecvSeq == 998) {				// Stop Packet
		LogPrintln(" DN] Update.end(false)");
		Update.end(false);							// 시스템 OTA 세션 이상 종료
		for (int i=0; i < DN_MAX_BUF; i++)
			g_DnSaveLen[i] = 0;
		g_DnExecIdx = g_DnSaveIdx;
		g_DnExecFlag = 0;
		return;
	}
	if (g_DnRecvSeq == 0) {
		if (g_DnExecFlag || Update.hasError()) {
			LogPrintln(" DN] Update.end(false)");
			Update.end(false);							// 시스템 OTA 세션 정리
		}
		LogPrintln(" DN] Update.begin(UPDATE_SIZE_UNKNOWN), Statck 여유:" + String(uxTaskGetStackHighWaterMark(NULL)));
		if (Update.begin(UPDATE_SIZE_UNKNOWN)) {		// 시스템 OTA start
		//if (true) {
			g_DnExecFlag = 1;
			g_DnRecvNxt = 0;
		}
		else {
			paraNG = 1;
		}
	}
	if ((g_DnRecvNxt != g_DnRecvSeq) || g_DnDataSum) {
		paraNG = 2;
		LogPrintf(" DN] g_DnRecvNxt(%d) != g_DnRecvSeq(%d) or g_DnDataSum(%d)\r\n", g_DnRecvNxt, g_DnRecvSeq, g_DnDataSum);
	}
	else {
		g_DnRecvNxt++;
	}
	respMsg = msg.substring(0,(0+7)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);
	Serial.flush();
//	LogPrintln(" DN] ApRsp : " + respMsg);
	if (g_DnExecFlag && (g_DnDataSum == 0)) {					//
		if (msg[3] == 'm' || msg[3] == 'r') {
			DnDataLen = DecodeLZ(g_DnSaveBuf[g_DnExecIdx], g_DnExecBuf, g_DnSaveLen[g_DnExecIdx]);
		//	LogPrintf(" DN] DecodeLZ(%d) : %d->%d\r\n", g_DnRecvSeq, g_DnSaveLen[g_DnExecIdx], DnDataLen);
		}
		else {
			DnDataLen = g_DnSaveLen[g_DnExecIdx];
			memcpy(g_DnExecBuf, g_DnSaveBuf[g_DnExecIdx], DnDataLen);
		}
		g_DnSaveLen[g_DnExecIdx] = 0;
		g_DnExecIdx = (g_DnExecIdx+1) & (DN_MAX_BUF-1);
		//LogPrintln(" DN] Update.write(" + String(DnDataLen) + ")");
		if (DnDataLen) {
			Update.write(g_DnExecBuf, DnDataLen);
		}
	}
}

//---------------------------------------------------------
//	CONTROL STEP LOG CURR_STEP
//--------------------------------------------------------
void Log_Control_Step()
{
	
		// LED Step
		LogPrintln(" LG] ext_led_dim_step " + String(ext_led_dim_step));
		// bldc Mot Stop
		LogPrintln(" LG] bbnMotorStop_Step " + String(bbnMotorStop_Step));
	
		// Wave
		LogPrintln(" LG] wave_tq_step " + String(wave_tq_step));
		LogPrintln(" LG] wave_bbn_tq_step " + String(wave_bbn_tq_step));

		LogPrintln(" LG] wave_torq_bbn_step " + String(wave_torq_bbn_step));
		
	
		// Bite Step
		LogPrintln(" LG] bite_wf_step " + String(bite_wf_step));
	
		// Bite BLDC
		LogPrintln(" LG] bite_justTorq_step " + String(bite_jt_step));
		LogPrintln(" LG] bite_incTorq_step " + String(bite_it_step));
		LogPrintln(" LG] bite_bldc_step " + String(bite_bldc_step));
	
			// Resist
			LogPrintln(" LG] hitResistRdStep " + String(hitResistRdStep));
			LogPrintln(" LG] hit_resist_step " + String(hit_resist_step));
			
			// HIT
			LogPrintln(" LG] hit_BldcStep " + String(hit_BldcStep));
			LogPrintln(" LG] hit_brk_step " + String(hit_brk_step));
	
		// Fight
		LogPrintln(" LG] bbnFight_step " + String(bbnFight_step));
	
			// HOLD ON
			LogPrintln(" LG] hit_holdon_step " + String(hit_holdon_step));
			LogPrintln(" LG] holdon_bldc_step " + String(holdon_bldc_step));
			
		// Randing
		LogPrintln(" LG] randingStep " + String(randingStep));
		// Fast Hold On
		LogPrintln(" LG] fastHoldStep " + String(fastHoldStep));
		// Button Combo
		LogPrintln(" LG] btnComboStep " + String(btnComboStep));
	
	
		// Ready INC
		LogPrintln(" LG] rdy_incStep " + String(rdy_incStep)); 
		
		
		// Pwr_Off Step
	//	LogPrintln(" LG] pwr_off_step " + String(pwr_off_step));		// (V108)
				
}

//
//
//	CALL 10ms
void reelAllOff_SendControl()
{
	static int _to;

	switch(reelAlloffStep)
	{
		// IDLE
		case 0:
			break;

		// START
		case 10:
			setTO(_to);
			reelAlloffStep = 20;	// NA
			break;
		
		case 20:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				imuDataOutCmdSend(IMU_DATA_OFF);	// IMU OUT OFF
				setTO(_to);
				reelAlloffStep = 30;
			}
			break;
		
		case 30:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				sendRodVrtCmd(0,0,0,0); 	// Vrt Mot OFF
				setTO(_to);
				reelAlloffStep = 40;
			}
			break;
		
		case 40:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				sendRodBtnLedCmd(0,0,0,0,0);	// BTN LED ALL OFF
				setTO(_to);
				reelAlloffStep = 50;
			}
			break;

		case 50:
			if(checkTO(_to, SEND_NOW_STEP_TIME))
			{
				breakMotorOff();				// BREAK MOT OFF
				setTO(_to);
				reelAlloffStep = 90;	// END
			}
			break;
		
		case 90:
			// END
			reelAlloffStep = 0;			// STEP IDLE
			reelOut_AllOff_Flag = 0;	// FLAG CLR
			break;
			
		default:
			break;
	}
}


//--------------------------------------------------------------
//WIRE_HOME_BBN_CNT
//--------------------------------------------------------------

// 후킹, 랜딩시 개시
void lmLongWireCheck_Start()
{
	wireLongCheck_Flag = 1;
	currWirePulse = mainEnc.bbnCnt;	// 현재 Pulse Count저장

}

// Lm Wire Check , Hit=>버티기, Fail=>Casting대기
void lmLongWireCheck_Stop()
{
	if(wireLongCheck_Flag)
	{
		wireLongCheck_Flag = 0;
	}
}

#define LM_WIRE_ADD_TARGET	5		// 10 pulse
static volatile short targetAddPulse = LM_WIRE_ADD_TARGET;		// 10 Pulse , 10*10 mm = 10 cm
static short lmWire_Step = 0;

// Hooking & Randing중 BBN Encoder CCW/CW Check
void checkBbnCcwCheck()
{
	if(!lmWireControl_Flag)
	{
		if(BBN_MOT_CCW == mainEnc.bbnDir)	// 상태 무관
		{
			//CCW 전환후 + 10 Pulse 이동	// = 10 * 10 mm
			if( (currWirePulse + targetAddPulse) < mainEnc.bbnCnt)
			{
				targetWirePulse = mainEnc.bbnCnt;	// []Hooking 시작/Randing시작 ] + 10Pulse
				lmWireControl_Start();
			}
		}
	}
}

void lmWireControl_Start()
{
	lmWireControl_Flag = 1; 	// Control Start
	lmWire_Step = 10;
}

void lmWireControl_End()
{
	lmWire_Step = 90;
}

void lmWireControl_Stop()
{

	lmWireControl_Flag = 0;
	lmWire_Step = 0;

	wireLongCheck_Flag = 0;

	
}

#define LM_WIRE_CONTROL_TO	(15*1000)
//
//	1) Taget 갱신
//	2) 현재+10 Pulse위치 검지(CW) => 1a 출력
//	3) 1a<->1c 반복제어
//
void lmWireControl()
{
	// 최대치를 항상 갱신
  
	// CW로 전환, 1a 출력 조건 확인	
	if(19 < lmWire_Step && 40 > lmWire_Step && BBN_MOT_CW == mainEnc.bbnDir)
	{
		// CW전환후, Current + 10 Pulse이동 //
		if((currWirePulse + targetAddPulse) > mainEnc.bbnCnt)
		{
			// 모터 1a 출력
			lmWire_Step = 40;	// A출력 고정
		}
	}

	static unsigned long _endto=0;
	static unsigned long acCnt=0;
	static unsigned long _to;
	int tmVal = 0;
	int bbnVal = 0;
	switch(lmWire_Step)
	{
		case 0:
			break;

		case 10:
			acCnt=0;
			setTO(_endto);
			lmWire_Step = 20;
			break;

		// c<-a 반복
		case 20:
			// C출력
			tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].tm;
			bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].bm;
			torqMotor.on(tmVal);
			bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
			acCnt++;
			setTO(_to);
			lmWire_Step = 21;
			break;
			
		case 21:
			// C출력 대기
			if(checkTO(_to,stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_c].bmTime ))
			{
				// A출력
				tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
				bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
				torqMotor.on(tmVal);
				bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
				setTO(_to);
				lmWire_Step = 22;
			}
			break;

		case 22:
			// A출력 대기
			if(checkTO(_to,stLmDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bmTime ))
			{
				lmWire_Step = 20;	// REPEAT
			}
			break;

		//1a출력
		case 40:
			// A출력
			tmVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tm;
			bbnVal = stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].bm;
			torqMotor.on(tmVal);
			bbnMotor.onBldc(BBN_MOT_CW, bbnVal);
			setTO(_to);
			lmWire_Step = 41;
			break;

		case 41:
			if(checkTO(_to,stLmWireDutyTbl[FISH_LVL_1][MOT_PWR_LVL_a].tmTime ))
			{
				lmWire_Step = 90;	// END
			}
			if(currWirePulse > mainEnc.bbnCnt)
			{
				lmWire_Step = 90;	// END
			}
			break;

		case 90:
			if(68 > reqTorqueMotor) { tmVal = 68; }					// #1호기 UNIQ
			else					{ tmVal = reqTorqueMotor; }
			torqMotor.on(tmVal);
			bbnMotor.offBldc();
			// CLR Var & Flag
			lmWireControl_Stop();
			break;
			
		default:
			break;
	}

	//ALL TO (15 초)
	if(19 <= lmWire_Step && checkTO(_endto, LM_WIRE_CONTROL_TO))	// Start ~ 30sec
	{
		lmWire_Step = 90;	// END
	}
	
	// END처리
	
}

//-----------------------------------------
//	인터럽트에서 저장한 데이타를 LOG출력
//
//-----------------------------------------
void wat_isrLogOut()
{
	// 외부INT 테스트
	bool encFlagCheck;
	int encOutLog;
	unsigned int target;

	// INT내 변수 취득
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		encFlagCheck = bIsrEncInterruptFlag;
		if(encFlagCheck) { bIsrEncInterruptFlag = 0; }
		encOutLog = pidExecOutLog;
		if(encOutLog) { pidExecOutLog = 0; target = iTargetDist; }
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료

	if(encFlagCheck)
	{
		unsigned int isrCnt;
		unsigned int interval;
		unsigned short sEncDir;
		int intEncCnt;
		unsigned int currDist;
		double dCalVal;
		short sOutDuty;
	
		portENTER_CRITICAL(&mux);  // 임계 영역 시작
			isrCnt = iIsrEncInterruptCnt;
			interval = iEncIntrIntervalTime;
			sEncDir = sIsrBbnEncDir;
			intEncCnt = iIsrbbnEncCnt;
			currDist = iWireDistance;
			dCalVal = dIsrOutDuty;
			sOutDuty = sMotorSpeed;
	
		portEXIT_CRITICAL(&mux);   // 임계 영역 종료
	
		LogPrintln(" lg] WireC EncINT c:" + String(isrCnt)
				+ ", invt:" + String(interval)
				+ ", dir:" + String(sEncDir)
				+ ", enc:" + String(intEncCnt)
				+ ", dist:" + String(currDist)
				+ ", out:" + String(sOutDuty)
				+ ", calVal:" + String(dCalVal)
				);
	}
	
	if(encOutLog)
	{
		LogPrintln(" lg] WireC Target:" + String(target)+ ", GmSts:" + String(gmWatStatus));
	}
}

/*----------------------------------------------------------------
	주기적 처리
----------------------------------------------------------------*/
void t1ms_Process_Exec()
{

	// Input Process  
	mainEnc.rotate();	// BBN Encoder Reotation

	  
	// Device Check
	if(devReadychkFlag)   { devChkReady_Control(); }				// 장치 장애 검지
	
	if(AP_IS_TM == apType)
	{
		if(mainMotAutoSet_Flag)
		{
			//mainMot_MinOut_AutoSet_Control(); 	// 메인모터 최소출력 자동 설정
			//mainMot_MinOut_AutoSet_Control2_Time();	// 메인모터 최소출력 자동 설정
			mainMot_MinOut_AutoSet_Control3_36PulseTime();
		}
	}

	


	// 1MS STBY OFF CONTROL

}

/*------------------------------------------------------------------
   10ms EXEC Peocess
   1) casting
   2) biting
   3) Hiting
   4) Success or Fail
   5) Ending
------------------------------------------------------------------*/
void t10ms_Process_Exec()
{


	// opreation Condition Check

// AP MOT Control, by Pattern포함
	// FW자제 로직 (손맛 개선) 무시. 

	//Bite ONLY Pattern2 Control From AP
		apPatternReq_Bite_Hold();		// AP Control BITE PATTERN_2
	

	// Diag Input test (10ms)
	diagInputMonitor();

	// NA
	
	// Diag BBN Motor T/O Count
	if(diagOutflag_bbnTimeOut) { diagBbnMotor_TimeoutControl(); }
	// Diag BLDC TOKTOK Control
	if(diagBbnflag_TokTokControl) { diagBbnMotor_TokTokControl(bbnActKind); }
	// Normal BLDC STOP T/O Control
	if(exeFlag_bbnMotorStop) { bbnMotor_OffTime_Control(); }

	if(exeFlag_torqMotorStop) { torqMotor_OffTime_Control(); }

	// Diag BBN Motor T/O Count
	if(diagOutflag_lmMotTimeOut) { diagLmMotor_TimeoutControl(); }


	// Sub AC On/Off Control

	//if(pwrSafyTimeSet)	// 2SEC
	{
		check_PowerSwitch_Change_10ms();
	//	if(subAcOffSeq) { subAcOff_Control(); }					// (V108) delete
	}

	//-- External LED Control
	extLed_Control();

	// PC_Main USB Cable DisConnection Check
	usb5vChange_Check();

	// Reel now SEND step
	//if(reelOut_AllOff_Flag)	{ reelAllOff_SendControl(); }	// 10ms Interval NOW SEND to ROD

	// LM-JIG 위치 제어는 LM JIG 보드에서만 실행한다.
	if(DF_CONFIG_LMJIG == dfConfig)
	{
		if(lmHome_Flag) { lmHome_Control(); }
		else if(LM_PRIORITY_LEFT == lmPriority && lmLeft_Flag) { lmLeft_Control(); }
		else if(LM_PRIORITY_RIGHT == lmPriority && lmRight_Flag) { lmRight_Control(); }
		else if(lmLeft_Flag) { lmLeft_Control(); }
		else if(lmRight_Flag) { lmRight_Control(); }
	}

	// 낚시줄 에이징 테스트
	if(wireAgingTest_Flag)
	{
		if(AP_IS_TM == apType)	// TM의 경우만 실시.
		{
			wireAging_Test_Control(); 
		}
		else
		{
			wireAging_Test_Stop("$98010");	// Flag CLR 및 동작 정지, $98,01-WireAging,0-OFF
		}
	}

	// 10ms STBY OFF CONTROL
		// 대기중 텐션유지
		if(stbyTorqOffControl_Flag) { stbyTorqOff_Control(); }	// 메인모터 최소출력 자동 설정


}


//=== 100ms Process
//
//-------------------------------------------------
void t100ms_Process_Exec()
{
	if(exeTo1secLog)	{ setAfterTo_LogOut_Control(); }
	
	if(rodRegistMode)	{ rodRegistToControl();		}	// Regist TO Check

}

// Call 500MS
void t500ms_Process_Exec()
{
	//===== MOVE to T500ms Proc => Move to 1SEC
	//if (isGameEnable) {		// if Main Enable
	//	checkSlaveTimer();	// 500ms Slave Check Timer
	//}

	// board Led Blinking for 500ms
	boardLed_Control();
}

//
//
//
void t1sec_Process_Exec()
{
	// Power On(reset) SEND to ROD, PowerOn1st CMD
	sendCmdSts_MainPwrOn();
	
	// AP-MAIN Connection (Alive Recv) Check
	apAlive_Check();
	
	if(!rodRegistMode)		//통상, ROD등록mode시 Alive송부 금지
	{
		// Send ROD_Alive CHK
		send2RodAlive_CntCheck();		// Send Alive ROD Check
		//
		checkRodTimer();  // ROD Slave Check Timer
	}
}

static int pwrOn1stCnt = 0;
int pwrOn1stCmdSendFlag = 1;
//------------------------------------------------------------
//	SEND CMD(PWRON_1stCMD) , Only Once  MAIN=>ROD
//------------------------------------------------------------
void sendCmdSts_MainPwrOn()
{
	// PWR_1ST_CMD Send , First
	if(0 == pwrOn1stCmdSendFlag)			// After 1SEC, at PowerON 
	{
		return;
	}
	// CLR SEND FLAG
	pwrOn1stCmdSendFlag = 0;

	String respMsg = "";

	//=== 1) SEND PWR_ON MAIN=>ROD
	eNow.write(DF_Protocol_MainToRod_PowerOn, respMsg);

	//=== 2) ROD Regist Wait T/O SEC
	respMsg = STX_INFO_REQ + ROD_REGIST_TO_SEC;
	respMsg += String(ROD_REGI_CANCEL_TO/1000);
	respMsg += "%";
	Resp2ApPrintln(respMsg);
	LogPrintln(" FW] INFOR wto:" + respMsg);

	//=== 3) SEND AP TYPE
	eNow.write(DF_Protocol_MainToRod_ApInfo, "0100");	// AP TYPE = 00(DF), apType

	//===X) TBD

		
}


//----------------------------------------------------------------------
//
//---------------------------------------------------------------------
static volatile int logRcved_flag = 0;
static volatile int logRcved_Seq = 0;
static String logRcvApMsg = "";
static String logRcvApMsg_Save = "";

// LOG UART_SERIAL 1 DATA Parsing : NOT BREAK
void logUartRecvParsing()
{

	// 1) Rcv Msg Parsing
	int msglength = LogSerial.available();
	if (msglength)						// Length of Serial Buffer from PC
	{
		// NOT DELAY(BREAK) CONTROL
		for(int i=0; i<msglength; i++)
		{
			char recvCh = LogSerial.read();
			if(0) { }
			else if( 0x24 == recvCh)		// '$'
			{
				logRcved_Seq = 50;	// Rcv Start Seq
				logRcvApMsg = "$";
			}
			else if( 0x0A == recvCh)		// LF (Not SAVE)
			{
				logRcved_Seq = 53; // Recv Seq( Remove MSG)
				//LogPrint("N");
			}
			else if( 0x0D == recvCh)		// CR (Not SAVE)
			{
				logRcved_Seq = 52; // Recv Seq( Remove MSG)
				//LogPrint("R");
			}
			else if( 0x25 == recvCh)		// "%" (Not SAVE)
			{
				if(50 == logRcved_Seq)
				{
					logRcved_Seq = 51;	// Recv Seq(STOP)
					// Data Save
					logRcvApMsg_Save = logRcvApMsg;
					logRcved_flag = 1;				// SET AP MSG RCVED
					break;
				}
			}
			else			// Data Save
			{
				if(50 == logRcved_Seq)
				{
					logRcvApMsg += recvCh;
				}
			}
		}

	}

}

/*
=================================================================
	COMMAND ANAYSIS & CONTROL EXEC with RECVED  LOG SERIAL PORT
	
=================================================================
*/
void logRecvHandler()
{


	// MSG미수신시 처리하지 않음
	if (0 == logRcved_flag)					//Not Recved
	{
		return;
	}

	String msg = logRcvApMsg_Save;
	logRcvApMsg_Save = "";
	logRcved_flag = 0; // MSG ANA FINISH

	msg.trim(); 								// Delete Space in String

	// MSG가 없는 경우 처리하지 않음
	if (msg == "")			// Return, if not Recv
	{
		return;
	}

	// Recv STX String Switch
		if (0) { }	// Dummy

		else if (msg.startsWith(STX_ROD_REGIST))	// ROD REGIST
		{
			anaRodRegist(msg);
		}
		
		// 1) $11 - Mac Addr READ - ack : M + my Addr + Targ Addr+%
		else if (msg.startsWith(STX_GET_ADDR))
		{
		}
		//2) $12 - Target Addr WRITE  - ack : void
		else if (msg.startsWith(STX_SET_ADDR))
		{
		}
		// $01 - INFO_REQ
		else if (msg.startsWith(STX_INFO_REQ))
		{
			LogPrintln( "LG] InfRq IF_ver: " + IfVer);
		}
		// $03 - Servo Motor SET
		else if (msg.startsWith(STX_SERVO_MOTOR))
		{
		}
		// $02 - Torque Motor SET
		else if (msg.startsWith(STX_TORQUE_MOTOR))
		{
		}

		// $04 - Bldc Motor SET <=  $40
		else if (msg.startsWith(STX_BBN_MOTOR))
		{
		}

		// $06 - LED CONT CMD
		else if (msg.startsWith(STX_LED_CONT))
		{
			analedContCmd(msg);
		}

		// $10 <= $0A - Version Read
		else if (msg.startsWith(STX_VER_READ))
		{
			if ((ROD_CONN == rod_conn_status) && (gRodSleepStat == 0))
			{
				eNow.write(DF_Protocol_MainToRod_VersionRead, ""); 	// Send Version READ
				//String str = STX_VER_READ + mainVer + "," + eNow.getVersion() + "," + eNow.getTargetAddress() + "%";
			}
			else
			{
				String str;
				if (g_RespVersion[0] == '$')
					str = g_RespVersion;
				else
					str = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + "Vs99.99.99" + "," + "Vi99.99.99" + "%";
			
				LogPrintln(" FR] Ver: " + str);		// Log
			}
		}

		// $F0 - Wave Val SET
		else if (msg.startsWith(STX_WAVE_SET))		// $F0
		{
			Set_WaveData(msg);
		}

		// $F1 - BITE Val SET
		else if (msg.startsWith(STX_BITE_SET))
		{
			Set_BiteData(msg);
		}

		// $F2 - HIT Val SET
		else if (msg.startsWith(STX_HIT_SET))
		{
			Set_HitData(msg);
		}

		// $F3 - Break Val SET
		else if (msg.startsWith(STX_BREAK_SET))
		{
			Set_BreakData(msg);
		}

		// $F4 - BLDC TOKTOK Val SET
		else if (msg.startsWith(STX_TOKTOK_SET))
		{
			Set_BbnTokTokData(msg);
		}
		// $F5 - EXT LED SET
		else if (msg.startsWith(STX_EXTLED_SET))
		{
			Set_ExtledData(msg);
		}
		

		// $FF - Set ENB/Kind
		else if (msg.startsWith(STX_ENB_SET))
		{
			Set_EnbData(msg);
		}
		else if (msg.startsWith(STX_TORQ_ADJ_SET))
		{
			Set_TorqAdjValData(msg);
		}

		// TEST RANDING ($F6)
		else if (msg.startsWith(STX_RANDING_START))
		{
			anaRanding(msg);
		}


		else if (msg.startsWith(STX_LED_SET))
		{
			Set_LedValue(msg);
		}

		// $DI - Diag Input
		else if (msg.startsWith(STX_DIAG_INPUT))
		{
			mDiag_entry();
			diagInputTest(msg);
		}
		// $D0 - Diag Output
		else if (msg.startsWith(STX_DIAG_OUTPUT))
		{
			mDiag_entry();
		 	diagOutputTest(msg);
		}
		else if (msg.startsWith(STX_DIAG_EXIT))
		{
			mDiag_exit();
		}

		// Data Set/Read ($99)
		else if (msg.startsWith(STX_DATA_PARA_SET))
		{
			ana_SetDataSave(msg);
		}

		// Test Execute ($TE)
		else if (msg.startsWith(STX_TEST_EXEC))			// (V108) TEST
		{
			ana_TestExecution(msg);
		}
		//	else Slave Connection ( Send Other STX to Slave )
		// $05(Break) or $08(IMU Pass to SLAVE)
		else if (ROD_CONN == rod_conn_status)
		{
			// Send to ROD
		}
	  
}


//-------------------------------------------------------------
#define	EX_STX_LED_OFF	"$10"
#define	EX_STX_LED_ON	"$11"
#define	EX_STX_OTHER	"$12"

//====================================================
//
//
//====================================================
void extIOuartRecvHandler()
{
	
}


static short imuIntervalCheckCnt =0;
static unsigned long measureCnt=0;
static unsigned long imuIntervalTime=0;

#define IMU_MEASURE_CNT	5
//-----------------------------------------------
//	IMU Interval 간격 측정
//		-- Only TM
//-----------------------------------------------
void imuInterval_Measure()
{
  	if(imuIntervalCheckCnt)
  	{
  		imuIntervalCheckCnt++;
		if(0) {}
  		else if(2 == imuIntervalCheckCnt) { measureCnt = curr_ms_tick;	}// 1st
  		else if(3 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 2nd
  		else if(4 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 3rd
  		else if(5 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 4th
  		else if(6 == imuIntervalCheckCnt) { imuIntervalTime += (int)(curr_ms_tick - measureCnt); measureCnt = curr_ms_tick;	}// 5th
		else
		{
			imuIntervalTime += (int)(curr_ms_tick - measureCnt);		// 5th
			imuIntervalTime = imuIntervalTime/IMU_MEASURE_CNT;
			// Send to PC
			strImuInterval = fillZero4Char((unsigned int)imuIntervalTime);
			Resp2ApPrintln(STX_INFO_REQ+ IMU_INTVAL_TIME + strImuInterval +"%");	// ms
			
			imuIntervalCheckCnt = 0;	// 측정 종료
			imuIntervalTime = 0;
			measureCnt = 0;
		}
  	}

}

//---- Delay Check
static unsigned short imuRecv1stTime = 0;

static String rodVer = "Vr99.99.99";


//-------------------------------------------------------------------
//
int handEncCnt = 0;
static int oldhandEncCnt = -99;

/*  =====================================
       From SLAVE, Check Unlimit Loop, For (ESP Now) Recv Data Hanlder From Slave
  ===================================== */
void nowRecvHandler()
{
	enum { DF_Main_Scheduler_ReceiveBufferSize = 128 };
	char receiveBuffer[DF_Main_Scheduler_ReceiveBufferSize];
	uint8_t sourceAddress[6];
	unsigned int receiveLength = 0;
	unsigned long overwrittenCount = 0;
	unsigned long invalidCount = 0;
	int now_rcv_id;

	DF_Main_Communication_ProcessSendResult();
	if (!DF_Main_Communication_TakeReceived(receiveBuffer, sizeof(receiveBuffer), &receiveLength, sourceAddress, &overwrittenCount, &invalidCount))
	{
		return;
	}
	if (DF_Main_RodOtaRelay_HandleRodFrame((const unsigned char *)receiveBuffer, receiveLength, sourceAddress))
	{
		return;
	}
	String now_msg_str(receiveBuffer);
	now_rcv_id = now_msg_str.substring(0, 2).toInt();
	if (0 == now_rcv_id)
	{
		return;
	}
	if (overwrittenCount || invalidCount)
	{
		LogPrintln(" LG] nowRcv drop:" + String(overwrittenCount) + ",invalid:" + String(invalidCount));
	}
	(void)receiveLength;
	String respMsg;
	//String sendBtnMsg = "99";
	//String logMsg = " FS] BTNNG 99";

//====== ROAD RESP/STS ANALISIS===
	int notDefineCmdFlag = 0;
	String now_cmd_data = now_msg_str.substring(2); // Msg_Data저장, ID(2 char) 제외

	if (now_rcv_id != DF_Protocol_Pid_RodToMain_Alive) {
		if (now_msg_str.length() > 18) {
			LogPrintln(" RD] MSG:  " + now_msg_str.substring(0, 18) + "#");	  // LOG-SERIAL1
		}
		else {
			LogPrintln(" RD] MSG:  " + now_msg_str + "%");	  // LOG-SERIAL1
		}
	}
	
// *1. ANA CMD : Switch Recv Pid
	//1) Recv Slave Control ( 11 )
	if(0) { }	// Dummy

	// ROD교체 ADDR등록 진입(L버튼 3초이상 누름)
	else if (DF_Protocol_Pid_RodToMain_RodAddress == now_rcv_id)	// ROD -> MAIN (BroadCast Addr)
	{
		memcpy(rcv_src_addr_back, sourceAddress, 6);
		// Addr Mode인 경우만 처리
		//String recvRodAddr = now_cmd_data.substring(0,(0+17));  // NG
		LogPrintln(" LG] RodRg BC RcvRodaddr:" + now_cmd_data);
		if(1 == rodRegistMode)		// REGIST 진입상태 ( 개시이후는 처리 안함)
		{
			rodRegistExec(now_cmd_data);
		}
  	}


	// Rod Button
  else if (DF_Protocol_Pid_RodToMain_Button == now_rcv_id && (!rodRegistMode))	// Button & Encoder
  {
  		respMsg = RESP_STX_BUTTON + now_cmd_data + "%";		// PASS
		StsSendPrintln(respMsg);	  // NEW IF
		
  }

	// Reel ECN COUNTER
  else if (DF_Protocol_Pid_RodToMain_Encoder == now_rcv_id && (!rodRegistMode))	// Button & Encoder
  {
  		respMsg = RESP_STX_ENCODER + now_cmd_data + "%";
		StsSendPrintln(respMsg);	  // NEW IF

  }


  // 2) IMU EULER Data( 12 )

 	// ( 14 ) IMU Data RECV
  else if (now_rcv_id == DF_Protocol_Pid_RodToMain_ImuData && (!rodRegistMode))
  {
    //if (eulerStr.length() != 0)	// EULER Data exist
    {
    	// send tp PC ($09 + EulerData + ',' + message String + %)
      //String imuStr = STX_IMU_DATA_STS + eulerStr + "," + now_msg_str + "%";
      
      String imuStr = STX_IMU_DATA_STS + now_cmd_data + "%";
      //String imuStr = STX_IMU_DATA_STS + "#__" + now_msg_str + "__#";	// TEST
      ApPrintln(imuStr);		// Send to PC


	  //== IMU Interval 간격 측정
	  if(AP_IS_TM == apType && imuIntervalCheckCnt)
	  {
		imuInterval_Measure();
	  }
	  //-------------------------------------------------------
    }
  }
  // 4) Recv ROD_ALIVE( 10 )
  else if (now_rcv_id == DF_Protocol_Pid_RodToMain_Alive && (!rodRegistMode))
  {
		//slave_resp_over_cnt = 0;			// CLR OverCNT at RECVED
		//battLevelSet(now_msg_str);
		setRod_Connected();
  }


	// (22) 버전 RESP
	else if (DF_Protocol_Pid_RodToMain_Version == now_rcv_id)	//(20)
	{
	//	String respVer = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + now_cmd_data + "%";	// Main,Rod,Imu
		g_RespVersion = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + now_cmd_data + "%";	// Main,Rod,Imu
		Resp2ApPrintln(g_RespVersion);
		
		LogPrintln(" FM] Ver__ " + g_RespVersion);	  // Log
	}


	// 배터리 잔량 수신.
  else if (DF_Protocol_Pid_RodToMain_Battery == now_rcv_id && (!rodRegistMode))	//BAT LEVEL
  {
		// Batt Level Set [ INT / STRING ]
		battLevelSet(now_cmd_data);
		
		//slave_resp_over_cnt = 0;			// CLR Over CNT at RECVED
		respMsg = STX_BAT_LVL + battLvlStr + "%";	// 2 char
		StsSendPrintln(respMsg);	// pass to PC

		// 배터리 잔량 경고( 7020: 15%미만) 통지
		if(BAT_LVL_LOW > ibatteryLvl)
		{
			respMsg = STX_DEV_CONT_REQ + ROD_BAT_LOW_WARN;
			respMsg += "%";
			StsSendPrintln(respMsg);	// pass to PC	
		}

  }

  // 6) IMU Connection(PID "21")
  else if (DF_Protocol_Pid_RodToMain_ImuConnection == now_rcv_id)	// IMU CONN
  {
	// TBD : Edit ( OLD => NEW)
	int recvIMUconn = now_cmd_data.substring(0).toInt();
	if(IMU_CONN_2CHAR_INT == recvIMUconn)	// "11"
	{
		imu_conn_status = CONNECT;		// IMU CONN ( 0-CONN, 1-DISCONN, -1 : UNKNOWN)
	}
	else
	{
		imu_conn_status = DISCONNECT;		// IMU CONN ( 0-CONN, 1-DISCONN, -1 : UNKNOWN)
	}

		respMsg = STX_IMU_CONN;
		if(CONNECT == imu_conn_status)	{ respMsg += STATE_CONNECTED; }
		else							{ respMsg += STATE_DISCONN; }
		respMsg += "%";
		StsSendPrintln(respMsg);	// bypass to PC

	LogPrintln(" FM] IMUCN " + respMsg + ", " + String(imu_conn_status) );	// Log
  }

  else if (DF_Protocol_Pid_RodToMain_BoardType == now_rcv_id) // Board TYPE
  {
  	// 1) ROD보드 Type SAVE
  	strRodBoardType = now_cmd_data;	// 3 Char (OLD-000, NEW-001)

	rodBoardType = strRodBoardType.substring(0).toInt();

	// 2) SEND to AP , ROD보드 TYPE
	respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;		// Board TYPE
  	respMsg += strRodBoardType;		// 3 Char (OLD-000, NEW-001)
	respMsg += "%";
	StsSendPrintln(respMsg);	  // SEND ROD BOARD TYPE to AP
	
  	LogPrintln(" FS] INFOR " + respMsg + ",type:" + String(rodBoardType) );  // Log
  }
  else if (DF_Protocol_Pid_RodToMain_Sleep == now_rcv_id) // Rod wakeup
  {
	int stm;
	// Batt Level Set [ INT / STRING ]
	battLevelSet(now_cmd_data);
	
	//slave_resp_over_cnt = 0;			// CLR Over CNT at RECVED
	respMsg = STX_BAT_LVL + battLvlStr + "%";	// 2 char
	StsSendPrintln(respMsg);	// pass to PC

	if (now_cmd_data[3] == 'S') {
		stm = now_cmd_data.substring(4,4+4).toInt();
 	 	LogPrintf(" LG] SLEEP[r]: gRodSleepMode(%d) gRodSleepTime(%d->%d) gRodSleepStat(%d->1)\r\n", gRodSleepMode, gRodSleepTime, stm, gRodSleepStat);  // Log
		gRodSleepTime = stm;
		gRodSleepStat = 1;
	}
	else {
		if (gRodSleepMode) {
			if (gRodSleepMode == SS_EXIT)
				stm = TIME_SLEEP_DEEP;
			else
				stm = TIME_SLEEP_LIGHT;
		}
		else {
			stm = 0;
		}
 	 	LogPrintf(" LG] SLEEP[R]: gRodSleepMode(%d) gRodSleepTime(%d->%d) gRodSleepStat(%d->0)\r\n", gRodSleepMode, gRodSleepTime, stm, gRodSleepStat);  // Log
		gRodSleepStat = 0;
		sendSleepEnable(gRodSleepMode, stm, 3);
	}
  }


	// NOT DEFINE CMD
  else
  {
  	notDefineCmdFlag = 1;
  }

	// * 2. Check Not Define & Error SET & CLR
  if(notDefineCmdFlag)
  {
  		notDefineCmdFlag = 0;
		//Error
  }
  else				// Valid SLAVE STS
  {
	  //check_slave_dis_conn_cnt = 0; 			  // Check Cnt CLR( Check Restart )
	  // TEST, Alive NG
	  rodAlive_SendCnt = 0;
  }

  
}

volatile int rcved_flag = 0;
int rcved_Seq = 0;
String rcvApMsg = "";
String rcvApMsg_Save = "";

// AP_SERIAL DATA Parsing : NOT BREAK
void uartRecvParsing()
{


	// 1) Rcv Msg Parsing
	int msglength = Serial.available();
	if (msglength > 0)						// Length of Serial Buffer from PC
	{
	//	Serial1.printf("%10d",curr_ms_tick);Serial1.printf(" DN] Serial.available() = %d\r\n", msglength);
		// NOT DELAY(BREAK) CONTROL
		for(int i=0; i<msglength; i++)
		{
			if(g_DnExecStep)
			{
				int idx;
				unsigned short cSum;
				unsigned char rChr;
				if (g_DnExecStep < 100) {
					rChr = (unsigned char)Serial.read();
				//	g_readBuf[g_SavePtr] = rChr;
				//	g_SavePtr = (g_SavePtr + 1) & (MAX_SAVE_LEN-1);
				}
				switch (g_DnExecStep) {
				case 1 :
				case 2 :
				case 3 :
				case 4 :
					rcvApMsg += (char)rChr;
					g_DnExecStep++;
					break;
				case 5 :
					g_DnRecvSeq = rcvApMsg.substring(4).toInt();
				//	if (rcvApMsg[3] == 'm' || rcvApMsg[3] == 'r') {
					g_DnRecvBuf = g_DnSaveBuf[g_DnSaveIdx];				// 
					g_DnRecvLen = rChr << 8;
					g_DnExecStep = 6;
					break;
				case 6 :
					g_DnRecvLen += rChr;
					g_DnRecvIdx = 0;
					g_DnCalcSum = 0;
					g_DnExecStep = 100;
				//	DbgPrintf(" DN] g_DnRecvSeq(%d) DnRecvLen(0x%x, %d)\r\n", g_DnRecvSeq, g_DnRecvLen, g_DnRecvLen);
					break;
				case 7 :
					g_DnDataSum = rChr << 8;
					g_DnExecStep = 8;
					break;
				case 8 :
					g_DnDataSum += rChr;
					if (g_DnDataSum == g_DnCalcSum) {
						g_DnDataSum = 0;
						g_DnSaveLen[g_DnSaveIdx] = g_DnRecvLen;
						g_DnSaveIdx = (g_DnSaveIdx+1) & (DN_MAX_BUF-1);
					}
					else {
						LogPrintf(" DN] DataSum(%x), DnCalcSum(%x)\r\n", g_DnDataSum, g_DnCalcSum);
						g_DnDataSum = 1;
					}
					g_DnExecStep = 0;
					break;
				case 100 :
					if (g_DnRecvIdx < g_DnRecvLen) {
						int rbytes = min(g_DnRecvLen-g_DnRecvIdx, msglength-i);
						Serial.readBytes(&g_DnRecvBuf[g_DnRecvIdx], rbytes);
					//	rbytes = ApReadBytes(&g_DnRecvBuf[g_DnRecvIdx], rbytes);
						for (int jdx=0; jdx < rbytes; jdx++)
							g_DnCalcSum += g_DnRecvBuf[g_DnRecvIdx++];
						i += rbytes - 1;
					}
					if (g_DnRecvIdx >= g_DnRecvLen)
						g_DnExecStep = 7;
					break;
				}
			}
			else
			{
				char recvCh = Serial.read();
				if( 0x24 == recvCh)		// '$'
				{
					rcved_Seq = 50;	// Rcv Start Seq
					rcvApMsg = "$";
				}
				else if( 0x0A == recvCh)		// LF (Not SAVE)
				{
					rcved_Seq = 53; // Recv Seq( Remove MSG)
					//LogPrint("N");
				}
				else if( 0x0D == recvCh)		// CR (Not SAVE)
				{
					rcved_Seq = 52; // Recv Seq( Remove MSG)
					//LogPrint("R");
				}
				else if( 0x25 == recvCh)		// "%" (Not SAVE)
				{
					if(50 == rcved_Seq)
					{
						rcved_Seq = 51;	// Recv Seq(STOP)
						// Data Save
						if (rcved_flag == 0) {
							rcvApMsg_Save = rcvApMsg;
							rcved_flag = 1;				// SET AP MSG RCVED
						}
						break;
					}
				}
				else			// Data Save
				{
					if(50 == rcved_Seq)
					{
						rcvApMsg += recvCh;
						if ((rcvApMsg.length() == 3) && (rcvApMsg == STX_DNLD_EXEC)) {
							g_DnExecStep = 1;
						}
					}
				}
			}
		}

	}
}

	
/*  =====================================
       From PC, (Uart) Recv Data Hanlder From PC
  ===================================== */
void uartRecvHandler()
{
/*
	if(!Serial)
	{
		return;
	}
*/

	if (0 == rcved_flag) 	// Length of Serial Buffer from PC
	{
		return;
  	}	// 처리안함

	// 수신때만 처리
	String msg = rcvApMsg_Save;
	rcvApMsg_Save = "";
	rcved_flag = 0; // MSG ANA FINISH

	msg.trim();				// Delete Space in String

	// msg있을때만 처리    
    if (msg == "")			// Return, if not Recv
    {
    	return;
    }
//	LogPrintln(" AP] ApCmd Recved len:" + String(g_SavePtr));
//	HexDump(0, g_readBuf, g_SavePtr);
//	g_SavePtr = 0;
//======================================================================
	// LOG OUT : AP CMD
	if(!(msg.startsWith(STX_SERVO_MOTOR)) && !(msg.startsWith(STX_MAINALIVE)))
	{
		if (msg.length() > 10) {
			LogPrintln(" AP] ApCmd " + msg.substring(0, 10) + "%");	  // LOG-SERIAL1
		}
		else {
			LogPrintln(" AP] ApCmd " + msg + "%");	  // LOG-SERIAL1
		}
	}
//========================================================================
	setTO(g_LastCmdTick);						// (V108)
	g_RecvCmdCount++;							// (V108)

	// ANA AP CMD & EXEC
	// Recv STX String Switch
	if(0) { /* Dummy */ }
	else if (DF_Main_RodOtaRelay_HandlePcCommand(msg))
	{
		return;
	}

	//($30) Rod Regist
    else if (msg.startsWith(STX_ROD_REGIST))	// ROD REGIST
	{
		anaRodRegist(msg);
    }

	// $00 - PC & MAIN Connection Check
    else if (msg.startsWith(STX_MAINALIVE))	// MAIN ALIVE
	{
		anaMainAlive(msg);
    }

	// $29 - AP INFO SEND
    else if (msg.startsWith(STX_AP_INFO_SEND))	// AP SET
	{
		anaApInfo(msg);
    }
	
	// $27 - TM INPUT TEST REQ
    else if (msg.startsWith(STX_TM_INPUT_REQ))	// INPUT TEST
	{
		if(AP_IS_TM == apType)
		{
			anaTmInputTestReq(msg);
		}
    }

	// $28 - TM OUTPUT TEST REQ
    else if (msg.startsWith(STX_TM_OUTPUT_REQ))	// OUTPUT TEST
	{
		if(AP_IS_TM == apType)
		{
			anaTmOutputTestReq(msg);
		}
    }
	

	// ($01) DEVICE INFO
	else if (msg.startsWith(STX_INFO_REQ))
	{
		anaDeviceInfoReq(msg);
	}
	
	
	// 1) $11 - Mac Addr READ - ack : M + my Addr + Targ Addr+%
    else if (msg.startsWith(STX_GET_ADDR))
	{
      String str = STX_GET_ADDR + "M" + eNow.getMyAddress() + "," + eNow.getTargetAddress() + "%";
      Resp2ApPrintln(str);
    }

	//2) $12 - Target Addr WRITE  - ack : void
    else if (msg.startsWith(STX_SET_ADDR))
	{
      msg = msg.substring(3, msg.length());		// Catch 3rd ~ end String
	  int len = msg.length();
      if((DF_Protocol_MacAddressTextLength - 1) < len)
      {
      	// TBD , Return at FAIL
      	eNow.setTargetAddress(msg);
	  		LogPrintln(" LG] T_ADDR Len:" + String(len) + ",msg:" + msg);
      }
	  else
	  {
	  	LogPrintln(" LG] ERROR T_ADR Len:" + String(len) + ",msg:" + msg);
	  }
    }

	// $02 - Main ( Torque Motor ) SET
    else if (msg.startsWith(STX_TORQUE_MOTOR))
    {
 		ana_MainMotControl(msg);
	}

	// $03 - Angle (Servo Motor) SET
    else if (msg.startsWith(STX_SERVO_MOTOR))
    {
		ana_AngleControl(msg);
    }

	// $04 - BOBBIN(BLDC) CONTROL by AP
    else if (msg.startsWith(STX_BBN_MOTOR))
    {
		//ana_BobbinControl(msg);
		ana_BobbinControl_Check(msg);
    }

	// $07 - LM JIG motor control. Reserved on a normal MAIN board.
	else if((DF_CONFIG_LMJIG == dfConfig) && msg.startsWith(STX_RESERVE07))
	{
		ana_LineMotControl_Check(msg);
	}

	// $06 - LED CONT CMD
	else if (msg.startsWith(STX_LED_CONT))
	{
		analedContCmd(msg);
	}

	// $0A - Version Read
    else if (msg.startsWith(STX_VER_READ))
    {
    
      ApFgVer = msg.substring(3);

		if ((ROD_CONN == rod_conn_status) && (gRodSleepStat == 0))
		{
			eNow.write(DF_Protocol_MainToRod_VersionRead, "");		// Send Version READ to SLAVE
			// Responce is Sended When Recived Slave Version
		}
		else
		{
			String str;
			if (g_RespVersion[0] == '$')
				str = g_RespVersion;
			else
				str = STX_VER_READ + mainVer + (g_AutoReboot ? "-R" : "") + "," + "Vs99.99.99" + "," + "Vi99.99.99" + "%";
			Resp2ApPrintln(str);
			LogPrintln(" FM] Ver__ " + str);		// LOG-SERIAL1
		}
	}

	// $11 - Game Status
	else if (msg.startsWith(STX_GAME_STS))
    {
		anaGameStatus(msg);
	}

	// $25 - Game Status WTA
	else if (msg.startsWith(STX_GAME_STS_WAT))
    {
		anaGameStatusWAT(msg);
	}


	// [장치장애검지] 요구
	else if (msg.startsWith(STX_DEV_CONT_REQ))
    {
		anaDeviceContReq(msg);
		
	}
	
	// Recved RESP POWER OFF EXEC(by AP)
	
	else if (msg.startsWith(STX_APSTS_RESP))
	//else if (msg.startsWith("$17"))
	{
		int i = -1; 
		// TBD  toInt  is return " 0 "	- Need IF Edit
		i = msg.substring(3).toInt();
		if(0 == i)		// PC OFF
		{ 
			recvPwrOffExec_Resp = 1;
			//recvPwrOffExec_Resp = 1;
	  		LogPrintln(" LG] PWR_C RecvPowerOffExec from AP opt= " + String(i));
		}
	}

	// MKT_TEST_2
	else if (msg.startsWith(STX_MOT_OUT_RATE_SET))
	{
		anaMotOutRateSet(msg);
	}

//==== Diag Input & Output
	// $DI - Diag Input
	else if (msg.startsWith(STX_DIAG_INPUT))
	{
		Resp2ApPrintln(msg);
		mDiag_entry();
		diagInputTest(msg);
	}
	// $D0 - Diag Output
	else if (msg.startsWith(STX_DIAG_OUTPUT))
	{
		Resp2ApPrintln(msg);
		mDiag_entry();
		diagOutputTest(msg);
	}
	else if (msg.startsWith(STX_DIAG_EXIT))
	{
		mDiag_exit();
	}


//====else Slave Connection ( Send Other STX to Slave ),  $05(Break) or $08(IMU Pass to SLAVE)
    //else if (isSlaveConn)
    //else if (ROD_CONN == rod_conn_status)
    
    //if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
    //{

	//test Beake Change
	else if (msg.startsWith(STX_BREAK_MOTOR))
	{
		// ROD연결 & 낚시대등록 아닌경우만 전송
		if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
		{
			// ROD보드Type(브레이크 유무)에 따라, 없는 경우는 전송하지 않음
	  		if(ROD_BOARD_V1 == rodBoardType)	// OLD BOARD Only
	  		{
		 		reqBreakMotor = msg.substring(3).toInt();	//
				String sendmsg = String(reqBreakMotor);		// For AP(3 char) & SETUP (8 char)
				//msg = STX_BREAK_MOTOR + "00000" + msg.substring(3);
				eNow.write(DF_Protocol_MainToRod_Break, sendmsg);
	  		}
		}
	}

	// ($08) IMU DATA/ IMU SET
	else if (msg.startsWith(STX_IMU_SET))
	{
	//FW자체 Condition 확인을 위해 분석 추가
		// ROD연결 & 낚시대등록 아닌경우만 전송
		if (ROD_CONN == rod_conn_status  && (!rodRegistMode))
		{	
			// IMPROVE TBD
			int act = msg.substring(3,(3+1)).toInt();		//Test
			if( (IMU_DATA_OFF == act) || (IMU_DATA_ON  == act) )
			{
				String sendmsg = msg.substring(3,(3+1));
				eNow.write(DF_Protocol_MainToRod_ImuDataControl, sendmsg);		// Control & LOG
				reqImuMeasFlag = act;

				// Send to ROD(OK) or to AP(NG)
					String respStr = msg + "%";
					Resp2ApPrintln(respStr);	// 받은값 그대로 RETURN

				// IMU데이타 간격 측정
				if(AP_IS_TM == apType)	// Only TM
				{
					if(IMU_DATA_ON  == act) { imuIntervalCheckCnt = 1; }		// Interval측정 Start
					else					{ imuIntervalCheckCnt = 0; }		// Interval측정 Stop
				}
			}
			//else if(3==act)	{ msg = STX_IMU_SET + "10101010"; }
			//else if(4==act)	{ msg = STX_IMU_SET + "10100000"; }
			//else if(20000 < act) { msg = STX_IMU_SET + "0000" + String(reqImuMeasFlag-20000); }
			else	// Error
			{
					String respStr = msg + "_ER%";
					Resp2ApPrintln(respStr);
			}
		
		}
	}


	// VRT MOT CONT(ON/OFF)
	else if (msg.startsWith(STX_VRT_MOT_CONT))
	{
		String logMsg = "";
		if(ROD_BOARD_V2 == rodBoardType)
		{
			logMsg = anaVrtMotCmd(msg);
		}
		Resp2ApPrintln(msg + "%");
		
	}

	// ($24) 버튼 LED
	else if (msg.startsWith(STX_BTN_LED_CONT))
	{
		String logMsg = "";
		if(ROD_BOARD_V2 == rodBoardType)
		{
			logMsg = anaBtnLedCmd(msg);
		}
		Resp2ApPrintln(msg + "%");
	  
	}

	// STX_SETUP_REQ
	else if(msg.startsWith(STX_DEV_SET_REQ))	// 설정 요구
	{
		String	rtsMsg = "";
		short errK = ERR_OK;
		
		rtsMsg = anaDevSetReq(msg);
		if(0 > rtsMsg.toInt())	// -1:실패, -2:범위오버, -3: 파라NG
		{
			// NG, 곧바로 응답
			errK = ERR_NG;
			Resp2ApPrintln(msg + rtsMsg + "%");
		}
		else
		{
			// OK, 실행후 응답
			//Resp2ApPrintln(msg + "00" + "%");
		}
	
	}

	// DEV_FUNCTION_REQ($98)
	else if (msg.startsWith(STX_DEV_FUNCTION_REQ))
	{
		String	rtsMsg = STR_ERR_OK;
		short errK = ERR_OK;
		
		rtsMsg = anaDevFunctionReq(msg);
		//바로 응답
		Resp2ApPrintln(msg.substring(0,(0+6)) + rtsMsg + "%");	// OK , NG

	}

	
	// LM DATA SAVE
	else if (msg.startsWith(STX_DATA_PARA_SET))
	{
		ana_SetDataSave(msg);
	}

	// Download Main B/D ($DN)
	else if (msg.startsWith(STX_DNLD_EXEC))			// (V1081) 
	{
		Exec_Download(msg);
	}

	else if (msg.startsWith(STX_TEST_EXEC))			// (V108) 엔코더 값 콘솔에 출력
	{
		ana_TestExecution(msg);
	}
	
//==== Not Define AP_CMD, in ROD NOT_CONNECTED
	else
	{
  		// NOT DEFINE & SLAVE NOT CONN
	}

}

//
//
//	$23, AccNNNFFF%
String anaVrtMotCmd(String msg)
{
	String logMsg = " ok";

	int act = msg.substring(3,(3+1)).toInt();
	if(5 < act)
	{
		return " Ng act";;
	}

	switch(act)
	{
			case 0: // off
			case 1: // on
			case 2: // cnt
			case 3: // time
			case 4: // c+ time
			case 5: // Pattern, TBD
				//sendRodVrtCmd(act, cnt, ontime, offtime);
				eNow.write(DF_Protocol_MainToRod_VerticalMotor, msg.substring(3));		// Control & LOG
				break;

			default:
				break;
				
	}
	
	return logMsg;
}

//
// $99kk	// kk-구분
//	kk=xy (11~33)
//		: xydt(3)	x =1,2,3  /  y=1,2,3(a,b,c)  / dt (3개 SET lm,bm,tm)
//
//	kk = 40 : Bbn Enc 인터벌 Buff LOG출력
//	
void ana_SetDataSave(String msg)
{
	int act;
	int kind;
	int iPara;
	double dPara;
	int paraNG = 0;		// 0-OK
	String respMsg;
	String logMsg;

	int fish,pwr,d1,t1,d2,t2,d3,t3;
	
	act = msg.substring(3,(3+2)).toInt();	// 1
	switch(act)
	{
		case 0:		// TBD
			paraNG=1;
			break;
			

		case 40:		// Enc INV LOG OUT
			// 조건문
			for(int i=0; i < ENC_INTV_BUFF_SIZE; i++)
			{
				LogPrintln(" lg] EnInt Interval:" +String(i)
					+ ","+ String(stEncIntvBuff[i].tick)
					+ ","+ String(stEncIntvBuff[i].width)
					+ ","+ String(stEncIntvBuff[i].dir)
					+ ","+ String(stEncIntvBuff[i].enc)
					+ ","+ String(stEncIntvBuff[i].dist)
					);
			}

			break;

		case 41:		// PID ParaSet
			wat_PidParaSet(msg);
			break;
			
		default:
			paraNG = 1;
			break;
	}
	
	// Response
	respMsg = msg.substring(0,(0+5)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);

}

//
// ROD BTN LED Control
//	$24,XA ccNNNFFF
String anaBtnLedCmd(String msg)
{
		String logMsg = " ok";
		//if(ROD_BOARD_V2 == rodBoardType)
		{
			int act = msg.substring(4,(4+1)).toInt();		// 2nd CHAR, ACT
			if(5 < act)
			{
				return " Ng act";;
			}
			
			int posi = msg.substring(3,(3+1)).toInt();		// 1st CHAR , LED POSITION 1=LEFT, 2=RIGHT
			if(BTN_LED_ALL_POSI == posi && (0==act||1==act))	// ALL OFF,ON
			{
				// NA, PASS to ROD
			}
			else if(BTN_LED_LAST_POSI < posi)		// MAX이상이면 처리 안함
			{
				return " Ng posi";
			}
			
			switch(act)
			{
				case 0:	// off
				case 1:	// on
				case 2: // cnt
				case 3: // time
				case 4: // cnt+time
				case 5:	// pattern
					eNow.write(DF_Protocol_MainToRod_ButtonLed, msg.substring(3));		// Control & LOG
					break;
		  
				default:
					break;
			}
		}
	return logMsg;	
}


#define STR_RTS_OK			"00"
#define STR_RTS_NG  		"-1"
#define STR_RTS_RANGE_OVER	"-2"
#define STR_RTS_PARANG  	"-3"

//-----------------------------------------------
// 장치 설정 수행 요구
//-----------------------------------------------
String anaDevSetReq(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iKind = msg.substring(3,(3+2)).toInt();		// 2nd CHAR, kk
	switch(iKind)
	{
		case 0:
			rts = STR_RTS_PARANG;
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			if(AP_IS_TM == apType)
			{
				rts = anaDevSet_MainMotor_AutoSet(msg);
			}
			else
			{
				rts = STR_ERR_FAIL;
			}			
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}

// 메인모터 최소출력값 자동설정
String anaDevSet_MainMotor_AutoSet(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iAct = msg.substring(5,(5+1)).toInt();		// 1 CHAR, ACT
	switch(iAct)
	{
		case 0:
			mainMot_MinOut_AutoSet_Stop(msg);	// void
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			mainMot_MinOut_AutoSet_Start(msg);
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


//-----------------------------------------------
// 장치 기능 수행 요구
//-----------------------------------------------
String anaDevFunctionReq(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iKind = msg.substring(3,(3+2)).toInt();		// 2nd CHAR, kk
	switch(iKind)
	{
		case 0:
			rts = STR_RTS_PARANG;
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			if(AP_IS_TM == apType)
			{
				rts = anaDevFunc_wireAging(msg);
			}
			else
			{
				rts = STR_ERR_FAIL;
			}
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


// 에이징 테스트 , 낚시줄
String anaDevFunc_wireAging(String msg)
{
	String rts = STR_RTS_OK;	// OK
	
	int iAct = msg.substring(5,(5+1)).toInt();		// 1 CHAR, ACT
	switch(iAct)
	{
		case 0:
			wireAging_Test_Stop(msg);
			break;

		case 1:
			//strMainMot_AutoSet_Result =STR_ RTS_OK;
			wireAging_Test_Start(msg);
			break;

		default:
			rts = STR_RTS_PARANG;
			break;
	}

	return rts;
}


//---------------------------------------------------------------------
// Start
void mainMot_MinOut_AutoSet_Start(String msg)
{
	mainMotAutoSet_Flag = 1;
	mainMotAutoSet_Step = 10;
}

void mainMot_MinOut_AutoSet_Stop(String msg)
{
	mainMotAutoSet_Flag = 0;
	mainMotAutoSet_Step = 0;
	torqMotor_DefaultOut();		// 토크 DEFAULT출력
	bbnMotor.offBldc();			// BLDC OFF
}

// 토크모터 초기값 출력 혹은 AP요구값 출력
void torqMotor_DefaultOut()
{
	if(recv_TorqMotor_Flag)
	{
		torqMotor.setValue(reqTorqueMotor);
	}
	else
	{
		torqMotor.setValue(defaultTorqueMotor);
	}
}

//-------------------------------------------------------------
#define MAINMOT_MIN_RETRY_CNT	(1)	// Retry 5회(총 5회)


// 1ms => 100us


#define MAINENCA_SPEC_PERIOD_SP_HIGH	102 // (100 + MAINENCA_SPEC_ADD)		// 10.0 ms
#define MAINENCA_SPEC_PERIOD_SP_MID		112 // (110 + MAINENCA_SPEC_ADD)		//11.0 ms
#define MAINENCA_SPEC_PERIOD_SP_LOW		122 // (120 + MAINENCA_SPEC_ADD)		//12.0 ms


// 보빈 회전 카운트
#define BBN_REV_1TIME_SIGCHG_CNT	(18*2*1)
#define BBN_REV_2TIME_SIGCHG_CNT	(18*2*2)
#define BBN_REV_3TIME_SIGCHG_CNT	(18*2*3)
#define BBN_REV_4TIME_SIGCHG_CNT	(18*2*4)

#define BBN_REV_READY_SIGCHG_CNT	(70*2*1)
#define BBN_REV_TIMEOUT_SIGCHG_CNT	(80*2*1)

#define MAINMOT_MIN_AUTOSET_TO_1S		1000 //
#define MAINMOT_MIN_AUTOSET_TO_2S		2000 //
#define MAINMOT_MIN_AUTOSET_TO_2p5S		2500 //
#define MAINMOT_MIN_AUTOSET_TO_3S		3000 //
#define MAINMOT_MIN_AUTOSET_TO_3p2S		3200 //


#define MAINMOT_MIN_AUTOSET_REV_1ST_TIME	500 // 200 //500 //300	//
#define MAINMOT_MIN_AUTOSET_REV_TIME	1200 // 1500 

#define MAINMOT_MIN_AUTOSET_BREAK_TO		50

static unsigned long lastPulseWidth = 9999;
unsigned long pulseWidthMs = 9999;

#define PULSE_RANGE_OK (MAINENCA_SPEC_PERIOD_SP_HIGH < pulseWidthMs) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs)

#define PULSE_MEASURE_CNT	4	// 4부터 측정( 4이상)
#define PULSE_OK_MIN_CNT	6
#define PULSE_OK_SPEC_CNT	36	// 2회전

//--------------------------------------------------------------
//	1) 실행
//	2) 결과
//		- 성공 :
//		- 실패 : 엔코더 나오지 않음
//		- 실패2 : 성공했지만 범위(60~80)을 넘음.
//	3) 결과를 "반환용문자(strMainMot_AutoSet_Result)"에 저장
//--------------------------------------------------------------

// NOT USE
void mainMot_MinOut_AutoSet_Control1_Period()
{
		static unsigned long _to;
	
		static unsigned long _toSenChg;

		static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;
	
		static int _out;
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
	
	
		
		switch(mainMotAutoSet_Step)
		{
	
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				_toReverseTime = 0;
				
				_chgCnt = 0;
				_toSenChg = 0;
				speed_high = 0;
				errK = ERR_OK;
				_out = (70+1);		// 최소값 70보다 크게.
				_deltaOut = 0;

				setTO(_to);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				if(0 == _retryCnt)	{ _toReverseTime = MAINMOT_MIN_AUTOSET_REV_1ST_TIME; }
				else				{ _toReverseTime = MAINMOT_MIN_AUTOSET_REV_TIME; }
				//
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 5회 OVER
				{
					errK = ERR_RETRY_OVER;		// REPEAT OVER
			
					setTO(_to);
					mainMotAutoSet_Step = 75; // ERROR
				}
				else	// OK
				{
					// 반복 1 ]  BLDC CCW로 돌려 줄을 풀어줌
					bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );	// 줄 풀기(Duty 50)

					setTO(_to);
					mainMotAutoSet_Step = 21;		
				}
				break;
				
			case 21:
				if(checkTO(_to, _toReverseTime) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					torqMotor.setValue(70);	//DUTY = 70 START
					setTO(_to);
					mainMotAutoSet_Step = 22;	// REPEAT
				}
				break;
				
			case 22:
				//if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
				if(checkTO(_to, 80) )	//WAIT 100 ms TO
				{
					//bbnMotor.offBldc();
					torqOff();
					setTO(_to);
					mainMotAutoSet_Step = 23;
				}
				break;

			case 23:
				//if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )	//WAIT 100 ms TO
				if(checkTO(_to, 70) )	//WAIT 100 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 30;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				_chgCnt = 0;
				_oldSen = LVL01_BBNENCA;
				pulseWidthMs = 9999;				
			//
				// == 펄스측정 START SET ???
			//
				torqMotor.setValue(_out);	//DUTY = 70 START
				setTO(_to);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) SEN CHANGE L=>H or H=>L
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy
				// 3-1) 3초 경과, 펄스OVER 혹은 T/O 체크
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					// 회전을 못함 = 반복안함
					if(PULSE_MEASURE_CNT > _chgCnt)	//  센서 변화가 없음( 4번 미만)
					{
						errK= ERR_FAIL;
						setTO(_to);
						mainMotAutoSet_Step = 75;
					}

					//else if(!PULSE_RANGE_OK)
					else
					{
						// 정지 & 반복
						_deltaOut = ( (pulseWidthMs - MAINENCA_SPEC_PERIOD_SP_MID)*10/15 );
						if(0 == _deltaOut)
						{
							if(MAINENCA_SPEC_PERIOD_SP_MID < pulseWidthMs)	// SP_LOW
								_deltaOut++;
							else
								_deltaOut--;
						}
						_out += _deltaOut;
						if(60 > _out) {_out = 60;}
						else if(80 < _out) {_out = 80;}
						/*
						if(MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs)
						{
							_out--; 		// 감소는 -1 씩
						}
						else
						{
							_out++;
						}
						*/
						setTO(_to);
						mainMotAutoSet_Step = 32;		// REPEAT			
					}
				}

			
				// 3-2) 2초 경과, 조건 만족?
				//else if( speed_high && PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2p5S) )
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2S) )
				{
					// 2nd 조건 만족?
					if(speed_high && PULSE_RANGE_OK )
					{
						speed_high = 0;
						//_out++;
					 	if( 59 < _out && 81 > _out) // 60 ~ 80
						{
							errK = ERR_OK;		//성공
							// 설정값 저장, _out
						}
						else
						{
							errK = ERR_RANGE_OVER; //RANGE OVER?
						}
						setTO(_to);
						mainMotAutoSet_Step = 75;					
					}
					// 조건 불만족, 1st조건 만족?
					else if(!speed_high && (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs))
					{
						speed_high = 1;
						// OVER SPEED = OK
						_out--; 		// 감소는 -1 씩
						setTO(_to);
						mainMotAutoSet_Step = 32;		
					}
				}

				// 2초 전, 1st조건 만족?
				else if( !speed_high && (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs))	// (속도빠름) = SPEC 10.5 ms
				{
					speed_high = 1;
					// OVER SPEED = OK
					_out--; 		// 감소는 -1 씩
					setTO(_to);
					mainMotAutoSet_Step = 32;
				}
				break;

			// TORQ OFF
			case 32:
				// Last Pulse Width 저장
				lastPulseWidth = pulseWidthMs;
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;					


				break;

			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// TORQ STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{
					_retryCnt++;		// 반복회수 증가
					setTO(_to);
					mainMotAutoSet_Step = 20;	// REPEAT
				}
				break;


			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				lastPulseWidth = pulseWidthMs;
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(_out));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = (int)_out;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FAIL;
						mainMotAutoSet_Step = 80; //NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				
				break;
			
			case 90:
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 91;					
				break;

			case 91:
				if(checkTO(_to, 50) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 93;	// REPEAT
				}
				break;
			// OK
			case 93:

				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
		
				break;
			
			default:
				break;
				

		}

}

static int minOut = 0;
//---------------------------------------------------------------
// 일정속도 도달까지의 시간을 측정, 최소출력값으로 환산
//---------------------------------------------------------------
void mainMot_MinOut_AutoSet_Control2_Time()
{

		static unsigned long _to;
		static unsigned long _toErr;
		static long okTime;
		static long logTime;
	
		static unsigned long _toSenChg;

		static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;

		static int _out;
		
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
		String logMsg;
	
	
		
		switch(mainMotAutoSet_Step)
		{
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				_toReverseTime = 0;
				
				_chgCnt = 0;
				_toSenChg = 0;
				speed_high = 0;
				errK = ERR_OK;
				
				_out = (0);		// 최소값 70보다 크게.
				minOut = 0;
				_deltaOut = 0;

				okTime = 1060;		// 계산오류 방지
				logTime = 5555;		// 계산오류 방지

				setTO(_to);
				setTO(_toErr);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				_retryCnt++;
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 2회 OVER
				{
					if(ERR_OK == errK) { errK = ERR_RETRY_OVER; }		//반복오버  ( -1 )
					setTO(_to);
					mainMotAutoSet_Step = 75; //반복회수 오버 => 장애 처리
				}
				else	// OK
				{
					torqMotor.setValue(70);	//DUTY = 70 START
					setTO(_to);
					mainMotAutoSet_Step = 30;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				_chgCnt = 0;
				_oldSen = LVL01_BBNENCA;
				pulseWidthMs = 9999;

				setTO(_to);
				setTO(_toErr);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) 엔코더 신호 변화 검지.
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy

				// 		3-1) 3초 경과, 펄스OVER 혹은 T/O 체크
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					logTime = getTO(_to);
					// 회전을 못함 = 반복안함
					setTO(_to);
					if(PULSE_OK_MIN_CNT > _chgCnt)
					{
						errK= ERR_NOT_ROTATE;	//( -6 )
						mainMotAutoSet_Step = 75;		//회전 안됨 => 바로 장애
					}
					else
					{
						errK= ERR_TO_OVER;	// ( -4 )
						mainMotAutoSet_Step = 32;		//시간경과 => 반복
					}
				}
				
				//		3-2) 일정속도(펄스폭) 도달
				else if( (PULSE_OK_MIN_CNT < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs) )
				{
					lastPulseWidth = pulseWidthMs;
					okTime = getTO(_to);
					logTime = okTime;
					if(0 > okTime)
					{
						errK = ERR_FW_CAL_NG; //RANGE OVER? ( - 2 )
						setTO(_to);
						mainMotAutoSet_Step = 75;		// 급정지
					}
					else
					{
						_out = MAIN_MOT_DEFAULT_MIN_DUTY + ((okTime - 1060)/88);
						//_out = 50 + ((okTime - 1060)/176);	// TEST
						//_out = 50 + ((okTime - 1060)/27);		// TEST
						// 계산 결과 범위 밖 : 60 ~80 외
						//if(60 > _out && 80 < _out)
						//if(60 > _out || 85 < _out)
						if(MAIN_MOT_DEFAULT_MIN_DUTY > _out)
						{
							// Error
							minOut = MAIN_MOT_DEFAULT_MIN_DUTY;
						}
						else if (MAIN_MOT_DEFAULT_MAX_DUTY < _out)
						{
							minOut = MAIN_MOT_DEFAULT_MAX_DUTY;
						}
						else
						{
							minOut = _out;
						}				
						errK= ERR_OK;	// 성공
						setTO(_to);
						mainMotAutoSet_Step = 32;		// 급정지
					}
				}
				break;

			
			// 반복 , TORQ OFF
			case 32:
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;
				break;

			// 급정지 대기
			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// BLDC STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{

					if(ERR_OK == errK)
					{
						setTO(_to);
						mainMotAutoSet_Step = 75; 	//성공 => 종료
					}
					else
					{
						setTO(_to);
						mainMotAutoSet_Step = 20;	// REPEAT
					}
				}
				break;

			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(minOut));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = minOut;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FS_WRITE;		// FS쓰기 에러( -7 )
						mainMotAutoSet_Step = 80;	//NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}

				// 검출결과, AP에 출력(확인용)
				Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG") + ",try:" + String(_retryCnt) + ",Out:" + String(_out)  + ",Save:" + String(minOut) +",ms:" + String(logTime) );
				
				
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				
				break;

				// 성공
			case 90:
				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
		
				break;

			default:
				break;
	}
}

#define MMOT_AUTOSET_OUT_DUTY	80	// 80 Duty
//---------------------------------------------------------------
//  측정시 출력 : 80 Duty
//	측정 : 36 펄스(2회전)까지의 경과 시간 측정
// 	에러: 3.2초(완전율 2배)경과해도 36펄스 도달못하면 장애.
//---------------------------------------------------------------

void mainMot_MinOut_AutoSet_Control3_36PulseTime()
{

		static unsigned long _to;
		
		static long okTime = 0;
	
		//static unsigned long _toSenChg;

		//static unsigned long _toReverseTime;
	
		static unsigned short oldmainMotAutoSet_Step;
		static short errK;

		static int _out;
		//static int _hz;
		
		static unsigned short _oldSen;
	
		static unsigned short _retryCnt;
		static unsigned short _chgCnt;

		//static unsigned short speed_high;	// 속도OVER 만족 FLAG

		static int _deltaOut;
		
		short result;
		
		String msg;
		String logMsg;
	
	
		
		switch(mainMotAutoSet_Step)
		{
			//IDLE
			case 0:
				break;
			
			// === START
			case 10:
				torqOff();					// 낚시줄 텐센 제거
				bbnMotor.offBldc();

				_retryCnt = 0;		// 반복회수 초기화
				//_toReverseTime = 0;
				
				_chgCnt = 0;
				//_toSenChg = 0;
				//speed_high = 0;
				errK = ERR_OK;
				
				_out = (0);		//
				//_hz = 2000;
				minOut = 0;
				//_deltaOut = 0;

				okTime = 0;		// 계산오류 방지
				//logTime = 0;		// 계산오류 방지

				setTO(_to);
				//setTO(_toErr);
				mainMotAutoSet_Step = 11;
				break;
			
			// 정지 50ms 대기
			case 11:
				if(checkTO(_to, 50))	//WAIT 50 ms TO
				{
					setTO(_to);
					mainMotAutoSet_Step = 20;
				}
				break;


			// === 반복 회수 체크
			case 20:
				//
				_retryCnt++;
				if(MAINMOT_MIN_RETRY_CNT < _retryCnt)	// Retry 2회 OVER
				{
					if(ERR_OK == errK) { errK = ERR_RETRY_OVER; }		//반복오버  ( -1 )
					setTO(_to);
					mainMotAutoSet_Step = 75; //반복회수 오버 => 장애 처리
				}
				else	// OK
				{
					_chgCnt = 0;
					_oldSen = LVL01_BBNENCA;
					pulseWidthMs = 9999;
					
					torqMotor.setValue(MMOT_AUTOSET_OUT_DUTY);	//DUTY = 70 START
					setTO(_to);
					//mainMotAutoSet_Step = 30;
					mainMotAutoSet_Step = 31;
				}
				break;

			// === CW, 메인모터 출력, 펄스주기 측정
			case 30:
				// 반복시 변수 CLR
				//_chgCnt = 0;
				//_oldSen = LVL01_BBNENCA;

				setTO(_to);
				//setTO(_toErr);
				mainMotAutoSet_Step = 31;
				break;

			case 31:
				//== 1) 엔코더 신호 변화 검지.
				if(LVL01_BBNENCA != _oldSen)	// 1st Change times
				{
					_oldSen = LVL01_BBNENCA;	// 센서상태 저장
					_chgCnt++;
				}
				
				// == 2) 펄스폭 가져오기, 회전해서 4번 이후(Pulse2번)
				if(PULSE_MEASURE_CNT <= _chgCnt)	// 4이상~
				{
					//펄스 HIHG때 펄스주기 저장!!
					portENTER_CRITICAL(&mux);  // 임계 영역 시작
						pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
					portEXIT_CRITICAL(&mux);   // 임계 영역 종료
				}

				// == 3) 판단처리 : 3가지
				if(0) {}	// Dummy

				// 		3-1) 3.2초 경과, 36펄스 미도달
				else if( checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3p2S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
				{
					okTime = getTO(_to);
					lastPulseWidth = pulseWidthMs;
					// 회전을 못함 = 반복안함
					setTO(_to);
					if(PULSE_OK_MIN_CNT > _chgCnt)
					{
						errK= ERR_NOT_ROTATE;	//( -6 )
						mainMotAutoSet_Step = 75;		//회전 안됨 => 바로 장애
					}
					else
					{
						errK= ERR_TO_OVER;	// ( -4 )
						mainMotAutoSet_Step = 32;		//시간경과 => 반복
					}
				}
				
				//		3-2) 36펄스(2회전) 도달
				else if( (PULSE_OK_SPEC_CNT*2) < _chgCnt )
				{
					lastPulseWidth = pulseWidthMs;
					okTime = getTO(_to);
					//Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG") + ",ms:" + String(okTime));
					//logTime = okTime;
					if(0 > okTime)
					{
						errK = ERR_FW_CAL_NG; //RANGE OVER? ( - 2 )
						setTO(_to);
						mainMotAutoSet_Step = 75;		// 급정지
					}
					else
					{
						if(900 > okTime)
						{
							_out = 10 + (0.0667*okTime);
							//_out = 0 + (0.0667*okTime);		// TEST
						}
						else
						{
							_out = 53 + ( 0.0175*okTime);
						}

						if(MAIN_MOT_DEFAULT_MIN_DUTY > _out)
						{
							// Error
							minOut = MAIN_MOT_DEFAULT_MIN_DUTY;
						}
						else if (MAIN_MOT_DEFAULT_MAX_DUTY < _out)
						{
							minOut = MAIN_MOT_DEFAULT_MAX_DUTY;
						}
						else
						{
							minOut = _out;
						}				
						errK= ERR_OK;	// 성공
						setTO(_to);
						mainMotAutoSet_Step = 32;		// 급정지
					}
				}
				break;

			
			// 반복 , TORQ OFF
			case 32:
				torqOff();
				bbnMotor.onBldc(BBN_MOT_CCW, (DEVCHK_BBN_WIRE_REL_DUTY+0) );
				setTO(_to);
				mainMotAutoSet_Step = 33;
				break;

			// 급정지 대기
			case 33:
				if(checkTO(_to, MAINMOT_MIN_AUTOSET_BREAK_TO) )	//WAIT 500 ms TO
				{
					bbnMotor.offBldc();
					setTO(_to);
					mainMotAutoSet_Step = 34;	// REPEAT
				}
				break;

			// BLDC STOP WAIT
			case 34:
				if(checkTO(_to, 200) )	//WAIT 500 ms TO
				{

					if(ERR_OK == errK)
					{
						setTO(_to);
						mainMotAutoSet_Step = 75; 	//성공 => 종료
					}
					else
					{
						setTO(_to);
						mainMotAutoSet_Step = 20;	// REPEAT
					}
				}
				break;

			// === NOT USE
			case 50:
				break;


			// === OK,ERR 판별
			case 75:
				// 모터 OFF
				torqOff();
				bbnMotor.offBldc();
			//
				// 펄스측정 정지.
			//
			
				if(ERR_OK == errK)
				{
					// 파일 저장
					result = fsInfo.saveInfo(MAINMOT_INFO, String(minOut));
					if(0 < result)	// 파일쓰기 OK
					{
						defaultTorqueMotor = minOut;
						
						// == 설정값 응답
						msg = STX_INFO_REQ + MAINMOT_INFO_READ;
						msg += String(defaultTorqueMotor);
						msg += "%";
						Resp2ApPrintln(msg);
						
						mainMotAutoSet_Step = 90; // END-OK
					}
					else	// FS쓰기 NG
					{
						errK = ERR_FS_WRITE;		// FS쓰기 에러( -7 )
						mainMotAutoSet_Step = 80;	//NG
					}
				}
				else
				{
					//setTO(_to);
					mainMotAutoSet_Step = 80; //NG
				}

				// 검출결과, AP에 출력(확인용)
				Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + MAIN_MMOT_AUTOSET_ERR + (ERR_OK==errK? "OK":"NG")
					+ ",try:" + String(_retryCnt) + ",Out:" + String(_out)  + ",Save:" + String(minOut) 
					+",cnt:" + String(_chgCnt) +",ms:" + String(okTime) +",plsWid:" + String(lastPulseWidth));
				
				
				break;
				
			// Error SET
			case 80:
				mainMotAutoSet_Flag = 0;	//에러 => DEVICE CHK종료
				mainMotAutoSet_Step = 0;
				//devCheckFlagStep_AllClear();

				// 1) 실패 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ + "1" + String(errK);	// KK=MMOT AUTOSET, a=sTART
				msg += "%";
				Resp2ApPrintln(msg);
				
				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_MMOT_AUTOSET_ERR;
				msg += "%";
				Resp2ApPrintln(msg);
			
				
				break;

				// 성공
			case 90:
				mainMotAutoSet_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
				mainMotAutoSet_Step = 0;	// Clr Step

				// 1) 성공 응답
				msg = STX_DEV_SET_REQ + STR_MAINMOT_MINSET_REQ;
				msg += "1";		// ACT =1 : START
				msg += String(defaultTorqueMotor);
				msg += "%";
				Resp2ApPrintln(msg);

				//2) 장애코드 통지
				msg = STX_DEV_CONT_REQ + MAIN_DEV_CHK_OK;
				msg += "%";
				Resp2ApPrintln(msg);
				
		
				break;

			default:
				break;
	}
}


//-------------------------------------------------------------------------
//--- TORQ OFF CONTROL for STBY

void stbyTorqOffControl_Start()
{
	// [장치장애검지] 중이면, 요구Flage세트
	if(devReadychkFlag)
	{
		stbyTorqOffControl_Req_Flag = 1;	// []장치장애검지]처리 종료까지 대기.
	}
	else
	{
		stbyTorqOffControl_Req_Flag = 0;
		stbyTorqOffControl_Flag = 1;		//Control ENB
		stbyTorqOffControlStep = STEP_START;
	}
}

// (V108) flag 추가함 - 0:토크모터 동작 안함. 이외 기본값 출력
void stbyTorqOffControl_Stop(int flag)
{
	int outTorq=0;
	//if(stbyTorqOffControl_Flag)
	{
		stbyTorqOffControl_Flag = 0;		// Control DIS
		stbyTorqOffControlStep = STEP_IDLE;

		if (flag)
		{
			if(recv_TorqMotor_Flag)
			{
				torqMotor.setValue(reqTorqueMotor);	//토크 =  AP 요구값
				outTorq = reqTorqueMotor;
			}
			else
			{
				torqMotor.setValue(defaultTorqueMotor); //토크 = 최소출력값 OUT
				outTorq = defaultTorqueMotor;
			}
		}
		bbnMotor.offBldc();						// BLDC OFF
		// FW자체 출력 응답
		//Resp2ApPrintln(STX_CMD_TORQUE_MOTOR + fillZero2String3Char(defaultTorqueMotor) + "%");
		send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);		//FW자체출력응답
		//Resp2ApPrintln(STX_CMD_BLDC_MOTOR + "00000000%");	// $04AdddTTTT%
		Resp2ApPrintln(STX_BBN_MOTOR + "00000000%");	// $04AdddTTTT%
	}
}

// [STBY 토크OFF제어] 요구가 있는 경우, OFF제어 시작
void stbyTorqOff_ReqCheck()
{
	if(stbyTorqOffControl_Req_Flag)
	{
		// TBD : 일정시간후에 시작 필요(조작감)

		// 예약이면, "대기OFF제어"를 시작
		stbyTorqOffControl_Req_Flag = 0;
		stbyTorqOffControl_Start();	
	}
}

//------------
#define ENC_STOP_WAIT_TO		CHECK_TIME_1SEC_TO //3000	// 2 SEC
#define TENSION_HOLD_TIME_TO	7000 // 5000	// 5SEC

#define STBY_OFF_CONT_SEN	LVL10_BBNENCA	// 10ms 센서 데이타
#define STBY_OFF_CONT_ENCAB	LVL10_ENCAB		// 10ms 센서 데이타

#define	STBY_BBN_MIN_DUTY		10
#define	BBN_MOVE_DETECT_CNT		10
//---------------------------------------------
// 1) 대기중에 OFF제어 개시.
//  -- STBY시 토크OFF
// 	-- 엔코더 감시중 변화 검지 => 최소토크 출력
// 	--엔코더 감시중 정지 검지 => 토크 OFF
// 2) 대기중에서 상태가 변경되면
//  -- 토크 최소출력값 출력후
//  --OFF제어 중지
// 3) STBY 토크 OFF중에는 AP명령 무시
//----------------------------------------------
// CALL 10ms
//----------------------------------------------
void stbyTorqOff_Control()
{

	static unsigned long _to;
	static unsigned long _toErr;

	static unsigned short oldstbyTorqOffControlStep;
	static short errK = ERR_OK;

	static unsigned short _out;
	static unsigned short _oldSen;

	static unsigned short _retryCnt = 0;
	static unsigned short _chgCnt = 0;

	static unsigned short _senErrK = ENC_SEN_OK;

	int outTorq;
	
	String msg;
	String logMsg;

	switch(stbyTorqOffControlStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			bbnMotor.offBldc();

			_retryCnt = 0;
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;

			//== 먼저 "줄끊어짐 검지"==
			setTO(_to);
			stbyTorqOffControlStep = 1;
			break;

			break;

		// 모터정지 대기
		case 1:
			if(checkTO(_to, 50))	//WAIT 50 ms TO
			{
				setTO(_to);
				stbyTorqOffControlStep = 2;
			}
			break;

		//토크모터 기동(최소값+5)
		case 2:
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;

			if(recv_TorqMotor_Flag)
				torqMotor.setValue(reqTorqueMotor);	// 최소값 출력
			else
				torqMotor.setValue(defaultTorqueMotor); // 최소값 출력
			setTO(_to);
			setTO(_toErr);
			stbyTorqOffControlStep = 3;
			break;

		case 3:
			// ERR
			if( checkTO(_toErr, CHECK_TIME_10SEC_TO) )
			{
				//ERR
				errK = ERR_WIRE_BROKEN;		// 10초 경과 줄 끊어짐
				setTO(_to);
				stbyTorqOffControlStep = 75;
			}
			// OK
			else if( checkTO(_to, CHECK_TIME_1SEC_TO) )	//ENC센서 안정후 2초 경과 => OK
			{
				_retryCnt = 0;
				setTO(_to);
				stbyTorqOffControlStep = 11;		// 대기OFF제어 시작
			}
			else if(STBY_OFF_CONT_SEN != _oldSen)
			{
				_oldSen = STBY_OFF_CONT_SEN;
				// TBD, ENC방향 검지(유저가 당겼다 놓았다는 건 방향이 계속 바뀌니까)
				_chgCnt++;
				setTO(_to);
			}
			break;


			
		// 줄 서서히 풀기(0.3마다 4번)
		case 11:
			_retryCnt++;
			if(12 < _retryCnt)
			{
				torqOff();
				setTO(_to);
				stbyTorqOffControlStep = 20;
			}
			else
			{
				// 1.2 SEC간 토크를 서서히 줄임!
				if(recv_TorqMotor_Flag)
					torqMotor.setValue( reqTorqueMotor + 0 - (_retryCnt*2) );					// 낚시줄 텐센 제거
				else
					torqMotor.setValue( defaultTorqueMotor + 0 - (_retryCnt*2) );					// 낚시줄 텐센 제거
				setTO(_to);
				stbyTorqOffControlStep = 12;
			}
			break;

			// 0.3초 마다 * 4번 반복
		case 12:
			if(checkTO(_to, 100))
			{
				setTO(_to);
				stbyTorqOffControlStep = 11;
			}
			break;


		// OFF제어 개시  & 반복
		case 20:
			_chgCnt = 0;
			_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장

			setTO(_to);
			setTO(_toErr);
			stbyTorqOffControlStep = 21;
			break;
			
		// OFF시, ENC정지 대기 = 소비전력 SAVE
		case 21:
			// 20초간 ENC정지 안함
			if(checkTO(_toErr, CHECK_TIME_20SEC_TO))
			{
				// Error : ???

				errK = ERR_ENC_NOT_STABLE;
				// 상태 변경
				setTO(_to);
				stbyTorqOffControlStep = 75;	// TBD
			}
			else
			{
				// 센서 변화 없이,  3초 경과
				//if(checkTO(_to, ENC_STOP_WAIT_TO))		// 3초
				if(checkTO(_to, ENC_STOP_WAIT_TO))		// 3초
				{
					// 변수 CLR
					_chgCnt = 0;
					_oldSen = STBY_OFF_CONT_SEN;
					// 상태 변경
					setTO(_to);
					stbyTorqOffControlStep = 22;
				}
				// 센서 변화 유?
				else if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
				{
					_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
					_chgCnt++;		// 신호변화 cnt
					setTO(_to);		// 변화시 TO CLR
				}
			}
			break;
			
		//엔코더 변화 check (at OFF상태)
		case 22:
			// 센서변화 CHECK
			if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
			{
				_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
			}

			// 1초내, 4회 이상 변화
			if( (4 < _chgCnt) && checkTO(_to, CHECK_TIME_1SEC_TO))
			{
				setTO(_to);
				stbyTorqOffControlStep = 30;
			}
			break;

		//텐션 유지 개시
		case 30:
			_chgCnt = 0;
			if(recv_TorqMotor_Flag)
			{
				torqMotor.setValue(reqTorqueMotor);
				outTorq =reqTorqueMotor;
			}
			else
			{
				torqMotor.setValue(defaultTorqueMotor);
				outTorq = defaultTorqueMotor;
			}
			//Resp2ApPrintln(STX_CMD_TORQUE_MOTOR + fillZero2String3Char(defaultTorqueMotor) + "%");	// FW자체출력 응답
			send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);
		
			setTO(_to);
			stbyTorqOffControlStep = 31;
			break;

			
		// ON시, ENC정지 대기 = 텐션유지
		case 31:
			// 센서 변화 없이,  7초 경과
			if(checkTO(_to, TENSION_HOLD_TIME_TO))
			{
				// 변수 CLR
				_chgCnt = 0;
				_oldSen = STBY_OFF_CONT_SEN;
				// 상태 변경
				setTO(_to);
				stbyTorqOffControlStep = 32;
			}
			// 센서 변화 유?
			else if(STBY_OFF_CONT_SEN != _oldSen)	// 1 times
			{
				_oldSen = STBY_OFF_CONT_SEN;	// 센서상태 저장
				_chgCnt++;		// 신호변화 cnt
				setTO(_to);		// 변화시 TO CLR
			}
			break;

		case 32:
			//torqOff();		// 모터 OFF
			bbnMotor.offBldc();
		
			//stbyTorqOffControlStep = 20; 	// 반복
			_retryCnt = 0;
			stbyTorqOffControlStep = 11; 	// 반복 (서서히 풀기)
			break;

		//
		case 40: 
			break;

		// NOT_USE
		case 60:
			break;

		// ERR | OK 판별
		case 75:
			torqOff();		// 모터 OFF
			bbnMotor.offBldc();

			// OK,ERR판별
			if(ERR_OK == errK)
			{
				stbyTorqOffControlStep = 90;	// END-OK
			}
			else
			{
				stbyTorqOffControlStep = 80;	//NG
			}
			break;

		// Error SET
		case 80:
			stbyTorqOffControl_Flag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			stbyTorqOffControlStep = 0;
	
			// Err MSG Send
			if(ERR_WIRE_BROKEN == errK)
			{
				msg = STX_DEV_CONT_REQ + MAIN_WIRE_BROKEN_ERR;
				logMsg = MAIN_WIRE_BROKEN_ERR;
			}
			else
			{
				msg = STX_DEV_CONT_REQ + MAIN_WIRE_NOTSTABLE_ERR;
				logMsg = MAIN_WIRE_NOTSTABLE_ERR;
			}
			msg += "%";
			Resp2ApPrintln(msg);
	
				
			break;
	
		// OK
		case 90:
			stbyTorqOffControl_Flag = 0;	// CLR BLDC Flag
			stbyTorqOffControlStep = 0;		// Clr Step
	
	
			break;

		default:
			break;
		
	}

}

//-------------------------------------------------------------------------
typedef struct _stAgingPara
{
	unsigned short cw_duty1;
	unsigned short bldc_duty1;		// BLDC CW
	unsigned long cw_time1;
	unsigned short cw_duty2;
	unsigned short bldc_duty2;	
	unsigned long cw_time2;
} stAgingPara;
typedef stAgingPara* ptrstAgingPara;

stAgingPara wireAging_Mmot =
{
	100, 20, 2000,			// duty1, time1
	 70,  0, 1000,			// duty2, time2
};


//---------- WIRE AGING TEST------------------------
static String strData[10] =
{
	"",
};

//------- WIRE AGING TEST -----------------------------
// 설정값 저장 & START
void wireAging_Test_Start(String msg)
{
	unsigned short length;

	unsigned short i = 0;

	// == 1) PARA SET
	//ptrstAgingPara ptrSt = wireAging;
	length = splitTextToStrings(msg.substring(7), strData);

	for(i=0; i<length; i++)
	{
		if(0==i) wireAging_Mmot.cw_duty1 = strData[i].toInt();
		if(1==i) wireAging_Mmot.bldc_duty1 = strData[i].toInt();
		if(2==i) wireAging_Mmot.cw_time1 = strData[i].toInt();
		if(3==i) wireAging_Mmot.cw_duty2 = strData[i].toInt();
		if(4==i) wireAging_Mmot.bldc_duty2 = strData[i].toInt();
		if(5==i) wireAging_Mmot.cw_time2 = strData[i].toInt();
	}


	// == 2) START
	wireAgingTest_Flag = 1;
	wireAgingTest_Step = 10;
	
}

unsigned short splitTextToStrings(String msg, String *s)
{
	unsigned short  idx = 0;
	int startIdx = 0;
	int commaIdx = msg.indexOf(',');

	while (commaIdx != -1) {
		s[idx++] = msg.substring(startIdx, commaIdx);
		startIdx = commaIdx + 1;
		commaIdx = msg.indexOf(',', startIdx);
	}

	// 마지막 토큰
	s[idx++] = msg.substring(startIdx);

	return idx;
}

// 정지
static unsigned long agingPulseWidth = 9999;

void wireAging_Test_Stop(String msg)
{
	wireAgingTest_Flag = 0;
	wireAgingTest_Step = 0;
	torqOff();
}

// 동작 제어(반복)
void wireAging_Test_Control()
{
			static unsigned long _to;
		
			static unsigned long _toSenChg;
		
			static unsigned short oldwireAgingTest_Step;
			static short errK;
		
			static int _out;
			static unsigned short _oldSen;
		
			static unsigned short _retryCnt;
			static unsigned short _chgCnt;
	
			static unsigned short speed_high;	// 속도OVER 만족 FLAG
	
			static int _deltaOut;
			
			short result;
			
			String msg;
		
		
			
			switch(wireAgingTest_Step)
			{
		
				//IDLE
				case 0:
					break;
				
				// === START
				case 10:
					torqOff();					// 낚시줄 텐센 제거
					bbnMotor.offBldc();
	
					_retryCnt = 0;
					_chgCnt = 0;
					//_toSenChg = 0;
					//speed_high = 0;
					errK = ERR_OK;
					//_out = (70+1);		// 최소값 70보다 크게.
					///_deltaOut = 0;
	
					setTO(_to);
					wireAgingTest_Step = 11;
					break;
				
				// 정지 50ms 대기
				case 11:
					if(checkTO(_to, 50))	//WAIT 50 ms TO
					{
						setTO(_to);
						wireAgingTest_Step = 20;
					}
					break;
	
	
				// === 반복 회수 체크
				case 20:
					_retryCnt++;		// 반복회수 증가
					// 동작 개시
					{
						// 반복 1 ]  BLDC CCW로 돌려 줄을 풀어줌
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt = 0;
						torqMotor.setValue(wireAging_Mmot.cw_duty1);	// 줄 감기1(세게)
						bbnMotor.onBldc(BBN_MOT_CW, wireAging_Mmot.bldc_duty1);	// 줄 감기1(세게)
						setTO(_to);
						wireAgingTest_Step = 21;
					}
					break;
					
				case 21:
					if( checkTO(_to, wireAging_Mmot.cw_time1) ) //WAIT 500 ms TO
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt = 0;
						torqMotor.setValue(wireAging_Mmot.cw_duty2);	// 줄 감기2 (약하게)
						bbnMotor.onBldc(BBN_MOT_CW, wireAging_Mmot.bldc_duty2);	// 줄 감기1(세게)
						setTO(_to);
						wireAgingTest_Step = 22;	//
					}
					// 안도는 경우
					else if( (2 > _chgCnt) && checkTO(_to,(wireAging_Mmot.cw_time1 - 50)) )	// 1st Change times
					{
						// ERR
						errK = ERR_NG;
						setTO(_to);
						wireAgingTest_Step = 75;	//		
					}
					// 너무 빨리 도는 경우(줄 끊어짐) = 검지 어려움
					//{
					//}
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
					
				case 22:
					if(checkTO(_to, wireAging_Mmot.cw_time2) ) //WAIT 500 ms TO
					{
						setTO(_to);
						wireAgingTest_Step = 23;	// 
					}
					// 안도는 경우
					else if( (2 > _chgCnt) && checkTO(_to,(wireAging_Mmot.cw_time2-50)) )	// 1st Change times
					{
						// ERR
						errK = ERR_NG;
						setTO(_to);
						wireAgingTest_Step = 75;	//
					}
					// 너무 빨리 도는 경우(줄 끊어짐) = 검지 어려움
					//{
					//}
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
					
				case 23:
					// 에이징 카운트 up통지
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += "9+1";
					msg += "%";
					Resp2ApPrintln(msg);

					setTO(_to);
					wireAgingTest_Step = 20;	// REPEAT
					break;


				// === NOT USE ===
				
				// === CW, 메인모터 출력, 펄스주기 측정
				case 30:
					// 반복시 변수 CLR
					_chgCnt = 0;
					_oldSen = LVL01_BBNENCA;
					pulseWidthMs = 9999;				
				//
					// == 펄스측정 START SET ???
				//
					torqMotor.setValue(_out);	//DUTY = 70 START
					setTO(_to);
					wireAgingTest_Step = 31;
					break;
	
				case 31:
					// 회전해서 4번 이후(Pulse2번)
					if(6 < _chgCnt)
					{
						//펄스 HIHG때 펄스주기 저장!!
						portENTER_CRITICAL(&mux);  // 임계 영역 시작
							pulseWidthMs = iEncIntrIntervalTime;	// 인터럽트에서 측정한 펄스주기 가져오기
						portEXIT_CRITICAL(&mux);   // 임계 영역 종료
					}
	
					// 판단 3가지
					//if( !speed_high && (4 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs)) // (속도빠름) = SPEC 10.5 ms
					//if( !speed_high && (6 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_MID > pulseWidthMs)) // (속도빠름) = SPEC 10.5 ms
					if( !speed_high && (6 < _chgCnt) && (MAINENCA_SPEC_PERIOD_SP_LOW > pulseWidthMs) )	// (속도빠름) = SPEC 10.5 ms
					{
						speed_high = 1;
						// OVER SPEED = OK
						_out--; 		// 감소는 -1 씩
						setTO(_to);
						wireAgingTest_Step = 32;			
	
					}
	
					else if( (speed_high) && PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_2S) )
					{
						//if(speed_high)
						{
							speed_high = 0;
							//_out++;
							if( 59 < _out && 81 > _out) // 60 ~ 80
							{
								errK = ERR_OK;		//성공
								// 설정값 저장, _out
							}
							else
							{
								errK = ERR_RANGE_OVER; //RANGE OVER?
							}
							setTO(_to);
							wireAgingTest_Step = 75;					
						}

					}
	
					//펄스OVER 혹은 T/O 체크
					//if( ((BBN_2REV_SIGCHG_CNT < _chgCnt) || checkTO(_to, MAINMOT_MIN_AUTOSET_TO) ) && (HIGH == LVL01_BBNENCA) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
					else if( !PULSE_RANGE_OK && checkTO(_to, MAINMOT_MIN_AUTOSET_TO_3S) ) // 2회전 (36회 펄스 ) 혹은 T/O  1 SEC && HIGH
					{
						// 회전을 못함 = 반복안함
						if(6 > _chgCnt) //	센서 변화가 없음
						{
							errK= ERR_FAIL;
							setTO(_to);
							wireAgingTest_Step = 75;
						}
						else
						{
							// 정지 & 반복
							_deltaOut = ( (pulseWidthMs - MAINENCA_SPEC_PERIOD_SP_MID)*10/15 );
							if(0 == _deltaOut)
							{
								if(MAINENCA_SPEC_PERIOD_SP_MID < lastPulseWidth)	// SP_LOW
									_deltaOut = 1;
								else
									_deltaOut = -1;
							}
							_out += _deltaOut;

							setTO(_to);
							wireAgingTest_Step = 32;					
						}
					}
	
			
					// CHANGE L=>H or H=>L
					else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
					{
						_oldSen = LVL01_BBNENCA;	// 센서상태 저장
						_chgCnt++;
					}
					break;
	
	
				// === NOT USE
				case 50:
					break;
	
	
				// === OK,ERR 판별
				case 75:
					// 모터 OFF
					agingPulseWidth = pulseWidthMs;
					torqOff();
					bbnMotor.offBldc();
				//
					// 펄스측정 정지.
				//
				
					if(ERR_OK == errK)
					{
						wireAgingTest_Step = 90; //OK
					}
					else
					{
						//setTO(_to);
						wireAgingTest_Step = 80; //NG
					}
					break;
					
				// Error SET
				case 80:
					wireAgingTest_Flag = 0;	//에러 => DEVICE CHK종료
					wireAgingTest_Step = 0;
					//devCheckFlagStep_AllClear();
	
					// 1) 실패 응답
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += "1" + String(errK);		// Act=1
					msg += "%";
					Resp2ApPrintln(msg);
					
					
					break;

				// OK
				case 90:
	
					wireAgingTest_Flag = 0;	// 정상 => End에서 종료. CLR BLDC Flag
					wireAgingTest_Step = 0;	// Clr Step
	
					// 1) 성공 응답
					msg = STX_DEV_FUNCTION_REQ + STR_WIRE_AGING_TEST;
					msg += STR_ERR_OK;
					msg += "%";
					Resp2ApPrintln(msg);
	
			
					break;
				
				default:
					break;
					
	
			}

}
//-------------------------------------------------------


//--------------------------------------------
// 보드 LED_2 Control
//	AP-O + ROD -O : 0.5sec
//	AP-O + ROD -X : 2 sec
//	AP-X + ROD-O : ON
//	AP-X + ROD-X : OFF
//	CALL 500ms
//--------------------------------------------
static volatile short bdLed2_out = 0;
//--------------------------------------------
void boardLed_Control()
{


	// 보드 LED Control
	if(DIAG_MODE != mainMode)
	{
		if(PROG_END == gameStatus)		// NOT AP EXEC = LED3 OFF
		{
			bdLed2_out = LOW;
			digitalWrite(BD_LED2_PIN, bdLed2_out); 	// LED3 OFF
		}
		else
		{

			//if (ROD_CONN == rod_conn_status && CONNECT == imu_conn_status)		  // ROD & IMU Conected
			if (CONNECT == ap_conn_status)	  // ROD & IMU Conected
			{
				if(ROD_CONN == rod_conn_status)
				{
					// 0.5 SEC
					bdLed2_out = !bdLed2_out;
					digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 500ms
				}
				else
				{
					// 2.5 SEC
					if(!(count_500ms % 5))	// 2.5 SEC
					{
						bdLed2_out = !bdLed2_out;
						digitalWrite(BD_LED2_PIN, bdLed2_out);	  // LED3 Blinking 2 SEC
					}
				}
			}
			// AP NOT CONN
			else
			{
				if(ROD_CONN == rod_conn_status)
				{
					bdLed2_out = HIGH;
					digitalWrite(BD_LED2_PIN, bdLed2_out); 	// LED 3 ON
				}
				else
				{
					bdLed2_out = LOW;
					digitalWrite(BD_LED2_PIN, bdLed2_out);		// LED 3 OFF
				}
			}
		}
	}

}


//-----------------------------------------------------------------
//  READ INPUT
//	1ms 정도가 필요한 센서만 1ms READ
//	Encoder, FG
//
void read_input_1ms()
{
	// 아래를 센서 Map개수 만큼 For문 돌릴 것. 현재는  Sensor Map 1개(16bit)만 있음!
	{
	// 1) Input Read
	
	sensor1ms[SENSOR_1].curr =  (digitalRead(BBN_ENC_B_DT_PIN) << SNS_bbnEncB) |
								(digitalRead(BBN_ENC_A_CK_PIN) << SNS_bbnEncA) |
								(digitalRead(BLDC_FG_PIN) << SNS_bbnMotFg);
	if(DF_CONFIG_LMJIG == dfConfig)
	{
		sensor1ms[SENSOR_1].curr |= (digitalRead(LM_HOME_SEN_PIN) << SNS_LM_HOME) |
									(digitalRead(LM_LEFT_SEN_PIN) << SNS_LM_LEFT) |
									(digitalRead(LM_RIGHT_SEN_PIN) << SNS_LM_RIGHT) |
									(digitalRead(LM_ENC_SEN_PIN) << SNS_LM_ENC) |
									(digitalRead(LM_MOT_FG_PIN) << SNS_LMMOT_FG);
	}
	//sensor1ms[SENSOR_1].curr |= ((digitalRead(PWROFF_SWC_PIN)) << SNS_PwrOffSwc);	// Low Active
	//sensor1ms[SENSOR_1].curr |= (digitalRead(PC_USB5V_PIN) << SNS_usb5V);
	//sensor1ms[SENSOR_1].curr |= ((!digitalRead(BOOT_MODE_PIN)) << SNS_boot);		// Low Active

	// 2) 연산
	sensor1ms[SENSOR_1].lvl = ( (sensor1ms[SENSOR_1].lvl & sensor1ms[SENSOR_1].old) | (sensor1ms[SENSOR_1].curr & (sensor1ms[SENSOR_1].lvl ^ sensor1ms[SENSOR_1].old)));		// (V108)
	// 3) LE/TE검출
	sensor1ms[SENSOR_1].le = 0;
	sensor1ms[SENSOR_1].te = 0;
	
	sensor1ms[SENSOR_1].le = ((~sensor1ms[SENSOR_1].old2) & sensor1ms[SENSOR_1].old & sensor1ms[SENSOR_1].curr);
	sensor1ms[SENSOR_1].te = ( sensor1ms[SENSOR_1].old2 & (~sensor1ms[SENSOR_1].old) & (~sensor1ms[SENSOR_1].curr) );
	}

	// save data
	sensor1ms[SENSOR_1].old2 = sensor1ms[SENSOR_1].old;
	sensor1ms[SENSOR_1].old  = sensor1ms[SENSOR_1].curr;

}

//----------------------------------------------------------------------------
//  10ms마다 센서확정level, LE/TE를 검출
//	Diag IN Test : LVL_10MS
//

#define FREQ_CHANGE_BIT_MASK 0x5F1F	 //0xFF1F		// BbnEncA+B+BbnFG

static int lc10msCnt = 0;
//
//
//
void read_input_10ms()
{
	// 아래를 센서 Map개수 만큼 For문 돌릴 것. 현재는  Sensor Map 1개(16bit)만 있음!
	{
	// 1) Input Read
	sensor10ms[SENSOR_1].curr  = sensor1ms[SENSOR_1].lvl |
								(digitalRead(PWROFF_SWC_PIN) << SNS_PwrOffSwc) |	// Low Active
								(digitalRead(PC_USB5V_PIN) << SNS_usb5V) |
								(digitalRead(BOOT_MODE_PIN) << SNS_boot) |			// Low Active
								(digitalRead(SUB_ACOFF_PIN) << SNS_AcOff);			// Low Active(V108)
	if(DF_CONFIG_LMJIG == dfConfig)
	{
		sensor10ms[SENSOR_1].curr |= (digitalRead(LM_HOME_SEN_PIN) << SNS_LM_HOME) |
									 (digitalRead(LM_LEFT_SEN_PIN) << SNS_LM_LEFT) |
									 (digitalRead(LM_RIGHT_SEN_PIN) << SNS_LM_RIGHT) |
									 (digitalRead(LM_ENC_SEN_PIN) << SNS_LM_ENC) |
									 (digitalRead(LM_MOT_FG_PIN) << SNS_LMMOT_FG);
	}


	// 2) 연산
	sensor10ms[SENSOR_1].lvl = ( (sensor10ms[SENSOR_1].lvl & sensor10ms[SENSOR_1].old) | (sensor10ms[SENSOR_1].curr & (sensor10ms[SENSOR_1].lvl ^ sensor10ms[SENSOR_1].old)));		// (V108)

	// 3) LE/TE검출
	sensor10ms[SENSOR_1].le = 0;
	sensor10ms[SENSOR_1].te = 0;
	
	sensor10ms[SENSOR_1].le = ( (~sensor10ms[SENSOR_1].old2) & sensor10ms[SENSOR_1].old & sensor10ms[SENSOR_1].curr );
	sensor10ms[SENSOR_1].te = ( sensor10ms[SENSOR_1].old2 & (~sensor10ms[SENSOR_1].old) & (~sensor10ms[SENSOR_1].curr) );
	}


	// save data
	sensor10ms[SENSOR_1].old2 = sensor10ms[SENSOR_1].old;
	sensor10ms[SENSOR_1].old  = sensor10ms[SENSOR_1].curr;



}

void read_input_100ms()
{
	// TBD
}



//===========================================================
//		SETUP Part
//
//===========================================================

#define eepromLogPrintln	Resp2ApPrintln
//-----------------------------------------------------
//	eeprom Test
//-----------------------------------------------------
void eeprom_test()
{

	unsigned int reqlen = 6;
	/*
		int getTo = Wire.getTimeOut();
		eepromLogPrintln (" LG] NvmBF TO default:" + String(getTo) + " /ms");
		Wire.setTimeOut(50);		// default 1000ms => 50 ms
		getTo = eNvm.getTimeOut();
		eepromLogPrintln (" LG] NvmBF TO set:" + String(getTo) + " /ms");
	*/
		unsigned int len;
	
		byte readTempBuff[128];
		byte writeTempBuff[128];
	
		// 1) READ Offset:0 addr, 6 Length
		eNvm.readSend(0, reqlen);
		delay(100);
	
		len = eNvm.read(&readTempBuff[0], 0, reqlen);
		eNvm.grazingGeddong(&readTempBuff[0], 0, reqlen);
		delay(100);
	
		// 2) WRITE READ Val + 1,  6 Length
		writeTempBuff[0]=readTempBuff[0]+1;   
		writeTempBuff[1]=readTempBuff[1]+1;   
		writeTempBuff[2]=readTempBuff[2]+1;
		writeTempBuff[3]=readTempBuff[3]+1;
		writeTempBuff[4]=0x89;
		eNvm.write(&writeTempBuff[0], 0, 5);
		delay(100);

		// 3) reREAD Data, 6 Length
		eNvm.readSend(0, reqlen);
		delay(100);
		len = eNvm.read(&readTempBuff[0], 0, reqlen);
		eNvm.grazingGeddong(&readTempBuff[0], 0, reqlen);
		
	/*	
		rdLen = eNvm.rReadSend(10, 3);
		eepromLogPrintln(" LG] I2C__ ReadSend OK: " + String(rdLen));
		delay(100);
		
		bfLen = Wire.available();
		if(bfLen)
		{
			eNvm.buffSave(10,bfLen);		// addr=0, lenth = 10 byte
			eNvm.grazingGeddong(10, bfLen);
		}
		else
		{
			eepromLogPrintln(" LG] I2C__ RxBuffer Len: " + String(bfLen));
		}
	
	
	//	1) Buffer Write
		wrtLen = 20;
		int i=0;
		for(i=0; i<wrtLen; i++)
		{
			if(i%2) { eNvm.buff[i] = 0x55+i; }
			else	{ eNvm.buff[i] = 0xAA+i; }
		}
	
	
		bfLen = eNvm.write(0,wrtLen);
	
		rdLen = eNvm.currentRead(bfLen);
		delay(50);
		bfLen = Wire.available();
		if(bfLen)
		{
			eNvm.read(0,bfLen);   // addr=0, lenth = 10 byte
			eNvm.grazingGeddong(0, bfLen);
		}
		else
		{
			eepromLogPrintln(" LG] I2C__ RxBuffer bgLen: " + String(bfLen) +", wrtLen: "+String(wrtLen));
		}
	
		Wire.beginTransmission(0x50);
		Wire.write(0x55);
		Wire.endTransmission();
	
		eNvm.read(0,2);
		eNvm.grazingGeddong(0, 2);
	
		//eepromLogPrintln(" LG] I2C__ RxBuffer bgLen: " + String(bfLen) +", wrtLen: "+String(wrtLen));
	*/
	

}

//-----------------------------------------------------------
// PowerOn SUB_AC ON/OFF, by PWR_SW_STS
//-----------------------------------------------------------
void subAc_Control_atReset()
{
	LogPrintln(" LG] PWRAC SUB_AC On");
	
	if(LVL10_PWROFF_SWC)		// POWER SWC OFF?
	{
		//subAC_Off();

	// 24.11.7 Delete AC OFF at PWR_SWC_OFF
		//subAcOff_WithOtherOff();
	}
	else
	{
		subAC_On();
		motor_AllOff();		// MAIN ALL OFF
		ledOn_MbBtm();
		reelOut_AllOff();	// REEL ALL OFF
	}
}

//--- CONFIG Define
#define CONFIG_AD_MAX	4096	// ADC 12 BIT
#define CONFIG_AD_EA	2 // 8		// Dipsw 3개 = 8개 조합
#define CONFIG_AD_DIVS  (CONFIG_AD_MAX/CONFIG_AD_EA)
#define CONFIG_AD_GAP	(CONFIG_AD_DIVS/2)

#define CONF_READ_CNT	5
//-------------------------------------------------------------
//		CONFIG (Normal / LM-JIG) SET
//-------------------------------------------------------------
short setConfig()
{
	short i;
	short conf = 0;
	short rts;
	short confAvg = -1;
	int dummy;
	int confAd[CONF_READ_CNT];

	//==[ 1 ] INPUT - CONFIG [ ]보드 타입: 노멀(0) /LM보드(1) 구분] ,  only POWER ON
	pinMode(DF_CONFIG_PIN, INPUT_PULLDOWN);	// INPUT Check

// GPIO입력 (LOW/HIGH)
	//delay(100);	//OK
	delay(50);
	//delay(30);	//OK
	//delay(10);
	//delay(1);	// NG
	// Dummy, 3번
	for(i=0; i<CONF_READ_CNT; i++)
	{
		dummy= analogRead(DF_CONFIG_PIN);
		delay(10);
	}
	// READ
	for(i=0; i<CONF_READ_CNT; i++)
	{
		confAd[i] = analogRead(DF_CONFIG_PIN);
		conf += confAd[i];
		delay(10);
	}
	confAvg = conf/CONF_READ_CNT;

	//== [ 2 ] OUT : UART_RXD or LM_MOT_24VON
	
	//== [ 3 ] CONFIG 판단
	//curr_ms_tick = millis();
	if(0) {}
	else if( (CONFIG_AD_DIVS*(CONFIG_AD_EA-1)) + CONFIG_AD_GAP < conf) { rts = DF_CONFIG_LMJIG; }
	//else if( conf) { }
	//else if( conf) { }
	//else if( 0 + CONFIG_AD_GAP < conf) { rts = DF_CONFIG_STAND_ALONE; }
	else			{ rts = DF_CONFIG_STAND_ALONE; }	

	return rts;
}

//-----------------------------------------------------------------
//	PORT(PIN) SETTING
//-----------------------------------------------------------------
void ioLedOffSetting()
{
//
	// 중앙 LED
	pinMode(LED_CENT_R_PIN, OUTPUT);
	pinMode(LED_CENT_G_PIN, OUTPUT);
	pinMode(LED_CENT_B_PIN, OUTPUT);

	// 하단 LED
	pinMode(LED_BTM_PIN, OUTPUT);

	extLed.init();
	
	// 옵션 LED
		//pinMode(LED_OPT_L_PIN, OUTPUT);	// GPIO-1
		//pinMode(LED_OPT_R_PIN, OUTPUT);	// GPIO-2
  

}

void ioPinSetting()
{

	//=== 1) input
	//pinMode(, INPUT);

	//pinMode(BTMODE_PIN, INPUT);	// BOOT MODE PIN not PULL_UP
	


	pinMode(PC_USB5V_PIN, INPUT_PULLUP);
	pinMode(BLDC_FG_PIN, INPUT_PULLUP);
	//pinMode(PWROFF_SWC_PIN, INPUT_PULLUP);			// NG, LOW ACTIVE
	//pinMode(PWROFF_SWC_PIN, INPUT_PULLDOWN);		// NG, LOW ACTIVE
	pinMode(PWROFF_SWC_PIN, INPUT);			// LOW ACTIVE

	pinMode(BBN_ENC_A_CK_PIN, INPUT_PULLUP);
	pinMode(BBN_ENC_B_DT_PIN, INPUT_PULLUP);

	//pinMode(ILKDOOR_PIN, INPUT_PULLUP);				// 예비 - Interlock Door
	//pinMode(SEL_BOARD_PIN, INPUT_PULLUP);

	  
	// === 2) output
	//pinMode(, OUTPUT);

	// 0) - Power Control & WDT SET

	// TBD 	Boot(Download)시 WDT출력 안됨. Reeset됨!!!
	//	WDT

	// 보드타입(IN)
		
		// WDT(OUT)
		if(DF_CONFIG_LMJIG != dfConfig)
		{
			pinMode(WDT_OUT_PIN, OUTPUT);
			digitalWrite(WDT_OUT_PIN, HIGH);	//
			digitalWrite(WDT_OUT_PIN, LOW); 	//
			digitalWrite(WDT_OUT_PIN, HIGH);	//	미사용포트, 출력, HIGH유지
		}
	

	//	보드 LED
	pinMode(BD_LED2_PIN, OUTPUT);	  // SET Pin OUT
	
	pinMode(BD_LED3_PIN, OUTPUT);		// IO-40 / LM_MOT_ON
	pinMode(BD_LED4_PIN, OUTPUT);		// IO-41 / LM_MOT_DIR

	if(DF_CONFIG_LMJIG == dfConfig)
	{
		pinMode(LM_HOME_SEN_PIN, INPUT_PULLUP);
		pinMode(LM_LEFT_SEN_PIN, INPUT_PULLUP);
		pinMode(LM_RIGHT_SEN_PIN, INPUT_PULLUP);
		pinMode(LM_ENC_SEN_PIN, INPUT_PULLUP);
		pinMode(LM_MOT_FG_PIN, INPUT_PULLUP);
		pinMode(LM_24VON_PIN, OUTPUT);
		pinMode(LM_MOT_PWM_PIN, OUTPUT);
		pinMode(LM_MOT_ON_PIN, OUTPUT);
		pinMode(LM_MOT_DIR_PIN, OUTPUT);
	}

	// TORQ PORT
	pinMode(TORQ_SS, OUTPUT);				   // PIN SET to SS =OUT

	//pinMode(SERVOPWM_PIN, OUTPUT);

	// BLDC PORT
	pinMode(BBN_MOT_ON_PIN, OUTPUT);		// BLDC_ON_PIN
	pinMode(BBN_MOT_DIR_PIN, OUTPUT);
	pinMode(BBN_MOT_PWM_PIN, OUTPUT);
	
	pinMode(BBN_BLDC_24VON_PIN, OUTPUT);
	digitalWrite(BBN_BLDC_24VON_PIN, BBN_BLDC_24VOFF);	// BBN BLDC_24V_ON OFF

//
	// 중앙 LED
	pinMode(LED_CENT_R_PIN, OUTPUT);
	pinMode(LED_CENT_G_PIN, OUTPUT);
	pinMode(LED_CENT_B_PIN, OUTPUT);

	// 하단 LED
	pinMode(LED_BTM_PIN, OUTPUT);
	
	// 옵션 LED
		//pinMode(LED_OPT_L_PIN, OUTPUT);	// GPIO-1
		//pinMode(LED_OPT_R_PIN, OUTPUT);	// GPIO-2
  

}

//--------------------------------------------------
//	PID제어
//--------------------------------------------------


//--------------------------------------------------
//	PID제어 종료
//--------------------------------------------------

void wat_PidParaSet(String msg)
{
	// 시작전에 설정하므로, Mutex사용안 함.
	int iPara = msg.substring(5,(5+1)).toInt(); // 
	double dPara = msg.substring(6).toDouble();
	switch(iPara)
	{
		case 0: // View
			LogPrintln(" lg] PidPr:" + String(Kp)+","+String(Ki)+","+String(Kd));
			break;
		default:
			break;
	}

}

void wat_HookRandCheckExec()
{
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		bHookRandingCheckOnce = true;
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
}

void wat_PidStart_TargetSet(short _targetDist)
{
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		iTargetDist = (_targetDist);	// 목표거리로 설정
		pidExecFlag = 1;
		pidExecOutLog = 1;
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
}


void wat_PidStop()
{	
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		pidExecFlag = 0;
		iWireDistance = 0;		// INIT
		sMotorSpeed = 0;
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료

	int iOut;
	// 모터 멈춤
	
}



//------------------------------------------------------------------------------
//	1ms SYSTEM TIMER COUNT, FREE RUN : 0 ~ 0xFFFFFFFF
//------------------------------------------------------------------------------
void IRAM_ATTR ISR_onTimerHandler() {
  // 타이머 인터럽트 내에서 current_time 증가
  //portENTER_CRITICAL();  // 임계 영역 시작
  //lSys1MsTime++;        // current_time 값 증가
  ///portEXIT_CRITICAL();   // 임계 영역 종료

  portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작 (인터럽트 내에서)
  
	lSys100UsTime++;		// 100us FREE RUN
	
	sSys100UsCnt++;			// 100us CNT
	if((10-1) < sSys100UsCnt)
	{
  		sSys100UsCnt = 0;
		
		lSys1MsTime++;         // 1ms SYS TIME : FREE RUN 값 증가
	}
  portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
}


#define INTERVAL_2MS_1MS	(2 * 1)
#define INTERVAL_2MS_100US	(2 * 1000/100)
//-------------------------------------------------------------------------
// PID제어 : ENC TE 인터럽트 서비스 루틴 (ISR)
//
//
//-------------------------------------------------------------------------
void IRAM_ATTR ISR_encIntrHandle()
{
	unsigned long _lSys1MsTimer;
	// 현재 1ms 시간을 취득
	portENTER_CRITICAL_ISR(&mux);  // 임계 영역 시작
		// [1] - 속도 : 인터벌 측정, 
    	// GPIO 인터럽트 내에서 current_time 값 읽기
    	_lSys1MsTimer = lSys1MsTime;
			lCurrentTime = lSys100UsTime;	//100US current_time 값 읽기
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료
	//iEncIntrIntervalTime = lCurrentTime - lastInterruptTime;
	//lastInterruptTime = lCurrentTime;  // 마지막 시간을 현재 값으로 갱신

	portENTER_CRITICAL_ISR(&mux);
	// [1]인터벌 타임 [MS] 계산
	if (lCurrentTime < lLastTime) {  // currentTime이 lastTime보다 작으면 오버플로우 발생
	  iEncIntrIntervalTime = ((ULONG_MAX - lLastTime) + lCurrentTime + 1);
	} else {
	  iEncIntrIntervalTime = (lCurrentTime - lLastTime);  // 일반적인 시간 차이 계산
	}

//=== 신호에 문제 있음. TBD TBD =========
	//if(2 > iEncIntrIntervalTime)	// 2ms미만은 INT 무시
	if(INTERVAL_2MS_100US > iEncIntrIntervalTime)	// 2ms미만은 INT 무시
	{
		portEXIT_CRITICAL_ISR(&mux);
		return;		// 0ms, 1ms간격의 INT는 오동작(CPU특성) => INT무시
	}
	// 갱신은 누적해서 처리.
	lLastTime = lCurrentTime;  // 마지막 시간을 갱신

	//-- ENC 방향 판별
	// CW 	(TE & LOW)
	sIsrEncBdata = (digitalRead(BBN_ENC_B_DT_PIN)? 1:0);
	if(!sIsrEncBdata)
	{
		iIsrbbnEncCnt--;
		if(10 > iIsrbbnEncCnt) { iIsrbbnEncCnt = 10; }	// 신호문제 = 잠정 조치
		sIsrBbnEncDir = BBN_ENC_CW;
	}
	// CCW	(TE & HIGH)
	else
	{
		iIsrbbnEncCnt++;
		sIsrBbnEncDir = BBN_ENC_CCW;
	}
	
	// [2] 거리 계산
	//iWireDistance = (mainEnc.bbnCnt*BBN_ENC_1PULSE_DIST)/1000;
	if(0 > iIsrbbnEncCnt) { iIsrbbnEncCnt = 1; }	// 잠정조치
	iWireDistance = (iIsrbbnEncCnt*BBN_ENC_1PULSE_DIST)/1000;

	// [2]-2 : Data기록, 인터벌 타임 기록
	//stEncIntvBuff[sEncBuffIdx].tick = lCurrentTime;
	stEncIntvBuff[sEncBuffIdx].tick = _lSys1MsTimer;			// 1ms
	stEncIntvBuff[sEncBuffIdx].width = iEncIntrIntervalTime;	// 100us
	stEncIntvBuff[sEncBuffIdx].dir = sIsrBbnEncDir;
	stEncIntvBuff[sEncBuffIdx].enc = iIsrbbnEncCnt;
	stEncIntvBuff[sEncBuffIdx].dist = iWireDistance;
	sEncBuffIdx++;
	sEncBuffIdx = (sEncBuffIdx & (ENC_INTV_BUFF_SIZE-1));

	// INT=>TASK처리, 전개(Flag)
		bIsrEncInterruptFlag = true;
		iIsrEncInterruptCnt++;
	portEXIT_CRITICAL_ISR(&mux);   // 임계 영역 종료



// PID제어 실시 여부

}

// _pin : TORQ_SS
void spi_out(int _pin, int _val)
{
	digitalWrite(_pin, LOW);
	SPI.transfer(B00010001);  		// CMD [ 0001-write, 0001 - Exec On Potentio_0 ] SEND
	SPI.transfer(_val);       		// VAL Send
	digitalWrite(_pin, HIGH);
}

static unsigned int system_delay_cnt = 0;
/*------------------------------------------------------------------
   Create System Counter
     1ms * 10 => 10ms
     10ms *10 => 100ms
 Ca;; : Unlimit Loop
------------------------------------------------------------------*/
void system_counter()
{
	unsigned long add_tick = 0;

	// 임계 영역을 사용하여 current_time 값을 안전하게 읽음
	portENTER_CRITICAL(&mux);  // 임계 영역 시작 (loop 내에서)
		curr_ms_tick = lSys1MsTime;	   // current_time 값 읽기
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
	
	if(0)
	{
	}
/*
	else if(!isPowerOnReady)				// Once Only
	{
		if(999 < curr_ms_tick)
		{
			isPowerOnReady = true;
			//old_ms_tick = (curr_ms_tick + 0xFFFFF800); // test
			old_ms_tick = curr_ms_tick;
		}
	}
*/	
	//if(isPowerOnReady)
	else
	{
		// 1) count Up check
		//curr_ms_tick = (millis()+ 0xFFFFF800); // test
		if(curr_ms_tick != old_ms_tick)
		{
			if( curr_ms_tick > old_ms_tick )
			{
				add_tick = curr_ms_tick - old_ms_tick;
			}
			else
			{
				add_tick = (0xFFFFFFFF - old_ms_tick) + curr_ms_tick + 1;
			  /*
				Serial1.println();
				Serial1.println();
				Serial1.print("old:");
				Serial1.print(old_ms_tick, HEX);
				Serial1.print("curr:");
				Serial1.print(curr_ms_tick, HEX);
				Serial1.print("add:");
				Serial1.print(add_tick, HEX);
				Serial1.println();
				Serial1.println();
			  */
			}
			sys_count_1ms += add_tick;
			count_1ms += add_tick;
			
gotoSysTimer:
			if( 9 < count_1ms )		// 10ms?
			{
				count_1ms -= 10;
				
				sys_count_10ms++;
				count_10ms++;
				if( 9 < count_10ms)		// 100ms?
				{
					count_10ms -= 10;
					
					sys_count_100ms++;
					count_100ms++;
 					if( 9 < count_100ms) 	// 1 SEC?
					{
						count_100ms -= 10;
						
						sys_count_1sec++;
						count_1sec++;
						if( 9 < count_1sec) 	// 1 SEC?
						{
							count_1sec -= 10;
						}
 					}
						
					// 500ms , 5 SEC
					if(!(count_100ms % 5)) 	// 500ms? [ 0, 5 ]
					{
						sys_count_500ms++;

						count_500ms++;
						if( 9 < count_500ms )	// 500ms
						{
							count_500ms -= 10;
					
							sys_count_5sec++;
							//count_5sec++;
						}
					}
					//---
				}
			}

			if(9 < count_1ms)	//
			{
				goto gotoSysTimer;
			}
			
			//if(9 < add_tick)
			//if(10 < add_tick)
			//if(5 < add_tick)
			if(2 < add_tick)
			{
				// 10ms Over Delay
				system_delay_cnt++;
				LogPrintln(" LG] SYSTK MN_DLY:" + String(add_tick) + " ms,cnt:" + String(system_delay_cnt));
			}
		  
			old_ms_tick = curr_ms_tick;
		}
	}
}

//---------------------------------------------------------
// 메인보드 타입 검지
//	구보드(V1) - NC : Open(HIGH)
//	신보드(V1.1) - GND : LOW
//---------------------------------------------------------
int getBoardType()
{


	return MAIN_BOARD_V10;


}



/*------------------------------------------------------------------

   SETUP Arduino
------------------------------------------------------------------*/
/*------------------------------------------------------------------
   LOOP Arduino
------------------------------------------------------------------*/
