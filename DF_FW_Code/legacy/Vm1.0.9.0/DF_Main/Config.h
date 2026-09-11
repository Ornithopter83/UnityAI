//
//  Define Compile Switch FOR CONFIG
//

//==========================================
// LM-JIG & NORAML 구분
//==========================================

#define DEF_NOT_USE	(0)		// NOT COMPILE CODE

//==REL용
#define CONF_LM_JIG	(0)		//	NORMAL CONFIG, TBD-LM : COMPILE ERROR
//#define CONF_LM_JIG	(1)		//LM-MOT CONFIG

// LM-JIG용
#if (CONF_LM_JIG)
	#define CONFG_LM_MOT_CONTROL	(1) //LM CONT(버티기시 LM모터 제어 여부)
	//#define CONFG_LM_MOT_CONTROL	(0)	//without LM CONT + PC_TEST
	
	#define AUTO_TENSION_PID	(0)		// BBN오토텐션 제어 = 0
//== REL 용
#else
	#define AUTO_TENSION_PID	(0)		// BBN오토텐션 제어 = 0
	//#define AUTO_TENSION_PID	(1)		// BBN오토텐션 제어 = 1
#endif

	//-----------------------------------------
	//----- DEBUG용, PC에서 TEST---
#define DEV_CHECK_SKIP		(0)		// REL
//#define DEV_CHECK_SKIP		(1)		// 모터없는 TEST버전
	//==========================================
	
// == 보드 하루에 한번 리셋
#define PWR_SW_OFF_CPU_SOFT_RESET	(0)

// == BLDC 24V FORCE ON TEST
#define TEST_BLDC24V_FORCE_ON	(0)		//debug, REL(0)
//#define TEST_BLDC24V_FORCE_ON	(1)		// for Debug TEST

//== 보드 IO변경 : OUT(WDT) => IN(BRD_TYPE) 25/06/26 => TDB(추후)
//#define BOARD_NEW_WDT_DIS	(0)		// 구보드(V1.2 - 출력 WDT)
//#define BOARD_NEW_WDT_DIS	(1)		// 신보드(V1.3 - 입력 보드타입)

//-------------------------------------------
//  엔코더 A신호 인터럽트 추가
//	모터박스 부하편차 대응 : 토크모터 최소DUTY 자동 설정
//-------------------------------------------

//--------------------------------------------
// NORMAL AUTO TENSION
//--------------------------------------------
//==LM-JIG용
#if (CONF_LM_JIG)
	#define SYS_TIMER_INTR_ENB		(0)		// TIMER INTR
	#define GPIO_INTR_ENCA_ENB		(0)		// LM ENC INTR
	#define GPIO_INTR_PID_CONT		(0) 	// ENC INTR PID CONT ENB
//== REL용
#else	// REL==NOT LM
	#define SYS_TIMER_INTR_ENB		(1) 	// TIMER INTR
	#define GPIO_INTR_ENCA_ENB		(1) 	// NORMAL ENC_A INTR
	
	//==오토텐션용
	#if (AUTO_TENSION_PID)
		#define GPIO_INTR_PID_CONT	(1) 	// ENC INTR PID CONT ENB
	//==REL용
	#else
		#define GPIO_INTR_PID_CONT	(0) 	// ENC INTR PID CONT ENB
	#endif
#endif

//===PID 출력 모터 선택===
#if (AUTO_TENSION_PID)
	//---BBN출력 ---
	#define PID_OUT_BLDC	(0)
	//#define PID_OUT_BLDC	(1)
	//--- TORQ 출력
	//#define PID_OUT_TORQ	(0)
	#define PID_OUT_TORQ	(1)
#endif

//====  1. COMPILE CONFIG
#define HAND_TEST_01		(1)			//손맛개선 1차
#define HAND_TEST_02		(0)			//손맛개선 2차
#define BBN_BLDC_TEST 		(1)			// 보빈모터 BLDC추가 테스트
#define HAND_TEST_03		(BBN_BLDC_TEST)			//손맛개선 3차
  //#define BBN_PWM_INVERT	(1)		// INVERT BBN PWM DUTY
#define FW_CONDITION_CHK 	(0)			// Self Condition Check
#define CMD_CONTROL_2407E	(0)
#define NEW_IO_DIAG			(1)			// I2C, LED_PWM, PC_OFF, USB#5V, etc 24-7-23
//#define NEW_IF				(0)			// OLD_ IF 24/8/9
#define NEW_IF				(1)			// NEW IF 24/8/10 K1.2~

