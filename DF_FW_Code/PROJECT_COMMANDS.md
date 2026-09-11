# DF Firmware 프로젝트 명령서

Updated: 2026-09-01

## 기본 경로

```text
Repository : C:\Projects\AI-AGENTS\DF_Project\DF_FW_Code
Arduino CLI: toolchain\arduino-cli\arduino-cli.exe
CLI Config : toolchain\arduino-cli.yaml
Arduino Data: toolchain\arduino-data
Arduino User: toolchain\arduino-user
```

`toolchain/`은 Git에 포함하지 않는다. 새 PC 또는 새 clone에서는 아래 환경 구성을 다시 수행한다.

## Arduino CLI 로컬 설정

`toolchain/arduino-cli.yaml`:

```yaml
board_manager:
  additional_urls:
    - https://espressif.github.io/arduino-esp32/package_esp32_index.json
directories:
  data: X:/toolchain/arduino-data
  downloads: X:/toolchain/arduino-downloads
  user: X:/toolchain/arduino-user
```

ESP32의 Windows GCC 8.4는 현재 저장소의 긴 절대 경로에서 `no-rtti/bits/c++config.h`를 열지 못한다. Build script는 저장소를 `X:`에 자동 매핑하고 위 짧은 경로를 사용한다. `X:`가 다른 용도로 사용 중이면 build 전에 비워 두거나 `arduino-env.cmd`와 이 설정의 drive letter를 함께 변경한다. 필요하면 `subst X: /d`로 매핑을 해제할 수 있다.

## Core 및 라이브러리 준비

PowerShell에서 저장소 루트를 현재 경로로 둔다.

```powershell
$cli = '.\toolchain\arduino-cli\arduino-cli.exe'
$config = '.\toolchain\arduino-cli.yaml'

& $cli --config-file $config core update-index
& $cli --config-file $config core install esp32:esp32@2.0.17
& $cli --config-file $config lib install 'ESP32Servo@1.1.2'
```

설치 확인:

```powershell
& $cli --config-file $config version
& $cli --config-file $config core list
& $cli --config-file $config lib list
& $cli --config-file $config board details --fqbn esp32:esp32:esp32s3
```

확정된 기준:

```text
Arduino CLI : 1.5.2-rc.1 (현재 로컬 설치본)
ESP32 Core  : 2.0.17
ESP32Servo  : 1.1.2
Base FQBN   : esp32:esp32:esp32s3
```

Rod의 `Kalman.h`는 현재 실행 코드에서 사용되지 않는 의존성으로 판정했으므로 설치하지 않는다. 원본 보존 후 활성 코드 정리 과정에서 include와 죽은 코드를 제거한다.

## 확정 Board option

Main/Rod 공통 전체 FQBN:

```text
esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,CPUFreq=240,FlashMode=dio,FlashSize=4M,PartitionScheme=default,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default
```

PowerShell 변수:

```powershell
$fqbn = 'esp32:esp32:esp32s3:UploadSpeed=921600,USBMode=hwcdc,CDCOnBoot=cdc,MSCOnBoot=default,DFUOnBoot=default,UploadMode=default,CPUFreq=240,FlashMode=dio,FlashSize=4M,PartitionScheme=default,DebugLevel=none,PSRAM=disabled,LoopCore=1,EventsCore=1,EraseFlash=none,JTAGAdapter=default'
```

판정 근거는 `docs/build/ARDUINO_REFERENCE_AUDIT.md`에 기록했다.

## 빌드와 검증 정책

- Main clean build script:

```powershell
.\tools\build-main.cmd
```

- Rod clean build script:

```powershell
.\tools\build-rod.cmd
```

- Main/Rod 통합 clean build:

```powershell
.\tools\build-all.cmd
```

- 공유 protocol/OTA frame host test:

```powershell
.\tools\test-protocol.cmd
```

Host test는 PATH에서 `g++.exe`를 찾아 C++98 모드로 실행하며 결과 실행 파일은 Git 제외 `artifacts/host-tests/`에 둔다.

각 script는 실패 code를 반환하며 필수 flash file의 존재를 검사한다. 인수를 생략하면 `release x64`를 사용하며, 필요하면 `build-all.cmd Debug x64`처럼 지정한다.

Arduino CLI의 전체 중간 산출물은 `artifacts/`에 둔다. 실제 플래시용 배포 결과는 아래 위치에 대상별 4개 파일만 복사한다.

