# DF Rod 하드웨어 기준선

Updated: 2026-08-27

## 범위와 판정 기준

이 문서는 `Vr1.0.1.0`의 활성 상수, IO type 선택, `setup()` 및 각 객체 초기화 경로에서 확인한 현재 Rod 구성이다. 실제 장착 부품 확인이 필요한 내용은 별도로 구분한다.

## 코드에서 확인된 현재 구성

| 항목 | 현재 코드 기준 | 근거 |
| --- | --- | --- |
| MCU 계열 | ESP32-S3 | 기존 빌드 경로 `esp32.esp32.esp32s3`, Core 2.0.17 map |
| 펌웨어 버전 | `Vr1.0.1.0` | `Version.h` 활성 `rodVer` |
| Rod PCB | New Board V2 고정 | `ROD_BDTYPE_DETECT=0`, `rodBoardType=NEW_BOARD_2` |
| Button | Button V2 | GPIO 5/4, `buttonType=BUTTON_V2` |
| Button LED | 좌/우 2개 | GPIO 8/9, LOW active |
| Brake motor | 없음 | `breakType=BREAK_NO`; SPI potentiometer/PWM 초기화 안 함 |
| Vibration motor | 있음 | GPIO 12, HIGH active |
| Reel encoder | V3 magnetic Hall, 1 signal | GPIO 18, `reelEncType=REEL_ENC_V3` |
| Battery | Battery 800 V2 | GPIO 14 ADC, `battType=BATT_800_V2` |
| IMU | 활성, 외부 UART 장치 | UART1 115200 8N1, GPIO 1 RX/2 TX |
| Main–Rod 통신 | ESP-NOW/Wi-Fi STA | Normal mode에서 `ENow::init()` |
| PC/Setup 통신 | USB CDC `Serial` | 115200, 10 ms timeout |
| Debug 통신 | UART2 115200 8N1 | GPIO 43 TX/44 RX |
| 내부 파일시스템 | SPIFFS 사용 | target address와 IMU interval 저장 |
| Setup mode | GPIO 48 | LOW Normal, HIGH Setup이라는 코드 주석 기준 |
| Sleep | 비활성 | `SLEEP_SET=0` |
| Kalman filter | 비활성 | `BAT_KAL_FILTER=0`; Kalman library 불필요 |

`REEL_ENC_V3`는 값이 `2`인 type 상수이면서 매크로로 정의되어 있어 `#ifdef REEL_ENC_V3`가 참이 되고 V3 경로가 선택된다.

## 활성 GPIO 기준

| GPIO | 기능 | 방향/비고 |
| ---: | --- | --- |
| 0 | Boot mode | 보드 고정 기능 |
| 1, 2 | IMU UART1 RX/TX | 115200 8N1 |
| 4 | 왼쪽 Button V2 | 입력 |
| 5 | 오른쪽 Button V2 | 입력 |
| 6 | Rod/스위치 보드 Green LED | 출력 |
| 7 | Rod main board Red LED | 출력 |
| 8 | 왼쪽 button LED | 출력, LOW active |
| 9 | 오른쪽 button LED | 출력, LOW active |
| 12 | Vibration motor ON | 출력, HIGH active; Brake SPI는 비활성 |
| 14 | Battery level ADC | 입력 |
| 18 | Reel encoder V3 Hall sensor | `INPUT_PULLUP` |
| 39 | Board type detect 후보 | `ROD_BDTYPE_DETECT=0`으로 판별 경로 비활성 |
| 43, 44 | Debug UART2 TX/RX | 115200 8N1 |
| 48 | Setup mode | 입력 |

## 비활성 구 보드 경로

- Button V1의 GPIO 41 경로
- Brake motor용 GPIO 10/11/12 SPI, GPIO 15 DC 24 V, GPIO 38 PWM 경로
- Reel encoder V1/V2의 2-signal 또는 이전 ratio 경로
- Battery 3350 V1 환산 경로
- 런타임 board type 자동 판별 경로

이 경로들은 작업 03에서 현재 제품 코드와 분리하며, 제거되는 코드는 `deprecated/`에 보존한다.

## 빌드에 직접 영향을 주는 설정

- ESP32-S3 generic board definition과 Arduino-ESP32 Core 2.0.17이 필요하다.
- USB CDC `Serial`, SPIFFS partition, Flash/PSRAM 설정은 Main과 동일한 FQBN 검토 대상이다.
- Rod 활성 코드에는 별도 외부 library가 필요하지 않다. `Kalman.h` 관련 코드는 비활성이다.

## 실제 장비 확인 필요

다음은 VS2022 솔루션 생성 자체를 막지는 않지만 장비 기준선 완료 전에 확인해야 한다.

- Rod PCB가 실제 New Board V2인지
- 버튼 배선이 GPIO 5/4인 Button V2인지
- brake motor가 없고 GPIO 12 vibration motor를 사용하는지
- Battery 800 V2 및 GPIO 14 ADC 분압 회로가 현재 환산값과 일치하는지
- Reel encoder가 V3 Hall sensor이고 GPIO 18 단일 신호인지
- IMU 모델과 UART 배선/115200 설정이 일치하는지
- GPIO 48 setup mode active level이 실제 회로와 일치하는지

## 검증 기록

- `Common.h`, `Version.h`, type 초기값, `settingIOtype()`, `setup()` 및 각 장치 `init()` 분기를 교차 확인했다.
- 소스나 컴파일 설정은 변경하지 않았다.
