# 작업 08

독립 모듈 정착과 상태/Callback 안전성

## 목표

작업 07에서 확정한 순서에 따라 `.inc`를 독립 C++98/03 `.cpp/.h`로 정착시키고, 전역 상태, 객체 초기화, ESP-NOW callback과 고정 버퍼 경계를 명시적으로 안전하게 만든다.

## 착수 기준

- 기능 경계는 `.inc`로 나뉘었지만 application `.cpp`의 단일 번역 단위에 의존한다.
- 헤더 정의 전역과 다수의 전역 상태 owner가 명확하지 않다.
- Main/Rod 수신 callback의 길이, 문자열 처리 및 callback-to-loop 전달을 재검토해야 한다.
- 세부 실행 순서는 작업 07의 승인된 초안을 기준으로 한다.

## 관련 영역

- Main/Rod application state, module API, hardware object ownership, ENow callbacks, scheduler

## 구현 원칙

- 한 번에 한 모듈 또는 한 안전성 항목만 변경한다.
- C++03 호환 구조체와 명시적인 owner를 사용한다.
- callback은 고정 버퍼 복사와 flag 설정만 담당하게 한다.
- 물리적 이동과 동작 변경을 같은 문자 작업에 섞지 않는다.

## 세부 작업

실행 기준은 `docs/refactoring/MODULE_MIGRATION_PLAN.md`다.

### A. 현재 unity 구조 기준 Main/Rod build와 최초 error 기준 확정 (완료: 2026-08-28)

### B. 공통 선언, compatibility 경계와 ODR 1차 정리 (완료: 2026-08-28)

### C. Main leaf 모듈 독립 번역 단위 전환 (완료: 2026-08-28)

### D. Main control 모듈 독립 번역 단위 전환 (완료: 2026-08-28)

### E. Main scheduler/application 정착, `.inc` 제거와 build (완료: 2026-08-28)

### F. Rod leaf 모듈 독립 번역 단위 전환 (완료: 2026-08-28)

### G. Rod control/application 정착, `.inc` 제거와 build (완료: 2026-08-28)

### H. Internal compatibility header 축소와 상태 owner/API 정착 (완료: 2026-08-28)

### I. ESP-NOW callback, 고정 버퍼와 ISR 경계 안전화 (완료: 2026-08-28)

### J. protocol host test와 정적 정책 검사

### K. 사용자 수동 부하/통신/장비 검증 게이트

### L. 활성 소스 주석/상수 정리 기준 확정 (완료: 2026-08-28)

## 진행

잔여 작업 2개 (J, K)

## 변경 금지

- 동적 메모리 기반 queue, 스마트 포인터, lambda callback을 도입하지 않는다.
- 독립 번역 단위 전환 중 protocol과 장비 동작을 의도적으로 변경하지 않는다.
- 주석 정리와 기능 변경을 같은 변경 단위에 섞지 않는다. 주석 처리된 구 코드, 테스트 로그와 단순 수치 변경 이력은 제거하고 현재 제약의 이유·단위·범위만 짧게 남긴다.
- 중요 값은 별도 대형 문서 대신 owner 모듈의 고유 접두어 `const`/`enum`으로 이름을 부여한다. 장비 교정에 실제 필요한 값만 최소 표로 문서화한다.

## 완료 조건

- 활성 application이 `.inc`를 include하지 않는다.
- 수신 길이 초과가 차단되고 callback은 경량이다.
- 전역 상태와 객체 소유 위치 및 공개 API가 명확하다.
- Main/Rod compile/link 결과와 요구 수동 결과가 사실대로 기록되어 있다.

## 시험 방법

- 각 독립 모듈 전환 단계에서 관련 대상 compile/link를 수행한다.
- 사용자가 연속 ESP-NOW 송수신, 비정상 길이 입력, 재연결 및 장시간 장비 시험을 수행한다.

## 결과

