# DF Main/Rod 통합 펌웨어 현재 작업 상태

## 로컬 main 복구·실패 연구 격리 (2026-09-09)

- 현재 로컬 `main`은 `8b2fd4f`이며 upstream을 해제했다. Main `Vm1.0.10.0`, Rod `Vr1.0.2.0`과 실패 보정 UI가 제거된 TM V041이 현재 작업 기준이다.
- `origin/main`에는 실패 연구 중간 코드가 남아 있으므로 사용자 지시 없이 pull·merge·reset·push하지 않는다.
- 실패 연구는 `codex/imu-calibration-experiment-failed` (`3117e6e`)에 보관했다. 실패 사유는 `roll값과의 회전이 상쇄되거나 가중되어 좌우 대칭으로 회전이 일어나지 않는 문제 해결안됨`이다.
- 활성 작업은 없다. 과거 IMU 보정 연구를 자동 재개하거나 재검증하지 않는다.
- 이번 정리에서 빌드·테스트·게시·플래시는 수행하지 않았다.

## TM V041 실패 보정 UI 제거 (2026-09-09)

- 자동 보정 연구 종료에 맞춰 TM에서 정지 문턱값, Yaw 보정 배율, 가상 Roll, 게임 좌우, 보정 상태와 충전 기준 보정 버튼을 제거했다.
- 45도 제한을 포함한 TM 게임 좌우 계산 코드와 관련 Main 설정 명령도 제거했다.
- 장비 확인용 원본 Roll/Pitch/Yaw/GX/GY/GZ, 회전 박스와 TM 전용 0점 보정은 유지했다.
- 최신 모터 UI, OTA 기능, V041 파일명과 `CodexManage\bin\testmodule\Release\win-x64` 게시 경로는 유지했다.
- 빌드·게시·화면 검증은 수행하지 않았다.

Updated: 2026-09-02

## 작업 11-K LM JIG 실제 제어 완료 (2026-09-02)

- 컴파일 Variant를 다시 켜지 않고 기존 런타임 보드 판별값 `dfConfig == DF_CONFIG_LMJIG`에서만 LM 핀, 센서, PWM, `$07` 명령을 활성화했다. 일반 MAIN에서는 `$07`을 예약 명령으로 유지한다.
- 1.0.9.0의 LM 핀과 10bit 반전 PWM 구동을 복구했다. 외부 명령 duty는 0~255로 받되 기존 안전 상한 192를 유지하며, 정지 시 24V/OFF와 PWM 1023을 출력한다.
- 직접 제어 `$070%`, `$071dddtttt%`, `$072dddtttt%`와 위치 제어 `$0750%`~`$0754ddd%`를 연결했다. 직접 제어 타이머는 새 명령마다 초기화한다.
- 홈·좌·우 위치 제어는 10ms 상태 처리에서 센서를 감시하며 목표 센서 도달 또는 3초 timeout에서 반드시 LM 모터를 끈다. GPIO 40/41 보드 LED와 GPIO 47 WDT는 LM JIG 모드에서 LM 출력/엔코더와 충돌하지 않도록 분리했다.
- TestModule의 비활성 경고를 제거하고 LM JIG 보드에서만 동작한다는 안내와 3초 안전 정지 조건을 표시했다.
- MAIN clean build 성공: application 888,768 bytes, 동적 메모리 153,788 bytes, `bin/release/x64/Vm1.0.10.0/DF_Main.ino.bin`, SHA-256 `A64B04C4F304E02B2573BE6236302CF93E78757EDC019BFF430260C8BDDE122C`.
- TestModule Release 단일 EXE build 및 self-test exit 0: 78,445,404 bytes, SHA-256 `18D1A23EF32B78C91CAFF134CE441C065ED1DFD08B82D29AF3CB7AB517F6D289`.
- 플래시와 LM JIG 장비 동작 시험은 수행하지 않았다. 사용자 지시로 이를 별도 잔여 작업으로 만들지 않으며, 활성 잔여 작업은 0개다.

## 작업 11-J 무선 업데이트 보완 완료 (2026-09-02)

- ROD 무선 업데이트 시작 전 확인 경고를 제거했다. MAIN 연결과 ROD 연결 조건 및 폴더 검증은 유지한다.
- 현재 프로토콜은 128-byte 수신 버퍼 안에서 19-byte frame overhead를 제외한 96-byte payload를 사용하고 data frame마다 ACK를 기다린다. 정상 경로의 인위적 delay는 없으며, 기존 Main/Rod와 호환되는 범위에서 이미 최대 전송량이므로 frame 크기와 115200bps를 유지했다.
- 전송 완료 즉시 `$00%`을 한 번 보내고 2초 polling 타이머를 완료 팝업 전에 재개한다. 최대 12초 동안 `$2001%` ROD 재연결을 기다린 뒤 `$10%`를 한 번 조회한다.
- 성공 팝업은 전송 완료와 ROD 재연결 확인 여부를 함께 표시한다. 실패 경로에서도 MAIN 포트가 열려 있으면 polling을 즉시 복구한다.
- Release 단일 EXE 빌드 및 self-test exit 0. `bin/testmodule/Release/win-x64/DFTestModule.exe`, 78,445,404 bytes, SHA-256 `8B7AF98AA7094EFF6C2A403A4505460B5911E5D9FDE979B15E2682E9FED11F68`.
- 11-J 완료 후 11-K LM JIG 실제 제어를 이어서 수행했으며, 현재는 두 작업 모두 완료됐다.

