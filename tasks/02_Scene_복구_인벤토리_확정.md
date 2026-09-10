# Scene 복구 인벤토리 확정

## 목표

현재 및 미복구 Scene 전체의 근거, 역할, 의존성, 복구 수준과 우선순위를 확정한다.

## 현재 기준선

- 작업본: Title, LoadingScene, SelectScene, Bootstrap
- 미복구: LastScene, Gameplay 11개
- 기존 실행 성공은 과거 로그 기준이며 현재 환경 재실행은 아직 하지 않았다.

## 상세 작업 단위

### A. Scene별 근거와 복구 수준 표 작성

- 작업본 4개와 원본 12개 Scene을 provenance 기준으로 분류한다.
- Scene별 원본 스크린샷·동영상 존재 여부와 비교 가능한 화면 상태를 기록한다.
- 해시 Scene/지역 매핑을 재검증한다.

### B. Scene별 의존 자산 closure 작성

- Script, Prefab, Mesh, Material, Texture, Shader, Animator, Audio, Data, package를 원본 추출물에서 검색한다.
- 존재/누락/식별 불가를 구분하고 공통/Scene 고유 의존성을 분리한다.
- 누락과 충돌은 `DF_AP_Custom/Docs/RECOVERY_ISSUES.md`에 기록한다.

### C. 상태 전환과 로드 경계 확정

- Title부터 LastScene까지 진입·종료 조건과 Build Settings/Addressables 경계를 기록한다.

### D. 첫 구현 단위 승인 기준 확정

- 레벨 선택과 Busan 중 선행 범위 및 누락 근거를 확정한다.

## 진행률

잔여 작업 4개 (A, B, C, D)

## 변경 금지

- 이 작업에서 Scene을 생성·수정하거나 누락 의존성을 임의 생성·교체하지 않는다.
- 스크린샷·동영상 등 시각 근거가 없는 요소를 유추해 배치하지 않는다.

## 인수 기준과 테스트

- 모든 Scene의 역할, 구조/시각 근거, 원본 자산 존재 여부, 복구 상태, 선행 조건을 한 표에서 추적한다.
- Build Settings, 원본 YAML, Addressables, GUID, 자산 인벤토리를 정적 대조한다.

## 결과

- 아직 작업 전