```text
bin/<configuration>/<platform>/<version>/
├─ <sketch>.ino.bin
├─ <sketch>.ino.bootloader.bin
├─ <sketch>.ino.partitions.bin
└─ boot_app0.bin
```

출력 경로는 저장된 소스 버전을 따른다. 현재 사용자 변경값은 Main `Vm1.0.10.0`, Rod `Vr1.0.2.0`이므로 Release 출력은 각각 `bin/release/x64/Vm1.0.10.0`, `bin/release/x64/Vr1.0.2.0`이다. 앞선 설치/복구 기준 `Vm1.0.9.0`, `Vr1.0.1.3` 및 Rod OTA 최초 설치 기준 `Vr1.0.1.0`은 과거 산출물로 구분한다. `Vr1.0.1.2`는 과거 HANDLE LED 시험 산출물이다. Configuration은 소문자 `debug` 또는 `release`, 현재 지원 Platform은 `x64`다. 펌웨어 버전은 해당 `firmware/DF_Main/src/Version.cpp`의 `mainVer` 또는 `firmware/DF_Rod/src/Version.cpp`의 `rodVer`만 수정하고 저장한다. `tools/firmware-versions.cmd` → `firmware-versions.ps1`이 매 build/clean 실행 때 소스 값을 읽으므로 별도 버전값 동기화는 필요 없다. 형식은 Main `Vm숫자.숫자.숫자.숫자`, Rod `Vr숫자.숫자.숫자.숫자`이며 최대 24자다. 누락·중복·조건부 정의·잘못된 형식은 이전 값으로 대체하지 않고 build/clean을 중단한다. `bin/`과 `artifacts/`는 Git에서 제외된다.

주의: 2026-08-31 동적 경로 수정 전에 사용자가 소스 버전만 바꾸어 빌드한 결과, 이전 `Vm1.0.9.0`/`Vr1.0.1.3` 폴더의 app에 새 버전 내용이 들어가 있었다. 해당 폴더를 과거 기준 image로 간주하지 않는다. 아래 과거 복구 명령을 재사용할 때도 image 버전과 hash를 확인한다. 현재 빌드 결과는 `Vm1.0.10.0`/`Vr1.0.2.0` 폴더에 있다.

## Main 경유 Rod 무선 application 업데이트

최초 한 번 Rod에 OTA 수신기가 포함된 `Vr1.0.1.0` 네 파일을 유선 전체 설치하고, Main에도 현재 `Vm1.0.9.0` 네 파일을 설치한다. 이후 Rod application 하나만 Main USB 포트를 통해 전송한다.

```powershell
.\tools\upload-rod-ota.cmd '<MAIN_PORT>' '.\bin\release\x64\Vr1.0.1.3\DF_Rod.ino.bin' 'Vr1.0.1.3'
```

장비 없이 frame 생성, image 크기와 SHA-256 계산만 확인하려면:

```powershell
.\tools\upload-rod-ota.ps1 -Firmware '.\bin\release\x64\Vr1.0.1.3\DF_Rod.ino.bin' -Version 'Vr1.0.1.3' -DryRun
```

- 무선 대상은 `.ino.bin` 하나뿐이다. bootloader, partition, `boot_app0.bin`은 기존 유선 네 영역 설치 명령으로만 변경한다.
- Main에 등록되어 현재 연결된 Rod 한 대만 허용한다. 전송 중단 시 같은 명령을 처음부터 다시 실행한다.
- 성공 시 `[OTA] Completed`가 출력되고 Rod가 재부팅한다. 실패 상태는 Rod가 보고한 숫자와 frame 종류/sequence로 출력된다.
- 과거 `Vr1.0.1.2` 시험(현재 `Vr1.0.1.3`에서는 시험 LED 제거): R 버튼을 놓고 Rod HANDLE을 돌린다. 홀센서 입력이 변하면 즉시 점등하며 마지막 변화 후 200ms에 꺼진다. 연속 회전 중에는 계속 켜질 수 있다. 설치 버전 확인 없이 미점등만으로 센서/LED 고장을 판정하지 않는다.
- 2026-08-31 실제 OTA: 767,072 bytes, 명령 호출 11:16:54~완료 출력 11:19:43(KST), 약 2분 49초. PC 준비/ACK/검증을 포함하고 앞선 유선 설치는 제외한 실행 로그 기준이다. 재부팅 후 version과 사용자 HANDLE LED 점등·소등·연속 점등을 확인했다.

