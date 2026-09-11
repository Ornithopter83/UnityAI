# 작업 11

## TM V041 실패 보정 UI 제거 (완료: 2026-09-09)

- 정지 문턱값, Yaw 보정 배율, 가상 Roll, 게임 좌우, 보정 상태, 충전 기준 보정 버튼과 45도 제한 계산을 제거했다.
- 원본 IMU 수치, 회전 박스와 TM 전용 0점 보정은 유지했다.
- 모터 UI와 OTA 기능, V041 게시 설정은 유지했다.
- 빌드·게시·화면 검증은 수행하지 않았다.

독립형 TestModule 프로그램

## 목표

legacy/TM_V034를 기준 자료로만 사용하고, 새 TestModule을 testModule 경로의 독립 프로젝트로 구축한다. 루트 DF_Firmware.sln에 포함하지만 Main/Rod 펌웨어와 프로젝트 참조를 만들지 않는다. 배포 결과는 Windows x64 단일 EXE 한 개다.

## 완료 작업

- A. 독립 프로젝트와 단일 EXE 게시 기준 (완료: 2026-09-01)
- B. 좌측 카테고리·상단 연결·우측 명령/로그 기본 화면 (완료: 2026-09-01)
- C. 시리얼 포트 재검색·연결·자동 버전 조회 (완료: 2026-09-01)
- D. 수동 명령과 수신/동작 로그 (완료: 2026-09-01)
- E. Main/BLDC 동시 모터 제어 (완료: 2026-09-01)
- F. 릴 버튼·출력·IMU 상태 및 제어 (완료: 2026-09-01)
- G. LED·입력·전원 기능 (완료: 2026-09-01)
- H. 압축 firmware package 검증·다운로드·버전 갱신 (완료: 2026-09-01)
- I. 기존 명령 호환 검증·단일 EXE 릴리스와 장비 gate (코드/빌드 완료, 장비 확인 대기)

## 구현 결과

- 상단 고정 시리얼 선택·재검색·연결·Main/Rod/IMU 버전, 좌측 5개 메뉴, 우측 고정 수동 명령 6개와 로그를 구현했다.
- 에이징과 설정/제조 화면은 만들지 않았다.
- 연결 직후 $00%, $290101%, $1101%, $10%, $0112%, $0110%, $1501%를 송신한다.
- 모터는 기존 $02DDDTTTT%, $04ADDDTTTT%, $02000%, $0400000000% 형식을 사용한다.
- 릴 진동·좌 LED·우 LED와 IMU는 항목별 ON/OFF 라디오로 전개했다. 수신 버튼 상태는 왼쪽 빨강, 오른쪽 파랑, 기본 회색이다.
- LED 출력/밝기, 입력 모니터, AC 전원 명령과 주요 입력 상태를 구현했다.
- ZIP 안의 단일 DF_MAIN 또는 DF_ROD BIN을 검증한다. 선택적 manifest.json의 target/version/size/sha256을 검증하며 기존 $DN 32 KiB 블록, LZ 압축, ACK/timeout, 998 abort, 999 complete를 적용한다.
- System.IO.Ports 8.0.0은 self-contained 단일 EXE에 포함한다. legacy/TM_V034는 수정하지 않았다.

## 확인 근거

- tools/build-testmodule.cmd Release 게시 성공.
- 자체 검사에서 명령 문자열, download frame, 분할 수신 frame, LZ 압축·복원, ZIP 판별과 전체 WinForms control tree 생성을 확인했다.
- 발행 EXE --self-test 종료 코드 0, 배포 폴더 단일 파일을 확인했다.
- 실제 COM 연결, 물리 동작 및 firmware 전송은 수행하지 않았다.

## 진행

잔여 작업 0개


## 2026-09-01 UI/사양 보완

- 상단 버튼 순서를 연결, 재검색으로 변경하고 우측 수동 명령 영역을 확대했다.
- 연결 직후 $1101%를 자동 송신하고 $20/$21 자발 응답으로 릴/IMU 연결 상태를 표시한다. IMU 초기 명령은 후속 요청에 따라 $080% OFF로 변경했다.
- 릴 버튼 기본 배경은 옅은 회색, 눌림은 왼쪽 빨강/오른쪽 파랑이다.
- LED 위치 0~3, 표시 액션 0~3, 색상 01~14를 콤보로 제한하고 밝기를 10~100%로 제한했다.
- IF 사양서 관련 범위와 legacy 명령을 대조했다. Release 게시와 self-test는 통과했으며 실제 장비 gate는 남아 있다.

## 2026-09-01 입력/전원 탭 교체

