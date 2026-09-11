# DF Firmware 코딩 및 구 코드 관리 정책

Updated: 2026-08-27

## 활성 코드 범위

- 활성 코드는 현재 사용하는 Main과 Rod 하드웨어만 지원한다.
- 과거 전체 기준 소스는 `legacy/`에 변경 금지 상태로 보존한다.
- 리팩토링 중 제거하는 코드는 저장소 루트 `deprecated/main/` 또는 `deprecated/rod/`에 기능별 파일로 보존한다.
- deprecated 파일에는 원본 경로, 제거일, 제거 이유, 대체 파일을 기록한다.
- deprecated 코드는 include, compile, test 또는 현대화하지 않는다.

## 언어 수준

Arduino toolchain의 실제 컴파일 모드와 무관하게 프로젝트 작성 코드는 C++98/03 문법 범위로 제한한다.

금지:

- 람다, 스마트 포인터
- `auto`, `nullptr`, `constexpr`, `enum class`
- range-for, initializer list, move 문법, `override`, `final`
- C++11 이후 표준 라이브러리 추상화
- namespace
- 불필요한 동적 할당과 예외 기반 제어

선호:

- `typedef struct`, `typedef enum`, 함수 포인터
- 고정 크기 배열과 명시적 길이
- `char` 버퍼와 정적 메모리
- 명시적인 자료형과 초기화/종료 함수
- 상태가 필요한 경우에만 단순한 클래스
- 선언은 `.h`, 구현과 저장 공간은 `.cpp`

## 네이밍

- 클래스: `CDFMainApplication`, `CDFRodApplication`, `CDFProtocol`
- Main 전역 함수/자료형: `DF_Main_...`
- Rod 전역 함수/자료형: `DF_Rod_...`
- 공통 프로토콜: `DF_Protocol_...`
- 전역 변수: `g_`와 기능 접두어
- 클래스 멤버: `m_`
- 충돌 가능성이 있는 짧은 전역 이름을 만들지 않는다.

## 전처리기

- 제품 Variant용 조건부 컴파일을 새로 만들지 않는다.
- 현재 항상 사용하는 기능은 조건 없이 직접 호출한다.
- 활성 제품 소스에는 제품 선택용 `#if/#ifdef/#else/#endif`를 두지 않는다. 변경 전 분기는 `deprecated/`와 `legacy/`에서만 조회한다.
- 헤더 보호는 `#pragma once`를 사용한다.
- 정수 상수는 익명 `enum` 또는 선언/정의를 분리한 `const`를 사용한다.
- 문자열 상수는 헤더의 `extern const char[]`와 `.cpp`의 단일 정의로 관리한다.
- 외부 Arduino/ESP32 Core 내부 전처리기는 변경 대상이 아니다.

## 참고 스타일

선언/구현 분리와 명시적인 자료형 사용은 `C:\Projects\VS\RS232\BoardIO.h/.cpp` 형식을 참고한다. Windows 전용 API, 전역 singleton 및 동적 할당을 그대로 복제하라는 의미는 아니다.
