# DF 통합 Git 관리 지침

## 1. 관리 기준

- 통합 Git 루트는 `C:\Projects\AI-AGENTS\DF_Project`이다.
- 기본 브랜치는 `main`, 원격 저장소는 `https://github.com/Ornithopter83/UnityAI.git`이다.
- `DF_FW_Code`는 더 이상 독립 Git 저장소나 서브모듈로 관리하지 않는다.
- 펌웨어, TestModule, AP 연동 문서는 모두 상위 `DF_Project` 저장소에서 함께 변경·검토·커밋한다.

## 2. 독립 저장소 종료 기준

- 독립 `DF_FW_Code` 저장소의 마지막 확정 커밋은 `c84be58`이다.
- 이 커밋은 `https://github.com/Ornithopter83/DF_FW_Code.git`의 `main`에 마지막으로 푸시했다.
- 이후 `DF_FW_Code.git`에는 새 커밋을 만들거나 푸시하지 않는다.
- `DF_FW_Code` 내부에서 `git init`을 실행하거나 별도 `.git`을 다시 만들지 않는다.

## 3. 일상 작업 위치

모든 Git 명령은 상위 루트에서 실행한다.

```powershell
cd C:\Projects\AI-AGENTS\DF_Project
git status -- DF_FW_Code DF_Integration
git diff -- DF_FW_Code DF_Integration
git add -- DF_FW_Code DF_Integration
git commit -m "변경 내용"
git push origin main
```

`DF_FW_Code` 폴더 안에서 명령을 실행하더라도 Git 루트는 상위 폴더로 확인되어야 한다.

```powershell
git rev-parse --show-toplevel
```

정상 결과:

```text
C:/Projects/AI-AGENTS/DF_Project
```

## 4. 추적 및 제외 원칙

- 추적: 펌웨어 소스, TestModule 소스, 프로젝트·솔루션 파일, 프로토콜 및 작업 문서, TestModule 이미지 자산과 필수 내장 도구.
- 제외: `bin`, `obj`, VS 빌드 로그, 복원 가능한 레거시 패키지, 게시 결과, 임시 파일, 서명키, 로컬 설정 및 비밀 정보.
- 빌드 결과를 버전 관리해야 하는 특별한 경우에는 상위 저장소 정책을 먼저 갱신하고 별도 승인을 받는다.
- `DF_Integration/PROTOCOL.md`와 연관 구현은 가능하면 같은 커밋에서 함께 관리한다.

## 5. 금지 사항

- `DF_FW_Code`를 독립 저장소 또는 Git 서브모듈로 다시 추가하지 않는다.
- 과거 독립 저장소소의 `.git` 보관본을 작업 폴더로 복원하지 않는다.
- 원격 이력이 갈라졌을 때 `--force` 또는 `--force-with-lease`로 임의 덮어쓰기하지 않는다.
- 빌드·테스트·게시 결과를 소스 변경과 무관하게 자동 커밋하지 않는다.

## 6. 전환 기록

- 전환일: 2026-09-11
- 독립 저장소 마지막 브랜치: `main`
- 독립 저장소 마지막 커밋: `c84be58`
- 전환 후 관리 주체: 상위 `DF_Project` 저장소
- 독립 `.git`은 삭제하지 않고 작업 트리 밖의 복구용 보관 위치로 이동한다.
- 복구용 보관 위치: `C:\Projects\AI-AGENTS\GitArchives\DF_FW_Code.git-20260911-c84be58`
