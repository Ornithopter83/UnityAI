# DF AP 현재 작업 상태

Updated: 2026-09-10

## 기준선

- 루트: `C:\Projects\AI-AGENTS\DF_Project`
- Unity 프로젝트: `DF_AP_Custom/UnityProject`
- Unity: `2022.3.62f3` (`96770f904ca7`)
- 활성 작업지시: `tasks/02_Scene_복구_인벤토리_확정.md`
- Git: `main`, 아직 커밋 없음. `origin`은 `https://github.com/Ornithopter83/UnityAI.git`

## 검증된 현재 구현

- Build Settings: Title, LoadingScene, SelectScene, Bootstrap 4개 Scene.
- Assets: Scene 4, C# 22, PNG 91, Material 33, Shader 6, DLL 2.
- Title: 광안대교, 수면, 선박, UI와 유휴 루프 관련 자산.
- Loading: 원본 배경, Spine 로딩 자산, 전환 코드.
- SelectScene: 주의사항 2단계와 SINGLE/BATTLE 선택 흐름.
- Serial codec/parser/simulator 코드.
- 2026-07-20 생성된 Assembly-CSharp/Editor DLL과 2026-07-16~20 검증 성공 로그.

## 검증 해석

- 이번 재분석에서는 Unity를 다시 실행하지 않았다. 실행 성공은 보존 로그와 컴파일 산출물 기준이다.
- 과거 로그 일부의 LastScene scaffold 흐름은 폐기됐다. 현재 유효 흐름은 Title → LoadingScene → SelectScene이다.
- Gameplay 11개 Scene과 최종 LastScene은 작업본에 없으므로 전체 복구 완료가 아니다.

## 목표 흐름

Title → LoadingScene → SelectScene → 레벨/맵 선택 → Gameplay Stage → LastScene → 재시작/종료

Gameplay 핵심: Casting → Wait → Bite → Hooking → Fight/Holding → Finish/Catch

## 진행률

잔여 작업 7개 (02, 03, 04, 05, 06, 07, 08)

## 정책

- P0: 근거 없는 임의 수정은 어떤 경우에도 허용하지 않는다.
- 시각 요소의 유추 복구는 원본 스크린샷·동영상 등 비교 가능한 근거가 있을 때만 진행한다.
- Scene, Material, Texture, Mesh 등은 작업 전에 원본 추출물에서 존재 여부를 검색한다.
- 원본에서 찾지 못한 항목은 자동 대체하지 않고 `DF_AP_Custom/Docs/RECOVERY_ISSUES.md`에 기록한다.
- 원본/추출 영역은 읽기 전용이며 채택 자산만 `DF_AP_Custom/UnityProject/Assets` 정상 경로에 둔다.
- AP–FW 프로토콜은 `DF_Integration/PROTOCOL.md`를 기본 통합 계약으로 사용한다.
- `DF_FW_Code`는 읽기 전용이며 Integration 자료가 불충분하거나 충돌 검증이 필요할 때만 참조한다.
- 유효한 `.meta`와 GUID를 보존하고 Missing Script를 조사 없이 제거하지 않는다.
- 근거 없는 Scene 구성과 승인 없는 Placeholder/Estimated 배치를 금지한다.

## 표준 검증

- Git: `git status --short --branch`
- Unity: `DF_AP_Custom/UnityProject`를 2022.3.62f3으로 import/compile
- Scene: GUID 정적 검사, Scene load, PlayMode, 1920×1080 GPU 캡처를 구분 기록
- 실제 장비: simulator/adapter 후 별도 승인된 환경에서만 검증

## 초기 Git 범위

- 포함: 루트 정책/작업 문서, `DF_AP_Custom`의 설정·도구·기술 문서·실제 Unity `Assets/Packages/ProjectSettings`, `DF_Integration`
- 제외: `DF_AP_1.3.5_Ext`, 읽기 전용 `DF_FW_Code`, `DF_AP_Custom/ExportedProject`, `DF_AP_Custom/AuxiliaryFiles`, Unity/IDE 생성물과 로그
- Git LFS: `*.webm`과 80MiB Title Mesh `Object427_0.asset`
- 상태: 573개 파일 스테이징 완료. 제외 대상이 스테이징되지 않았고 두 대용량 자산은 LFS 포인터로 확인했다. 커밋과 푸시는 아직 수행하지 않았다.

## 다음 작업

02-A: 작업본 4개와 원본 12개 미복구 Scene의 구조·시각 근거, 원본 자산 존재 여부, 누락 문제와 복구 수준 표 확정
