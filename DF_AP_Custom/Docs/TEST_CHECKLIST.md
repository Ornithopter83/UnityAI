# Test Checklist

## Phase 0 정적 검증

- [x] 원본 경로가 존재하며 새 프로젝트와 분리됐다.
- [x] 전체 manifest와 SHA-256을 생성했다.
- [x] 생성 전후 원본 파일 수, 크기, 최신 수정 시각이 동일하다.
- [x] GUID 참조 무결성을 R1 범위로 전수 조사했다.
- [x] 중복 GUID와 미해결 Scene/Prefab script 참조가 없다.
- [x] 해소되지 않은 일반 YAML 참조 후보를 별도 목록으로 기록했다.

## Unity 검증

- [x] Unity `2022.3.62f3`에서 새 프로젝트가 열린다.
- [x] 패키지 해석 오류가 없다.
- [x] 스크립트 컴파일 오류가 없다.
- [x] Bootstrap과 Loading Scene을 저장·재로드할 수 있다.
- [x] 두 Scene의 필수 컴포넌트와 Build Settings 등록을 확인했다.
- [x] Scene 자체는 하드웨어와 외부 서비스에 의존하지 않는다.
- [ ] Bootstrap → Loading → Bootstrap 버튼 흐름의 사용자 Play Mode 확인

## 빌드 검증

사용자가 별도로 허가할 때까지 수행하지 않는다.
