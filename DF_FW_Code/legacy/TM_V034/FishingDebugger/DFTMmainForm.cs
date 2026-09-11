

//#define LM_CMD_BTN false

//using DFTM;
using System;
using System.Collections.Generic;
using System.Data.SqlTypes;
using System.Diagnostics.Eventing.Reader;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Runtime.Remoting.Contexts;
using System.Management;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace DF_TM
{

    public partial class DFTMmainForm : Form
    {

        public string sVersion;

        public bool com_RTS = true;
        public bool com_DTR = true;

        //=== 각 클래스에 넘길 생성자 선언.
        private LogMgr logMgr;
        //private LogText logText;

        private SerialManager uartSerial;          // 통신 instance 선언

        private SerialPort serialPort;      // 통신포트 instance 선언
        //------------------------------------------------------------

        private string recvMsgKind = "";          // 수신msg 종류

        private int iTorque_Control_Out_Value;
        //private int iBreak_Control_Value;
        private int iBLDC_Duty_Value;
        private int iBLDC_OnTime_Value;
        private int iLED_Mode_Number;
        private int iLED_Pos_Number;
        private int iLED_Color_Number;
        private int iLED_Control_Time;
        private int iLED_Brightness_Number;

        private int iLM_Duty_Value;
        private int iLM_OnTime_Value;



        private Timer timer;
        private int iTmSystemTick500ms;
        //private int iTmSystemTick500msCnt;
        private short iPowerOnCmdSendCnt;

        private short iTmSystemTick2secFlag;

        private int iMainAliveReqSendCnt;

        private const short iCONTMAIN_DISCONN_TIME = (8 / 2);    // = 20초 / 2초 마다

        private string strMsg = "";
        private int intMsg;
        private short shtVal;

        private bool isEnterSend1Released;

        private string tmKind = "";

        private string mainMot_DefaultUnknown = "--";  // 메인모터 기본값 Duty
        private string mainMot_DefaultDuty = "70";  // 메인모터 기본값 Duty

        private int iDutyVal = 0;    // 10~255

        // 프로그램 시작시 에이징 카운트 가져오기
        private string agingCntFileName = Application.StartupPath + @"\AgingCount.txt";
        public string comLogFileName = Application.StartupPath + @"\comLog";

		//public static string logFileName = Application.StartupPath + @"\log.txt";

		public bool Poll_Filter = false;   // false or true
		private int Poll_Interval = 2;      // 0-60

		public delegate void ReceiveMessageCallback(string msg);
		public event ReceiveMessageCallback ReceiveDnMessage;
		public const string VID = "303A";
		public const string PID = "1001";
		public string MainComPortName;

		class UsbComPort
		{
			public string ComPort;
			public string DeviceId;
		}


		//
		// -1 : 모름
		// -2 : Reel연결끊김
		// -3 : 충전 변화없음
		// -4 : 충전중
		// -5 : 사용중(방전)
		private int batLevelCharging = 0;    // 0~99, 잔량

        enum EREELGroup
        {
            REEL_MOTOR,
            REEL_LED_LEFT,
            REEL_LED_RIGHT,
        }
        private EREELGroup eSelectedReelMenu = EREELGroup.REEL_MOTOR;

        // 약어에 따른 설명 매핑용 딕셔너리
        private Dictionary<string, string> fullnameMap = new Dictionary<string, string>()
        {
            { "SF", "Strong Family" },
            { "FF", "Fishing Family" },
            { "KF", "Kiosk Fishing" },
            { "TF", "Table Fishing" },
            { "PF", "PC Fishing" },
        };

        public DFTMmainForm()
        {
            try
            {
                InitializeComponent();

                InitializeSerialPortUI();
                LM_JIG_Init();

                //InitializeLogSaveMin();   // 더 공부해!!!


                this.StartPosition = FormStartPosition.Manual;
                this.Location = new Point(70, 80);      // 좌측, 상단

                uartSerial = new SerialManager(this);     // 인스턴스 생성

                logMgr = new LogMgr(this);  // 생성자 주입

                //logText = new LogText(this);  // 생성자 주입

                LogText.Log("DO 메인폼 초기화");
            }
            catch (Exception ex)
            {
                LogText.Log("메인폼 에러: " + ex.ToString());
                MessageBox.Show("메인폼 에러: " + ex.Message);
                // 또는 로그로 저장
            }
        }


        // 버튼,텍스트박스에서의 참조 <<예>>
        /*
        public void AppendToLogBox(string msg)
        {
            if (txtLog.InvokeRequired)
            {
                txtLog.BeginInvoke(new Action(() => AppendToLogBox(msg)));
            }
            else
            {
                txtLog.AppendText(msg + Environment.NewLine);
                txtLog.SelectionStart = txtLog.TextLength;
                txtLog.ScrollToCaret();
            }
        }
        */

        // 외부에서 메시지 SHOW용
        //----------------------------------------------
        public void ShowErrorSafe(string msg)
        {
            if (this.InvokeRequired)
                this.BeginInvoke(new Action(() => ShowError(msg)));
            else
                ShowError(msg);
        }

        public void ShowError(string msg)
        {
            MessageBox.Show(msg, "에러", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        //-----------------------------------------
        public void ShowErrorSafeRTB(string msg)
        {
            if (this.InvokeRequired)
                this.BeginInvoke(new Action(() => rTb_ShowError(msg)));
            else
                rTb_ShowError(msg);
        }
        //===================
        private void rTb_ShowError(string message)
        {
            rTb_Error_ErrMsg.Text = message;
            //lb_ErrMessage.Visible = true;
        }

        public void ShowErrorSafeTB(string msg)
        {
            if (this.InvokeRequired)
                this.BeginInvoke(new Action(() => tb_ShowError(msg)));
            else
                tb_ShowError(msg);
        }

        public void SetMainVersion(string strVer)
        {
            Textbox_Main_Ver.Text = strVer;
		}
		public string GetMainVersion()
		{
			return Textbox_Main_Ver.Text;
		}

		//=======================
		private void tb_ShowError(string message)
        {
            tb_Error_OkMsg.Text = message;
            //lb_ErrMessage.Visible = true;
        }

        private void LoadItemsToDomainUpDown()
        {
            try
            {
                //string currentDir = Application.StartupPath;
                string txtPath = Path.Combine(Application.StartupPath, "MainConfig.txt");

                if (!File.Exists(txtPath))
                {
                    // 제조정보 사용하는 경우, true로 변경할 것
                    if (false)
                    {
                        MessageBox.Show($"파일이 없습니다:\n{txtPath}");
                    }
                    return;
                }

                // 파일 내용 읽어서 DomainUpDown에 추가
                string[] lines = File.ReadAllLines(txtPath);
                foreach (string line in lines)
                {
                    if (!string.IsNullOrWhiteSpace(line))
                        dud_config.Items.Add(line.Trim());
                }

                // 초기 선택 설정 (첫 번째 항목)
                if (dud_SerialNumber.Items.Count > 0)
                    dud_config.SelectedIndex = 0;

                dud_config.Wrap = true;  // 끝 → 처음 순환 허용

                LogText.Log("DO 제조정보 불러오기");
            }

            catch (Exception ex)
            {
                LogText.Log("에러 : 제조정보 불러오기" + ex.ToString());
                MessageBox.Show("에러 : 제조정보 불러오기" + ex.Message);
                // 또는 로그로 저장
            }

        }

        // 버튼 클릭 시 현재 선택된 항목 출력 예제
        //       private void button1_Click(object sender, EventArgs e)
        //       {
        //          MessageBox.Show($"선택된 항목: {domainUpDown1.Text}");
        //      }

        private void dud_config_SelectedItemChanged(object sender, EventArgs e)
        {
            string selected = dud_config.Text;

            // 매핑에 있으면 Fullname 표시, 없으면 그대로 표시
            if (fullnameMap.ContainsKey(selected))
                label1.Text = $"{fullnameMap[selected]}";
            else
                label1.Text = $"선택: {selected}";        // TBD, ???

            //this.Location = new Point(100, 80);

            MessageBox.Show($"선택된 항목: {label1.Text}");
        }

        // 프로그램 로드
        private void DFTM_Load(object sender, EventArgs e)
        {

            try
            {

                uartSerial.OnErrorMessage = ShowErrorMessage;

#if (false)
            //----프로세스 이름 확인---
            var proc = System.Diagnostics.Process.GetCurrentProcess();
            string msg = $"프로세스 이름: {proc.ProcessName}\n실행 경로: {proc.MainModule.FileName}";
            MessageBox.Show(msg, "프로세스 확인");
#endif

                //------------
                LogText.Log("DO 프로그램 실행>>>");

                //string txtPath = Path.Combine(Application.StartupPath, "MainConfig.txt");
                LoadItemsToDomainUpDown();
                // 선택 변경 시 자동 출력되도록 이벤트 연결
                //dud_config.SelectedItemChanged += dud_config_SelectedItemChanged;

                // 버전 로드
                VersionInfo Vi = new VersionInfo();
                this.Text = "DFTM  Ver : " + Vi.Version + Vi.getConfig() + "   " + Vi.BuildYYmmdd;
                sVersion = "DFTM_" + Vi.Version + Vi.getConfig();       // FULL Version SAVE

                // 시리얼 설정
                //uartSerial = new Serial();                                      // 신규 직렬 생성
                uartSerial.ReceiveMessage += UartSerial_ReceiveMessage;         // 핸들러 함수 할당
                uartSerial.SetLogBox(ref tb_LOG_Window);                        // 로그박스 할당

                // 타이머 설정
                timer = new System.Windows.Forms.Timer();
                timer.Interval = 500;  // 2초-> 500ms
                timer.Tick += new EventHandler(timer_Tick_500ms);

                // Tick & Cnt CLEAR
                iTmSystemTick500ms = 0;
                //iTmSystemTick500msCnt = 0;
                iTmSystemTick2secFlag = 0;

                iMainAliveReqSendCnt = 0;

                iPowerOnCmdSendCnt = 0;
                //iTmSystemTick500msCnt = 0;

                updown_Bobbin_Duty.Value = 20;
                Tb_BLDC_Duty.Value = 20;

                //updown_Bobbin_timems.Value = 0;
                //Tb_BLDC_OnTime.Value = 0;
                updown_Bobbin_timems.Value = 3000;
                Tb_BLDC_OnTime.Value = 3000;

                radio_BLDC_CW.Checked = true;
                radio_BLDC_CCW.Checked = false;

                iBLDC_Duty_Value = (int)updown_Bobbin_Duty.Value;
                iBLDC_OnTime_Value = (int)updown_Bobbin_timems.Value;

                numUpDown_LedPosNo.Value = 1;       // 중앙
                iLED_Pos_Number = (int)numUpDown_LedPosNo.Value;

                numUpDown_LedModeNo.Value = 3;      // 디밍
                iLED_Mode_Number = (int)numUpDown_LedModeNo.Value;

                numUpDown_LedColorNo.Value = 6;     // 6가지 색
                iLED_Color_Number = (int)numUpDown_LedColorNo.Value;
                //lblLedColor.ForeColor = Color.Red;
                lblLedColor.ForeColor = Color.White;
                lblLedColor.BackColor = Color.Magenta;

                numUpDown_LedRead.Value = 100;      // 밝기 100%
                iLED_Brightness_Number = 100;

                numUpDown_LedControlTime.Value = 2000;  // 2초
                iLED_Control_Time = (int)numUpDown_LedControlTime.Value;

                ///////////// LM Jig, REEL option //////////////////////////////////////////////
#if (false) //(LM_CMD_BTN)
            updown_LM_Duty.Value    =   13;
            tb_LM_Duty.Value        =   13;
            updown_LM_timems.Value  =   0;
            tb_LM_timems.Value      =   0;
            iLM_Duty_Value          =   (int)updown_LM_Duty.Value;
            iLM_OnTime_Value        =   (int)updown_LM_timems.Value;

            radio_LM_CW.Checked     =   true;
            radio_LM_CCW.Checked    =   false;
#endif

                //num_Model_person.Value = "1";
                //num_Model_Config.Value = "SF";
                //num_Model_Motor.Value = "3";
                //num_Model_fps.Value = "060";

                Init_tbText_byLoad();

                ////////////////////////////////////////////////////////////////////////////////
                Operation_Disable();
                // ENB Initail VAL
                // btn_REEL_motor.Enabled = true;


                // == COM Port CONFIG
                //rb_RTS.Enabled = true;
                //rb_RTS.Checked = true;
                //rb_DTR.Enabled = true;
                //rb_DTR.Checked = true;

                // 에이징 카운트 로드
                LoadAgingCount();

                LogText.Log("DO 폼 로드");
            }
            catch (Exception ex)
            {
                LogText.Log("에러 폼 로드: " + ex.ToString());
                MessageBox.Show("에러 폼 로드:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        // 메세지 처리
        private void ShowErrorMessage(string msg)
        {
            // UI 스레드인지 확인하고 안전하게 메시지 박스 띄움
            if (this.InvokeRequired)
            {
                this.Invoke(new Action(() => MessageBox.Show(msg)));
            }
            else
            {
                MessageBox.Show(msg);
            }
        }

        private void Init_tbText_byLoad()
        {
            tb_Manuf_Model.Text = "DF1SF300";

            tb_Manuf_No.Text = "DF" + DateTime.Now.ToString("yyMM") + "KSNL1";

            tb_Manuf_SerialNo.Text = DateTime.Now.ToString("yyMM");
            tb_Manuf_SerialNo.Text += "ikr";
            tb_Manuf_SerialNo.Text += "000010";

            tb_Manuf_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            tb_Install_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            tb_MainMot_Default.Text = mainMot_DefaultUnknown;

        }

        private void Operation_Disable()
        {
            btn_Torque_OFF.Enabled = false;
            //btn_Break_OFF.Enabled = false;
            btn_BLDC_On.Enabled = false;
            btn_BLDC_Off.Enabled = false;

            btn_LED_On.Enabled = false;
            btn_LED_Off.Enabled = false;

            Btn_IMU_On.Enabled = false;
            Btn_IMU_Off.Enabled = false;

            numUpDown_Torque.Enabled = false;
            Tb_Drag_TorqVal.Enabled = false;

            //numUpDown_Break.Enabled = false;
            //Tb_Break_Motor.Enabled = false;

            updown_Bobbin_Duty.Enabled = false;
            updown_Bobbin_timems.Enabled = false;

            Tb_BLDC_Duty.Enabled = false;
            Tb_BLDC_OnTime.Enabled = false;

            numUpDown_LedPosNo.Enabled = false;
            numUpDown_LedModeNo.Enabled = false;
            numUpDown_LedColorNo.Enabled = false;
            numUpDown_LedRead.Enabled = false;
            numUpDown_LedControlTime.Enabled = false;

            Btn_GameStart.Enabled = false;
            Btn_VersionRead.Enabled = false;

            btn_LED_Read.Enabled = false;
            btn_LED_Write.Enabled = false;

            Btn_Device_Check.Enabled = false;

#if (false) // LM_CMD_BTN

            updown_LM_Duty.Enabled  =   false;
            updown_LM_timems.Enabled=   false;
            tb_LM_Duty.Enabled      =   false;
            tb_LM_timems.Enabled    =   false;
            btn_LM_OFF.Enabled      =   false;
            btn_LM_ON.Enabled       =   false;
#endif
            btn_REEL_motor.Enabled = false;
            btn_REEL_LED_left.Enabled = false;
            btn_REEL_LED_right.Enabled = false;
            btn_REEL_ON.Enabled = false;
            btn_REEL_OFF.Enabled = false;

            LM_JIG_SetActivate(false);

            LogCommand_SetActivate(true);


            btn_Reel_Regist.Enabled = false;
            tb_ReelRegi_Result.Enabled = false;

            //iTmSystemTick500ms = 0;
            //iTmSystemTick500msCnt = 0;
            //iPowerOnCmdSendCnt = 0;         // 포트오픈후 송신CMD 카운트 초기화(재송신)


            // Bbn Mot INPUT Start/Stop BTN
            Btn_Bbn_In_Start.Enabled = false;
            Btn_Bbn_In_Stop.Enabled = true;

            // 제조 정보 Disable
            tb_Manuf_Model.Enabled = false;
            tb_Manuf_SerialNo.Enabled = false;
            tb_Manuf_Date.Enabled = false;

            btn_Manuf_Read.Enabled = false;
            btn_Manuf_Write.Enabled = false;
            btn_Manuf_Erase.Enabled = false;

            // 필드 설치 정보 Disable
            tb_Install_Date.Enabled = false;

            btn_Install_Read.Enabled = false;
            btn_Install_Write.Enabled = false;
            btn_Install_Delete.Enabled = false;

            tb_MainMot_Default.Enabled = false;
            btn_Mmot_Init.Enabled = true;
            btn_Mmot_Read.Enabled = false;
            btn_Mmot_Write.Enabled = false;

            //BbnEncA.Enabled = false;
            //BbnEncB.Enabled = false;

            tb_Bat_Charging.Enabled = false;

        }

        private void Clear_TextBox_byComClose()
        {
            tb_Manuf_Model.Text = "";
            tb_Manuf_SerialNo.Text = "";
            tb_Manuf_Date.Text = "";

            tb_Install_Date.Text = "";
        }

        private void Operation_Enable()
        {
            btn_Torque_OFF.Enabled = true;
            //btn_Break_OFF.Enabled = true;
            btn_BLDC_On.Enabled = true;
            btn_BLDC_Off.Enabled = true;

            btn_LED_On.Enabled = true;
            btn_LED_Off.Enabled = true;

            Btn_IMU_On.Enabled = true;
            Btn_IMU_Off.Enabled = true;

            numUpDown_Torque.Enabled = true;
            Tb_Drag_TorqVal.Enabled = true;

            //numUpDown_Break.Enabled = true;
            //Tb_Break_Motor.Enabled = true;

            updown_Bobbin_Duty.Enabled = true;
            updown_Bobbin_timems.Enabled = true;

            Tb_BLDC_Duty.Enabled = true;
            Tb_BLDC_OnTime.Enabled = true;

            numUpDown_LedPosNo.Enabled = true;
            numUpDown_LedModeNo.Enabled = true;
            numUpDown_LedColorNo.Enabled = true;
            numUpDown_LedRead.Enabled = true;
            numUpDown_LedControlTime.Enabled = true;

            Btn_GameStart.Enabled = true;
            Btn_VersionRead.Enabled = true;

            btn_LED_Read.Enabled = true;
            btn_LED_Write.Enabled = true;

            Btn_Device_Check.Enabled = true;

#if (false) // LM_CMD_BTN
            updown_LM_Duty.Enabled  =   true;
            updown_LM_timems.Enabled=   true;
            tb_LM_Duty.Enabled      =   true;
            tb_LM_timems.Enabled    =   true;
            btn_LM_OFF.Enabled      =   true;
            btn_LM_ON.Enabled       =   true;
#endif
            btn_REEL_motor.Enabled = true;
            btn_REEL_LED_left.Enabled = true;
            btn_REEL_LED_right.Enabled = true;
            btn_REEL_ON.Enabled = true;
            btn_REEL_OFF.Enabled = true;

            LM_JIG_SetActivate(true);

            LogCommand_SetActivate(true);

            btn_Reel_Regist.Enabled = true;
            tb_ReelRegi_Result.Enabled = true;

            // Bbn Mot INPUT Start/Stop BTN
            Btn_Bbn_In_Start.Enabled = true;
            Btn_Bbn_In_Stop.Enabled = true;

            //-- 제조정보,필드 정보 기본 금지.
            // 제조 정보 Enable
            tb_Manuf_Model.Enabled = false;
            tb_Manuf_No.Enabled = false;
            tb_Manuf_SerialNo.Enabled = false;
            tb_Manuf_Date.Enabled = false;

            btn_Manuf_Read.Enabled = false;
            btn_Manuf_Write.Enabled = false;     // 비밀번호 설정 추가 필요
            btn_Manuf_Erase.Enabled = false;    // 비밀번호 설정 추가 필요

            // 필드 설치 정보 Enable
            tb_Install_Date.Enabled = false;

            btn_Install_Read.Enabled = false;
            btn_Install_Write.Enabled = false;     // 비밀번호 설정 추가 필요
            btn_Install_Delete.Enabled = false;   // 비밀번호 설정 추가 필요


            if (false) { }  // dummy
            else if (Config.IsBoard)
            {
                // NA
            }

            else if (Config.IsManuFeildService)
            {
                // NA - 기능 DROP함.
            }
            /*
            else if (Config.IsField)
            {
               // 필드 설치 정보 Enable
               tb_Install_Date.Enabled = true;

               btn_Install_Read.Enabled = true;
               btn_Install_Write.Enabled = true;     // 비밀번호 설정 추가 필요
               btn_Install_Delete.Enabled = true;   // 비밀번호 설정 추가 필요
            }
            */
            else if (Config.IsDevelope)
            {
                tb_Manuf_Model.Enabled = true;
                tb_Manuf_SerialNo.Enabled = true;
                tb_Manuf_Date.Enabled = true;

                btn_Manuf_Read.Enabled = true;
                btn_Manuf_Write.Enabled = true;     // 비밀번호 설정 추가 필요
                btn_Manuf_Erase.Enabled = true;    // 비밀번호 설정 추가 필요

                // 필드 설치 정보 Enable
                tb_Install_Date.Enabled = true;

                btn_Install_Read.Enabled = true;
                btn_Install_Write.Enabled = true;     // 비밀번호 설정 추가 필요
                btn_Install_Delete.Enabled = true;   // 비밀번호 설정 추가 필요

            }
            else
            {
                // ERROR
            }

            // 메인모터 기본값 설정
            tb_MainMot_Default.Enabled = true;  // 텍스트박스, 메인모터 기본Duty값 ENB
            btn_Mmot_Init.Enabled = true;       // 버튼, 초기값 설정
            btn_Mmot_Read.Enabled = true;       // 버튼, 읽기
            btn_Mmot_Write.Enabled = true;      // 버튼, 쓰기

            tb_Bat_Charging.Enabled = true;

        }

        private void Init_TbText_byComOpen()
        {
            // 통신포트 Open시 텍스트 박스 초기 문자값
            tb_Manuf_Model.Text = "DF" + dud_person.Text + dud_config.Text + "***";
            tb_Manuf_SerialNo.Text = DateTime.Now.ToString("yyMM");
            tb_Manuf_SerialNo.Text += "ikr";
            tb_Manuf_SerialNo.Text += "NNNNNN";
            tb_Manuf_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            tb_Install_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            //tb_MainMot_Default.Text = mainMot_DefaultDuty;
            tb_MainMot_Default.Text = "--";

        }

        private void LogCommand_SetActivate(bool bActive)
        {

            btn_Custom_send1.Enabled = bActive;
            tb_CustomSend1.Enabled = bActive;
            btn_Custom_send2.Enabled = bActive;
            tb_CustomSend2.Enabled = bActive;
            btn_Custom_send3.Enabled = bActive;
            tb_CustomSend3.Enabled = bActive;
            btn_Custom_send4.Enabled = bActive;
            tb_CustomSend4.Enabled = bActive;


            //btn_log_autoscroll.Enabled	=	bActive;
            //btn_log_save.Enabled		=	bActive;
            //btn_log_clear.Enabled		=	bActive;
            //tb_LOG_Window.Enabled		=	bActive;
            btn_log_autoscroll.Enabled = true;
            btn_log_save.Enabled = true;
            btn_log_clear.Enabled = true;
            tb_LOG_Window.Enabled = true;
        }

        private void DFTM_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                // uartSerial.WriteMessage("$0100000000%");     // I/F삭제, Game Disable
                uartSerial.WriteMessage("$1199%");              // 프로그램 종료를 전송
                uartSerial.Close();
                MainComPortName = null;

				LogText.Log("DO 프로그램 종료||||");
            }
            catch (Exception ex)
            {
                LogText.Log("에러 폼종료: " + ex.ToString());
                MessageBox.Show("에러  폼종료: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Cb_Ports_DropDown(object sender, EventArgs e)
        {
            try
            {
                ComboBox cb = (ComboBox)sender;
            //   cb.Items.Clear();
            //   cb.Items.AddRange(SerialPort.GetPortNames().Distinct().ToArray());
                ComboBoxSerialPort(cb);

				// Port가 변경이 되면 일단 Open되어 있는 Port는 Close시킨다.
				// [문제] 동일한 포트번호일때도 close/open이 진행되므로 불필요한 오류 발생여지가 있음.
				uartSerial.Close();
                MainComPortName = null;

				// Btn_Port_Open.Text = "Open" + Environment.NewLine;
				// Btn_Port_Open.BackColor = Color.Gainsboro;
				// Btn_Port_Open.BackColor = SystemColors.Control;
				// Btn_Port_Open.FlatStyle = FlatStyle.Standard;

				Lab_Main_Conn.BackColor = Color.Gray;

                Tb_Drag_TorqVal.Value = 70;
                //Tb_Break_Motor.Value = 0;
                Tb_BLDC_Duty.Value = 20;
                Tb_BLDC_OnTime.Value = 3000;

                numUpDown_Torque.Value = 70;
                //numUpDown_Break.Value = 0;
                updown_Bobbin_Duty.Value = 20;
                updown_Bobbin_timems.Value = 3000;

                iMainAliveReqSendCnt = 0;
                iPowerOnCmdSendCnt = 0;
                //iTmSystemTick500msCnt = 0;

                nud_TM_together.Value = 70;         // TORQ DUTY 70
                nud_BM_together.Value = 10;         // BLDC DUTY 10
                nud_time_together.Value = 0;     // ON TIME 0 [ms]

                LogText.Log("DO 포트번호 드롭다운");
            }
            catch (Exception ex)
            {
                LogText.Log("에러 : 포트번호 드롭다운" + ex.ToString());
                MessageBox.Show("에러 : 포트번호 드롭다운" + ex.Message);
                // 또는 로그로 저장
            }
        }

		/*
           private void Tb_Break_Motor_KeyUp(object sender, KeyEventArgs e)
           {
               BreakCtrl();
           }

           private void Tb_Break_Motor_MouseUp(object sender, MouseEventArgs e)
           {
               BreakCtrl();
           }

           private void numUpDown_Break_ValueChanged(object sender, EventArgs e)
           {
               Tb_Break_Motor.Value = (int)numUpDown_Break.Value;
               // Key Up이나 Mouse Up으로 값이 변경되면, numUpDown도 값이 변경되어
               // 결과적으로 두번 데이터가 전송되어진다. (두번 송신에도 문제없으므로 한 번 송신 변경 대응하지 않는다.)
               BreakCtrl();
           }

           private void Btn_Break_Off_Click(object sender, EventArgs e)
           {
               Tb_Break_Motor.Value = 0;
               BreakCtrl();
           }

           private void BreakCtrl()
           {
               // string CtrlStr = Tb_Break_Motor.Value.ToString("000");
               string CtrlStr = "";
               iBreak_Control_Value = Tb_Break_Motor.Value;
               numUpDown_Break.Value = iBreak_Control_Value;
               CtrlStr = iBreak_Control_Value.ToString("000");

               // Lab_Break_Motor.Text = CtrlStr;
               uartSerial.WriteMessage("$0500000" + CtrlStr + "%");
           }
        */

#if (false)
        private void InitializeSerialPortUI()
        {
            try
            {
                // 사용 가능한 COM 포트를 콤보박스에 추가
                string[] ports = SerialPort.GetPortNames();
                Cb_Ports.Items.AddRange(ports);

                // 기본 포트를 COM_5 => COM_4 로 설정 (목록에 있을 경우만)
                if (Array.Exists(ports, p => p.Equals("COM4", StringComparison.OrdinalIgnoreCase)))
                {
                    Cb_Ports.SelectedItem = "COM4";
                }
                //else if (ports.Length > 0)
                //{
                //    Cb_Ports.SelectedIndex = 0; // 기본으로 첫 포트 선택
                //}
                LogText.Log("DO COM포트 리스트 취득");
            }

            catch (Exception ex)
            {
                LogText.Log("에러 COM포트 리스트 취득: " + ex.ToString());
                MessageBox.Show("에러 COM포트 리스트 취득: " + ex.Message);
                // 또는 로그로 저장
            }
        }
#else
        private void InitializeSerialPortUI()
        {
            ComboBoxSerialPort(Cb_Ports);
		}
		private void ComboBoxSerialPort(ComboBox cb)
		{
			cb.Items.Clear();

			foreach (var port in FindEsp32S3CdcPorts())
			{
				// 표시용: COM번호만
				cb.Items.Add(port.ComPort);
			}
			// 🔽 COM4 우선 선택
			int idx = cb.Items.IndexOf("COM4");
			if (idx >= 0)
				cb.SelectedIndex = idx;
			else if (cb.Items.Count > 0)
				cb.SelectedIndex = 0;
		}
		List<UsbComPort> FindEsp32S3CdcPorts()
		{
			var result = new List<UsbComPort>();

			var searcher = new ManagementObjectSearcher(
				"SELECT * FROM Win32_PnPEntity WHERE Name LIKE '%(COM%'"
			);

			foreach (ManagementObject device in searcher.Get())
			{
				string deviceId = device["DeviceID"]?.ToString();
				string name = device["Name"]?.ToString();

				if (deviceId == null || name == null)
					continue;

				if (!deviceId.Contains($"VID_{VID}") ||
					!deviceId.Contains($"PID_{PID}"))
					continue;

				var match = Regex.Match(name, @"\(COM(\d+)\)");
				if (!match.Success)
					continue;

				result.Add(new UsbComPort
				{
					ComPort = "COM" + match.Groups[1].Value,
					DeviceId = deviceId
				});
			}

			return result;
		}
#endif

		private void Tb_Torque_Val_KeyUp(object sender, KeyEventArgs e)
        {
            try
            {
                // [ON]버튼 클릭 혹은 Enter의 Keyup처리를 사용한다.
                // TorqueCtrl((int)numUpDown_Torque.Value);
                if (e.KeyCode == Keys.Enter)
                {
                    TorqueCtrl((int)numUpDown_Torque.Value);
                }
            }
            catch (Exception ex)
            {
                LogText.Log("에러 토크값 키업:" + ex.ToString());
                MessageBox.Show("에러 토크값 키업:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Tb_Torque_Val_MouseUp(object sender, MouseEventArgs e)
        {
            try
            {
                numUpDown_Torque.Value = Tb_Drag_TorqVal.Value;
                TorqueCtrl((int)numUpDown_Torque.Value);
            }

            catch (Exception ex)
            {
                LogText.Log("에러 토크값 마우스업:" + ex.ToString());
                MessageBox.Show("에러 토크값 마우스업:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void numUpDown_Torque_ValueChanged(object sender, EventArgs e)
        {
            try
            {
                // Key Up이나 Mouse Up으로 토크값이 변경되면, numUpDown도 값이 변경되어
                // 결과적으로 두번 데이터가 전송되어진다.

                // [ON]버튼 클릭 혹은 Enter의 Keyup처리를 사용하므로
                // ==== 아래는 삭제
                TorqueCtrl((int)numUpDown_Torque.Value);
                //if (e.KeyCode == Keys.Enter)
                //{
                //    TorqueCtrl((int)numUpDown_Torque.Value);
                //}
            }

            catch (Exception ex)
            {
                LogText.Log("에러 토크값 변경:" + ex.ToString());
                MessageBox.Show("에러 토크값 변경:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Btn_Torque_Off_Click(object sender, EventArgs e)
        {
            try
            {
                //TorqueCtrl(0);
                //CtrlStr = "000";
                uartSerial.WriteMessage("$02" + "000" + "%");
            }

            catch (Exception ex)
            {
                LogText.Log("에러 토크OFF 버튼: " + ex.ToString());
                MessageBox.Show("에러 토크OFF 버튼: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void btn_Torue_On_Click(object sender, EventArgs e)
        {
            // OFF TIme CLEAR
            tb_Torq_Off_time.Text = "--";          // Te:xxxx

            try
            {
                TorqueCtrl((int)numUpDown_Torque.Value);
            }

            catch (Exception ex)
            {
                LogText.Log("에러 토크ON 버튼: " + ex.ToString());
                MessageBox.Show("에러 토크ON 버튼: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        // 토크 출력 처리
        private void TorqueCtrl(int iOut_val)
        {
            try
            {
                // string CtrlStr = Tb_Torque_Val.Value.ToString("000");
                string CtrlStr = "";

                if (false) { }  // Dummy
                else if (0 == iOut_val) { } // NA
                else if (10 > iOut_val) iOut_val = 10;
                else if (255 < iOut_val) iOut_val = 255;

                if (0 == iOut_val)       // OFF CLICK
                {
                    // 값 저장 안함(텍스트 박스 값 , 갱신안함)
                    CtrlStr = iOut_val.ToString("000");    // 정수 3자리 변환(0채우기)
                }
                else
                {
                    // 값 저장(텍스트 박스 값 , 갱신)
                    Tb_Drag_TorqVal.Value = iOut_val;
                    numUpDown_Torque.Value = iOut_val;
                    iTorque_Control_Out_Value = iOut_val;
                    CtrlStr = iTorque_Control_Out_Value.ToString("000");    // 정수 3자리 변환(0채우기)
                }

                // 송신
                uartSerial.WriteMessage("$02" + CtrlStr + "%");
            }

            catch (Exception ex)
            {
                LogText.Log("에러 토크명령 전송: " + ex.ToString());
                MessageBox.Show("에러 토크명령 전송: " + ex.Message);
                // 또는 로그로 저장
            }
        }


        // 서보모터 컨트롤은 삭제됨
        private void Tb_Servo_Motor_KeyUp(object sender, KeyEventArgs e)
        {
            ServoCtrl();
        }
        private void Tb_Servo_Motor_MouseUp(object sender, MouseEventArgs e)
        {
            ServoCtrl();
        }

        private void ServoCtrl()
        {
            /*
            int val = Tb_BLDC_Motor.Value;
            string CtrlStr;
            if (val > 0)
            {
                CtrlStr = "+" + val.ToString("00");
            }
            else if (val < 0)
            {
                CtrlStr = val.ToString("00");
            }
            else
            {
                CtrlStr = val.ToString("000");
            }
            // Lab_BLDC_Motor.Text = CtrlStr;
            uartSerial.WriteMessage("$0300000" + CtrlStr);
            */
        }

        // COM포트 오픈 버튼
        private async void Btn_Port_Open_Click(object sender, EventArgs e)
        {
            try
            {
                // 일단 열려있는 포트를 Close한다.
                if (uartSerial.IsOpen)
                {
                    uartSerial.Close();
                    MainComPortName = null;
				}
                // Btn_Port_Open.Text = "Open" + Environment.NewLine;
                // Btn_Port_Open.BackColor = Color.Gainsboro;
                // Btn_Port_Open.BackColor = Color.White;

                if (Cb_Ports.SelectedIndex < 0)
                {
                    MessageBox.Show("올바른 Port번호를 선택하세요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    return;
                }

                string portNum = Cb_Ports.SelectedItem.ToString();
                if (uartSerial.IsOpen || (0 == portNum.Length))
                {
                    MessageBox.Show("올바른 Port번호를 선택하세요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    return;
                }

                try
                {
                    // 시리얼 오픈
                    bool result = await uartSerial.Open(portNum, 115200, cb_RTS.Checked, cb_DTR.Checked);
                    if (uartSerial.IsOpen)
                    {
                        // Btn_Port_Open.Text = portNum + " " + "Opened" + Environment.NewLine;
                        // Btn_Port_Open.BackColor = Color.LawnGreen;
                        Lab_Main_Conn.BackColor = Color.LawnGreen;
                        timer.Start();

                        Operation_Enable();         // 각 버튼등 ENB
                        Init_TbText_byComOpen();    // 텍스트박스 초기값 설정

                        iPowerOnCmdSendCnt = 0;     // PowerOn Cmd재송신 SET

                        // ver 표시 CLR
                        Textbox_Main_Ver.Text = "---";
                        Textbox_Rod_Ver.Text = "---";
                        Textbox_Imu_Ver.Text = "---";

                        Textbox_Error_Code.Text = "---";
                        TextBox_Error_Content.Text = "---";

                        tb_MainBoardVer.Text = "---";
                        MainComPortName = portNum;
					}
                    else
                    {
                        // Btn_Port_Open.Text = portNum + " " + "Open Failed" + Environment.NewLine;
                        // Btn_Port_Open.BackColor = Color.Red;
                        MessageBox.Show("OPEN 실패, 다른 프로그램이 쓰는지 확인해주세요", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                    }
                }
                catch (Exception ex)
                {
                    LogText.Log("에러 COM포트 오픈:" + ex.ToString());
                    MessageBox.Show("에러 COM포트 오픈:" + ex.Message);
                    // 또는 로그로 저장
                }
            }
            catch (Exception ex)
            {
                LogText.Log("에러 COM포트 클로즈:" + ex.ToString());
                MessageBox.Show("에러 COM포트 클로즈:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        // 500ms마다 주기 처리
        void timer_Tick_500ms(object sender, EventArgs e)
        {
            try
            {
                // 통신포트 살아 있어?
                if (uartSerial.IsOpen)    // PORT OPEN?
                {
                    // Console.WriteLine("Timer Timeout");
                    iTmSystemTick500ms++;
                //    if (4 <= iTmSystemTick500ms)    // 2초마다 통신 처리
					if ((Poll_Interval != 0) && ((Poll_Interval*2) <= iTmSystemTick500ms))    // 2초마다 통신 처리
					{
						iTmSystemTick500ms = 0;
                        iTmSystemTick2secFlag = 1;      // 2초 FLAG SET
                    }

                    // 초기 PowerOn 500ms마다 자동 CMD송신+ Alive포함 (3번)
                    if (3 > iPowerOnCmdSendCnt) // 0 ~ 2(3번)
                    {
                        SendCmd_PowerOn(iPowerOnCmdSendCnt);    // 500ms, PowerOn_CMD + Main Alive Request
                        iPowerOnCmdSendCnt++;
                    }
                    // 초기 PowerOn 자동 CMD송신 이후, 2초마다 Alive송신
                    else if (1 == iTmSystemTick2secFlag)  // 초기 500ms간격 종료되면, 2초마다 송신.
                    {
                        iTmSystemTick2secFlag = 0;
                        iMainAliveReqSendCnt++;     // 2SEC Cnt
                        uartSerial.WriteMessage("$00%");        // 2SEC, Main Alive Request
                    }

                    // 연속 20번 응답 없는 경우, 2*20번 = 40초 => 20초로 변경
                    if (iCONTMAIN_DISCONN_TIME < iMainAliveReqSendCnt)
                    {
                        Lab_Main_Conn.BackColor = Color.Gray;
                        Textbox_Main_Ver.Text = "---";
                        Textbox_Rod_Ver.Text = "---";
                        Textbox_Imu_Ver.Text = "---";

                    }
                }
                // 통신포트 죽었으면
                else
                {
                    iMainAliveReqSendCnt = 0;

                    Lab_Main_Conn.BackColor = Color.Gray;

                    //Textbox_Main_Ver.Text = "";
                    //Textbox_Rod_Ver.Text = "";
                    //Textbox_Imu_Ver.Text = "";

                    // == 통신 재연결 처리는? **
                    //string portNum = Cb_Ports.SelectedItem.ToString();
                    //uartSerial.Open(portNum, 115200);

                }

            }
            catch (Exception ex)
            {
                LogText.Log("에러 타임주기:" + ex.ToString());
                MessageBox.Show("에러 타임주기:" + ex.Message);
                // 또는 로그로 저장
            }
        }

#if (false)
        void timer_Tick_2Sec(object sender, EventArgs e)
        {
            if (uartSerial.IsOpen)    // PORT OPEN?
            {
                // Console.WriteLine("Timer Timeout");
                if(3 > iPowerOnCmdSendCnt)     // 프로그램실행 ~ 10회(10초)
                {
                    SendCmd_PowerOn(iPowerOnCmdSendCnt);    // Main Alive Request
                    iPowerOnCmdSendCnt++;
                }
                else
                {
                     uartSerial.WriteMessage("$00%");              // Main Alive Request
                }
                iMainAliveReqSendCnt++;
                if (20 < iMainAliveReqSendCnt)
                {
                    Lab_Main_Conn.BackColor = Color.Gray;
                    Textbox_Main_Ver.Text = "";
                    Textbox_Rod_Ver.Text = "";
                    Textbox_Imu_Ver.Text = "";

                }
            }
            else
            {
                iMainAliveReqSendCnt = 0;

                Lab_Main_Conn.BackColor = Color.Gray;
                Textbox_Main_Ver.Text = "";
                Textbox_Rod_Ver.Text = "";
                Textbox_Imu_Ver.Text = "";

                //string portNum = Cb_Ports.SelectedItem.ToString();
                //uartSerial.Open(portNum, 115200);
            }

        }
#endif
        // 프로그램 실행후. COM포트 Open후 TM송신CMD송신 처리
        // 시간 간격 500ms간격 * 3번
        private void SendCmd_PowerOn(short cnt)
        {
            try
            {
                // 자동 송신
                iMainAliveReqSendCnt++;
                uartSerial.WriteMessage("$00%");   // Main Alive Request
#if (true)
                if (0 == cnt)
                {
                    uartSerial.WriteMessage("$290101%");   // TM통지
                    uartSerial.WriteMessage("$1101%");     // 게임시작 통지
                }
                else if (1 == cnt)
                {
                    uartSerial.WriteMessage("$10" + sVersion + "%");       // Ver읽기   // sVersion
                    if (Config.IsDevelope)
                    {
                        uartSerial.WriteMessage("$081%");     // IMU Data ON
                    }
                }
                else if (2 == cnt)
                {
                    uartSerial.WriteMessage("$0112%");       // 송부 : 메인보드 VER
                    uartSerial.WriteMessage("$0110%");       // 송부 : 메인모터 기본Duty값 읽기
                    uartSerial.WriteMessage("$1501%");       // 송부 : Device Check
                }
#endif
            }

            catch (Exception ex)
            {
                LogText.Log("예러 파워온 송신: " + cnt.ToString() + "," + ex.ToString());
                MessageBox.Show("에러 파워온 송신: " + cnt.ToString() + "," + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Btn_Port_Close_Click(object sender, EventArgs e)
        {
            try
            {
                uartSerial.Close();
                MainComPortName = null;

				Lab_Main_Conn.BackColor = Color.Gray;

                Operation_Disable();
                Clear_TextBox_byComClose();
            }

            catch (Exception ex)
            {
                LogText.Log("예외 포트클로즈 버튼: " + ex.ToString());
                MessageBox.Show("에러 포트클로즈 버튼: " + ex.Message);
                // 또는 로그로 저장
            }

        }

        private void Btn_GameStart_Click(object sender, EventArgs e)
        {
            try
            {
                // uartSerial.WriteMessage("$0111111111%");
                // 버튼 기능 변경 Enable --> Program Start
                uartSerial.WriteMessage("$1101%");
                //uartSerial.WriteMessage("$10%");
            }

            catch (Exception ex)
            {
                LogText.Log("예외 게임시작 버튼: " + ex.ToString());
                MessageBox.Show("에러 게임시작 버튼: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Btn_VersionRead_Click(object sender, EventArgs e)
        {
            try
            {
                // uartSerial.WriteMessage("$0100000000%");
                // Lab_Main_Conn.BackColor = Color.Gray;
                // 버튼 기능 변경 Enable --> Program End --> Version Read
                uartSerial.WriteMessage("$10" + sVersion + "%");       // Ver읽기   // sVersion
                /*
                // IMU와의 연결을 끊음.
                uartSerial.WriteMessage("$080%");
                //Lab_IMU_Conn.BackColor = Color.Gray;

                Tb_Roll.Text = "";
                Tb_Pitch.Text = "";
                Tb_Yaw.Text = "";
                Tb_Ax.Text = "";
                Tb_Ay.Text = "";
                Tb_Az.Text = "";
                */
            }

            catch (Exception ex)
            {
                LogText.Log("에러 버전읽기 버튼: " + ex.ToString());
                MessageBox.Show("에러 버전읽기 버튼:" + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void Btn_Device_Check_Click(object sender, EventArgs e)
        {
            try
            {
                // 에러코드,결과 CLR
                Textbox_Error_Code.Text = "";
                TextBox_Error_Content.Text = "";

                // 장치확인 CMD 요구
                string CtrlStr = "";
                CtrlStr = "$1501%";
                uartSerial.WriteMessage(CtrlStr);
            }

            catch (Exception ex)
            {
                LogText.Log("에러 장치확인 버튼: " + ex.ToString());
                MessageBox.Show("에러 장치확인 버튼: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        // 수신데이타 CB처리
        private void UartSerial_ReceiveMessage(string msgAll)
        {
            String msg;
            try
            {
                //invoke처리 있음!!!
                this.BeginInvoke(new Action(() =>
                {
                    msg = msgAll.Substring(0, msgAll.Length - 1);     // latt "% Delete
                    if (msg.StartsWith("$") || msg.StartsWith("ENCINT"))
                    {
                        anaRecvMsg(msg);
                    }
                }));
            }
            catch (Exception ex)
            {
                LogText.Log("에러 수신메시지 CB처리: " + ex.ToString());
                MessageBox.Show("에러 수신메시지 CB처리: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        private void anaRecvMsg(String msg)
        {
            // 명령별 분기 처리 심플하게 개선 필요!!!
            if ((ReceiveDnMessage != null) && msg.StartsWith("$DN"))

			{
                ReceiveDnMessage(msg);
			}
            else if (msg.StartsWith("$01"))     // Device Info
            {
                if (false) { }   // dummy

                else if ("03" == msg.Substring(3, 2))  // IMU Interval [ms]
                {
                    String strmsg = msg.Substring(5, 4); //[0000 ~ 1000]
                    int intval = (int.Parse(strmsg)); //[0000 ~ 1000]
                    Lab_ImuInterval.Text = intval.ToString();
                }
                else if ("04" == msg.Substring(3, 2))   //ManuFact Info READ
                {
                    strMsg = msg.Substring(5);
                    string[] parts = strMsg.Split(',');
                    // 데이타 4개, 모델,제번,SN, Date
                    if (parts.Length >= 4)
                    {
                        tb_Manuf_Model.Text = parts[0];
                        tb_Manuf_No.Text = parts[1];
                        tb_Manuf_SerialNo.Text = parts[2];
                        if ("999999" != parts[3])
                        {
                            tb_Manuf_Date.Text = parts[3];
                        }
                        //Console.WriteLine($"1: {part1}");
                        //Console.WriteLine($"2: {part2}");
                        //Console.WriteLine($"3: {part3}");
                    }
                    else
                    {
                        Console.WriteLine("문자열이 4개 미만입니다.");
                    }
                }
                else if ("07" == msg.Substring(3, 2))   // Field Install Info READ
                {
                    tb_Install_Date.Text = msg.Substring(5);
                }

                else if ("10" == msg.Substring(3, 2) || "11" == msg.Substring(3, 2))   // MAIN MOT Info READ / WRITE
                {
                    tb_MainMot_Default.Text = msg.Substring(5);     // FW응답값 TB에 표시.
                }

                else if ("12" == msg.Substring(3, 2))   // MAIN 보드 버전
                {
                    tb_MainBoardVer.Text = msg.Substring(5);     // FW응답값 TB에 표시.
                }

            }

            else if (msg.StartsWith("$02"))     // 수신 FW , 메인모터 출력값.
            {
                iDutyVal = int.Parse(msg.Substring(3, 3));
                if (0 != iDutyVal)
                {
                    //numUpDown_Torque.Value = iDutyVal;
                }
            }
            else if (msg.StartsWith("$04"))     // 수신 FW , BBN BLDC 출력값.
            {
                iDutyVal = int.Parse(msg.Substring(4, 3));
                if (0 != iDutyVal)
                {
                    //updown_Bobbin_Duty.Value = iDutyVal;
                }
            }

            else if (msg.StartsWith("$09"))
            {
                ShowIMUData(msg);
                if (Lab_IMU_Conn.BackColor != Color.LawnGreen)
                {
                    Lab_IMU_Conn.BackColor = Color.LawnGreen;
                }
                if (Lab_Slave_Conn.BackColor != Color.LawnGreen)
                {
                    Lab_Slave_Conn.BackColor = Color.LawnGreen;
                }
            }
            else if (msg.StartsWith("$10"))
            {
                // string msgVer = msg.Substring(msg.IndexOf("%") + 1);
                string msgVer = msg.Substring(3);
                msgVer = msgVer.Replace("%", "");
                string[] strVer = msgVer.Split(',');
                if (strVer.Length >= 1)
                    Textbox_Main_Ver.Text = strVer[0];
				if (strVer.Length >= 2)
					Textbox_Rod_Ver.Text = strVer[1];
				if (strVer.Length >= 3)
					Textbox_Imu_Ver.Text = strVer[2];
			}
            else if (msg.StartsWith("$15"))
            {
                string strErrorCode1 = msg.Substring(3, 2);
                string strErrorCode2 = msg.Substring(5, 2);
                // 1) Error Code
                //Textbox_Error_Code.Text = strErrorCode1 + " - " + strErrorCode2;
                Textbox_Error_Code.Text = strErrorCode1 + strErrorCode2;

                // 2) Error Text
                string strErrorCode = strErrorCode1 + strErrorCode2;
                string strErrorText = "OK";
                //int errCode = strErrorCode.Substring(1).Toint();
                int errCode = (int.Parse(strErrorCode));
                if (false) { }
                else if (0 == errCode) { strErrorText = "OK"; }
                else if (6010 == errCode) { strErrorText = "ERR BBN Mot"; }
                else if (6011 == errCode) { strErrorText = "ERR TORQ Mot"; }
                else if (6020 == errCode) { strErrorText = "ERR Enc A"; }
                else if (6021 == errCode) { strErrorText = "ERR Enc B"; }
                else if (6030 == errCode) { strErrorText = "ERR ROD DisCon"; }

                else if (6040 == errCode) { strErrorText = "ERR FileSystem"; }
                else if (6041 == errCode) { strErrorText = "ERR BLDC24V PWR"; }
                else if (6050 == errCode) { strErrorText = "ERR MMOT MinSet"; }

                else if (7020 == errCode) { strErrorText = "WAN BATT LOW"; }            // Warning
                else if (7030 == errCode) { strErrorText = "ERR IMU DisCon"; }

                else { strErrorText = "ERR NOT DEFINE"; }

                TextBox_Error_Content.Text = strErrorText;

            }
            else if (msg.StartsWith("$14"))             // $07 --> $14로 변경
            {
                msg = msg.Substring(3, 3);
                Lab_Rotate_Cnt.Text = msg;
            }
            else if (msg.StartsWith("$13"))             // $06 --> $13로 변경
            {
                SetSwitchState(msg);
            }
            else if (msg.StartsWith("$20"))             // $04 --> $20으로 변경 (Rod Connect Status)
            {
                /*
                int isConn = msg[3];
                if(isConn == 48)
                {
                    Lab_Slave_Conn.BackColor = Color.Gray;
                }
                else
                {
                    msg = msg.Substring(8, 3);
                    Lab_Battery.Text = msg;
                    Lab_Slave_Conn.BackColor = Color.LawnGreen;
                }
                */
                //int rodConnectState = int.Parse(msg.Substring(3));
                String rodConnectState = msg.Substring(3, 2);
                //rodConnectState = msg.Substring(3, 2);
                if ("00" == rodConnectState)                            // 11 --> 01 연결됨
                {                   // 연결 끊김
                    Lab_Slave_Conn.BackColor = Color.Gray;
                    //Lab_IMU_Conn.BackColor = Color.Gray;
                    Textbox_Rod_Ver.Text = "";
                    Textbox_Imu_Ver.Text = "";
                }
                else
                {                   // 연결됨
                    Lab_Slave_Conn.BackColor = Color.LawnGreen;
                    //uartSerial.WriteMessage("$10%");  // Ver Read

                    // 릴등록 결과값 CLR
                    tb_ReelRegi_Result.Text = "";
                    tb_ReelRegi_Result.BackColor = Color.White;
                    // 버전 읽기 추가
                    uartSerial.WriteMessage("$10" + sVersion + "%");       // Ver읽기   // sVersion
                }
            }
            else if (msg.StartsWith("$21"))                 // IMU CONNECTION
            {
                //int imuConnectState = int.Parse(msg.Substring(3));
                String imuConnectState = msg.Substring(3, 2);
                if ("00" == imuConnectState)    // 연결끊김
                                                //if (0 == imuConnectState)     // 연결끊김
                {
                    // Lab_Slave_Conn.BackColor = Color.Gray;
                    Lab_IMU_Conn.BackColor = Color.Gray;
                    Textbox_Imu_Ver.Text = "";
                }
                else
                {                           // 연결
                                            //Lab_Slave_Conn.BackColor = Color.LawnGreen;
                    Lab_IMU_Conn.BackColor = Color.LawnGreen;
                    //uartSerial.WriteMessage("$10%");  // Ver Read
                }

            }
            else if (msg.StartsWith("$12"))             // $0C --> $12로 변경
            {
                //Lab_Battery.Text = msg.Substring(3, 2);

                // 값 판단 추가
                batLevelCharging = int.Parse(msg.Substring(3, 2));
                if (0 < batLevelCharging)
                {
                    Lab_Battery.Text = msg.Substring(3, 2);
                }
                else if (-1 == batLevelCharging || -2 == batLevelCharging)
                {
                    Lab_Battery.Text = "--";
                }
                else if (-3 == batLevelCharging)
                {
                    tb_Bat_Charging.Text = "";
                    tb_Bat_Charging.BackColor = Color.LightGray;
                }
                else if (-4 == batLevelCharging)
                {
                    tb_Bat_Charging.Text = "충전";
                    tb_Bat_Charging.BackColor = Color.Red;
                }
                else if (-5 == batLevelCharging)
                {
                    tb_Bat_Charging.Text = "사용";
                    tb_Bat_Charging.BackColor = Color.LightGreen;
                }

            }
            else if (msg.StartsWith("$00"))
            {
                iMainAliveReqSendCnt = 0;       // 보드 응답 확인: 응답시 끊김 CNT 클리어
                Lab_Main_Conn.BackColor = Color.LawnGreen;
            }
            else if (msg.StartsWith("$0608"))   // LED밝기 READ
            {
                //if (8 == Int32.Parse(msg.Substring(4, 1)))
                //if (8 == Int32.Parse(msg.Substring(3,2) ) )
                {
                    string strReadValue;
                    if (7 < msg.Length)
                    {
                        strReadValue = msg.Substring(5, 3);
                    }
                    else
                    {
                        strReadValue = msg.Substring(5, 2);
                    }
                    numUpDown_LedRead.Value = (int)Int32.Parse(strReadValue);
                }
            }

            // 엔코더 카운트\
            /*
            else if (msg.StartsWith("$xx"))             // Device Function Response
            {
                lb_MainEncCount.Text = "--";
            }
            */

            //
            else if (msg.StartsWith("$98"))             // Device Function Response
            {
                int iKind = Int32.Parse(msg.Substring(3, 2));
                switch (iKind)
                {
                    case 0:
                        break;

                    case 1:     // kk=01, Wire Aging Function 응답처리
                        anaWireAging_Response(msg);
                        break;

                    default:
                        break;
                }

            }

            else if (msg.StartsWith("$99")) // LM JIG 응답형태
            {
                int lv = Int32.Parse(msg.Substring(3, 1));
                int pow = Int32.Parse(msg.Substring(4, 1)) - 1;
                bool bOK = (Int32.Parse(msg.Substring(5, 1))) == 0;

                SetLMValue_Response(lv, pow, bOK);
            }

            else if (msg.StartsWith("$27"))             // $27  - 단품(Diag IN)
            {
                SetInputState(msg);
            }

            else if (msg.StartsWith("$30"))             //$3005 릴 등록 완료.
            {
                AnaReelRegistMsg(msg);
            }
            // 설정 응답
            else if (msg.StartsWith("$26"))             // $26  - 설정 결과 응답
            {
                AnaDevSet_RespResult(msg);
            }

            else if (msg.StartsWith("ENCINT"))
            {
                ShowIMUData(msg);       // TORQ & BLDC MOT Off ~ Enc STOP [TIME]
            }

        }

        // 낚시줄 에이징 응답처리
        private void anaWireAging_Response(string msg)
        {
            int iAct = Int32.Parse(msg.Substring(5, 1));
            switch (iAct)
            {
                case 0:     // 정지
                case 1:     // 시작
                    int rts = Int32.Parse(msg.Substring(6));
                    tb_WireAgingResult.Text = (0 == rts) ? "OK:00" : ("NG:" + msg.Substring(6));
                    if (0 == rts)
                    {
                        tb_WireAgingResult.BackColor = Color.LightGreen;
                    }
                    else
                    {
                        tb_WireAgingResult.BackColor = Color.Yellow;
                    }
                    //tb_WireAgingResult.Text = msg.Substring(6);
                    break;

                case 9:     // 카운트 업
                    //nud_AgingCounter.Value = Int32.Parse(msg.Substring(6));
                    int cnt = Int32.Parse(msg.Substring(6));
                    nud_AgingCounter.Value += cnt;
                    nud_AgingCounter.BackColor = Color.LightSkyBlue;

                    // 카운트업마다 파일에 저장
                    writeAgingCountUp(agingCntFileName);

                    break;

                default:
                    break;
            }
        }

        private void ShowIMUData(string data)
        {
            //data = data.Substring(3, (data.Length - 4));
            string[] dataList = data.Split(',');
            if (4 <= dataList.Length)
            {
                //.Text = dataList[0];         // ENCINT:
                //.Text = dataList[1];        // CW_, CCW
                tb_Torq_Off_time.Text = dataList[2].Substring(4);          // Te:xxxx
                tb_Bldc_Off_time.Text = dataList[3].Substring(4);           // Be:xxxx
              }
        }

        private void ShowEncStopTime(string data)
        {
            data = data.Substring(3, (data.Length - 4));
            string[] dataList = data.Split(',');
            if (dataList.Length == 6)
            {
                Tb_Roll.Text = dataList[0];
                Tb_Pitch.Text = dataList[1];
                Tb_Yaw.Text = dataList[2];
                Tb_Ax.Text = dataList[3];
                Tb_Ay.Text = dataList[4];
                Tb_Az.Text = dataList[5];
            }
        }
        // 단품 센서 변화 정보 응답 처리
        private void SetInputState(string msg)
        {

            //$27xx-yy,V~V%
            // == 1) 데이타 처리
            String strCode = msg.Substring(3, 5);                // cc-ii,
            int idir = 0;
            int ival = 0;
            if (strCode.Equals("03-03"))
            {
                idir = int.Parse(msg.Substring(9, 1));
            }
            else
            {
                ival = int.Parse(msg.Substring(9));     // 9 이후 Val
            }
            //int val = int.Parse(data.Substring(8));     // 8 이후 Val, 프로그램 행업

            // == 2) 분기처리
            if (false) { }
            else if (strCode.Equals("01-02"))
            {
                if (0 == ival) { Cont_Boot_Sw.BackColor = Color.LightYellow; }
                else { Cont_Boot_Sw.BackColor = Color.Red; }
            }
            else if (strCode.Equals("03-01"))
            {
                if (0 == ival) { BbnEncA.BackColor = Color.LightYellow; }
                else { BbnEncA.BackColor = Color.Red; }
            }
            else if (strCode.Equals("03-03"))      // Encoder
            {
                if (false) { }   //dummy
                else if (1 == idir) { lb_MainEncDir.Text = "CW"; lb_MainEncDir.BackColor = Color.LightGreen; }
                else if (2 == idir) { lb_MainEncDir.Text = "ccw"; lb_MainEncDir.BackColor = Color.LightPink; }
                else
                {
                    lb_MainEncDir.Text = "NG";
                    lb_MainEncDir.BackColor = Color.LightGray;
                }
                lb_MainEncCount.Text = msg.Substring(11);
            }
            else if (strCode.Equals("03-02"))
            {
                if (0 == ival) { BbnEncB.BackColor = Color.LightYellow; }
                else { BbnEncB.BackColor = Color.Red; }
            }
            else if (strCode.Equals("03-05"))
            {
                if (0 == ival) { BbnFg.BackColor = Color.LightYellow; }
                else { BbnFg.BackColor = Color.Red; }
            }

            else if (strCode.Equals("05-02"))
            {
                if (0 == ival) { Cont_usb5V.BackColor = Color.LightYellow; }
                else { Cont_usb5V.BackColor = Color.Red; }
            }
            else if (strCode.Equals("05-01"))
            {
                if (0 == ival) { Cont_Pwr_Sw.BackColor = Color.LightYellow; }
                else { Cont_Pwr_Sw.BackColor = Color.Red; }
            }

        }

        // 릴 등록 응답 처리.
        private void AnaReelRegistMsg(string msg)
        {
            //$30xx%
            int ival = int.Parse(msg.Substring(3));
            //data = data.Substring(3, 5);

            if (false) { }
            else if (3 == ival)
            {
                tb_ReelRegi_Result.Text = "취소";
                tb_ReelRegi_Result.BackColor = Color.Gray;
            }
            else if (4 == ival)
            {
                tb_ReelRegi_Result.Text = "진행";
                tb_ReelRegi_Result.BackColor = Color.LightPink;
            }
            else if (5 == ival)
            {
                tb_ReelRegi_Result.Text = "완료";
                tb_ReelRegi_Result.BackColor = Color.LightBlue;
            }
            else
            {
                // NA
            }
        }

        private void SetSwitchState(string data)
        {
            // interface변경에 따른 변경 ($06 --> $13)
            // $1310%   - L 버튼 누름
            // $1311%   - L 버튼 뗌 
            // $1312%   - L 버튼 2번 빠르게  누름  (미사용)
            // $1316%   - L 버튼 길게(2초이상) 누름  (미사용)
            // $1320%   - R 버튼 누름
            // $1321%   - R 버튼 뗌 
            // $1322%   - R 번튼 2번 빠르게 누름.
            // $1326%   - R 버튼 길게(2초이상) 누름  (미사용)
            // data = data.Substring(7, 4);

            data = data.Substring(3, 2);

            if (false) { }
            else if (data.Equals("10"))
            {
                Lab_Btn_Left.BackColor = Color.Blue;
            }
            else if (data.Equals("11"))
            {
                Lab_Btn_Left.BackColor = Color.Gray;
            }

            // 미사용
            else if (data.Equals("12"))
            {
                Lab_Btn_Left.BackColor = Color.Yellow;
            }

            // 미사용
            else if (data.Equals("16"))
            {
                Lab_Btn_Left.BackColor = Color.Purple;
            }

            else if (data.Equals("20"))
            {
                Lab_Btn_Right.BackColor = Color.Red;
            }

            else if (data.Equals("21"))
            {
                Lab_Btn_Right.BackColor = Color.Gray;
            }

            else if (data.Equals("22"))
            {
                Lab_Btn_Right.BackColor = Color.Yellow;
            }

            else if (data.Equals("26"))
            {
                Lab_Btn_Right.BackColor = Color.Purple;
            }

            //  릴 SetUp 보드 Switch
            else if (data.Equals("80"))     // SETUP DOWN
            {
                Lab_Setup.BackColor = Color.Red;
            }
            else if (data.Equals("81"))
            {
                Lab_Setup.BackColor = Color.LightYellow;
            }
            //  릴 Boot 보드 Switch
            else if (data.Equals("90"))     // BOOT UP
            {
                Lab_Boot.BackColor = Color.Red;
            }
            else if (data.Equals("91"))     // BOOT DOWN
            {
                Lab_Boot.BackColor = Color.LightYellow;
            }

        }

        // 설정요구 결과응답 분석 처리.
        private void AnaDevSet_RespResult(string msg)
        {
            int iKind = int.Parse(msg.Substring(3, 2));     // 3,4th
            switch (iKind)
            {
                case 0:
                    break;

                case 1:     //메인모터 최소출력값 설정결과
                    //int iAct=int.Parse(msg.Substring(5, 1));  // Act구분

                    // 결과
                    int iVal = int.Parse(msg.Substring(6));
                    if (0 <= iVal)
                    {
                        tb_MainMot_Default.Text = msg.Substring(6);     //토크모터 출력값
                        tb_MainAutoSet_result.Text = "OK:" + msg.Substring(6);  //결과값
                        tb_MainAutoSet_result.BackColor = Color.White;
                    }
                    else
                    {
                        tb_MainAutoSet_result.Text = "NG:" + msg.Substring(6);  //결과값
                        tb_MainAutoSet_result.BackColor = Color.LightYellow;
                    }
                    break;

                default:
                    break;
            }

        }


        private void Btn_IMU_On_Click(object sender, EventArgs e)
        {
            // uartSerial.WriteMessage("$0811111111%");
            uartSerial.WriteMessage("$081%");               // I/F사양 변경
                                                            //Lab_IMU_Conn.BackColor = Color.LawnGreen;
            Lab_ImuInterval.Text = "---";  // IMU 인터벌 시간 클리어

        }

        private void Btn_IMU_Off_Click(object sender, EventArgs e)
        {
            // uartSerial.WriteMessage("$0800000000%");
            uartSerial.WriteMessage("$080%");        // I/F사양 변경
            //Lab_IMU_Conn.BackColor = Color.Gray;

            Tb_Roll.Text = "";
            Tb_Pitch.Text = "";
            Tb_Yaw.Text = "";
            Tb_Ax.Text = "";
            Tb_Ay.Text = "";
            Tb_Az.Text = "";

            //Lab_ImuInterval.Text = "";  // IMU 인터벌 시간 클리어
        }

        private void numUpDown_Duty_ValueChanged(object sender, EventArgs e)
        {
            iBLDC_Duty_Value = (int)updown_Bobbin_Duty.Value;
            Tb_BLDC_Duty.Value = (int)updown_Bobbin_Duty.Value;
            lbl_CW_CCW.Text = Btn_Bldc_On_Output();

        }
        private void Tb_BLDC_Duty_KeyUp(object sender, KeyEventArgs e)
        {
            BLDC_Duty_Ctrl();
        }
        private void Tb_BLDC_Duty_MouseUp(object sender, MouseEventArgs e)
        {
            BLDC_Duty_Ctrl();
        }

        // BLDC Duty박스 Enter 키 입력
        private void BLDC_Duty_Ctrl()
        {
            iBLDC_Duty_Value = (int)Tb_BLDC_Duty.Value;
            updown_Bobbin_Duty.Value = (int)Tb_BLDC_Duty.Value;
            lbl_CW_CCW.Text = Btn_Bldc_On_Output();
        }

        // BLDC TIME 박스 Enter 키 입력
        private void numUpDown_OnTime_ValueChanged(object sender, EventArgs e)
        {
            iBLDC_OnTime_Value = (int)updown_Bobbin_timems.Value;
            Tb_BLDC_OnTime.Value = (int)updown_Bobbin_timems.Value;
            lbl_CW_CCW.Text = Btn_Bldc_On_Output();

        }
        private void Tb_BLDC_OnTime_KeyUp(object sender, KeyEventArgs e)
        {
            BLDC_OnTimes_Ctrl();
        }
        private void Tb_BLDC_OnTime_MouseUp(object sender, MouseEventArgs e)
        {
            BLDC_OnTimes_Ctrl();
        }
        private void BLDC_OnTimes_Ctrl()
        {
            iBLDC_OnTime_Value = (int)Tb_BLDC_OnTime.Value;
            updown_Bobbin_timems.Value = (int)Tb_BLDC_OnTime.Value;
            lbl_CW_CCW.Text = "";
        }

        private void radio_BLDC_CW_CheckedChanged(object sender, EventArgs e)
        {
            lbl_CW_CCW.Text = "";
        }
        private void radio_BLDC_CCW_CheckedChanged(object sender, EventArgs e)
        {
            lbl_CW_CCW.Text = "";
        }

        private void Btn_Bldc_On_Click(object sender, EventArgs e)
        {
            // OFF TIme CLEAR
            tb_Bldc_Off_time.Text = "--";          // Te:xxxx

            lbl_CW_CCW.Text = Btn_Bldc_On_Output();
        }

        // 실제 출력 명령 CMD 송신
        private String Btn_Bldc_On_Output()
        {
            string sendCmd = "";
            String strResult = "";

            if (radio_BLDC_CW.Checked)
            {
                sendCmd = "$04" + "1" + iBLDC_Duty_Value.ToString("000") + iBLDC_OnTime_Value.ToString("0000") + "%";
            }
            else if (radio_BLDC_CCW.Checked)
            {
                sendCmd = "$04" + "2" + iBLDC_Duty_Value.ToString("000") + iBLDC_OnTime_Value.ToString("0000") + "%";
            }
            else
            {
                sendCmd = "$04" + "0" + "000" + "0000" + "%";
                // MessageBox.Show("CW 혹은 CCW를 선택해 주세요.");
                strResult = "CW 혹은 CCW를 선택해 주세요.";
            }

            // Value 송신.    
            uartSerial.WriteMessage(sendCmd);

            return strResult;
        }


        private void Btn_Bldc_Off_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";
            CtrlStr = "$04" + "0" + "000" + "0000" + "%";
            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);

            // radio_BLDC_CW.Checked = false;
            // radio_BLDC_CCW.Checked = false;
            lbl_CW_CCW.Text = "";
        }

        private void numUpDown_LedPosNo_ValueChanged(object sender, EventArgs e)
        {
            iLED_Pos_Number = (int)numUpDown_LedPosNo.Value;
            switch (iLED_Pos_Number)
            {
                case 1:
                    lblLedPos.Text = "중앙";
                    break;
                case 2:
                    lblLedPos.Text = "하단";
                    break;
                case 3:
                    lblLedPos.Text = "좌측";
                    break;
                case 4:
                    lblLedPos.Text = "우측";
                    break;
            }
        }

        private void numUpDown_LedModeNo_ValueChanged(object sender, EventArgs e)
        {
            iLED_Mode_Number = (int)numUpDown_LedModeNo.Value;
            switch (iLED_Mode_Number)
            {
                case 0:
                    lblLedMode.Text = "OFF";
                    break;
                case 1:
                    lblLedMode.Text = "ON";
                    break;
                case 2:
                    lblLedMode.Text = "점멸";
                    break;
                case 3:
                    lblLedMode.Text = "디밍";
                    break;
                case 8:
                    lblLedMode.Text = "밝기읽기";
                    break;
                case 9:
                    lblLedMode.Text = "밝기설정";
                    break;
                default:
                    lblLedMode.Text = "-";
                    break;
            }
        }

        private void numUpDown_LedColorNo_ValueChanged(object sender, EventArgs e)
        {
            iLED_Color_Number = (int)numUpDown_LedColorNo.Value;
            switch (iLED_Color_Number)
            {
                case 0:
                    lblLedColor.Text = "White";
                    lblLedColor.ForeColor = Color.Black;
                    lblLedColor.BackColor = Color.White;
                    break;
                case 1:
                    lblLedColor.Text = "Red";
                    //lblLedColor.ForeColor = Color.Red;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Red;
                    break;
                case 2:
                    lblLedColor.Text = "Lime";
                    //lblLedColor.ForeColor = Color.Lime;
                    lblLedColor.ForeColor = Color.Black;
                    lblLedColor.BackColor = Color.Lime;
                    break;
                case 3:
                    lblLedColor.Text = "Blue";
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Blue;
                    break;
                case 4:
                    lblLedColor.Text = "Yellow";
                    //lblLedColor.ForeColor = Color.Yellow;
                    lblLedColor.ForeColor = Color.Black;
                    lblLedColor.BackColor = Color.Yellow;
                    break;
                case 5:
                    lblLedColor.Text = "Cyan";
                    //lblLedColor.ForeColor = Color.Cyan;
                    lblLedColor.ForeColor = Color.Black;
                    lblLedColor.BackColor = Color.Cyan;
                    break;
                case 6:
                    lblLedColor.Text = "Magenta";
                    //lblLedColor.ForeColor = Color.Cyan;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Magenta;
                    break;
                case 7:
                    lblLedColor.Text = "Maroon";
                    //lblLedColor.ForeColor = Color.Maroon;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Maroon;
                    break;
                case 8:
                    lblLedColor.Text = "Olive";
                    //lblLedColor.ForeColor = Color.Olive;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Olive;
                    break;
                case 9:
                    lblLedColor.Text = "Green";
                    //lblLedColor.ForeColor = Color.Green;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Green;
                    break;
                case 10:
                    lblLedColor.Text = "Purple";
                    //lblLedColor.ForeColor = Color.Purple;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Purple;
                    break;
                case 11:
                    lblLedColor.Text = "Teal";
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Teal;
                    break;
                case 12:
                    lblLedColor.Text = "Navy";
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Navy;
                    break;
                case 13:
                    lblLedColor.Text = "Silver";
                    //lblLedColor.ForeColor = Color.Silver;
                    lblLedColor.ForeColor = Color.Black;
                    lblLedColor.BackColor = Color.Silver;
                    break;
                case 14:
                    lblLedColor.Text = "Gray";
                    //lblLedColor.ForeColor = Color.Gray;
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Gray;
                    break;
                case 15:
                    lblLedColor.Text = "Black";
                    lblLedColor.ForeColor = Color.White;
                    lblLedColor.BackColor = Color.Black;
                    break;
            }
        }

        private void numUpDown_LedControlTime_ValueChanged(object sender, EventArgs e)
        {
            iLED_Control_Time = (int)numUpDown_LedControlTime.Value;

        }

        private void numUpDown_LedRead_ValueChanged(object sender, EventArgs e)
        {
            iLED_Brightness_Number = (int)numUpDown_LedRead.Value;
        }

        private void Btn_LED_On_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";
            // 사양: $06xYY% --> x-위치,YY-LED패턴 (패턴01~99)
            // 변경: $06xYzzTTTT%
            // iLED_Pos_Number: x
            // iLED_Mode_Number: y
            // iLED_Color_Number:zz
            // iLED_Control_Time:TTTT
            switch (iLED_Mode_Number)
            {
                case 1:             // ON <-- $06x1zz%
                    CtrlStr = "$06" + iLED_Pos_Number.ToString("0") + iLED_Mode_Number.ToString("0") + iLED_Color_Number.ToString("00") + "%";
                    break;
                case 2:             // 점멸 <-- $06x2zzTTTT%
                    CtrlStr = "$06" + iLED_Pos_Number.ToString("0") + iLED_Mode_Number.ToString("0") + iLED_Color_Number.ToString("00") + iLED_Control_Time.ToString("0000") + "%";
                    break;
                case 3:             // 디밍 <-- $06x3zzTTTT%
                    CtrlStr = "$06" + iLED_Pos_Number.ToString("0") + iLED_Mode_Number.ToString("0") + iLED_Color_Number.ToString("00") + iLED_Control_Time.ToString("0000") + "%";
                    break;
                case 8:             // 밝기 일기 <-- $0608%
                    CtrlStr = "$06" + "0" + iLED_Mode_Number.ToString("0") + "%";
                    break;
                case 9:             // 밝기 쓰기 <-- $0609ppp%
                    CtrlStr = "$06" + "0" + iLED_Mode_Number.ToString("0") + iLED_Brightness_Number.ToString("000") + "%";
                    break;
                default:
                    break;
            }

            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
        }

        private void Btn_LED_Off_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";

            //numUpDown_LedModeNo.Value = 0;
            iLED_Mode_Number = (int)numUpDown_LedModeNo.Value;
            // 사양: $06xYY% --> x-위치,YY-LED패턴 (패턴01~99)
            // 변경: $06xYzzTTTT%
            // iLED_Pos_Number: x
            // iLED_Mode_Number: y
            // iLED_Color_Number:zz
            // iLED_Control_Time:TTTT
            CtrlStr = "$06" + iLED_Pos_Number.ToString("0") + "0" + "%";

            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
        }

        private void btn_LED_Read_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";
            // numUpDown_LedModeNo.Value = 8;
            // iLED_Mode_Number = (int)numUpDown_LedModeNo.Value;

            // 사양: $06xYY% --> x-위치,YY-LED패턴 (패턴01~99)
            // 변경: $06xYzzTTTT%
            // iLED_Pos_Number: x
            // iLED_Mode_Number: y
            // iLED_Color_Number:zz
            // iLED_Control_Time:TTTT
            CtrlStr = "$06" + "0" + "8" + "%";
            // Value 송신.
            uartSerial.WriteMessage(CtrlStr);

        }

        private void btn_LED_Write_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";
            // numUpDown_LedModeNo.Value = 9;
            // iLED_Mode_Number = (int)numUpDown_LedModeNo.Value;

            // 사양: $06xYY% --> x-위치,YY-LED패턴 (패턴01~99)
            // 변경: $06xYzzTTTT%
            // iLED_Pos_Number: x
            // iLED_Mode_Number: y
            // iLED_Color_Number:zz
            // iLED_Control_Time:TTTT
            CtrlStr = "$06" + "0" + "9" + iLED_Brightness_Number.ToString("000") + "%";
            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
        }

#if (false)
        private void Btn_Device_Check_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";
            CtrlStr = "$1501%";
            uartSerial.WriteMessage(CtrlStr);
        }
#endif


#if (false) // LM_CMD_BTN
        private void btn_LM_ON_Click (object sender, EventArgs e)
		{
            string CtrlStr = "";

            if (radio_LM_CW.Checked)
            {
                CtrlStr = "$07" + "1" + iLM_Duty_Value.ToString("000") + iLM_OnTime_Value.ToString("0000") + "%";
            }
            else if (radio_LM_CCW.Checked)
            {
                CtrlStr = "$07" + "2" + iLM_Duty_Value.ToString("000") + iLM_OnTime_Value.ToString("0000") + "%";
            }
            else
            {
                CtrlStr = "$07" + "0" + "%";
                // MessageBox.Show("CW 혹은 CCW를 선택해 주세요.");
                //lbl_CW_CCW.Text = "CW 혹은 CCW를 선택해 주세요.";
            }

            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);

		}

		private void btn_LM_OFF_Click (object sender, EventArgs e)
		{
            string CtrlStr = "";
            CtrlStr = "$07" + "0" + "%";
            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
		}

		private void updown_LM_Duty_ValueChanged (object sender, EventArgs e)
		{
            iLM_Duty_Value  =   (int)updown_LM_Duty.Value;
		}
        private void updown_LM_Duty_KeyUp (object sender, KeyEventArgs e)
        {
            updown_LM_Duty_Ctrl();
        }
        private void updown_LM_Duty_MouseUp (object sender, MouseEventArgs e)
        {
            updown_LM_Duty_Ctrl();
        }

        private void updown_LM_Duty_Ctrl()
        {
            iLM_Duty_Value  = (int)updown_LM_Duty.Value;
            tb_LM_Duty.Value= (int)updown_LM_Duty.Value;
        }
        private void updown_LM_timems_Ctrl()
        {
            iLM_OnTime_Value    =   (int)updown_LM_timems.Value;
            tb_LM_timems.Value  =   (int)updown_LM_timems.Value;
        }

		private void updown_LM_timems_ValueChanged (object sender, EventArgs e)
		{
            iLM_OnTime_Value=   (int)updown_LM_timems.Value;
		}

		private void tb_LM_Duty_Scroll (object sender, EventArgs e)
		{
            iLM_Duty_Value          =   tb_LM_Duty.Value;
            updown_LM_Duty.Value    =   tb_LM_Duty.Value;
		}

		private void tb_LM_timems_Scroll (object sender, EventArgs e)
		{
            iLM_OnTime_Value        =   tb_LM_timems.Value;
            updown_LM_timems.Value  =   tb_LM_timems.Value;
		}

		private void updown_LM_timems_KeyUp (object sender, KeyEventArgs e)
		{
            updown_LM_timems_Ctrl();
		}

		private void updown_LM_timems_MouseUp (object sender, MouseEventArgs e)
		{
            updown_LM_timems_Ctrl();
		}

#endif
        private void SetREELTarget(EREELGroup e)
        {
            btn_REEL_motor.BackColor = Color.Gainsboro;
            btn_REEL_LED_left.BackColor = Color.Gainsboro;
            btn_REEL_LED_right.BackColor = Color.Gainsboro;

            switch (e)
            {
                case EREELGroup.REEL_MOTOR:
                    btn_REEL_motor.BackColor = Color.GreenYellow;
                    break;
                case EREELGroup.REEL_LED_LEFT:
                    btn_REEL_LED_left.BackColor = Color.GreenYellow;
                    break;
                case EREELGroup.REEL_LED_RIGHT:
                    btn_REEL_LED_right.BackColor = Color.GreenYellow;
                    break;
            }
            eSelectedReelMenu = e;
        }
        private void btn_REEL_motor_Click(object sender, EventArgs e)
        {
            SetREELTarget(EREELGroup.REEL_MOTOR);
        }

        private void btn_REEL_LED_left_Click(object sender, EventArgs e)
        {
            SetREELTarget(EREELGroup.REEL_LED_LEFT);
        }

        private void btn_REEL_LED_right_Click(object sender, EventArgs e)
        {
            SetREELTarget(EREELGroup.REEL_LED_RIGHT);
        }

        private void btn_REEL_ON_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";

            switch (eSelectedReelMenu)
            {
                case EREELGroup.REEL_MOTOR: CtrlStr = "$23" + "1" + "%"; break;
                case EREELGroup.REEL_LED_LEFT: CtrlStr = "$24" + "11" + "%"; break;
                case EREELGroup.REEL_LED_RIGHT: CtrlStr = "$24" + "21" + "%"; break;
            }
            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
        }

        private void btn_REEL_OFF_Click(object sender, EventArgs e)
        {
            string CtrlStr = "";

            switch (eSelectedReelMenu)
            {
                case EREELGroup.REEL_MOTOR: CtrlStr = "$23" + "0" + "%"; break;
                case EREELGroup.REEL_LED_LEFT: CtrlStr = "$24" + "10" + "%"; break;
                case EREELGroup.REEL_LED_RIGHT: CtrlStr = "$24" + "20" + "%"; break;
            }
            // Value 송신.    
            uartSerial.WriteMessage(CtrlStr);
        }

        // 수동명령 송신 버튼 클릭
        private void btn_Custom_send1_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend1.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_Custom_send2_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend2.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_Custom_send3_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend3.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_Custom_send4_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend4.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_Custom_send5_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend5.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_Custom_send6_Click(object sender, EventArgs e)
        {
            string data = tb_CustomSend6.Text;
            uartSerial.WriteMessage(data);
        }

        private void btn_log_clear_Click(object sender, EventArgs e)
        {
            uartSerial.ClearLogBox();
        }

        private void btn_log_save_Click(object sender, EventArgs e)
        {
            uartSerial.SaveLog();
        }
        private void btn_log_autoscroll_Click(object sender, EventArgs e)
        {
            uartSerial.ToggleAutoScroll();
        }


        private void button1_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$3001%");  // 릴 등록
        }

#if (false)
        private void tb_CustomSend1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                isEnterSend1Released = true;  // Enter 키가 떼어졌다는 플래그 설정
            }
        }
        private void tb_CustomSend1_TextChanged(object sender, EventArgs e)
        {
            if (isEnterSend1Released)
            { 
                btn_Custom_send1_Click(sender, e);
                isEnterSend1Released = false;
            }
        }

        private void tb_CustomSend2_TextChanged(object sender, EventArgs e)
        {
            btn_Custom_send2_Click(sender, e);
        }

        private void tb_CustomSend3_TextChanged(object sender, EventArgs e)
        {
            btn_Custom_send3_Click(sender, e);
        }

        private void tb_CustomSend4_TextChanged(object sender, EventArgs e)
        {
            btn_Custom_send4_Click(sender, e);
        }

        private void tb_CustomSend5_TextChanged(object sender, EventArgs e)
        {
            btn_Custom_send5_Click(sender, e);
        }

        private void tb_CustomSend6_TextChanged(object sender, EventArgs e)
        {
            btn_Custom_send6_Click(sender, e);
        }
#endif
        // 메인보드 IO확인 시작
        private void Btn_Bbn_In_Start_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$2701-00,1%");  // IN Start, Board
            uartSerial.WriteMessage("$2705-00,1%");  // IN Start, Elec Switch
            uartSerial.WriteMessage("$2703-00,1%");  // IN Start, Monitor Start, BBN IN ALL
        }

        private void Btn_Bbn_In_Stop_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$2701-00,0%");  // IN Stop, Board
            uartSerial.WriteMessage("$2705-00,0%");  // IN Stop, Elec Switch
            uartSerial.WriteMessage("$2703-00,0%");  // IN Stop, BBN IN ALL

            // 표시박스 : 회색처리
            BbnEncA.BackColor = Color.Gray;
            BbnEncB.BackColor = Color.Gray;
            BbnFg.BackColor = Color.Gray;

            //
            Cont_Boot_Sw.BackColor = Color.Gray;
            Cont_usb5V.BackColor = Color.Gray;
            Cont_Pwr_Sw.BackColor = Color.Gray;
        }

        private void Btn_SubAc_On_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$2805-01,1%");  // OUT, Sub AC ON 
        }

        private void Btn_SubAc_Off_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$2805-01,0%");  // OUT, Sub AC OFF
        }


        private void button2_Click(object sender, EventArgs e)
        {
            uartSerial.WriteMessage("$3001%");  // 릴 등록
            tb_ReelRegi_Result.Text = "개시";
            tb_ReelRegi_Result.BackColor = Color.LightYellow;
        }

        // 제조 정보
        private void button1_Click_1(object sender, EventArgs e)
        {
            tb_Manuf_Model.Text = "DF1SF" + dud_Spec.Text;
            tb_Manuf_SerialNo.Text = DateTime.Now.ToString("yyMM");
            tb_Manuf_SerialNo.Text += "ikr";
            tb_Manuf_SerialNo.Text += dud_SerialNumber.Text;
            tb_Manuf_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
        }

        private void btn_Manuf_Read_Click(object sender, EventArgs e)
        {
            tb_Manuf_Model.Text = "-";
            tb_Manuf_SerialNo.Text = "-";
            tb_Manuf_Date.Text = "-";
            uartSerial.WriteMessage("$0104%");  // 제조정보 읽기
        }

        private void btn_Manuf_Write_Click(object sender, EventArgs e)
        {
            //tb_Manuf_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            strMsg = tb_Manuf_Model.Text + "," + tb_Manuf_No.Text + "," + tb_Manuf_SerialNo.Text + "," + tb_Manuf_Date.Text;
            strMsg = "$0105" + strMsg + "%";
            uartSerial.WriteMessage(strMsg);  // 제조정보 저장
        }

        private void btn_Manuf_Erase_Click(object sender, EventArgs e)
        {
            tb_Manuf_Model.Text = "-";
            tb_Manuf_SerialNo.Text = "-";
            tb_Manuf_Date.Text = "ok";
            uartSerial.WriteMessage("$0106%");  // 제조정보 지우기
        }

        // 필드 설치(Field Install)
        private void Btn_Install_Init_Click(object sender, EventArgs e)
        {
            tb_Install_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
        }
        private void btn_Install_Read_Click(object sender, EventArgs e)
        {
            tb_Install_Date.Text = "-";
            uartSerial.WriteMessage("$0107%");  // 설치 정보 읽기
        }

        private void btn_Install_Write_Click_1(object sender, EventArgs e)
        {
            //tb_Install_Date.Text = DateTime.Now.ToString("yyyy-MMdd");
            strMsg = tb_Install_Date.Text;
            strMsg = "$0108" + strMsg + "%";
            uartSerial.WriteMessage(strMsg);  // 설치 정보 저장

        }

        private void btn_Install_Delete_Click(object sender, EventArgs e)
        {
            tb_Install_Date.Text = "ok";
            uartSerial.WriteMessage("$0109%");  //설치 정보 지우기
        }

        private void btn_Mmot_Init_Click(object sender, EventArgs e)
        {
            tb_MainMot_Default.Text = "70";
        }

        private void btn_Mmot_Read_Click(object sender, EventArgs e)
        {
            tb_MainMot_Default.Text = "-";
            uartSerial.WriteMessage("$0110%");  //메인모터 Duty기본값 읽기
        }

        private void btn_Mmot_Write_Click(object sender, EventArgs e)
        {
            strMsg = tb_MainMot_Default.Text;
            int defaultDuty = (int.Parse(strMsg)); //[40 ~ 100]
        //  if (40 > defaultDuty || 99 < defaultDuty)
			if (99 < defaultDuty)
			{
					tb_MainMot_Default.Text = "NG";
            }
            else
            {
                strMsg = "$0111" + strMsg + "%";
                uartSerial.WriteMessage(strMsg);  // 설치 정보 저장
            }

        }



        // 메인모트 토크(최소값) 자동조정 CMD송신
        private void btn_MainMotAutoSet_Click(object sender, EventArgs e)
        {
            try
            {
                // 에러코드,결과 CLR
                Textbox_Error_Code.Text = "";
                TextBox_Error_Content.Text = "";

                strMsg = "$26011%";         // A=1 시작
                uartSerial.WriteMessage(strMsg);  // 메인모터 최소출력(Duty)자동설정 요구
                tb_MainAutoSet_result.Text = "--";
            }

            catch (Exception ex)
            {
                LogText.Log("예외 발생: " + ex.ToString());
                MessageBox.Show("에러 발생: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        //====에이징 동작===


        /*
             // 초기값 설정
             private void btn_WireInitValue_Click(object sender, EventArgs e)
             {
                 nud_mmot_cw_duty1.Value = 100;   // duty
                 nud_bldc_duty1.Value = 20;   // duty
                 nud_mmot_cw_time1.Value = 2000;  //ms
                 nud_mmot_cw_duty2.Value = 70;   // duty
                 nud_bldc_duty2.Value = 0;   // duty
                 nud_mmot_cw_time2.Value = 3000;  //ms
             }

             // Wire에이징 동작 시작
             private void btn_wireAgingStart_Click(object sender, EventArgs e)
             {
                 // kk=01, A=1 Start
                 strMsg = "$98011" + "," + nud_mmot_cw_duty1.Text + "," + nud_bldc_duty1.Text + "," + nud_mmot_cw_time1.Text + "," + nud_mmot_cw_duty2.Text + "," + nud_bldc_duty2.Text + "," + nud_mmot_cw_time2.Text;  ////d1,t1,d2,t2,repeat회수(0=무한)
                 strMsg += "%";
                 uartSerial.WriteMessage(strMsg);  // 메인모터 최소출력(Duty)자동설정 요구
                 tb_WireAgingResult.Text = "--";
                 tb_WireAgingResult.BackColor = Color.LightGray;
             }

             // Wire에이징 동작 정지
             private void btn_wireAgingStop_Click(object sender, EventArgs e)
             {
                 strMsg = "$98010"; // kk=01, A=0 Stop
                 strMsg += "%";
                 //strMsg += nud_mmot_cw_duty1.TextAlign + nud_mmot_cw_time1.TextAlign + nud_mmot_cw_duty2.TextAlign + nud_mmot_cw_time2.TextAlign; //d1,t1,d2,t2,repeat회수(0=무한)
                 // 메인모터 최소출력(Duty)자동설정 요구
                 uartSerial.WriteMessage(strMsg);
                 tb_WireAgingResult.Text = "--";
                 tb_WireAgingResult.BackColor = Color.LightGray;
                 nud_AgingCounter.BackColor = Color.White;
             }
        */

        //==== 표시 =====

        // 에이징 카운트 저장
        private void btn_AgingCntSave_Click(object sender, EventArgs e)
        {

            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Title = "로그 저장";
            sfd.Filter = "텍스트 파일 | *.txt; | 모든 파일 | *.*";
            if (DialogResult.OK == sfd.ShowDialog())
            {
                string fileName = sfd.FileName;

                if (!fileName.Contains(".txt"))
                {
                    fileName = fileName + ".txt";
                }

                try
                {
                    string todayStr = DateTime.Now.ToString("yyyy-MM-dd");
                    string currentTimeStr = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

                    // 숫자 11자리 고정폭으로 오른쪽 정렬
                    string countStr = nud_AgingCounter.Value.ToString().PadLeft(11, ' ');
                    //string countStr = nud_AgingCounter.Value.ToString("D11");

                    // 새 데이터 한 줄
                    //string newLine = $"{countStr} , {DateTime.Now:yyyy-MM-dd HH:mm:ss}";
                    string newLine = $"{countStr} , {currentTimeStr}";
                    List<string> updatedLines = new List<string>();

                    // 기존 내용 읽기 (있으면)
                    /*
                    string existingContent = "";
                    if (File.Exists(fileName))
                    {
                        existingContent = File.ReadAllText(fileName);
                    }

 

                    // 새 데이터 + 기존 내용 합치기
                    string combinedContent = newLine + Environment.NewLine + existingContent;

                    // 덮어쓰기
                    File.WriteAllText(fileName, combinedContent);

                    MessageBox.Show("카운트가 저장되었어요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    */

                    if (File.Exists(fileName))
                    {
                        string[] lines = File.ReadAllLines(fileName);

                        if (lines.Length > 0 &&
                            !string.IsNullOrWhiteSpace(lines[0]) &&
                            lines[0].Contains(",") &&
                            lines[0].Split(',').Length >= 2 &&
                            lines[0].Split(',')[1].Trim().StartsWith(todayStr))
                        {
                            // 첫 줄이 오늘 날짜 → 덮어쓰기
                            updatedLines.Add(newLine);
                            updatedLines.AddRange(lines.Skip(1));
                        }
                        else
                        {
                            // 첫 줄이 오늘 날짜가 아님 → 맨 위에 새 줄 추가
                            updatedLines.Add(newLine);
                            updatedLines.AddRange(lines);
                        }
                    }
                    else
                    {
                        // 파일이 없으면 새로 생성
                        updatedLines.Add(newLine);
                    }
                    File.WriteAllLines(fileName, updatedLines);

                    MessageBox.Show("카운트가 저장되었어요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }

                catch (Exception ex)
                {
                    MessageBox.Show("오류: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                MessageBox.Show("저장의 바른 경로를 알려줘요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }
 

        // 다른날짜는 줄 변경
#if (false)
                try
                {
                    string todayStr = DateTime.Now.ToString("yyyy-MM-dd");
                    string currentTimeStr = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
                    string countStr = nud_AgingCounter.Value.ToString("D11"); // 11자리, 0으로 채움

                    string newLine = $"{countStr} , {currentTimeStr}";
                    string[] lines = File.Exists(fileName) ? File.ReadAllLines(fileName) : new string[0];

                    bool updated = false;
                    List<string> updatedLines = new List<string>();

                    foreach (string line in lines)
                    {
                        if (!string.IsNullOrWhiteSpace(line) && line.Contains(","))
                        {
                            string[] parts = line.Split(',');
                            if (parts.Length >= 2 && parts[1].Trim().StartsWith(todayStr))
                            {
                                // 같은 날짜인 경우 → 현재 줄 대신 새 줄로 교체
                                updatedLines.Add(newLine);
                                updated = true;
                                continue; // 이 줄은 덮어썼으니 추가로 안 넣음
                            }
                        }

                        // 날짜 다르면 기존 줄 유지
                        updatedLines.Add(line);
                    }

                    if (!updated)
                    {
                        // 같은 날짜 없으면 맨 위에 새 줄 추가
                        updatedLines.Insert(0, newLine);
                    }

                    File.WriteAllLines(fileName, updatedLines);

                    MessageBox.Show("카운트가 저장되었어요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("오류: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
#endif



        /*
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Title = "로그 저장";

            // ✅ 필터 구문 정확히 지정
            sfd.Filter = "텍스트 파일 (*.txt)|*.txt|모든 파일 (*.*)|*.*";

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                string fileName = sfd.FileName;

                // ✅ 확장자 자동 추가 (.txt 없으면 붙이기)
                if (Path.GetExtension(fileName).ToLower() != ".txt")
                {
                    fileName += ".txt";
                }

                try
                {
                    string todayStr = DateTime.Now.ToString("yyyy-MM-dd");
                    string currentTimeStr = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");

                    // 숫자 11자리로 0 채움
                    string countStr = nud_AgingCounter.Value.ToString("D11");
                    string newLine = $"{countStr} , {currentTimeStr}";

                    // 기존 파일 읽기
                    string[] lines = File.Exists(fileName) ? File.ReadAllLines(fileName) : new string[0];
                    bool updated = false;
                    List<string> updatedLines = new List<string>();

                    foreach (string line in lines)
                    {
                        if (!string.IsNullOrWhiteSpace(line) && line.Contains(","))
                        {
                            string[] parts = line.Split(',');
                            if (parts.Length >= 2 && parts[1].Trim().StartsWith(todayStr))
                            {
                                // 오늘 날짜가 이미 있으면 교체
                                updatedLines.Add(newLine);
                                updated = true;
                                continue;
                            }
                        }

                        // 다른 날짜는 유지
                        updatedLines.Add(line);
                    }

                    if (!updated)
                    {
                        // 오늘 날짜가 없으면 맨 위에 추가
                        updatedLines.Insert(0, newLine);
                    }

                    File.WriteAllLines(fileName, updatedLines);

                    MessageBox.Show("카운트가 저장되었어요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("오류: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        */


        //-----------------------

        // 에이징 카운트 읽기
        private void btn_AgingCntRead_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Title = "카운트 불러오기";
            ofd.Filter = "텍스트 파일 | *.txt; | 모든 파일 | *.*";

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                agingCntFileName = ofd.FileName;
                string fileName = ofd.FileName;

                try
                {
                    string firstLine = null;

                    // 파일의 첫 번째 줄만 읽기
                    using (StreamReader sr = new StreamReader(fileName))
                    {
                        firstLine = sr.ReadLine(); 
                    }

                    if (!string.IsNullOrEmpty(firstLine))
                    {
                        // 예: "5 , 2025-07-03 10:00:00"
                        string[] parts = firstLine.Split(',');
                        if (parts.Length >= 1)
                        {
                            string countString = parts[0].Trim();
                            decimal countValue;
                            if (decimal.TryParse(countString, out countValue))
                            {
                                nud_AgingCounter.Value = countValue;
                                nud_AgingCounter.BackColor = Color.White;
                                MessageBox.Show("최신 카운트로 표시했어요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Information);
                            }
                            else
                            {
                                MessageBox.Show("파일 형식이 바르지 않아요.", "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                        }
                        else
                        {
                            MessageBox.Show("파일에 데이터가 없어요.", "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        }
                    }
                    else
                    {
                        MessageBox.Show("파일이 비어 있어요.", "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("오류: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                MessageBox.Show("읽기의 바른 파일을 선택해 주세요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }


        // 에이징 카운트 클리어(0)
        private void btn_AgingCntClear_Click(object sender, EventArgs e)
        {
            // 1) 값 0으로 초기화
            nud_AgingCounter.Value = 0;
            nud_AgingCounter.BackColor = Color.LightGray;
            // 2) 기존 저장 함수 호출
            // 두 번째 인자는 필요 없지만 이벤트 핸들러 시그니처상 null 넣어줍니다.
            btn_AgingCntSave_Click(sender, e);
        }

        // 프로그램 시작시 파일에서 카운트 가져옴.
        private void LoadAgingCount()
        {
            string fileName = agingCntFileName;

            if (File.Exists(fileName))
            {
                try
                {
                    string firstLine = null;

                    using (StreamReader sr = new StreamReader(fileName))
                    {
                        firstLine = sr.ReadLine();
                    }

                    if (!string.IsNullOrEmpty(firstLine))
                    {
                        string[] parts = firstLine.Split(',');
                        if (parts.Length >= 1)
                        {
                            string countString = parts[0].Trim();
                            decimal countValue;
                            if (decimal.TryParse(countString, out countValue))
                            {
                                nud_AgingCounter.Value = countValue;
                                nud_AgingCounter.BackColor = Color.LightGreen;
                            }
                            else
                            {
                                MessageBox.Show("저장된 카운트 값이 바르지 않아요.", "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("파일을 읽는 중 오류가 발생했어요: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
                // 파일이 없으면 0으로 초기화
                nud_AgingCounter.Value = 0;
                nud_AgingCounter.BackColor = Color.LightGray;
                // 반복(에이징)테스트 필요시, 아래 ture로 변경하면 됨.
                if (false)
                {
                    MessageBox.Show(" AgingCount.txt 파일이 없어 - 0 - 으로 표시할께요! DH괜찮아요?");
                }
            }
        }


		// 동작중 카운트업은 젤위라인을 계속 갱신
		bool _errorShowing = false;
		private void writeAgingCountUp(string fileName)
        {

            //string fileName = Application.StartupPath + @"\AgingCount.txt";
 
            if (File.Exists(fileName))
            {
                try
                {
                    // 기존 내용 읽기 (있으면)
                    List<string> lines = new List<string>();
                    lines = File.ReadAllLines(fileName).ToList();
                    
                    // 숫자 11자리 고정폭으로 오른쪽 정렬
                    string countStr = nud_AgingCounter.Value.ToString().PadLeft(11, ' ');
                    // 새 데이터 한 줄
                    string newLine = $"{countStr} , {DateTime.Now:yyyy-MM-dd HH:mm:ss}";

                    if (0 == lines.Count || 1 == lines.Count)
                    {
                        // 줄이 없거나 한 줄만 있으면 새 줄을 추가
                        lines.Add(newLine);
                    }
                    else
                    {
                        // 두 줄 이상이면 첫 줄을 새 줄로 덮어쓰기
                        lines[0] = newLine;
                    }

                    // 덮어쓰기
                    File.WriteAllLines(fileName, lines);
                }

                catch (Exception ex)
                {
                    MessageBox.Show("오류: " + ex.Message, "오류", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            else
            {
				if (_errorShowing)
					return;
				_errorShowing = true;
				MessageBox.Show("카운트업의 바른 경로를 알려줘요.\n[읽기]버튼으로 파일을 지정해줘요", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				_errorShowing = false;
			}

        }


        //
        private void btn_MainEncoder_StartStop_Click(object sender, EventArgs e)
        {
            if(btn_MainEncoder_StartStop.Text.Equals("시작")) 
            {
                uartSerial.WriteMessage("$2703-03,1%");  // IN Start, Monitor Start, BBN IN ALL
                btn_MainEncoder_StartStop.Text = "정지";
            }
            else
            {
                uartSerial.WriteMessage("$2703-03,0%");  // IN Start, Monitor Start, BBN IN ALL
                btn_MainEncoder_StartStop.Text = "시작";
            }
        }

        private void btn_MainEnc_Init_Click(object sender, EventArgs e)
        {
            lb_MainEncCount.Text = "0";
        }


        // 로그 저장 시간 간격 조절
        private int ilogSaveMin = 5;
        
        private void tb_LogSaveMin_TextChanged(object sender, EventArgs e)
        {
           if (int.TryParse(tb_LogSaveMin.Text, out int value))
           { 
                if (1 > value) tb_LogSaveMin.Text = "1";
                else if (60 < value) tb_LogSaveMin.Text = "60";
            }
            else
            {
                MessageBox.Show("숫자를 입력하세요.");
            }
        }

#if (false)
        private void InitializeLogSaveMin()
        {
            // 텍스트박스 생성
            tb_LogSaveMin = new TextBox();
            tb_LogSaveMin.Location = new Point(20, 20);
            tb_LogSaveMin.Width = 100;

            // 이벤트 핸들러 연결
            tb_LogSaveMin.KeyDown += Tb_LogSaveMin_KeyDown;
            tb_LogSaveMin.KeyPress += Tb_LogSaveMin_KeyPress;

            this.Controls.Add(tb_LogSaveMin);
        }
        // 숫자만 입력 허용
        private void Tb_LogSaveMin_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar))
            {
                e.Handled = true;  // 숫자 외 입력 차단
            }
        }

        // Enter 키 입력 시 처리
        private void Tb_LogSaveMin_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                e.SuppressKeyPress = true; // 삑 소리 방지

                if (int.TryParse(tb_LogSaveMin.Text, out int value))
                {
                    // 숫자 처리 로직
                    // 예: logMgr.SetSaveInterval(value);
                    if (value >= 2 && value <= 60)
                    {
                        
                    }
                    else
                    {
                        if (2 > value) tb_LogSaveMin.Text = "2";
                        else if (60 < value) tb_LogSaveMin.Text = "60";
                    }
                }
                else
                {
                    MessageBox.Show("숫자를 입력하세요.");
                }
            }
        }
#endif

        // BLDC 출력값 제한
        /*
                public const int DUTY_MAX = 100;

                private void numericUpDown1_ValueChanged(object sender, EventArgs e)
                {
                    if (DUTY_MAX < nud_bldc_duty2.Value) { nud_bldc_duty2.Value = DUTY_MAX; }
                }

                private void nud_bldc_duty1_ValueChanged(object sender, EventArgs e)
                {
                    if(DUTY_MAX < nud_bldc_duty1.Value) { nud_bldc_duty1.Value = DUTY_MAX; }
                }
        */

        //private void groupBox4_Enter(object sender, EventArgs e)
        //{
        //
        //}


        //---------------------------------
        // 낚시줄 에이징 처리

        // 1) Duty,Time설정
         private void nud_mmot_cw_duty1_ValueChanged(object sender, EventArgs e)
        {
            //nud_mmot_cw_duty1.Value
        }

        private void nud_mmot_cw_duty2_ValueChanged(object sender, EventArgs e)
        {
            // NA
        }

        public const int DUTY_MAX = 100;
        private void nud_bldc_duty1_ValueChanged(object sender, EventArgs e)
        {
            if (DUTY_MAX < nud_bldc_duty1.Value) { nud_bldc_duty1.Value = DUTY_MAX; }
        }

        private void nud_bldc_duty2_ValueChanged(object sender, EventArgs e)
        {
            if (DUTY_MAX < nud_bldc_duty2.Value) { nud_bldc_duty2.Value = DUTY_MAX; }
        }

        private void nud_mmot_cw_time1_ValueChanged(object sender, EventArgs e)
        {
            // NA
        }

        private void nud_mmot_cw_time2_ValueChanged(object sender, EventArgs e)
        {
            //NA
        }

        // 2) 버튼 처리
        //      초기 버튼, 초기값 설정
        private void btn_WireInitValue_Click(object sender, EventArgs e)
        {
            nud_mmot_cw_duty1.Value = 150;   // duty
            nud_bldc_duty1.Value = 20;   // duty
            nud_mmot_cw_time1.Value = 2000;  //ms
            nud_mmot_cw_duty2.Value = 80;   // duty
            nud_bldc_duty2.Value = 0;   // duty
            nud_mmot_cw_time2.Value = 3000;  //ms
        }

        // Wire에이징 동작 시작
        private void btn_wireAgingStart_Click(object sender, EventArgs e)
        {
            // kk=01, A=1 Start
            strMsg = "$98011" + "," + nud_mmot_cw_duty1.Text + "," + nud_bldc_duty1.Text + "," + nud_mmot_cw_time1.Text + "," + nud_mmot_cw_duty2.Text + "," + nud_bldc_duty2.Text + "," + nud_mmot_cw_time2.Text;  ////d1,t1,d2,t2,repeat회수(0=무한)
            strMsg += "%";
            uartSerial.WriteMessage(strMsg);  // 메인모터 최소출력(Duty)자동설정 요구
            tb_WireAgingResult.Text = "--";
            tb_WireAgingResult.BackColor = Color.LightGray;
        }

        // Wire에이징 동작 정지
        private void btn_wireAgingStop_Click(object sender, EventArgs e)
        {
            strMsg = "$98010"; // kk=01, A=0 Stop
            strMsg += "%";
            //strMsg += nud_mmot_cw_duty1.TextAlign + nud_mmot_cw_time1.TextAlign + nud_mmot_cw_duty2.TextAlign + nud_mmot_cw_time2.TextAlign; //d1,t1,d2,t2,repeat회수(0=무한)
            // 메인모터 최소출력(Duty)자동설정 요구
            uartSerial.WriteMessage(strMsg);
            tb_WireAgingResult.Text = "--";
            tb_WireAgingResult.BackColor = Color.LightGray;
            nud_AgingCounter.BackColor = Color.White;
        }

        // 모터 동시 제어 ON 버튼 클릭
        private void btn_on_motor_together_Click(object sender, EventArgs e)
        {
            // OFF TIme CLEAR
            tb_Torq_Off_time.Text = "--";          // Te:xxxx
            tb_Bldc_Off_time.Text = "--";           // Be:xxxx

            String sendCmd;
            // TORQUE ON 명령 송신
            if (0 == nud_time_together.Value)
            {
                sendCmd = "$02" + nud_TM_together.Value.ToString("000") + "%";   // 방향무조건 CW
                uartSerial.WriteMessage(sendCmd);
            }
            else
            {
                sendCmd = "$02" + nud_TM_together.Value.ToString("000") + nud_time_together.Value.ToString("0000") + "%";   // 방향무조건 CW
                uartSerial.WriteMessage(sendCmd);
            }
            // BLDC ON 명령 송신
            sendCmd = "$04" + "1" + nud_BM_together.Value.ToString("000") + nud_time_together.Value.ToString("0000") + "%";   // 방향무조건 CW
            uartSerial.WriteMessage(sendCmd);

        }

        // 모터 동시 제어 OFF 버튼 클릭
        private void btn_off_motor_together_Click(object sender, EventArgs e)
        {
            String sendCmd;
            // TORQUE OFF 명령 송신
            sendCmd = "$02000%";   // TORQUE 정지
            uartSerial.WriteMessage(sendCmd);
            // BLDC OFF 명령 송신
            sendCmd = "$0400000000%";   // BLDC 정지
            uartSerial.WriteMessage(sendCmd);

        }

        private void groupBox11_Enter(object sender, EventArgs e)
        {

        }

		private void tb_Poll_Interval_TextChanged(object sender, EventArgs e)
		{
			if (int.TryParse(tb_Poll_Interval.Text, out int value))
			{
                if (1 > value) {
                    tb_Poll_Interval.Text = "0";
                    value = 0;
                }
                else if (60 < value) {
                    tb_Poll_Interval.Text = "60";
                    value = 60;
                }
                Poll_Interval = value;
			}
			else
			{
				MessageBox.Show("숫자를 입력하세요.");
			}
		}

		private void label46_DoubleClick(object sender, EventArgs e)
		{
			Label clickedLabel = sender as Label;

            Poll_Filter = !Poll_Filter;
			if (Poll_Filter) {
				clickedLabel.BackColor = Color.Cyan;
//				clickedLabel.ForeColor = Color.White;
			}
            else {
				clickedLabel.BackColor = SystemColors.Control;
//				clickedLabel.ForeColor = SystemColors.ControlText;
			}
		}
		public void SendDownloadData(byte[] buffer, int offset, int count)
		{
			uartSerial.Write(buffer, offset, count);
		}

		public void WriteMessage(string text)
		{
			uartSerial.WriteMessage(text);
		}

		public bool IsSerialConnected()
		{
			return uartSerial.IsOpen;
		}

		private void btn_BoardDownload_Click(object sender, EventArgs e)
		{
			timer.Stop();
			using (var downloadForm = new DownloadForm())
			{
				if (uartSerial.IsOpen)
                {
					uartSerial.DownloadReceive += downloadForm.DownloadReceiveData;
				}
				downloadForm.Owner = this;          // ← 이거 필수!
				downloadForm.ShowDialog();          // 모달 팝업
                uartSerial.DownloadReceive -= downloadForm.DownloadReceiveData;
			}
			timer.Start();
		}

		private void cb_RTS_CheckedChanged(object sender, EventArgs e)
		{
            if (uartSerial.IsOpen)
                uartSerial.SignalConfig(cb_RTS.Checked, cb_DTR.Checked);
		}

		private void cb_DTR_CheckedChanged(object sender, EventArgs e)
		{
			if (uartSerial.IsOpen)
				uartSerial.SignalConfig(cb_RTS.Checked, cb_DTR.Checked);
		}
	}
}
