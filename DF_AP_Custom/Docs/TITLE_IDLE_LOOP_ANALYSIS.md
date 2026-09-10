# Title 대기·랭킹·영상 루프 분석

조사일: 2026-07-16

## 결론

원본 Title에는 단순 결제 대기만 있는 것이 아니라 다음 유휴 루프가 존재한다.

```text
Title 결제 대기
  -> 30초 대기
  -> 랭킹 화면 스크롤
  -> 안내 이미지 2장(각 3.5초, 중복 없이 임의 선택)
  -> Intro 영상 페이드 인 및 재생
  -> 영상 종료
  -> 영상 페이드 아웃
  -> Title BGM과 GAMEWAIT 복귀
  -> 다시 30초 대기
```

결제 입력과 이 유휴 루프가 동시에 존재하므로, 향후 복원에서는 입력 발생 시 랭킹·이미지·영상 표시를 중단하고 결제 흐름을 우선하는 조건도 원본 호출 관계에서 확인해야 한다.

## 코드 근거

### `GameSupport.cs`

- `Start()`에서 `LoadIntroVideo()` 후 `IStartIntro()` coroutine을 시작한다.
- Serial 모드에서는 slave 연결 완료까지 기다린다.
- `_intro_wait_time`만큼 기다린 뒤 `TitlteRankUi.RankStart()`를 실행한다.
- 8개의 `_rand_images` 중 두 장을 중복 없이 골라 각각 3.5초 표시한다.
- 영상 재생 전에 RawImage를 약 0.5초 동안 투명에서 흰색으로 페이드한다.
- 영상이 끝나면 외부 음원을 감쇠·정지하고 Title BGM을 재개한다.
- RawImage를 다시 투명하게 만든 뒤 모터 상태를 `GAMEWAIT`로 보낸다.
- 전체 과정은 `while (true)`로 반복된다.

### `TitlteRankUi.cs`

- content Y 위치는 `0 -> 765 -> 1545 -> 2120 -> 0` 순서로 이동한다.
- 각 구간 전 3초 대기, 이동 시간 1초, 마지막 복귀 후 1.5초 대기가 직렬화되어 있다.
- 확보된 소스는 `_ranks` 30개를 설정한다.

### `DataManager.cs`

- `RankingLoad()`와 `RankingInit()`은 `i = 1; i < 31` 범위로 30개 랭킹 데이터를 읽는다.
- 따라서 현재 export 증거만 보면 1~30위 버전이다.

## Scene 직렬화 근거

- `GameSupport._intro_wait_time`: `30`
- `GameSupport._title_rank`: `TitlteRankUi` component
- `_rand_images`: `Hold`, `Hook`, `Store`, `Point`, `Mystic`, `WaitAndHook`, `LRCont`, `Lv`
- `VideoPlayer`: `intro` GameObject, Play On Awake 꺼짐, Looping 꺼짐
- 원본 VideoClip: `VideoClip/fishing.webm`, GUID `df79dba8ba6a59047bbbcf0f17cd29f4`
- 영상 RenderTexture: `RenderTexture/intro_render_texture.renderTexture`, GUID `89e274a6979f22d4abb68d8ebc9d0105`
- 내장 음원: `AudioClip/Intro_bg.ogg`, GUID `65072be49fcecec48b3dee173c293030`
- 외부 대체 파일 경로: 실행 파일 폴더의 `Intro/Intro.mp4`, 선택적 `Intro/IntroSound.mp3`

## 수면 근거

- Title의 `ocean`은 Crest OceanRenderer를 사용한다.
- `_globalWindSpeed = 150`
- `FFT` 입력의 `_waveDirectionHeadingAngle = 0`, `_windSpeed = 150`, `_windTurbulence = 0.145`
- `maxVerticalDisplacement = 10`, `maxHorizontalDisplacement = 15`
- `_createFoamSim = 1`, `_createFlowSim = 0`
- 따라서 보이는 우측 물살은 별도 Flow simulation보다는 강한 방향성 Animated Waves/FFT와 foam 결과로 판단된다.
- 현 커스텀 물 셰이더에는 +X 진행 파동, 수평 chop, +X normal scroll을 1차 반영했다. Crest FFT/foam과 동일하지 않으므로 `Estimated` 재작업 대상으로 둔다.

## 랭킹 수량 확정

- 현재 export 코드/Scene은 30개 항목과 1~30위 데이터를 사용한다.
- 사용자가 랭킹 수량은 30개가 맞다고 확인했다.
- 따라서 이번 복원 대상은 30개로 확정한다.

## 구현 상태

- `TitleIdleLoop`가 원본 정상 시간과 순서를 보유한다.
- `TitleRankRowView` 30개와 원본 PlayerPrefs 키 계약을 복원했다.
- `Rank`, `GameSupport`, 8개 안내 이미지, `intro` VideoPlayer/RenderTexture 계층을 원본 이름과 자산으로 구성했다.
- `fishing.webm` 내장 재생과 실행 폴더 `Intro/Intro.mp4` 대체 경로를 지원한다.
- Unity Scene/compile 검증과 단축 Play Mode 전체 루프 검증을 통과했다.
- 정상 30초 장시간 실행, 외부 영상 파일, 결제 입력에 의한 유휴 루프 취소 조건은 추가 수동/코드 조사가 남아 있다.

## 복원 순서

1. 우측 방향으로 요동치는 수면을 작은 단계로 보정하고 매 단계 캡처 비교
2. 30개 랭킹의 정상 시간 전체 스크롤을 수동 확인
3. 안내 이미지 8개와 두 장 비반복 선택을 정상 시간으로 수동 확인
4. `fishing.webm` 및 외부 `Intro.mp4` fallback을 실제 파일로 수동 확인
5. Title 입력 시 유휴 루프 취소 조건과 Title 복귀 반복 검증

구조와 단축 실행 검증은 완료했지만 위 수동 검증과 입력 취소 조건이 끝나기 전에는 Title 전체 복원을 완료로 표시하지 않는다.