## Visual Studio 2022

Solution:

```text
DF_Firmware.sln
```

- Solution에는 실제 firmware project인 `DF_Main`, `DF_Rod`만 둔다.
- `Build Solution`: Main 다음 Rod 순서로 두 project를 각각 한 번씩 build한다.
- 명령행 통합 build에는 `tools/build-all.cmd`를 사용한다.
- 자세한 절차와 결과물 위치: `docs/build/VS2022_BUILD.md`
- Arduino/ESP32-S3 IntelliSense 공통 설정: `vs/DF_Arduino_ESP32S3.props`
- 활성 소스: `firmware/DF_Main`, `firmware/DF_Rod`
- 공통 protocol library: `libraries/DFProtocol`
- solution이 열린 상태에서 project 파일이 변경되면 `Reload All`을 선택하거나 solution을 다시 연다.
- Solution Build는 compile만 수행하고 upload/flash는 수행하지 않는다.
- Codex는 compile/link 빌드를 직접 실행하고 결과를 기록한다.
- upload, flash 및 장비 검증은 사용자가 수행하며 장비 게이트에서 필요한 명령, 예상 결과와 전달받아야 할 항목을 제시한다.

2026-08-28 callback/ISR 안전화 후 `build-all.cmd Release x64`와 VS2022 `DF_Firmware.sln` Rebuild가 모두 성공했다. 현재 기록은 Main application 882,749 bytes/배포 bin 883,120 bytes, Rod application 762,153 bytes/배포 bin 762,512 bytes이며 대상별 배포 파일 4개가 생성된다. project 파일이 변경되었으므로 열려 있던 VS2022에서는 `Reload All` 또는 solution 재열기를 수행한다.

## ESP32-S3 native USB 테스트 보드 업로드

업로드는 장비 종류와 포트를 확인하고 명시적으로 승인받은 테스트 보드에만 수행한다. Core 2.0.17의 기본 flasher stub이 native USB에서 끊기는 보드는 115200 baud ROM bootloader 방식으로 배포 파일 4개를 기록할 수 있다. `<PORT>`는 실행 시 확인한 포트로 바꾸며 저장소 문서에 개인 장비 포트를 고정하지 않는다.

```powershell
$esptool = '.\toolchain\arduino-data\packages\esp32\tools\esptool_py\4.5.1\esptool.exe'
$firmwareBin = '.\bin\release\x64\Vm1.0.9.0' # Rod 현재 복구는 Vr1.0.1.3으로 변경
$sketchName = 'DF_Main'                         # Rod는 DF_Rod로 변경

& $esptool --chip esp32s3 --port '<PORT>' --baud 115200 --before default_reset --after hard_reset --no-stub write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB --verify 0x0000 "$firmwareBin\$sketchName.ino.bootloader.bin" 0x8000 "$firmwareBin\$sketchName.ino.partitions.bin" 0xE000 "$firmwareBin\boot_app0.bin" 0x10000 "$firmwareBin\$sketchName.ino.bin"
```

2026-08-28 외부 장치가 없는 Main 테스트 보드와 Rod 테스트 보드에서 네 영역의 write/hash/verify와 각 firmware 버전 응답을 확인했다. Main–Rod 상호 등록 뒤 무선 버전 왕복도 확인했다. 999ms 1회 진동 명령에서 사용자가 릴 연동 LED 점멸과 약 1초 실제 진동을 확인했다. 테스트 보드의 물리 flash가 8MB로 검출돼도 제품 FQBN과 partition 기준은 4MB이므로 설정을 변경하지 않는다.

## Rod USB 빠른 업데이트 (2026-08-31 확인)

여기서 빠른 업데이트는 PC와 **Rod USB 직접 연결**이다. Main 경유 OTA가 아니다. 대상 장치/포트를 직접 확인하고 버전 조회 `$10%`에 Rod 응답만 오는지 확인한다. 사용자 승인 범위 밖에서 자동 실행하지 않는다. 시리얼 모니터를 닫고 출력 부하를 안전하게 정지한 상태에서 수행한다.