## 활성 범위 재정리 (2026-09-02)

- 사용자 지정 범위였던 `11-J 무선 업데이트 보완`과 `11-K LM JIG 실제 제어`를 모두 완료했다.
- 작업 01의 잔여 기준선 확인, 작업 08-J/K, 작업 09 전체, 작업 10-H 실패 복구/마감, 작업 11-I 포괄 장비 gate는 활성 범위에서 제외했다. 과거 결과와 파일은 삭제하지 않는다.
- 활성 잔여 작업은 0개다.

## 작업 11 업데이트 직후 첫 연결 로그 기반 재보완 (2026-09-01)

- `DFLOG[202609011503].txt`에서 포트 연결 직후 과거 `$2001/$2101/$1800` 프레임이 한꺼번에 수신되고, TestModule의 자동 `$1800%` 응답 뒤 `$2211` MAIN reset 통지가 발생한 사실을 확인했다. 사용자가 업데이트 뒤 약 10초 기다린 것과 무관하게 포트/드라이버에 남은 수신 데이터가 새 연결에 전달된 경우다.
- 포트를 연 직후 입력·출력 버퍼를 비우고 나서 `DataReceived`를 연결하며, 연결할 때마다 frame decoder의 미완성 프레임도 초기화한다.
- `$1800%`는 숨기지 않고 수신 로그와 연결당 한 번의 경고를 남긴다. TestModule은 실제 Windows 종료를 수행하지 않으므로 종료 완료 ACK를 자동 송신하지 않는다. 아래의 과거 자동 응답 기록은 이 항목으로 대체한다.
- 초기 명령은 기존 500ms 3단계와 단계 내부 60ms 간격을 유지한다. `$10%` 버전 조회는 첫 단계에서 제거하고 ROD `$20xx`, IMU `$21xx` 상태를 모두 받거나 최대 2초를 기다린 뒤 한 번만 보낸다. 따라서 연결 전 상태의 `Vs99.99.99/Vi99.99.99`가 화면에 고정되는 문제를 줄인다.
- 표준 Release 단일 EXE 게시와 self-test를 완료했다. `bin/testmodule/Release/win-x64/DFTestModule.exe`, 78,445,404 bytes, SHA-256 `6F300C57328983138A3790F407CCA86F3E0C49645DFFA738428E998907D64819`.
- 실제 업데이트 직후 장비 재연결 시험은 수행하지 않았다. 작업 11-I 장비 gate와 전체 잔여 번호 5개를 유지한다.

## 작업 11 업데이트 후 첫 연결 초기화 보완 (2026-09-01)

- 업데이트 후 첫 연결에서 ROD/IMU 정보가 없고 $1800%가 반복되지만 재연결하면 정상인 원인을 초기 명령 burst로 확인했다.
- 새 TestModule은 포트를 열자마자 8개 명령을 연속 송신했다. MAIN parser는 rcved_flag 한 건만 보관하므로 처리 전에 도착한 후속 명령이 버려질 수 있다. 특히 $1101%가 누락되면 PROG_START가 되지 않아 후속 $00%에서 sendCurrentStatus()가 실행되지 않는다.
- 레거시 TestModule의 500ms 3단계 초기화 흐름을 복원했다. 포트 오픈 후 500ms 대기하고 $00/$290101/$1101, 다음 500ms 후 $00/$10/$080, 다음 500ms 후 $00/$0112/$0110/$1501 순으로 보낸다. 같은 단계 안에서도 60ms 간격을 둔다.
- 초기화가 끝나기 전에는 일반 2초 polling을 시작하지 않는다. 연결 버튼도 초기화 동안 비활성화한다. 재시도 없이 한 시퀀스만 실행한다.
- $1800% MAIN→AP 종료 요청은 숨기지 않고 연결당 한 번 $1800%로 응답하는 앞선 처리를 유지한다.
- Release compile 경고 0/오류 0, self-test exit 0, 단일 EXE 게시 성공. 78,444,892 bytes, SHA-256 42463DC0003174E03E43099B1D1F29B86F1190B2CFD018D4F478CBD6A41A4450.
- 실제 업데이트 직후 장비 연결 시험은 수행하지 않았다. 작업 11-I 장비 gate와 전체 잔여 번호 5개를 유지한다.
## 작업 11 AP 종료 응답·ROD 유선 esptool 보완 (2026-09-01)

- MAIN 버튼의 (복구) 표기를 제거했다. MAIN 기능은 내장 esptool 네 영역 직접 기록을 그대로 유지한다.
- $1800%는 polling 응답이 아니라 MAIN이 AP에 보내는 PC 종료 요청이다. MAIN은 전원 스위치 OFF 또는 자동 재부팅 조건에서 500ms마다 재전송하며 AP의 동일 $1800% 응답을 기다린다.
- TestModule을 AP로 처리해 연결당 첫 $1800% 수신을 로그에 표시하고 동일 프레임을 한 번 응답한다. RECV $1800%, SEND $1800%와 $00% polling을 숨기지 않는다. Windows 종료 명령은 실행하지 않는다.
- ROD 유선 업데이트도 기존 $DN application 전송에서 내장 esptool 방식으로 변경했다. ROD 폴더의 bootloader, partitions, boot_app0, application 네 파일과 Vr 버전을 검사하고 0x0/0x8000/0xe000/0x10000에 기록한다.
- MAIN/ROD 유선 updater와 package 검증 코드를 공통화했다. ROD 무선 업데이트는 기존 MAIN 경유 application OTA를 유지한다.
- Release compile 경고 0/오류 0, self-test exit 0, 단일 EXE 게시 성공. 78,442,332 bytes, SHA-256 856148C7E06CE948B70F0BFCF80CE7345C8B0E7DE58414CECAC4F45BAC3E6248.
- 실제 ROD flash와 장비 동작 검증은 수행하지 않았다. 작업 11-I 장비 gate와 전체 잔여 번호 5개를 유지한다.
## 작업 11 MAIN ROM 복구 업데이트 통합 (2026-09-01)

