# DF\_Main 개발환경 전환 및 단계별 리팩토링 계획

## 1\. 문서 목적

본 문서는 `Vm1.0.9.0.zip`을 현재 최신 소스로 간주하고, 다음 두 목표를 안전하게 달성하기 위한 개발 기준을 정의한다.

1. **Visual Micro 없이 Visual Studio 2022를 주 개발 IDE로 사용**
2. **현재 22,000줄 이상인 `DF\_Main.ino`를 일반 C++ 구조로 단계적으로 분리·리팩토링**

가장 중요한 원칙은 다음과 같다.

> \*\*개발환경 변경과 로직 리팩토링을 동시에 수행하지 않는다.\*\*

먼저 기존 Arduino 빌드 환경과 동일한 결과를 재현할 수 있게 만든 뒤, 파일 분리와 구조 개선을 단계적으로 진행한다.

\---

# 2\. 현재 프로젝트 분석 결과

## 2.1 프로젝트 개요

현재 ZIP 기준 주요 소스는 다음과 같다.

|파일|역할|규모|
|-|-|-:|
|`DF\_Main.ino`|메인 제어 및 대부분의 업무 로직|약 22,715줄|
|`Common.h`|공통 정의, GPIO, 프로토콜, 상수|약 789줄|
|`Config.h`|제품/기능별 컴파일 옵션|약 378줄|
|`Version.h`|펌웨어 버전 및 이력|약 835줄|
|`BobbinMotor.cpp/.h`|Bobbin BLDC 제어|약 295줄|
|`ENow.cpp/.h`|ESP-NOW 통신|약 428줄|
|`Eeprom.cpp/.h`|EEPROM 제어|약 315줄|
|`Encoder.cpp/.h`|Encoder 제어|약 286줄|
|`FileSys.cpp/.h`|SPIFFS 파일 관리|약 394줄|
|`Led.cpp/.h`|LED 제어|약 300줄|
|`LmMotor.cpp/.h`|LM Motor 제어|약 245줄|
|`Potentiometer.cpp/.h`|아날로그 입력|약 191줄|
|`ServoMotor.cpp/.h`|Servo 제어|약 148줄|
|`Diag.cpp/.h`|진단 기능|약 152줄|

`DF\_Main.ino` 내부에는 대략 **357개의 함수**가 존재한다.

따라서 이 프로젝트의 핵심 문제는 파일 개수가 아니라:

* 지나치게 큰 `DF\_Main.ino`
* 많은 전역 상태
* 기능별 코드의 물리적 혼재
* 여러 상태머신과 주기 태스크가 하나의 파일에 존재
* 컴파일 옵션이 `Config.h`에 광범위하게 분포

하는 점이다.

\---

## 2.2 대상 MCU

빌드 출력 디렉터리가 다음과 같이 존재한다.

```text
build/
└─ esp32.esp32.esp32s3/
```

따라서 현재 대상은 **ESP32-S3**로 판단한다.

기존 빌드 산출물도 포함되어 있다.

```text
DF\_Main.ino.bin
DF\_Main.ino.bootloader.bin
DF\_Main.ino.partitions.bin
DF\_Main.ino.elf
DF\_Main.ino.map
```

이는 향후 리팩토링 시 매우 중요한 기준 자료가 된다.

\---

## 2.3 주요 프레임워크 및 외부 의존성

현재 코드에서 확인되는 주요 의존성은 다음과 같다.

### ESP32 Arduino Core

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp\_now.h>
#include <esp\_wifi.h>
#include <esp\_pm.h>
#include <esp\_sleep.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <SPI.h>
#include <Update.h>
```

### 별도 라이브러리

```cpp
#include <ESP32Servo.h>
```

현재 프로젝트는 단순 Arduino API만 사용하는 것이 아니라 다음이 혼합되어 있다.

* Arduino API
* ESP32 HAL
* ESP-NOW
* FreeRTOS 관련 기능
* Timer Interrupt
* GPIO Interrupt
* SPIFFS
* I²C EEPROM
* UART
* OTA/Update
* Motor / Encoder / PWM 제어

따라서 초기 리팩토링 단계에서는 **ESP-IDF 네이티브 프로젝트로 즉시 변경하지 않는다.**

\---

# 3\. 개발환경 최종 목표

## 3.1 권장 구성

1차 목표 개발환경은 다음과 같이 한다.

```text
Visual Studio 2022
      │
      │ 편집 / 탐색 / Git / 코드 분석
      │
      ▼
