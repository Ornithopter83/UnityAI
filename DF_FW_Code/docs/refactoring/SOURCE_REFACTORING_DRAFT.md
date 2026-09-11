# Main/Rod 소스 리팩토링 초안

Updated: 2026-08-28  
Status: 작업 07 확정 초안 1.0

## 2026-08-28 구현 갱신

- 작업 08-A~G에서 Main/Rod 활성 `.inc`를 모두 대응 `.cpp/.h` 독립 번역 단위로 전환했다.
- `DF_Main_Application.cpp`, `DF_Rod_Application.cpp`가 setup/loop 구현을 소유한다.
- 모든 module source/header를 VS2022 project에 compile/include 항목으로 등록했다.
- Arduino CLI 통합 clean build와 VS2022 Solution Rebuild가 성공했다.
- 초기 전환 호환성을 위한 `DF_Main_Internal.h`, `DF_Rod_Internal.h`는 아직 넓은 전역 선언 경계다. 이 header 축소와 최소 owner API 정착은 작업 08-H에 남아 있다.
- callback/ISR 안전화, protocol test 및 장비 검증은 수행하지 않았다.

## 목적

현재의 1차 파일 분리 결과를 최종 구조로 오인하지 않고, Main/Rod 소스를 독립적인 C++98/03 `.cpp/.h` 모듈로 정착시키기 위한 기준을 먼저 확정한다. 이 문서는 동작 변경 작업서가 아니라 다음 코드 작업의 경계, 순서 및 검증 시점을 합의하기 위한 초안이다.

## 중간 상태 판정

### 완료된 기반

- 변경 전 원본은 `legacy/Vm1.0.9.0`, `legacy/Vr1.0.1.0`에 보존되어 있다.
- 활성 소스는 `firmware/DF_Main`, `firmware/DF_Rod`로 분리되어 있다.
- 양쪽 `.ino`에는 `setup()`, `loop()`와 application 위임만 남아 있다.
- 활성 제품 구성만 유지하고 제거한 구 코드는 `deprecated/`에 격리했다.
- 공통 메시지 정의의 기준 위치는 `libraries/DFProtocol/`로 정했다.

### 아직 완료되지 않은 부분

- 기능 파일은 독립 번역 단위가 아니라 `DF_*_Application.cpp`에 포함되는 `.inc` 형태다.
- 기존 단일 `.ino`의 선언 순서, Arduino 자동 함수 원형 생성 및 광범위한 전역 상태 의존성이 남아 있다.
- 모듈별 공개 API, private 상태, hardware object 소유자가 확정되지 않았다.
- 짧거나 충돌 가능성이 있는 기존 전역 심볼을 고유 접두어 체계로 정리하지 않았다.
- callback, 고정 버퍼, 초기화 순서 및 ODR 위험을 아직 안전화하지 않았다.
- 작업 03~06 이후 현재 활성 소스의 compile/link와 프로토콜 호환 시험을 수행하지 않았다.

따라서 현재 결과는 **대형 `.ino`를 기능 경계로 나눈 1차 구조 초안**이며, 최종 소스 리팩토링 완료 상태가 아니다.

## 상세 분석 문서

- Main 상태, 함수 의존성과 owner/API: `docs/refactoring/MAIN_DEPENDENCY_INVENTORY.md`
- Rod 상태, 함수 의존성과 owner/API: `docs/refactoring/ROD_DEPENDENCY_INVENTORY.md`
- `.inc` 제거 순서와 단계별 검증: `docs/refactoring/MODULE_MIGRATION_PLAN.md`

조사 결과 Main `.inc`는 약 17,011줄/확인 함수 338개, Rod `.inc`는 약 2,509줄/확인 함수 56개다. 단순 파일명 변경으로는 전역 저장 공간, 함수 원형과 순환 호출이 해결되지 않으므로 owner를 먼저 만들고 leaf 모듈부터 독립 번역 단위로 전환한다.

## 목표 의존 방향

```text
DF_Main.ino / DF_Rod.ino
        ↓
DF_*_Application
        ↓
기능별 공개 헤더와 독립 .cpp
        ↓
보드 드라이버 / DFProtocol / Arduino-ESP32 API
```

- `.ino`에는 Arduino 진입점인 `setup()`, `loop()`만 둔다.
- 다른 모듈의 `.cpp` 또는 `.inc`를 include하지 않는다.
- 모듈 간 호출은 최소 공개 헤더를 통해서만 수행한다.
- 각 전역 상태와 hardware object는 한 모듈만 저장 공간을 소유한다.
- callback은 길이 확인, 고정 버퍼 복사, flag 설정까지만 담당하고 해석과 장비 제어는 `loop()` 흐름에서 수행한다.
- 물리적 이동 단계에서는 PID, payload, timing 및 장비 동작을 바꾸지 않는다.

## Main 목표 모듈 초안

