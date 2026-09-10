# DF Project repository guide

## P0 — 최우선 복구 정책

이 프로젝트는 신규 제작이 아니라 기존 게임의 복구다. 아래 규칙은 다른 모든 작업 편의, 일정, 구현 판단보다 우선한다.

- 근거 없는 임의 수정, 배치, 교체, 보정, 창작을 엄격히 금지한다.
- Scene의 화면, 3D Object, UI, 카메라, 조명, Material, Texture, 색상, 문구, 애니메이션, 이펙트와 상태 전환을 복구하려면 먼저 원본 근거를 확보해야 한다.
- 시각 요소를 유추해 복구할 때는 최소한 원본 스크린샷, 동영상 또는 이에 준하는 프레임 근거가 있어야 한다. 시각 근거가 없으면 조사와 목록화까지만 하고 Scene에 배치하지 않는다.
- 원본 Scene/Prefab YAML과 정확한 Transform·RectTransform·GUID가 남아 있으면 가장 높은 우선순위의 구조 근거로 사용하되, 실제 표시 결과는 가능한 경우 스크린샷·동영상과 다시 비교한다.
- Scene, Prefab, Mesh, Material, Texture, Sprite, Shader, Animator, Audio 등 필요한 항목은 구현 전에 원본 추출물 전체에서 존재 여부를 검색한다.
- 원본 추출물에 없거나 식별할 수 없는 항목은 임의 생성하거나 비슷한 자산으로 자동 교체하지 않는다. `DF_AP_Custom/Docs/RECOVERY_ISSUES.md`에 누락 위치, 검색 범위, 필요한 근거, 영향과 다음 확인 방법을 기록한다.
- 유사 복구가 허용된 경우에도 확보된 근거의 범위 안에서 최대한 원본과 가깝게 재현하고 `Reconstructed` 또는 `Estimated`로 명시한다. `Estimated` 시각 배치는 사용자 승인이 필요하다.
- 근거가 충돌하면 수정하지 않고 충돌 내용을 기록한 뒤 사용자 확인을 받는다.

### 필수 근거 순서

1. 원본 Scene/Prefab YAML, GUID, `.meta`, 직렬화 값
2. 원본 추출물의 실제 Mesh/Material/Texture/Shader/Animator/Audio/Data
3. 원본 코드, DLL, Addressables/Build Settings 및 상태 전환 자료
4. 원본 스크린샷, 동영상과 프레임별 비교 자료
5. 사용자의 직접 확인

각 Scene 작업은 변경 전에 `확정 근거`, `시각 근거`, `원본 추출물 검색 결과`, `미확정/누락`, `구현 금지 항목`을 활성 작업지시에 기록해야 한다.

## Start here

Read `FISHING_GAME_RECOVERY_PLAN.md`, `CurrentWork.md`, the active root `tasks/*.md`, then only the relevant evidence under `DF_AP_Custom/Docs` and `DF_Integration`.

## Scope and evidence

- Final deliverable: a fully restored `DF_AP_Custom/UnityProject`.
- Read-only evidence: `DF_AP_Custom/ExportedProject`, `DF_AP_Custom/AuxiliaryFiles`, `DF_AP_1.3.5_Ext`, and `DF_FW_Code`. Never move, rename, edit, or delete them.
- `DF_Integration` is the default AP–FW integration reference folder. Treat `DF_Integration/PROTOCOL.md` as the current integration contract.
- Consult `DF_FW_Code` only when `DF_Integration` is insufficient or a protocol conflict requires source-level verification. Record verified integration conclusions in `DF_Integration`; do not make Unity recovery depend on paths inside the firmware repository.
- Keep Unity recovery findings in `DF_AP_Custom/Docs`; keep work control only in the root roadmap, `CurrentWork.md`, handoff, and root `tasks/`.

## Recovery rules

- Recover one Scene at a time with only its required code and assets.
- Before editing a Scene, record its evidence, hierarchy, main Transform/RectTransform values, state transitions, and dependency closure.
- Valid evidence is original YAML/Prefab data, GUID/meta pairs, recovered code or DLLs, extracted assets, runtime images/video, firmware where applicable, or direct user confirmation.
- Do not invent UI, 3D objects, text, buttons, animations, effects, transitions, or values. `Estimated` and `Placeholder` do not authorize visual placement.
- Preserve valid original `.meta` files and GUIDs. Investigate GUIDs, metas, source, and DLLs before removing Missing Scripts.
- Adopt runtime assets under normal `DF_AP_Custom/UnityProject/Assets` paths. `_ReferenceTemp` must not become a runtime dependency.
- Record provenance as `Original`, `Reconstructed`, `Estimated`, or `Placeholder`; do not add `Recovery` prefixes to production names.

## Execution and validation

- Work on one numbered order and one lettered unit at a time unless the user requests a batch.
- Update completion dates, remaining identifiers, results, `CurrentWork.md`, and the roadmap after verified work.
- Separate static inspection, Unity import/compile, EditMode/PlayMode, GPU capture, build, and physical-device verification. Opening successfully is not proof of restoration.
- Preserve simulator/adapter boundaries so the project runs without cabinet hardware.
- Never connect real COM ports, drive motors, disable USB devices, shut down the OS, charge cards, or contact production services without explicit authorization.
- Preserve unrelated changes. Do not upgrade Unity, packages, render pipelines, or architecture during recovery without approval.
- Never expose credentials, tokens, payment data, device identifiers, or sensitive URLs.
- Do not commit, pull, push, build, deploy, or mutate external systems without explicit authorization.