Arduino CLI
      │
      │ 기존 Arduino 빌드 규칙 유지
      ▼
ESP32 Arduino Core
      │
      ▼
ESP32-S3 Firmware
```

### 선택 이유

* Visual Micro 라이선스가 필요 없음
* Arduino IDE에 종속되지 않음
* 기존 펌웨어 빌드 방식을 최대한 그대로 유지
* 리팩토링 과정에서 빌드 시스템 변화에 따른 변수를 최소화
* 향후 GitHub Actions 등 CI 빌드로 확장 가능
* Arduino CLI 명령을 배치 파일 또는 VS2022 Build Target과 연결 가능

\---

# 4\. 당장은 ESP-IDF로 전환하지 않는 이유

ESP-IDF + Arduino Component 방식도 장기적으로 좋은 구조이지만, 현재 단계에서는 권장하지 않는다.

현재 동시에 바뀌게 되는 것이 너무 많기 때문이다.

```text
기존
Arduino Build System
+ 22,715줄 INO
+ 기존 ESP32 Core
+ 기존 라이브러리

        ↓

동시에 변경할 경우

ESP-IDF/CMake
+ Arduino Component
+ 소스 구조 변경
+ include 구조 변경
+ 전역변수 구조 변경
+ ISR/Timer 구조 확인
```

문제가 발생했을 때 원인이 다음 중 무엇인지 알기 어려워진다.

* 환경 문제
* ESP32 Core 버전 문제
* CMake 문제
* ESP-IDF 문제
* 파일 분리 문제
* 전역 변수 문제
* 초기화 순서 문제
* 실제 리팩토링 버그

따라서 **빌드 환경 유지 → 소스 구조 개선 → 필요하면 ESP-IDF 전환** 순서를 사용한다.

\---

# 5\. 개발환경 고정

리팩토링 전에 반드시 현재 빌드 환경 정보를 기록한다.

## 5.1 반드시 고정할 항목

다음 항목을 `docs/BUILD\_ENVIRONMENT.md` 등에 기록한다.

```text
Arduino CLI Version
ESP32 Arduino Core Version
Board FQBN
Partition Scheme
Flash Size
Flash Mode
Flash Frequency
CPU Frequency
USB CDC On Boot
USB DFU On Boot
Upload Mode
Upload Speed
PSRAM 설정
ESP32Servo Version
사용 COM Port
```

특히 다음 네 가지는 반드시 확인한다.

```text
ESP32 Arduino Core Version
Board FQBN
Partition Scheme
USB CDC On Boot
```

현재 ZIP만으로는 정확한 Arduino Core 버전을 확정하지 않는다.

현재 실제 빌드가 성공하는 PC에서 버전을 확인하고 고정해야 한다.

\---

# 6\. Arduino CLI 구성

예시 설치 후 다음을 확인한다.

```bat
arduino-cli version
```

설치된 ESP32 Core 확인:

```bat
arduino-cli core list
```

보드 검색:

```bat
arduino-cli board listall esp32s3
```

현재 사용 중인 정확한 Board FQBN을 확인한 뒤 스크립트에 고정한다.

예:

```bat
set BOARD=esp32:esp32:esp32s3
```

단, 실제 Board Option은 기존 Arduino IDE의 설정을 그대로 옮겨야 한다.

\---

# 7\. VS2022 프로젝트 운용 방식

## 7.1 VS2022의 역할

Visual Studio가 ESP32 코드를 직접 MSVC로 컴파일하면 안 된다.

Visual Studio는 다음 역할만 담당한다.

```text
코드 편집
코드 탐색
IntelliSense
Git
검색 / Replace
Refactoring 지원
Build Command 실행
Upload Command 실행
Serial Tool 실행
```

실제 ESP32 Firmware는 Arduino CLI가 컴파일한다.

\---

## 7.2 권장 프로젝트 방식

초기에는 무리하게 `.vcxproj`로 ESP32 컴파일 규칙 전체를 구현하지 않는다.

다음 중 하나를 사용한다.

### 권장 A

**VS2022 Open Folder + Arduino CLI Script**

가장 단순하고 고장 가능성이 낮다.

### 권장 B

**VS2022 Makefile/NMake Project**

Build Command를 다음과 같이 연결한다.

```text
tools\\build.cmd
```

Rebuild:

```text
tools\\rebuild.cmd
```

Upload:

```text
tools\\upload.cmd
```

이 방식이면 VS2022에서 F7 또는 Build 메뉴를 사용하면서도 실제 컴파일은 Arduino CLI가 담당한다.

\---

# 8\. 권장 Repository 구조

초기 리팩토링 완료 후 목표 구조는 다음과 같다.

```text
DF\_Main/
│
├─ DF\_Main.ino
│
├─ Common.h
├─ Config.h
├─ Version.h
│
├─ src/
│  │
│  ├─ app/
│  │  ├─ AppControl.cpp
│  │  ├─ AppControl.h
│  │  ├─ GameControl.cpp
│  │  └─ GameControl.h
│  │
│  ├─ communication/
│  │  ├─ ApProtocol.cpp
│  │  ├─ ApProtocol.h
│  │  ├─ RodProtocol.cpp
│  │  ├─ RodProtocol.h
│  │  ├─ ENow.cpp
│  │  └─ ENow.h
│  │
│  ├─ control/
│  │  ├─ BiteControl.cpp
│  │  ├─ BiteControl.h
│  │  ├─ HitControl.cpp
│  │  ├─ HitControl.h
│  │  ├─ HoldControl.cpp
│  │  ├─ HoldControl.h
│  │  ├─ WireControl.cpp
│  │  └─ WireControl.h
│  │
│  ├─ motor/
│  │  ├─ BobbinMotor.cpp
│  │  ├─ BobbinMotor.h
│  │  ├─ LmMotor.cpp
│  │  ├─ LmMotor.h
│  │  ├─ ServoMotor.cpp
│  │  └─ ServoMotor.h
│  │
│  ├─ input/
│  │  ├─ Encoder.cpp
│  │  ├─ Encoder.h
│  │  ├─ Potentiometer.cpp
│  │  └─ Potentiometer.h
│  │
│  ├─ output/
│  │  ├─ Led.cpp
│  │  └─ Led.h
│  │
│  ├─ storage/
│  │  ├─ Eeprom.cpp
│  │  ├─ Eeprom.h
│  │  ├─ FileSys.cpp
│  │  └─ FileSys.h
│  │
│  ├─ diagnostics/
│  │  ├─ Diag.cpp
│  │  ├─ Diag.h
│  │  ├─ DeviceCheck.cpp
│  │  └─ DeviceCheck.h
│  │
│  └─ system/
│     ├─ Scheduler.cpp
│     ├─ Scheduler.h
│     ├─ SystemState.cpp
│     └─ SystemState.h
│
├─ tools/
│  ├─ build.cmd
│  ├─ rebuild.cmd
│  ├─ upload.cmd
│  └─ serial.cmd
│
├─ docs/
│  ├─ BUILD\_ENVIRONMENT.md
│  ├─ REFACTORING.md
│  └─ PROTOCOL.md
│
└─ build/
```

단, **처음부터 이 구조로 한 번에 이동하지 않는다.**

\---

# 9\. Arduino 파일 분리 원칙

## 9.1 여러 `.ino` 파일로 분리하지 않는다

다음과 같은 구조는 피한다.

```text
DF\_Main.ino
Motor.ino
Game.ino
Protocol.ino
Diag.ino
```

Arduino는 여러 `.ino` 파일을 일반 C++의 독립적인 Translation Unit으로 다루지 않고 전처리 단계에서 연결한다.

따라서 이것은 실제 구조 개선 효과가 작다.

\---

## 9.2 `.cpp/.h`로 분리한다

권장 방식:

```text
GameControl.cpp
GameControl.h

