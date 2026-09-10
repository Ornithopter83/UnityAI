# UnityProject

Unity `2022.3.62f3`에서 검증하는 Fishing Game 프로젝트다.

복원한 코드와 자산은 `Assets/Scripts`, `Assets/Scenes`, `Assets/Prefabs`, `Assets/Art` 등 정상 제작 경로에 배치한다. `Assets/_ReferenceTemp`는 분석용 임시 복사본만 허용하며 런타임에서는 참조하지 않는다.

현재 Bootstrap과 Loading Scene은 외부 하드웨어, 결제, 네트워크, Addressables 없이 기본 Scene 흐름을 검증하는 안전한 진입점이다.