- 입력/전원 탭을 LM JIG 탭으로 교체하고 엔코더 회전 수신을 릴/IMU 탭으로 이동했다.
- LM 직접/위치 제어 명령을 구현했으나 현재 활성 Main 펌웨어에서 LM 기능과 IO가 비활성이고 분석 함수도 비어 있음을 화면에 경고한다.
- 코드 compile, 표준 경로 단일 EXE 게시와 self-test를 완료했다. 실제 장비 gate는 남아 있다.
## 2026-09-01 연결 초기화·업데이트 경로 보완

- 최초 MAIN 시리얼 연결 직후 IMU 데이터 출력 OFF 명령 $080%를 보내며, 릴/IMU 화면의 기본 선택도 OFF로 유지한다.
- 업데이트 화면을 MAIN 펌웨어 업데이트, ROD 펌웨어 업데이트 (무선), ROD 펌웨어 업데이트 (유선), ROD 등록 네 버튼으로 분리했다.
- MAIN은 기존 연결이 열려 있으면 사용자 확인 후 먼저 닫고, DF_Main.ino.bin이 있는 폴더를 선택한 뒤 $DN M 전송을 수행한다.
- ROD 무선은 MAIN과 ROD 연결 상태를 확인하고, DF_Rod.ino.bin 폴더를 선택한 뒤 기존 검증 도구와 같은 $OR<DFRO hex>% 96-byte/CRC32/SHA-256/ACK 5회 재시도 규약으로 전송한다.
- ROD 유선은 별도 팝업에서 MAIN 포트를 제외한 통신포트를 먼저 선택하고, ROD 폴더를 선택한 뒤 독립 시리얼 연결로 $DN R 전송한다.
- ROD 등록은 $3001% 후 “ROD 왼쪽 버튼을 길게 누르세요” 상태로 대기한다. $3004% 진행, $3005% 완료, $3003% 취소를 표시하며 사용자가 취소하면 $3002%를 보낸다.
- 폴더의 application BIN 파일명과 BIN 내부 Vm/Vr 버전을 검사한다. bootloader/partitions/boot_app0는 이 앱 업데이트 경로의 전송 대상이 아니다.
- LM JIG 과거 코드는 legacy/TM_V034/FishingDebugger/LM_JIG_Settings.cs의 $99 레벨 설정과 $0750~$0754 위치 명령, DFTMmainForm.cs의 비활성 직접 제어 블록에서 확인했다. 해당 UI들은 #if(false)로 막혀 있고 현재 활성 Main도 CONF_LM_JIG, IO_LM_MOT, FUNC_FW_CONT_LM_MOT가 0이며 분석 함수가 비어 있다.
- dotnet build 경고 0/오류 0, 폴더 대상·버전 판별과 DFRO frame을 포함한 self-test exit 0, 표준 Release 단일 EXE 게시 성공. 파일 1개, 71,755,726 bytes, SHA-256 3107CBB8E0BF293F7BB731134F79B3D7DED2F1CF5D02BA9F68C1324F2E68A3BC.
- 실제 COM 연결, 펌웨어 전송, ROD 등록 및 장비 동작은 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.
## 2026-09-01 MAIN 포트 선택·화면 높이 보완

- MAIN 펌웨어 업데이트도 별도 통신포트 선택 팝업을 거친 뒤 폴더를 선택하도록 변경했다.
- COM4가 존재하면 MAIN 업데이트 포트 팝업의 기본값으로 표시한다.
- 상단 일반 시리얼 목록도 기존 선택이 없고 COM4가 존재하면 COM4를 우선 선택한다.
- 기본 ClientSize 높이를 850에서 900으로, MinimumSize 높이를 720에서 770으로 각각 50px 늘려 릴/IMU 하단 메뉴 공간을 확보했다.
- compile 경고 0/오류 0, UI self-test exit 0, 표준 단일 EXE 게시 성공. 파일 1개, 71,755,876 bytes, SHA-256 9ADE69D520030D9D5DCBDFFDAE5E0700E903513D0596FCA9DACA24B0FDB4EEC8.
- 실제 포트 연결과 장비 업데이트는 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.
## 2026-09-01 MAIN 업데이트 후 COM timeout 보완

