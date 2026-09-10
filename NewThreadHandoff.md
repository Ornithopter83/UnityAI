# DF Project 새 스레드 인계

Updated: 2026-09-10

## 위치와 읽기 순서

- 루트: `C:\Projects\AI-AGENTS\DF_Project`
- Unity에서 열 대상: `DF_AP_Custom/UnityProject`
- 먼저 `AGENTS.md` → `FISHING_GAME_RECOVERY_PLAN.md` → `CurrentWork.md` → `tasks/02_Scene_복구_인벤토리_확정.md`를 읽는다.

## 현재 상태

- Title, LoadingScene, SelectScene, Bootstrap이 작업본과 Build Settings에 있다.
- 과거 로그상 Title/Loading/SelectScene/Serial 검증이 통과했다.
- Gameplay 11개와 LastScene은 아직 작업본에 없다.
- 다음은 전체 Scene 인벤토리, SINGLE 이후 선택 흐름, Busan 수직 조각 순이다.

## 경계

- 읽기 전용: `DF_AP_Custom/ExportedProject`, `DF_AP_Custom/AuxiliaryFiles`, `DF_AP_1.3.5_Ext`, `DF_FW_Code`
- 통합 프로토콜 기본 참조: `DF_Integration/PROTOCOL.md`
- 실제 변경: `DF_AP_Custom/UnityProject`
- 보존 기술 문서: `DF_AP_Custom/Docs`
- 원본 근거 없는 Scene/3D/UI/동작을 만들지 않는다.
- 실제 COM, 결제, 모터, USB/OS 동작과 빌드·커밋·푸시는 별도 승인 없이는 수행하지 않는다.

## 대기 항목

- 미복구 Scene 의존 자산 closure
- SINGLE 이후 레벨/맵 선택
- Busan 3D Gameplay 수직 조각
- Enviro 3, Crest, SWS 대체 구현의 시각 차이
- 실제 장비 검증