ApProtocol.cpp
ApProtocol.h

DeviceCheck.cpp
DeviceCheck.h
```

각 `.cpp`는 독립적인 C++ Translation Unit으로 컴파일된다.

이것이 최종적으로 VS2022 / CMake / ESP-IDF 등으로 이동하기 쉬운 구조다.

\---

# 10\. 리팩토링 핵심 원칙

## 원칙 1 — 동작 변경과 파일 이동을 분리

예:

### 잘못된 방식

```text
함수 이동
+ 함수명 변경
+ 클래스화
+ 전역변수 제거
+ 상태머신 변경
```

를 한 Commit에 수행.

### 권장 방식

```text
Commit 1: 함수만 이동
Commit 2: include/extern 정리
Commit 3: 함수명 정리
Commit 4: 상태 구조화
Commit 5: 필요하면 클래스화
```

\---

## 원칙 2 — 최초 목표는 코드 개선이 아니라 동일 동작

첫 번째 목표는:

> \*\*22,715줄짜리 INO를 여러 CPP로 이동하되 프로그램의 동작은 단 한 줄도 바꾸지 않는다.\*\*

이다.

\---

## 원칙 3 — 전역변수를 당장 제거하지 않는다

현재 INO에는 많은 전역 상태가 존재한다.

이를 처음부터 모두 클래스 멤버로 바꾸면 리스크가 매우 커진다.

1차 분리에서는 다음 패턴을 허용한다.

```cpp
// SystemState.h
#pragma once