- 기존 $DN 방식은 실행 중인 MAIN 펌웨어가 정상적으로 시리얼 명령을 처리해야 한다. SPIFFS 초기화 실패나 불완전 설치로 부팅이 반복되면 첫 블록 전송 전에 막히므로 MAIN 업데이트를 ESP32-S3 ROM 부트로더 방식으로 교체했다.
- testModule/esptool_V4.5.1.exe를 단일 배포 EXE의 내장 리소스로 포함한다. 실행할 때 임시 폴더로 추출하고 종료 후 삭제하므로 배포 폴더에는 DFTestModule.exe 한 파일만 남는다.
- testModule/update.bat의 핵심 명령을 코드로 이식했다. 선택 포트에 921600bps, default_reset, hard_reset, 압축 쓰기를 사용하고 0x0 bootloader, 0x8000 partitions, 0xe000 boot_app0, 0x10000 application을 한 번만 기록한다. 일반 장비 통신은 기존 115200bps를 유지한다.
- MAIN 폴더 선택 시 네 파일과 application 내부 Vm 버전을 검사한다. esptool 출력을 우측 로그에 표시하고 기록 퍼센트를 전체 파일 크기에 환산해 프로그레스바에 반영한다. 프로그램은 외부 프로세스를 비동기로 기다리므로 UI를 막지 않는다.
- Release compile 경고 0/오류 0, self-test exit 0, 단일 EXE 게시 성공. 78,441,820 bytes, SHA-256 9EE409FC59B56091C731C1F77F2A27C093C1FFC080C87262E5D1BE1928A0A56C.
- 실제 flash 및 장비 검증은 수행하지 않았다. 작업 11-I 장비 gate와 전체 잔여 번호 5개를 유지한다.
## 작업 11 연결·업데이트 timeout 단순화 (2026-09-01)

- 사용자 지시로 MAIN 사전 $10%/Vm 검사, 업데이트 후 자동 재연결과 반복 버전 확인을 제거했다. 포트 선택 후 한 번만 바로 전송하고 완료 뒤 사용자가 수동 재연결한다.
- 115200bps는 유지한다. WriteTimeout은 500ms, frame ACK는 재시도 없이 3초다. 초기 연결 명령은 첫 write 실패 시 팝업 없이 중단하고 poll도 정지한다.
- 16KiB protocol frame/1KiB write chunk는 유지했으며 firmware는 변경하지 않았다.
- Release compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 71,755,983 bytes, SHA-256 C96566198AD6992A15CCEE8F68B4F4548E5CFDFE051010E2A606FF47FE03F31D.
- 실제 장비 재시험은 수행하지 않았다. 작업 11 잔여 I 1개와 전체 잔여 번호 5개를 유지한다.
## 작업 11 유선 업데이트 무진행·포트 식별 보완 (2026-09-01)

- 정확히 약 20초 뒤 실패한 로그는 10초 첫 frame write와 10초 abort write가 모두 막힌 결과다. 진행률 계산 전 단일 Write 단계의 실패로 확인했다.
- COM4와 COM3가 모두 ESP32-S3 USB 직렬 장치이므로 MAIN 업데이트 전 $10%를 최대 5회 보내 Vm 응답을 확인한다. 응답이 없으면 대용량 frame 없이 잘못된 포트/MAIN 미응답으로 중단한다.
- 16KiB protocol frame은 유지하고 .NET SerialPort.Write만 1KiB 단위로 나눴다. firmware protocol과 활성 firmware는 변경하지 않았다.
- Release compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 71,757,374 bytes, SHA-256 B11BE3A2ECA583CB2C7B8EE81C1DF95CA2AC136EC9BF64E39856EC20189FBC2A.
- 실제 장비 재시험은 수행하지 않았다. 작업 11 잔여 I 1개와 전체 잔여 번호 5개를 유지한다.
## 작업 11 유선 업데이트 첫 블록 timeout 보완 (2026-09-01)

- Vm1.0.8.0 실제 BIN 분석에서 32KiB LZ payload 최대 31,427 bytes, 115200bps 선로 시간 2.73초로 확인됐다. 기존 write timeout 3초와 여유가 없어 첫 쓰기와 abort 쓰기가 각각 timeout되는 약 6초 실패였다.
- $DN 블록을 수신 한도 32KiB보다 작은 16KiB로 낮췄다. 최대 선로 시간은 1.38초, 전체 순수 전송시간은 58.7초로 계산됐다. WriteTimeout도 10초로 늘렸으며 MAIN/ROD 유선 경로에 공통 적용된다.
- 외부 Vm1.0.8.0과 활성 펌웨어는 수정하지 않았다. Release compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 71,756,812 bytes, SHA-256 CFC19DAF83E0D466314F01DEFA5252135B896E2689F6D157690BEA66D86DAE5B.
- 실제 장비 재시험은 수행하지 않았다. 작업 11 잔여 I 1개와 전체 잔여 번호 5개를 유지한다.
## 작업 11 MAIN 업데이트 후 COM timeout 보완 (2026-09-01)

