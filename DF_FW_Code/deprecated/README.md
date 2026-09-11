# Deprecated source index

이 폴더는 활성 펌웨어에서 제거한 구 코드와 이전 빌드 보조 파일을 보존한다. 여기의 파일은 include, compile, test 또는 현대화하지 않는다.

| 경로 | 원본 | 제거일 | 제거 이유 | 활성 대체 경로 |
| --- | --- | --- | --- | --- |
| `main/DF_Main_PreRefactor.ino.txt` | `Vm1.0.9.0/DF_Main/DF_Main.ino` | 2026-08-27 | Main `.ino` 진입점 축소 및 기능 경계 분리 | `firmware/DF_Main/DF_Main.ino`, `firmware/DF_Main/src/` |
| `main/Config_PreCurrentOnly.h.txt` | `Vm1.0.9.0/DF_Main/Config.h` | 2026-08-27 | 과거 Variant 및 비활성 설정 격리 | `firmware/DF_Main/src/Config.h` |
| `main/Common_PreCurrentOnly.h.txt` | `Vm1.0.9.0/DF_Main/Common.h` | 2026-08-27 | 과거/비활성 조건과 중복 프로토콜 정의 격리 | `firmware/DF_Main/src/Common.h`, `libraries/DFProtocol/` |
| `main/Version_History.h.txt` | `Vm1.0.9.0/DF_Main/Version.h` | 2026-08-27 | 과거 Main 버전 선택과 이력 격리 | `firmware/DF_Main/src/Version.h`, `Version.cpp` |
| `rod/DF_Rod_PreRefactor.ino.txt` | `Vr1.0.1.0/DF_Rod.ino` | 2026-08-27 | Rod `.ino` 진입점 축소 및 기능 경계 분리 | `firmware/DF_Rod/DF_Rod.ino`, `firmware/DF_Rod/src/` |
| `rod/Common_PreCurrentOnly.h.txt` | `Vr1.0.1.0/Common.h` | 2026-08-27 | 과거 Rod 보드/입력/배터리 Variant 격리 | `firmware/DF_Rod/src/Common.h`, `libraries/DFProtocol/` |
| `rod/Version_History.h.txt` | `Vr1.0.1.0/Version.h` | 2026-08-27 | 과거 Rod 버전 선택과 이력 격리 | `firmware/DF_Rod/src/Version.h`, `Version.cpp` |
| `tools/stage-rod.cmd` | `tools/stage-rod.cmd` | 2026-08-27 | 활성 Rod sketch 경로가 `firmware/DF_Rod`로 정규화되어 staging 불필요 | `tools/build-rod.cmd` 직접 빌드 |

| `rod/handle-led-test-Vr1.0.1.2/` | Rod Application·Encoder·Communication 5개 파일 | 2026-08-31 | 사용자 요청으로 HANDLE 시험 LED 표식 제거; 원본 파일은 `.txt`로 보존 | `firmware/DF_Rod/src/`의 `Vr1.0.1.3` |

| `tools/firmware-versions-static.cmd.txt` | `tools/firmware-versions.cmd` | 2026-08-31 | 소스 버전과 별도 고정값의 불일치 제거 | `tools/firmware-versions.cmd`, `tools/firmware-versions.ps1` |

| `main/bldc-before-restore-20260831/` | Main `BobbinMotor.cpp`, `Common.h` | 2026-08-31 | PWM 반전 누락 및 구형 모터 상수 복구 전 코드 보존 | `firmware/DF_Main/src/BobbinMotor.cpp`, `Common.h` |

| `main/bldc-before-recompare-20260831/` | Main BobbinMotor.cpp/.h, DF_Main_LmWireControl.cpp | 2026-08-31 | 비활성 시험 배율 제거 및 PWM/명령 단위·UART 로그 보완 전 보존 | 동일 활성 소스 경로 |

전체 변경 전 기준 원본은 `legacy/Vm1.0.9.0`, `legacy/Vr1.0.1.0`에도 변경 금지 상태로 보존한다.
