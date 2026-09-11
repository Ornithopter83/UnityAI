// Main application command routing implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_CommandRouter.h"
void ana_TestExecution(String msg)
{
	int act;
	int paraNG = 0;		// 0-OK
	String respMsg;
	String logMsg;
	static int nEncIdxSave;
	static int nBbnHallCount;
	int i;
	int hasEvent;
	int encoderCount;
	unsigned short encoderDirection;
	unsigned int wireDistance;
	unsigned long encoderInterval;
	encIsrDataBuff encoderEvent;
	unsigned int act2, act3;
	
	act = msg.substring(3,(3+1)).toInt();	// 1
	switch(act)
	{
		case 0:
			portENTER_CRITICAL(&mux);
			nEncIdxSave = sEncBuffIdx;
			nBbnHallCount = gBbnHallCount;
			portEXIT_CRITICAL(&mux);
			break;
			
		case 1:		// Enc INV LOG OUT
			for (i=0; ; i++) {
				portENTER_CRITICAL(&mux);
				hasEvent = (nEncIdxSave != sEncBuffIdx);
				if (hasEvent)
				{
					encoderEvent = stEncIntvBuff[nEncIdxSave];
					nEncIdxSave = (nEncIdxSave + 1) & (ENC_INTV_BUFF_SIZE-1);
				}
				portEXIT_CRITICAL(&mux);
				if (!hasEvent)
				{
					break;
				}
				LogPrintln(" lg] EnInt Interval:" +String(i)
					+ ","+ String(encoderEvent.tick)
					+ ","+ String(encoderEvent.width)
					+ ","+ String(encoderEvent.dir)
					+ ","+ String(encoderEvent.enc)
					+ ","+ String(encoderEvent.dist)
					);
			}
		//	LogPrintf("%10d LG] HallCnt : (%d) -> (%d) = %d\r\n", curr_ms_tick, nBbnHallCount, gBbnHallCount, gBbnHallCount - nBbnHallCount);	// TTTT
			LogPrintln(" LG] HallCnt: (" + String(nBbnHallCount) + ") -> (" + String(nBbnHallCount) + ") = " + String(gBbnHallCount - nBbnHallCount));
			break;
		case 3:		// 
			eNow.write(DF_Protocol_MainToRod_Alive, "");
			break;
		case 4:		// Disable
		//	SerialPortPinInput();
			SerialPortDisable();
			break;
		case 5:		// Enable
			SerialPortEnable();
			break;
		case 6:		// Disable
			subAC_Off();
			break;
		case 7:		// Enable
			subAC_On();
			break;

		case 9:		// boot info
			if (msg.substring(4,4+1) == "-") {
				fsInfo.deleteInfo(BOOTING_INFO);
			}
			else {
				fsInfo.saveInfo(BOOTING_INFO, msg.substring(4, 4+5));
			}
			break;
		case '8':		// Enable
		//	LogPrintln(" DN] ApCmd Recved len:" + String(g_SavePtr));
		//	HexDump(0, g_readBuf, g_SavePtr);
			g_SavePtr = 0;
			LogPrintf(" DN] ExecFlag(%d) RecvSeq(%d) g_DnExecStep(%d) RecvLen(%d) RecvIdx(%d)\r\n",
				g_DnExecFlag, g_DnRecvSeq, g_DnExecStep, g_DnRecvLen, g_DnRecvIdx);
		//	HexDump(0, g_DnSaveBuf, g_DnRecvIdx);
			break;
		default:
			paraNG = 1;
			break;
	}
	if (act < 2) {
		portENTER_CRITICAL(&mux);
		encoderCount = iIsrbbnEncCnt;
		encoderDirection = sIsrBbnEncDir;
		wireDistance = iWireDistance;
		encoderInterval = iEncIntrIntervalTime;
		portEXIT_CRITICAL(&mux);
		LogPrintln(" LG] Encoder: Cnt(" + String(encoderCount) + ") Dir(" + String(encoderDirection) + ") Dist(" + String(wireDistance) + ") Intv(" + String(encoderInterval) + ")");
	}
	
	// Response
	respMsg = msg.substring(0,(0+5)) + String(paraNG) + "%";
	Resp2ApPrintln(respMsg);
}

