# DF Main/Rod protocol contract

Updated: 2026-08-27

## Wire format

ESP-NOW application message는 ASCII 두 자리 PID 뒤에 payload를 이어 붙인다. delimiter와 terminating NUL은 무선 전송 길이에 포함하지 않는다. 현재 고정 송신 버퍼는 128바이트이므로 계약상 wire 길이는 최대 127바이트다.

```text
byte 0..1 : decimal PID (`00`..`99`)
byte 2..N : command-specific payload
```

## Main to Rod

| PID | 이름 | 현재 payload |
| ---: | --- | --- |
| 05 | PowerOn | 현재 상태 문자열 |
| 07 | RodInfoRequest | 없음 |
| 09 | Alive/SleepCheck | 없음 또는 sleep mode 한 자리 |
| 11 | Break | 기존 break 제어 문자열 |
| 13 | ImuDataControl | IMU 출력 제어 문자열 |
| 19 | VerticalMotor | count/on/off 제어 문자열 |
| 21 | VersionRead | 없음 |
| 27 | ButtonLed | 위치/동작/count/on/off 문자열 |
| 29 | AllOutputsOff | `00` |
| 31 | MainAddress | MAC address 문자열 |
| 33 | ApInfo | AP/Main 정보 문자열 |
| 34 | Sleep | sleep enable 설정 문자열 |

## Rod to Main

| PID | 이름 | 현재 payload |
| ---: | --- | --- |
| 02 | BoardType | board 정보 문자열 |
| 10 | Alive | 없음 |
| 14 | ImuData | IMU 측정 문자열 |
| 16 | Button | button 상태 문자열 |
| 18 | Encoder | encoder count 문자열 |
| 22 | Version | Rod 및 IMU version 문자열 |
| 24 | ImuConnection | IMU 연결 상태 문자열 |
| 26 | Battery | 3자리 battery level 문자열 |
| 32 | RodAddress | MAC address 문자열 |
| 34 | Sleep | sleep/wakeup 상태 문자열 |

PID 번호, payload 조합 순서와 기존 문자열 형식은 이번 정리에서 변경하지 않는다. 코드 계약은 `libraries/DFProtocol/src/DFProtocol.h`가 단일 기준이다.

## Golden messages

| 방향 | 의미 | byte 문자열 |
| --- | --- | --- |
| Main → Rod | Rod 정보 요청 | `07` |
| Main → Rod | Alive | `09` |
| Main → Rod | 전체 출력 OFF | `2900` |
| Main → Rod | AP type 정보 | `330100` |
| Rod → Main | Alive 응답 | `10` |
| Rod → Main | Battery 95% 예시 | `2695%` |

Golden 문자열은 현재 결합 규칙의 대표 fixture이며 실제 장비 결과로 검증하지 않은 상태다.
