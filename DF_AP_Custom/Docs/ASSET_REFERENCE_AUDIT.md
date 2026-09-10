# Asset Reference Audit

생성일: 2026-07-13 16:59:00 +09:00

## 전체 결과

- 비-meta 파일: 8888
- meta 파일: 8651
- meta 누락 파일: 237
- 고유 GUID: 8651
- 중복 GUID: 0
- 조사한 YAML GUID 참조: 73209
- 해소되지 않은 고유 파일/GUID 조합: 73
- Scene/Prefab script 참조: 16048
- 해소되지 않은 Scene/Prefab script 참조: 0
- 누락 Shader 후보: 0
- 누락 Animator/Animation 후보: 0
- 누락 Audio 후보: 0
- 누락 TMP/Font 후보: 0

전체 파일/GUID 상태는 `ASSET_GUID_INVENTORY.csv`, Scene/Prefab의 script 참조는 `SCENE_PREFAB_SCRIPT_GUIDS.csv`, 해소되지 않은 상세 참조는 `UNRESOLVED_ASSET_REFERENCES.csv`에 기록했다.

## 중복 GUID

- 없음

## 해소되지 않은 Scene/Prefab script 참조

- 없음

## 해소되지 않은 YAML 참조 분포

| 참조 파일 확장자 | 고유 파일/GUID 조합 |
|---|---:|
| `.asset` | 32 |
| `.mat` | 31 |
| `.unity` | 10 |

해소되지 않은 참조에는 Unity built-in 이외의 package/Asset Store 파일, export 누락 파일, 실제 Missing Reference가 함께 포함될 수 있다. 자동 생성하거나 삭제하지 않는다.

## P0 수직 조각 선정

- Scene: 신규 `Bootstrap`, 안전한 `LoadingScene`, 첫 Gameplay 기준인 Busan
- Core: `StateManager`, `IStateinterface`, `FishGameManager`, `StageManager`, `DataManager`
- Gameplay: `MainObjectCont`, `RodCont`, `TensionCont`
- 데이터: `StageData_stage_busan` 및 직접 참조되는 map/fish/quest 설정
- 입력: 실제 Serial/Card 대신 keyboard/mouse simulator와 adapter

## Placeholder 후보

- 라이선스와 버전이 확정되지 않은 Spine 로딩 애니메이션
- 실제 Serial, IMU, Reel Motor, Card 장치 연결
- 결제와 외부 네트워크 서비스
- 원본 패키지를 확인하기 전의 누락 Shader/Material 효과
- 첫 Gameplay 수직 조각에서 아직 닫히지 않은 환경·연출 자산
