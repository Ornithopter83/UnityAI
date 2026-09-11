# DF Main 하드웨어 기준선

Updated: 2026-08-27

## 범위와 판정 기준

이 문서는 `Vm1.0.9.0/DF_Main`의 활성 컴파일 설정과 `setup()` 초기화 경로에서 확인한 현재 Main 구성이다. 코드로 확인한 사실과 실제 장비 확인이 필요한 항목을 분리한다.

## 코드에서 확인된 현재 구성

| 항목 | 현재 코드 기준 | 근거 |
| --- | --- | --- |
| MCU 계열 | ESP32-S3 | 기존 빌드 경로 `esp32.esp32.esp32s3`, Core 2.0.17 map |
| 펌웨어 버전 | `Vm1.0.9.0` | `Version.h` 활성 `mainVer` |
| 운용 구성 | Normal/REL, LM-JIG 아님 | `CONF_LM_JIG=0` |
| Main torque motor | 활성, 30 W 구성 | `IO_TORQ=1`, `CONFIG_TORQ_WATT_30W=1` |
| Bobbin motor | 활성, 16 W BLDC | `IO_BBN=1`, `BBN_MOT_16W=1` |
| Bobbin encoder | 활성, GPIO interrupt | `GPIO_INTR_ENCA_ENB=1`, `attachInterrupt(...FALLING)` |
| Servo motor | 비활성 | `IO_SERVO=0`; GPIO 3은 USB 5 V 감지 입력으로 사용 |
| LM motor/JIG | 비활성 | `IO_LM_MOT=0` |
| 표시 LED | 활성 | `IO_DLED=1`, 중앙 RGB와 하단 LED |
| 전원 제어 | 활성 | `IO_PC_PWR_OFF=1`, SUB AC와 전원 스위치 처리 |
| 외부 I2C EEPROM | 활성, 주소 `0x50` | `IO_I2C=1`, `Eeprom.cpp` |
| 내부 파일시스템 | SPIFFS 사용 | `FileSys.cpp`, `ENow.cpp` |
| Main–Rod 통신 | ESP-NOW/Wi-Fi STA | `ENow::init()` |
| AP 통신 | USB CDC `Serial` | `setup()` 주석과 `Serial` command path |
| 로그 통신 | UART1 115200 8N1 | GPIO 43 TX, GPIO 44 RX |
| 외부 UART | 비활성 | `FUNC_IO_EXT_UART=0`, `IO_EXTIO_UART_PIN=0` |
| 주기 처리 | timer 0, 100 us alarm | `SYS_TIMER_INTR_ENB=1` |

## 활성 GPIO 기준

| GPIO | 기능 | 방향/비고 |
| ---: | --- | --- |
| 0 | Boot mode | 보드 고정 기능 |
| 3 | PC USB 5 V 감지 | `INPUT_PULLUP`; Servo 비활성 구성 |
| 4 | Bobbin BLDC FG | `INPUT_PULLUP` |
| 5 | Bobbin motor ON | 출력 |
| 6 | Bobbin motor direction | 출력 |
| 7 | Bobbin motor PWM | 출력, LEDC channel 0, 22 kHz/10 bit |
| 8 | I2C SDA | `Wire` 기본 핀 가정 |
| 9 | I2C SCL | `Wire` 기본 핀 가정 |
| 10 | Torque SPI MOSI | 출력 |
| 11 | Torque SPI SCK | 출력 |
| 12 | Torque SPI SS | 출력 |
| 13, 14, 21 | 중앙 RGB LED | 출력 |
| 15 | SUB AC OFF | 출력, LOW가 ON |
| 16 | 전원 스위치 | 입력, LOW active |
| 17 | Bobbin encoder A/interrupt | `INPUT_PULLUP`, FALLING interrupt |
| 18 | Bobbin encoder B | `INPUT_PULLUP` |
| 19, 20 | USB D-/D+ | ESP32-S3 USB |
| 38 | Bobbin BLDC 24 V enable | 출력, LOW가 ON |
| 39 | Board LED2 | 출력 |
| 40, 41 | Board LED3/LED4 | 출력; LM-JIG 비활성 구성 |
| 42 | 하단 LED | 출력 |
| 43, 44 | UART1 TX/RX | 로그 115200 8N1 |
| 47 | WDT output | 부팅 시 1회 toggle 후 HIGH; 주기 toggle 코드는 비활성 |
| 48 | Board type input 후보 | 런타임 판별 코드가 `DEF_NOT_USE=0`으로 비활성 |

## 빌드에 직접 영향을 주는 설정

- ESP32-S3 generic board definition과 Arduino-ESP32 Core 2.0.17이 필요하다.
- USB CDC `Serial` 사용을 위해 FQBN의 USB Mode/CDC On Boot가 현재 장비 환경과 일치해야 한다.
- SPIFFS를 포함하는 partition scheme이 필요하다.
- Flash size/mode와 PSRAM 설정은 작업 01-D에서 기존 산출물과 보드 설정을 통해 확정한다.

## 실제 장비 확인 필요

다음은 코드만으로 실제 장착 부품을 확정할 수 없으며, VS2022 솔루션 생성 자체를 막지는 않는다.

- Main PCB 실크/리비전과 실제 보드가 V1.0 계열인지 V1.1 수정 보드인지
- Torque motor가 실제 30 W 구성인지
- Bobbin motor가 실제 16 W BLDC인지
- 외부 EEPROM 장착 여부와 `0x50` 응답 여부
- 중앙 RGB/하단 LED, SUB AC, 전원 스위치, encoder 배선이 위 GPIO 표와 일치하는지
- PSRAM 장착 여부

현재 코드는 `mainBoardType=MAIN_BOARD_V10`을 기본값으로 사용하고 실제 board type 판별은 비활성이다. 따라서 PCB 리비전은 코드로 확정했다고 기록하지 않는다.

## 검증 기록

- `Config.h`, `Common.h`, 객체 생성부, `setup()`, 각 장치 `init()` 호출을 교차 확인했다.
- 소스나 컴파일 설정은 변경하지 않았다.
