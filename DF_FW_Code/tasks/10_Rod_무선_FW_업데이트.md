# 작업 10

Main 중계형 Rod 무선 FW 업데이트

## 목표

유선 서비스 경로에 연결된 Main을 진입점으로 사용해, 해당 Main에 등록되어 현재 통신 중인 Rod 한 대의 애플리케이션 firmware만 ESP-NOW로 업데이트한다.

```text
서비스 PC/업로드 프로그램
        │ USB
        ▼
      Main
        │ ESP-NOW, 등록된 Rod 단일 대상
        ▼
 Rod 비활성 OTA app slot
```

## 확정 정책

- 무선 업데이트 대상은 `DF_Rod.ino.bin` 하나다. bootloader, partition 및 `boot_app0.bin`은 유선 전체 설치에서만 갱신한다.
- Main에 등록되어 현재 연결된 Rod 한 대만 대상으로 하며 broadcast 업데이트를 허용하지 않는다.
- Rod는 OTA frame의 송신 주소가 저장된 Main 주소와 일치할 때만 수신한다.
- 현재 4MB default partition과 Arduino-ESP32 Core 2.0.17을 유지한다. OTA app slot 절대 크기는 1,310,720 bytes이고 서비스 허용 상한은 1,250,000 bytes로 제한한다.
- packet 누락은 ACK/timeout/retry로 복구하고 전체 image 검증 성공 후에만 부팅 slot을 전환한다.
- 전송 중단 시 이어받지 않고 처음부터 다시 수행한다.
- 새 firmware의 기능 이상에 대한 자동 rollback은 구현하지 않는다. 이 경우 사용자가 유선 전체 재설치한다.
- OTA 중 Rod 일반 출력과 입력 전송을 정지시키고, 실패 또는 취소 시 기존 application 동작으로 복귀한다.
- OTA 수신 기능을 포함한 최초 Rod 기준 firmware는 한 번 유선 설치해야 한다.
- 자체 코드는 C++98/03 문법, 고유 접두어 및 namespace 미사용 정책을 따른다.

## 시험용 Rod 변경 (과거 Vr1.0.1.2 시험; 현재 제거됨)

- Rod HANDLE 홀센서(GPIO18)의 입력 변화에서 오른쪽 버튼 LED(GPIO9)를 즉시 켠다.
- 마지막 입력 변화로부터 200ms 후 버튼의 원래 LED 상태로 복귀한다. R 버튼을 누르지 않은 상태에서 시험하면 꺼진다. 연속 회전 중에는 점등 시간이 연장된다.
- `delay()` 없이 `millis()` 경과 시간으로 처리하며 기존 encoder count 계산과 전송 규격은 유지한다.
- 실제 pin은 `BTN_LED_RT_BLUE_PIN`, 출력은 active-low인 `BTN_LED_ON`/`BTN_LED_OFF`를 사용한다.
- 이 동작은 무선 업데이트된 Rod application이 실행 중임을 사용자가 확인하는 시험 표식이다.

## 세부 작업

### A. 현행 OTA/파티션/통신/시험 LED 경계 확정 (완료: 2026-08-28)

### B. 공유 OTA frame 계약과 기존 Main `$DN R/r` 오동작 차단 (완료: 2026-08-28)

### C. Rod OTA 수신기, 등록 Main 검증과 비활성 app slot 기록 (완료: 2026-08-28)

### D. Main 단일 Rod 중계 상태기계와 서비스 PC 전송 도구 (완료: 2026-08-28)

### E. HANDLE 감지 시 오른쪽 버튼 LED 200ms 시험 동작과 Rod 시험 버전 (완료: 2026-08-28, 보완: 2026-08-31)

### F. Main/Rod build, protocol host test와 오류 주입 검사 (완료: 2026-08-28)

### G. 최초 유선 기준 설치와 Main 경유 Rod 무선 업데이트 장비 gate (정상 경로 완료: 2026-08-31)

### H. 결과 기록, 사용자 재설치 절차와 배포 경계 확정

