# 작업 05

Main 펌웨어 물리적 모듈 분리

## 목표

`DF_Main.ino`의 동작을 유지하면서 기능별 C++98/03 `.cpp/.h` 번역 단위로 이동한다.

## 현재 기준

- `DF_Main.ino`는 22,714줄과 약 359개 최상위 함수를 포함한다.
- 기존 driver 클래스는 별도 파일로 일부 분리되어 있다.

## 관련 영역

- 향후 `firmware/DF_Main/DF_Main.ino`, `firmware/DF_Main/src/`

## 구현 원칙

- 함수 이동, 이름 변경, 상태 변경을 같은 단위에 섞지 않는다.
- 전역 객체는 초기 물리 분리 동안 한 소유 위치에 유지한다.
- ISR, timer, ESP-NOW callback은 후순위다.

## 세부 작업

### A. Utility와 문자열 처리 분리 (완료: 2026-08-27)

### B. DeviceCheck와 Diagnostics 분리 (완료: 2026-08-27)

### C. LED application control 분리 (완료: 2026-08-27)

### D. AP/UART와 Rod 상위 통신 분리 (완료: 2026-08-27)

### E. LM/Wire control 분리 (완료: 2026-08-27)

### F. Bite/Hit/Hold/Game control 분리 (완료: 2026-08-27)

### G. Scheduler와 Main application entry 분리 (완료: 2026-08-27)

### H. 단계별 수동 빌드와 주요 smoke 게이트 (미수행: 2026-08-27, 사용자 지시)

- 각 문자 이동 뒤 Codex가 build를 실행한다.
- C, D, E, F, G 완료 지점에서는 관련 장비 smoke test도 요청한다.

## 진행

잔여 구현 작업 0개. 빌드 및 장비 smoke 검증 미수행.

## 변경 금지

- protocol, 상태머신 의미, motor timing, ISR 동작을 물리 이동과 함께 바꾸지 않는다.

## 완료 조건

- `.ino`에는 `setup()`과 `loop()` 위임만 남았다. 기능 경계는 unity implementation module로 분리했으며 독립 번역 단위 빌드는 검증하지 않았다.

## 시험 방법

- 모든 문자 작업에 사용자 수동 compile/link 결과가 필요하며 주요 경계마다 장비 smoke 결과가 필요하다.

## 결과

- 2026-08-27 작업 05-A~G 완료: `DF_Main.ino`를 10줄의 setup/loop 위임 파일로 축소하고 Application `.cpp/.h`, Foundation, Communication, LedApplication, ApplicationCommands, DeviceCheck, LmWire, GameControl, Diagnostics, Scheduler 구현 경계로 분리했다.
- 기존 driver `.cpp/.h`는 `firmware/DF_Main/src` 아래로 이동했다. 기능 구현 파일은 자동 생성 선언과 기존 전역 정의 순서를 보존하기 위해 Application `.cpp` 한 곳에서 순서대로 include하는 `.inc` unity module이다.
- 독립 `.cpp` 전환에 필요한 전역 상태 계약 정리는 작업 07 이후 검증 가능한 단위에서 수행해야 한다. 구조 근거는 `docs/architecture/ACTIVE_SOURCE_LAYOUT.md`에 기록했다.
- 2026-08-27 작업 05-H는 사용자 지시에 따라 compile/link, firmware 생성 및 장비 smoke를 수행하지 않았다.
- 2026-08-28 후속 작업 08-C~E에서 모든 Main `.inc`를 대응 `.cpp/.h` 독립 번역 단위로 전환하고 VS2022 project에 등록했다. Arduino CLI clean build와 VS2022 Solution Rebuild가 성공했으며 장비 smoke는 여전히 미수행이다.