extern int gameStatus;
extern bool rodConnected;
extern unsigned long systemCounter;
```

```cpp
// SystemState.cpp

#include "SystemState.h"

int gameStatus = 0;
bool rodConnected = false;
unsigned long systemCounter = 0;
```

리팩토링 안정화 후 점진적으로 Context/State 구조로 변경한다.

\---

# 11\. 가장 먼저 정리해야 할 영역

현재 `DF\_Main.ino`의 함수들을 기준으로 다음 영역이 뚜렷하게 보인다.

## 11.1 통신 / Protocol

예:

```text
uartRecvParsing
uartRecvHandler
uartRecvHandlerImp
nowRecvHandler
recv\_cb\_esp\_now\_msg
sent\_cb\_esp\_now\_sts
anaGameStatus
anaDeviceInfoReq
anaDeviceContReq
anaRodRegist
```

후보 파일:

```text
ApProtocol.cpp
RodProtocol.cpp
CommunicationControl.cpp
```

\---

## 11.2 Device Check / Self Diagnostic

대규모 함수 집합:

```text
devChkReady\_Control
devchk\_bbnAll\_Control
devchk\_bbnMot\_Control
devchk\_bbnEnc\_Control
devchk\_bldc24v\_Control
devchk\_torq\_Control
devchk\_lmMot\_Control
devchk\_lmEnc\_Control
devchk\_WireBroken\_Control
devReadyCheck
```

후보:

```text
DeviceCheck.cpp
DeviceCheck.h
```

이 영역은 기능 경계가 명확하기 때문에 **초기 파일 분리 대상으로 매우 적합하다.**

\---

## 11.3 LM / Wire Control

예:

```text
lmHome\_Control\_Start
lmHome\_Control\_Stop
lmLeft\_Control\_Start
lmLeft\_Control
lmRight\_Control
lmReturn\_Control
lmPosition\_Set
wireHomeControl\_Start
wireHome\_Control
```

후보:

```text
LmControl.cpp
LmControl.h

WireControl.cpp
WireControl.h
```

\---

## 11.4 Bite Control

예:

```text
biteType\_TorqSet
execBite\_servo
execBite\_torque\_justTarget
execBite\_torque\_incTarget
execBite\_BldcStart
execBite\_BldcControl
execBite\_torquewf\_Control
execBite\_ptn1\_Control
execBite\_ptn2\_Control
execBite\_ptn3\_Control
```

후보:

```text
BiteControl.cpp
BiteControl.h
```

\---

## 11.5 Hit / Hold Control

예:

```text
execHit\_BldcStart
execHit\_1st\_BldcControl
execHiting
execHit\_ResistRandomStart
execHit\_HoldServo
execHit\_ResistTorqControl
execHit\_Holdon\_BldcControl
execHit\_HoldOn\_TorqControl
execHit\_BreakControl
```

후보:

```text
HitControl.cpp
HitControl.h
HoldControl.cpp
HoldControl.h
```

\---

## 11.6 LED / Output Control

현재 별도의 `Led.cpp`가 있지만, 메인 INO에도 다음과 같은 상위 LED 제어가 존재한다.

```text
ledCont\_OffStart
ledCont\_OnStart
ledCont\_BlinkStart
execledCont\_Blink
ledCont\_DimmStart
execledCont\_Dimm
extLed\_Control
```

권장 구분:

```text
Led.cpp
    = Hardware Driver

