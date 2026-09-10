# DF Project — Unity 게임 복구

조각난 Unity 낚시 게임 자료를 근거로 `DF_AP_Custom/UnityProject`를 전체 복구하는 작업공간입니다.

현재 Title, Loading, SelectScene과 PC용 Serial simulator 기반까지 복구되어 있습니다. 다음 단계는 게임 상태별 Scene 인벤토리를 확정하고, SINGLE 선택 이후 레벨 선택과 첫 3D Gameplay Stage를 복원하는 것입니다.

## 주요 경로

- `DF_AP_Custom/UnityProject`: 실제 복구 대상
- `DF_AP_Custom/ExportedProject`, `DF_AP_Custom/AuxiliaryFiles`: 읽기 전용 원본 증거
- `DF_AP_Custom/Docs`: 기존 분석 결과
- `DF_AP_1.3.5_Ext`: 추가 추출 참조
- `DF_Integration`: AP–FW 통합 프로토콜 기본 참조
- `DF_FW_Code`: 필요 시에만 확인하는 읽기 전용 펌웨어 소스

작업 전 `AGENTS.md`, `FISHING_GAME_RECOVERY_PLAN.md`, `CurrentWork.md`, 활성 `tasks/` 문서를 확인합니다.