void analedContCmd(String msg)
{
	int lightReadFlag = 0;
	
	unsigned int pos;
	unsigned int act;
	unsigned int color;
	unsigned int time;

	unsigned int light;

  	pos = msg.substring(3,(3+1)).toInt();
	if(1> pos) { pos = 0; } if(4 < pos) { pos = 0; } // error	// if Pos=0, Start실행하지 않음.

  	act = msg.substring(4,(4+1)).toInt();
	if(0> act) { act = 10; } if(9 < act) { act = 10; } // error // NO ACTION

	switch(act)
	{
		case 0:		// off
			ledCont[pos].cont = act;
			
			ledCont_OffStart(pos);
			break;
			
		case 1:		// on
			ledCont[pos].cont = act;

			color = msg.substring(5,(5+2)).toInt();
			if(0> color) { color = 0; } if(14 < color) { color = 0; } // error
			ledCont[pos].colorNo = color;
			
			ledCont_OnStart(pos);
			break;
			
		case 2:		// blink
			ledCont[pos].cont = act; 

			color = msg.substring(5,(5+2)).toInt();
			if(0> color) { color = 0; } if(14 < color) { color = 0; } // error
			ledCont[pos].colorNo = color; 

			time = msg.substring(7,(7+4)).toInt();
			if(10> time) { time = 10; } if(9999 < time) { time = 9999; } // error
			ledCont[pos].bTime = time; 

			ledCont_BlinkStart(pos);
			break;
			
		case 3:		// dimm
			ledCont[pos].cont = act; 

			if(LED_IDX_BTM == pos)
			{
				ledCont[pos].colorMany = 1;		// FIX 1 Color = Blue (HardFix)
			}
			else
			{
				color = msg.substring(5,(5+2)).toInt();
				if(1> color) { color = 1; } if(14 < color) { color = 14; } // if Range Over,  Set Defuat(6) Color Many
				ledCont[pos].colorMany = color;	// IF SPEC :  52 -> 2, 56 -> 6, 57->7 
			}

			time = msg.substring(7,(7+4)).toInt();
			if(10> time) { time = 10; } if(9999 < time) { time = 9999; } // error
			ledCont[pos].dTime = time; 
			ledCont_DimmStart(pos);
			break;

		case 7:			// Color Oder, $06x7cccccccccccc%
			ledColorOrder[pos][1] = msg.substring(5,(5+1)).toInt();
			ledColorOrder[pos][2] = msg.substring(6,(6+1)).toInt();
			ledColorOrder[pos][3] = msg.substring(7,(7+1)).toInt();
			ledColorOrder[pos][4] = msg.substring(8,(8+1)).toInt();
			ledColorOrder[pos][5] = msg.substring(9,(9+1)).toInt();
			ledColorOrder[pos][6] = msg.substring(10,(10+1)).toInt();
			ledColorOrder[pos][7] = msg.substring(11,(11+1)).toInt();
			ledColorOrder[pos][8] = msg.substring(12,(12+1)).toInt();
			ledColorOrder[pos][9] = msg.substring(13,(13+1)).toInt();
			//ledColorOrder[pos][10] = msg.substring(14,(14+1)).toInt();
			break;
			
		case 8:			// read
			lightReadFlag=1;
			break;
		
		case 9:			// write
			light = msg.substring(5).toInt();
			if(  10 > light) { light = 10; }
			if(  100 < light) { light = 100; }
			ledLight = light;			// Light % 2 char 99 => 100
			lightReadFlag=1;
			break;
			
		default:
			//pos = 0;	// NO ATION ( Include Act 4,5,6,7)
			break;
	}

	if(lightReadFlag)
	{
		lightReadFlag = 0;
		light = ledLight;
		if(  10 > light) { light = 10; }
		if ( 100 < light) { light = 100; }
		Resp2ApPrintln("$060" + String(act) + String(light)+"%");
	}
	else
	{
	}
	
}


//
//
//
void anaRanding(String msg)
{

  	randingPtn = msg.substring(5,(5+2)).toInt();
	exeFight = 0;			// Fight UpDown Control STOP
	
	execRanding_SetVal();
	
	exeRanding = 1;
	execRanding_Start();
	gameStatus = GAME_RANDING;
	
	LogPrintln(" LG] GameS RANDING");	// LOG-SERIAL1
}

//
//
//
void anaFastHold(String msg)
{
	unsigned int iDir = msg.substring(5,(5+2)).toInt();
	unsigned int iStartStop = msg.substring(7,(7+2)).toInt();
	// TBD

	LogPrintln(" LG] FastH Dir/stop: " + String(iDir) +"," + String(iStartStop));
}

//
//
//
void anaFastComboBtn(String msg)
{
	unsigned int iStartStop = msg.substring(5,(5+2)).toInt();
	// TBD
	
	LogPrintln(" LG] FastB COMBO StartStop: " + String(iStartStop));
}


//volatile uint uIntTest = 0;
//void sendRodVrtCmd(uint_t cnt, uint_t ontime, uint_t offtime)
void sendRodVrtCmd(int act, unsigned int cnt, unsigned int ontime, unsigned int offtime)
{
	String msg = "0";
	int rts = 0;		// 0-OK, 1-NG
	//if( 5 < act) { act = 0; }	//

	switch(act)
	{
		case 0:
		case 1:
			msg = String(act);
			break;
		
		case 2:
		case 3:
			rts = 1;
			break;		// 처리 안함
		
		case 4:
			msg = String(act);
			if(99 < cnt){ cnt = 1;}	// 기본 1회
			if(9 < cnt) { msg += String(cnt); }			// 2CHAR
			else		{ msg += "0" + String(cnt); }

			if(999 < ontime) { ontime = 100;}
			msg += fillZero2String3Char(ontime);
			if(999 < offtime) { offtime = 100;}
			msg += fillZero2String3Char(offtime);
			break;
			
		default:
			rts = 1;
			break;		// 처리안함
	}

	// error판별 처리
	if(rts)	// error
	{
		// error
		LogPrintln(" lg] VrtMot CMD Error");
	}
	else
	{
		eNow.write(DF_Protocol_MainToRod_VerticalMotor,msg);
	}
	
}

//
// send ROD BTN LED CMD , 27,xAccNNNFFF
//
void sendRodBtnLedCmd(int posi, int act, unsigned int cnt, unsigned int ontime, unsigned int offtime)
{
	String msg = "00";
	int rts = 0;		// 0-OK, 1-NG

	if(2 < posi)	// 0,1,2
	{ 
		LogPrintln(" lg] BtnLED POSI NG:" + String(posi));
		return; 
	}	//처리  중지

	msg = String(posi);
	msg += String(act);
	//if( 5 < act) { act = 0; }	// OFF

	switch(act)
	{
		case 0:
		case 1:
			//msg = String(act);
			break;
		
		case 2:
		case 3:
			rts = 1;
			break;		// 처리 안함
		
		case 4:
			if(99 < cnt){ cnt = 5;}			// 기본 5 번
			if(9 < cnt) { msg += String(cnt); }			// 2CHAR
			else		{ msg += "0" + String(cnt); }

			if(999 < ontime) { ontime = 100;}
			msg += fillZero2String3Char(ontime);
			if(999 < offtime) { offtime= 100;}
			msg += fillZero2String3Char(offtime);
			break;

		default:
			rts = 1;
			break;		// 처리안함
	}

	// error판별 처리
	if(rts)	// Error
	{
		// error
		LogPrintln(" lg] BtnLED CMD Error");
	}
	else
	{
		eNow.write(DF_Protocol_MainToRod_ButtonLed,msg);
	}
	
}