### I. HANDLE 시험 표식 제거 및 USB 직접 복구 (완료: 2026-08-31)

### J. 전체 소스·함수·프로그램 흐름 PowerPoint 인수인계 (완료: 2026-08-31)

## 진행

잔여 작업 0개 (H는 2026-09-02 사용자 지시로 수행 범위 제외)

## 안전 경계

- 현재 Main의 `$DN` parser에서 `R/r` 표기는 Rod용으로 주석 처리되어 있지만 실제 `Update.write()` 대상은 Main이다. B 완료 전 Rod image를 기존 `$DN R/r` 경로로 보내지 않는다.
- OTA binary frame을 기존 문자열 PID parser에 그대로 통과시키지 않는다. 길이가 명시된 binary buffer와 별도 frame 판별을 사용한다.
- Rod ESP-NOW callback에서 flash 기록, 문자열 분석, ACK 재전송을 수행하지 않는다. callback은 source address, length와 payload 게시만 담당한다.
- MAC 계층 송신 성공을 application 기록 성공으로 간주하지 않는다. Rod가 block 번호와 기록 결과를 ACK해야 다음 block으로 진행한다.
- image 크기, Rod 대상 식별, 전체 길이와 전체 검증값이 맞지 않으면 부팅 slot을 전환하지 않는다.
- OTA 중 진동, 브레이크 및 버튼 LED의 외부 제어 명령은 실행하지 않는다.
- bootloader와 partition의 무선 갱신은 범위 밖이다.

## 완료 조건

- Main이 등록·연결된 단일 Rod가 아니면 업데이트를 시작하지 않는다.
- Rod가 저장된 Main 주소가 아닌 송신자의 OTA frame을 거부한다.
- 현재 Rod application image가 비활성 OTA slot에 완전하게 기록되고 검증 후 재부팅한다.
- 중단, 중복, 누락, 순서 오류, 크기 초과 및 잘못된 대상 image가 기존 application을 손상시키지 않는다.
- 과거 G 정상 OTA 시험에서는 HANDLE LED 표식으로 실행을 확인했다. 사용자 요청으로 I에서 해당 표식을 제거했으며 현재 동작 요구사항은 아니다.
- Main/Rod compile/link, host test와 실제 Main 경유 장비 시험 결과를 사실대로 기록한다.

## A 결과

- 현재 partition은 `otadata`, `app0`, `app1`을 포함하며 app slot은 각각 `0x140000`(1,310,720 bytes)다.
- 현재 `DF_Rod.ino.bin`은 762,512 bytes로 OTA slot에 들어가며 서비스 상한 1,250,000 bytes에도 여유가 있다.
- Main에는 USB binary를 자기 OTA slot에 쓰는 `Update.begin/write/end` 구현이 있지만 Rod에는 `Update` 기록 구현이 없다.
- 기존 Main 다운로드 주석은 `R/r=Rod`를 정의하지만 실제 실행 경로는 대상 구분 없이 Main의 `Update.write()`를 호출한다. B에서 Rod 표기를 거부하도록 먼저 보호한 뒤 새 중계 경로를 분리한다.
- Rod ESP-NOW callback은 현재 최대 127-byte payload를 복사하지만 `mac_info`를 보존하지 않는다. C에서 source address를 payload와 함께 게시하고 OTA frame은 저장된 `main_board_addr`와 일치할 때만 처리한다.
- 기존 encoder는 회전 count 문자열이 바뀔 때 `rotateChangeCallback()`을 호출한다. 오른쪽 버튼 LED에는 이미 10ms 비차단 pattern 제어기와 100ms 설정 범위가 있어 E에서 이를 재사용할 수 있다.
- 현재 작업은 설계/기준 확정만 수행했다. source, build output, firmware version 및 장비 상태는 변경하지 않았다.

## B 결과

