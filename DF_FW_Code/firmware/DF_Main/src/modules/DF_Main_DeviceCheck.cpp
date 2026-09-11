// Main device check implementation.
#include "../DF_Main_Internal.h"
#include "DF_Main_DeviceCheck.h"
void devChkReady_Control()
{
	if(0)	{}	// dummy
	else if(devCheckAllFlag)		{ devchk_bbnAll_Control(); }
	else if(devCheckLmMotFlag)		{ devchk_lmMot_Control(); }
	// CHECK LAST
	else
	{
		// LM은 
		devchk_OkEnd();			// 모터 OFF, 보빈모터 초기값 출력(줄 텐션)
	}
	// LM MOT Check - Home Control
	// TBD-LM

}


void resp_MainBoard_Ver()
{
	String respMsg;
	// Main보드타입 송신(무조건)
	respMsg = STX_INFO_REQ + MAIN_BOARD_TYPE + strMainBoard[mainBoardType]+ "%";
	Resp2ApPrintln(respMsg);
}


//--------------------------------------------
// 장치 에러 검사 결과 : OK종료
void devchk_OkEnd()
{
	int outTorq = 0;

	// 1) Device CHK 종료 & 결과 응답
	//=== Last OK Msg Send
	devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
	devChkMsgSend(MAIN_DEV_CHK_OK);

	//메인보드 버전 송부
	resp_MainBoard_Ver();
	
	
	// 3) 토크 모터 OFF
	//  3-1) AP=TM이면 토크모터 OFF
	if(AP_IS_TM == apType)		// TM이면 토크모터 OFF
	{
		torqOff();
		outTorq = 0;
	}
	//  3-2) AP=게임이면 토크모터 최소값 출력(낚시줄 텐션 유지 목적)
	else
	{
/* (V108) 장치 체크 종료시 토크 모터 동작 생략
		if(recv_TorqMotor_Flag)
		{
			torqMotor.setValue(reqTorqueMotor); 	//DEFAULT( 70 ) OUT = BELT SPRING TENSION
			outTorq = reqTorqueMotor;
		}
		else
		{
			torqMotor.setValue(defaultTorqueMotor); 	//DEFAULT( 70 ) OUT = BELT SPRING TENSION
			outTorq = defaultTorqueMotor;
		}
*/					
	}
	// 토크모터 FW자체 출력 , AP에 응답.
	send2resp_FwSelfControlCmd(STX_TORQUE_MOTOR, outTorq);

		//== 장치장애검지 OK(정상)종료시 [STBY토크OFF제어]요구 Check필요
		stbyTorqOff_ReqCheck();

}

// FW SELF CMD 응답 : CMD + VAL
void send2resp_FwSelfControlCmd(String Cmd, int torq)
{
	Resp2ApPrintln(Cmd + stringTo3char(torq) + "%");
}

String stringTo3char(int val)
{
	String strRts;

	if(  0 > val)	val = 0;
	if(999 < val)	val = 999;

	if(0)				{} // Dummy
	else if(100 < val)	{strRts = String(val); }		// 3자리
	else if( 10 < val)	{strRts = "0" + String(val); }	// 0 + 2자리
	else				{strRts = "00" + String(val); }	// 00 + 1자리

	return strRts;
}

//
void devChkMsgSend(String msg)
{

	// 1st READY(0000) RESP
	msg = STX_DEV_CONT_REQ + msg;	// Ready 00-00 (xx-yy)
	//msg = STX_DEV_CONT_REQ + "6010";	// Ready 00-00 (xx-yy)
	msg += "%";
	Resp2ApPrintln(msg);
}

//-------------------------------------------
// Device Ready Check START form AP CMD
//
void devchk_bbnAllStart()
{
	devCheckAllFlag = 1;
	devBbnAllChkStep = 10;
}
void devchk_lmMotStart()
{
}

//--- All Flag & Step Clear
void devCheckFlagStep_AllClear()
{
	devCheckAllFlag = 0;
	devBbnAllChkStep = 0;
}

#define DEVCHK_BBN_DIR_CHANGE_TO	50 // 100	// 300ms(OK후 CCW오버런)
#define DEVCHK_BBN_RETRY_OFF_TO		50	// 300ms(OK후 CCW오버런)