// GAME STATUS DEFINE
#define GMWAT_MIN	GMWAT_AP_INFO	// 주의 , MIN
//--------------------------------------------------------------
#define GMWAT_AP_INFO	1
#define GMWAT_PAY		3
#define GMWAT_WIREHOME	4
#define GMWAT_SELECT	5
#define GMWAT_POINTMOVE	6
#define GMWAT_WAVE		7
#define GMWAT_BITE		8
#define GMWAT_HOOK		9
#define GMWAT_HIT		10
#define GMWAT_FAIL		11
#define GMWAT_FIGHT		12
#define GMWAT_HOLDON	13
#define GMWAT_HARDACT	14
#define GMWAT_RANDING	15
#define GMWAT_SUCCESS	16
#define GMWAT_POINTUP	17
#define GMWAT_TUTORIAL	18
#define GMWAT_BONUS		19
#define GMWAT_STAGECLR	20
#define GMWAT_MALL		21
#define GMWAT_CONTINUE	22
#define GMWAT_OVER		23
//--------------------------------------------------------------
#define GMWAT_MAX	(1+GMWAT_OVER)	// 주의 MAX :  STATUS추가시 MAX변경 필요.
//--------------------------------------------------------------

// LM FISH LEVEL SET, by AP FISH_PWR
short lmFish_3Level_Set(short pwr)
{
	short level;
	if(0) {}
	else if(66 < pwr)	{level = FISH_LVL_3; }
	else if(33 < pwr)	{level = FISH_LVL_2; }
	else				{level = FISH_LVL_1; }

	return level;
}

// 실페시 CALL
void lmReturnOrHome_Start()
{
	if(0) {}
	else if(LM_POSI_RIGHT <= lmPosi)	// Right구동중
	{	
		lmRight_Control_Return();
	}
	else if(LM_POSI_LEFT <= lmPosi)		// Left구동중
	{
		lmLeft_Control_Return();
	}
	else	// Home Control
	{
		lmHome_Control_Start(0, reqTorqueMotor);	//
	}	
}

static volatile short wireHome_Step=0;
static volatile short wireHomeExe_Flag=0;


void wireHomeControl_Start()
{
	wireHomeExe_Flag=1;
	wireHome_Step=STEP_START;
}
void wireHomeControl_Stop()
{
	wireHomeExe_Flag=0;
	wireHome_Step=STEP_IDLE;
}

// TBD-WIREHOME
void wireHome_Control()
{
}

// Wire HOME 거리(3미터), 카운트(286카운트) 설정
void setWireHome_EncCnt()
{
	mainEnc.setBbnCnt(WIRE_HOME_BBN_CNT);
	
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		iIsrbbnEncCnt = WIRE_HOME_BBN_CNT;
		iWireDistance = WIRE_HOME_DIST;
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료
	
}


//byte nvm_fish_level = FISH_LVL_AUTO;
//byte nvm_fish_level = FISH_LVL_2;
byte nvm_fish_level = FISH_LVL_AUTO;

/*---------------------------------------------------------------------------
	Analysis RECV GAME STATUS WTA( Wire Auto Tension )
---------------------------------------------------------------------------*/
void anaGameStatusWAT(String msg)
{
	// Control 
	short cmdNG = 0;	// 0-OK
	short paraNG = 0;	// 0-OK
	
	short act;
	short dir;
	short para;

	short para1 = 0;
	short para2 = 0;
	short para3 = 0;

	short fishType = 0;
	short ptn = 0;	// "023" - Deeop & TokTok
	
	short ptn1 = 0;
	short ptn2 = 0;
	short ptn3 = 0;

	String respMsg;
	String logMsg;

	short cmd = msg.substring(3,5).toInt();
	portENTER_CRITICAL(&mux);  // 임계 영역 시작
		gmWatStatus = cmd;	// STATUS SAVE
	portEXIT_CRITICAL(&mux);   // 임계 영역 종료

	//--- CMD분석
	switch(cmd)
	{

		// (02)
		case GMWAT_STBY:

			// 장치에러 검지]중에는 종료후 OFF제어 실시를 예약한다.
			if(devReadychkFlag)
			{
				// 중지 & 예약
				stbyTorqOffControl_Flag = 0;
				stbyTorqOffControl_Stop(0);			// (V108) 토크모터 동작 안함.
				stbyTorqOffControl_Req_Flag = 1;	// 예약
			}
			// 대기중 [토크OFF제어] 중이 아니면 제어를 시작한다. 
			else if(!stbyTorqOffControl_Flag)	// TBD : 기능 무효(0)/유효(1) 설정 추가?
			{
				stbyTorqOffControl_Start();
			}
			sendSleepEnable(SS_WAIT, TIME_SLEEP_LIGHT, 2);

			break;
		
		//(03)
		case GMWAT_PAY:
			gRodSleepMode = SS_NONE;
			break;
		//(04)
		case GMWAT_WIREHOME:	// WIRE HOME(04)
			gRodSleepMode = SS_NONE;
			setWireHome_EncCnt();
			break;
		//(05)
		case GMWAT_SELECT:
			break;
		//(06)
		case GMWAT_POINTMOVE:	// 포인트 이동 화면
			gRodSleepMode = SS_NONE;

		  
			break;
		//(07)
		case GMWAT_WAVE:
			break;
			
		//==($2508xxx%)  입질 물고기 Power(0~100)
		case GMWAT_BITE:
			para1 = msg.substring(5).toInt();
			if(100 < para1) { para1 = 100; }
			break;

		// (09)
		case GMWAT_HOOK:
			break;
		//(10)
		case GMWAT_HIT:
			break;
		//(11)
		case GMWAT_FAIL:	// FAIL - LM RETURN
			//1) LM_RETURN
			break;

		//(12)
		case GMWAT_FIGHT:
			break;
	
		//== ( $2513xy% ) x-Dir, Y-Act
		case GMWAT_HOLDON :		//HOLD ON START/STOP
		//(14)
		case GMWAT_HARDACT:
			break;

		//(15)
		case GMWAT_RANDING:
			break;
		//(16)
		case GMWAT_SUCCESS:
			break;
		// (17)
		case GMWAT_POINTUP:
			break;
		// (18)
		case GMWAT_TUTORIAL:
			break;
		// (19)
		case GMWAT_BONUS:
			break;
		// (20)
		case GMWAT_STAGECLR:
			break;
		// (21)
		case GMWAT_MALL:
			break;
		// (22)
		case GMWAT_CONTINUE:
			break;
		// (23)
		case GMWAT_OVER:
			break;

		default:
			break;
	}

	// 상태변화시
	if(oldGmWatStatus != gmWatStatus)
	{
		if(GMWAT_STBY == oldGmWatStatus)
		{
			// 1) 대기상태 -> 다른상태로 이동, 대기중 토크OFF중지
			stbyTorOffStop_Check();
		}

		// END) 상태 갱신
		oldGmWatStatus = gmWatStatus;
	}

	//--- GAME WAT STATUS SAVE
	if(0 < gmWatStatus && GMWAT_MAX > gmWatStatus)
	{
		respMsg = STX_GAME_STS_WAT + msg.substring(3,5) + "00%";	// 0-OK
	}
	else
	{
		// Error, NOT DEFINE STATUS
		respMsg = STX_GAME_STS_WAT + msg.substring(3,5) + "-1%";	// 1-NG
	}
	//--- RESP
	Resp2ApPrintln(respMsg);
	
}


