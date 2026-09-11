# Main/Rod 독립 모듈 전환 및 검증 계획

Updated: 2026-08-28  
Status: 08-A~G 1차 구조 전환 완료, H~K 대기

## 확정 원칙

- `.ino`에는 `setup()`, `loop()`만 유지한다.
- 다른 `.cpp` 또는 `.inc`를 include하지 않는다.
- 상태는 모듈 owner `.cpp`의 file-scope 저장 공간으로 두며, 하나의 거대 전역 state 구조체로 합치지 않는다.
- 외부 호출 함수만 header에 선언하고 나머지는 `static`으로 제한한다.
- 전환 중에는 기존 Arduino `String`, PID, payload, timing과 장비 동작을 유지한다.
- callback 안전화는 구조 전환과 분리해 별도 단계에서 수행한다.
- C++98/03 문법, namespace 미사용, 고유 접두어 원칙을 적용한다.

## 현재 `.inc`별 목표

| 현재 파일 | 목표 처리 |
| --- | --- |
| `DF_Main_Foundation.inc` | State, Timebase, Hardware, Input, UpdateService 및 각 기능 owner로 해체 후 제거 |
| `DF_Main_Communication.inc` | `DF_Main_Communication.cpp/.h` |
| `DF_Main_LedApplication.inc` | `DF_Main_LedControl.cpp/.h` |
| `DF_Main_ApplicationCommands.inc` | `DF_Main_CommandRouter.cpp/.h`; registration/wire 상태는 해당 owner로 이동 |
| `DF_Main_DeviceCheck.inc` | `DF_Main_DeviceCheck.cpp/.h` |
| `DF_Main_LmWire.inc` | `DF_Main_LmWireControl.cpp/.h` |
| `DF_Main_GameControl.inc` | `DF_Main_GameControl.cpp/.h` |
| `DF_Main_Diagnostics.inc` | `DF_Main_Diagnostics.cpp/.h`; 전원/input helper는 owner로 이동 |
| `DF_Main_Scheduler.inc` | Application, Scheduler, Input, Timebase, UpdateService로 해체 후 제거 |
| `DF_Rod_Foundation.inc` | BoardConfig, State, Hardware, Timebase, Communication owner로 해체 후 제거 |
| `DF_Rod_Communication.inc` | Communication과 OutputControl로 분리 |
| `DF_Rod_SetupProtocol.inc` | `DF_Rod_SetupProtocol.cpp/.h` |
| `DF_Rod_BatteryMonitor.inc` | `DF_Rod_BatteryMonitor.cpp/.h` |
| `DF_Rod_PowerAndImu.inc` | PowerControl과 ImuService로 분리 |
| `DF_Rod_Registration.inc` | `DF_Rod_Registration.cpp/.h` |
| `DF_Rod_SchedulerAndInput.inc` | Application, Scheduler, Timebase, InputControl로 해체 후 제거 |

## 작업 08 실행 순서

### 08-A 현재 unity 구조 기준 build

- 변경 전 Main/Rod compile/link를 실행한다.
- 실패하면 구조 전환 전에 현재 실패 원인과 최초 error를 고정한다.
- 이전 구조 변경 전 바이너리를 현재 기준 성공으로 사용하지 않는다.

### 08-B owner 기반 공통 선언과 ODR 정리

- Timebase, State, BoardConfig, Hardware owner 위치를 만든다.
- Foundation의 함수 원형 묶음을 기능 header로 분리한다.
- 기존 driver의 raw `extern`은 compatibility 목록을 만들어 중복 저장 공간 없이 연결한다.

### 08-C Main leaf 모듈 전환

- formatting/Input/LedControl/DeviceCheck 순으로 독립 번역 단위를 만든다.
- 각 묶음 후 Main compile/link를 수행한다.

### 08-D Main control 모듈 전환

- Diagnostics, Communication, CommandRouter, LmWireControl, GameControl 순으로 전환한다.
- 순환 호출은 Hardware/State API 또는 Scheduler 단방향 호출로 끊는다.

### 08-E Main application 정착

- Scheduler, Timebase, UpdateService와 Application을 분리한다.
- Main의 모든 `.inc` include를 제거하고 동일 구현을 대응 `.cpp`로 전환한다. 구현 내용이 계속 활성 상태이므로 중복 deprecated 사본은 만들지 않는다.
- Main clean compile/link와 필수 flash 파일 4개를 확인한다.