#define AP_MOT_CONT			(1)		// REQ Marketing, WithOut MKT_TEST_3
//#define AP_MOT_CONT		(0)		// OLD - FW MOT CONT

#define MKT_TEST_1			(0)		// MKT TUNNIG, 24/10/18 $F9xxxx
//#define MKT_TEST_2		(1)

//==REL용
#if	(AP_MOT_CONT)
	//---- REQ Marketing, WithOut MKT_TEST_3 : 2024/10/29
	//#define MKT_TEST_3	(1) 	// All Operation After Hooking ~ Success,  Motor 1~20 Step
	#define MKT_TEST_3		(0)		// REL=0
	#define COST_10W_CHANGE (0)		// REL=0
//==TEST용
#else
	#define MKT_TEST_3	(1)		// All Operation After Hooking ~ Success,  Motor 1~20 Step

	//--- test ROM Config, 10W Value Out to 30W Motor
	//#define COST_10W_CHANGE	(1)
	#define COST_10W_CHANGE (0)
#endif

//=== FW SELF CONTROL, by BITE PATTERN
#define	FW_CONT_BITE_HOLD	(1)		// BITE FW CONTROL From AP CMF BITE_PTN2($1153Kxyz)
//-----------------------------------------

//==== 2. IO  CONFIG==================

//--- TORQ MOTOR CONFIG
//#define CONFIG_TORQ_WATT_10W	(0)
#define CONFIG_TORQ_WATT_30W	(1)
//==REL용
#if(CONFIG_TORQ_WATT_30W)
	#define CONFIG_TORQ_HUD			(0)		// NOT EDIT
	#define BBN_SPRING_AUTOTENSION	(1)
//==TEST용
#else
	//#define CONFIG_TORQ_HUD 	(0)		// Select HUD
	#define CONFIG_TORQ_HUD	(1)			// HAUNDAE FOLLOW
#endif

//=== MC CONFIG
//#define MC_CONF_NO_0	(0)		// Proto 1 #0 & #1,#2
//#define MC_CONF_NO_3	(1)		// Proto 1	, #3
#define MC_CONF_NO_4	(1)		// Proto 2	, #4,5,6,7


//=== BLDC MOT CONFIG
#define BBN_MOT_16W		(1)
#if(BBN_MOT_16W)
	#define BBN_MOT_38W	(0)		// 16W
#else
	#define BBN_MOT_38W (1)		// 38W
#endif


//====  3. IO ENB ========================
#define IO_TORQ		(1)

#define IO_SERVO	(0)		// (1)

#define IO_BBN	 	(1)

//--- LM_JIG Config
#if	(CONF_LM_JIG)
#define IO_LM_MOT 	(1)
#else
#define IO_LM_MOT 	(0)
#endif
//----------------

// BREAK now SEND
#define IO_BREAK_MOT	(0)		// Break MOT

#define IO_DLED		(1)			// Display LED

#define IO_PC_PWR_OFF 	(1)		// Power OFF Switch대응

#define IO_I2C		(1)

#define IO_NVM_TEST		(0)		// Read/Write Test
//#define IO_NVM_TEST		(1)		// Read/Write Test

//--- LM_JIG시 사용하면 안되는 사항 설정
#if (CONF_LM_JIG)
#define IO_EXTIO_UART_PIN		(0)		// PIN DEFINE  EXT_IO_UART or OPT_LED

//=REL용
#else
//#define IO_EXTIO_UART_PIN		(1)		// PIN DEFINE  EXT_IO_UART or OPT_LED , 
#define IO_EXTIO_UART_PIN		(0)		// PIN DEFINE  EXT_IO_UART or OPT_LED , 

#endif
//--------------------------------------

#define CPU_RESET_PIN_SUPPORT	(0)
//#define CPU_RESET_PIN_SUPPORT	(1)


//===== FUNC ENB/DIS===========
										//---- 사용 : ADD EXT_IO__UART ( RX : GPIO_1, TX : GPIO_2) = SERIAL_2 
#if (CONF_LM_JIG)
#define FUNC_IO_EXT_UART		(0)		//	미사용 : 사용포트 미정으로 DIS
#else
#define FUNC_IO_EXT_UART		(0)		//	미사용 : 사용포트 미정으로 DIS
//#define FUNC_IO_EXT_UART		(1)		//
#endif

#define FUNC_LOG_UART_RCV_ENB		(1)  //REL=1, LogUart명령 분석,처리	// 사용 : AP게임시작시 1회 10ms지연 => 전체 영향없음

