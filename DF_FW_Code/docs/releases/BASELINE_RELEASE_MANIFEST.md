# DF Main/Rod 기존 Release 기준 Manifest

Updated: 2026-08-27

## 기준 Release

| 대상 | 펌웨어 버전 | 기존 산출물 경로 | 파일 수정 시각 참고값 |
| --- | --- | --- | --- |
| Main | `Vm1.0.9.0` | `Vm1.0.9.0/DF_Main/build/esp32.esp32.esp32s3/` | 2026-05-17 16:14:40 |
| Rod | `Vr1.0.1.0` | `Vr1.0.1.0/build/esp32.esp32.esp32s3/` | 2026-01-05 17:55:21 |

수정 시각은 현재 파일시스템 참고값이며 공식 release date로 간주하지 않는다. 펌웨어 버전은 각 `Version.h`의 활성 문자열을 기준으로 한다.

## Firmware 크기와 SHA-256

| 대상 | 파일 | 바이트 | App partition 점유율 | SHA-256 |
| --- | --- | ---: | ---: | --- |
| Main | `DF_Main.ino.bin` | 882,320 | 67.32% | `E17ACE8EDEA6E5076CD2E4E384E860772E2EA34F361723DB4E66FF2449F3C2D9` |
| Rod | `DF_Rod.ino.bin` | 763,568 | 58.26% | `589A9682C912E313B2D7F9DEF7233AFF7B5E22221A320184026298F3F67B845D` |

점유율은 app0 크기 1,310,720 bytes(1280 KiB)를 기준으로 계산했다.

## Flash 구성 파일

Main:

| Offset | 파일 | 바이트 | SHA-256 |
| ---: | --- | ---: | --- |
| `0x0000` | `DF_Main.ino.bootloader.bin` | 15,104 | `E01A2300DE23C8D601E9F2A37684B5DBC83AE37132D5A8833E23520AFD895F92` |
| `0x8000` | `DF_Main.ino.partitions.bin` | 3,072 | `148B959CBFF1C38AA8E1D5C0BA9D612C54997B945E56A63F41223EEF650653A1` |
| `0xE000` | `boot_app0.bin` | 8,192 | `F94C5D786A7A8FAB06AC5D10E33BF37711A6697636DC037559EA19CC410A17F0` |
| `0x10000` | `DF_Main.ino.bin` | 882,320 | `E17ACE8EDEA6E5076CD2E4E384E860772E2EA34F361723DB4E66FF2449F3C2D9` |

Rod:

| Offset | 파일 | 바이트 | SHA-256 |
| ---: | --- | ---: | --- |
| `0x0000` | `DF_Rod.ino.bootloader.bin` | 15,104 | `E01A2300DE23C8D601E9F2A37684B5DBC83AE37132D5A8833E23520AFD895F92` |
| `0x8000` | `DF_Rod.ino.partitions.bin` | 3,072 | `148B959CBFF1C38AA8E1D5C0BA9D612C54997B945E56A63F41223EEF650653A1` |
| `0xE000` | `boot_app0.bin` | 8,192 | `F94C5D786A7A8FAB06AC5D10E33BF37711A6697636DC037559EA19CC410A17F0` |
| `0x10000` | `DF_Rod.ino.bin` | 763,568 | `589A9682C912E313B2D7F9DEF7233AFF7B5E22221A320184026298F3F67B845D` |

Main/Rod의 bootloader, partition 및 `boot_app0.bin`은 동일하다.

## Partition table

| Name | Type/Subtype | Offset | Size |
| --- | --- | ---: | ---: |
| `nvs` | data/nvs | `0x009000` | 20 KiB |
| `otadata` | data/ota | `0x00E000` | 8 KiB |
| `app0` | app/ota_0 | `0x010000` | 1280 KiB |
| `app1` | app/ota_1 | `0x150000` | 1280 KiB |
| `spiffs` | data/spiffs | `0x290000` | 1408 KiB |
| `coredump` | data/coredump | `0x3F0000` | 64 KiB |

