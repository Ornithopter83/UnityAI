# Main 소스 의존성 및 owner 초안

Updated: 2026-08-28  
Status: 작업 07 확정 초안

## 조사 범위와 판정 방법

- 대상: `firmware/DF_Main/src/modules/*.inc`, application 파일 및 기존 driver 헤더
- 함수 수는 반환형과 함수 본문이 있는 정의를 정규식으로 추출한 값이다. 생성자, 연산자 및 함수 포인터는 포함하지 않는다.
- 상태 목록은 전역 저장 공간, hardware object, ISR/callback 공유 값과 다른 헤더의 `extern` 의존을 함께 조사했다.
- 아래 목표 owner와 API 이름은 작업 08에서 적용할 설계이며 아직 구현된 심볼이 아니다.

## 현재 모듈 규모

| 현재 파일 | 줄 수 | 확인된 함수 정의 수 | 현재 역할과 집중 위험 |
| --- | ---: | ---: | --- |
| `DF_Main_Foundation.inc` | 2,661 | 40 | 모든 함수 원형, 상수, 전역 상태, hardware object, ISR/OTA buffer가 혼재 |
| `DF_Main_Communication.inc` | 558 | 22 | ESP-NOW callback, 수신 상태, motor off timer, 상태 송신이 혼재 |
| `DF_Main_LedApplication.inc` | 564 | 12 | LED 상태 machine과 전역 dimming 상태 |
| `DF_Main_ApplicationCommands.inc` | 1,450 | 33 | AP command 분석, Rod 등록, wire home, test command가 혼재 |
| `DF_Main_DeviceCheck.inc` | 465 | 14 | 장치 ready/check state machine |
| `DF_Main_LmWire.inc` | 929 | 29 | LM 위치, wire, motor command와 연결 상태 처리 |
| `DF_Main_GameControl.inc` | 3,038 | 75 | wave/bite/hit/hold/randing의 다수 전역 step state |
| `DF_Main_Diagnostics.inc` | 2,272 | 46 | 입출력 진단, 파라미터 설정, 전원 감시가 혼재 |
| `DF_Main_Scheduler.inc` | 5,074 | 67 | setup/loop, timer, 입력, UART, OTA, command dispatch 및 scheduler가 혼재 |

합계는 약 17,011줄, 확인된 함수 정의 338개다. 현재 application `.cpp`의 include 순서가 곧 선언 및 저장 공간의 결합 순서다.

## 전역 상태와 목표 owner