LedControl.cpp
    = Application-level LED behavior
```

즉 Driver와 동작 정책을 분리한다.

\---

# 12\. Driver와 Business Logic을 분리

현재 프로젝트에서 가장 중요한 구조 개선 중 하나다.

예:

```text
BobbinMotor.cpp
```

는 가능한 한 다음 역할만 담당해야 한다.

```text
PWM 출력
방향 설정
Duty 설정
정지
Encoder 관련 HW 접근
```

반대로:

```text
물고기 버티기
Bite Pattern
Hit Pattern
Game State
Random Pattern
```

은 BobbinMotor Driver에 넣지 않는다.

다음 구조가 좋다.

```text
Game / Pattern Logic
        │
        ▼
Control Layer
        │
        ▼
Motor Driver
        │
        ▼
ESP32 Hardware
```

\---

# 13\. Scheduler 정리

현재 메인에서는 다음과 같은 주기 처리 함수가 존재한다.

```text
t1ms\_Process\_Exec()
t10ms\_Process\_Exec()
t100ms\_Process\_Exec()
system\_counter()
```

그리고 `loop()`가 여러 주기 작업과 통신 처리를 수행한다.

이 구조 자체는 임베디드 시스템에서 문제가 아니다.

FreeRTOS Task로 즉시 전환할 필요도 없다.

오히려 최초 리팩토링에서는 **기존 Super Loop 구조를 유지한다.**

최종적으로:

```cpp
void loop()
{
    scheduler.run();
}
```

또는:

```cpp
void loop()
{
    systemProcess();
    communicationProcess();
    periodicProcess();
}
```

수준으로 정리할 수 있다.

\---

# 14\. Interrupt 관련 리팩토링 주의사항

다음 코드는 특별 취급한다.

```text
Timer ISR
GPIO ISR
Encoder ISR
ESP-NOW Callback
volatile 변수
portENTER\_CRITICAL
portEXIT\_CRITICAL
```

ISR 또는 Callback에서 사용하는 변수는 일반 전역변수와 동일하게 이동해서는 안 된다.

반드시 확인할 항목:

```text
volatile 여부
IRAM\_ATTR 여부
Critical Section
ISR에서 호출되는 함수
ISR Safe API 여부
ESP-NOW Callback Context
```

따라서 Interrupt 및 ESP-NOW Callback 코드는 **리팩토링 후반부에 이동한다.**

\---

# 15\. Static / 전역 초기화 순서

`.ino` 하나에 존재하던 코드가 여러 `.cpp`로 분리되면 C++ 전역 객체 초기화 순서 문제가 발생할 수 있다.

현재 확인되는 전역 객체 예:

```cpp
ENow eNow;
Potentiometer torqMotor;
ServoMotor sMotor;
BobbinMotor bbnMotor;
Encoder mainEnc;
LmMotor lmMotor;
Led extLed;
Eeprom eNvm;
FileSys fsInfo;
```

이들을 여러 CPP로 무분별하게 분산시키면 다른 Translation Unit 간 초기화 순서는 보장하기 어렵다.

따라서 1차 리팩토링에서는 이 전역 객체들을 한 곳에서 관리한다.

예:

```text
HardwareContext.cpp
HardwareContext.h
```

또는 기존 `DF\_Main.ino`에 잠시 그대로 둔다.

초기화 구조가 안정화된 뒤 변경한다.

\---

# 16\. Config.h 정책

현재 `Config.h`는 제품 Variant와 기능 Switch를 광범위하게 담당한다.

예:

```text
NEW\_IF
AP\_MOT\_CONT
CONFIG\_TORQ\_WATT\_30W
MC\_CONF\_NO\_4
BBN\_MOT\_16W
IO\_TORQ
IO\_SERVO
IO\_BBN
IO\_I2C
```

초기 리팩토링에서는 **Config.h의 의미를 변경하지 않는다.**

특히 다음 작업을 피한다.

```text
#define을 constexpr로 일괄변환
조건부 컴파일 제거
옵션명을 대규모 변경
Variant 통합
```

구조가 안정화된 뒤 별도 작업으로 진행한다.

\---

# 17\. 리팩토링 단계

# Phase 0 — 기준선 확보

목표:

> 현재 Firmware가 다시 언제든 동일하게 빌드될 수 있게 한다.

작업:

1. 원본 ZIP 별도 보존
2. Git Repository 생성
3. 현재 소스를 `baseline/v1.0.9.0` Tag로 저장
4. Arduino Core Version 기록
5. Board Option 기록
6. 기존 `.bin/.elf/.map` 보존
7. 현재 Firmware Flash 성공 확인
8. 기기의 기본 동작 확인

Git 예:

```bash
git tag baseline-v1.0.9.0
```

\---

# Phase 1 — Arduino CLI 빌드 구축

목표:

> Arduino IDE를 열지 않고 명령 한 줄로 동일 Firmware를 빌드한다.

예:

```bat
tools\\build.cmd
```

결과:

```text
BUILD SUCCESS
```

이 단계에서는 소스 코드를 변경하지 않는다.

\---

# Phase 2 — VS2022 연결

목표:

> VS2022에서 편집하고 Build를 실행하면 Arduino CLI가 호출되게 한다.

작업:

* VS2022 Solution/Folder 구성
* Source/Header 표시
* Build command 연결
* Upload command 연결
* Git 연결
* Arduino Header의 IntelliSense 설정

이 단계에서도 로직은 변경하지 않는다.

\---

# Phase 3 — 자동 Prototype 의존 제거

Arduino `.ino`는 함수 선언을 자동 생성해 주는 특성이 있다.

일반 `.cpp`로 분리하기 전에 이 의존성을 제거한다.

예:

기존:

```cpp
void setup()
{
    motorStart();
}