//-------------------------------------------------------------------------------------------------------
//  대기중 외에는 OFF제어를 중지한다.
void stbyTorOffStop_Check()
{
	// 상태가 대기중 외의 경우, 무조건 중지
	if(GMWAT_STBY != gmWatStatus)
	{
		// 예약 플래그 클리어
		stbyTorqOffControl_Req_Flag = 0;
		// 제어 플래그 및 상태 클리어 & 모터 OFF
		stbyTorqOffControl_Stop(1);			// (V108) 토크모터 기본값 출력
	}

}

//-------------------------------------------------------------------------------------------------------
#define FIGHT_PTN_CW_CCW	1

/*---------------------------------------------------------------------------
	Analysis RECV GAME STATUS
---------------------------------------------------------------------------*/
void anaGameStatus(String msg)
{
	// Control 
	int msgKind = msg.substring(3,5).toInt();
	int msgOption = msg.substring(5,7).toInt();
	int para = msg.substring(7,9).toInt();

	int para1 = 0;
	int para2 = 0;
	int para3 = 0;

	int fishType = 0;
	int ptn = 0;	// "023" - Deeop & TokTok
	
	int ptn1 = 0;
	int ptn2 = 0;
	int ptn3 = 0;

	switch(msgKind)
	{
		case 1 :		// PROGRAM Start
			//torqMotor.setValue(torqMotor.defaultVal);
			//motor_AllDefault();						// (V108) delete 토크 모터 기본값 출력 생략
			motor_AllOff();								// (V108) replace 모든 모터 off 시킴

			// SEND STATUS CNT CLR
			recvMainConn_Pwr1stCnt = 0;

			gameStatus = PROG_START;

			// Delete Send Current to MAIN
			
			//sendCurrentStatus();
			//recvMainConn_Pwr1stCnt++;

			// Send to ROD REQ_ROD_INFO
			eNow.write(DF_Protocol_MainToRod_RodInfoRequest, "");		// 	Send TO ROD

				// STBY [토크OFF제어] 중이 아니면 OFF제어를 실시한다.
				// 삭제 -불필요(게임실행후 반드시 대기상태 옴)
				//if(!stbyTorqOffControl_Flag)
				//{
				//	stbyTorqOffControl_Start();
				//}
			if (gRodSleepMode == SS_EXIT) {
				gRodSleepMode = SS_WAIT;
			}
			LogPrintln(" LG] GameS PROG Start ");
			
 			break;

		// Program END
		case 99:	// Program End

			isGameEnable = false;				// MAIN DIS

			g_RecvCmdCount = 0;					// (V108)
//			g_FlagCmdStop  = 1;					// (V108)

			execHit_FightBldcStop();
			motor_AllOff();

			// TBD, AP수정 필요(통신처리 소용시간 대기후 프로그램 종료 필요) . IMU STOP SEND
			//SetIMU_Measure_Out_OnOff(IMU_DATA_OFF);			// IMU OUT = 0
			reelOut_AllOff();

			// SEND STATUS CNT CLR
			recvMainConn_Pwr1stCnt = 0;

			gameStatus = PROG_END;
			exeProgramEnd = 1; // Flag Clear

			sendSleepEnable(SS_EXIT, TIME_SLEEP_DEEP, 3);
			LogPrintln(" LG] GameS ProgEND");   // LOG-SERIAL1

		// 릴 진동모터, 버튼LED 제어
			//rodReelVrtControl_Start(3, 500, 100); //무의미, Main이 RESET되므로 실행 안됨.
 			break;

		case 53 :		// NEW Bite Pattern For FW BITE Control
			// $1153Kxyz%

			fishType = msg.substring(5,(5+1)).toInt();	// Fish Kind 1~ 6
			if(1 > fishType ) {fishType = 1; }
			if(6 < fishType ) {fishType = 6; }
		
			ptn = msg.substring(6,(6+3)).toInt();	// PTN2 = xyz
			// 1) xyz = "000"
			if(0 == ptn)
			{
				//	// NA
			}
			// 2) xyz = "999" Bite Stop
			else if(999 == ptn)	// Bite STOP
			{
				exeBite = 0;
				bitePtn_Flag = 0;
				execBite_ptn_Stop();

				//임시,  VRT Mot 1초 On/Off, 회수
				//rodReelVrtControl_Start(12, 50, 30);		// FIX
				sendRodVrtCmd(4, 10, 50, 50);
 			}
			
			// 2) else : ex) xyz = 123, 
			else	// BITE CONTROL REQ
			{
			
				fishTypeAuto = fishType;
		
				ptn1= msg.substring(6,(6+1)).toInt();
				ptn2= msg.substring(7,(7+1)).toInt();
				ptn3= msg.substring(8,(8+1)).toInt();
		
				if(0 != ptn1) { exeBitePtn1flag = 1; execBite_ptn1_Start(); }
				if(0 != ptn2) { exeBitePtn2flag = 1; execBite_ptn2_Start(); }
				if(0 != ptn3) { exeBitePtn3flag = 1; execBite_ptn3_Start(); }
		
				exeBite = 1;
				bitePtn_Flag = 1;

				//임시,  VRT Mot 1초 On/Off, 회수
				// 12회 제어하지만 모터특성(?)으로 6회 체감됨
				//rodReelVrtControl_Start(12, 50, 30);	// FIX
				sendRodVrtCmd(4, 10, 50, 50);
 			}
				

			break;

		case 84 :		// NEW HoldOn IF for FW Control
			// $1184KP%

			fishType = msg.substring(5,(5+1)).toInt();	// Fish Kind 1~ 6
			if(1 > fishType ) {fishType = 1; }
			if(6 < fishType ) {fishType = 6; }
		
			//ptn = msg.substring(6,(6+1)).toInt();	// PTN
			ptn = msg.substring(6).toInt();	// PTN
			if(0 == ptn)
			{
				//	// NA
			}
			// 2) else : 1=Exec, 2~9: Reserved
			else if(1 == ptn)
			{
			
				fishTypeAuto = fishType;
		
				if(0 != ptn) { execHoldon_ptn1_Start(); }
				//exeHoldon = 1;			// Old Flag
			}
				

			break;		
			
//=============================================================
// 이하는 AP_MOT_CONT시 무효함.
			
	// NA

		default:
			break;
	}


	if(oldgameStatus != gameStatus)
	{
		// LOG
		//LogPrintln(" AP] GameS Change: " + msg + ",Kind: " + String(msgKind) + ", opt:" + String(msgOption) + ", STS: " + String(gameStatus) + ", old: " + String(oldgameStatus));	// LOG-SERIAL1
		LogPrintln(" AP] GameS chg: " + msg);

		oldgameStatus = gameStatus;
	}

}