- `DFProtocol`에 C++03 호환 Rod OTA binary frame encode/decode와 CRC32를 추가했다. packed struct나 namespace를 사용하지 않고 byte offset과 big-endian 정수 helper로 wire 형식을 고정했다.
- 공통 frame은 `DFRO` magic, protocol version, frame type, session ID, sequence, payload length, 최대 96-byte payload와 CRC32로 구성되며 최대 길이는 115 bytes다. 기존 128-byte ESP-NOW 수신 경계 안에 들어간다.
- frame type은 `Start`, `Data`, `Finish`, `Abort`, `Ack` 다섯 개다. Start payload는 Rod target, image size, 32-byte digest와 최대 24-byte version을 전달하고, Ack payload는 응답 frame type, 상태, 다음 sequence와 기록 byte 수를 전달하도록 offset을 확정했다.
- 상태값은 정상, frame 오류, 미등록 송신자, busy, sequence 오류, flash write 오류, image 오류, size 초과, target 오류와 취소를 구분한다.
- 기존 Main `$DN R/r` frame은 `Exec_Download()` 진입 즉시 오류 3으로 거부하고 진행 중인 잘못된 session과 buffer를 정리한다. 이 경로에서는 `Update.begin/write/end(true)`가 실행되지 않는다.
- `tools/test-protocol.cmd`를 추가했다. C++98 host test에서 기존 PID encode/decode, OTA 최대 frame, 알려진 CRC32 값, encode/decode 왕복, CRC 손상, 길이 손상, payload 초과와 빈 Finish frame을 확인했다.
- `tools/build-all.cmd Release x64` 성공: Main application 883,129 bytes/배포 bin 883,488 bytes, Rod application 762,153 bytes/배포 bin 762,512 bytes다.
- VS2022 `DF_Firmware.sln` Release/x64 Rebuild는 일반 사용자 환경 재실행에서 경고 0, 오류 0으로 성공했다. 최초 sandbox 실행은 사용자 Windows SDK 경로 접근 제한으로 실패했으며 source/build 결함이 아니다.
- firmware version, Rod 동작과 장비 flash는 변경하지 않았다.

## C 결과

- Rod ESP-NOW callback이 payload, 길이와 송신 주소를 고정 슬롯에 함께 게시하도록 변경했다. callback에서는 flash 기록, 문자열 분석 또는 ACK 송신을 수행하지 않는다.
- `DF_Rod_Ota` 모듈을 추가했다. 저장된 `main_board_addr`와 일치하는 송신자의 `DFRO` frame만 처리하며 Start/Data/Finish/Abort, session, sequence, 중복 block과 기록 byte 수를 관리한다.
- `Update.begin(imageSize, U_FLASH)`로 비활성 app slot을 열고 최대 1,250,000 bytes만 허용한다. 각 data block 기록과 streaming SHA-256 계산을 수행하고 전체 길이와 digest가 모두 일치할 때만 `Update.end(false)` 후 재시작한다.
- OTA 진행 중 진동과 버튼 LED를 끄고 일반 Main 제어 frame은 실행하지 않는다. ACK 재전송을 위해 이미 기록한 sequence의 중복 data는 다시 쓰지 않고 현재 진행 위치를 응답한다.
- OTA 수신 기능만 포함한 `Vr1.0.1.0` 기준 image를 `bin/release/x64/Vr1.0.1.0`에 생성했다. 이 image는 최초 한 번 Rod에 유선 설치해야 한다.

## D 결과

- Main에 `$OR<DFRO frame hex>%` 전용 서비스 경로를 추가했다. 기존 문자열 PID와 `$DN` download 경로를 사용하지 않는다.
- Main은 Rod 연결 상태와 등록 모드 여부를 확인한 뒤 정상적인 Start/Data/Finish/Abort frame만 현재 `slave_board_addr`로 raw ESP-NOW 전송한다.
- 등록된 Rod 주소에서 수신한 정상 Ack frame만 `$OR<ACK frame hex>%`로 USB에 반환한다. 다른 송신 주소, 손상 frame과 Ack 이외의 binary frame은 서비스 응답으로 전달하지 않는다.
- `tools/upload-rod-ota.ps1`과 `.cmd` wrapper를 추가했다. 도구는 Rod application 크기와 version을 검사하고 SHA-256, session, sequence, CRC32 frame을 생성해 block별 ACK/timeout/최대 5회 retry로 전송한다. `-DryRun`은 장비 없이 전체 image frame 생성을 확인한다.

