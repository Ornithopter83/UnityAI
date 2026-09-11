# Active firmware source layout

Updated: 2026-08-28

## Source ownership

```text
firmware/
├─ DF_Main/
│  ├─ DF_Main.ino                 # setup/loop delegation only
│  └─ src/
│     ├─ DF_Main_Application.cpp/.h  # application setup/loop owner
│     ├─ DF_Main_State.cpp/.h        # shared state and low-level compatibility owner
│     ├─ DF_Main_Internal.h           # temporary internal declaration boundary
│     ├─ modules/
│     │  ├─ DF_Main_Communication.cpp/.h
│     │  ├─ DF_Main_LedControl.cpp/.h
│     │  ├─ DF_Main_CommandRouter.cpp/.h
│     │  ├─ DF_Main_DeviceCheck.cpp/.h
│     │  ├─ DF_Main_LmWireControl.cpp/.h
│     │  ├─ DF_Main_GameControl.cpp/.h
│     │  ├─ DF_Main_Diagnostics.cpp/.h
│     │  └─ DF_Main_Scheduler.cpp/.h
│     └─ existing device drivers
└─ DF_Rod/
   ├─ DF_Rod.ino                  # setup/loop delegation only
   └─ src/
      ├─ DF_Rod_Application.cpp/.h  # application setup/loop owner
      ├─ DF_Rod_State.cpp/.h        # shared state and low-level compatibility owner
      ├─ DF_Rod_Internal.h           # temporary internal declaration boundary
      ├─ modules/
      │  ├─ DF_Rod_Communication.cpp/.h
      │  ├─ DF_Rod_SetupProtocol.cpp/.h
      │  ├─ DF_Rod_BatteryMonitor.cpp/.h
      │  ├─ DF_Rod_PowerAndImu.cpp/.h
      │  ├─ DF_Rod_Registration.cpp/.h
      │  └─ DF_Rod_SchedulerAndInput.cpp/.h
      └─ existing device drivers
```

2026-08-28 작업 08-A~G에서 모든 활성 `.inc`와 application의 구현 include를 제거했다. 위 module `.cpp`는 Arduino CLI와 VS2022 프로젝트에서 각각 독립 번역 단위로 취급되며 대응 `.h`가 Solution Explorer에 등록되어 있다.

첫 전환에서 기존 동작과 저장 공간을 유지하기 위해 `DF_Main_Internal.h`, `DF_Rod_Internal.h`가 공통 type/macro/prototype/`extern` compatibility 경계를 제공한다. 정상 compile/link는 확인했지만 이 header는 최종 public API가 아니다. 다음 owner 정리 단계에서 모듈 내부 상태를 file-scope로 제한하고 module header의 실제 공개 API만 남긴다.

검증 결과:

- `tools/build-all.cmd Release x64`: 성공
- VS2022 `DF_Firmware.sln` `Rebuild|Release|x64`: 성공
- Main application: 882,281 bytes, 배포 `.bin` 882,640 bytes
- Rod application: 761,721 bytes, 배포 `.bin` 762,080 bytes
- 대상별 `bin/release/x64/<version>/` flash 파일 4개 확인
- flash 및 장비 smoke/HIL: 미수행

## Preservation

- 전체 변경 전 원본: `legacy/Vm1.0.9.0`, `legacy/Vr1.0.1.0`
- 활성 코드에서 제거한 선택 분기와 이전 진입 파일: `deprecated/main`, `deprecated/rod`
- 활성 펌웨어는 `legacy/` 또는 `deprecated/`를 include하거나 빌드하지 않는다.