//-------------------------------------------------
//
//	Ana INFO REQ & RESP INFO (ex IF VER)
//
//	$01kk%
//-------------------------------------------------
void anaDeviceInfoReq(String msg)
{


	String respMsg = "";

	String strKind = msg.substring(3,(3+2));
	//String strPara = msg.substring(5,(5+2));

	int iKind = strKind.toInt();
	
	switch(iKind)
	{
		// IF VER
		case NUM_MAIN_IF_VER:
			respMsg = STX_INFO_REQ + MAIN_IF_VER + IfVer;
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq IF_Ver: " + IfVer);
			break;

		//BOARD TYPE
		case NUM_ROD_BOARD_TYPE:
			respMsg = STX_INFO_REQ + ROD_BOARD_TYPE;
			respMsg += strRodBoardType;		// Old/New BOARD
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq ROD Board Type: " + strRodBoardType);
			break;

		// ROD Regist Wait T/O SEC (Deafult:25SEC)
		case NUM_ROD_REGIST_TO_SEC:
			respMsg = STX_INFO_REQ + ROD_REGIST_TO_SEC;
			respMsg += String(ROD_REGI_CANCEL_TO/1000);	
			respMsg += "%";
			Resp2ApPrintln(respMsg);
			
			LogPrintln(" LG] InfRq ROD Regist Wait T/O Sec: " + respMsg);
			break;

		// IMU 간격
		case NUM_IMU_INTVAL_TIME:		// IMU INTERVAL[ms]
			//if(AP_IS_TM == apType)
			{
				respMsg = STX_INFO_REQ + IMU_INTVAL_TIME + strImuInterval + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ImuInterval[ms]: " + respMsg);
			}
			break;

		// MANUFATORY INFO
		case NUM_MANUF_INFO_READ:		// 4
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(MANUF_INFO);
				respMsg = STX_INFO_REQ + MANUF_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Read[model,SN,Date]: " + respMsg);
			}
			break;

		case NUM_MANUF_INFO_SAVE:		// 5
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(MANUF_INFO, msg.substring(5));
				respMsg = STX_INFO_REQ + MANUF_INFO_SAVE + String((0 < result)? 1:0) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Save[model,SN,Date]:" + respMsg + ", len:" + String(result));
			}
			break;

		case NUM_MANUF_INFO_DEL:		// 6
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.deleteInfo(MANUF_INFO);
				respMsg = STX_INFO_REQ + MANUF_INFO_DEL + String(result) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq ManufInfo Del[model,SN,Date]: " + respMsg);
			}
			break;

		// FIELD INSTALL  INFO
		case NUM_INSTALL_INFO_READ:		// 7
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(INSTALL_INFO);
				respMsg = STX_INFO_REQ + INSTALL_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Read[Date]: " + respMsg);
			}
			break;

		case NUM_INSTALL_INFO_SAVE:		// 8
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(INSTALL_INFO, msg.substring(5));
				respMsg = STX_INFO_REQ + INSTALL_INFO_SAVE + String((0 < result)? 1:0) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Save[Date]:" + respMsg + ", len:" + String(result));
			}
			break;

		case NUM_INSTALL_INFO_DEL:		// 9
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.deleteInfo(INSTALL_INFO);
				respMsg = STX_INFO_REQ + INSTALL_INFO_DEL + String(result) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

				LogPrintln(" LG] InfRq FieldInfo Del[Date]: " + respMsg);
			}
			break;

		// FIELD INSTALL  INFO
		case NUM_MAINMOT_INFO_READ:		// 10
			//if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(MAINMOT_INFO);
				respMsg = STX_INFO_REQ + MAINMOT_INFO_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

			}
			break;

		case NUM_MAINMOT_INFO_SAVE:		// 11
			//if(AP_IS_TM == apType)
			{
				short result = fsInfo.saveInfo(MAINMOT_INFO, msg.substring(5));
				// 파일저장 성공시, 값 갱신
				if(0 < result)	// write길이
				{
					defaultTorqueMotor = msg.substring(5).toInt();
				}
				
				respMsg = STX_INFO_REQ + MAINMOT_INFO_SAVE + ((0 < result)? String(defaultTorqueMotor): String(-1)) + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

			}
			break;

		case NUM_MAIN_BOARD_TYPE:		// 12
			// Main보드타입 송신(무조건)
			respMsg = STX_INFO_REQ + MAIN_BOARD_TYPE + strMainBoard[mainBoardType]+ "%";
			Resp2ApPrintln(respMsg);
			
			break;


		//-- BLDC LIMIT READ
		case NUM_BLDC_MOT_LIMIT_READ:		// 13
			if(AP_IS_TM == apType)
			{
				respMsg = fsInfo.getInfo(BLDC_LIMIT_INFO);
				//bldcLimitVal = respMsg.toInt();
				respMsg = STX_INFO_REQ + BLDC_MOT_LIMIT_READ + respMsg + "%";	// 0000-Not Measure
				Resp2ApPrintln(respMsg);

			}
			break;

		case NUM_BLDC_MOT_LIMIT_WRITE:		// 14,  $0114xxx%
			if(AP_IS_TM == apType)
			{
				short result;
				short val = msg.substring(5).toInt();
				//범외 밖 = NG, 저장안함
				if( (BLDC_LIMIT_MIN > val) || (BLDC_LIMIT_MAX < val) )
				//if( (BLDC_LIMIT_MIN > val) || (BLDC_LIMIT_MAX+1 < val) )	// TEST
				{
					result = ERR_PARA_NG;	// RANGE OVER
				}
				// 범위 내 = OK, 저장
				else
				{
				
					result= fsInfo.saveInfo(BLDC_LIMIT_INFO, String(val));
					// 파일저장 성공시, 값 갱신
					if(0 < result)	// write길이
					{
						bldcLimitVal = val;
					}
				}
				respMsg = STX_INFO_REQ + BLDC_MOT_LIMIT_WRITE + ((0 < result)? String(bldcLimitVal): String(result)) + "%";	//100 ~ 255
				Resp2ApPrintln(respMsg);

			}
			break;


		//--- CMD_PARA NG			
		default:
			Resp2ApPrintln(STX_INFO_REQ + strKind + "1%");	// NG Response
			break;
	}


}