#define DEVCHK_ENC_SEN_TO	300 // 500 // 300		// 200

#define DEVCHK_TORQ_MOT_ON_TO	300 // 500	//800
#define DEVCHK_TORQ_MOT_STEP_TO	100	//800
#define DEVCHK_TORQ_START_DELAY_TO	150

#define DEVCHK_TORQ_MOT_OFF_TO	50 // 100	//800

#define DEVCHK_LM_MOT_TO	5000  //

//=== 모터확인시 토크 출력값 ( 너무세면 낚시대가 TV를 때림)
#define DEVCHK_BBN_DEFAULT_DUTY	30
#define DEVCHK_BBN_STEP_DUTY	10		//25	// 20

#define DEVCHK_BBN_WIRE_REL_ON_TO	300		// (V108)

#define DEVCHK_BLDC24V_REL_ON_TO	150 // 500	//800	// TBD, 시간확인 필요(1회전)

#define DEVCHK_BLDC24V_BREAK_TO		50		// Beak 50ms

#define DEVCHK_TORQ_TO_ENCA_1ST_ON_CHK_TO	600
#define DEVCHK_TORQ_TO_ENCA_1ST_ON_SPEC_TO	250 // 300

#define DEVCHK_BLDC24V_RETRY_WAIT_TO	500 // 300 // 100

#define DEVCHK_BLDC24V_OFF_WAIT_TO		1500	// 1.5 SEC

#define DEVCHK_BLDC24V_SEN_CNT_TO	1200 // 300 // 100

#define DEVCHK_TORQ_STOP_WAIT_TO	500
#define	DEVCHK_TORQ_MOT_CHK_CNT		(9*2)		// To

#define DEVCHK_LM_DUTY	35		//
//===



#define RETRY_CNT	3	// Retry 3회 (총 3회)
#define IN_SIG_CHG_CNT (3*2)	// Pulse 3회 => 신호변화 6회


typedef struct _stDevchkBbn
{
	// Duty, Time
	unsigned short duty;
	unsigned short ccw_ontime;	
	unsigned short cw_ontime;	
}stDevchkBbn;

#define DEVCHK_BBNMOT_SIZE	(1 + 3)
stDevchkBbn	stDevchkBbnMot[DEVCHK_BBNMOT_SIZE] =
{
	// duty, Ccw_on(풀림) , cw_on(감김) 시간
	{0,},	// Dummy
	// 1회
	{30, 300, 300},	// 
	// 2회
	{40, 250, 250},	//
	// 3회
	{50, 200, 200},	// 
};

