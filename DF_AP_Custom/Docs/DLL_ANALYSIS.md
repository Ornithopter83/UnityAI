# DLL Analysis

## 범위와 증거

- 분석 대상: `AuxiliaryFiles/GameAssemblies/Assembly-CSharp.dll`
- 함께 확인한 자료: `ExportedProject/Assets/Scripts/*.cs`
- 현재 분석 수준: 파일/타입 존재와 일부 선언부를 확인한 R1
- 메서드 IL, 상수, 호출 그래프의 상세 분석은 아직 수행하지 않았다.

## 확인된 구조

`StateManager`는 `MonoBehaviour`이며 0~55 범위의 `eState` enum, 현재/이전 상태, `Dictionary<eState, IStateinterface>`를 가진다. 확인된 enum에는 메뉴, 캐스팅, 입질, 후킹, Fight/Hold, 결과, 튜토리얼 상태가 포함된다.

주요 관리자와 확인된 책임:

| 타입 | 확인 내용 | 상태 |
|---|---|---|
| `StateManager` | 상태 enum 및 상태 객체 사전 | Recovered 후보 |
| `FishGameManager` | 게임/플레이/결제/난이도 모드 enum, Scene 관련 의존 | Recovered 후보 |
| `StageManager` | StageData, 점수, 클리어 시간, 퀘스트 진행 | Recovered 후보 |
| `DataManager` | 맵, 아이템, 퀘스트, 난이도 설정 enum | Recovered 후보 |
| `RodCont` | 낚싯대 상태, 찌/바늘 Rigidbody, Cable, 카메라/연출 참조 | Recovered 후보 |
| `TensionCont` | 장력 UI, 체력/경고, Reel SFX, URP Volume 참조 | Recovered 후보 |

## 주의점

- C# 파일은 역추출 산출물일 수 있으므로 `Original`로 분류하지 않는다.
- Crest, DOTween, Filo, MK Glow, TextMeshPro, ParticleImage, URP 등 서드파티/패키지 의존이 보인다.
- 원본 직렬화 필드 복구를 위해 클래스명, namespace, 필드명/타입을 우선 보존해야 한다.
- DLL을 새 프로젝트의 주 실행 코드로 넣지 않고, 가능한 동작을 C#로 재구성한다.

## 다음 분석 항목

1. DLL과 C# 타입 목록 및 signature 대조
2. StateManager 상태 등록과 전환 메서드 호출 그래프
3. `State_Casting` → `Wait` → `Bite` → `Hooking` → `Fight/Holding` → `Finish/Catch` 조건
4. DataManager 상수/배열/파일 접근 및 저장 형식
5. Serial/Card 계층의 패킷 구조와 메인 스레드 결합 지점