/*----------------------------
	($01)GAME_ENB, GAME_WAIT
----------------------------*/
void anaGameEnbOver(String msg)
{
		 // test LOG
	 
	   // 3-1) Game STBY - Title Display( Creadit Waiting)
	   if (msg.endsWith(STATE_WAITING))		// Game Waiting ( 1st Title or Game Over )
	   {
		   isGameEnable = false;			   // MAIN DIS
		   //isGameStby = true;			   // GAME STBY
		   //motor_AllDefault();		// TBD HUD Follow
		   if(DIAG_MODE != mainMode)
		   {
		   		digitalWrite(BD_LED3_PIN, LOW);    // LED3 OFF
		   }
			gameStatus = GAME_WAIT;
			execGameOver();
			LogPrintln(" AP] GamWT " + msg + ", o/c " + String(oldgameStatus) + "," + String(gameStatus));	  // GAME WAIT
			if(oldgameStatus != gameStatus)
			{
			   oldgameStatus = gameStatus;
	   		}

	   }
	   // 3-2) Game ENB - Creadit OK
	   else if (msg.endsWith(STATE_ENABLE))	// GAME ENB
	   {
		 //LogPrintln(" AP] GamEN " + msg);	   // LOG-SERIAL1
		 LogPrintln(" AP] GamEN " + msg + ", o/c:" + String(oldgameStatus) + "," + String(gameStatus)); 	 // GAME WAIT
		 
		 isGameEnable = true;				 // GAME ENB
		 //isGameStby = false;			 // GAME STBY
		   //motor_AllDefault();		// TBD HUD Follow
		 //torqMotor.setValue(65);	// TBD
		 torqDefault();
	   }
	   else
	   {
	   		// Error
	   }

}

#define AP_ALIVE_NOTRECV_TO	10		// 10 SEC
//---------------------------------------------------------------
// AP Alive Recved Check = [ AP-MAIN] Connection Check
//	DF프로그램 - FW간 통신 끊김 검지
//--------------------------------------------------------------
//	CALL 1 SEC
//--------------------------------------------------------------
void apAlive_Check()
{
	if( CONNECT == ap_conn_status)			// CONNECTED Only
	{
		if( AP_ALIVE_NOTRECV_TO > apAliveConn_NotRecvCnt)	// 10 SEC
		{
			apAliveConn_NotRecvCnt++;
			if (gRodSleepMode == SS_EXIT) {
				if (gmWatStatus == 2) {
					sendSleepEnable(SS_WAIT, TIME_SLEEP_LIGHT, 0);
				}
				else {
					sendSleepEnable(SS_NONE, 0, 0);
				}
			}
		}
		else
		{
			// 통신장애로  Alive Check명령이 오지 않으면
			// 모터 OFF, IMU출력 정지로 처리함.
			ap_conn_status = DISCONNECT;	// NOT CONNECTED, exec Once Only
			sendSleepEnable(SS_EXIT, TIME_SLEEP_DEEP, 1);
			motor_AllOff();

			reelOut_AllOff();


		// 불필요 에러코드 통지 삭제, 릴등록시 통신끊김으로 통지됨.
		  	// AP에 장애출력(무의미)
		// 	Resp2ApPrintln("$159999%");		// AP 폴링 안함!!!
		//	LogPrintln(" LG] APCON AP Alive NOT RECV 10 sec");

			
		}
	}

	// UNKNOWN (POWER ON)
	else if( UNKNOWN == ap_conn_status)
	{
	
	
	}
	else
	{
		// NA
	}
}

