//using DFTM;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace DF_TM
{
    //public static class VersionInfo
    public class VersionInfo
    {
        // 상수: 버전 문자열
        public string Version = "V0.3.4.00_";

		public static readonly string BuildType =
#if DEBUG
            "Debug";
#else
            "Release";
#endif

		// 상수: 빌드 날짜
		private static DateTime GetBuildTime()
		{
			string filePath = Assembly.GetExecutingAssembly().Location;
			return File.GetLastWriteTime(filePath);
		}
		private static readonly string ExecDate = "Run : " + DateTime.Now.ToString("yyyy-MMdd_HH:mm:ss");
		private static readonly string BuildDate = $"Build({BuildType}) : " + GetBuildTime().ToString("yyyy-MMdd_HH:mm:ss");

		public string BuildYYmmdd = BuildDate + ", " + ExecDate;
        public string getConfig()
        {

            if (false)
            {
                // dummy
            }
            else if (Config.IsDevelope) return "DEV";
            else if (Config.IsBoard) return "BRD";
            else if (Config.IsManuFeildService) return "MFS";
            else
            {
                return "NOTDEF";
            }
        }
    }
}


/* -- Version Histroty
 * 
 * 
 * ***M/S구분은 Program.cs파일에서************
 * *** Config 변수를 수정할 것 ***************
 * 
 *========아래는 양산용VER 이력============
 * ---------------------------------  
 * TEST V0.3.4.00_MFS, 26/1/15
 *   - 펌웨어 다운로드 기능 추가

 * TEST V0.3.3.02_MFS, 26/1/7
 *   - 타 통신포트 연결시 데드락 발생 방지.
 *   - Build Date와 Run Date 표시
 *   - Poll($00%) 주기 조정과 로그 필터링(폴주기(초) 더블클릭)

 * REL V0.3.3.00_MFS, 25/9/11
    Motor Off ~ End Stop Time BOX추가

 * TEST V0.3.2.50_MFS, 25/9/8
        - Torq + Bldc 동시 On/Off 버튼 추가
        - LOG출력 시간Stamp에 SS추가(HH:MM => HH:MM:SS)
 *   
 *== REL V0.3.2.01_MFS   25/7/11
 *  -토크모터 최소출력 설정 결과 오류 수정
 *
 *== REL V0.3.2.00_MFS   25/7/10
 *  - 통신로그 저장 추가
 *  - 예외처리 판별 추가
 *
 *
 *  rel V0.3.1.68_MFS   25/7/8
 *      - 메인모터 자동설정 위치 변경
 *      -
 * ---------------------------------  
 * TEST V0.3.2.50_MFS, 25/7/11
 *   -각종 예외처리 추가
 * 
 ** TEST V0.3.1.61_MFS, 25/7/8
 *      - BLDC Duty1,Duty2 추가
 ** TEST V0.3.1.60_MFS, 25/7/2
 *      - Aging Window ADD
 *      - Wire Aging Action ADD
 *      - Wire Count Display
 *      - 버전클래스 분리
 *  --------------------------------
 *  
 * == REL V0.3.1.0_MFS  25/06/30
 *      - 제조정보/설치정보 금지
 *      - 메인모터 최소출력 자동설정 추가
 *      - 메인보드 버전 추가
 *      - 메인 엔코더 회수 추가
 * 
 * REL V0.3.0.0_ManuFactory
 *     생산용
 * REL V0.3.0.0_Service
 *     서비스용
 *
 *=====아래는 시생산용VER 이력=======
 *
 * REl V0.2.7.0_M (생산용) 25/5/21
 *      - 
 *      -
 *      -
 * REl V0.2.6.0_M (생산용) 25/5/9
 *      -
 *      
 * REL V0.2.6.0_B(보드용) 25/4/28
 *
 *======아래는 개발용VER 이력========
 * V0.2.6.60, 25/4/28
 *  - LED 밝기 읽기 확인 필요 (Length변경)
 * 
 * V0.2.5.60_K  25/4/28
 *      - TM프로그램 종류추가(권한설정) -  K : D,B,M,F
 *      - IO 확인 변경(Cont메인)
 *      
 * REL V0.2.5.0 25/4/7
 * V0.2.4.62
 *      - LM모두 삭제
 *      - Cont메인 입력(6개) 추가
 *      - Cont메인 출력(1개 : SubAC on/OFF) 추가
 *      - LED출력 UI 위치이동/정리
 *      
 *      
 * V0.2.4.60    25/4/3 kps
 *      - JM Mot SetPara삭제
 *      - Rod BreakMotor삭제
 *      - 프로그램 시작시 TM 통지 추가
 *      - 프로그램 시작시 IMU데이타 출력ON추가
 *      - IMU Connention 표시조건 변경
 *      - 실행버튼 위치/폰트크기 조정
 *      - 연결끊길때 Ver문구 클리어
 *      - 재연결시 Ver읽기 추가
 *      - 송신 커맨드창 2개(총6개)로 추가
 *      
*	V0.2.4.1a	25/3/20 kps
		- File Save "#end:" #추가
		- Version명칭 변경 DFTM_V~(스페이스 삭제)
		- Error문구 수정
 *  V0.2.4.1 25/03/20 박수호
 *      - LM Save, Load 파일형식을 AP의 ATset.txt 파일 형식에 맞춤.
 *  V0.2.4 25/03/14 박수호
 *      - LM관련 명령버튼 추가 (LM ALL OFF, LM HOME, LM LEFT, LM RIGHT, LM RETURN)
 *      - LM 값변경 설정창 추가
 *      - LM Load 버튼, 기능 추가
 *      - LM Save 버튼, 기능 추가
 *      - LM Level별 Send 버튼, 결과창 추가
 *      - 실패/성공 메시지 팝업창 추가
 *  V0.2.3 25/03/07 박수호
 *      - Reel 진동모터, LED 추가
 *      - LM모터 추가
 *      - 문구수정 BLDC->보빈
 *      - 송신 커맨드창 추가
 *      - 송수신 로그 출력창 추가
 *      - 연결시 ProgramStart/VersionRead 자동 전송
 *  V0.2.2 24/12/20 kang
 *      -에러 문구(4개+ 2개) 추가
 *      -에러 응답 cmd오류 수정(15 -> $15)
 *  V0.2.2 24/12/09 shin
 *      -Device Check 버튼/에러코드 text박스, 에러문구 text박수 추가
 *      -Version Read추가
 *      -배치변경(왼쪽 Main, 오른쪽 Rod)
 *  V0.2.1 shin
 *      - New IF대응
 *      - LED추가
 *  V0.2.0 shin
 *      -DFTM ioTest용으로 변경
 *      - Servo삭제, BLDC추가
 *  V0.1.0 fotents
 *      -Debugger/AddressManager명칭의 Code입수
 */