- 사용자 화면의 진행률 100% 후 COM4 쓰기 제한시간 오류를 분석했다. 모든 데이터 블록 ACK와 완료 프레임 전송 뒤 MAIN 펌웨어가 Update.end(true), 20ms 대기, soft reset을 수행하지만 앱이 1.8초 후 재부팅 전의 기존 SerialPort 객체로 초기 명령을 보내 발생한 후처리 오류다.
- 완료 후 기존 포트를 닫고 선택했던 포트가 다시 나타날 때까지 최대 12초 기다린 뒤 새 SerialPort로 연다.
- 재연결 직후 초기 명령은 중복 오류 팝업 없이 송신하고, 최대 6초 동안 $10%를 재요청하여 선택 BIN의 Vm 버전과 일치하는지 확인한다.
- 버전이 일치하면 업데이트·버전 확인 완료, 포트만 재연결되고 버전 응답이 없으면 확인 필요 경고, 포트가 12초 내 돌아오지 않으면 업데이트/재연결 실패로 구분한다.
- firmware Main 수신 코드와 legacy DownloadForm의 완료 후 1.8초 버전 조회 흐름을 대조했다. 펌웨어 코드는 변경하지 않았다.
- compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 파일 1개, 71,756,811 bytes, SHA-256 380EBECB30CC18CC7A5EF5E4FD12F868C0D7498BFE32B774520DC14F28506108.
- 실제 장비 재업데이트는 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.
## 2026-09-01 유선 업데이트 첫 블록 write timeout 보완

- 재시험 로그의 MAIN 업데이트 준비 후 약 6초 timeout을 첫 데이터 블록 송신 실패로 분리했다. 3초 write timeout 두 번(첫 블록과 abort)이 연속 발생한 시간과 일치한다.
- 사용자가 선택한 Vm1.0.8.0 DF_Main.ino.bin 873,648 bytes를 읽어 LZ 전송 크기를 계산했다. 32KiB 블록은 27개, 최대 payload 31,427 bytes, 115200bps 최대 선로 시간 2.73초로 기존 3초 제한과 여유가 거의 없었다.
- 블록을 16KiB로 낮추면 54개, 최대 선로 시간 1.38초이며 전체 순수 전송시간은 58.3초에서 58.7초로 0.4초만 증가한다.
- MAIN 수신 펌웨어의 DN_MAX_LEN 0x8000은 최대 허용 크기이므로 16KiB 프레임은 기존 프로토콜과 호환된다. FirmwareDownloader 블록을 0x4000으로 낮추고 SerialConnection WriteTimeout을 ACK timeout과 같은 10초로 변경했다. 공유 $DN 경로인 MAIN과 ROD 유선에 함께 적용된다.
- 외부 Vm1.0.8.0 폴더는 읽기만 했으며 변경하지 않았다. 활성 펌웨어도 변경하지 않았다.
- compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 파일 1개, 71,756,812 bytes, SHA-256 CFC19DAF83E0D466314F01DEFA5252135B896E2689F6D157690BEA66D86DAE5B.
- 실제 장비 재업데이트는 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.
## 2026-09-01 유선 업데이트 무진행·포트 식별 보완

- 후속 로그는 MAIN 업데이트 준비 후 정확히 약 20초 뒤 실패했다. WriteTimeout 10초인 첫 frame과 abort frame이 각각 timeout된 것으로, 진행률 계산 전 단일 SerialPort.Write에서 막힌 상태다.
- 현재 Windows에는 ESP32-S3 USB 직렬 장치가 COM4와 COM3 두 개 모두 존재한다. 포트 번호만으로 MAIN/ROD를 구분할 수 없다.
- MAIN 업데이트 포트를 연 뒤 400ms 안정화하고 $10%를 최대 5회 송신하여 Vm 응답을 먼저 확인한다. Vm 응답이 없으면 큰 frame을 보내지 않고 MAIN 포트를 다시 선택하라는 오류를 표시한다.
- 16KiB $DN protocol frame은 유지하되 .NET SerialPort에는 1KiB씩 나누어 Write한다. firmware에는 같은 연속 byte stream으로 전달되므로 frame 계약은 바뀌지 않는다.
- 정상 사전 확인 시 로그에 업데이트 대상 MAIN 확인: Vm...이 남고 그 뒤 진행률이 올라간다. 이 로그가 없으면 선택 포트/MAIN 응답 문제로 구분할 수 있다.
- compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 파일 1개, 71,757,374 bytes, SHA-256 B11BE3A2ECA583CB2C7B8EE81C1DF95CA2AC136EC9BF64E39856EC20189FBC2A.
- 실제 장비 재업데이트는 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.
## 2026-09-01 연결·업데이트 timeout 단순화