| 상태 묶음 | 현재 대표 심볼 | 현재 위치 | 주 사용 영역 | 목표 단일 owner |
| --- | --- | --- | --- | --- |
| hardware object | `eNow`, `torqMotor`, `sMotor`, `bbnMotor`, `mainEnc`, `lmMotor`, `extLed`, `eNvm`, `fsInfo` | Foundation | 거의 모든 기능 | `DF_Main_Hardware` |
| 공통 시간 | `curr_ms_tick`, `sys_count_*`, `lSys1MsTime`, `timer`, `mux` | Foundation/Scheduler | driver, scheduler, ISR, timeout | `DF_Main_Timebase` |
| 제품/연결 상태 | `mainBoardType`, `ap_conn_status`, `rod_conn_status`, `imu_conn_status`, `rodRegistMode`, `dfConfig`, `apType` | Foundation | command, communication, scheduler | `DF_Main_State` |
| ESP-NOW 수신 | `now_msg_str_cb`, `now_rcv_id_cb`, `rcv_src_addr_cb`, `now_msg_str`, `now_rcv_id` | Foundation/Communication | callback, scheduler, registration | `DF_Main_Communication` |
| 게임 상위 상태 | `isGameEnable`, `gameStatus`, `gameLevel`, `fishType*`, `exe*` | Foundation | command, game, driver headers | `DF_Main_GameControl`과 최소 조회 API |
| game pattern 내부 상태 | `wave_*_step`, `bite_*_step`, `hit_*_step`, `Holdon_*`, `randingStep` | GameControl | GameControl 내부 | `DF_Main_GameControl` file-scope storage |
| LM/wire 상태 | `lmPosi`, `lm*Flag`, `wireLongCheck_Flag`, `lmWireControl_Flag`, `targetWirePulse` | Foundation/ApplicationCommands/LmWire/Scheduler | LM, scheduler, command | `DF_Main_LmWireControl` |
| LED 상태 | `ledCont`, `ledLight`, `sRGB`, `outRGB`, `ext_led_dim_step` | Foundation/LedApplication | LED, diagnostics | `DF_Main_LedControl` |
| 장치 검사 | `devReady*`, `devCheck*`, `mainMotAutoSet_*`, `wireAgingTest_*` | Foundation/Scheduler | DeviceCheck, Scheduler | `DF_Main_DeviceCheck`; 자동설정/aging은 별도 diagnostics owner |
| 진단/입력 상태 | `sensor*`, `stDiagSen`, `diagInFlag_*`, `diagOutFlag_*`, power monitor 값 | Foundation/Diagnostics | input, diagnostics, scheduler | `DF_Main_Input`, `DF_Main_Diagnostics` |
| ISR/PID 상태 | `stEncIntvBuff`, `sEncBuffIdx`, `bIsrEncInterruptFlag`, PID 값 | Foundation/Scheduler | ISR, wire/game | `DF_Main_EncoderIsr` |
| OTA/download buffer | `g_Dn*`, `g_readBuf`, `g_SavePtr`, `g_ProcPtr` | Foundation/Scheduler | download path | `DF_Main_UpdateService` |

원칙은 모든 상태를 하나의 거대 구조체로 옮기는 것이 아니다. 각 owner `.cpp`가 file-scope 저장 공간을 갖고, 정말 필요한 값만 고유 접두어의 getter/setter 또는 command 함수로 노출한다.

## 확인된 교차 의존

Foundation은 다른 모듈 함수 300여 개의 원형을 한곳에 선언하므로 독립 모듈화 시 가장 먼저 해체해야 한다. 실제 기능 파일에서 확인한 주요 호출 방향은 다음과 같다.

| 호출 모듈 | 직접 의존하는 기능 |
| --- | --- |
| ApplicationCommands | Communication, Diagnostics, Hardware, GameControl, LedControl, LmWireControl, Scheduler |
| Communication | Hardware의 all-off/formatting과 공유 연결 상태 |
| DeviceCheck | Hardware torque 제어와 standby torque 요청 |
| Diagnostics | Hardware I/O, LED, torque parameter 설정 |
| GameControl | Hardware motor 제어, Communication flag, DeviceCheck 응답, Scheduler log |
| LedApplication | Diagnostics에 있는 RGB helper를 역호출함 |
| LmWire | Communication timer, DeviceCheck, GameControl fish/torque, Hardware all-off |
| Scheduler | 모든 command/control 모듈을 호출하며 application setup/loop까지 포함 |

가장 중요한 순환은 `LedApplication ↔ Diagnostics`, `LmWire ↔ GameControl`, command/scheduler와 여러 control 모듈 사이에서 발생한다. 해결 방법은 상대 모듈 내부 함수를 직접 호출하는 것이 아니라 다음 세 경계로 낮추는 것이다.

- 순수 hardware 출력: `DF_Main_Hardware_*`
- 상태 조회/변경: `DF_Main_State_*` 또는 해당 owner API
- 주기 실행: Scheduler가 각 모듈의 `Poll1ms/10ms/100ms/1s`를 단방향 호출

## 기존 헤더 결합 위험