## E 결과

- Rod encoder count 문자열이 실제로 바뀌어 `rotateChangeCallback()`이 호출되면 기존 오른쪽 버튼 LED 비차단 제어기를 `1회, 100ms`로 시작한다.
- blocking `delay()`와 새 timer는 추가하지 않았으며 OTA 진행 중에는 해당 callback을 실행하지 않는다.
- Rod 시험 version을 `Vr1.0.1.1`로 올리고 `Version.cpp`, `tools/firmware-versions.cmd`와 배포 폴더 이름을 함께 갱신했다.

## F 결과

- protocol host test 성공: CRC 손상, 길이 손상, payload 초과와 frame 왕복 오류 주입을 포함한다.
- OTA uploader dry run 성공: 최종 `DF_Rod.ino.bin` 766,896 bytes, data frame 7,989개를 생성했다. SHA-256은 매 build image에서 다시 계산해 Start frame에 싣는다.
- Main Release build 성공: application 884,857 bytes, 배포 bin 885,216 bytes.
- Rod 최종 Release build 성공: application 766,525 bytes, 배포 bin 766,896 bytes. app slot 1,310,720 bytes 및 서비스 상한 1,250,000 bytes 이내다.
- VS2022 `DF_Firmware.sln` Release/x64 Rebuild 성공: 경고 0, 오류 0. 최초 sandbox 실행의 사용자 SDK 접근 거부는 외부 재실행에서 해소됐다.
- G 장비 gate 전까지 flash와 물리 장비 상태는 변경하지 않았다.

## G 장비 gate 절차

1. Rod에 `Vr1.0.1.0`의 네 파일을 유선 전체 설치한다. 이 기준 image에 OTA 수신기가 들어 있다.
2. Main에 `Vm1.0.9.0`의 네 파일을 유선 전체 설치한다. 이 image에 USB↔ESP-NOW 중계기가 들어 있다.
3. Main/Rod가 기존 등록 주소로 정상 연결되는지 확인한다.
4. Main USB 포트에서 `tools/upload-rod-ota.cmd <MAIN_PORT> bin\release\x64\Vr1.0.1.2\DF_Rod.ino.bin Vr1.0.1.2`를 실행한다. OTA 기반 firmware가 이미 설치돼 있으면 1~2를 반복하지 않는다.
5. `[OTA] Completed`와 Rod 재부팅 후 `Vr1.0.1.2` 응답을 확인한다.
6. R 버튼을 누르지 않은 상태에서 HANDLE을 천천히 돌린다. LED가 즉시 켜지고 회전을 멈춘 후 약 0.2초에 꺼지는지 확인한다.

## HANDLE 미점등 보완 (2026-08-31)

- 사용자 보고: 대상은 Rod HANDLE 회전이며 앞선 시험에서 R LED가 점등되지 않았다. 실제 설치 버전과 OTA 완료 출력은 아직 확인되지 않았으므로 미점등 원인을 확정하지 않는다.
- `Encoder::rotate()`의 원시 홀센서 변화에 activity callback을 연결했다. 상위 전송 문자열 변경 여부 및 계산용 테스트 callback과 LED 동작을 분리했다.
- 기존 패턴 제어기 재시작 대신 즉시 점등 및 마지막 펄스 후 200ms 복귀로 변경했다. OTA 중에는 동작하지 않는다. source/배포 version은 `Vr1.0.1.2`다.
- 이 보완 당시에는 실제 설치 버전 및 사용자 HANDLE 결과를 기다렸다. 후속 확인 결과는 아래 G 결과에 기록한다.
- 2026-08-31 자동 확인: VS2022 Release/x64 Rebuild 경고 0/오류 0, protocol host test 및 OTA dry run 성공. Main bin 885,216 bytes, Rod `Vr1.0.1.2` application 766,713 bytes/bin 767,072 bytes, OTA data frame 7,991개. Rod 폴더 내 배포 파일은 4개다. flash는 수행하지 않았다.