// 보빈 모터 관련 전체 장애 검지(V108)
// 1) BLDC 모터 풀기 (CCW, Duty 50, 300ms)
//   - Hall 센서로 모터 동작 체크
//   - ENC A,B 센서 정상 체크
// 3) 토크 모터 감기
//   - ENC A 센서로 토크 모터 동작 체크
//
void devchk_bbnAll_Control()
{
	static unsigned long _to;
	static unsigned long _toSenChg;
	static unsigned long pulseWidthMs;
	
	static unsigned short olddevBbnAllChkStep = 0;
	static short errK = ERR_OK;

	static unsigned short _oldSen;
	unsigned short _newSen, _chgSen;

	static unsigned short _chgCnt  = 0;
	static unsigned short _chgCntA = 0;
	static unsigned short _chgCntB = 0;
	static unsigned short _chgCntF = 0;

	static int nDevChkErrCount = 0;
	static int nDevChkTorqDuty = 0;
	static int _nEncCount[4] = {0};
	static String msgErr;
	String msg;

	
	switch(devBbnAllChkStep)
	{
		//IDLE
		case 0:
			break;
			
		// START
		case 10:
			_oldSen = LVL01_BBNALL;
			torqOff();					// 낚시줄 텐센 제거
			bbnMotor.offBldc();

			errK = ERR_OK;
			msgErr = MAIN_DEV_CHK_OK;
			setTO(_to);
			devBbnAllChkStep = 14;
			break;

		case 14:
			if(checkTO(_to, DEVCHK_TORQ_MOT_OFF_TO) )		// 50 ms
			{
				setTO(_to);
				devBbnAllChkStep = 20;
			}
			break;

		// CCW CHECK - Duty 50, 300ms 회전후 센서 체크
		case 20:
			_nEncCount[0] = iIsrbbnEncCnt;
			_nEncCount[1] = _nEncCount[2] = 0;
			bbnMotor.onBldc(BBN_MOT_CCW, DEVCHK_BBN_WIRE_REL_DUTY);
			_oldSen = LVL01_BBNALL;				// (V108)
			_chgCntA = 0;						// * Sensor A 신호변화 Cnt
			_chgCntB = 0;						// * Sensor B 신호변화 Cnt
			_chgCntF = 0;						// * Sensor F 신호변화 Cnt
			nDevChkTorqDuty = 0;
			setTO(_to);
			devBbnAllChkStep = 21;
			break;

		case 21:								// 300 ms 대기후 모터 off & 센서 체크
			if(checkTO(_to, DEVCHK_BBN_WIRE_REL_ON_TO) )		// 300 ms
			{
				bbnMotor.offBldc();
				// CNT확인
				// 1) OK
				if ((IN_SIG_CHG_CNT < _chgCntF) && (IN_SIG_CHG_CNT < _chgCntA) && (IN_SIG_CHG_CNT < _chgCntB))		// (V108)
				{
					setTO(_to);
					devBbnAllChkStep = 29;	// OK, 100ms후 정지
				}
				// 2) NG
				else
				{
					if (IN_SIG_CHG_CNT >= _chgCntF)				// (V108)
						msgErr = MAIN_BLDC_MOT_ERR;				// *
					else if (IN_SIG_CHG_CNT >= _chgCntA)		// *
						msgErr = MAIN_BBNENCA_SEN_ERR;			// *
					else if (IN_SIG_CHG_CNT >= _chgCntB)		// *
						msgErr = MAIN_BBNENCB_SEN_ERR;			// *
					else
						msgErr = MAIN_BLDC_MOT_ERR;
					devBbnAllChkStep = 75;	// NG
					errK = ERR_CCW;
				}
				_nEncCount[1] = iIsrbbnEncCnt;

			}
			else {
				_newSen = LVL01_BBNALL;						// (V108) 이하 수정 
				_chgSen = _oldSen ^ _newSen;				// *
				if(_chgSen)									// * BBN Sensor 신호 변화
				{
					_oldSen = _newSen;						// * 센서상태 저장
					if (_chgSen & BIT_SNS_BBNFG)			// *
						_chgCntF++;							// * F 신호변화 cnt + 1
					if (_chgSen & BIT_SNS_BBNENCA)			// *
						_chgCntA++;							// * A 신호변화 cnt + 1
					if (_chgSen & BIT_SNS_BBNENCB)			// *
						_chgCntB++;							// * B 신호변화 cnt + 1
				}
			}
			break;

		//1-2] OK, CW CHECK
		case 29:
			if(checkTO(_to, DEVCHK_BLDC24V_OFF_WAIT_TO))	// 1500ms(24V OFF WAIT) 대기후 토크모터 동작 및 FET 체크
			{
				devBbnAllChkStep = 30;	// CW 체크 Start
			}
			break;

		// 2] CW확인
		// 토크 모터 동작 & FET 장애 검지 시작
		case 30:
			_oldSen = LVL01_BBNENCA;
			_chgCnt = 0;			// 신호변화 Cnt
			_nEncCount[1] = iIsrbbnEncCnt;
//			nDevChkTorqDuty = GetDefValue4torqMotor() + 3 * ((nDevChkErrCount < 3) ? nDevChkErrCount : 3);
			nDevChkTorqDuty = defaultTorqueMotor + 3 * ((nDevChkErrCount <= 2) ? nDevChkErrCount : 2);
			if (MAIN_MOT_FS_MAX_DUTY < nDevChkTorqDuty)
				nDevChkTorqDuty = MAIN_MOT_FS_MAX_DUTY;
			torqMotor.setValue(nDevChkTorqDuty); 		// DEFAULT + 3 * Retry
			setTO(_to);
			devBbnAllChkStep = 34;
			break;

		case 34:
			if(checkTO(_to, DEVCHK_BLDC24V_SEN_CNT_TO) || (DEVCHK_TORQ_MOT_CHK_CNT <= _chgCnt))		//WAIT 300 ms TO
			{
				torqOff();
				//if( (6*2) > _chgCnt)	// 펄스 미만 이면 NG
				//if( (15*2) > _chgCnt)	// 펄스 미만 이면 NG
				if( DEVCHK_TORQ_MOT_CHK_CNT > _chgCnt)	// 펄스 미만 이면 NG
				{
					nDevChkErrCount++;
					msgErr = MAIN_BOARD_BLDC_24V_ERR;
					errK = ERR_CW;
				}
				else
				{
					errK = ERR_OK;
				}
				_nEncCount[2] = iIsrbbnEncCnt;
				devBbnAllChkStep = 75;
				
				// LOG
			}
			else if(LVL01_BBNENCA != _oldSen)	// 1st Change times
			{
				_oldSen = LVL01_BBNENCA;		// 센서상태 저장
				_chgCnt++;
				pulseWidthMs = getTO(_toSenChg);
				setTO(_toSenChg);
			}
			break;

		// 모터 정지, ERR/OK 분기
		case 75:
			bbnMotor.offBldc();
			// 검출결과, AP에 출력(확인용)
			Resp2ApPrintln( STX_DEV_CHK_ERR_LOG + msgErr + (ERR_OK==errK? "OK":"NG") + ",Err:" + String(nDevChkErrCount)+",Cnt:" + String(_chgCnt)+"("+ String(_chgCntF)+","+ String(_chgCntA)+","+ String(_chgCntB)+"),ms:" + String(getTO(_to)) + ", Enc:" + String(_nEncCount[0]) + "," + String(_nEncCount[1]) + "," + String(_nEncCount[2])+ ",Duty:" + String(nDevChkTorqDuty));
			if(ERR_OK == errK)	// OK
			{
				devBbnAllChkStep = 90;		// 정상 종료
			}
			else	// ERROR CCW, CW
			{
				devBbnAllChkStep = 80;		// Error
			}
			break;


		// Error SET
		case 80:
			devReadychkFlag = 0;	// CLR DEV_RDY CHK FLAG, STOP DEV_RDY CHECK
			devCheckFlagStep_AllClear();
			
			msg = STX_DEV_CONT_REQ + msgErr + "%";
			Resp2ApPrintln(msg);


				// 장치장애검지 종료시, STBY토크OFF제어 요구 CHECK
				stbyTorqOff_ReqCheck();
			
			break;
			
		//END_OK
		case 90:
			devCheckAllFlag = 0;	// CLR BLDC Flag
			devBbnAllChkStep = 0;		// Clr Step
			if (defaultTorqueMotor != nDevChkTorqDuty) {					// 정상 처리한 duty와 default와 다르면
				fsInfo.saveInfo(MAINMOT_INFO, String(nDevChkTorqDuty));		// 에러 체크 생략
				defaultTorqueMotor = nDevChkTorqDuty;						// default 토크모터 duty 설정
				nDevChkErrCount = 0;										// 에러 회수 초기화
			}
			break;

		default:
			break;
	}
}