| 모듈 | 주 책임 | 우선 정리 대상 |
| --- | --- | --- |
| `DF_Main_Application` | 초기화와 최상위 실행 순서 | `setup/loop` 위임, 초기화 순서 |
| `DF_Main_State` | application 상태 저장 공간 | 전역 변수, 장비 객체 소유자 |
| `DF_Main_Communication` | ESP-NOW 송수신과 메시지 전달 | callback 경계, `DFProtocol` 사용 |
| `DF_Main_LedControl` | LED application 동작 | LED 상태와 명령 API |
| `DF_Main_DeviceCheck` | 장치 상태 확인 | 진단 결과 전달 경계 |
| `DF_Main_LmWireControl` | LM/Wire 제어 흐름 | motor/encoder 의존성 |
| `DF_Main_GameControl` | 게임 상태와 명령 처리 | 상태 전이, 통신 의존성 |
| `DF_Main_Diagnostics` | 로그와 진단 명령 | 운영 코드와 진단 코드 분리 |
| `DF_Main_Scheduler` | 주기 작업 호출 순서 | timing 소유권, callback 후속 처리 |

## Rod 목표 모듈 초안

| 모듈 | 주 책임 | 우선 정리 대상 |
| --- | --- | --- |
| `DF_Rod_Application` | 초기화와 최상위 실행 순서 | `setup/loop` 위임, 초기화 순서 |
| `DF_Rod_State` | application 상태 저장 공간 | 전역 변수, 장비 객체 소유자 |
| `DF_Rod_Communication` | ESP-NOW 송수신과 메시지 전달 | callback 경계, `DFProtocol` 사용 |
| `DF_Rod_SetupProtocol` | 설정 모드 통신 | 정상 운전과 설정 흐름 분리 |
| `DF_Rod_Registration` | Main 등록 및 재연결 | 상태 전이와 timeout |
| `DF_Rod_BatteryMonitor` | 배터리 측정과 상태화 | ADC 읽기와 보고 API |
| `DF_Rod_InputControl` | encoder/switch 입력 처리 | 입력 상태 소유권 |
| `DF_Rod_ImuService` | IMU 읽기와 변환 | 센서 객체와 결과 전달 |
| `DF_Rod_PowerControl` | 전원/절전 제어 | 종료 순서와 장비 상태 |
| `DF_Rod_Scheduler` | 주기 작업 호출 순서 | timing 소유권, callback 후속 처리 |

## 코드 작성 경계

- 프로젝트 작성 코드는 C++98/03 문법 범위만 사용한다.
- lambda, 스마트 포인터, `auto`, `nullptr`, `constexpr`, `enum class`, range-for 및 move 문법을 사용하지 않는다.
- namespace를 사용하지 않고 `DF_Main_`, `DF_Rod_`, `DF_Protocol_` 또는 기능 고유 접두어를 사용한다.
- 제품 Variant용 전처리 분기를 새로 만들지 않는다.
- 제거 대상 코드는 바로 삭제하지 않고 `deprecated/`에 출처와 제거 이유를 남긴다.
- 기존 driver class까지 한 번에 재작성하지 않는다. application 경계를 먼저 정착시킨 뒤 필요한 항목만 별도 작업으로 다룬다.

## 실행 순서 초안

1. Main/Rod `.inc`의 전역 변수, 객체, 함수 및 교차 의존성 목록을 만든다.
2. 각 상태의 단일 owner와 모듈별 public API를 문서에서 먼저 확정한다.
3. 의존성이 적은 기능부터 `.inc`를 독립 `.cpp/.h`로 한 모듈씩 전환한다.
4. Main 전환 후 compile/link 게이트를 수행하고, 같은 방식으로 Rod를 전환한다.
5. 구조 정착 후 callback-to-loop, 버퍼 경계 및 초기화 순서를 안전화한다.
6. protocol host test와 Main/Rod build를 통과한 뒤 사용자 수동 smoke/HIL 게이트로 이동한다.

세부 실행 순서는 `MODULE_MIGRATION_PLAN.md`의 작업 08-A~J로 확정한다.

## 다음 작업에서 먼저 만들 산출물

- Main 전역 상태/함수 의존성 표
- Rod 전역 상태/함수 의존성 표
- 모듈별 owner/public API 표
- `.inc` 제거 순서와 단계별 영향 파일 목록
- 각 단계의 compile/link 및 수동 장비 검증 게이트

## 초안 완료 조건

- 모든 `.inc`의 목표 owner가 정해져 있다.
- 공개 API와 private 구현 경계가 문서에 나타나 있다.
- 독립 `.cpp/.h` 전환 순서와 각 단계의 검증 시점이 정해져 있다.
- 미결정 사항은 추측으로 코드화하지 않고 명시적인 보류 항목으로 남아 있다.

이 조건은 소스 리팩토링 **설계 초안의 완료 조건**이다. 실제 코드 전환, 빌드 성공, 프로토콜 호환 및 장비 동작 완료를 의미하지 않는다.

## 초안 검토 결론

- 2026-08-28 작업 07-A~G 분석을 완료했다.
- 중앙 거대 state 하나를 만들지 않고 기능 owner `.cpp`의 file-scope 상태와 최소 API를 사용한다.
- Foundation과 Scheduler는 그대로 독립 모듈로 승격하지 않고 책임별로 해체한다.
- 기존 driver header의 raw `extern`은 임시 compatibility 단계를 거쳐 제거한다.
- 구조 전환과 callback 동작 변경은 같은 단계에 섞지 않는다.
- 작업 08-A에서 현재 unity 구조를 먼저 compile/link해 실제 출발점을 확정한다.
