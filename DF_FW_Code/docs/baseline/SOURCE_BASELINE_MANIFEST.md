# DF Main/Rod 소스 기준선 Manifest

Updated: 2026-08-27

## 목적

리팩토링 전 Main `Vm1.0.9.0`과 Rod `Vr1.0.1.0`의 소스 위치, 파일 수, 줄 수 및 기존 빌드 산출물 SHA-256을 기록한다.

## Git 기준

- 최초 소스 기준선 commit: `a7580619aac2a9ec641e533fa2eba9463e3635e4`
- 소스 파일은 `.gitattributes`의 `*.ino -text`, `*.cpp -text`, `*.h -text` 규칙으로 줄바꿈 변환 없이 저장한다.
- 기존 `build/` 산출물은 Git에 포함하지 않고 이 문서에 크기와 SHA-256만 기록한다.

## 소스 요약

| 대상 | 원본 경로 | 파일 수 | 바이트 | 줄 수 | 진입 파일 SHA-256 |
| --- | --- | ---: | ---: | ---: | --- |
| Main | `Vm1.0.9.0/DF_Main/` | 25 | 673,965 | 27,469 | `210B4ACF94D22EC5EB5FF8805547E25775F6281C1645AC4198CB02A1E0C8C279` |
| Rod | `Vr1.0.1.0/` | 15 | 141,897 | 5,807 | `CDA76DC5863D9E5A3B72D34932E9228C1C0C59F4BD35B4945F483CBECA62BBA7` |
| 합계 | - | 40 | 815,862 | 33,276 | - |

줄 수는 각 `.ino`, `.cpp`, `.h` 파일을 원본 줄바꿈 기준으로 계산했다.

## 소스 파일 목록

Main 25개:

```text
BobbinMotor.cpp
BobbinMotor.h
Common.h
Config.h
DF_Main.h
DF_Main.ino
Diag.cpp
Diag.h
ENow.cpp
ENow.h
Eeprom.cpp
Eeprom.h
Encoder.cpp
Encoder.h
FileSys.cpp
FileSys.h
Led.cpp
Led.h
LmMotor.cpp
LmMotor.h
Potentiometer.cpp
Potentiometer.h
ServoMotor.cpp
ServoMotor.h
Version.h
```

Rod 15개:

```text
Battery.cpp
Battery.h
Common.h
DF_Rod.ino
EB_IMU.cpp
EB_IMU.h
ENow.cpp
ENow.h
Encoder.cpp
Encoder.h
Potentiometer.cpp
Potentiometer.h
SwitchButton.cpp
SwitchButton.h
Version.h
```

## 기존 빌드 산출물

### Main

기준 경로: `Vm1.0.9.0/DF_Main/build/esp32.esp32.esp32s3/`

| 파일 | 바이트 | SHA-256 |
| --- | ---: | --- |
| `boot_app0.bin` | 8,192 | `F94C5D786A7A8FAB06AC5D10E33BF37711A6697636DC037559EA19CC410A17F0` |
| `DF_Main.ino.bin` | 882,320 | `E17ACE8EDEA6E5076CD2E4E384E860772E2EA34F361723DB4E66FF2449F3C2D9` |
| `DF_Main.ino.bootloader.bin` | 15,104 | `E01A2300DE23C8D601E9F2A37684B5DBC83AE37132D5A8833E23520AFD895F92` |
| `DF_Main.ino.elf` | 15,805,332 | `397341E439B37D277607D7327A0848AD243D81B4A5B584DB5C32ABED3D8844A6` |
| `DF_Main.ino.map` | 14,108,238 | `3E44D994BF3E46ADA8CCFBF2CBB38F47598DD13ABE890FF520D9394A52D9013B` |
| `DF_Main.ino.partitions.bin` | 3,072 | `148B959CBFF1C38AA8E1D5C0BA9D612C54997B945E56A63F41223EEF650653A1` |

### Rod

기준 경로: `Vr1.0.1.0/build/esp32.esp32.esp32s3/`

| 파일 | 바이트 | SHA-256 |
| --- | ---: | --- |
| `boot_app0.bin` | 8,192 | `F94C5D786A7A8FAB06AC5D10E33BF37711A6697636DC037559EA19CC410A17F0` |
| `DF_Rod.ino.bin` | 763,568 | `589A9682C912E313B2D7F9DEF7233AFF7B5E22221A320184026298F3F67B845D` |
| `DF_Rod.ino.bootloader.bin` | 15,104 | `E01A2300DE23C8D601E9F2A37684B5DBC83AE37132D5A8833E23520AFD895F92` |
| `DF_Rod.ino.elf` | 14,017,704 | `DBEC78D06ADB1AF2587688BD316DCD2F316102956109059FA5742B5909F2EE27` |
| `DF_Rod.ino.map` | 13,040,628 | `07601B14BDB967229664F80F2A62D75D13597A09B0792891EE9D8C7CC8127B45` |
| `DF_Rod.ino.partitions.bin` | 3,072 | `148B959CBFF1C38AA8E1D5C0BA9D612C54997B945E56A63F41223EEF650653A1` |

## `legacy/` 전환 결정

- 작업 01과 02 동안 현재 원본 경로를 유지해 기존 산출물과 재현 빌드 결과를 같은 위치에서 비교한다.
- 기준 빌드가 확정된 뒤 작업 03에서 `git mv`로 Main과 Rod 원본을 `legacy/` 아래로 이동한다.
- 이동 대상은 각각 `legacy/Main/Vm1.0.9.0/`과 `legacy/Rod/Vr1.0.1.0/`으로 한다.
- 활성 펌웨어는 `firmware/DF_Main/`, `firmware/DF_Rod/`에 현재 제품 코드만 별도로 구성한다.
- 과거 Variant를 `legacy/`에 새로 수집하지 않는다. 필요하면 해당 과거 펌웨어 저장소에서 찾는다.
- 이후 활성 코드에서 제거하는 파일이나 코드 조각만 저장소 루트 `deprecated/`에 출처와 제거 이유를 붙여 보존한다.

## 검증 기록

- PowerShell `Get-ChildItem`, `System.IO.File.ReadAllLines`, `Get-FileHash -Algorithm SHA256`으로 2026-08-27에 산출했다.
- 이번 작업에서는 소스, 빌드 산출물, 폴더 위치를 변경하지 않았다.