- 진행률 100% 뒤 MAIN soft reset으로 기존 COM4 핸들이 무효화된 상태에서 초기 명령을 보내 발생한 timeout을 확인했다. 데이터 전송 단계가 아니라 업데이트 후 재연결 단계의 문제다.
- 완료 프레임 뒤 기존 포트를 닫고 최대 12초 동안 같은 COM 포트 재등장을 기다려 새로 연다. 이후 초기 명령을 보내고 최대 6초 동안 선택 BIN의 Vm 버전을 확인한다.
- 재연결 실패, 버전 미확인, 버전 확인 완료를 서로 다른 결과로 표시한다. 활성 펌웨어는 변경하지 않았다.
- Release compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 71,756,811 bytes, SHA-256 380EBECB30CC18CC7A5EF5E4FD12F868C0D7498BFE32B774520DC14F28506108.
- 실제 장비 재시험은 수행하지 않았다. 작업 11 잔여 I 1개와 전체 잔여 번호 5개를 유지한다.
## 작업 11 MAIN 포트 선택·화면 높이 보완 (2026-09-01)

- MAIN 업데이트에 통신포트 선택 팝업을 추가했다. COM4가 존재하면 기본 선택한다. 상단 일반 포트 목록도 COM4를 우선 표시한다.
- 창 기본/최소 높이를 각각 50px 늘려 900/770px로 변경하고 릴/IMU 메뉴 표시 공간을 확보했다.
- Release compile 경고 0/오류 0, UI self-test exit 0, 표준 단일 EXE 게시 성공. 71,755,876 bytes, SHA-256 9ADE69D520030D9D5DCBDFFDAE5E0700E903513D0596FCA9DACA24B0FDB4EEC8.
- 실제 COM 및 장비 시험은 수행하지 않았다. 작업 11 잔여 I 1개와 전체 잔여 번호 5개를 유지한다.
## 작업 11 연결 초기화·업데이트 경로 보완 (2026-09-01)

- 최초 MAIN 연결 시 IMU 출력은 $080%로 OFF 초기화한다.
- 업데이트를 MAIN, ROD 무선, ROD 유선, ROD 등록 네 버튼으로 분리했다. MAIN은 기존 포트를 확인 후 닫고 폴더 선택, ROD 무선은 MAIN/ROD 연결 조건과 DFRO OTA, ROD 유선은 포트 팝업 후 폴더 선택과 별도 $DN R, 등록은 $3001% 후 왼쪽 버튼 장기 누름 및 $3003~$3005 상태 표시를 적용한다.
- DF_Main.ino.bin/DF_Rod.ino.bin과 BIN 내부 Vm/Vr 버전을 검사한다. LM JIG 레거시 $99, $0750~$0754, 직접 제어 코드를 확인했으나 레거시 UI와 현재 활성 Main 모두 비활성 상태다.
- TestModule Release compile 경고 0/오류 0, 폴더/DFRO/UI self-test exit 0, 표준 단일 EXE 게시 성공. bin/testmodule/Release/win-x64/DFTestModule.exe 71,755,726 bytes, SHA-256 3107CBB8E0BF293F7BB731134F79B3D7DED2F1CF5D02BA9F68C1324F2E68A3BC.
- 장비 연결·전송은 수행하지 않았다. 작업 11 잔여 문자 1개(I 장비 gate), 전체 잔여 번호 5개를 유지한다.
## 작업 11 입력 탭 제거·LM JIG 구성 (2026-09-01)

- 입력/전원 탭을 제거하고 LM JIG 탭으로 교체했다. $14 엔코더 회전 카운트와 $2703-03 수신 시작/정지는 릴/IMU 탭으로 이동했다.
- 레거시에서 LM 직접 제어 $071dddtttt%/$072dddtttt%/$070%, 위치 제어 $0750%~$0754ddd% 및 $99 레벨 파라미터 형식을 확인했다.
- 새 탭에는 직접 CW/CCW Duty/시간, 정지, 전체 정지, 홈, 좌/우 이동, 홈 복귀를 구성했다. 복잡한 $99 파라미터 편집은 포함하지 않았다.
- 현재 활성 Main은 CONF_LM_JIG=0, IO_LM_MOT=0, FUNC_FW_CONT_LM_MOT=0이며 ana_LmMotControl()도 빈 함수다. 따라서 탭에 현재 펌웨어 미지원 경고를 표시한다.
- compile과 표준 경로 단일 EXE 게시 성공. 크기 71,749,660 bytes, SHA-256 A96E2CBDAFF65BA2845D6FB362A455A3678A016EE5FEB41C77869B125DC104A9, self-test 종료 코드 0이다.
- 실행 중이던 기존 창 종료 후 표준 bin/testmodule/Release/win-x64 경로 재게시까지 완료했다. 장비 송신은 수행하지 않았다.
## 작업 11 UI/IF 사양 보완 (2026-09-01)

