# DF AP 전체 복구 로드맵

Updated: 2026-09-10

## 최종 목표

`DF_AP_Custom/UnityProject`를 3D Object, UI, 상태 전환, 게임플레이, 결과 흐름을 포함한 최종 복구 완료 Unity 프로젝트로 만든다. 복구는 Scene 단위로 진행하며 확인 가능한 원본 근거만 채택한다.

## 최우선 정책

- 근거 없는 임의 수정은 엄격히 금지한다.
- 시각 복구에는 원본 스크린샷·동영상 등 비교 가능한 근거가 필요하다.
- Scene/Material/Texture/3D Object 등은 먼저 원본 추출물에서 존재 여부를 확인한다.
- 찾을 수 없는 항목은 대체 제작하지 않고 `DF_AP_Custom/Docs/RECOVERY_ISSUES.md`에 기록한다.
- 근거가 확보된 범위 안에서만 최대한 원본과 유사하게 복구한다.

## 기준 영역

- 작업본: `DF_AP_Custom/UnityProject`
- 1차 증거: `DF_AP_Custom/ExportedProject`, `DF_AP_Custom/AuxiliaryFiles`
- 추가 추출 참조: `DF_AP_1.3.5_Ext`
- 통합 프로토콜 기준: `DF_Integration`
- 필요 시 소스 확인: 읽기 전용 `DF_FW_Code`
- 보존 분석 결과: `DF_AP_Custom/Docs`

## 작업지시

| No. | 작업지시 | 목적 | 상태 |
| --- | --- | --- | --- |
| 01 | [기존 복구 상태 통합](tasks/01_기존_복구상태_통합.md) | 내부 기록과 실제 작업본을 루트 정책으로 통합 | 완료 |
| 02 | [Scene 복구 인벤토리 확정](tasks/02_Scene_복구_인벤토리_확정.md) | 모든 Scene과 의존 자산의 복구 순서 확정 | 진행 대기 |
| 03 | [레벨 선택과 게임 진입 복구](tasks/03_레벨선택과_게임진입_복구.md) | SINGLE 이후 Stage 선택·로드 흐름 복원 | 대기 |
| 04 | [Busan Gameplay 수직 조각](tasks/04_Busan_Gameplay_수직조각_복구.md) | 첫 3D Stage와 핵심 낚시 루프 복원 | 대기 |
| 05 | [공통 Gameplay 시스템](tasks/05_공통_Gameplay_시스템_복구.md) | 3D·상태·물고기·낚싯대·UI 공통 기반 | 대기 |
| 06 | [나머지 Stage Scene](tasks/06_나머지_Stage_Scene_복구.md) | 나머지 10개 지역과 Mystic 복원 | 대기 |
| 07 | [LastScene과 전체 흐름](tasks/07_LastScene과_전체흐름_복구.md) | 결과·재시작을 포함한 전체 루프 | 대기 |
| 08 | [하드웨어 통합과 최종 검증](tasks/08_하드웨어_통합과_최종검증.md) | 펌웨어/장치, 빌드, 회귀 검증 | 대기 |

## 순서와 Scene 기준

01 → 02 → 03 → 04 → 05 → 06 → 07 → 08

- 작업본 존재: Title, LoadingScene, SelectScene, Bootstrap
- 미복구: LastScene과 Busan, Incheon, Pohang, Osaka, Sydney, Mekong, Amazon, Baikal, Antarctica, USA, Mystic

한 번에 하나의 번호와 알파벳 단위를 진행한다. 모든 Scene 작업은 근거표와 원본 추출물 검색을 먼저 완료한다. “Unity에서 열림”과 “원본 화면·동작 복구”를 구분하고 실제 근거 없이 완료로 표시하지 않는다.

## 현재 상태

현재 작업: 02 Scene 복구 인벤토리 확정

잔여 작업 7개 (02, 03, 04, 05, 06, 07, 08)