void motorStart()
{
}
```

명시적 선언:

```cpp
void motorStart();

void setup()
{
    motorStart();
}

void motorStart()
{
}
```

가능하면 앞으로 이동할 함수들은 해당 `.h`에 prototype을 선언한다.

\---

# Phase 4 — 가장 독립적인 기능부터 이동

첫 대상으로 다음을 권장한다.

```text
DeviceCheck
Diagnostics
문자열 Format Utility
독립적인 Command Analyzer
```

이유:

* 함수 경계가 명확함
* HW Driver보다 위험도가 낮음
* ISR과 연관이 상대적으로 적음
* 이동 후 테스트하기 쉬움

각 기능 이동 후:

```text
Compile
Flash
Basic Test
Commit
```

을 수행한다.

\---

# Phase 5 — Protocol 분리

다음 영역:

```text
UART Parsing
AP Command
ESP-NOW Message Handling
Rod Registration
Response Generation
```

권장 구조:

```text
communication/
├─ ApProtocol.cpp
├─ ApProtocol.h
├─ RodProtocol.cpp
├─ RodProtocol.h
├─ UartProtocol.cpp
└─ UartProtocol.h
```

이 단계에서는 프로토콜 문자열 자체를 변경하지 않는다.

\---

# Phase 6 — Game / Pattern Logic 분리

대용량 영역:

```text
Bite
Hit
Hold
Randing
FastHold
Combo
GameStatus
```

권장:

```text
control/
├─ BiteControl.cpp
├─ HitControl.cpp
├─ HoldControl.cpp
├─ GameControl.cpp
└─ PatternControl.cpp
```

이 단계가 완료되면 `DF\_Main.ino` 크기가 크게 줄어들 것으로 예상한다.

\---

# Phase 7 — 전역 상태 구조화

파일 이동이 안정화된 후에 수행한다.

현재:

```cpp
int xxx;
bool yyy;
unsigned long zzz;
```

형태로 분산된 상태를 기능별 구조체로 묶는다.

예:

```cpp
struct GameState
{
    int status;
    int step;
    bool running;
    uint32\_t startTime;
};
```

그러나 처음부터 Singleton이나 복잡한 DI Framework를 도입하지 않는다.

임베디드 코드이므로 단순한 명시적 구조가 우선이다.

\---

# Phase 8 — 클래스화 여부 결정

모든 모듈을 클래스로 만들 필요는 없다.

다음 기준을 적용한다.

### Class가 적합

* 독립 상태를 가짐
* 같은 종류 객체를 여러 개 만들 가능성이 있음
* HW Resource를 캡슐화할 필요가 있음

예:

```text
BobbinMotor
LmMotor
Encoder
Led
ENow
```

### Namespace + Function이 더 적합할 수 있음

* 시스템 전체에 하나뿐
* 상태가 거의 없음
* 프로토콜 Parsing
* Utility

예:

```cpp
namespace ApProtocol
{
    void process();
}
```

무조건적인 OOP 전환은 하지 않는다.

\---

# Phase 9 — Main 최소화

최종 목표:

```cpp
void setup()
{
    systemInit();
}