- 연결 버튼을 재검색보다 앞으로 옮기고 우측 고정 영역을 500px로 넓혔다. 수동 명령 제목 행과 입력 폭을 확대했다.
- 시리얼 연결 직후 릴 활성화용 게임 시작 $1101%를 자동 송신한다. IMU 초기 명령은 후속 요청에 따라 $080% OFF로 변경했으며, $20XX%/$21XX% 자발 상태를 받아 릴/IMU 연결 상태를 표시한다.
- 릴 버튼 기본 배경을 옅은 회색으로 변경했다. 눌림 상태는 사용자 지정대로 왼쪽 빨강, 오른쪽 파랑을 유지한다.
- LED 위치를 중앙(0)/왼쪽(1)/오른쪽(2)/하단(3), 표시 액션을 꺼짐(0)/켜짐(1)/점멸(2)/디밍(3), 색상을 01~14 콤보로 변경했다. 밝기 입력은 10~100%로 제한했다.
- 참고 근거는 DF_IF_Spec_AP2FW_K34_260517.xlsx CmdSts A75:H83, A103:H126, A196:H211이다. 첨부 사양은 읽기만 했고 수정하지 않았다.
- Release 단일 EXE 게시 및 UI control tree/프로토콜 자체 검사 성공. 배포 파일은 1개, 크기 71,748,646 bytes, SHA-256 4847A0D0A1ACFCAC88E266EA80F4BCFEDA145145249CBE51DDE03D567E4EB92D, self-test 종료 코드 0이다.
- computer-use helper가 Windows sandbox helper_unknown_error로 두 차례 종료되어 실제 창 screenshot 검증은 수행하지 못했다. 장비 연결과 송신은 수행하지 않았다.
## 작업 11-B~H: TestModule 화면과 장비 기능 구현 완료 (2026-09-01)

- 합의한 화면대로 상단 고정 연결/버전, 좌측 5개 카테고리, 우측 고정 수동 명령/로그를 구현했다. 에이징·설정/제조 메뉴는 제외했다.
- 시리얼 재검색·115200 연결·초기 버전 조회, Main/BLDC 동시출력, 릴/IMU, LED, 입력/전원, ZIP firmware package 검증 및 기존 $DN 다운로드 흐름을 연결했다.
- 공식 System.IO.Ports 8.0.0을 단일 EXE에 포함한다. legacy/TM_V034는 읽기만 했고 수정하지 않았다.
- 장비 없는 자체 검사는 명령 문자열, frame 분할, download checksum, LZ 압축/복원, ZIP 대상 판별과 전체 WinForms control tree 생성을 확인한다. 실제 장비 연결·명령·firmware 전송은 수행하지 않았다.
- 11-B~H 완료, 11-I는 장비 확인만 남는다. 작업 11 잔여 문자 1개(I).
## 작업 11-A: 독립 TestModule 기반 구성 완료 (2026-09-01)

- 사용자 요청으로 `legacy/TM_V034`를 참조하는 새 독립 프로그램 작업 11을 추가하고 우선 활성화했다. 원본 legacy는 변경하지 않았다.
- `testModule`은 .NET 8 WinForms, Windows x64, self-contained single-file 프로젝트다. 루트 `DF_Firmware.sln`에 직접 포함되지만 Main/Rod 및 기존 TestModule과 프로젝트 참조가 없다.
- 사용자 지정 관리 경로 `testModule`로 솔루션·빌드 스크립트·문서를 통일했다. 게시 경로 `bin/testmodule`은 유지했으며 이동 전 `apps/DFTestModule` 캐시는 0건이다.
- 최종 tools/build-testmodule.cmd Release 게시 성공: 오류 0. bin/testmodule/Release/win-x64에는 DFTestModule.exe 한 파일만 있으며 크기 71,748,646 bytes, SHA-256 4847A0D0A1ACFCAC88E266EA80F4BCFEDA145145249CBE51DDE03D567E4EB92D이다.
- 최종 EXE --self-test 종료 코드 0을 확인했다. UI와 장비 기능은 구현했으며 실제 COM 연결·명령 송신·장비 업데이트 시험은 대기다.
- 11-A~H 완료, 11-I 장비 gate 대기. 잔여 번호 작업은 5개(01, 08, 09, 10, 11)다.

## BLDC 처리 완료 (2026-08-31)

- 사용자가 최신 코드가 옳다고 확인하고 분석 중지 및 해결 완료 처리를 요청했다. 해당 건은 해결 완료이며 미해결 이슈와 추가 분석·로그 요청은 제거했다.
- 최신 활성 코드는 10bit PWM 반전식 `1023 - (1023 * duty / 255)`과 초기화·정지 PWM 1023을 적용한다. 이 코드를 그대로 유지했다.
- 완료 근거는 사용자 확인이다. Codex가 새 빌드·테스트·장비 검증·업로드를 수행한 것으로 기록하지 않는다.
- 작업 03-I 잔여 0개. 기존 잔여 번호 작업은 4개(01, 08, 09, 10), 작업 10 잔여 H 1개이며 이 건과 무관한 기존 작업은 유지한다.

## 소스 버전 자동 출력 경로 (2026-08-31, 최신)