//
#define FUNC_FW_CONT_LM_MOT		(0)		// LM-JIG , AP Control, GameStatus WAT
//#define FUNC_FW_CONT_LM_MOT	(1)		// LM-JIG , FW Control, GameStatus WAT
#define FUNC_RF_CERTIFICATION_TEST	(0)		// REL=0, 인증TEST용(Reel모터,버튼LED 자동 동작)

//---
//#define FUNC_STBY_OFF_CONT	(0)		// Debug
#define FUNC_STBY_OFF_CONT	(1)			// REL

#define FUNC_FET_PROTECT	(1)			// REL, FET손상 방지, BLDC 100미만 출력 제한

//==REL용 
#define PID_PARA_SET_ENB		(0)		// REL=0, AT Kp,Ki,Kd설정
//#define PID_PARA_SET_ENB		(1)		// TEST, AT Kp,Ki,Kd설정

// === 4. IF CONFIG =================
#define RESP_IO				(0) // (0)		// $02(Torq), $03(Servo), $04(Bldc)
#define RESP_IO_TEST		(0)		// Duty 1023 Val OUT

#if (AP_MOT_CONT)
#define RESP_IO_MAIN_TORQ	(1)
#define RESP_IO_BBN_BLDC	(1)
#endif

#define RESP_LED_CONT_CMD	(0) //(1)		// REL(0)

#define RESP_IMU_DATA		(1)

//#define RESP_GMWAT_CMD		(0)		// choice, GAME_WAT_STATUS 응답
#define RESP_GMWAT_CMD		(1)		// choice, GAME_WAT_STATUS 응답

//====5. LOG OUT DEFINE ============
#define LOG_BBN_ENC_LOGOUT	(0)	//(1)	// REL=0
#define LOG_LM_ENC_LOGOUT	(0)	// REL(0), LM Enc Pulse COUNT LogOUT

#define LOG_AP_CMD			(1)	//(1) 	// 0 -REL:실행시간 개선, 1- fo DEBUG

#define LOG_MAIN_CONN		(0)		// Main_Alive_Check POLLING
#define LOG_DEV_CONN		(1)		// Change SEND

#define LOG_IO				(0) //(1)
#define LOG_IO_TORQ			(1)			// must LOG


#define LOG_BLDC_AP_CMD 	(1)		// REL (AP CMD)
#define LOG_IO_BBN			(1)		// REL (FW OUT)
#define LOG_IO_LM			(0) //(0)

#define LOG_FW_BAT_CHG		(0)
//#define LOG_ENOW	(0)

#define LOG_ROD_BTN			(0)		// debug
#define LOG_ROD_ENC			(0)		// debug

#define LOG_FISH_REGIST		(1)		// REL, Random Fish Regist LOG OUT, 물고기 버티기

#define LOG_FUNC_STEP		(0)
#define LOG_FUNC_STEP_BITE	(0)		// Bite PTN_2
#define LOG_LED_STEP		(0)

#define LOG_I2C			(0) //(1)

#define LOG_ELEC_IO		(0) // (1) // (1)		// DebugPort ON/OFF, SubAC On/Off LOG OUT

#define LOG_LE_TE			(0) //(1)
#define LOG_LE_TE_1MS		(0) //(1)
#define LOG_LE_TE_10MS		(0) //(1)

#define LOG_READ_IN_1MS		(0)
#define LOG_READ_IN_10MS	(0)

#define LOG_PWRSWC_OFFON	(1) //(1)	//변화 : Switch On, Off,Off Start, Off Cancel
#define LOG_POWEROFF_FUNC	(1) //(1)	// OFF Seq Step별 Log출력

#define LOG_CONT_STEP		(1)		// REL, 각 Control Step상태값을 LOG출력

#define LOG_MKT_TEST_23		(1)		// REL, MK_TEST_2 설정값 LOG출력

#define LOG_BITE_IMPROV		(0)	//(1)
#define LOG_HOLDON_IMPROV	(0) //(1)

#define LOG_DEV_CHECK_OK	(0) // (1)		// Device Check Log

#define LOG_NOW_GET_INFO	(0) //(1) //(1)		// for Debug

// Target Address Load
#define LOG_TART_ADDR_LOD	(1)		// LOG 타켓주소 , MUST Once Only

// AP msg Rcv Parsing
#define LOG_AP_RCV_MSG_PARSING		(0)				// AP rcv MSG 파싱 로그

