# 작업 06

Rod 펌웨어 물리적 모듈 분리

## 목표

`DF_Rod.ino`의 정상/설정 모드, 통신, IMU, 입력, 배터리 및 전원 처리를 기능별 C++98/03 모듈로 분리한다.

## 현재 기준

- `DF_Rod.ino`는 3,065줄이며 여러 장치와 mode 처리가 결합되어 있다.

## 관련 영역

- 향후 `firmware/DF_Rod/DF_Rod.ino`, `firmware/DF_Rod/src/`

## 구현 원칙

- 현재 최신 Rod 하드웨어만 지원한다.
- IMU UART와 IMU application 정책을 구분한다.
- setup/normal mode 동작 순서를 보존한다.

## 세부 작업

### A. Utility와 PC setup protocol 분리 (완료: 2026-08-27)

### B. Rod registration 분리 (완료: 2026-08-27)

### C. Battery monitor 분리 (완료: 2026-08-27)

### D. Button/Encoder input control 분리 (완료: 2026-08-27)

### E. IMU service 분리 (완료: 2026-08-27)

### F. Power/Sleep 및 Main communication 분리 (완료: 2026-08-27)

### G. Scheduler와 Rod application entry 분리 (완료: 2026-08-27)

### H. 단계별 수동 빌드와 주요 smoke 게이트 (미수행: 2026-08-27, 사용자 지시)

- 각 문자 이동 뒤 Codex가 build를 실행한다.
- 입력, IMU, 등록, 통신 및 mode 경계에서는 장비 smoke test도 요청한다.

## 진행

잔여 구현 작업 0개. 빌드 및 장비 smoke 검증 미수행.

## 변경 금지

- IMU 명령 형식, button/encoder 의미, registration 및 sleep sequence를 파일 이동과 함께 바꾸지 않는다.

## 완료 조건

- `.ino`에는 `setup()`과 `loop()` 위임만 남고 각 장치/서비스 구현 경계가 분리된다. 독립 번역 단위 빌드는 검증하지 않았다.

## 시험 방법

- 모든 문자 작업에 사용자 수동 compile/link 결과가 필요하며 관련 장비 smoke 결과가 필요하다.

## 결과

- 2026-08-27 작업 06-A~G 완료: `DF_Rod.ino`를 10줄의 setup/loop 위임 파일로 축소하고 Foundation, Communication, SetupProtocol, BatteryMonitor, PowerAndImu, Registration, SchedulerAndInput 구현 경계로 분리했다.
- 기존 Battery, Button, Encoder, IMU, ESP-NOW 및 장치 driver는 `firmware/DF_Rod/src`로 이동했다. 현재 Rod는 New Board V2, Button V2, Break 없음, Encoder V3, Battery 800 V2로 고정하고 구보드 GPIO와 runtime 선택 경로를 활성 코드에서 제거했다.
- 구현 파일은 자동 생성 선언과 기존 전역 정의 순서를 보존하는 `.inc` unity module이며 Application `.cpp`에서만 순서대로 include한다. 구조 근거는 `docs/architecture/ACTIVE_SOURCE_LAYOUT.md`에 기록했다.
- 2026-08-27 작업 06-H는 사용자 지시에 따라 compile/link, firmware 생성, 입력/IMU/등록/통신 smoke를 수행하지 않았다.
- 2026-08-28 후속 작업 08-F~G에서 모든 Rod `.inc`를 대응 `.cpp/.h` 독립 번역 단위로 전환하고 VS2022 project에 등록했다. Arduino CLI clean build와 VS2022 Solution Rebuild가 성공했으며 장비 smoke는 여전히 미수행이다.
