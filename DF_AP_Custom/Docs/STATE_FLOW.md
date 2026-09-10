# State Flow

현재 문서는 클래스명과 `StateManager.eState`를 근거로 한 구조 초안이다. 전환 조건은 상세 IL 분석 전까지 `Estimated`다.

```text
Title
  -> Loading
  -> Game/Play Mode
  -> Character / Level / Map
  -> StageInfo
  -> PlayerWait
  -> CastingRotate / CastingAction / Casting
  -> Wait
  -> Bite
  -> Hooking
       -> HSuccess -> Fight <-> Hold -> RodAction
       -> HFail ---------------------> Wait/Fail (Estimated)
  -> Finish
       -> FSuccess -> Catch
       -> FFail / FightFail
  -> Continue / Store / StageClear / Gameover
  -> BonusGame / FinalQuest / MatchResult / Last
```

## 1차 Vertical Slice 목표

```text
Test Stage
-> Casting
-> Wait
-> Bite
-> Hooking
-> Fight <-> Holding
-> Finish
-> Catch Result
-> Restart same stage
```

## 전환 로그 목표 형식

```text
[State] State_Wait -> State_Bite
Reason: BiteTimerElapsed
FishId: FISH-003
StageId: STAGE-01
```

## 조사 과제

- 실제 초기 상태와 Scene별 StateManager 생성 위치
- 각 상태 `Enter/Update/Exit` 상당 메서드 signature
- 성공/실패/타임아웃 조건
- Fight/Holding의 장력, 체력, Reel 입력 계산
- Continue/Store/StageClear 분기와 저장 부작용