//
//
//	Call 1SEC
void send2RodAlive_CntCheck()
{
	int	CheckTime;

	if (rodAlive_SendCnt == 0) {
		if (gRodSleepMode == SS_NONE) {
			gRodSleepTime = 0;
		}
	}
	if (gRodSleepStat) {
		CheckTime = gRodSleepTime + 4;
	}
	else {
		// 1)무조건 Send , for Auto Recovery
//		eNow.write(DF_Protocol_MainToRod_Alive, "");
		eNow.write(DF_Protocol_MainToRod_Alive, String(gRodSleepMode));
		CheckTime = ROD_RESP_OVER_CNT;
	}

	// 2) Send CNT OVER Check
	if(CheckTime > rodAlive_SendCnt)	// Check Time OVER?
	{
		// NOT Over, Count UP
		rodAlive_SendCnt++;
	}
	else
	{
		// SET SLAVE DIS_CONN
		rod_conn_status = (ROD_DISCONN);
	}
}

//---------------------------------------------
// 낚시대(REEL) 교체 CONTROL
//---------------------------------------------

static unsigned int rodRegistToStep = 0;	// TO control Step

//TO정지
void rodRegistToStop()
{
	rodRegistToStep = 0;	// IDLE SET
}

// TO 확인 개시
void rodRegistToStart()
{
	rodRegistToStep = 10;	// 10SEC T/O Start	
}

void rodRegi3secToStart()
{
	rodRegistToStep = 30;	// 3SEC T/O  Start
}

//
// CALL 100ms ,  (20초 경과) / (3초 경과) Check Control
//
void rodRegistToControl()
{
	static unsigned int oldStep = 0;
	static unsigned long _to = 0;

	switch(rodRegistToStep)
	{
		case 0:
			break;
			
		case 10:	//Start
			setTO(_to);
			rodRegistToStep = 11;
			break;		
		case 11:
			if(checkTO(_to, ROD_REGI_CANCEL_TO))
			{
				// Rod Regist Cancel
				rodRegistCancel();		// MODE CLR
				rodRegistToStep = 0;		// to IDLE
				StsSendPrintln(STX_ROD_REGIST + ROD_REGI_FW_CANCEL); // AP에 송부(FW_등록취소: 20초 T/O)
			}
			break;

		case 30:	//Start
			rodRegistToStep = 31;
			setTO(_to);
			break;		
		case 31:
			if(checkTO(_to, ROD_REGI_END_TO_1ST))	// 1.5 SEC
			{
				StsSendPrintln(STX_ROD_REGIST+ ROD_REGI_FINISH); // AP에 송부(FW_등록완료: 3초 T/O)
				rodRegistToStep = 32;		// to IDLE
				setTO(_to);
			}
			break;
		case 32:
			if(checkTO(_to, ROD_REGI_END_TO_2ND))	// 3초
			{
				rodRegistToStep = 0;		// to IDLE
				rodRegistCancel();
				//ESP.restart();				// ESP Restart, rst:0xc (RTC_SW_CPU_RST),boot:0x28 (SPI_FAST_FLASH_BOOT)
				//esp_restart();			// Soft Reset, rst:0xc (RTC_SW_CPU_RST),boot:0x28 (SPI_FAST_FLASH_BOOT)
				esp_soft_reset();
				//esp_hard_reset();		// NOT IMP
			}
			break;
			
		default:
			break;
	}
}

// 등록 취소, Mode CLR
void rodRegistCancel()
{
	rodRegistMode = 0;
}

// 낚시대 등록 명령 분석 처리(진입, 취소)
void anaRodRegist(String msg)
{
	String respMsg;
	int Act = msg.substring(3).toInt();
	switch(Act)
	{
		case ROD_REGI_AP_ENTRY:		// 진입
			rodRegistMode = 1;
			LogPrintln(" LG] RodRg ApMsg:" + String(Act));
			// 진입처리
			// 1) 10초 Time Set
			rodRegistToStart();
			break;
		case ROD_REGI_AP_CANCEL:		// AP취소, FW자체 취소도 있음
			if(1 == rodRegistMode)		// 등록개시(2)이후는 Cancel 무시
			{
				rodRegistToStop();		// TO 취소
				rodRegistCancel();		// Mode CLR
				LogPrintln(" LG] RodRg ApMsg:" + String(Act));
				// 취소 처리
			}
			break;
		default:
			break;
	}
		
}

