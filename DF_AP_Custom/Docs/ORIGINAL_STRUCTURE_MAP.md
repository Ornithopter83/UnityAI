# Original Structure Map

조사일: 2026-07-13
대상 task: `04A-1`
현재 수준: R1 구조 확인

## 결론

복구 산출물은 원래 Unity 폴더 구조를 그대로 보존하지 않고 에셋 타입 중심으로 평탄화되어 있다. 현재 증거로 볼 때 실행 구조는 다음 네 영역으로 나뉜다.

```text
Entry/System Scene
  └─ Title.unity
     ├─ StateManager / FishGameManager / DataManager
     ├─ UI / Sound / Tutorial
     ├─ Serial / Motor / Card
     └─ Client / Server / TCP / UDP

Transition Scene
  └─ LoadingScene.unity
     └─ LoadingSceneManager

Gameplay Map Scenes (11개, 원래 AssetReference GUID 이름)
  ├─ StageManager
  ├─ MainObjectCont
  ├─ RodCont
  └─ TensionCont

Result Scene
  └─ LastScene.unity
     ├─ ResultCont
     ├─ ResultUi
     └─ FishCardListUi
```

추가 Scene `776bde1206771494880c13770e84312e.unity`는 `LoadScene == 2`일 때 로드되는 `_assets[0]`이며 `characterIcon`, `player_*`, `LV_bg` 등의 GameObject 이름을 가진다. 캐릭터 또는 플레이어 선택 Scene으로 추정하며 역할 표시는 `Estimated`다.

## Gameplay Scene 이름 복원

`AdsManager`가 `AssetReference[] _assets`를 보존하고 있고, 배열 값은 export된 Scene 파일의 basename과 일치한다. `LoadingSceneManager`는 `SceneLoadTest(loadScene - 2)`를 호출한다. 이를 11개 `StageData._scene_number`와 결합해 다음 매핑을 확인했다.

| LoadScene | Asset index | Map | Scene 파일 | 근거 |
|---:|---:|---|---|---|
| 2 | 0 | 선택 Scene (`Estimated`) | `776bde1206771494880c13770e84312e.unity` | UI object 이름 |
| 3 | 1 | Busan | `e661fe7e48f559a40970741e68ca9fc8.unity` | `StageData_stage_busan` |
| 4 | 2 | Incheon | `926600b55d7004b4ba4420d4ebc2aba6.unity` | `StageData_stage_Incheon` |
| 5 | 3 | Pohang | `4a90dea7b55b6844dab8e1f52ac402e1.unity` | `StageData_stage_Pohang` |
| 6 | 4 | Osaka | `9c14a515c7cd3a341ac3b298d80a115d.unity` | `StageData_stage_osaka` |
| 7 | 5 | Sydney | `49199a014f95e754680a7bc67316c4e8.unity` | `StageData_stage_australia` |
| 8 | 6 | Mekong | `9336adcd49129d54a8b218f89c9ddfd8.unity` | `StageData_stage_mekong` |
| 9 | 7 | Amazon | `54544c6a5cc50d74b9cb7928f02a336c.unity` | `StageData_stage_amazon` |
| 10 | 8 | Baikal | `7a6d424450b49d44385a1562124f93da.unity` | `StageData_stage_mockba`, enum `BAIKAL` |
| 11 | 9 | Antarctica | `1860b791fe332d5469ef6bfe7a064ecd.unity` | `StageData_stage_antarctica` |
| 12 | 10 | USA | `41cba86cf8bae97448ca454ab3c6ec46.unity` | `StageData_stage_usa` |
| 13 | 11 | Mystic | `427872b136b426d43abbb4b07f3dc390.unity` | `StageData_stage_mystic` |

`StageData_stage_mystic.stageNumber`는 98이며 `LastStage` 조건과 일치한다. `LastScene`은 `LoadScene == 99`에서 build index 2로 로드되는 결과 Scene이다.

## DLL과 복구 C# 대응

`Assembly-CSharp.dll`은 정상적으로 metadata reflection이 가능했고 794개 타입이 확인됐다. 아래 8개 타입은 모두 DLL과 C# 양쪽에 존재한다.

비교 방법:

- DLL: declared field/property/method metadata 확인
- C#: compiler-generated member를 제외한 일반 member 이름 존재 여부 확인
- 결과: 아래 8개 타입의 일반 field/property/method 이름이 전부 C#에서 확인됨
- 제한: 메서드 본문이 의미상 동일한지는 아직 검증하지 않았으며 C# provenance도 확정하지 않음

| 타입 | Base | Field | Property | Method name | Nested type |
|---|---|---:|---:|---:|---|
| `StateManager` | `MonoBehaviour` | 11/11 | 5/5 | 13/13 | `eState` |
| `IStateinterface` | interface | 0/0 | 0/0 | 8/8 | - |
| `FishGameManager` | `MonoBehaviour` | 28/28 | 19/19 | 29/29 | 모드/난이도/결제 enum 9개 |
| `StageManager` | `MonoBehaviour` | 16/16 | 12/12 | 14/14 | coroutine/compiler types |
| `DataManager` | `MonoBehaviour` | 35/35 | 21/21 | 64/64 | 데이터 enum/struct 및 compiler types |
| `RodCont` | `MonoBehaviour` | 40/40 | 4/4 | 39/39 | `eRodState`, coroutine types |
| `TensionCont` | `MonoBehaviour` | 20/20 | 0/0 | 12/12 | compiler type |
| `MainObjectCont` | `MonoBehaviour` | 35/35 | 10/10 | 43/43 | `eStageState`, coroutine types |

DLL metadata의 compiler-generated backing field와 special accessor는 위 일반 member 수에서 제외했다.

