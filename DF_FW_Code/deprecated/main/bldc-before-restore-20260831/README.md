# BLDC 복구 전 코드

- 보존일: 2026-08-31
- 출처: firmware/DF_Main/src/BobbinMotor.cpp, Common.h
- 이유: 리팩토링 중 빠진 PWM 반전과 잘못 선택된 구형 모터 설정을 기준 원본으로 복구하기 전 파일 보존.
- 대체: 동일한 활성 소스 경로. 이 폴더는 include/빌드하지 않는다.
- 복구 후 검증·빌드·테스트는 사용자 수행이며 Codex는 실행하지 않았다.