- 사용자 수정 소스 버전 Main `Vm1.0.10.0`, Rod `Vr1.0.2.0`을 그대로 보존했다. 출력 폴더의 별도 고정 버전값을 없애고 `tools/firmware-versions.cmd`/`.ps1`이 Version.cpp를 매 build/clean 때 읽도록 02-H를 보완했다.
- 소스 버전만 수정·저장하면 VS2022/CLI 모두 같은 버전 폴더를 사용한다. 버전 읽기 실패 시 이전 폴더로 출력하거나 정리하지 않고 중단한다.
- 통합 Release x64 compile/link 성공: `bin/release/x64/Vm1.0.10.0` (Main app 885,216 bytes), `bin/release/x64/Vr1.0.2.0` (Rod app 766,848 bytes). 각 4파일/바이너리 내 버전 문자열 확인 및 오류 시나리오 포함 8개 검사 통과. 근거는 `artifacts/version-output-20260831/`이다.
- 이전 `Vm1.0.9.0`/`Vr1.0.1.3` 폴더의 app은 앞선 사용자 빌드로 이미 새 버전 내용이 포함되어 있었다. 이번에는 이전 파일을 보존했으므로 폴더명만 보고 과거 기준 image로 사용하지 않는다. 새 빌드 결과는 새 버전 폴더를 사용한다.
- source/library 파일과 이전 배포 파일 hash 보존 확인. 이번에는 업로드하지 않았으며 마지막 장비 확인 버전과 소스/빌드 버전은 별개다. 기존 장비 게이트 및 잔여 번호 4개는 그대로다.
## 기준선

- 작업공간: `C:\Projects\VS\DF_FW_Code\CodexManage`
- Git 브랜치: `main`
- 원격 저장소: `https://github.com/Ornithopter83/DF_FW_Code.git`
- 최초 기준선 commit/push: 완료 (2026-08-27)
- 환경 재구성 및 미확정 설정 명령서: `PROJECT_COMMANDS.md`
- 소스 및 기존 산출물 기준선: `docs/baseline/SOURCE_BASELINE_MANIFEST.md`
- Main 코드 기준 하드웨어 표: `docs/hardware/MAIN_HARDWARE_BASELINE.md`
- Rod 코드 기준 하드웨어 표: `docs/hardware/ROD_HARDWARE_BASELINE.md`
- 기존 release 산출물 기준: `docs/releases/BASELINE_RELEASE_MANIFEST.md`
- Main 활성 소스: `firmware/DF_Main`
- Rod 활성 소스: `firmware/DF_Rod`
- 변경 전 기준 원본: `legacy/Vm1.0.9.0/DF_Main`, `legacy/Vr1.0.1.0`
- 확인된 MCU/Core: ESP32-S3, Arduino-ESP32 Core 2.0.17
- 확정 FQBN: `PROJECT_COMMANDS.md`의 Main/Rod 공통 전체 FQBN
- 소스 리팩토링 초안: `docs/refactoring/SOURCE_REFACTORING_DRAFT.md`
- Main 의존성/owner 초안: `docs/refactoring/MAIN_DEPENDENCY_INVENTORY.md`
- Rod 의존성/owner 초안: `docs/refactoring/ROD_DEPENDENCY_INVENTORY.md`
- 독립 모듈 전환 순서: `docs/refactoring/MODULE_MIGRATION_PLAN.md`
- 다음 활성 작업서: `tasks/11_독립형_TestModule_프로그램.md`

## 확인된 현재 구현