#define LOG_UART1_RCV_MSG_PARSING	(0)				// LOG_UART rcv MSG 파싱 로그
#define LOG_UART1_RCV_MSG			(0)

// Delay Check
#define LOG_SYSTICK_DELAY		(1)		// REL, System Run Time, Delay Measure
#define LOG_1MS_EXE_DELAY		(1)		// REL, 1ms EXE Loop Delay Time

#define LOG_IMU_RCV_1ST_TIME	(0)		// debug
//#define LOG_IMU_RCV_1ST_TIME	(1)		// IMU 1st OUT TIME

// Rod Alive, REL(0), DEBUG(1)
#define LOG_ROD_ALV_CHK		(0) //(1) //(0)	// debug, Send ALIVE
#define LOG_ROD_ALV_RESP	(0) //(1) //(0)	// debug, Recv ALIVE

// Now SEND/SENT Check
#define LOG_NOW_SEND_ERR		(1)		// REL, Send Error Log Out, MUST(REL=1)
#define LOG_NOW_SEND_SEQNO		(0)		// debug
#define LOG_NOW_CB_SENT_FAIL	(1)		// REL,
#define LOG_NOW_CB_SENT_REGI	(1)		// REL,

// NOW RCV CHECK
#define LOG_NOW_RCV_OVER_WRITE	(1) 	// REL, Now수신 Overwrite LOG
#define LOG_NOW_RCV_CB_SEQNO	(0)
#define LOG_NOW_RCV_ANA_PID		(0)		//debug, LOG NOW_ID Out, at Recv ANA

// ROD REGIST BC
#define LOG_ROD_REGI_TO_STEP			(1)	// REL, TO STEP(SEQ) LOG출력, BIG_STEP
#define LOG_BC_ROD_ADDR_WRITE_STEP		(0)	//   BC수신 MSG LOG출력
#define LOG_BC_ROD_TART_ADDR_WRITE_TIME	(0)	//   타겟주소 저장
#define LOG_BC_MAIN_ADDR_SEND			(1)	// REL, ROD에 MAIN주소 송부(브로드캐스트)

//
#define LOG_VRT_AP_CMD			(0)			// for DEBUG
#define LOG_VRT_AP_CMD_ERR		(1)			// REL, MUST 진동모터 ERR
#define LOG_BTN_LED_AP_CMD		(0)
#define LOG_BTN_LED_AP_CMD_ERR	(0)

// LOG LM MOT
#define LOG_IO_LM_MOT		(0)		// LmMot IO_OUT Log Out for Debug

#define LOG_LMMOT_AP_CMD	(0)		// REL=0, Recv CMD OUT
#define LOG_LM_HOME_END		(0)		// REL=0, HOME END OK/NG LOG

#define LOG_LM_SET_PARA_OUT	(0)		// REL =0, Cmd LmParaSet, Result LogOut

#define LOG_CONFIG			(0)		// REL=0, CONF Log OUT

//
#define LOG_GAME_WAT_CMD	(0)	// REL=0,

#define LOG_INTR_ENC_A		(0)		// REL=0
//#define LOG_INTR_ENC_A		(1)		//debug

#define LOG_WAT_FISH_LEVEL	(0)

#define LOG_LM_WIRE_CONTROL	(0)
#define LOG_AP_TYPE			(0)			// REL=0

#define LOG_MANUF_INFO		(0)			// REL =0

#define LOG_DEVCHK_BBN_MOT	(0)			// debug
#define LOG_ENC_SEN_ERR		(0)			// debug
#define LOG_DEVCHK_TORQ_ERR	(0)			// debug

#define LOG_DEVCHK_BLDC24V_ERR	(0)		// debug

#define LOG_MAIN_BOARD_TYPE		(0)		// debug

#define LOG_MAINMOT_AUTOSET_ERR			(0)		// debug
#define LOG_MAINMOT_AUTOSET_LAST_PULSE	(0)		// debug

#define LOG_WIRE_AGING_TEST		(0)		// debug
#define LOG_WIRE_AGING_TEST_ERR	(0)		// debug, ERR

//-------------------------------------
//#define WIRE_TEST_DESK	(1)			// debug
#define WIRE_TEST_DESK	(0)			//debug, REL(0)

#define LOG_MMOT_AUTOSET_STEP	(0)		// debug

//
#define LOG_STBY_TORQ_OFF_STEP	(0)		// debug
#define LOG_STBY_TORQ_OFF_ERR	(0)		// debug, ERR


