# Rod 소스 의존성 및 owner 초안

Updated: 2026-08-28  
Status: 작업 07 확정 초안

## 조사 범위와 판정 방법

- 대상: `firmware/DF_Rod/src/modules/*.inc`, application 파일 및 기존 driver 헤더
- 함수 수는 반환형과 함수 본문이 있는 정의를 정규식으로 추출한 값이다.
- 아래 목표 owner와 API 이름은 작업 08에서 적용할 설계이며 아직 구현된 심볼이 아니다.

## 현재 모듈 규모

| 현재 파일 | 줄 수 | 확인된 함수 정의 수 | 현재 역할과 집중 위험 |
| --- | ---: | ---: | --- |
| `DF_Rod_Foundation.inc` | 530 | 4 | 모든 함수 원형, hardware object, 통신/시간/제품 상태와 callback이 혼재 |
| `DF_Rod_Communication.inc` | 797 | 14 | 수신 분석, VRT/LED control state와 Rod 상태 송신 |
| `DF_Rod_SetupProtocol.inc` | 365 | 17 | PC setup protocol, board type, mode 전환 및 reset |
| `DF_Rod_BatteryMonitor.inc` | 44 | 1 | battery change 보고 |
| `DF_Rod_PowerAndImu.inc` | 91 | 3 | alive timeout, sleep, IMU connection 확인 |
| `DF_Rod_Registration.inc` | 150 | 8 | registration timeout와 주기 처리 일부 |
| `DF_Rod_SchedulerAndInput.inc` | 532 | 9 | setup/loop, timer, GPIO, input, 주기 scheduler가 혼재 |

합계는 약 2,509줄, 확인된 함수 정의 56개다. Main보다 작지만 Foundation의 전역 원형과 두 개의 scheduler 계층 때문에 독립 번역 단위 전환 시 경계 정리가 필요하다.

## 전역 상태와 목표 owner

| 상태 묶음 | 현재 대표 심볼 | 현재 위치 | 목표 단일 owner |
| --- | --- | --- | --- |
| hardware object | `eNow`, `encoder`, `sBtn`, `brkMotor`, `imu`, `battery` | Foundation | `DF_Rod_Hardware` |
| 공통 시간 | `curr_ms_tick`, `curr_us_tick`, `sys_*_cnt`, `system_delay_cnt` | Foundation/Scheduler | `DF_Rod_Timebase` |
| 제품 고정 구성 | `rodBoardType`, `buttonType`, `breakType`, `battType`, `reelEncType`, `apType` | Foundation | `DF_Rod_BoardConfig` |
| 실행/연결 상태 | `isEnable`, `rodMode`, `mainStatus`, `rodSelfStatus`, `imuStatus`, polling 값 | Foundation | `DF_Rod_State` |
| ESP-NOW 수신 | `now_msg_str_cb`, `now_rcv_id_cb`, `nowChStr`, `now_overcnt` | Foundation | `DF_Rod_Communication` |
| 등록 상태 | `rodRegistMode`, `rodRegistToStep`, `btn_L_DownCnt`, `AddrWrite_LedBlink` | Foundation/Registration | `DF_Rod_Registration` |
| VRT/버튼 LED state | `vrt_*`, `btn_led_*` | Foundation/Communication | `DF_Rod_OutputControl` |
| setup protocol | `pwrOn_1st_imuRecv`, `imuIntervalTime`, `isSetupMode` | Foundation/SetupProtocol | `DF_Rod_SetupProtocol` |
| battery/IMU/power | `fRecvImuData`, `flagIMUSTOP`, timeout counters | Foundation/PowerAndImu | `DF_Rod_ImuService`, `DF_Rod_PowerControl` |
| application-local | `pwrOn1st_loop`, `resp_imu` | SchedulerAndInput | `DF_Rod_Application` |

## 확인된 교차 의존