- 변경 전 Main의 `DF_Main.ino`는 22,714줄이며 최상위 함수가 약 359개였다.
- 변경 전 Rod의 `DF_Rod.ino`는 3,065줄이며 통신, IMU, 설정 모드, 전원 및 입력 처리가 결합되어 있었다.
- 두 기존 `.map` 파일은 Arduino-ESP32 Core 2.0.17 경로를 포함한다.
- 프로젝트 로컬 `toolchain/arduino-cli/arduino-cli.exe`가 추가되었고 로컬 설정으로 실행된다. 확인 버전은 1.5.2-rc.1이다.
- `toolchain/arduino-cli.yaml`은 data/downloads/user를 모두 저장소의 Git 제외 `toolchain/` 아래로 격리한다.
- 프로젝트 로컬 `toolchain/arduino-data`에 ESP32 Core 2.0.17과 요구 compiler/tools가 설치되어 있다. 시스템 Core 3.3.0과 분리되어 있다.
- 프로젝트 로컬 library에서 ESP32Servo 1.1.2가 인식된다.
- Rod의 `Kalman.h` 참조는 실제 사용 코드가 `#if 0`에만 있어 현재 제품에는 불필요한 의존성으로 판정했다. 설치하지 않고 이후 활성 코드에서 제거한다.
- Main의 외부 의존성은 프로젝트 로컬 ESP32Servo 1.1.2로 확정했으며, Rod의 `Kalman.h`는 현재 제품 빌드에 불필요하므로 설치 대상에서 제외했다.
- 여러 파일이 `Arduino.h` 대신 `arduino.h`를 사용한다.
- compile/link 빌드는 Codex가 직접 수행하며 플래시와 장비 검증은 사용자가 수동 수행한다.
- `DF_Firmware.sln`에는 Main/Rod NMake project만 있으며 Rod가 Main에 의존한다. `Release|x64` Solution Build가 실제로 성공했다.
- Main/Rod는 `vs/DF_Arduino_ESP32S3.props`의 공통 IntelliSense 설정을 사용한다. 평가된 경로 207개가 모두 존재하며 `Update.h`, `FreeRTOSConfig.h` 등 핵심 헤더 해석을 확인했다.
- Rod의 원본 폴더/대표 `.ino` 이름 불일치는 빌드 환경 정리 단계에서 clean staging으로 확인했으며, 현재 활성 소스는 정규화된 `firmware/DF_Rod`를 직접 사용한다.
- Windows GCC 8.4의 긴 경로 문제는 저장소를 `X:`에 자동 매핑해 해결했다.
- 2026-08-27 재현 build application은 Main 882,320 bytes, Rod 763,552 bytes다. 필수 flash 파일 8개 생성을 확인했으며 upload/flash는 수행하지 않았다.
- 배포 산출물은 `bin/<configuration>/<platform>/<version>/`에 대상별 플래시 파일 4개만 둔다. 현재 Release 경로는 Main `Vm1.0.9.0`, Rod OTA 기준 `Vr1.0.1.0`, Rod 현재 복구 `Vr1.0.1.3`로 분리된다. `Vr1.0.1.2`는 과거 시험 산출물이다.
- 작업 03~06에서 현재 Variant 단일화, deprecated/legacy 보존, DFProtocol 공유 계약, Main/Rod 기능 경계 1차 분리를 적용했다. 두 `.ino`에는 setup/loop 위임만 남았다.
- 작업 03~06 변경 이후에는 사용자의 명시적 지시에 따라 build, host test, protocol test 및 장비 smoke를 수행하지 않았다. 직전 성공 산출물은 구조 변경 전 기준일 뿐 현재 소스를 검증하지 않는다.
- 2026-08-28 활성 `.inc` 16개를 모두 독립 `.cpp/.h`로 전환했다. Main/Rod Application `.cpp`가 각각 setup/loop 구현을 소유하며 VS2022 project에 모든 module source/header가 등록되어 있다.
- 공용 상태 owner는 `DF_Main_State.cpp/.h`, `DF_Rod_State.cpp/.h`이며 기능 상태 선언은 각 모듈 헤더가 소유한다. `DF_*_Internal.h`는 구현 전용 집계 헤더로 축소되었고 공개 모듈 헤더에서는 사용하지 않는다.
- Arduino CLI 통합 clean build와 VS2022 Solution Rebuild가 현재 구조에서 성공했다.
- Main/Rod ESP-NOW callback은 길이를 검사한 뒤 고정 128-byte 단일 슬롯에 복사하고, 문자열 분석과 로그는 loop 문맥에서 수행한다. Main encoder ISR과 task 사이의 다중 필드 snapshot도 임계영역으로 일치시켰다.
- 외부 장치가 없는 Main 테스트 보드에서 4개 flash 영역 쓰기/재검증, 12초 시리얼 안정성 및 `Vm1.0.9.0` 버전 응답을 확인했다.
- Rod 테스트 보드에도 현재 배포 파일 4개를 ROM bootloader `--no-stub` 방식으로 기록·재검증했다. `Vr1.0.1.0` 부팅, Main–Rod 상호 등록, 무선 버전 왕복과 999ms 1회 진동 명령/후속 OFF 전송을 확인했다. 사용자는 릴 연동 LED가 점멸 상태로 바뀌고 진동이 약 1초간 동작한 것을 확인했다. BLDC/encoder 부하는 미수행이다.

## Rod 시험 표식 제거·USB 복구·PowerPoint 인수인계 (2026-08-31)

- 사용자 묶음 요청과 “빠른 업데이트는 USB 연결 업로드” 정정에 따라 작업 10-I/J를 추가 수행했다. Main 경유 OTA를 이번 복구에 사용하지 않았다.
- HANDLE 원시 입력 activity callback, 200ms 시험 LED 상태·처리 및 Application 호출만 제거했다. 기존 encoder 전송, 버튼/부팅/연결 LED 및 OTA 수신기는 유지했다. 변경 전 5개 파일은 `deprecated/rod/handle-led-test-Vr1.0.1.2/`에 보존했다.
- 현재 Main은 `Vm1.0.9.0`, Rod는 `Vr1.0.1.3`이다. `tools/build-all.cmd Release x64` compile/link와 `tools/test-protocol.cmd`가 exit 0으로 성공했다. Main bin 885,216 bytes, Rod application 766,485 bytes/bin 766,848 bytes, 대상별 배포 4파일이다.
- 이번 명시적 요청 범위에서 Rod USB 직접 업로드를 수행했다. 대상 버전과 파티션 일치를 먼저 확인하고 ROM `--no-stub`, 921600 baud, 압축 전송으로 `otadata 0xe000`와 `app0 0x10000`만 기록했다. 두 영역 hash/verify 및 재부팅 후 `Vr1.0.1.3` 응답 확인 완료. bootloader/partition/NVS/SPIFFS와 Main은 기록하지 않았다.
- 파티션 확인·기록·검증은 33.86초, application 전송은 약 21.7초였다. 이후 버전 조회 시간은 제외한다. 근거: `artifacts/rod-restore-20260831/build.log`, `usb-upload.log`, `result.json`.
- 인수인계 PowerPoint: `docs/handoff/DF_Firmware_Source_Handoff_2026-08-31.pptx` (140장). VS2022/폴더 소스 탐색, 초기화·loop·callback/ISR·USB/OTA 흐름, 활성 경로의 함수 정의 555개(Main 420/Rod 127/공유 8)를 파일별 표로 수록했다. 전체 시그니처와 직접 호출 목록은 발표자 노트에 있다. 빈 함수와 소스 정의 기준이라는 한계를 표시했다.
- 140장 렌더·개별 시각 검토와 함수 목록 대조 완료. 복구 후 물리 HANDLE LED 관찰 및 VS2022 재분석 화면은 사용자 확인 대기다. 작업 10-H 실패 복구 검증과 기존 장비 게이트를 완료 처리하지 않았다.
- 잔여 번호 작업 4개(01/08/09/10), 작업 10 잔여 문자 1개(H). I/J의 요청 범위는 완료했다. commit/push는 수행하지 않았다.