// TBD-LM
// LM_MOT Device Check
void devchk_lmMot_Control()
{
	// TBD-LM
	
}

// TBD-LM
// LM ENC Check Control
void devchk_lmEnc_Control()
{

}


//-----------------------------------------------
//	줄 끊어짐 검지
//-----------------------------------------------

//-------- STEP CONTROL END -----------------


//----------------------------------------------
// Sensor & Motor Ready Check ( Home Position etc)
//	1. bldc Motor Check
//	2-1. Bobbin Enc Sensor A Check
//	2-2. Bobbin Enc Sensor B Check
//	3. BLDC 24V(FET) Check
//	4. Torque Motor Check
//	5. Wire Broken Check
//
//======= TBD ============
void devReadyCheck()
{
	//장애 검지 Start
	devReadychkFlag = 1;

	devChkErrOccure = 0;
	// 각 Dev Check Flag및 Control Step START

	devchk_bbnAllStart();

	// stop 
	// Error발생시 Dev Check 중지
}

// 에러 CLR
void send_Error_Clr()
{
	String respMsg;
	respMsg = STX_DEV_CONT_REQ + "0000";	// OK응답 0000 (4자리)
	respMsg += "%";
	Resp2ApPrintln(respMsg);
}

//-------------------------------------------------------------------
// 장치 제어 명령
