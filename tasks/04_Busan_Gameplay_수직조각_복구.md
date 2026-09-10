# Busan Gameplay 수직 조각 복구

## 목표

Busan Scene의 3D 환경과 핵심 낚시 상태 흐름을 end-to-end로 복원한다.

## 상세 작업 단위

### A. Scene hierarchy와 3D 자산 채택
- Terrain, water, lighting, StageObjects, MainObject, castPoint를 복원한다.

### B. 공통 manager와 UI 연결
- StageManager, MainObjectCont, RodCont, TensionCont와 필수 UI/Data를 연결한다.

### C. 핵심 낚시 상태 흐름 복원
- Casting부터 Catch까지 최소 수직 조각을 복원한다.

### D. PC simulator와 Scene 검증
- 하드웨어 없이 입력·상태·복귀를 검증한다.

## 진행률

잔여 작업 4개 (A, B, C, D)

## 변경 금지

- 원본 3D 배치/수치를 추측하거나 다른 지역 고유 자산을 섞지 않는다.

## 인수 기준과 테스트

- Busan 3D 환경과 낚시 루프가 GUID/Transform, compile, PlayMode, GPU 검증을 통과한다.

## 결과

- 아직 작업 전