- 사용자 재지시에 따라 MAIN 포트 사전 $10%/Vm 검사와 업데이트 후 자동 재연결·반복 버전 확인을 제거했다.
- 전송속도는 115200bps로 유지한다. SerialPort WriteTimeout은 10초에서 500ms로 줄였다. 1KiB chunk의 이론 전송시간 약 89ms 대비 약 5.6배 여유다.
- 일반 연결 초기 명령은 오류 팝업을 표시하지 않으며 첫 write 실패에서 즉시 중단한다. poll write가 실패하면 poll timer도 정지한다.
- MAIN 업데이트는 선택 포트를 열고 150ms 뒤 사전 명령 없이 한 번만 $DN 전송한다. 각 frame ACK는 재시도 없이 최대 3초만 기다린다.
- 완료 후 자동 포트 재연결이나 버전 재조회 없이 포트를 닫고, 사용자가 보드 재부팅 후 연결 버튼으로 다시 연결하도록 안내한다.
- 16KiB protocol frame과 1KiB SerialPort.Write chunk는 유지한다. 전송속도와 firmware protocol/활성 firmware는 변경하지 않았다.
- compile 경고 0/오류 0, self-test exit 0, 표준 단일 EXE 게시 성공. 파일 1개, 71,755,983 bytes, SHA-256 C96566198AD6992A15CCEE8F68B4F4548E5CFDFE051010E2A606FF47FE03F31D.
- 실제 장비 재시험은 수행하지 않았다. 잔여 작업은 I 장비 gate 1개다.


## 2026-09-01 MAIN ROM 복구 업데이트 통합

- MAIN 업데이트를 실행 중 펌웨어의 $DN 수신기에 의존하지 않는 ESP32-S3 ROM bootloader 기록으로 변경했다.
- esptool_V4.5.1.exe를 단일 EXE에 리소스로 포함하고 update.bat 기준 네 영역(0x0/0x8000/0xe000/0x10000)을 선택 포트에 921600bps로 기록한다.
- 선택 폴더의 bootloader, partitions, boot_app0, application과 Vm 버전을 검사한다. esptool 출력은 로그, 기록 비율은 프로그레스바에 표시한다.
- Release build와 self-test를 완료했다. 단일 EXE 78,441,820 bytes, SHA-256 9EE409FC59B56091C731C1F77F2A27C093C1FFC080C87262E5D1BE1928A0A56C.
- 실제 보드 기록은 수행하지 않았으므로 I 장비 gate 1개는 유지한다.


## 2026-09-01 polling 로그·ROD 유선 esptool 보완

- MAIN 업데이트 버튼의 (복구) 표기를 제거했다.
- 1초 $00% 연결 polling은 유지하고 정상 반복 수신 $00%와 $1800%만 로그에서 숨긴다. 그 밖의 $18xx%는 표시한다.
- ROD 유선 업데이트를 $DN에서 내장 esptool 네 영역 직접 기록으로 변경했다. ROD용 네 파일과 Vr 버전을 검사하며 MAIN과 같은 flash 주소를 사용한다.
- MAIN/ROD 유선 package와 updater를 공통화하고 두 대상의 flash 인수 및 polling 필터 자체검사를 추가했다.
- Release build와 self-test를 완료했다. 단일 EXE 78,442,332 bytes, SHA-256 D977F582EF97003B77E45E347DA3574935820768302EE1CD907E2F2A6BAEE7B0.
- 실제 ROD flash는 수행하지 않았으므로 I 장비 gate 1개는 유지한다.

## 2026-09-01 $1800 방향 정정

- 앞선 $1800 로그 숨김 판단을 취소했다. $1800%는 MAIN의 AP 종료 요청이며 TestModule이 AP이므로 수신 내용을 그대로 표시하고 같은 $1800%를 한 번 응답한다.
- $00% polling 송수신도 숨기지 않는다. TestModule은 Windows 종료를 실행하지 않는다.
- ROD 유선 내장 esptool 전환은 그대로 유지한다.
- 최종 Release build/self-test 성공. 단일 EXE 78,442,332 bytes, SHA-256 856148C7E06CE948B70F0BFCF80CE7345C8B0E7DE58414CECAC4F45BAC3E6248.

## 2026-09-01 업데이트 후 첫 연결 초기화 보완

- MAIN의 단일 rcved_flag 수신 구조와 레거시 TestModule의 500ms 3단계 초기화를 대조했다.
- 새 프로그램의 8개 연속 송신을 500ms 간격 3단계로 변경하고 단계 내부에도 60ms 간격을 적용했다. $1101 이후 다음 $00에서 ROD/IMU current status가 나오도록 순서를 보존했다.
- 초기화 완료 후에만 2초 polling을 시작하며 재시도는 하지 않는다.
- Release build/self-test 성공. 단일 EXE 78,444,892 bytes, SHA-256 42463DC0003174E03E43099B1D1F29B86F1190B2CFD018D4F478CBD6A41A4450.
- 실제 업데이트 직후 첫 연결 검증은 장비 gate로 유지한다.
## 2026-09-01 업데이트 직후 첫 연결 로그 기반 재보완