void loop()
{
    systemProcess();
}
```

또는:

```cpp
void loop()
{
    scheduler.run();
    communication.process();
    game.process();
}
```

`DF\_Main.ino` 목표 크기는 **300\~800줄 이하**로 잡는다.

가능하면 장기적으로는 `setup()/loop()` 외 대부분의 구현이 일반 `.cpp`에 존재하게 한다.

\---

# 18\. 단계별 목표 Line 수

정확한 수치가 목표는 아니지만 진행 상태 판단에 사용할 수 있다.

|단계|DF\_Main.ino 예상|
|-|-:|
|현재|22,715|
|Device Check 분리|약 18,000\~20,000|
|Protocol 분리|약 14,000\~17,000|
|LM/Wire 분리|약 11,000\~14,000|
|Bite/Hit/Hold 분리|약 4,000\~8,000|
|Scheduler/System 분리|약 1,000\~3,000|
|최종|약 300\~800|

Line 수 감소 자체가 목적은 아니며 **의존성 경계가 명확해지는 것**이 목적이다.

\---

# 19\. Commit 정책

대규모 리팩토링에서는 Commit 단위를 작게 유지한다.

예:

```text
refactor: move device check functions
refactor: move bite control functions
refactor: move hit control functions
refactor: extract AP protocol parser
refactor: extract scheduler
refactor: centralize system state
```

피해야 할 Commit:

```text
refactor everything
```

한 Commit에서 수정하는 영역은 가능하면 한 모듈로 제한한다.

\---

# 20\. 검증 정책

모든 파일 이동마다 전체 기능을 수동 검증할 필요는 없다.

다음 3단계로 구분한다.

## Level 1 — Compile Check

항상 수행.

```text
Compile 성공
Link 성공
Firmware 생성 성공
```

## Level 2 — Smoke Test

주요 모듈 이동 후 수행.

예:

```text
Boot
UART
ESP-NOW
Motor Enable
LED
Main input
```

## Level 3 — Full Device Test

다음 시점에 수행.

```text
Protocol 구조 변경 완료
Game Control 구조 변경 완료
Interrupt 관련 변경
Release Candidate
```

즉 파일 하나를 옮길 때마다 전체 제품 검증을 반복하지 않는다.

\---

# 21\. Binary 비교 정책

리팩토링 전후 Binary가 동일하지 않아도 곧바로 오류로 판단하지 않는다.

함수의 Translation Unit이나 Link 순서만 바뀌어도 Binary는 달라질 수 있다.

따라서 다음을 기준으로 비교한다.

```text
Firmware Size
Map File
Memory Usage
주요 Symbol 존재 여부
Runtime 동작
```

필요하면 `.map` 파일을 이전 버전과 비교한다.

\---

# 22\. 절대 한 번에 하지 않을 작업

초기 단계에서 다음 작업을 동시에 수행하지 않는다.

```text
ESP-IDF Migration
FreeRTOS Task 대규모 전환
모든 Global 제거
모든 Module Class화
Protocol 재설계
Config.h 제거
GPIO 정의 대규모 변경
Interrupt 구조 재설계
String → std::string 일괄변경
Arduino String 제거
동적메모리 최적화
```

이 작업들은 파일 구조가 안정된 이후 별도의 개선 과제로 수행한다.

\---

# 23\. 장기적인 2차 개발환경

1차 리팩토링이 완료된 후에는 다음 구조를 검토할 수 있다.

```text
Visual Studio 2022
      │
      ▼
CMake
      │
      ▼