- 2026-08-28 08-A: `tools/build-all.cmd Release x64`를 실행해 Main에서 `modules/DF_Main_Foundation.inc: No such file or directory`를 재현했다. Arduino CLI가 `.inc` unity 구조를 정상 build하지 못하는 것을 출발 결함으로 확정했다.
- 2026-08-28 08-B: Main/Rod Foundation 저장 공간과 `DF_*_Internal.h` compatibility 선언을 분리하고 각 전역 저장 공간이 한 번만 정의되도록 정리했다. 이 internal header의 축소와 최종 owner API 정착은 08-H에 남겼다.
- 2026-08-28 08-C~E: Main 활성 `.inc` 9개를 대응 `.cpp/.h`로 전환하고 `DF_Main_Application.cpp`가 setup/loop 구현을 소유하도록 이동했다. 모든 파일을 `vs/DF_Main.vcxproj` compile/include 항목에 등록했다.
- 2026-08-28 08-F~G: Rod 활성 `.inc` 7개를 대응 `.cpp/.h`로 전환하고 `DF_Rod_Application.cpp`가 setup/loop 구현을 소유하도록 이동했다. 모든 파일을 `vs/DF_Rod.vcxproj`에 등록했다.
- 2026-08-28 자동 검증: 활성 `.inc` 파일과 `.inc` include 0개, 두 VS project의 모든 등록 경로 존재를 확인했다.
- 2026-08-28 `tools/build-all.cmd Release x64`: 성공. Main application 882,281 bytes/global 153,436 bytes/배포 bin 882,640 bytes, Rod application 761,721 bytes/global 45,064 bytes/배포 bin 762,080 bytes다. 대상별 flash 파일 4개를 생성했다.
- 2026-08-28 VS2022 MSBuild `DF_Firmware.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64`: 성공. 최초 sandbox 실행은 사용자 SDK 경로 접근 거부였고 승인된 일반 사용자 환경 재실행에서 Main/Rod 모두 성공했다.
- 2026-08-28 08-L: 활성 Main 23,012줄과 Rod 4,946줄을 분석했다. 주석 전용 4,355줄, 인라인 주석 3,209줄이며 자동 식별된 주석 처리 코드가 최소 634줄이다. 정리 우선순위는 `DF_Main_Scheduler.cpp`, `DF_Main_GameControl.cpp`, `DF_Main_State.cpp`로 정했고, 실제 삭제는 각 owner/API 작업 뒤 별도 기계적 변경과 build로 확인한다.
- 2026-08-28 08-H: 공용 저장 공간 owner를 `DF_Main_State.cpp/.h`, `DF_Rod_State.cpp/.h`로 정착시키고 기능 상태 선언은 각 모듈 헤더로 이동했다. 공개 모듈 헤더에서 internal include를 모두 제거했으며 단일 구현에서만 쓰는 상태 87개를 `static`으로 제한했다. `DF_Main_Internal.h`는 44,911바이트에서 406바이트, Rod는 6,872바이트에서 327바이트의 구현용 집계 헤더로 축소했다.
- 2026-08-28 08-H 검증: `tools/build-all.cmd Release x64`와 VS2022 Solution Rebuild가 모두 성공했다. Main application 882,173 bytes/global 153,404 bytes/배포 bin 882,544 bytes, Rod application 761,721 bytes/global 45,064 bytes/배포 bin 762,080 bytes이며 대상별 파일 4개를 확인했다. VS project 누락 경로와 공개 header의 internal include는 0개다.
- 2026-08-28 08-I: Main/Rod ESP-NOW 수신 callback에 `2 <= length < 128` 경계를 적용하고, 고정 단일 슬롯에 데이터·길이·Main 송신 주소를 함께 게시하도록 변경했다. 처리 지연 중 새 프레임은 기존 동작처럼 최신 값으로 교체하되 교체/비정상 입력 횟수를 기록한다. callback의 `String`, PID 분석과 로그는 loop 처리 함수로 이동했다.
- 2026-08-28 08-I: 일반 ESP-NOW callback/작업 함수에는 task용 임계영역 API를, 실제 timer/encoder ISR에는 ISR용 API를 사용하도록 구분했다. encoder ISR의 count/direction/distance/event buffer 게시와 task 측 복사를 같은 임계영역으로 묶어 다중 필드 snapshot 불일치를 막았다.
- 2026-08-28 08-I 검증: `tools/build-all.cmd Release x64`와 VS2022 `DF_Firmware.sln` Rebuild가 모두 성공했다. Main application 882,749 bytes/global 153,508 bytes/배포 bin 883,120 bytes, Rod application 762,153 bytes/global 45,152 bytes/배포 bin 762,512 bytes다. `bin/release/x64/Vm1.0.9.0`, `Vr1.0.1.0`에 대상별 flash 파일 4개를 확인했다.
- 2026-08-28 08-K 부분 검증: 사용자 승인 후 외부 장치가 연결되지 않은 Main 테스트 보드에 현재 4개 flash 영역을 기록했다. native USB의 flasher stub 방식은 전송 전환 후 끊겼으나, 115200 baud ROM bootloader `--no-stub` 방식은 네 영역 쓰기와 digest 재검증이 모두 성공했다. 12초 연속 규격 출력에서 exception/watchdog/reset 반복이 없었고, 읽기 전용 버전 요청에 `Vm1.0.9.0` 응답을 확인했다.
- 2026-08-28 Main 강제 reset 재현: reset 직후 전원 reset 통지 `$2211%`, Rod 등록 대기시간 `$010225%`, AP 상태 `$1800%`, Rod 미연결 `$2000%`, 배터리 미확정/변화 없음 `$12-2%`/`$12-3%` 순으로 출력됐다. 15초 수집 중 exception, watchdog 및 반복 reset은 없었다.
- 테스트 보드의 물리 flash는 8MB로 검출됐으나 현재 제품 설정과 partition은 기존 기준인 4MB를 유지했다. 이 테스트 보드는 제품 하드웨어 기준 변경 근거로 사용하지 않는다.
- 2026-08-28 08-K Rod 부분 검증: 사용자 승인 후 Rod 테스트 보드에 현재 배포 파일 네 영역을 115200 baud ROM bootloader `--no-stub` 방식으로 기록했다. 네 영역의 write hash와 명시적 verify가 모두 일치했으며 `Vr1.0.1.0` 부팅 로그와 exception/watchdog 없음도 확인했다.
- Main/Rod 테스트 보드의 저장된 상대 장치 정보를 서로 맞추고 재시작한 뒤, Main 버전 질의에 `Vm1.0.9.0`과 `Vr1.0.1.0`이 함께 반환되어 ESP-NOW 왕복을 확인했다. 장비 주소와 포트는 기록하지 않았다.
- Main을 통해 Rod에 1회/999ms/20ms-off 자체 종료 진동 명령을 보냈고 약 2초 뒤 별도 OFF 명령도 전송했다. 두 명령의 Main 에코와 양쪽 runtime fault 문구 없음은 확인했으며, 이어진 사용자 관찰 결과는 아래에 기록했다.
- 사용자 지시: Rod 테스트 보드 덮어쓰기를 승인하고 현재 Rod firmware 업로드 후 사용자가 인지할 수 있도록 약 1초간 진동 명령을 수행하도록 요청했다.
- 사용자 확인 결과: 릴 연동 LED가 점멸 상태로 전환되었고 진동이 약 1초간 동작했다. 따라서 이번 Main–Rod 통신, 명령 전달 및 Rod 진동 출력 항목은 통과로 판정한다.
- 08-K는 부분 완료 상태다. BLDC, encoder 및 기타 외부 장치의 ISR 부하와 실제 입출력은 시험하지 못했다. 다음은 08-J 자동 검사와 잔여 실제 장비 검증 gate다.