아래 예시는 현재 Core 2.0.17 default partition이 이미 설치된 Rod의 복구 절차다. 파티션을 읽어 배포 파일과 일치하는 경우에만 otadata와 app0를 기록한다. 이전 OTA가 app1을 선택했을 수 있으므로 app0만 기록하지 않고 otadata도 초기화한다. bootloader/partition/NVS/SPIFFS는 쓰지 않는다. 파티션이 다르거나 최초 설치라면 이 절차를 사용하지 않고 장비 구성을 확인한 뒤 네 영역 설치 절차를 사용한다.

```powershell
$dfEsptool = '.\toolchain\arduino-data\packages\esp32\tools\esptool_py\4.5.1\esptool.exe'
$dfRodBin = '.\bin\release\x64\Vr1.0.1.3'
$dfRodPort = '<ROD_PORT>'  # 실행 직전에 Rod 직접 포트로 바꿈
$dfPartitionRead = '.\artifacts\rod-partitions-check.bin'
New-Item -ItemType Directory -Path '.\artifacts' -Force | Out-Null

& $dfEsptool --chip esp32s3 --port $dfRodPort --baud 921600 --before default_reset --after no_reset --no-stub read_flash 0x8000 3072 $dfPartitionRead
if ($LASTEXITCODE -ne 0) { throw '파티션 읽기 실패: 업로드 중지' }
if ((Get-FileHash $dfPartitionRead -Algorithm SHA256).Hash -ne (Get-FileHash "$dfRodBin\DF_Rod.ino.partitions.bin" -Algorithm SHA256).Hash) {
    throw '파티션 불일치: 쓰지 말고 장비 구성을 먼저 확인'
}

& $dfEsptool --chip esp32s3 --port $dfRodPort --baud 921600 --before no_reset --after hard_reset --no-stub write_flash --compress --verify --flash_mode keep --flash_freq keep --flash_size keep 0xe000 "$dfRodBin\boot_app0.bin" 0x10000 "$dfRodBin\DF_Rod.ino.bin"
if ($LASTEXITCODE -ne 0) { throw '기록 또는 verify 실패: 성공으로 간주하지 않음' }
```

- 성공 기준: 두 영역 hash/verify 일치, 재부팅 후 직접 `$10%` 조회에서 `Vr1.0.1.3` 응답. 그 뒤 R 버튼 해제 상태의 HANDLE 회전에서 시험용 LED가 켜지지 않는지 사용자가 확인한다. 원래 버튼·부팅·연결 LED는 유지된다.
- 읽기/비교 중 중단되면 장비가 ROM 모드에 남을 수 있다. 임의로 다음 쓰기를 진행하지 말고 실패 이유를 확인한다. 고속 연결이 불안정하면 낮은 baud로 처음부터 재시도하며 성공 여부를 다시 검증한다.
- 이번 실행 결과: application 766,848 bytes → 502,058 bytes 압축, application 기록 약 21.7초. 파티션 확인·기록·검증 전체 33.86초(후속 버전 조회 제외). 두 영역 verify와 `Vr1.0.1.3` 응답은 완료, 물리 LED 관찰은 사용자 대기다.
- 이번 USB 직접 업로드는 사용자 명시 요청으로 수행했다. 이 절차 기록은 향후 자동 업로드의 상시 승인이 아니다.

## Git

```powershell
git status --short --branch
git remote -v
```

원격 저장소:

```text
https://github.com/Ornithopter83/DF_FW_Code.git
```

2026-09-09 현재 로컬 `main`은 실패 보정이 제거된 `8b2fd4f`이며 upstream을 해제했다. `origin/main`에는 실패 연구 중간 코드가 있으므로 사용자 지시 없이 pull, merge, reset 또는 push하지 않는다. 실패 연구는 `codex/imu-calibration-experiment-failed`에 보관한다.

commit, push 및 tag는 사용자의 명시적 요청이 있을 때만 수행한다.


## 독립형 TestModule 단일 EXE

새 프로그램은 루트 `DF_Firmware.sln`의 솔루션에 직접 포함되지만 펌웨어 프로젝트와 독립적이다. 배포 파일은 다음 명령으로 생성한다.

```powershell
.\tools\build-testmodule.cmd Release
```

배포 결과는 `bin/testmodule/Release/DFTestModule_V041.exe` 하나다. 일반 `dotnet build`의 개발용 중간 출력이 아니라 이 publish 결과를 배포한다. 대상 PC에 .NET 런타임을 별도로 설치할 필요가 없다.
