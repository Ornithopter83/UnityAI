//
// Version.h
//

#include "Common.h"

// Version

#if(!CONF_LM_JIG)			//정식
	#if (DEV_CHECK_SKIP)		// 자리 테스트(보드필요, 모터 없어도 됨)

		//--String mainVer = "Vm1.0.1.50";	// PC_TEST

		String mainVer = "Vm1.0.7.1";	//REL

	#else					// 정식 REL
		#if (AUTO_TENSION_PID)
			String mainVer = "Dm0.6.2.63";	//PID TEST
		#else
			//--String mainVer = "Vm1.0.1.51";	//REL
			//--String mainVer = "Vm1.0.1.52";	//REL

			//String mainVer = "Vm1.0.3.50";	//REL
			//String mainVer = "Vm1.0.3.51";	//REL
			//String mainVer = "Vm1.0.5.51";	//REL
			//String mainVer = "Vm1.0.5.60";	//REL
			//String mainVer = "Vm1.0.6.0";	//REL
			//String mainVer = "Vm1.0.6.70";	//6041
			//String mainVer = "Vm1.0.7.0"; //REL
			String mainVer = "Vm1.0.9.0";		//(V1090)
		#endif
	#endif
#else						// LM-JIG
	#if (!CONFG_LM_MOT_CONTROL)
		String mainVer = "Dm0.5.50.0"; 	// PC_LIFE_TEST(LM보드) +  without LM CONTROL
	#else
		String mainVer = "Dm0.5.2.0"; 	// LM_JIG
	#endif
#endif

#if (NEW_IF)
String ApFgVer = "----";
//String IfVer = "k11";
  #if (AP_MOT_CONT)
	//String IfVer = "K22";
	//String IfVer = "K30";
	String IfVer = "K31";		// 25/7/11a
  #else
	String IfVer = "K20";
  #endif
#endif