## 승인된 장비 업데이트 결과 (2026-08-31)

- 사용자 지시: “업데이트 승인할게. 진행해”. 이번 장비 업데이트에 한해 Codex 직접 업로드를 승인받았다. 일반 수동 장비 gate 정책은 유지한다.
- 업데이트 전 Rod 직접/무선 버전 응답은 `Vr1.0.1.0`이었고 Main은 OTA 중계 검사에 응답하지 않았다. 시험 변경 version이 실행 중이라고 볼 근거가 없었다.
- Main 중계 image와 Rod OTA 기준 image를 115200 baud ROM bootloader `--no-stub` 방식으로 유선 설치했다. 양쪽 모두 bootloader/partition/boot_app0/application 네 영역의 hash 및 verify가 일치했다. 전체 flash erase와 페어링 주소 재등록은 하지 않았다.
- 이후 최종 Rod application은 Main USB→ESP-NOW로만 전송했다. `Vr1.0.1.2`, 767,072 bytes의 모든 block/Finish ACK를 받아 `[OTA] Completed` 및 종료 코드 0을 확인했다.
- 전송 image SHA-256: `FA5D18C9AAD4CB7B9B0A21F3EC92105FB2E975166B9E4C83FFDD08FE5A7878B7`.
- 재부팅 후 Main 경유 응답은 `Vm1.0.9.0,Vr1.0.1.2`, Rod USB 직접 응답은 `Vr1.0.1.2`였다. 두 조회의 관찰 구간에서 panic/Guru Meditation/Brownout 표시는 검출되지 않았다. 장시간 안정성 시험으로 간주하지 않는다.
- 업로드 직후 사용자에게 R 버튼 해제 상태의 HANDLE 회전·정지 후 200ms 소등 확인을 요청했다. 후속 사용자 확인은 아래에 기록한다. 실패/전원중단 장비 시험은 미수행이다.

## G 사용자 관찰 결과 및 소요 시간 (2026-08-31)

- 사용자 보고: “LED 점등도 완료되었고, 소등도 확인했고, 연속 회전 중에는 계속 켜졌어”. HANDLE 회전 점등·정지 후 소등·연속 회전 중 점등 유지 세 항목을 모두 사용자 확인 완료로 기록한다. 200ms 정확도는 계측하지 않았으며 코드 설정값이다.
- 실제 실행 명령 형식: `tools/upload-rod-ota.cmd <MAIN_PORT> bin\release\x64\Vr1.0.1.2\DF_Rod.ino.bin Vr1.0.1.2`. 포트는 Rod가 아니라 Main USB 포트다. PC→Main 서비스 frame은 `$OR<DFRO frame hex>%`이며 기존 `$DN R/r`를 사용하지 않는다.
- 이번 OTA 명령 호출은 11:16:54.119, 완료 출력 수신은 11:19:43.376(KST)로 실행 로그 기준 약 169.3초(2분 49초)다. PC image/frame 준비, block ACK 및 최종 검증 대기를 포함하며 앞선 유선 기반 설치·이후 버전 조회 시간은 제외한다. 무선 구간만의 별도 계측값은 아니다.
- 대상은 767,072-byte Rod application, data frame 7,991개다. 정상 OTA·재부팅·버전 조회·HANDLE 시험을 묶어 G 정상 경로를 완료한다.
- H 마감 전 잔여: 전송 중단/재시도, 잘못된 대상·크기·digest 거부, 전원 차단 후 기존 앱 유지 및 유선 재설치 복구의 실제 장비 검증. 이번 정상 경로 성공으로 이 항목을 통과 처리하지 않는다.