// Rod Regist EXEC
void rodRegistExec(String msg)
{
	  rodRegistMode = 2;	  //등록 개시 SET
	  //1) 등록 취소 T/O CANCEL
	  rodRegistToStop();
	  // 2) AP에 교체개시 송부
	  StsSendPrintln(STX_ROD_REGIST+ ROD_REGI_WRITE_START);   // AP에 송부(등록개시)
	
	  // 3)=== PEER 바꾸기
	  rodRegistMode = 3;	  //Peer변경
	  // 기존 PEER삭제
	  int peerRet;
	  peerRet = esp_now_del_peer(slave_board_addr);
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR DEL PEER:" + String(peerRet));
	  }
	  // 수신주소 PEER추가, TBD  : 수신한 STR_ADDR을 HEX로 변환하여 PEER추가는 추후
	  peerRet = eNow.addPeerAddress(rcv_src_addr_back);   // HEX
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR Add PEER: newROD" + String(peerRet));
	  }
	  // 4)=== ROD에 통지 ( Main주소 쓰기)
	  rodRegistMode = 4;
	  // BC PEER추가
	  peerRet = eNow.addPeerAddress(broad_cast_addr); // HEX
	  if (ESP_OK != peerRet)
	  {
		  LogPrintln(" LG] ERROR Add PEER: BC" + String(peerRet));
	  }
	  // == ROD에 주소 송부
	  String myAddr = eNow.getMyAddress();
	  eNow.writeBC(DF_Protocol_MainToRod_MainAddress, myAddr);	  // 보내기전에 BC설정 필요?
		  LogPrintln(" LG] Write Send:" + myAddr);
	  // ** 대기 (ROD Write완료 수신) - NA
	  
	  // **BC PEER 삭제
	  peerRet = esp_now_del_peer(broad_cast_addr);
	  if (ESP_OK != peerRet)
	  {
		LogPrintln(" LG] ERROR nowPeer DEL: " + String(peerRet));
	  }
	  esp_now_peer_num_t rodPeer;
	  esp_now_get_peer_num(&rodPeer);
	  LogPrintln(" LG] nowPR PeerNum:" + String(rodPeer.total_num));
	
	  // 5) 타켓주소 쓰기
	  rodRegistMode = 5;
	
	  //if((DF_Protocol_MacAddressTextLength - 1) < now_cmd_data.length())
	  {
		  //bool errRet = eNow.setTargetAddress(now_cmd_data);	  // Write STRING, 처리시간 6ms
		  bool errRet = eNow.setTargetAddress(msg);	  // Write STRING, 처리시간 6ms
		  if(false == errRet)
		  {
			  LogPrintln(" LG] rodRg ERROR TRGT ADDR WRITE");
		  }
	  }
	  
	  String wrtAddr = eNow.getTargetAddress();
	  LogPrintln(" LG] rodRg Addr:" + wrtAddr);
	  
	  // 6) T/O처리 대기
	  rodRegistMode = 6;	  // 3초 대기 시작
	  rodRegi3secToStart(); 	  // 3초 /3초 대기 Start (1st3sec:FINISH send to AP, 2nd 3sec: restart)

}


//---------------------------------------------------------------------
// 보드 & System Elec Input
void tmInput_BoardElecMonitorExec(short _exec)
{
	if(AP_IS_TM == apType)
	{
		//tmInputAllcheck = _exec;
		diagInFlag_BootSwc = _exec;		// In 01-02
		diagInFlag_PonSwc = _exec;		// In 05-01
		diagInFlag_PcUsb5V = _exec;		// In 05-02
	}
}

void tmInput_BbnMotorMonitorExec(short _exec)
{
	if(AP_IS_TM == apType)
	{
		//tmInputAllcheck = _exec;
		diagInFlag_BobbinEncA = _exec;		// In 03-01
		diagInFlag_BobbinEncB = _exec;		// In 03-02
		diagInFlag_BbnBldcFG = _exec;			// In 03-05
	}
}


// TM DIAG INPUT TEST
void anaTmInputTestReq(String msg)
{
	// TM에서는 Cat 01-xx/Cat 05-xx, Cat 03-xx  의 All Mon ON/ OFF만 사용
	diagInputTest(msg);
}

// TM DIAG OUTPUT TEST
void anaTmOutputTestReq(String msg)
{
	// TM에서는 Cat 05-01 (Sub AC On/Off)만 사용
	diagOutputTest(msg);
}

//----------------------------------------------
//	AP타입 ( DF or TM ) 수신 처리
// $29kkxx%
//----------------------------------------------
void anaApInfo(String msg)
{

	short kind = msg.substring(3,(3+2)).toInt();
	short act = msg.substring(5,(5+2)).toInt();
	
	//String respMsg;
	//String logMsg;
	String send2rodMsg = msg.substring(3,(3+4));


	switch(kind)
	{
		// kk = 1	// AP Type
		case DEF_AP_TYPE:		// $2901 : RECV AP_TYPE(DF or TM)
			apType = act;	//AP가 통지하지 않으면, 기본값 AP=DF(게임프로그램)
			eNow.write(DF_Protocol_MainToRod_ApInfo, send2rodMsg);

			// TM타입 수신시, 모든 IO자동 출력 : 보드 & Elec Input Monitor 시작
			if(AP_IS_TM == apType)	// TYPE is TM
			{
				//( 삭제 ) TM타입 수신시, 모든 IO 입력 자동 출력 : 보드 & Elec Input Monitor 시작
			}
			//
			break;

		default:
			break;
	}
	
}

/*----------------------------
	MAIN ALIVE(AP<->MAIN) 응답 처리
----------------------------*/
void anaMainAlive(String msg)
{

	//== 1) RESP MAIN CONN
	//str = STX_MAINALIVE + "11111" + battLvlStr + "%"; // CONN (1~99 %)
	String respMsg;
	respMsg = STX_MAINALIVE; // CONN
	respMsg += "%";
	Resp2ApPrintln(respMsg);		  // Resp to PC
	  		

	// 2) OTHER STATE(ROD/BAT/IMU) SEND CHECK
  	if( (2 > recvMainConn_Pwr1stCnt) &&  (PROG_START == gameStatus ))	// 2 Times SEND
  	{
		recvMainConn_Pwr1stCnt++;
 		sendCurrentStatus();
 	}

	//3) AP-Main 연결 SET
	apAliveElse_NotRecvCnt = 0;
	apAliveConn_NotRecvCnt = 0;		// Not recv CNT CLR
	ap_conn_status = CONNECT;	// AP CONNECT
	
}
//------------------------------------------------------------------------------------



//---------------------------------------------
// Device Ready Check CALL 1ms
//
