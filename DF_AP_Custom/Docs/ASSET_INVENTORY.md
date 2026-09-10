# Asset Inventory

조사일: 2026-07-13

## 초기 정적 집계

대상은 `ExportedProject/Assets`이며 파일을 이동하거나 수정하지 않았다.

| 유형 | 수량 |
|---|---:|
| 전체 파일 | 17,539 |
| `.meta` | 8,651 |
| `.asset` | 3,289 |
| `.png` | 2,639 |
| `.mat` | 1,121 |
| `.cs` | 362 |
| `.prefab` | 316 |
| `.shader` | 272 |
| `.anim` | 230 |
| `.controller` | 194 |
| `.bundle` | 185 |
| `.ogg` | 97 |
| `.dll` | 47 |
| `.unity` | 15 |

총 파일 크기는 약 16,750,275,480 bytes다.

## Meta 상태

- `.meta`가 없는 비-meta 파일: 237개
- 대응 원본이 없는 orphan `.meta`: 0개
- 위 수치는 폴더 `.meta`가 아니라 파일 짝만 비교한 초기 결과다.
- 일부 누락은 역추출 도구가 생성한 소스/보조 파일일 수 있어 자동 생성하거나 삭제하지 않는다.

## Scene

Build Settings에 활성화된 Scene:

1. `Assets/Scenes/Title.unity`
2. `Assets/Scenes/LoadingScene.unity`
3. `Assets/Scenes/LastScene.unity`

해시형 이름의 Scene 12개가 추가로 존재한다. 용도와 참조는 미확인이다.

## 우선 조사 대상

| ID | 경로/유형 | 상태 | 예상 용도 | 우선순위 |
|---|---|---|---|---|
| SCN-001 | `Assets/Scenes/Title.unity` | Recovered 후보 | 타이틀/진입 | P0 |
| SCN-002 | `Assets/Scenes/LoadingScene.unity` | Recovered 후보 | 로딩 | P0 |
| SCN-003 | `Assets/Scenes/LastScene.unity` | Recovered 후보 | 메인 게임/종료 미확정 | P0 |
| SRC-001 | `Assets/Scripts/StateManager.cs` | Recovered 후보 | 상태 전환 | P0 |
| SRC-002 | `Assets/Scripts/RodCont.cs` | Recovered 후보 | 낚싯대/캐스팅 | P0 |
| SRC-003 | `Assets/Scripts/TensionCont.cs` | Recovered 후보 | Fight/Holding | P0 |
| DAT-001 | `Assets/Resources`, `StreamingAssets` | Unknown | 데이터/설정 | P0 |
| PLG-001 | `Assets/Plugins` | Unknown | 네이티브/서드파티 | P0 |

## R1 참조 감사 결과

- 고유 GUID 8,651개, 중복 GUID 0개
- YAML GUID 참조 73,209건 조사
- Scene/Prefab script 참조 16,048건, 미해결 script 참조 0건
- 해소되지 않은 고유 파일/GUID/property 조합 73건
- 누락 Shader, Animator/Animation, Audio, TMP/Font 속성 후보 0건

상세 결과는 `ASSET_REFERENCE_AUDIT.md`, 전체 inventory는 `ASSET_GUID_INVENTORY.csv`, Scene/Prefab script 참조는 `SCENE_PREFAB_SCRIPT_GUIDS.csv`를 참조한다.

## Task 08 선택 Scene 채택 자산

| 용도 | 원본 | 채택 경로 | 상태 |
|---|---|---|---|
| 주의사항 1 | `Sprite/precautionsBg3.asset`, `Texture2D/precautionsBg3.png` | `Assets/Art/Selection/Warning/` | Original, GUID 보존 |
| 주의사항 2 | `Sprite/rod_controller.asset`, `Texture2D/rod_controller.png` | `Assets/Art/Selection/Warning/` | Original, GUID 보존 |
| 모드 제목 상자 | `Sprite/ui_diffiBox.asset`, `Texture2D/ui_diffiBox.png` | `Assets/Art/Selection/Mode/` | Original, GUID 보존 |
| SINGLE 카드 | `Sprite/singelMod.asset`, `Texture2D/singelMod.png` | `Assets/Art/Selection/Mode/` | Original, GUID 보존 |
| BATTLE 카드 | `Sprite/battleMod.asset`, `Texture2D/battleMod.png` | `Assets/Art/Selection/Mode/` | Original, GUID 보존 |
| 선택 테두리 | `border_0.asset`, `border_0.png` | `Assets/Art/Selection/Mode/` | Original, GUID 보존 |

- 배경 `bg_gameDiffi _2`, 입력 아이콘 `btn_l`, `btn_r`, 폰트 `SCDream5`는 앞선 작업에서 채택한 동일 원본 자산을 재사용했다.
- 원본 증거 Scene은 `ExportedProject/Assets/776bde1206771494880c13770e84312e.unity`이며 수정하지 않았다.