## I 결과: 시험 표식 제거 및 USB 복구 (2026-08-31)

- “빠른 업데이트”는 사용자 정정에 따라 Rod USB 직접 업로드를 의미한다. 이번 요청에 한하여 Codex 직접 기록 권한이 있으며 상시 장비 제어 권한으로 확대하지 않는다.
- `Encoder::setActivityCallback`과 원시 입력 activity 호출, Communication의 시험 LED 상태/200ms 만료 함수, Application의 등록/loop 호출을 제거했다. 원래 encoder 보고와 버튼·부팅·연결 LED 및 OTA 수신은 보존했다.
- 변경 전 5개 파일을 `deprecated/rod/handle-led-test-Vr1.0.1.2/`에 보존했다. source/배포 버전은 `Vr1.0.1.3`이며 기준 legacy는 변경하지 않았다.
- Main/Rod Release x64 compile/link와 protocol host test exit 0. Main bin 885,216 bytes, Rod application 766,485 bytes/bin 766,848 bytes; Rod SHA-256 `06404D1419D77BE56D025951B5175A15128C2D7AB3B4328B4D655B2DEFB62EBC`.
- 직접 버전 조회로 Rod를 식별한 뒤 장비 파티션과 배포 파티션의 SHA-256 일치를 확인했다. ROM no-stub 921600 baud 압축으로 otadata와 app0만 기록하고 두 영역 hash/verify 성공, 재부팅 후 `Vr1.0.1.3` 확인. bootloader/partition/NVS/SPIFFS, Main은 기록하지 않았다. 전체 erase도 수행하지 않았다.
- 파티션 확인부터 쓰기/검증까지 33.86초(부팅 버전 조회 제외), application 압축 기록 약 21.7초. 근거는 `artifacts/rod-restore-20260831/usb-upload.log`, `result.json`, `build.log`다.
- 복구 후 물리 확인: R 버튼을 놓고 HANDLE을 돌릴 때 시험용 오른쪽 LED가 켜지지 않는지 사용자 확인 대기다. 코드 제거/기록/버전 확인 완료를 장비 동작 확인으로 확대하지 않는다.

## J 결과: 소스 PowerPoint 인수인계 (2026-08-31)

- 산출물: `docs/handoff/DF_Firmware_Source_Handoff_2026-08-31.pptx`, 총 140장.
- 전체 소스를 VS2022 솔루션/물리 폴더로 보는 방법과 F12/호출 참조 탐색, Main/Rod 초기화·loop·주기 처리·callback/ISR·USB/OTA 흐름을 작성했다.
- 활성 경로 소스 정의 555개(Main 420, Rod 127, 공유 8)를 파일별 함수/역할/줄 번호 표로 수록했다. 빈 실행문·기존 예약 함수도 구분했으며, 선언·SDK 함수·runtime coverage와 혼동하지 않도록 범위를 표시했다. 시그니처/직접 호출 목록/출처는 발표자 노트에 포함했다.
- 140장 모두 렌더링하고 개별 시각 검토했다. 제목 겹침 1곳을 수정했으며 함수 표 목록 555개 일치, 슬라이드/노트 각 140개, 개인 장비 주소·포트 패턴 없음 및 캔버스 경계를 확인했다.
- 근거: `artifacts/handoff-ppt-20260831/`의 source catalog, slide plan, render, QA 결과. 사용자 요청 산출물은 PowerPoint이며 중간 파일은 배포 문서가 아니다.
- I/J 완료 후 잔여 문자 작업은 H 1개, 잔여 번호 작업은 4개다. 실패 주입·전원 중단 장비 검증을 완료 처리하지 않았다.
## 2026-09-02 범위 종료

- 사용자는 H의 실패 복구 검증과 최종 배포 경계 마감을 더 이상 필요하지 않은 작업으로 제외했다.
- 정상 무선 업데이트 구현과 과거 시험 결과는 보존한다. 후속 기능 보완은 작업 11-J에서 관리한다.