| 기존 헤더 | 외부 상태 의존 |
| --- | --- |
| `BobbinMotor.h`, `LmMotor.h`, `Potentiometer.h` | `curr_ms_tick`, `gameStatus`, motor test 값, STX 문자열 |
| `ServoMotor.h`, `Encoder.h`, `Led.h`, `Eeprom.h`, `ENow.h`, `Diag.h` | `curr_ms_tick` 또는 application 전역 상태 |
| `DF_Main.h`, `Diag.h` | `mainMode`와 application 함수 직접 참조 |

이 `extern`들은 기존 driver가 application 전역 저장 공간을 역으로 요구하게 만든다. 작업 08에서는 한 번에 driver를 재작성하지 않고 다음 순서를 사용한다.

1. 저장 공간을 목표 owner `.cpp` 하나에 둔다.
2. 필요한 동안 고유 접두어 `extern` 선언을 compatibility header 한 곳에서만 제공한다.
3. driver 입력을 함수 인자나 owner 조회 API로 바꾼 뒤 compatibility 선언을 제거한다.

## 목표 모듈과 최소 public API

| 목표 모듈 | 최소 공개 API 초안 | private로 둘 항목 |
| --- | --- | --- |
| `DF_Main_Application` | `DF_Main_Application_Setup`, `DF_Main_Application_Loop` | 초기화 단계 함수 |
| `DF_Main_Timebase` | `DF_Main_Timebase_Init`, `DF_Main_Timebase_Run`, `DF_Main_Timebase_GetMs` | timer handle, counters, ISR 공유 값 |
| `DF_Main_State` | board/config/연결 상태의 명시적 Get/Set | 저장 구조와 이전 값 |
| `DF_Main_Hardware` | `Init`, `AllOff`, `SetDefaults` 및 기능별 명시 명령 | hardware object와 direct pin helper |
| `DF_Main_Communication` | `Init`, `Poll`, `SendCurrentStatus`, `SendSleep`, 등록 주소 전달 | callback buffer, receive flags, send log |
| `DF_Main_CommandRouter` | `HandleApMessage`, `Poll` | PID별 분석 함수 |
| `DF_Main_LedControl` | `Init`, `Poll10ms`, `Off/On/Blink/Dim` | LED step과 RGB table |
| `DF_Main_DeviceCheck` | `Start`, `Poll`, `Clear`, `IsReady` | 장치별 check step |
| `DF_Main_LmWireControl` | command 함수, `Poll1ms/10ms`, `StopAll` | 위치/step/timeout 상태 |
| `DF_Main_GameControl` | `SetStatus`, `HandlePattern`, `Poll`, `Stop` | wave/bite/hit/hold 세부 함수와 step |
| `DF_Main_Diagnostics` | `Enter`, `Exit`, `HandleCommand`, `Poll` | 진단 세부 함수와 전원 monitor 상태 |
| `DF_Main_Input` | `Init`, `Poll1ms/10ms/100ms`, 상태 조회 | sensor filter 저장 공간 |
| `DF_Main_UpdateService` | `HandleFrame`, `Poll`, `Abort` | download buffers와 sequence |
| `DF_Main_Scheduler` | `Init`, `RunOnce` | 주기 counter 소비와 호출 순서 |

실제 header에는 외부에서 호출하는 함수만 선언한다. 기존 338개 함수 대부분은 해당 `.cpp`의 `static` 함수로 유지하며 이름 변경은 독립 번역 단위가 먼저 빌드된 뒤 별도 단계에서 수행한다.

## Main 전환 우선순위

1. Timebase/State/Hardware의 저장 공간과 compatibility 선언 위치 확정
2. formatting helper, LedControl, Input처럼 비교적 경계가 작은 기능
3. DeviceCheck와 Diagnostics
4. Communication과 CommandRouter
5. LmWireControl과 GameControl
6. Scheduler/Application 및 Foundation 제거

Main의 각 묶음 전환 후에는 Main compile/link를 실행한다. Main 전체 `.inc` 제거 전까지 Rod 코드와 protocol payload는 변경하지 않는다.