- `DFLOG[202609011503].txt`에서 연결 직후 포트를 열기 전의 상태 프레임과 다수 `$1800%`가 즉시 전달됐고, 자동 ACK 직후 `$2211%` MAIN reset 통지가 이어진 것을 확인했다.
- `SerialConnection.Open`은 포트를 연 뒤 입출력 버퍼를 비우고 수신 이벤트를 연결한다. `FrameDecoder.Reset`도 호출해 이전 연결의 미완성 프레임이 다음 연결에 섞이지 않게 했다.
- `$1800%`는 MAIN의 AP 종료 요청으로 표시하되 TestModule이 실제 AP 종료를 수행하지 않으므로 자동 ACK를 제거했다. 연결당 첫 요청에는 원인을 알 수 있는 경고를 남긴다.
- `$10%`는 ROD와 IMU 연결 상태를 모두 받거나 최대 2초가 지난 뒤 한 번만 보낸다. 초기 상태 조회와 일반 polling의 단계적 송신 및 무재시도 정책은 유지한다.
- Release build와 self-test를 완료했다. 표준 단일 EXE는 78,445,404 bytes, SHA-256 `6F300C57328983138A3790F407CCA86F3E0C49645DFFA738428E998907D64819`이다.
- 실제 업데이트 직후 장비 재연결 시험은 I 장비 gate로 유지한다. 잔여 작업은 I 1개다.
## 2026-09-02 활성 범위 재정리

### J. 무선 업데이트 보완 (완료: 2026-09-02)

- 시작 전 경고 제거, 허용 범위의 전송 속도 최대화, 업데이트 후 재연결 시 MAIN/AP `$00%` polling 복구, 성공 완료 팝업을 구현한다.

### K. LM JIG 실제 제어 (완료: 2026-09-02)

- 현재 UI의 LM 직접/위치 명령을 활성 MAIN 펌웨어가 실제 처리하도록 런타임 LM JIG 판별, 핀/PWM, 명령 라우팅, 센서 기반 안전 정지를 구현했다.

- 기존 I 포괄 장비 gate와 그 밖의 프로젝트 검증·마감은 2026-09-02 사용자 지시로 활성 범위에서 제외했다.
- 11-J/K 완료. 활성 잔여 작업은 0개다.
## 2026-09-02 J 결과

- 시작 전 확인 경고를 제거하고 전송 직후 MAIN `$00%` polling을 완료 팝업보다 먼저 복구했다.
- 최대 12초 ROD 재연결을 확인한 뒤 버전을 조회하며, 완료 팝업에 재연결 확인 여부를 표시한다. 실패 시에도 열린 MAIN 연결의 polling을 복구한다.
- 현재 96-byte payload/프레임별 ACK에는 정상 전송 인위적 지연이 없다. 128-byte 펌웨어 수신 경계와 기존 설치 image 호환성을 유지하기 위해 protocol frame은 변경하지 않았다.
- Release build와 self-test 성공. 단일 EXE 78,445,404 bytes, SHA-256 `8B7AF98AA7094EFF6C2A403A4505460B5911E5D9FDE979B15E2682E9FED11F68`.

## 2026-09-02 K 결과

- 기존 `setConfig()`의 런타임 LM JIG 판별을 사용해 일반 MAIN과 LM JIG를 한 firmware에서 분리했다. 새 제품 Variant 전처리 분기는 추가하지 않았다.
- 레거시 LM 핀, PWM driver, `$07` 라우팅, 직접 구동과 센서 기반 홈/좌/우/복귀 제어를 활성 코드에 복구했다.
- 위치 이동은 센서 도달 또는 3초 timeout에서 정지한다. 일반 MAIN의 GPIO 40/41 LED와 GPIO 47 WDT 동작은 유지하고 LM JIG일 때만 공유 핀의 LM 용도를 우선한다.
- MAIN clean build 성공(application 888,768 bytes, SHA-256 `A64B04C4F304E02B2573BE6236302CF93E78757EDC019BFF430260C8BDDE122C`). TestModule Release 단일 EXE build와 self-test exit 0, EXE 78,445,404 bytes, SHA-256 `18D1A23EF32B78C91CAFF134CE441C065ED1DFD08B82D29AF3CB7AB517F6D289`.
- 플래시·물리 LM JIG 시험은 수행하지 않았다. 활성 구현 잔여 작업은 0개다.