### 08-F Rod leaf 모듈 전환

- BoardConfig/Timebase/State/Hardware 다음 Battery/Input/Output/Imu/Power 순으로 전환한다.
- 각 묶음 후 Rod compile/link를 수행한다.

### 08-G Rod control/application 정착

- SetupProtocol, Registration, Communication, Scheduler/Application 순으로 전환한다.
- Rod의 모든 `.inc` include를 제거하고 동일 구현을 대응 `.cpp`로 전환한다. 구현 내용이 계속 활성 상태이므로 중복 deprecated 사본은 만들지 않는다.
- Rod clean compile/link와 필수 flash 파일 4개를 확인한다.

### 08-H Internal compatibility header 축소와 상태 owner/API 정착

- `DF_Main_Internal.h`, `DF_Rod_Internal.h`의 raw global `extern`을 실제 owner module header로 이동한다.
- module 내부에서만 사용하는 상태와 함수는 `.cpp` file-scope 및 `static`으로 제한한다.
- 기존 driver의 application 전역 역참조를 compatibility API로 단계적으로 대체한다.
- Main/Rod compile/link를 각 묶음 후 수행한다.

### 08-I callback 및 ISR 경계 안전화

- `data == NULL`, `mac_info == NULL`, `data_len < 2`, `data_len >= buffer size`를 처리한다.
- callback에서는 고정 버퍼 복사와 flag/address 저장만 수행한다.
- `String`, substring, logging, PID 해석과 장비 제어는 loop의 Communication poll로 이동한다.
- ISR 공유 값의 `volatile`, critical section과 단일 writer/reader를 확인한다.
- Main/Rod compile/link를 다시 수행한다.

### 08-J protocol host test와 정적 정책 검사

- `DFProtocol` host test와 golden fixture를 실행한다.
- 활성 코드에서 `.inc` include, 새 Variant 전처리 분기, namespace 및 C++11 이후 문법이 없는지 검색한다.
- build 결과물 4개 복사와 버전 경로를 확인한다.

### 08-K 사용자 수동 장비 게이트

- Main 단독 부팅/출력 안전 상태
- Rod 단독 입력/IMU/battery/절전
- Main–Rod 등록, 재연결, 장시간 송수신
- 최대 길이/비정상 frame, 연속 수신, 전원 재투입

이 단계에서는 Codex가 명령과 체크리스트를 제공하고 멈춘다. 사용자 결과를 받기 전에는 08-K를 완료로 기록하지 않는다.

## 검증 판정

| 게이트 | 실행 주체 | 필수 근거 |
| --- | --- | --- |
| 문서 일관성 | Codex | 경로, owner, 작업 번호, 잔여 수 일치 |
| compile/link | Codex | 종료 code 0, 최초 error 없음 |
| firmware 산출물 | Codex | 대상별 지정된 4개 파일 |
| protocol host test | Codex | test 종료 code 0과 fixture 결과 |
| flash/smoke/HIL | 사용자 | 실행 버전, 시나리오별 결과, 이상 로그 |

## 알려진 선행 위험

- 양쪽 수신 callback은 128바이트 지역 배열에 길이 상한 확인 없이 복사하고 `data_len` 위치에 종료문자를 쓴다.
- callback에서 `String`, substring 및 log가 수행된다.
- Main Foundation은 ISR, OTA 대형 buffer, game/diagnostics/hardware 상태를 동시에 소유한다.
- 여러 driver header가 `curr_ms_tick`, `gameStatus`, STX 문자열과 application 함수를 raw `extern`으로 참조한다.
- Main/Rod에 동일한 전역 함수/변수명이 있어 고유 접두어 전환이 필요하다.
- 1차 구조 전환은 compile/link되었으나 `DF_*_Internal.h`가 아직 넓은 compatibility 경계를 노출한다.

이 위험은 작업 08에서 해결하거나 검증할 대상이며 작업 07 완료로 해소된 것으로 간주하지 않는다.

## 1차 구조 전환 결과

- 2026-08-28 08-A~G 완료
- 활성 `.inc` 및 `.inc` include 0개
- Main/Rod module별 `.cpp/.h`와 Application setup/loop owner 정착
- VS2022 project compile/include 항목 등록 및 전체 경로 존재 확인
- Arduino CLI 통합 clean build와 VS2022 Solution Rebuild 성공
- `DF_*_Internal.h` 축소, callback 안전성, protocol test 및 장비 검증은 H~K에 남음