## VS2022 표시 오류 보완 (2026-08-31)

- 사용자 화면의 `IRAM_ATTR` E0070, `vPortEnterCritical` E0020 및 후속 E0065에 대해 작업 02-G IntelliSense 설정만 보완했다. 작업 10-H는 재개하지 않았다.
- `vs/DF_IntelliSense.h`를 `Arduino.h`보다 먼저 강제 포함하며 MSVC IntelliSense에서만 GCC `__attribute__` 구문을 생략한다. 활성 펌웨어와 Core 헤더는 변경하지 않았다.
- MSBuild 속성 평가로 Main/Rod Debug/Release x64 네 구성의 강제 포함 순서와 파일 존재를 확인했다. `tools/build-all.cmd Release x64` compile/link 성공(exit 0): Main bin 885,216 bytes, Rod bin 767,072 bytes, 대상별 배포 파일 4개. ELF의 `ISR_onTimerHandler()`는 `.iram0.text`에 유지되며 sketch 의존성에 분석 전용 헤더가 없다. VS 화면 재분석 결과는 사용자 확인 대기다.
- 잔여 번호/문자 작업 수는 기존과 같다. 이번 보완으로 장비 게이트를 완료 처리하지 않는다.

## 중간정리: 남은 코드 작업

- 충돌 가능한 전역 심볼, 매크로 및 상수의 고유 접두어 정리
- 초기화 순서, ODR 및 hardware object 수명 검토
- 주석 처리된 구 코드, 테스트 로그와 단순 수치 변경 이력 제거; 현재 제약을 설명하는 짧은 주석만 유지
- 현재 활성 소스의 Main/Rod compile/link, protocol host test와 장비 smoke 검증

독립 `.cpp/.h`, VS2022 편집/참조 구조, 상태 owner 및 callback/ISR 안전화까지 완료했다. protocol host/정적 검사와 실제 장비 검증은 남아 있어 최종 리팩토링 완료 상태는 아니다.

2026-08-28 작업 07에서 초안과 의존성 표를 완성했다. Main 약 17,011줄/확인 함수 338개, Rod 약 2,509줄/확인 함수 56개를 분류했으며, Foundation/Scheduler 해체, 기능 owner의 file-scope 상태, 최소 public API와 작업 08-A~J 순서를 확정했다.

## 목표 구조

- Main과 Rod는 독립 펌웨어로 유지한다.
- 현재 제품 구성만 활성 코드에 남긴다.
- 전체 원본은 `legacy/`, 제거 코드는 `deprecated/`에 둔다.
- 공통 메시지 계약만 `libraries/DFProtocol/`에서 공유한다.
- 자체 코드는 C++98/03 문법, 고유 접두어, namespace 미사용 원칙을 따른다.

## 진행

활성 잔여 번호 작업 0개

작업 11 잔여 문자 0개

## 작업 정책

- 한 번에 한 문자 작업만 수행한다.
- compile/link 빌드는 Codex가 직접 실행한다. 플래시와 장비 게이트에서는 사용자에게 필요한 명령과 확인 항목을 전달하고 결과를 기다린다.
- `deprecated/` 파일은 활성 sketch 폴더 밖에 두고 빌드에서 제외한다.
- C++11 이후 문법과 제품 Variant 전처리 분기를 추가하지 않는다.
- 프로토콜과 장비 동작을 파일 이동 작업에서 변경하지 않는다.
- 주석 정리는 기능 변경과 분리한다. 중요 값은 owner 모듈의 이름 있는 상수로 만들고 별도 문서는 장비 교정에 필요한 경우만 최소 작성한다.
- Arduino CLI와 Core는 Git 제외 `toolchain/`에 프로젝트 로컬로 구성하고 시스템의 Core 3.3.0과 분리한다.

## 표준 검증

- 확정 FQBN을 사용하는 Arduino CLI 명령은 작업 02의 VS2022 build script에 고정했으며 Codex가 직접 실행한다.
- 모든 코드 변경의 최소 자동 게이트: Main/Rod 대상 빌드 성공, link 성공, firmware 생성 확인. 문서 초안만 변경하는 작업 07에는 적용하지 않는다.
- 주요 경계 변경 후 수동 smoke test, 프로토콜 변경 후 Main–Rod 통합 시험을 요청한다.

## 다음 작업

2026-08-31 `Vr1.0.1.2` 최종 VS2022 Release/x64 Rebuild 경고 0/오류 0, protocol host test와 OTA dry run 성공. Main bin 885,216 bytes, Rod bin 767,072 bytes 및 대상별 배포 파일 4개를 확인했다. 이후 사용자 승인으로 기반 유선 설치와 최종 Rod OTA를 수행했다.

10-G 정상 경로 완료: 사용자 승인 후 Main/Rod OTA 기반 유선 설치·verify, Main 경유 Rod `Vr1.0.1.2` OTA, 재부팅 후 무선/직접 버전 조회가 성공했다. OTA 명령 호출~완료 출력은 실행 로그 기준 약 2분 49초(유선 설치 제외)다. 사용자가 HANDLE 점등·정지 후 소등·연속 회전 중 점등 유지까지 확인했다. 다음은 10-H의 전송 중단·재시도·잘못된 이미지 거부·전원 차단/유선 복구 검증과 최종 배포 경계 마감이다. 이후 08-J/K 자동 정책 검사/장비 부하 시험 및 09 통합 릴리스를 진행한다.