ESP-IDF
      │
      └─ Arduino as Component
```

이 구조의 장점:

* 정식 CMake
* ESP-IDF Component 구조
* 빌드 의존성 명확
* FreeRTOS / ESP-IDF API 사용 용이
* 장기 유지보수에 유리

그러나 이는 **현재 리팩토링의 필수 조건이 아니다.**

\---

# 24\. 최종 권장 전략

본 프로젝트는 다음 순서로 진행하는 것을 권장한다.

```text
\[현재 Vm1.0.9.0]
        │
        ▼
\[Git Baseline 고정]
        │
        ▼
\[Arduino CLI로 동일 빌드]
        │
        ▼
\[VS2022 개발환경 연결]
        │
        ▼
\[INO 자동처리 의존 제거]
        │
        ▼
\[기능별 CPP/H 물리적 분리]
        │
        ▼
\[전역 상태 정리]
        │
        ▼
\[Driver / Control / Protocol 계층화]
        │
        ▼
\[DF\_Main.ino 최소화]
        │
        ▼
\[필요 시 ESP-IDF + Arduino Component]
```

핵심은 다음 세 문장으로 정리한다.

> \*\*첫째, 기존 Arduino 빌드 환경을 먼저 완전히 재현한다.\*\*

> \*\*둘째, 22,715줄을 처음부터 다시 설계하지 말고 기능 단위로 그대로 CPP/H로 이동한다.\*\*

> \*\*셋째, 물리적 파일 분리가 끝난 뒤에 전역변수·상태머신·클래스 구조를 개선한다.\*\*

이 순서를 따르면 리팩토링 중 문제가 발생하더라도 원인을 좁히기 쉽고, 언제든 직전 동작 버전으로 돌아갈 수 있다.

\---

# 25\. 1차 리팩토링 우선순위

실제 작업 시작 순서는 다음을 권장한다.

```text
1. 개발환경/빌드 고정
2. DeviceCheck 분리
3. Diagnostic 분리
4. 문자열/Utility 분리
5. AP/UART Protocol 분리
6. ESP-NOW 상위 처리 분리
7. LM/Wire Control 분리
8. Bite Control 분리
9. Hit/Hold Control 분리
10. Game Control 분리
11. Scheduler 분리
12. Global State 구조화
13. DF\_Main.ino 최소화
```

**ISR / Timer / ESP-NOW Callback 자체의 구조 변경은 후순위**로 둔다.

\---

# 26\. 완료 기준

1차 리팩토링 완료 조건:

* VS2022에서 전체 프로젝트 탐색 가능
* VS2022에서 Build 명령 실행 가능
* Arduino CLI로 재현 가능한 Build
* Arduino IDE 없이 Build 가능
* 기존 ESP32-S3 Firmware Flash 가능
* 주요 기능 정상 동작
* `DF\_Main.ino` 1,000줄 이하
* 기능별 CPP/H 분리 완료
* Protocol / Control / Driver 경계 구분
* `Config.h` 호환 유지
* ISR 및 Callback 동작 보존
* Git Commit 단위로 각 단계 Rollback 가능

2차 리팩토링 완료 조건:

* 전역 상태 최소화
* 기능별 Context/State 명확
* 긴 함수 분해
* 중복 상태머신 정리
* Driver가 업무 로직을 포함하지 않음
* Protocol Parser와 Device Control 분리
* 필요 시 ESP-IDF/CMake 전환 가능

\---

## 결론

현재 프로젝트는 Arduino라는 이유로 파일 분리가 제한되는 구조가 아니다.

오히려 이미 여러 기능이 `.cpp/.h` 형태로 구현되어 있으므로, 동일한 방식으로 `DF\_Main.ino`의 대규모 로직을 단계적으로 분리하는 것이 적절하다.

다만 Arduino의 `.ino` 자동 Prototype 생성, 전역 객체 초기화 순서, ISR/Callback, `volatile`, 조건부 컴파일과 같은 요소 때문에 **단순히 코드를 파일별로 잘라 붙이는 방식은 피해야 한다.**

따라서 이 프로젝트의 최적 전략은:

**Arduino Runtime은 유지하고, 코드 구조부터 일반적인 임베디드 C++ 프로젝트로 변화시키는 것**이다.