Main/Rod partition binary를 Core 2.0.17 `gen_esp32part.py`로 각각 decode했으며 두 결과가 동일하다.

## Upload option

```text
Chip         : ESP32-S3
Upload speed : 921600
Flash mode   : DIO
Flash freq   : 80 MHz
Flash size   : 4 MB
Before       : default_reset
After        : hard_reset
Compression  : enabled (-z)
```

전체 compile FQBN은 `PROJECT_COMMANDS.md`와 `docs/build/ARDUINO_REFERENCE_AUDIT.md`를 따른다.

## Debug 산출물 기준

| 대상 | ELF 바이트/SHA-256 | MAP 바이트/SHA-256 |
| --- | --- | --- |
| Main | 15,805,332 / `397341E439B37D277607D7327A0848AD243D81B4A5B584DB5C32ABED3D8844A6` | 14,108,238 / `3E44D994BF3E46ADA8CCFBF2CBB38F47598DD13ABE890FF520D9394A52D9013B` |
| Rod | 14,017,704 / `DBEC78D06ADB1AF2587688BD316DCD2F316102956109059FA5742B5909F2EE27` | 13,040,628 / `07601B14BDB967229664F80F2A62D75D13597A09B0792891EE9D8C7CC8127B45` |

## 재현 빌드 비교 정책

- 새 빌드의 성공 여부는 먼저 compile/link와 필수 flash 파일 생성으로 판정한다.
- 도구 경로, build timestamp와 비결정적 metadata 때문에 새 binary가 기존 SHA-256과 반드시 같다고 가정하지 않는다.
- firmware 크기 증감, partition/offset, version string과 기능 smoke test를 함께 비교한다.
- 이 문서는 기존 산출물의 기준값이며 실제 flash 명령 실행을 승인하지 않는다.

## 검증 기록

- `Get-FileHash -Algorithm SHA256`, 파일 길이, image header, partition decode와 기존 upload script를 교차 확인했다.
- 2026-08-27 VS2022 재현 build는 실행했으며 upload/flash는 실행하지 않았다.

## 2026-08-27 VS2022 재현 Build 비교

| 대상/파일 | 새 바이트 | 기존 대비 | 새 SHA-256 | 판정 |
| --- | ---: | ---: | --- | --- |
| Main application | 882,320 | 0 | `84345EEBA06C63CE815AB9CC6901C345A2EFFFC05E3BC29167E54DF2937FBD31` | 크기 일치, hash 차이 |
| Rod application | 763,552 | -16 | `2BA9DE60CA93B33AB442507AF470E774A1AA037376B73CC98E6DE8EF458E78FF` | 소폭 차이, hash 차이 |
| Main/Rod bootloader | 14,032 | -1,072 | `9BF9AAB34441621D794360CA427CBDDC3D62F38A7950F8790696BB3D7986769F` | Core 재생성본 차이 |
| Main/Rod partitions | 3,072 | 0 | `148B959CBFF1C38AA8E1D5C0BA9D612C54997B945E56A63F41223EEF650653A1` | 기존과 동일 |
| Main/Rod boot_app0 | 8,192 | 0 | `F94C5D786A7A8FAB06AC5D10E33BF37711A6697636DC037559EA19CC410A17F0` | 기존과 동일 |

새 build는 compile/link 및 필수 파일 생성 기준을 충족한다. 바이너리 hash가 기존 release와 동일하다는 판정은 하지 않는다. 또한 build 시점의 Main 진입 파일에는 사용자의 정렬/공백 변경이 작업 트리에 존재했으며 이 변경은 보존했다.

기존/신규 bootloader는 모두 ESP32-S3 image version 1, 3 segments, valid checksum/hash이며 header의 flash mode/frequency/size byte `02 2F`도 같다. 그러나 entry point와 segment 길이가 달라 timestamp만의 차이는 아니다. 실제 flash 전에는 신규 Core 재생성 bootloader를 사용할지 기존 검증 bootloader를 유지할지 장비 gate에서 확인한다.