### 원본 상태 인터페이스

DLL과 C# 양쪽에서 아래 8개 이름이 일치한다.

```csharp
void start();
void update();
void end(bool continueState);
void LeftButtonDown();
void RightButtonDown();
void LeftButtonUp();
void RightButtonUp();
void ContinueEnd();
```

가이드의 일반화된 `Enter/Tick/Exit` 형태와 실제 원본 signature가 다르므로, 초기 복구에서는 원본 이름을 우선 보존해야 한다.

## 핵심 Script GUID와 Scene 참조

| 타입 | GUID | Scene 참조 | 판정 |
|---|---|---:|---|
| `StateManager` | `75c45a7cc424485921b11e230ff0e443` | `Title` 1개 | Entry/Core |
| `IStateinterface` | `.meta` 없음 | 0 | Unity component가 아닌 interface |
| `FishGameManager` | `d13a81579d0dcd3650bc4a5e506f3fc7` | `Title` 1개 | Entry/Core |
| `DataManager` | `82b0d7366d0e5a1845f23c51e32969a5` | `Title` 1개 | Entry/Data |
| `StageManager` | `91602c5410a67630016d902ed83bf964` | 해시 Scene 11개 | Gameplay 공통 |
| `RodCont` | `68905d76b2ec6591a45743687e6411b8` | 해시 Scene 11개 | Gameplay 공통 |
| `TensionCont` | `18fb9657248b2d79a22a94508eec0ec7` | 해시 Scene 11개 | Gameplay 공통 |
| `MainObjectCont` | `7417e63bed1fe0ced9135f3098d51eb7` | 해시 Scene 11개 | Gameplay 공통 |

이 8개 GUID는 Prefab YAML에서는 직접 발견되지 않았다. 이는 manager가 Scene에 직접 배치되었거나 Prefab이 다른 복구 형태로 분리되었음을 뜻한다.

## Metadata-visible 의존성 분류

아래는 field/property/method signature 및 `using`에서 보이는 직접 의존이다. 메서드 본문 내부의 singleton/static 호출은 별도 호출 그래프 조사 대상이다.

| 타입 | 게임 코드 | 외부 의존 | Unity 영역 | 하드웨어/네트워크 |
|---|---|---|---|---|
| `StateManager` | `IStateinterface`, `eState` | - | `MonoBehaviour` | - |
| `FishGameManager` | 자체 모드 enum | - | SceneManagement, Material, LogType | 본문 조사 필요 |
| `StageManager` | `MainObjectCont`, `StageData`, `UiManager`, `SoundManager`, `DataManager` | - | GameObject, coroutine | - |
| `DataManager` | Fish/Stage/Rank/Character/Level 데이터, `ServerManager.eMatchMode` | - | Sprite | Server 결합 있음 |
| `RodCont` | `CatchCont` | Crest, DOTween, Filo, MK Glow | Animator, Rigidbody, Camera, Particle, Volume | - |
| `TensionCont` | - | ParticleImage, DOTween, TMP, URP | UI Image, Canvas, Camera, Volume | - |
| `MainObjectCont` | `RodCont`, `CastPoint*`, `BonusCasting`, `JumpFish` | DOTween, URP | Camera, Material, Transform, Volume | - |

## 이름 있는 Scene의 실제 복잡도

| Scene | 사용자 C# GUID 수 | 핵심 내용 | 복구 난이도 |
|---|---:|---|---|
| `LoadingScene` | 1 | `LoadingSceneManager`; Sound/Serial/Core/Addressables 직접 결합 | Scene 구조 낮음, 런타임 결합 높음 |
| `LastScene` | 3 | 결과 UI와 물고기 카드 | 중간 |
| `Title` | 41 | Core/UI/Tutorial/Audio와 Serial/Card/Network | 매우 높음 |

`Title`에는 핵심 3개 manager뿐 아니라 `SerialManager`, `MotorDebug`, `CardManager`, `ClientManager`, `ServerManager`, `TcpManager`, `UdpManager` 등이 직접 배치되어 있다. 따라서 원본 흐름상 첫 Scene이지만 기술적으로는 첫 복구 Scene에 적합하지 않다.

## Scene 복구 순서 제안

구조 복원과 사용자 요구를 함께 만족시키기 위해 다음 순서가 안전하다.

1. `Bootstrap/Test` — 새로 만든 `Reconstructed` 검증 Scene; 하드웨어/Addressables 미사용
2. `LoadingScene` — 원본 Scene 구조는 작지만 adapter로 Sound/Serial/Core 의존을 차단한 뒤 복구
3. `LastScene` — 결과 UI 3개 script를 중심으로 복구
4. `Title` shell — 하드웨어/네트워크를 비활성 adapter로 격리한 뒤 복구
5. 해시 Gameplay Scene 1개 — 수직 조각의 기준 map 선정
6. 나머지 Gameplay/선택 Scene — 공통 의존성이 안정된 뒤 반복 이관

게임의 실제 실행 순서는 최종적으로 `Title → Loading → Gameplay → Last`를 유지하되, 복구 작업 순서는 의존성이 작은 Scene부터 진행한다.

## 다음 작업

실제 복구는 `tasks/05_sceneRecovery.md` 하나에서 Scene별로 관리한다. package/DLL 경계, 전이 asset closure, 복사 manifest, Unity 검증은 각 Scene 행의 완료 조건으로 포함한다.

## 불확실성

- 해시 Gameplay Scene 11개의 실제 map 이름
- C#이 원본 소스인지 DLL 역추출 결과인지
- Title에 배치된 하드웨어/network component가 비연결 상태에서 안전한지
- Addressables catalog로 해시 Scene의 원래 address를 복원할 수 있는지
