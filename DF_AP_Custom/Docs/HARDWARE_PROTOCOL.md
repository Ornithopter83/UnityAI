# Hardware and PC Mode Boundary

현재 상태: Task 09 메인 보드 Serial protocol R3 복원, 실제 장비 미검증

## 실행 모드

- 원본 `SerialManager.eGameMode`: `SERIAL`, `COMPUTER`
- `GameMode.txt` 첫 줄 `COM` 또는 `SERIAL`로 장치 모드를 선택한다.
- `COMPUTER`에서는 Serial 연결 coroutine 대신 PC 초기화 경로를 사용한다.
- 현재 새 프로젝트는 PC mode를 기본으로 유지한다.
- 실제 장치 출력은 자동 활성화하지 않는다.

## 메인 보드 포트

| 항목 | 원본 값 |
|---|---|
| 기본 port | `COM4` |
| override | 환경변수 `MAINPORT` |
| baud | 115200 |
| data/parity/stop | 8-N-1 |
| flow control | None |
| DTR/RTS | true/true |
| read/write timeout | 50ms / 50ms |
| read buffer | 4096 bytes |
| line I/O | `WriteLine` / `ReadLine`, 기본 LF |

메인 보드 문자열 frame:

```text
$ + 2자리 command id + payload + % + LF
```

복원 구현:

- `Assets/Scripts/Hardware/Serial/SerialProtocol.cs`: command factory와 receive parser
- `SerialFrame.cs`: id/payload/raw frame
- `ISerialTransport.cs`: hardware-independent line transport
- `SerialCommandBus.cs`: background receive queue와 main-thread pump
- `SimulatedSerialTransport.cs`: 실제 port를 열지 않는 PC simulator
- `SystemSerialTransport.cs`: 원본 포트 설정 보존용 선택 구현

현재 Unity API profile에는 `System.IO.Ports` assembly가 없으므로 `SystemSerialTransport`는 `FISHINGGAME_SYSTEM_IO_PORTS` define 전에는 컴파일되지 않는다. assembly/package 채택은 실제 장비 검증 작업과 분리한다.

## 주요 AP → FW 명령

| Frame | 의미 |
|---|---|
| `$00%` | main alive |
| `$02{value:000}%` | main torque |
| `$04{onoff}{power:000}{time:0000}%` | sub/BLDC motor |
| `$05{value:000}%` | handle/brake |
| `$06...%` | LED control policy |
| `$080%`, `$081%` | IMU stream stop/start |
| `$10-%` | version request |
| `$1101%` | serial open 초기 명령, FW 세부 의미 미확정 |
| `$1153{rank}{pattern}%` | bite motor pattern |
| `$1153{rank}999%` | bite pattern stop |
| `$1199%` | quit/recovery 전 board close 계열, FW 세부 의미 미확정 |
| `$1500%`, `$1501%` | device all-off / status request |
| `$1800%` | AP status ACK |
| `$23...%` | reel motor 설정 command |
| `$24...%` | button LED 설정 command |
| `$25{state:00}{optional}%` | tension controller state |
| `$3001%`, `$3002%` | rod/reel change start/cancel |

## 주요 FW → AP 상태

| ID | 상태 | payload |
|---:|---|---|
| 00 | main alive | alive count reset |
| 01 | device info | rod direction 또는 change timeout |
| 02/04/05 | motor response | raw frame 보관 |
| 08 | IMU setup | success/fail |
| 09 | IMU | roll,pitch,yaw,x,y,z |
| 10 | version | main,rod,imu |
| 12 | battery | 2자리 값/상태 |
| 13 | switch | 10 left down, 11 left up, 20 right down, 21 right up |
| 14 | rotary | signed 3자리 reel position |
| 15 | device | 4자리 error code, 0000 normal |
| 18 | AP status | 00 종료 요청 |
| 20 | rod connect | 00/01/11/12/99 |
| 21 | IMU connect | 00/01 |
| 30 | rod change | 03 cancel, 04 wait, 05 success |

06, 11, 16, 19는 원본 enum에는 있으나 handler가 등록되지 않아 payload 의미가 미확정이다.

## 안전 경계

- 수신 line은 `SerialCommandBus`의 thread-safe queue에 저장하고 `PumpReceived()` 때만 consumer에 전달한다.
- PC simulator는 송신 frame을 기록하고 firmware frame을 주입하지만 COM port를 열지 않는다.
- 실제 Serial backend 연결 실패가 Title/Loading/SelectScene 흐름을 막지 않도록 PC mode를 유지한다.
- 원본 `APSTATUS 00` 처리의 `$1800%` ACK 뒤 Windows `shutdown /f /s /t 1` 실행은 복원 runtime에 연결하지 않았다.
- 원본 USB 장치 disable/enable, 모터 출력, 자동 재연결도 실제 장비 검증 전에는 연결하지 않는다.

## 카드 단말기

- 기본 port 정보는 `COM10`, 환경변수 override는 `CARDPORT`다.
- 결제는 메인 보드 ASCII frame이 아니라 `spero.vcat.SPEROW.executePay(...)` vendor DLL API를 사용한다.
- 확인된 응답은 `0000` success, `0006...` timeout이다.
- vendor DLL 내부 baud/packet은 현재 증거에서 확인되지 않아 미복원이다.

## 검증

- Unity compile/import 및 codec/simulator 검증: `UnityProject/Logs/Task09SerialProtocolFinal.log`
- 검증 범위: 송신 고정 폭, delimiter/id, switch, rotary, IMU, version, battery, device, connection, rod change, malformed frame, simulator queue
- 실제 COM port와 물리 장치는 열지 않았다.

세부 근거와 전체 명령표는 `tasks/09_serialCommandProtocolRestoration.md`를 참조한다.