| 호출 모듈 | 직접 의존하는 기능 |
| --- | --- |
| Communication | Registration timeout, SetupProtocol 주소/board type, hardware 송수신 |
| PowerAndImu | SetupProtocol의 board/break type 조회와 hardware IMU/power |
| Registration | Communication 상태 송신과 output control |
| SchedulerAndInput | Communication, Battery, Power/IMU, Registration, SetupProtocol 전체를 주기 호출 |
| SetupProtocol | Communication test data와 formatting helper |

현재 `DF_Rod_Foundation.inc`에는 모든 모듈 함수의 원형이 있어 include 순서로 의존을 숨긴다. 또한 Main과 Rod에 `sent_cb_esp_now_sts`, `recv_cb_esp_now_msg`, `nowRecvHandler`, `curr_ms_tick`, `eNow` 같은 동일 전역명이 존재한다. 바이너리는 독립이지만 저장소 검색과 향후 공통 test에서 충돌과 오인 위험이 있으므로 public 심볼은 `DF_Rod_` 접두어로 바꾼다.

## 기존 헤더 결합 위험

| 기존 헤더 | 외부 상태 의존 |
| --- | --- |
| `Encoder.h`, `Common.h` | `curr_ms_tick`, `curr_us_tick` |
| `SwitchButton.h` | button counter, LED output 상태, `apType`, `getButtonType` |
| `Battery.h` | `getBattType` |
| `ENow.h` | application reset 함수 |

Main과 동일하게 저장 공간을 owner `.cpp` 하나로 이동하고, compatibility header는 임시로만 사용한다. 최종 목표는 driver가 application 전역을 `extern`으로 역참조하지 않도록 함수 인자 또는 owner API를 사용하는 것이다.

## 목표 모듈과 최소 public API

| 목표 모듈 | 최소 공개 API 초안 | private로 둘 항목 |
| --- | --- | --- |
| `DF_Rod_Application` | `DF_Rod_Application_Setup`, `DF_Rod_Application_Loop` | 모드별 loop 세부 단계 |
| `DF_Rod_Timebase` | `Init`, `Run`, `GetMs`, 주기 consume API | system counters |
| `DF_Rod_BoardConfig` | board/button/break/battery/encoder type 조회 | 고정 상수 저장 |
| `DF_Rod_State` | enable/mode/연결 상태 Get/Set | 상태 저장 구조와 이전 값 |
| `DF_Rod_Hardware` | `Init`, `AllOff`, sensor/output 접근 | hardware object |
| `DF_Rod_Communication` | `Init`, `Poll`, 상태/센서 송신 | callback buffer와 receive flags |
| `DF_Rod_SetupProtocol` | `Enter`, `Exit`, `Poll` | UART parsing과 setup step |
| `DF_Rod_Registration` | `Start`, `Cancel`, `Poll` | timeout step와 LED 표시 상태 |
| `DF_Rod_InputControl` | `Init`, `Poll1ms`, callback 전달 | button/encoder 세부 상태 |
| `DF_Rod_ImuService` | `Init`, `Poll`, `SetMeasure`, `IsConnected` | IMU timing/status |
| `DF_Rod_BatteryMonitor` | `Init`, `Poll100ms`, change report 조회 | 이전 battery 값 |
| `DF_Rod_OutputControl` | VRT/LED `Start`, `Stop`, `Poll` | on/off count와 step |
| `DF_Rod_PowerControl` | alive/sleep `Poll1s`, sleep request | timeout counters |
| `DF_Rod_Scheduler` | `Init`, `RunOnce` | 주기 counter 소비와 호출 순서 |

기존 56개 함수 대부분은 owner `.cpp` 내부 `static` 함수로 둔다. Setup/normal mode 전환과 callback 처리만 필요한 최소 API로 노출한다.

## Rod 전환 우선순위

1. BoardConfig/Timebase/State/Hardware owner와 compatibility 선언 확정
2. BatteryMonitor, InputControl, OutputControl
3. ImuService와 PowerControl
4. SetupProtocol과 Registration
5. Communication
6. Scheduler/Application 및 Foundation 제거

각 묶음 뒤 Rod compile/link를 수행하며, 모든 `.inc` 제거 후 `DFProtocol` host test를 수행한다.