/*---------------------------------------------
History
	// Vs xx.yy.zz - yyyy/mm/dd
 	;Reason
 	1) item 1
 	2) item 2
/----------------------------------------------


//==============================================
//=== REL Vm1.0.2.0 양산(25/7월)	 25/7/??
//==============================================

// REL  Vm1.0.7.0, 25/8/28
// --- test  Vm1.0.6.70, 25/8/27
	- 장치체크 처리중에 재요구는 무시함(AP 장치체크 계속 요구 대책)
// REL Vm1.0.6.0, 25/8/19

// --- test Vm1.0.5.60, 25/8/18
	- 배터리 충전 표시 : 배터리 잔량 +2% => +4%로 변경 

// --- test Vm1.0.5.51, 25/8/12
	-자동설정 에러 빈발 발생 대책
	*제어 로직 변경.
	  - 출력 : 70 => 80 Duty
	  - 측정 : 2회전(36펄스)까지의 경과시간으로 계산하여 최소출력값 결정
	  - 3.2초 경과해도 36펄스 미도달시 장애(6050)

// --- test Vm1.0.5.50, 25/8/12
   -EncodermLOG출력

// REL Vm1.0.5.0 25/8/8
	-양산2 REL
	- 9999삭제
	- 자동설정 60~80범위외의 경우 장애 => 정상 (60~80으로 저장)
	
// ---test Vm1.0.4.50
	- 9999 삭제
	
// REL Vm1.0.4.0	25/8/7
	1. BLDC제한 (FS)
	2. 6041오검지 대책
	
//---TEST Vm1.0.3.50  25/7/23
	:BLDC출력값 제한 추가 (제한값 설정-FS에 저장 추가)

	
// REL Vm1.0.3.0 25/7/22
	-이하 모두 
	
// --- Vm1.0.2.60   25/7/18
	- 메인모터 최소출력값 자동설정 제어 변경
	  (정속까지의 시간으로 최소출력값 환산)

// REL Vm1.0.2.0   25/7/18
	- 양산 2 (7월 생산분 ) 발행
	
// ---Vm 1.0.1.72    25/7/18
	- BLDC 출력DUTY제한(FET파손대책) 없앰(100 => 255), 손맛개선 TEST

// Vm 1.0.1.71	25/7/15
	- 대기중 토크OFF

// Vm 1.0.1.70	25/7/11
	- 메인모터 최소출력값 설정
	
//------------------------------------------
//-- TEST	Vm1.0.2.62 낚시줄 에이징	25/7/10
	- 불필요 LOG출력 금지로 설정(config)

//-- TEST	Vm1.0.2.61 낚시줄 에이징	25/7/8
	- bldc(CW) Duty1,Duty2 추가

//-- TEST	Vm1.0.2.60 낚시줄 에이징	25/7/7
	-Device Function Req($98)추가
	-종류(kk)-01:낚시줄에이징 테스트 FUNC추가
//------------------------------------------
	

// Vm 1.0.1.68	25/7/8
	- 메인모터 최소출력값 설정

// Vm 1.0.1.68	25/7/2
	- SYSTEM타임 인터럽트 인터벌 변경 1ms => 100us * 10번

// Vm 1.0.1.67	25/6/25
	-WDT출력 삭제
	- FET파손 대책 :BLDC 출력  최대값 100으로 한정
	- Mmot 최소출력 자동설정 추가

// Vm 1.0.1.66	25/6/23
	-BLDC_24V 보드불량 (6041)추가
	-배터리 잔량 경고 (7020)추가	

// Vm 1.0.1.65	25/6/11
	- ;torq Motor OFF Time지정 추가
	
// Vm 1.0.1.64	25/6/11
	- BLDC모터 불량 검지 개선(Retry 3회, FG 3회 검지로 변경)
	
// Vm 1.0.1.63	25/6/9
	- 배터리 충전기 정상여부 통지 추가($12, -3,-4,-5)

// Vm 1.0.1.61	25/5/29
	- 디바이스 이상 제어 변경 검토
	
// Vm1.0.1.60	25/5/26
	-파일 쓰기의 리턴값 읽기와 동일하게 통일(변경) 
	 ;ParaNG:-2=>-3, Val NG : -1 => -2

//==============================================
//=== REL Vm1.0.1.0 시양산2	 25/5/22
//==============================================

// Vm1.0.0.65	25/5/22 생산
	-1
	-2
	-3


// Vm1.0.0.61
	- 메인모터 초기값 변경 60 => 70 (시양산 모터박스 부하 증가)
	- 기본값 변경 가능 (범위 : 40 ~100)

// Vm1.0.0.60
	- TM시 엔코더 신호 계속 올라오는 것을 방지
	 (TM연결시 자동으로 엔코더 신호 변화 출력을 삭제)

//== 시양산용 발행  REL Vm1.0.0.0  25/4/28

//--------------------------------------------------------
// Dm0.6.2.xx - PID용,     Dm0.6.3.xx-양산용
// Dm0.5.1.xx-LMJIG용, Dm0.5.2.xx-LMJIG+PC_Life_TEST
//--------------------------------------------------------

//== REL Dm0.6.6.0, 25/4/28
	- 설치정보(설치날짜) 읽기,저장,지우기 추가

//== REL Dm0.6.5.0, 25/4/25
	- 보드업체 IO확인 변경(모터 FG등 )

//== REL Dm0.6.4.0, 25/4/14
	-PC OFF안됨 대책 ( IO_PC_PWR_OFF 0=>1로 변경 )
	
//== REL Dm0.6.3.0, 25/4/10
//-- Dm0.6.2.64
	-Manufactory Info 추가(모델명,시리얼번호,생산날짜) " manufacfInfo.txt"
	
// Dm0.6.2.63
	-Cont메인 i/o 추가(for TM)
	
// Dm0.6.2.62
	- PowerOn시 AP type DF(초기값)로 통지

// Dm0.6.2.61
	- AP Type구분($2901,00-DF, 01-TM) 추가
	- AP Type 수신시, ROD에 통지 추가(PID:33) / 응답 추가(PID:34)
	- TM의 경우, IMU Interval 응답 AP에 전송
	
// Dm0.6.2.60	생산용 대응
	- Opt LED삭제
	-확장 UART삭제
	
// Dm0.5.2.0
	- LM JIG : LM모터 동작  안시킴 - PC수명 테스트 버젼
	
// 0.60, Dm0.6.0.xx / Dm0.5.0.xx
	- Enc INT내 [] 시간측정, 거리계산,  PID제어 ] 추가
	- 
	
//== REL Dm0.5.1.0	(0.6.1.0)
	- 1ms 인터럽트 추가
	- 보빈 EncA신호 인터럽트 추가
	- Diag입력 처리, digtalRead() => digitalRead()? 1:0 으로 변경
	
	- LM HOME센서 논리 변경(하드웨어 수리 변경)
	
// Dm0.6.-.xx / Dm0.5.-.xx  마지막 동일Version으로 관리

// x.x.0.23 	25/3/20
	- LM-JIG : 초기값 변경(튜닝완료 값으로 변경)
	- LM_JIG : Left/Right버티기시 Power를 AP의 물고기 Power로 제어로 변경(Fish Level : Fix_LVL2 => AUTO )

// x.x.0.21/22	 25/3/20
	- GPIO 1 : Config 구별 추가 (0:Normal, 1:LMJIG), PowerOn시 읽은후 Config에 따라 입력/출력설정
	- Config AD값 읽기로 변경

// x.x.0.20, 25/3/20
	-Fish Level추가(Bite)
	- NVM Offset 추가
	- 실패시 LM Return추가

// Dm 0.6.0.10 (LM-JIG Code포함한 기존 Version) 25/3/19
	- ap게임상태(WAT) 추가

//=======================================
//	LM-JIG,	25/2/28 ~
//-------------------------------------

//Dm0.4.90.19 - LM JIG (D 0.5.~), 25/3/17
	- Start WAIT TIME 300 -> 10 ms

//Dm0.4.90.18 - LM JIG (D 0.5.~), 25/3/17
	- AP Start,Start~ Stop => Start,Stop/Start/Stop~ 변경 대응
	
//Dm0.4.90.16 - LM JIG (D 0.5.~), 25/3/17
	- Duty TUn2

//Dm0.4.90.16 - LM JIG (D 0.5.~), 25/3/17
	- Duty UP
	
//Dm0.4.90.15 - LM JIG (D 0.5.~), 25/3/14
	- LM Para SET ADD
	- Limit AP CMD for TORQ & BBN CMD

//Dm0.4.90.13 - KM JIG (D 0.5.~), 25/3/12
	- HOME ~ing

//Dm0.5.0.13 - KM JIG (D 0.5.~), 25/3/12
	- LEFT/RIGHT OK

//Dm0.5.0.12 - KM JIG (D 0.5.~), 25/3/11
	-Home/Left Control
	TBD Debug
	- Pulse Count
	- Left Control
	
// Dm0.4.90.11  25/3/11
	- HOME Control

// Dm0.4.90.10
	- 24V 논리 변경 ON : LOW->HIGH => 재변경 HIGH=>LOW
//-------------------------------------
	
	
// Dm 00.04.03.00 25/3/4
	- AP연결 끊어질 경우, Reel진동모터, 버튼LED OFF처리 추가
	- Reel Out ALL OFF(29: PID)추가
	

//=== REL Dm00.04.02.00, 25/2/28

// "Dm00.04.01.70", 25/2/27
	-Move to ROD, Vrt & LED, 10ms Control

// Dm00.04.01.60
	- ROD Regist Wait T/O IF Send추가($0102xx%) 기본값 25 SEC
	
// Dm00.04.01.50, LOG_UART IMP, 25/2/25

// Dm00.04.01.40, LED-IF ADD, , 25/2/25

//===REL Dm00.04.01.00
	-VRT mot, Btn LED
	
// Dm00.04.00.60
	- BTN LED IF($24 -> ) ADD
// Dm00.04.00.60
	- VRT Motor IF($23->19) ADD

//=== REL Dm00.04.00.00
//-- Improve M-R now, Dm0.3.09.80
	- MainID:Odd, Rod Id-Even
	
//-- Improve M-R now, Dm0.3.09.72
	- Delay Time Condition Chane : EXEC/SYSTICK =  2/5 => 1/2 ms

//-- Improve M-R now, Dm0.3.09.71
	- Code정리
	
//-- Improve M-R now, Dm0.3.09.70
	-PID 분리
	-msg축소
	
//-- Improve M-R now, Dm0.3.09.70
	[id+chStr] => []String] - 가변길이, 여러번 보내지 말고 한번에 보내자(Overwrite & 소비전력 개선)
	
//-- Improve M-R now, Dm0.3.09.60
	- pid: int -> short
	- Ver : 20 -> 19/20
	- Str : 32 -> 28
	- ImuData : 12,14 => 13,14

//== REL Dm00.03.09.00==
// Dm00.03.08.34, Delay개선, 25/2/17
	- memcpy 함수 사용
	- IMU /Break명령 수신 처리 Code 개선
	- BC Peer삭제 추가
	- Reset 방법 변경
	
// Dm 00.03.08.33
	- Delay개선
	
// Dm00.03.08.32
	- MAIN수신 금지/허가 추가

// Dm00.03.08.31
	- BC Rod에 송신 OK

// Dm00.03.08.30
	- BC Rod수신 OK
	
// === REL Dm00.03.08.00, 25/2/12
	- IMU RESP추가
	- AP UART수신 %시 Break추가
	
// Dm00.03.07.80, 25/2/11
	- 다른낚시대에서 오는 MSG는 버림
	
// Dm00.03.07.79, 25/2/11
	-낚시대 연결 회수 변경 : 3회 => 5회(5초)
	
// Dm00.03.07.78, 25/2/11
	불필요LOG출력 금지

// Dm00.03.07.77
	- LOG(Sent PID추가)

// Dm00.03.07.76
	- LOG출력 허가

// Dm00.03.07.75
	- LOG명령입력 삭제
	
// Dm00.03.07.74
	- ROD올드보드의 경우, Break모터 명령 모두 차단
	- Beak Mot 송신 TEST
	-Enow수신 Over Write LOG출력 추가
	
// Dm00.03.07.73
	System Delay TEST: 처리시간 측정 LOG추가
	Ap수신 시리얼데이타 (Until % 지연) - 개선
	
// Dm00.03.07.72
	- LOG출력 최소화
	- Main보드 LED 점멸 (2 SEC => 2.5 SEC)
	- DEVICE_READY_CHK SKIP추가(TEST용)

//=== REL, Dm00.03.07.00, 24/12/26
// Dm00.03.06.60
	낚시대 보드 Type송부 추가
	
//=== REL, Dm00.03.06.00, 24/12/23

// Dm00.03.05.64, 24/12/23
	- Source 정리
	- 불필요 Code부분 없앰
	- 함수 순서등 정리
	
// Dm00.03.05.63, 24/12/20
	- usb케이블 빠짐 -> 모든 Output OFF
	- AP와 통신 끊김(Ap로부터 10초간 Alive미수신시) -> 모든 Ouput OFF
	
// Dm00.03.05.62 , 24/12/19
	- Error Code추가

// Dm00.03.05.61 , 24/12/13
	-Change to AC OFF Time Delay 500ms After USB5V OFF

// Dm00.03.05.60 , 24/12/11
	- New Hold On for FW HoldOn Control (Only BLDC :  $1184KP )

//== Dm00.03.05.00, 24/12/10
	New Bite IF for FW Control BITE
	
// Dm00.03.04.60 , 24/12/6
	Bite Change to FW Cont
	
// == Dm00.03.04, 24/12/04
	Version Only Change
	
// Input Test, 24/11/28
	Dm00.02.06.72 - TEST
	-  ADD Input Test : Boot Mode Switch

// Proto2 10W Change PreTest(to 30W Motor), 24/11/27
	Dm00.02.06.71 - TEST
	- Delete BBN_BELT_FOLLOW (0) : [Out Torq = Req Torq + 4 ]
	계산식 : 설정값 * 0.58 + 6.5

// Proto2 10W Change PreTest(to 30W Motor), 24/11/26
	Dm00.02.06.70 - TEST
	- 20 Step Default 변경  60%(12) => 100%(20)
	- 10W 모터 환산 출력
	  계산식 : 설정값 * 1.1 - 38.6


// Proto2 LED Control, 24/11/20
	Dm00.02.06.61 
		- ONLY : Buttom Dimming ADD
		- Color No Change (07~12)
	Dm00.02.06.60 
		- Dimm Color Oder ($06x7~ ADD)

//=== Proto2 LED Control, 24/11/20
	Dm00.03.03.00 / Dm00.02.06.00 - Dimm 6 Color , 3Sec/1Color = Total 18 Sec , Hold Time 10ms

//Proto2 LED Control, 24/11/20
	Dm00.02.05.61, IF_2 Range Change

//Proto2 LED Control, 24/11/20
	Dm00.02.05.60
	-LED Control : OFF/ON/BLINK/DIMM For Game Interaction

//== REL Dm00.02.05.00 / Dm00.03.02.00
	24/11/19, Proto2
	New Board - BLDC_24V ON/OFF Logic Change

//== REL Dm00.02.04.00 / Dm00.03.01.00
	24/11/18, Proto_2
	Version Only Change (LED 제외)

//=== Proto2 LED Control, 24/11/15
	Dm00.02.03.62
	-LED Control : Game연동

//=== #4 호기 IO확인, 24/11/8
	Dm00.02.03.61
	-Break Mot제어 AP_CONT시 제어 안함으로 변경
	-배터리 잔량 통지 개선(연결 끊김시 UNKNOWN2로 변경)
	-전원 ON시 모터Default => All 모터 Off로 변경

//=== #4 호기 IO확인, 24/11/7
	Dm00.03.00.60 / Dm00.02.03.60 , 24/11/7
	1. BLDC DIR /PWM 반대
	2. 중앙LED 로보트LED - HW확인 -> Download후 부드러워 짐.
	3. 아래LED - ON안됨 -> ON추가
	4. 토크모터 + 4 => 제한 조건 추가(30이상, 255이상시 255설정)
	5. 전원 ON시, 모터Default, Btm LED ON 추가

//=== REL  Dm00.03.00.00 (AP_MOT_CONT)
	`24/11/1
	- Delete Hand TEST(FW Motor Control) 
	- Use Only [ ]Game Status / Program Start, Program End ]
	- Delete FW Control Condition in Motor Output Part ( TORQ/SERVO/BLDC/BREAK )
	
//======NEW_IF=========================

//=== #4 호기 IO확인, 24/11/7
	1. BLDC DIR 반대
	2. 중앙LED 로보트LED - HW확인
	3. 아래LED - ON안됨
	
//=== REL  Dm00.02.03.00 ( NOT AP_MOT_CONT )
	`24/10/31
	- Change Battery Level Send at Battery Out / IN 

//=== REL  Dm00.02.02.00
	`24/10/31

// Dm00.02.01.64
   24/10/30
     - AP BLDC 모터제어 명령 처리(Timer처리 수정)
     - Imu Setup Req CMD NG처리 (0,1의 경우만 OK처리)
     - main ReBoot대책
     - PointUp Motor DEFAULT out

// Dm00.02.01.63
   24/10/24
   -PCL용 - MKT_TEST삭제
   -POWER OFF Off조건 변경 ( TE -> LVL), CMD변경( AP $17->$18)
 
// Dm00.02.01.62
  24/10/23, Marketing Test 3
   - MKT_TEST_2(Hit, Hold 20 Step)삭제
   - [ Hooking이후 모든 동작의 모터출력값 ]을 100%로 하고, 1~20단계로 설정 가능할 것.

// Dm00.02.01.61
  24/10/23, Marketing Test 2
  - Hit시 Torq출력 누락 => [main_idx]값 출력으로 추가
  -IO_RESP 통지로 변경
  -LOG_BLDC 출력으로 변경

// Dm00.02.01.60
  24/10/22
  	Marketing Test 2 : 20 step

//=== REL  Dm00.02.01.00, 	24/10/22
// Dm00.02.00.62
  24/10/22
	- New IF_2
  24/10/18
	- New IF_2

// Dm00.02.00.61
  24/10/15
	- Merge Conn + New IF(K1.2~)
  24/10/10
	- Change Button Up/Down VAL

// Dm00.02.00.60
  24/10/9
	- I/F Change

//======OLD IF==========================

//== test Dm00.01.05.60
  24/10/18
   - HIT & Hold ON TUNING : OLD_IF
   - 2.8 kg (100%)

//== REL Dm00.01.05.01
  24/10/16
  - After Success, Torq Strong( Not Default Torq) Follow

//=== REL  Dm00.01.05.00, 	24/10/15
//=========================================
// Dm00.01.04.74
  24/10/11
	- IMU Connetion ( Send to ROD_INFO REQ) at PROG_START Recv
	- Send to CURR_CONN 2 TIMES at Main Alive RECV

// Dm00.01.04.73
  24/10/8
	- PC OFF DEbug
	; PWR SWC Bit Change bit3 -> bit2
	
// Dm00.01.04.72
  24/10/5,
	2) Randing
	 - SPRING AUTO TENSION TUNING
	 - Randing Coding
  24/10/3,
	1) Wave NG : Normal & Hard NG CHECK
	 - Not Exec After 1st EXEC

// Dm00.01.04.71
  24/10/2, PC PWR OFF
  - PC Power OFF Debug : OK
  - With LED OFF, LED Control BIT ADD

// Dm00.01.04.70
  24/10/1, PC On/Off기능
  - Pwr Switch Bouncing  

// Dm00.01.04.69
  24/9/30
  - IMU Conn - Add to Send IMU Conn Status at IMU Status Change
 

// Dm00.01.04.68
  24/9/26
  - Change IO
 
// Dm00.01.04.67
  24/9/26
  - EEPROM Read/Write


// Dm00.01.04.66
  24/9/25
  - EXT UART & I2C


// Dm00.01.04.65 
  24/9/24
  - EEPROM

// Dm00.01.04.64 
  24/9/24
  - 16W BLDC Active Logic


// Dm00.01.04.63 
  24/9/23
  - 16W BLDC Active Logic Change

// Dm00.01.04.62 
  24/9/20
  -

// Dm00.01.04.61
  24/9/19
  - System Timer Improve
  - Connection Debug

//==================================
// REL - Dm00.01.04.00 = #3호기 (BLDC)
  24/9/12
	- BLDC 기능도입

//=================================	
// rel 24/0909 Main(Torq) Tunning
   Dm00.01.02.01 - Emeregnce(kimBon)
 
//=================================	
//REL - Dm00.01.02.00 = #3호기 (BLDC)

// Tm 00.01.01.70  24.9.5b
- Bldc Belt Follow
-Connect

// Tm 00.01.01.69  24.9.5b
- Fight Bldc

// Tm 00.01.01.68  24.9.5a

// Tm 00.01.01.67
   24.9.4 
   -Log Mave Delete
   - Led Dimming


// Tm 00.01.01.66
   24.9.3
   

// Tm 00.01.01.65
   24.8.28 BLDC Control - Bite/Hit/Regist / Randing(not)


// Tm 00.01.01.64
   24.8.27 BLDC Level Change

// Tm 00.01.01.63
   24.8.27 BulaBula~~
   

// Tm 00.01.01.62	; BLDC Wave/Bite/Hit/Fight/ HoldOn / Success / Control
  24.8.21
    0-1) Game Satus / GameOver = TimeOver (99->98)로 변경 : 모든 Modot OFF로 변경
    0-2) Game Status / Program End (99) 추가 : -모든 모터 Off로 변경
    **설계 컨셉 ] BLDC제어시 각도모터(서보)제어는 뻄
    1) 토크모터 기본값 70(해운대 10W) => 60(신규 30W토크모터 )으로 변경
    2) 모든 토크값 튜닝 필요 ( 파워 10W => 파워 30W로  + 부하 증가[BLDC 기어] )
         토크 120 이상 설정하지 말것 : 낚시대 부러질것 같음(낚시대 강도 보강 필요)
	3) Wave
	  토크 제어 -> BLDC제어로 변경
	4) Bite
	  토크제어 -> BLDC제어로 변경
	    
	
// Tm 00.01.01.61	; BLDC BIT(TokTok) Control
  24.8.16
  1) bldc TokTok Control - Diag (wave/Bite/HIt/Fight/Holdon/Finish) 

// Tm 00.01.01.60	; BLDC BIT(TokTok) Control
  24.8.14
  1) Diag BLDC TokTok ($DO03-051xxx%)-OK
  2) $F4 SET : TokTok On%,Ontime, Off%, OffTime 

// Vm 00.01.00.00
; Main보드 변경(NewIO추가)
  24.8.13
     ; NewIO Diag(IO Check) FW Release

// vm 00.00.04.64 
  24.8.12
    5. DIAG IN/Out AP처리 추가.(기존은 LOG통신포트로만 명령받음)
	4. LED Pwm출력 -PWM(8개)사용 : BLDC Wave & LED Wave확인 OK 
  2024.8.8,
  	3. eeprom Wire(핀설정) 추가
  	2. Diag Output설계
  2024.8.6,
  	1.Diag Input설계
    
// Tm00.00.04.63 - 2024-7-24
	- IO Define
	
// Tm00.00.04.62 - 2024-7-24
  - Change String + Sign Char
  - Improve Log Out
  - Diag Input 센서 처리)

// Tm00.00.04.61 - 2024-7-23
  - ADD NEW IO Diag

// vm 00.00.02.89  - 2024.7.26
  1.서보: 입질,화이팅때  AP제어로 변경
  2.랜덤함수 변수 변경 sys_1m -> curr_tick
  
// vm 00.00.02.88  - 2024.7.17
  모든 Torq제어 +10
  서보 버티기만 -30~+30로 제한. 나머지 -90도 ~+90

// vm 00.00.02.87  - 2024.7.17
  Change TORQ Default 60 => 70

// vm 00.00.02.86  - 2024.7.16
	1. GameEnb
	2. Ready
	3.ADD gameStatus Analysis & Control


// vm 00.00.02.84  - 2024.7.15
   Change Servo Angle => Max Adj Angle : Map Control

// vm 00.00.02.82  - 2024.7.9
  ;Continue Torq Control
    1) Continue Torq 60 output
    2) Continu , torq 4step 60 -> Target Inceament
    3) Continue/Ready 추가
    4) Ready수신시 msg그대로 응답
    5) Torq수신시 msg그대로 응답
    6) ready시 단계적 Torq Inc출력값을 Debug용으로 PC에 전송.

// vm 00.00.02.64 (test Version) - 2024.7.5
  ;2nd-3 Torq(30W) + BLDC
  1. 30W-TORQ모터 각 모드별 설정값 확인
      1) 너울시 Torq :
      2) 입직시 Torq :
      3) Hit시 Torq
      4) 버티기시 Torq
  2. 30W TORQ + BLDC(6Kg)


// vm 00.00.02.63 (test Version) - 2024.7.5
  ;MB(MainBody) LED 
	1) Add Led.cpp/Led.h
  2) Add MainBody LED Pin Define

// vm 00.00.02.62 (test Version) - 2024.06.26
    BLDC TEST
    1) Encoder확인
    2) BLDC 전원 24V On/OFF추가
    3) 토크모터(30W) - 출력 설정값 변경 추가

// vm 00.00.02.61 (test Version) - 2024.06.25
    Change LED4 at PowerOn Blinking(0.3sec)

// vm 00.00.02.60 (test Version) - 2024.06.25
    TEST - ADD LOG(motor)


// vm 00.00.02.00 - 2024.06.24
    Reg Release For HAEUNDAE

// vm 00.00.01.81 - 2024.06.10
   - For Game Zone BUSAN
   1) Delete Diag Monito
   2) Delete SLEEP
   3) Delete BOBBIN TEST(Comment Out)

// vm 00.00.01.80 - 2024.06.10 
  ; TEST-Bobbin Motor (BLDC)
     1) ADD IO - Bobbin Motor - ADD BLDC PWM Control (IO 5,6,7), Add Bobbin Encoder (IO 17,18)
     2) ADD File Common.h (Common Define PIO IO Number) , reserch Timer EPS32
     3) ADD Diag Input/Output Test ( STX : $DI, $DO )

// vm 00.00.01.70  2024.06.07
  ; MERGE HandTest(V00.00.zz)  
     
// Vm 00.00.01.64,  2024/06/04 (AP-FW IntegDebug)
  1) AP미도입 항목 삭제(Slave비연결 -Mac주소)
  2) fSlave 처리 미스 수정(Slave응답시 Set가 누락됨)

// Vm 00.00.01.63,  2024/05/29
	1) ADD Slave Disconnect(SLEEP ENTRY) - use Slave Status
	2) Add Slave DisConnection(Mac Not SET)

// Vm 00.00.01.63
  2024/05/27, 
   1) Delete SLAVE_DIS set When Recved GAME_ENB
   2) Add NotCountUP, at  Time OVER

// Vm 00.00.01.62 -
  2024/05/22, New IF Design(Connection)
   1)PC-Main간 연결
     2) Main-Slave연결
     3) Slave SLEEP - 우측버튼(IO41은 WakeUp안됨)
     4) IMU Connection
    
// Vm00.00.01.61 -
  2024/05/02, LOG SERIAL 추가
  2024/05/01, System Timer(10ms,100ms)추가 

// Vm00.00.01.60 - 
  2024/04/30, Test Servo Angle -45 ~ +45 => -60~+60

// Vm00.00.01
  2024/4/10, Add Version

// Vm 00.00.00
  2024/3/30, From Potents
----------------------------------------------------------------------
*/

// Hangul (UTF-8) Test
// 한글이 깨지나?

