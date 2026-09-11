
namespace DF_TM
{
    partial class DFTMmainForm
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
			this.Btn_Port_Open = new System.Windows.Forms.Button();
			this.Cb_Ports = new System.Windows.Forms.ComboBox();
			this.label1 = new System.Windows.Forms.Label();
			this.Btn_VersionRead = new System.Windows.Forms.Button();
			this.Btn_GameStart = new System.Windows.Forms.Button();
			this.Tb_Drag_TorqVal = new System.Windows.Forms.TrackBar();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.cb_RTS = new System.Windows.Forms.CheckBox();
			this.cb_DTR = new System.Windows.Forms.CheckBox();
			this.Btn_Port_Close = new System.Windows.Forms.Button();
			this.sendCmd_Group = new System.Windows.Forms.GroupBox();
			this.Btn_Device_Check = new System.Windows.Forms.Button();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.btn_Torue_On = new System.Windows.Forms.Button();
			this.btn_Torque_OFF = new System.Windows.Forms.Button();
			this.numUpDown_Torque = new System.Windows.Forms.NumericUpDown();
			this.label28 = new System.Windows.Forms.Label();
			this.groupBox6 = new System.Windows.Forms.GroupBox();
			this.Lab_Btn_Right = new System.Windows.Forms.Label();
			this.Lab_Btn_Left = new System.Windows.Forms.Label();
			this.groupBox7 = new System.Windows.Forms.GroupBox();
			this.Lab_Rotate_Cnt = new System.Windows.Forms.Label();
			this.groupBox9 = new System.Windows.Forms.GroupBox();
			this.Btn_IMU_Off = new System.Windows.Forms.Button();
			this.Btn_IMU_On = new System.Windows.Forms.Button();
			this.Tb_Az = new System.Windows.Forms.TextBox();
			this.label15 = new System.Windows.Forms.Label();
			this.Tb_Ay = new System.Windows.Forms.TextBox();
			this.label16 = new System.Windows.Forms.Label();
			this.Tb_Ax = new System.Windows.Forms.TextBox();
			this.label17 = new System.Windows.Forms.Label();
			this.Tb_Yaw = new System.Windows.Forms.TextBox();
			this.label11 = new System.Windows.Forms.Label();
			this.Tb_Pitch = new System.Windows.Forms.TextBox();
			this.label10 = new System.Windows.Forms.Label();
			this.Tb_Roll = new System.Windows.Forms.TextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.groupBox10 = new System.Windows.Forms.GroupBox();
			this.tb_Bat_Charging = new System.Windows.Forms.TextBox();
			this.Lab_Battery = new System.Windows.Forms.Label();
			this.groupBox11 = new System.Windows.Forms.GroupBox();
			this.label12 = new System.Windows.Forms.Label();
			this.label9 = new System.Windows.Forms.Label();
			this.label7 = new System.Windows.Forms.Label();
			this.numUpDown_LedRead = new System.Windows.Forms.NumericUpDown();
			this.numUpDown_LedControlTime = new System.Windows.Forms.NumericUpDown();
			this.numUpDown_LedColorNo = new System.Windows.Forms.NumericUpDown();
			this.btn_LED_Off = new System.Windows.Forms.Button();
			this.label18 = new System.Windows.Forms.Label();
			this.lblLedColor = new System.Windows.Forms.Label();
			this.lblLedMode = new System.Windows.Forms.Label();
			this.lblLedPos = new System.Windows.Forms.Label();
			this.label13 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.btn_LED_Read = new System.Windows.Forms.Button();
			this.btn_LED_Write = new System.Windows.Forms.Button();
			this.btn_LED_On = new System.Windows.Forms.Button();
			this.numUpDown_LedPosNo = new System.Windows.Forms.NumericUpDown();
			this.numUpDown_LedModeNo = new System.Windows.Forms.NumericUpDown();
			this.updown_Bobbin_Duty = new System.Windows.Forms.NumericUpDown();
			this.updown_Bobbin_timems = new System.Windows.Forms.NumericUpDown();
			this.radio_BLDC_CW = new System.Windows.Forms.RadioButton();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.radio_BLDC_CCW = new System.Windows.Forms.RadioButton();
			this.btn_BLDC_On = new System.Windows.Forms.Button();
			this.groupBox5 = new System.Windows.Forms.GroupBox();
			this.btn_BLDC_Off = new System.Windows.Forms.Button();
			this.Tb_BLDC_OnTime = new System.Windows.Forms.TrackBar();
			this.Tb_BLDC_Duty = new System.Windows.Forms.TrackBar();
			this.label4 = new System.Windows.Forms.Label();
			this.lbl_CW_CCW = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.groupBox13 = new System.Windows.Forms.GroupBox();
			this.label21 = new System.Windows.Forms.Label();
			this.label22 = new System.Windows.Forms.Label();
			this.Lab_IMU_Conn = new System.Windows.Forms.Label();
			this.Lab_Slave_Conn = new System.Windows.Forms.Label();
			this.label25 = new System.Windows.Forms.Label();
			this.Lab_Main_Conn = new System.Windows.Forms.Label();
			this.groupBox8 = new System.Windows.Forms.GroupBox();
			this.Textbox_Rod_Ver = new System.Windows.Forms.TextBox();
			this.Textbox_Imu_Ver = new System.Windows.Forms.TextBox();
			this.Textbox_Main_Ver = new System.Windows.Forms.TextBox();
			this.tb_MainBoardVer = new System.Windows.Forms.TextBox();
			this.lb_BoardVer = new System.Windows.Forms.Label();
			this.label14 = new System.Windows.Forms.Label();
			this.Textbox_Error_Code = new System.Windows.Forms.TextBox();
			this.TextBox_Error_Content = new System.Windows.Forms.TextBox();
			this.groupBox14 = new System.Windows.Forms.GroupBox();
			this.btn_REEL_LED_right = new System.Windows.Forms.Button();
			this.btn_REEL_LED_left = new System.Windows.Forms.Button();
			this.btn_REEL_motor = new System.Windows.Forms.Button();
			this.btn_REEL_OFF = new System.Windows.Forms.Button();
			this.btn_REEL_ON = new System.Windows.Forms.Button();
			this.SendCmdGroup = new System.Windows.Forms.GroupBox();
			this.tb_CustomSend6 = new System.Windows.Forms.TextBox();
			this.tb_CustomSend5 = new System.Windows.Forms.TextBox();
			this.tb_CustomSend4 = new System.Windows.Forms.TextBox();
			this.tb_CustomSend3 = new System.Windows.Forms.TextBox();
			this.tb_CustomSend2 = new System.Windows.Forms.TextBox();
			this.tb_CustomSend1 = new System.Windows.Forms.TextBox();
			this.btn_Custom_send6 = new System.Windows.Forms.Button();
			this.btn_Custom_send5 = new System.Windows.Forms.Button();
			this.btn_Custom_send4 = new System.Windows.Forms.Button();
			this.btn_Custom_send3 = new System.Windows.Forms.Button();
			this.btn_Custom_send2 = new System.Windows.Forms.Button();
			this.btn_Custom_send1 = new System.Windows.Forms.Button();
			this.label30 = new System.Windows.Forms.Label();
			this.groupBox16 = new System.Windows.Forms.GroupBox();
			this.tb_Poll_Interval = new System.Windows.Forms.TextBox();
			this.label46 = new System.Windows.Forms.Label();
			this.tb_LogSaveMin = new System.Windows.Forms.TextBox();
			this.label38 = new System.Windows.Forms.Label();
			this.btn_log_autoscroll = new System.Windows.Forms.Button();
			this.btn_log_clear = new System.Windows.Forms.Button();
			this.btn_log_save = new System.Windows.Forms.Button();
			this.tb_LOG_Window = new System.Windows.Forms.RichTextBox();
			this.label27 = new System.Windows.Forms.Label();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.label19 = new System.Windows.Forms.Label();
			this.Lab_Boot = new System.Windows.Forms.Label();
			this.Lab_Setup = new System.Windows.Forms.Label();
			this.groupBox15 = new System.Windows.Forms.GroupBox();
			this.Lab_ImuInterval = new System.Windows.Forms.Label();
			this.groupBox12 = new System.Windows.Forms.GroupBox();
			this.label26 = new System.Windows.Forms.Label();
			this.Btn_Bbn_In_Stop = new System.Windows.Forms.Button();
			this.label31 = new System.Windows.Forms.Label();
			this.Cont_Pwr_Sw = new System.Windows.Forms.Label();
			this.Cont_usb5V = new System.Windows.Forms.Label();
			this.Btn_Bbn_In_Start = new System.Windows.Forms.Button();
			this.Cont_Boot_Sw = new System.Windows.Forms.Label();
			this.groupBox17 = new System.Windows.Forms.GroupBox();
			this.label32 = new System.Windows.Forms.Label();
			this.BbnFg = new System.Windows.Forms.Label();
			this.BbnEncB = new System.Windows.Forms.Label();
			this.BbnEncA = new System.Windows.Forms.Label();
			this.Btn_SubAc_Off = new System.Windows.Forms.Button();
			this.PowerSwitch = new System.Windows.Forms.GroupBox();
			this.Btn_SubAc_On = new System.Windows.Forms.Button();
			this.setupCmd_Group = new System.Windows.Forms.GroupBox();
			this.label34 = new System.Windows.Forms.Label();
			this.tb_ReelRegi_Result = new System.Windows.Forms.TextBox();
			this.btn_Reel_Regist = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.label35 = new System.Windows.Forms.Label();
			this.tb_MainAutoSet_result = new System.Windows.Forms.TextBox();
			this.btn_Mmot_Init = new System.Windows.Forms.Button();
			this.label24 = new System.Windows.Forms.Label();
			this.tb_MainMot_Default = new System.Windows.Forms.TextBox();
			this.btn_MainTorqAutoSet = new System.Windows.Forms.Button();
			this.btn_Mmot_Write = new System.Windows.Forms.Button();
			this.btn_Mmot_Read = new System.Windows.Forms.Button();
			this.ManufInfo_Group = new System.Windows.Forms.GroupBox();
			this.domainUpDown1 = new System.Windows.Forms.DomainUpDown();
			this.dud_SerialNumber = new System.Windows.Forms.DomainUpDown();
			this.domainUpDown2 = new System.Windows.Forms.DomainUpDown();
			this.tb_Manuf_No = new System.Windows.Forms.TextBox();
			this.domainUpDown3 = new System.Windows.Forms.DomainUpDown();
			this.label37 = new System.Windows.Forms.Label();
			this.dud_National = new System.Windows.Forms.DomainUpDown();
			this.dud_Service = new System.Windows.Forms.DomainUpDown();
			this.dud_Spec = new System.Windows.Forms.DomainUpDown();
			this.label36 = new System.Windows.Forms.Label();
			this.dud_config = new System.Windows.Forms.DomainUpDown();
			this.dud_person = new System.Windows.Forms.DomainUpDown();
			this.label29 = new System.Windows.Forms.Label();
			this.Btn_Manuf_Init = new System.Windows.Forms.Button();
			this.label20 = new System.Windows.Forms.Label();
			this.tb_Manuf_Date = new System.Windows.Forms.TextBox();
			this.tb_Manuf_SerialNo = new System.Windows.Forms.TextBox();
			this.tb_Manuf_Model = new System.Windows.Forms.TextBox();
			this.btn_Manuf_Read = new System.Windows.Forms.Button();
			this.btn_Manuf_Erase = new System.Windows.Forms.Button();
			this.btn_Manuf_Write = new System.Windows.Forms.Button();
			this.InstallGroup = new System.Windows.Forms.GroupBox();
			this.Btn_Install_Init = new System.Windows.Forms.Button();
			this.label23 = new System.Windows.Forms.Label();
			this.tb_Install_Date = new System.Windows.Forms.TextBox();
			this.btn_Install_Delete = new System.Windows.Forms.Button();
			this.btn_Install_Write = new System.Windows.Forms.Button();
			this.btn_Install_Read = new System.Windows.Forms.Button();
			this.pnl_reelPart = new System.Windows.Forms.Panel();
			this.panel1 = new System.Windows.Forms.Panel();
			this.panel2 = new System.Windows.Forms.Panel();
			this.panel3 = new System.Windows.Forms.Panel();
			this.groupBox18 = new System.Windows.Forms.GroupBox();
			this.btn_MainEnc_Init = new System.Windows.Forms.Button();
			this.btn_MainEncoder_StartStop = new System.Windows.Forms.Button();
			this.lb_MainEncDir = new System.Windows.Forms.Label();
			this.lb_MainEncCount = new System.Windows.Forms.Label();
			this.panel4 = new System.Windows.Forms.Panel();
			this.groupBox20 = new System.Windows.Forms.GroupBox();
			this.nud_AgingCounter = new System.Windows.Forms.NumericUpDown();
			this.btn_AgingCnt_Save = new System.Windows.Forms.Button();
			this.btn_AgingCnt_Clear0 = new System.Windows.Forms.Button();
			this.btn_AgingCnt_Read = new System.Windows.Forms.Button();
			this.groupBox19 = new System.Windows.Forms.GroupBox();
			this.groupBox22 = new System.Windows.Forms.GroupBox();
			this.nud_bldc_duty2 = new System.Windows.Forms.NumericUpDown();
			this.nud_bldc_duty1 = new System.Windows.Forms.NumericUpDown();
			this.label40 = new System.Windows.Forms.Label();
			this.label39 = new System.Windows.Forms.Label();
			this.label41 = new System.Windows.Forms.Label();
			this.nud_mmot_cw_time2 = new System.Windows.Forms.NumericUpDown();
			this.nud_mmot_cw_duty2 = new System.Windows.Forms.NumericUpDown();
			this.label42 = new System.Windows.Forms.Label();
			this.nud_mmot_cw_time1 = new System.Windows.Forms.NumericUpDown();
			this.nud_mmot_cw_duty1 = new System.Windows.Forms.NumericUpDown();
			this.btn_WireInitValue = new System.Windows.Forms.Button();
			this.groupBox21 = new System.Windows.Forms.GroupBox();
			this.lb_AgingResult = new System.Windows.Forms.Label();
			this.tb_WireAgingResult = new System.Windows.Forms.TextBox();
			this.tb_AgingResult = new System.Windows.Forms.TextBox();
			this.btn_wireAgingStop = new System.Windows.Forms.Button();
			this.btn_wireAgingStart = new System.Windows.Forms.Button();
			this.tb_Error_OkMsg = new System.Windows.Forms.TextBox();
			this.rTb_Error_ErrMsg = new System.Windows.Forms.RichTextBox();
			this.groupBox23 = new System.Windows.Forms.GroupBox();
			this.tb_Bldc_Off_time = new System.Windows.Forms.TextBox();
			this.tb_Torq_Off_time = new System.Windows.Forms.TextBox();
			this.label44 = new System.Windows.Forms.Label();
			this.nud_BM_together = new System.Windows.Forms.NumericUpDown();
			this.label45 = new System.Windows.Forms.Label();
			this.label43 = new System.Windows.Forms.Label();
			this.nud_time_together = new System.Windows.Forms.NumericUpDown();
			this.btn_on_motor_together = new System.Windows.Forms.Button();
			this.btn_off_motor_together = new System.Windows.Forms.Button();
			this.nud_TM_together = new System.Windows.Forms.NumericUpDown();
			this.label33 = new System.Windows.Forms.Label();
			this.btn_Download = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.Tb_Drag_TorqVal)).BeginInit();
			this.groupBox1.SuspendLayout();
			this.sendCmd_Group.SuspendLayout();
			this.groupBox3.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_Torque)).BeginInit();
			this.groupBox6.SuspendLayout();
			this.groupBox7.SuspendLayout();
			this.groupBox9.SuspendLayout();
			this.groupBox10.SuspendLayout();
			this.groupBox11.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedRead)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedControlTime)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedColorNo)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedPosNo)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedModeNo)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.updown_Bobbin_Duty)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.updown_Bobbin_timems)).BeginInit();
			this.groupBox5.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.Tb_BLDC_OnTime)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.Tb_BLDC_Duty)).BeginInit();
			this.groupBox13.SuspendLayout();
			this.groupBox8.SuspendLayout();
			this.groupBox14.SuspendLayout();
			this.SendCmdGroup.SuspendLayout();
			this.groupBox16.SuspendLayout();
			this.groupBox4.SuspendLayout();
			this.groupBox15.SuspendLayout();
			this.groupBox12.SuspendLayout();
			this.groupBox17.SuspendLayout();
			this.PowerSwitch.SuspendLayout();
			this.setupCmd_Group.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.ManufInfo_Group.SuspendLayout();
			this.InstallGroup.SuspendLayout();
			this.groupBox18.SuspendLayout();
			this.groupBox20.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.nud_AgingCounter)).BeginInit();
			this.groupBox19.SuspendLayout();
			this.groupBox22.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.nud_bldc_duty2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_bldc_duty1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_time2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_duty2)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_time1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_duty1)).BeginInit();
			this.groupBox23.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.nud_BM_together)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_time_together)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_TM_together)).BeginInit();
			this.SuspendLayout();
			// 
			// Btn_Port_Open
			// 
			this.Btn_Port_Open.Location = new System.Drawing.Point(4, 64);
			this.Btn_Port_Open.Name = "Btn_Port_Open";
			this.Btn_Port_Open.Size = new System.Drawing.Size(48, 27);
			this.Btn_Port_Open.TabIndex = 5;
			this.Btn_Port_Open.Text = "연결";
			this.Btn_Port_Open.UseVisualStyleBackColor = true;
			this.Btn_Port_Open.Click += new System.EventHandler(this.Btn_Port_Open_Click);
			// 
			// Cb_Ports
			// 
			this.Cb_Ports.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Cb_Ports.FormattingEnabled = true;
			this.Cb_Ports.Location = new System.Drawing.Point(45, 38);
			this.Cb_Ports.Name = "Cb_Ports";
			this.Cb_Ports.Size = new System.Drawing.Size(60, 20);
			this.Cb_Ports.TabIndex = 4;
			this.Cb_Ports.DropDown += new System.EventHandler(this.Cb_Ports_DropDown);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label1.Location = new System.Drawing.Point(4, 43);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(40, 12);
			this.label1.TabIndex = 3;
			this.label1.Text = "M포트";
			// 
			// Btn_VersionRead
			// 
			this.Btn_VersionRead.Location = new System.Drawing.Point(5, 40);
			this.Btn_VersionRead.Name = "Btn_VersionRead";
			this.Btn_VersionRead.Size = new System.Drawing.Size(75, 23);
			this.Btn_VersionRead.TabIndex = 7;
			this.Btn_VersionRead.TabStop = false;
			this.Btn_VersionRead.Text = "버전 읽기";
			this.Btn_VersionRead.UseVisualStyleBackColor = true;
			this.Btn_VersionRead.Click += new System.EventHandler(this.Btn_VersionRead_Click);
			// 
			// Btn_GameStart
			// 
			this.Btn_GameStart.Location = new System.Drawing.Point(5, 18);
			this.Btn_GameStart.Name = "Btn_GameStart";
			this.Btn_GameStart.Size = new System.Drawing.Size(75, 23);
			this.Btn_GameStart.TabIndex = 6;
			this.Btn_GameStart.Text = "게임 시작";
			this.Btn_GameStart.UseVisualStyleBackColor = true;
			this.Btn_GameStart.Click += new System.EventHandler(this.Btn_GameStart_Click);
			// 
			// Tb_Drag_TorqVal
			// 
			this.Tb_Drag_TorqVal.AutoSize = false;
			this.Tb_Drag_TorqVal.LargeChange = 15;
			this.Tb_Drag_TorqVal.Location = new System.Drawing.Point(6, 29);
			this.Tb_Drag_TorqVal.Maximum = 255;
			this.Tb_Drag_TorqVal.Name = "Tb_Drag_TorqVal";
			this.Tb_Drag_TorqVal.Size = new System.Drawing.Size(286, 23);
			this.Tb_Drag_TorqVal.SmallChange = 10;
			this.Tb_Drag_TorqVal.TabIndex = 9;
			this.Tb_Drag_TorqVal.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Tb_Torque_Val_KeyUp);
			this.Tb_Drag_TorqVal.MouseUp += new System.Windows.Forms.MouseEventHandler(this.Tb_Torque_Val_MouseUp);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.cb_RTS);
			this.groupBox1.Controls.Add(this.cb_DTR);
			this.groupBox1.Controls.Add(this.Cb_Ports);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.Btn_Port_Close);
			this.groupBox1.Controls.Add(this.Btn_Port_Open);
			this.groupBox1.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox1.Location = new System.Drawing.Point(12, 11);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(109, 104);
			this.groupBox1.TabIndex = 12;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "메인통신UART";
			// 
			// cb_RTS
			// 
			this.cb_RTS.AutoSize = true;
			this.cb_RTS.Checked = true;
			this.cb_RTS.CheckState = System.Windows.Forms.CheckState.Checked;
			this.cb_RTS.Enabled = false;
			this.cb_RTS.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.cb_RTS.Location = new System.Drawing.Point(4, 18);
			this.cb_RTS.Name = "cb_RTS";
			this.cb_RTS.Size = new System.Drawing.Size(51, 16);
			this.cb_RTS.TabIndex = 93;
			this.cb_RTS.Text = "RTS";
			this.cb_RTS.UseVisualStyleBackColor = true;
			this.cb_RTS.CheckedChanged += new System.EventHandler(this.cb_RTS_CheckedChanged);
			// 
			// cb_DTR
			// 
			this.cb_DTR.AutoSize = true;
			this.cb_DTR.Enabled = false;
			this.cb_DTR.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.cb_DTR.Location = new System.Drawing.Point(56, 18);
			this.cb_DTR.Name = "cb_DTR";
			this.cb_DTR.Size = new System.Drawing.Size(51, 16);
			this.cb_DTR.TabIndex = 93;
			this.cb_DTR.Text = "DTR";
			this.cb_DTR.UseVisualStyleBackColor = true;
			this.cb_DTR.CheckedChanged += new System.EventHandler(this.cb_DTR_CheckedChanged);
			// 
			// Btn_Port_Close
			// 
			this.Btn_Port_Close.Location = new System.Drawing.Point(53, 64);
			this.Btn_Port_Close.Name = "Btn_Port_Close";
			this.Btn_Port_Close.Size = new System.Drawing.Size(52, 27);
			this.Btn_Port_Close.TabIndex = 5;
			this.Btn_Port_Close.Text = "끊음";
			this.Btn_Port_Close.UseVisualStyleBackColor = true;
			this.Btn_Port_Close.Click += new System.EventHandler(this.Btn_Port_Close_Click);
			// 
			// sendCmd_Group
			// 
			this.sendCmd_Group.Controls.Add(this.Btn_GameStart);
			this.sendCmd_Group.Controls.Add(this.Btn_VersionRead);
			this.sendCmd_Group.Controls.Add(this.Btn_Device_Check);
			this.sendCmd_Group.Location = new System.Drawing.Point(123, 11);
			this.sendCmd_Group.Name = "sendCmd_Group";
			this.sendCmd_Group.Size = new System.Drawing.Size(147, 461);
			this.sendCmd_Group.TabIndex = 13;
			this.sendCmd_Group.TabStop = false;
			this.sendCmd_Group.Text = "명령 송신";
			// 
			// Btn_Device_Check
			// 
			this.Btn_Device_Check.Location = new System.Drawing.Point(5, 64);
			this.Btn_Device_Check.Name = "Btn_Device_Check";
			this.Btn_Device_Check.Size = new System.Drawing.Size(75, 23);
			this.Btn_Device_Check.TabIndex = 79;
			this.Btn_Device_Check.TabStop = false;
			this.Btn_Device_Check.Text = "장치 확인";
			this.Btn_Device_Check.UseVisualStyleBackColor = true;
			this.Btn_Device_Check.Click += new System.EventHandler(this.Btn_Device_Check_Click);
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.btn_Torue_On);
			this.groupBox3.Controls.Add(this.btn_Torque_OFF);
			this.groupBox3.Controls.Add(this.numUpDown_Torque);
			this.groupBox3.Controls.Add(this.Tb_Drag_TorqVal);
			this.groupBox3.Controls.Add(this.label28);
			this.groupBox3.Location = new System.Drawing.Point(450, 12);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(298, 52);
			this.groupBox3.TabIndex = 14;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "메인 모터(토크)";
			// 
			// btn_Torue_On
			// 
			this.btn_Torue_On.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_Torue_On.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_Torue_On.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Torue_On.Location = new System.Drawing.Point(184, 9);
			this.btn_Torue_On.Name = "btn_Torue_On";
			this.btn_Torue_On.Size = new System.Drawing.Size(51, 25);
			this.btn_Torue_On.TabIndex = 14;
			this.btn_Torue_On.Text = "ON";
			this.btn_Torue_On.UseVisualStyleBackColor = false;
			this.btn_Torue_On.Click += new System.EventHandler(this.btn_Torue_On_Click);
			// 
			// btn_Torque_OFF
			// 
			this.btn_Torque_OFF.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_Torque_OFF.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_Torque_OFF.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Torque_OFF.Location = new System.Drawing.Point(241, 9);
			this.btn_Torque_OFF.Name = "btn_Torque_OFF";
			this.btn_Torque_OFF.Size = new System.Drawing.Size(50, 25);
			this.btn_Torque_OFF.TabIndex = 6;
			this.btn_Torque_OFF.Text = "OFF";
			this.btn_Torque_OFF.UseVisualStyleBackColor = false;
			this.btn_Torque_OFF.Click += new System.EventHandler(this.Btn_Torque_Off_Click);
			// 
			// numUpDown_Torque
			// 
			this.numUpDown_Torque.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_Torque.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_Torque.Location = new System.Drawing.Point(102, 12);
			this.numUpDown_Torque.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.numUpDown_Torque.Name = "numUpDown_Torque";
			this.numUpDown_Torque.Size = new System.Drawing.Size(70, 22);
			this.numUpDown_Torque.TabIndex = 13;
			this.numUpDown_Torque.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_Torque.ValueChanged += new System.EventHandler(this.numUpDown_Torque_ValueChanged);
			// 
			// label28
			// 
			this.label28.AutoSize = true;
			this.label28.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label28.Location = new System.Drawing.Point(69, 15);
			this.label28.Name = "label28";
			this.label28.Size = new System.Drawing.Size(30, 12);
			this.label28.TabIndex = 3;
			this.label28.Text = "Duty";
			// 
			// groupBox6
			// 
			this.groupBox6.Controls.Add(this.Lab_Btn_Right);
			this.groupBox6.Controls.Add(this.Lab_Btn_Left);
			this.groupBox6.Location = new System.Drawing.Point(754, 13);
			this.groupBox6.Name = "groupBox6";
			this.groupBox6.Size = new System.Drawing.Size(111, 47);
			this.groupBox6.TabIndex = 17;
			this.groupBox6.TabStop = false;
			this.groupBox6.Text = "릴 버튼";
			// 
			// Lab_Btn_Right
			// 
			this.Lab_Btn_Right.BackColor = System.Drawing.Color.Gray;
			this.Lab_Btn_Right.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Lab_Btn_Right.Location = new System.Drawing.Point(46, 17);
			this.Lab_Btn_Right.Name = "Lab_Btn_Right";
			this.Lab_Btn_Right.Size = new System.Drawing.Size(28, 18);
			this.Lab_Btn_Right.TabIndex = 1;
			// 
			// Lab_Btn_Left
			// 
			this.Lab_Btn_Left.BackColor = System.Drawing.Color.Gray;
			this.Lab_Btn_Left.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Lab_Btn_Left.Location = new System.Drawing.Point(12, 17);
			this.Lab_Btn_Left.Name = "Lab_Btn_Left";
			this.Lab_Btn_Left.Size = new System.Drawing.Size(28, 18);
			this.Lab_Btn_Left.TabIndex = 0;
			// 
			// groupBox7
			// 
			this.groupBox7.Controls.Add(this.Lab_Rotate_Cnt);
			this.groupBox7.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox7.Location = new System.Drawing.Point(754, 61);
			this.groupBox7.Name = "groupBox7";
			this.groupBox7.Size = new System.Drawing.Size(112, 40);
			this.groupBox7.TabIndex = 18;
			this.groupBox7.TabStop = false;
			this.groupBox7.Text = "릴 핸들카운트";
			// 
			// Lab_Rotate_Cnt
			// 
			this.Lab_Rotate_Cnt.AutoSize = true;
			this.Lab_Rotate_Cnt.Font = new System.Drawing.Font("굴림", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Lab_Rotate_Cnt.Location = new System.Drawing.Point(35, 18);
			this.Lab_Rotate_Cnt.Name = "Lab_Rotate_Cnt";
			this.Lab_Rotate_Cnt.Size = new System.Drawing.Size(34, 16);
			this.Lab_Rotate_Cnt.TabIndex = 0;
			this.Lab_Rotate_Cnt.Text = "000";
			this.Lab_Rotate_Cnt.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// groupBox9
			// 
			this.groupBox9.Controls.Add(this.Btn_IMU_Off);
			this.groupBox9.Controls.Add(this.Btn_IMU_On);
			this.groupBox9.Controls.Add(this.Tb_Az);
			this.groupBox9.Controls.Add(this.label15);
			this.groupBox9.Controls.Add(this.Tb_Ay);
			this.groupBox9.Controls.Add(this.label16);
			this.groupBox9.Controls.Add(this.Tb_Ax);
			this.groupBox9.Controls.Add(this.label17);
			this.groupBox9.Controls.Add(this.Tb_Yaw);
			this.groupBox9.Controls.Add(this.label11);
			this.groupBox9.Controls.Add(this.Tb_Pitch);
			this.groupBox9.Controls.Add(this.label10);
			this.groupBox9.Controls.Add(this.Tb_Roll);
			this.groupBox9.Controls.Add(this.label8);
			this.groupBox9.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox9.Location = new System.Drawing.Point(863, 13);
			this.groupBox9.Name = "groupBox9";
			this.groupBox9.Size = new System.Drawing.Size(108, 192);
			this.groupBox9.TabIndex = 19;
			this.groupBox9.TabStop = false;
			this.groupBox9.Text = "IMU센서 데이타";
			// 
			// Btn_IMU_Off
			// 
			this.Btn_IMU_Off.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_IMU_Off.Location = new System.Drawing.Point(62, 161);
			this.Btn_IMU_Off.Name = "Btn_IMU_Off";
			this.Btn_IMU_Off.Size = new System.Drawing.Size(40, 23);
			this.Btn_IMU_Off.TabIndex = 19;
			this.Btn_IMU_Off.Text = "OFF";
			this.Btn_IMU_Off.UseVisualStyleBackColor = true;
			this.Btn_IMU_Off.Click += new System.EventHandler(this.Btn_IMU_Off_Click);
			// 
			// Btn_IMU_On
			// 
			this.Btn_IMU_On.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_IMU_On.Location = new System.Drawing.Point(9, 161);
			this.Btn_IMU_On.Name = "Btn_IMU_On";
			this.Btn_IMU_On.Size = new System.Drawing.Size(50, 23);
			this.Btn_IMU_On.TabIndex = 8;
			this.Btn_IMU_On.Text = "ON";
			this.Btn_IMU_On.UseVisualStyleBackColor = true;
			this.Btn_IMU_On.Click += new System.EventHandler(this.Btn_IMU_On_Click);
			// 
			// Tb_Az
			// 
			this.Tb_Az.Location = new System.Drawing.Point(38, 133);
			this.Tb_Az.Name = "Tb_Az";
			this.Tb_Az.Size = new System.Drawing.Size(64, 21);
			this.Tb_Az.TabIndex = 18;
			// 
			// label15
			// 
			this.label15.AutoSize = true;
			this.label15.Location = new System.Drawing.Point(9, 135);
			this.label15.Name = "label15";
			this.label15.Size = new System.Drawing.Size(21, 12);
			this.label15.TabIndex = 17;
			this.label15.Text = "AZ";
			// 
			// Tb_Ay
			// 
			this.Tb_Ay.Location = new System.Drawing.Point(38, 109);
			this.Tb_Ay.Name = "Tb_Ay";
			this.Tb_Ay.Size = new System.Drawing.Size(64, 21);
			this.Tb_Ay.TabIndex = 16;
			// 
			// label16
			// 
			this.label16.AutoSize = true;
			this.label16.Location = new System.Drawing.Point(11, 114);
			this.label16.Name = "label16";
			this.label16.Size = new System.Drawing.Size(21, 12);
			this.label16.TabIndex = 15;
			this.label16.Text = "AY";
			// 
			// Tb_Ax
			// 
			this.Tb_Ax.Location = new System.Drawing.Point(37, 85);
			this.Tb_Ax.Name = "Tb_Ax";
			this.Tb_Ax.Size = new System.Drawing.Size(65, 21);
			this.Tb_Ax.TabIndex = 14;
			// 
			// label17
			// 
			this.label17.AutoSize = true;
			this.label17.Location = new System.Drawing.Point(10, 90);
			this.label17.Name = "label17";
			this.label17.Size = new System.Drawing.Size(21, 12);
			this.label17.TabIndex = 13;
			this.label17.Text = "AX";
			// 
			// Tb_Yaw
			// 
			this.Tb_Yaw.Location = new System.Drawing.Point(37, 61);
			this.Tb_Yaw.Name = "Tb_Yaw";
			this.Tb_Yaw.Size = new System.Drawing.Size(65, 21);
			this.Tb_Yaw.TabIndex = 6;
			// 
			// label11
			// 
			this.label11.AutoSize = true;
			this.label11.Location = new System.Drawing.Point(3, 66);
			this.label11.Name = "label11";
			this.label11.Size = new System.Drawing.Size(30, 12);
			this.label11.TabIndex = 5;
			this.label11.Text = "Yaw";
			// 
			// Tb_Pitch
			// 
			this.Tb_Pitch.Location = new System.Drawing.Point(37, 38);
			this.Tb_Pitch.Name = "Tb_Pitch";
			this.Tb_Pitch.Size = new System.Drawing.Size(65, 21);
			this.Tb_Pitch.TabIndex = 4;
			// 
			// label10
			// 
			this.label10.AutoSize = true;
			this.label10.Location = new System.Drawing.Point(2, 43);
			this.label10.Name = "label10";
			this.label10.Size = new System.Drawing.Size(33, 12);
			this.label10.TabIndex = 3;
			this.label10.Text = "Pitch";
			// 
			// Tb_Roll
			// 
			this.Tb_Roll.Location = new System.Drawing.Point(37, 16);
			this.Tb_Roll.Name = "Tb_Roll";
			this.Tb_Roll.Size = new System.Drawing.Size(65, 21);
			this.Tb_Roll.TabIndex = 2;
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(8, 20);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(26, 12);
			this.label8.TabIndex = 0;
			this.label8.Text = "Roll";
			// 
			// groupBox10
			// 
			this.groupBox10.Controls.Add(this.tb_Bat_Charging);
			this.groupBox10.Controls.Add(this.Lab_Battery);
			this.groupBox10.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox10.Location = new System.Drawing.Point(754, 104);
			this.groupBox10.Name = "groupBox10";
			this.groupBox10.Size = new System.Drawing.Size(111, 43);
			this.groupBox10.TabIndex = 19;
			this.groupBox10.TabStop = false;
			this.groupBox10.Text = "릴 배터리잔량%";
			// 
			// tb_Bat_Charging
			// 
			this.tb_Bat_Charging.Font = new System.Drawing.Font("굴림", 11.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Bat_Charging.Location = new System.Drawing.Point(54, 16);
			this.tb_Bat_Charging.Name = "tb_Bat_Charging";
			this.tb_Bat_Charging.Size = new System.Drawing.Size(42, 25);
			this.tb_Bat_Charging.TabIndex = 38;
			this.tb_Bat_Charging.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// Lab_Battery
			// 
			this.Lab_Battery.AutoSize = true;
			this.Lab_Battery.Font = new System.Drawing.Font("굴림", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Lab_Battery.Location = new System.Drawing.Point(19, 20);
			this.Lab_Battery.Name = "Lab_Battery";
			this.Lab_Battery.Size = new System.Drawing.Size(34, 16);
			this.Lab_Battery.TabIndex = 0;
			this.Lab_Battery.Text = "000";
			// 
			// groupBox11
			// 
			this.groupBox11.Controls.Add(this.label12);
			this.groupBox11.Controls.Add(this.label9);
			this.groupBox11.Controls.Add(this.label7);
			this.groupBox11.Controls.Add(this.numUpDown_LedRead);
			this.groupBox11.Controls.Add(this.numUpDown_LedControlTime);
			this.groupBox11.Controls.Add(this.numUpDown_LedColorNo);
			this.groupBox11.Controls.Add(this.btn_LED_Off);
			this.groupBox11.Controls.Add(this.label18);
			this.groupBox11.Controls.Add(this.lblLedColor);
			this.groupBox11.Controls.Add(this.lblLedMode);
			this.groupBox11.Controls.Add(this.lblLedPos);
			this.groupBox11.Controls.Add(this.label13);
			this.groupBox11.Controls.Add(this.label5);
			this.groupBox11.Controls.Add(this.btn_LED_Read);
			this.groupBox11.Controls.Add(this.btn_LED_Write);
			this.groupBox11.Controls.Add(this.btn_LED_On);
			this.groupBox11.Controls.Add(this.numUpDown_LedPosNo);
			this.groupBox11.Controls.Add(this.numUpDown_LedModeNo);
			this.groupBox11.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.groupBox11.Location = new System.Drawing.Point(448, 247);
			this.groupBox11.Name = "groupBox11";
			this.groupBox11.Size = new System.Drawing.Size(298, 144);
			this.groupBox11.TabIndex = 16;
			this.groupBox11.TabStop = false;
			this.groupBox11.Text = "LED 출력";
			this.groupBox11.Enter += new System.EventHandler(this.groupBox11_Enter);
			// 
			// label12
			// 
			this.label12.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.label12.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label12.Location = new System.Drawing.Point(115, 60);
			this.label12.Name = "label12";
			this.label12.Size = new System.Drawing.Size(89, 18);
			this.label12.TabIndex = 20;
			this.label12.Text = "표시시간";
			this.label12.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label9
			// 
			this.label9.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.label9.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label9.Location = new System.Drawing.Point(6, 60);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(110, 18);
			this.label9.TabIndex = 19;
			this.label9.Text = "색깔번호/개수";
			this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label7
			// 
			this.label7.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.label7.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label7.Location = new System.Drawing.Point(115, 13);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(89, 18);
			this.label7.TabIndex = 18;
			this.label7.Text = "표시 형태";
			this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// numUpDown_LedRead
			// 
			this.numUpDown_LedRead.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_LedRead.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_LedRead.Location = new System.Drawing.Point(117, 113);
			this.numUpDown_LedRead.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.numUpDown_LedRead.Name = "numUpDown_LedRead";
			this.numUpDown_LedRead.Size = new System.Drawing.Size(84, 22);
			this.numUpDown_LedRead.TabIndex = 17;
			this.numUpDown_LedRead.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_LedRead.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
			this.numUpDown_LedRead.ValueChanged += new System.EventHandler(this.numUpDown_LedRead_ValueChanged);
			// 
			// numUpDown_LedControlTime
			// 
			this.numUpDown_LedControlTime.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_LedControlTime.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_LedControlTime.Location = new System.Drawing.Point(115, 79);
			this.numUpDown_LedControlTime.Maximum = new decimal(new int[] {
            9999,
            0,
            0,
            0});
			this.numUpDown_LedControlTime.Name = "numUpDown_LedControlTime";
			this.numUpDown_LedControlTime.Size = new System.Drawing.Size(89, 22);
			this.numUpDown_LedControlTime.TabIndex = 17;
			this.numUpDown_LedControlTime.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_LedControlTime.Value = new decimal(new int[] {
            1000,
            0,
            0,
            0});
			this.numUpDown_LedControlTime.ValueChanged += new System.EventHandler(this.numUpDown_LedControlTime_ValueChanged);
			// 
			// numUpDown_LedColorNo
			// 
			this.numUpDown_LedColorNo.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_LedColorNo.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_LedColorNo.Location = new System.Drawing.Point(6, 79);
			this.numUpDown_LedColorNo.Maximum = new decimal(new int[] {
            15,
            0,
            0,
            0});
			this.numUpDown_LedColorNo.Name = "numUpDown_LedColorNo";
			this.numUpDown_LedColorNo.Size = new System.Drawing.Size(48, 22);
			this.numUpDown_LedColorNo.TabIndex = 15;
			this.numUpDown_LedColorNo.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_LedColorNo.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numUpDown_LedColorNo.ValueChanged += new System.EventHandler(this.numUpDown_LedColorNo_ValueChanged);
			// 
			// btn_LED_Off
			// 
			this.btn_LED_Off.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_LED_Off.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_LED_Off.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_LED_Off.Location = new System.Drawing.Point(235, 52);
			this.btn_LED_Off.Name = "btn_LED_Off";
			this.btn_LED_Off.Size = new System.Drawing.Size(50, 28);
			this.btn_LED_Off.TabIndex = 6;
			this.btn_LED_Off.Text = "OFF";
			this.btn_LED_Off.UseVisualStyleBackColor = false;
			this.btn_LED_Off.Click += new System.EventHandler(this.Btn_LED_Off_Click);
			// 
			// label18
			// 
			this.label18.AutoSize = true;
			this.label18.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label18.Location = new System.Drawing.Point(205, 83);
			this.label18.Name = "label18";
			this.label18.Size = new System.Drawing.Size(23, 12);
			this.label18.TabIndex = 3;
			this.label18.Text = "ms";
			// 
			// lblLedColor
			// 
			this.lblLedColor.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lblLedColor.Location = new System.Drawing.Point(59, 81);
			this.lblLedColor.Name = "lblLedColor";
			this.lblLedColor.Size = new System.Drawing.Size(57, 18);
			this.lblLedColor.TabIndex = 3;
			this.lblLedColor.Text = "Red";
			this.lblLedColor.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblLedMode
			// 
			this.lblLedMode.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lblLedMode.Location = new System.Drawing.Point(162, 35);
			this.lblLedMode.Name = "lblLedMode";
			this.lblLedMode.Size = new System.Drawing.Size(42, 18);
			this.lblLedMode.TabIndex = 3;
			this.lblLedMode.Text = "OFF";
			this.lblLedMode.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lblLedPos
			// 
			this.lblLedPos.AutoSize = true;
			this.lblLedPos.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lblLedPos.Location = new System.Drawing.Point(58, 38);
			this.lblLedPos.Name = "lblLedPos";
			this.lblLedPos.Size = new System.Drawing.Size(31, 12);
			this.lblLedPos.TabIndex = 3;
			this.lblLedPos.Text = "중앙";
			// 
			// label13
			// 
			this.label13.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.label13.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label13.Location = new System.Drawing.Point(51, 113);
			this.label13.Name = "label13";
			this.label13.Size = new System.Drawing.Size(66, 20);
			this.label13.TabIndex = 3;
			this.label13.Text = "LED밝기";
			this.label13.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// label5
			// 
			this.label5.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.label5.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label5.Location = new System.Drawing.Point(6, 13);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(110, 18);
			this.label5.TabIndex = 3;
			this.label5.Text = "LED 위치";
			this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// btn_LED_Read
			// 
			this.btn_LED_Read.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_LED_Read.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_LED_Read.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_LED_Read.Location = new System.Drawing.Point(202, 111);
			this.btn_LED_Read.Name = "btn_LED_Read";
			this.btn_LED_Read.Size = new System.Drawing.Size(43, 25);
			this.btn_LED_Read.TabIndex = 6;
			this.btn_LED_Read.Text = "읽기";
			this.btn_LED_Read.UseVisualStyleBackColor = false;
			this.btn_LED_Read.Click += new System.EventHandler(this.btn_LED_Read_Click);
			// 
			// btn_LED_Write
			// 
			this.btn_LED_Write.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_LED_Write.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_LED_Write.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_LED_Write.Location = new System.Drawing.Point(245, 111);
			this.btn_LED_Write.Name = "btn_LED_Write";
			this.btn_LED_Write.Size = new System.Drawing.Size(40, 25);
			this.btn_LED_Write.TabIndex = 6;
			this.btn_LED_Write.Text = "쓰기";
			this.btn_LED_Write.UseVisualStyleBackColor = false;
			this.btn_LED_Write.Click += new System.EventHandler(this.btn_LED_Write_Click);
			// 
			// btn_LED_On
			// 
			this.btn_LED_On.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_LED_On.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_LED_On.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_LED_On.Location = new System.Drawing.Point(235, 25);
			this.btn_LED_On.Name = "btn_LED_On";
			this.btn_LED_On.Size = new System.Drawing.Size(50, 28);
			this.btn_LED_On.TabIndex = 6;
			this.btn_LED_On.Text = "ON";
			this.btn_LED_On.UseVisualStyleBackColor = false;
			this.btn_LED_On.Click += new System.EventHandler(this.Btn_LED_On_Click);
			// 
			// numUpDown_LedPosNo
			// 
			this.numUpDown_LedPosNo.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_LedPosNo.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_LedPosNo.Location = new System.Drawing.Point(6, 32);
			this.numUpDown_LedPosNo.Maximum = new decimal(new int[] {
            4,
            0,
            0,
            0});
			this.numUpDown_LedPosNo.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numUpDown_LedPosNo.Name = "numUpDown_LedPosNo";
			this.numUpDown_LedPosNo.Size = new System.Drawing.Size(48, 22);
			this.numUpDown_LedPosNo.TabIndex = 13;
			this.numUpDown_LedPosNo.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_LedPosNo.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numUpDown_LedPosNo.ValueChanged += new System.EventHandler(this.numUpDown_LedPosNo_ValueChanged);
			// 
			// numUpDown_LedModeNo
			// 
			this.numUpDown_LedModeNo.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.numUpDown_LedModeNo.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.numUpDown_LedModeNo.Location = new System.Drawing.Point(115, 33);
			this.numUpDown_LedModeNo.Maximum = new decimal(new int[] {
            9,
            0,
            0,
            0});
			this.numUpDown_LedModeNo.Name = "numUpDown_LedModeNo";
			this.numUpDown_LedModeNo.Size = new System.Drawing.Size(41, 22);
			this.numUpDown_LedModeNo.TabIndex = 13;
			this.numUpDown_LedModeNo.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.numUpDown_LedModeNo.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.numUpDown_LedModeNo.ValueChanged += new System.EventHandler(this.numUpDown_LedModeNo_ValueChanged);
			// 
			// updown_Bobbin_Duty
			// 
			this.updown_Bobbin_Duty.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.updown_Bobbin_Duty.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.updown_Bobbin_Duty.Location = new System.Drawing.Point(102, 9);
			this.updown_Bobbin_Duty.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.updown_Bobbin_Duty.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
			this.updown_Bobbin_Duty.Name = "updown_Bobbin_Duty";
			this.updown_Bobbin_Duty.Size = new System.Drawing.Size(69, 22);
			this.updown_Bobbin_Duty.TabIndex = 13;
			this.updown_Bobbin_Duty.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.updown_Bobbin_Duty.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
			this.updown_Bobbin_Duty.ValueChanged += new System.EventHandler(this.numUpDown_Duty_ValueChanged);
			// 
			// updown_Bobbin_timems
			// 
			this.updown_Bobbin_timems.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.updown_Bobbin_timems.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.updown_Bobbin_timems.Location = new System.Drawing.Point(102, 29);
			this.updown_Bobbin_timems.Maximum = new decimal(new int[] {
            9999,
            0,
            0,
            0});
			this.updown_Bobbin_timems.Name = "updown_Bobbin_timems";
			this.updown_Bobbin_timems.Size = new System.Drawing.Size(69, 22);
			this.updown_Bobbin_timems.TabIndex = 13;
			this.updown_Bobbin_timems.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.updown_Bobbin_timems.ValueChanged += new System.EventHandler(this.numUpDown_OnTime_ValueChanged);
			// 
			// radio_BLDC_CW
			// 
			this.radio_BLDC_CW.AutoSize = true;
			this.radio_BLDC_CW.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.radio_BLDC_CW.Location = new System.Drawing.Point(182, 12);
			this.radio_BLDC_CW.Name = "radio_BLDC_CW";
			this.radio_BLDC_CW.Size = new System.Drawing.Size(44, 16);
			this.radio_BLDC_CW.TabIndex = 14;
			this.radio_BLDC_CW.TabStop = true;
			this.radio_BLDC_CW.Text = "CW";
			this.radio_BLDC_CW.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			this.radio_BLDC_CW.UseVisualStyleBackColor = true;
			this.radio_BLDC_CW.CheckedChanged += new System.EventHandler(this.radio_BLDC_CW_CheckedChanged);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label2.Location = new System.Drawing.Point(69, 14);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(30, 12);
			this.label2.TabIndex = 3;
			this.label2.Text = "Duty";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label3.Location = new System.Drawing.Point(24, 31);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(78, 12);
			this.label3.TabIndex = 3;
			this.label3.Text = "OnTime(ms)";
			// 
			// radio_BLDC_CCW
			// 
			this.radio_BLDC_CCW.AutoSize = true;
			this.radio_BLDC_CCW.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.radio_BLDC_CCW.Location = new System.Drawing.Point(182, 32);
			this.radio_BLDC_CCW.Name = "radio_BLDC_CCW";
			this.radio_BLDC_CCW.Size = new System.Drawing.Size(54, 16);
			this.radio_BLDC_CCW.TabIndex = 14;
			this.radio_BLDC_CCW.TabStop = true;
			this.radio_BLDC_CCW.Text = "CCW";
			this.radio_BLDC_CCW.UseVisualStyleBackColor = true;
			this.radio_BLDC_CCW.CheckedChanged += new System.EventHandler(this.radio_BLDC_CCW_CheckedChanged);
			// 
			// btn_BLDC_On
			// 
			this.btn_BLDC_On.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_BLDC_On.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_BLDC_On.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_BLDC_On.Location = new System.Drawing.Point(241, 6);
			this.btn_BLDC_On.Name = "btn_BLDC_On";
			this.btn_BLDC_On.Size = new System.Drawing.Size(51, 21);
			this.btn_BLDC_On.TabIndex = 6;
			this.btn_BLDC_On.Text = "ON";
			this.btn_BLDC_On.UseVisualStyleBackColor = false;
			this.btn_BLDC_On.Click += new System.EventHandler(this.Btn_Bldc_On_Click);
			// 
			// groupBox5
			// 
			this.groupBox5.Controls.Add(this.btn_BLDC_Off);
			this.groupBox5.Controls.Add(this.btn_BLDC_On);
			this.groupBox5.Controls.Add(this.Tb_BLDC_OnTime);
			this.groupBox5.Controls.Add(this.Tb_BLDC_Duty);
			this.groupBox5.Controls.Add(this.radio_BLDC_CCW);
			this.groupBox5.Controls.Add(this.label4);
			this.groupBox5.Controls.Add(this.label3);
			this.groupBox5.Controls.Add(this.lbl_CW_CCW);
			this.groupBox5.Controls.Add(this.label6);
			this.groupBox5.Controls.Add(this.label2);
			this.groupBox5.Controls.Add(this.radio_BLDC_CW);
			this.groupBox5.Controls.Add(this.updown_Bobbin_timems);
			this.groupBox5.Controls.Add(this.updown_Bobbin_Duty);
			this.groupBox5.Location = new System.Drawing.Point(450, 71);
			this.groupBox5.Name = "groupBox5";
			this.groupBox5.Size = new System.Drawing.Size(298, 101);
			this.groupBox5.TabIndex = 16;
			this.groupBox5.TabStop = false;
			this.groupBox5.Text = "보빈 모터(bldc)";
			// 
			// btn_BLDC_Off
			// 
			this.btn_BLDC_Off.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_BLDC_Off.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_BLDC_Off.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_BLDC_Off.Location = new System.Drawing.Point(241, 28);
			this.btn_BLDC_Off.Name = "btn_BLDC_Off";
			this.btn_BLDC_Off.Size = new System.Drawing.Size(51, 22);
			this.btn_BLDC_Off.TabIndex = 6;
			this.btn_BLDC_Off.Text = "OFF";
			this.btn_BLDC_Off.UseVisualStyleBackColor = false;
			this.btn_BLDC_Off.Click += new System.EventHandler(this.Btn_Bldc_Off_Click);
			// 
			// Tb_BLDC_OnTime
			// 
			this.Tb_BLDC_OnTime.AutoSize = false;
			this.Tb_BLDC_OnTime.LargeChange = 15;
			this.Tb_BLDC_OnTime.Location = new System.Drawing.Point(52, 75);
			this.Tb_BLDC_OnTime.Maximum = 9999;
			this.Tb_BLDC_OnTime.Name = "Tb_BLDC_OnTime";
			this.Tb_BLDC_OnTime.Size = new System.Drawing.Size(240, 23);
			this.Tb_BLDC_OnTime.SmallChange = 15;
			this.Tb_BLDC_OnTime.TabIndex = 9;
			this.Tb_BLDC_OnTime.Value = 10;
			this.Tb_BLDC_OnTime.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Tb_BLDC_OnTime_KeyUp);
			this.Tb_BLDC_OnTime.MouseUp += new System.Windows.Forms.MouseEventHandler(this.Tb_BLDC_OnTime_MouseUp);
			// 
			// Tb_BLDC_Duty
			// 
			this.Tb_BLDC_Duty.AutoSize = false;
			this.Tb_BLDC_Duty.LargeChange = 15;
			this.Tb_BLDC_Duty.Location = new System.Drawing.Point(51, 50);
			this.Tb_BLDC_Duty.Maximum = 255;
			this.Tb_BLDC_Duty.Minimum = 1;
			this.Tb_BLDC_Duty.Name = "Tb_BLDC_Duty";
			this.Tb_BLDC_Duty.Size = new System.Drawing.Size(240, 25);
			this.Tb_BLDC_Duty.SmallChange = 15;
			this.Tb_BLDC_Duty.TabIndex = 9;
			this.Tb_BLDC_Duty.Value = 1;
			this.Tb_BLDC_Duty.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Tb_BLDC_Duty_KeyUp);
			this.Tb_BLDC_Duty.MouseUp += new System.Windows.Forms.MouseEventHandler(this.Tb_BLDC_Duty_MouseUp);
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label4.Location = new System.Drawing.Point(22, 81);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(35, 12);
			this.label4.TabIndex = 3;
			this.label4.Text = "[ms]";
			// 
			// lbl_CW_CCW
			// 
			this.lbl_CW_CCW.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.249999F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lbl_CW_CCW.ForeColor = System.Drawing.Color.Red;
			this.lbl_CW_CCW.Location = new System.Drawing.Point(114, 74);
			this.lbl_CW_CCW.Name = "lbl_CW_CCW";
			this.lbl_CW_CCW.Size = new System.Drawing.Size(180, 14);
			this.lbl_CW_CCW.TabIndex = 3;
			this.lbl_CW_CCW.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label6.Location = new System.Drawing.Point(25, 53);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(30, 12);
			this.label6.TabIndex = 3;
			this.label6.Text = "Duty";
			// 
			// groupBox13
			// 
			this.groupBox13.Controls.Add(this.label21);
			this.groupBox13.Controls.Add(this.label22);
			this.groupBox13.Controls.Add(this.Lab_IMU_Conn);
			this.groupBox13.Controls.Add(this.Lab_Slave_Conn);
			this.groupBox13.Controls.Add(this.label25);
			this.groupBox13.Controls.Add(this.Lab_Main_Conn);
			this.groupBox13.Location = new System.Drawing.Point(12, 120);
			this.groupBox13.Name = "groupBox13";
			this.groupBox13.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
			this.groupBox13.Size = new System.Drawing.Size(109, 76);
			this.groupBox13.TabIndex = 18;
			this.groupBox13.TabStop = false;
			this.groupBox13.Text = "연결상태";
			// 
			// label21
			// 
			this.label21.AutoSize = true;
			this.label21.Location = new System.Drawing.Point(5, 59);
			this.label21.Name = "label21";
			this.label21.Size = new System.Drawing.Size(51, 12);
			this.label21.TabIndex = 26;
			this.label21.Text = "[IMU부]";
			// 
			// label22
			// 
			this.label22.AutoSize = true;
			this.label22.Location = new System.Drawing.Point(5, 38);
			this.label22.Name = "label22";
			this.label22.Size = new System.Drawing.Size(53, 12);
			this.label22.TabIndex = 27;
			this.label22.Text = "[릴   부]";
			// 
			// Lab_IMU_Conn
			// 
			this.Lab_IMU_Conn.BackColor = System.Drawing.Color.Gray;
			this.Lab_IMU_Conn.Location = new System.Drawing.Point(59, 57);
			this.Lab_IMU_Conn.Name = "Lab_IMU_Conn";
			this.Lab_IMU_Conn.Size = new System.Drawing.Size(26, 15);
			this.Lab_IMU_Conn.TabIndex = 22;
			// 
			// Lab_Slave_Conn
			// 
			this.Lab_Slave_Conn.BackColor = System.Drawing.Color.Gray;
			this.Lab_Slave_Conn.Location = new System.Drawing.Point(59, 38);
			this.Lab_Slave_Conn.Name = "Lab_Slave_Conn";
			this.Lab_Slave_Conn.Size = new System.Drawing.Size(26, 14);
			this.Lab_Slave_Conn.TabIndex = 23;
			// 
			// label25
			// 
			this.label25.AutoSize = true;
			this.label25.Location = new System.Drawing.Point(5, 19);
			this.label25.Name = "label25";
			this.label25.Size = new System.Drawing.Size(53, 12);
			this.label25.TabIndex = 25;
			this.label25.Text = "[제어부]";
			// 
			// Lab_Main_Conn
			// 
			this.Lab_Main_Conn.BackColor = System.Drawing.Color.Gray;
			this.Lab_Main_Conn.Location = new System.Drawing.Point(59, 18);
			this.Lab_Main_Conn.Name = "Lab_Main_Conn";
			this.Lab_Main_Conn.Size = new System.Drawing.Size(26, 14);
			this.Lab_Main_Conn.TabIndex = 24;
			// 
			// groupBox8
			// 
			this.groupBox8.Controls.Add(this.Textbox_Rod_Ver);
			this.groupBox8.Controls.Add(this.Textbox_Imu_Ver);
			this.groupBox8.Controls.Add(this.Textbox_Main_Ver);
			this.groupBox8.Controls.Add(this.tb_MainBoardVer);
			this.groupBox8.Controls.Add(this.lb_BoardVer);
			this.groupBox8.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox8.Location = new System.Drawing.Point(11, 204);
			this.groupBox8.Name = "groupBox8";
			this.groupBox8.Size = new System.Drawing.Size(110, 103);
			this.groupBox8.TabIndex = 21;
			this.groupBox8.TabStop = false;
			this.groupBox8.Text = "버전";
			// 
			// Textbox_Rod_Ver
			// 
			this.Textbox_Rod_Ver.Font = new System.Drawing.Font("굴림", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Textbox_Rod_Ver.Location = new System.Drawing.Point(6, 36);
			this.Textbox_Rod_Ver.Name = "Textbox_Rod_Ver";
			this.Textbox_Rod_Ver.Size = new System.Drawing.Size(98, 20);
			this.Textbox_Rod_Ver.TabIndex = 24;
			this.Textbox_Rod_Ver.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// Textbox_Imu_Ver
			// 
			this.Textbox_Imu_Ver.Font = new System.Drawing.Font("굴림", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Textbox_Imu_Ver.Location = new System.Drawing.Point(6, 53);
			this.Textbox_Imu_Ver.Name = "Textbox_Imu_Ver";
			this.Textbox_Imu_Ver.Size = new System.Drawing.Size(98, 20);
			this.Textbox_Imu_Ver.TabIndex = 21;
			this.Textbox_Imu_Ver.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// Textbox_Main_Ver
			// 
			this.Textbox_Main_Ver.Font = new System.Drawing.Font("굴림", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Textbox_Main_Ver.Location = new System.Drawing.Point(6, 14);
			this.Textbox_Main_Ver.Name = "Textbox_Main_Ver";
			this.Textbox_Main_Ver.Size = new System.Drawing.Size(98, 20);
			this.Textbox_Main_Ver.TabIndex = 23;
			this.Textbox_Main_Ver.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// tb_MainBoardVer
			// 
			this.tb_MainBoardVer.Font = new System.Drawing.Font("굴림", 8.5F);
			this.tb_MainBoardVer.Location = new System.Drawing.Point(33, 77);
			this.tb_MainBoardVer.Name = "tb_MainBoardVer";
			this.tb_MainBoardVer.Size = new System.Drawing.Size(73, 21);
			this.tb_MainBoardVer.TabIndex = 89;
			this.tb_MainBoardVer.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// lb_BoardVer
			// 
			this.lb_BoardVer.AutoSize = true;
			this.lb_BoardVer.Location = new System.Drawing.Point(6, 82);
			this.lb_BoardVer.Name = "lb_BoardVer";
			this.lb_BoardVer.Size = new System.Drawing.Size(29, 12);
			this.lb_BoardVer.TabIndex = 88;
			this.lb_BoardVer.Text = "보드";
			// 
			// label14
			// 
			this.label14.AutoSize = true;
			this.label14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
			this.label14.Location = new System.Drawing.Point(11, 313);
			this.label14.Name = "label14";
			this.label14.Size = new System.Drawing.Size(53, 12);
			this.label14.TabIndex = 22;
			this.label14.Text = "에러코드";
			// 
			// Textbox_Error_Code
			// 
			this.Textbox_Error_Code.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.Textbox_Error_Code.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Textbox_Error_Code.Location = new System.Drawing.Point(70, 309);
			this.Textbox_Error_Code.Name = "Textbox_Error_Code";
			this.Textbox_Error_Code.Size = new System.Drawing.Size(44, 21);
			this.Textbox_Error_Code.TabIndex = 23;
			this.Textbox_Error_Code.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// TextBox_Error_Content
			// 
			this.TextBox_Error_Content.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
			this.TextBox_Error_Content.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.TextBox_Error_Content.Location = new System.Drawing.Point(1, 331);
			this.TextBox_Error_Content.Name = "TextBox_Error_Content";
			this.TextBox_Error_Content.Size = new System.Drawing.Size(120, 21);
			this.TextBox_Error_Content.TabIndex = 23;
			// 
			// groupBox14
			// 
			this.groupBox14.Controls.Add(this.btn_REEL_LED_right);
			this.groupBox14.Controls.Add(this.btn_REEL_LED_left);
			this.groupBox14.Controls.Add(this.btn_REEL_motor);
			this.groupBox14.Controls.Add(this.btn_REEL_OFF);
			this.groupBox14.Controls.Add(this.btn_REEL_ON);
			this.groupBox14.Location = new System.Drawing.Point(754, 205);
			this.groupBox14.Name = "groupBox14";
			this.groupBox14.Size = new System.Drawing.Size(217, 118);
			this.groupBox14.TabIndex = 25;
			this.groupBox14.TabStop = false;
			this.groupBox14.Text = "릴 출력";
			// 
			// btn_REEL_LED_right
			// 
			this.btn_REEL_LED_right.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_REEL_LED_right.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_REEL_LED_right.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_REEL_LED_right.Location = new System.Drawing.Point(8, 66);
			this.btn_REEL_LED_right.Name = "btn_REEL_LED_right";
			this.btn_REEL_LED_right.Size = new System.Drawing.Size(97, 25);
			this.btn_REEL_LED_right.TabIndex = 10;
			this.btn_REEL_LED_right.Text = "버튼LED(우)";
			this.btn_REEL_LED_right.UseVisualStyleBackColor = false;
			this.btn_REEL_LED_right.Click += new System.EventHandler(this.btn_REEL_LED_right_Click);
			// 
			// btn_REEL_LED_left
			// 
			this.btn_REEL_LED_left.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_REEL_LED_left.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_REEL_LED_left.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_REEL_LED_left.Location = new System.Drawing.Point(8, 39);
			this.btn_REEL_LED_left.Name = "btn_REEL_LED_left";
			this.btn_REEL_LED_left.Size = new System.Drawing.Size(97, 25);
			this.btn_REEL_LED_left.TabIndex = 9;
			this.btn_REEL_LED_left.Text = "버튼LED(좌)";
			this.btn_REEL_LED_left.UseVisualStyleBackColor = false;
			this.btn_REEL_LED_left.Click += new System.EventHandler(this.btn_REEL_LED_left_Click);
			// 
			// btn_REEL_motor
			// 
			this.btn_REEL_motor.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_REEL_motor.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_REEL_motor.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_REEL_motor.Location = new System.Drawing.Point(8, 13);
			this.btn_REEL_motor.Name = "btn_REEL_motor";
			this.btn_REEL_motor.Size = new System.Drawing.Size(97, 25);
			this.btn_REEL_motor.TabIndex = 8;
			this.btn_REEL_motor.Text = "진동모터";
			this.btn_REEL_motor.UseVisualStyleBackColor = false;
			this.btn_REEL_motor.Click += new System.EventHandler(this.btn_REEL_motor_Click);
			// 
			// btn_REEL_OFF
			// 
			this.btn_REEL_OFF.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_REEL_OFF.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_REEL_OFF.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_REEL_OFF.Location = new System.Drawing.Point(114, 52);
			this.btn_REEL_OFF.Name = "btn_REEL_OFF";
			this.btn_REEL_OFF.Size = new System.Drawing.Size(53, 25);
			this.btn_REEL_OFF.TabIndex = 6;
			this.btn_REEL_OFF.Text = "OFF";
			this.btn_REEL_OFF.UseVisualStyleBackColor = false;
			this.btn_REEL_OFF.Click += new System.EventHandler(this.btn_REEL_OFF_Click);
			// 
			// btn_REEL_ON
			// 
			this.btn_REEL_ON.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_REEL_ON.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_REEL_ON.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_REEL_ON.Location = new System.Drawing.Point(114, 26);
			this.btn_REEL_ON.Name = "btn_REEL_ON";
			this.btn_REEL_ON.Size = new System.Drawing.Size(53, 25);
			this.btn_REEL_ON.TabIndex = 6;
			this.btn_REEL_ON.Text = "ON";
			this.btn_REEL_ON.UseVisualStyleBackColor = false;
			this.btn_REEL_ON.Click += new System.EventHandler(this.btn_REEL_ON_Click);
			// 
			// SendCmdGroup
			// 
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend6);
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend5);
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend4);
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend3);
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend2);
			this.SendCmdGroup.Controls.Add(this.tb_CustomSend1);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send6);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send5);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send4);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send3);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send2);
			this.SendCmdGroup.Controls.Add(this.btn_Custom_send1);
			this.SendCmdGroup.Controls.Add(this.label30);
			this.SendCmdGroup.Location = new System.Drawing.Point(12, 473);
			this.SendCmdGroup.Name = "SendCmdGroup";
			this.SendCmdGroup.Size = new System.Drawing.Size(416, 192);
			this.SendCmdGroup.TabIndex = 25;
			this.SendCmdGroup.TabStop = false;
			this.SendCmdGroup.Text = "수동 송신";
			// 
			// tb_CustomSend6
			// 
			this.tb_CustomSend6.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend6.Location = new System.Drawing.Point(60, 156);
			this.tb_CustomSend6.Name = "tb_CustomSend6";
			this.tb_CustomSend6.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend6.TabIndex = 31;
			// 
			// tb_CustomSend5
			// 
			this.tb_CustomSend5.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend5.Location = new System.Drawing.Point(60, 127);
			this.tb_CustomSend5.Name = "tb_CustomSend5";
			this.tb_CustomSend5.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend5.TabIndex = 30;
			// 
			// tb_CustomSend4
			// 
			this.tb_CustomSend4.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend4.Location = new System.Drawing.Point(60, 100);
			this.tb_CustomSend4.Name = "tb_CustomSend4";
			this.tb_CustomSend4.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend4.TabIndex = 29;
			// 
			// tb_CustomSend3
			// 
			this.tb_CustomSend3.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend3.Location = new System.Drawing.Point(60, 72);
			this.tb_CustomSend3.Name = "tb_CustomSend3";
			this.tb_CustomSend3.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend3.TabIndex = 28;
			// 
			// tb_CustomSend2
			// 
			this.tb_CustomSend2.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend2.Location = new System.Drawing.Point(60, 46);
			this.tb_CustomSend2.Name = "tb_CustomSend2";
			this.tb_CustomSend2.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend2.TabIndex = 27;
			// 
			// tb_CustomSend1
			// 
			this.tb_CustomSend1.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_CustomSend1.Location = new System.Drawing.Point(60, 18);
			this.tb_CustomSend1.Name = "tb_CustomSend1";
			this.tb_CustomSend1.Size = new System.Drawing.Size(356, 21);
			this.tb_CustomSend1.TabIndex = 26;
			// 
			// btn_Custom_send6
			// 
			this.btn_Custom_send6.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send6.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send6.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send6.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send6.Location = new System.Drawing.Point(6, 150);
			this.btn_Custom_send6.Name = "btn_Custom_send6";
			this.btn_Custom_send6.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send6.TabIndex = 9;
			this.btn_Custom_send6.Text = "SEND";
			this.btn_Custom_send6.UseVisualStyleBackColor = false;
			this.btn_Custom_send6.Click += new System.EventHandler(this.btn_Custom_send6_Click);
			// 
			// btn_Custom_send5
			// 
			this.btn_Custom_send5.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send5.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send5.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send5.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send5.Location = new System.Drawing.Point(6, 124);
			this.btn_Custom_send5.Name = "btn_Custom_send5";
			this.btn_Custom_send5.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send5.TabIndex = 9;
			this.btn_Custom_send5.Text = "SEND";
			this.btn_Custom_send5.UseVisualStyleBackColor = false;
			this.btn_Custom_send5.Click += new System.EventHandler(this.btn_Custom_send5_Click);
			// 
			// btn_Custom_send4
			// 
			this.btn_Custom_send4.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send4.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send4.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send4.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send4.Location = new System.Drawing.Point(6, 96);
			this.btn_Custom_send4.Name = "btn_Custom_send4";
			this.btn_Custom_send4.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send4.TabIndex = 9;
			this.btn_Custom_send4.Text = "SEND";
			this.btn_Custom_send4.UseVisualStyleBackColor = false;
			this.btn_Custom_send4.Click += new System.EventHandler(this.btn_Custom_send4_Click);
			// 
			// btn_Custom_send3
			// 
			this.btn_Custom_send3.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send3.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send3.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send3.Location = new System.Drawing.Point(6, 69);
			this.btn_Custom_send3.Name = "btn_Custom_send3";
			this.btn_Custom_send3.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send3.TabIndex = 8;
			this.btn_Custom_send3.Text = "SEND";
			this.btn_Custom_send3.UseVisualStyleBackColor = false;
			this.btn_Custom_send3.Click += new System.EventHandler(this.btn_Custom_send3_Click);
			// 
			// btn_Custom_send2
			// 
			this.btn_Custom_send2.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send2.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send2.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send2.Location = new System.Drawing.Point(6, 41);
			this.btn_Custom_send2.Name = "btn_Custom_send2";
			this.btn_Custom_send2.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send2.TabIndex = 7;
			this.btn_Custom_send2.Text = "SEND";
			this.btn_Custom_send2.UseVisualStyleBackColor = false;
			this.btn_Custom_send2.Click += new System.EventHandler(this.btn_Custom_send2_Click);
			// 
			// btn_Custom_send1
			// 
			this.btn_Custom_send1.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_Custom_send1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_Custom_send1.Font = new System.Drawing.Font("굴림", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_Custom_send1.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_Custom_send1.Location = new System.Drawing.Point(6, 15);
			this.btn_Custom_send1.Name = "btn_Custom_send1";
			this.btn_Custom_send1.Size = new System.Drawing.Size(56, 25);
			this.btn_Custom_send1.TabIndex = 6;
			this.btn_Custom_send1.Text = "SEND";
			this.btn_Custom_send1.UseVisualStyleBackColor = false;
			this.btn_Custom_send1.Click += new System.EventHandler(this.btn_Custom_send1_Click);
			// 
			// label30
			// 
			this.label30.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.249999F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label30.ForeColor = System.Drawing.Color.Red;
			this.label30.Location = new System.Drawing.Point(114, 74);
			this.label30.Name = "label30";
			this.label30.Size = new System.Drawing.Size(180, 14);
			this.label30.TabIndex = 3;
			this.label30.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// groupBox16
			// 
			this.groupBox16.Controls.Add(this.tb_Poll_Interval);
			this.groupBox16.Controls.Add(this.label46);
			this.groupBox16.Controls.Add(this.tb_LogSaveMin);
			this.groupBox16.Controls.Add(this.label38);
			this.groupBox16.Controls.Add(this.btn_log_autoscroll);
			this.groupBox16.Controls.Add(this.btn_log_clear);
			this.groupBox16.Controls.Add(this.btn_log_save);
			this.groupBox16.Controls.Add(this.tb_LOG_Window);
			this.groupBox16.Controls.Add(this.label27);
			this.groupBox16.Location = new System.Drawing.Point(443, 473);
			this.groupBox16.Name = "groupBox16";
			this.groupBox16.Size = new System.Drawing.Size(527, 271);
			this.groupBox16.TabIndex = 30;
			this.groupBox16.TabStop = false;
			this.groupBox16.Text = "동작 로그";
			// 
			// tb_Poll_Interval
			// 
			this.tb_Poll_Interval.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Poll_Interval.Location = new System.Drawing.Point(495, 3);
			this.tb_Poll_Interval.Name = "tb_Poll_Interval";
			this.tb_Poll_Interval.Size = new System.Drawing.Size(29, 21);
			this.tb_Poll_Interval.TabIndex = 114;
			this.tb_Poll_Interval.Text = "2";
			this.tb_Poll_Interval.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.tb_Poll_Interval.TextChanged += new System.EventHandler(this.tb_Poll_Interval_TextChanged);
			// 
			// label46
			// 
			this.label46.AutoSize = true;
			this.label46.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label46.Location = new System.Drawing.Point(435, 7);
			this.label46.Name = "label46";
			this.label46.Size = new System.Drawing.Size(63, 12);
			this.label46.TabIndex = 113;
			this.label46.Text = "폴주기(초)";
			this.label46.DoubleClick += new System.EventHandler(this.label46_DoubleClick);
			// 
			// tb_LogSaveMin
			// 
			this.tb_LogSaveMin.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_LogSaveMin.Location = new System.Drawing.Point(404, 4);
			this.tb_LogSaveMin.Name = "tb_LogSaveMin";
			this.tb_LogSaveMin.Size = new System.Drawing.Size(25, 21);
			this.tb_LogSaveMin.TabIndex = 112;
			this.tb_LogSaveMin.Text = "5";
			this.tb_LogSaveMin.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.tb_LogSaveMin.TextChanged += new System.EventHandler(this.tb_LogSaveMin_TextChanged);
			// 
			// label38
			// 
			this.label38.AutoSize = true;
			this.label38.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label38.Location = new System.Drawing.Point(307, 6);
			this.label38.Name = "label38";
			this.label38.Size = new System.Drawing.Size(99, 12);
			this.label38.TabIndex = 112;
			this.label38.Text = "로그자동저장(분)";
			// 
			// btn_log_autoscroll
			// 
			this.btn_log_autoscroll.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_log_autoscroll.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_log_autoscroll.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_log_autoscroll.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_log_autoscroll.Location = new System.Drawing.Point(68, -1);
			this.btn_log_autoscroll.Name = "btn_log_autoscroll";
			this.btn_log_autoscroll.Size = new System.Drawing.Size(105, 25);
			this.btn_log_autoscroll.TabIndex = 32;
			this.btn_log_autoscroll.Text = "Auto Scroll";
			this.btn_log_autoscroll.UseVisualStyleBackColor = false;
			this.btn_log_autoscroll.Click += new System.EventHandler(this.btn_log_autoscroll_Click);
			// 
			// btn_log_clear
			// 
			this.btn_log_clear.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_log_clear.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_log_clear.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_log_clear.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_log_clear.Location = new System.Drawing.Point(243, -2);
			this.btn_log_clear.Name = "btn_log_clear";
			this.btn_log_clear.Size = new System.Drawing.Size(58, 25);
			this.btn_log_clear.TabIndex = 31;
			this.btn_log_clear.Text = "Clear";
			this.btn_log_clear.UseVisualStyleBackColor = false;
			this.btn_log_clear.Click += new System.EventHandler(this.btn_log_clear_Click);
			// 
			// btn_log_save
			// 
			this.btn_log_save.BackColor = System.Drawing.Color.DodgerBlue;
			this.btn_log_save.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
			this.btn_log_save.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_log_save.ForeColor = System.Drawing.SystemColors.ControlLightLight;
			this.btn_log_save.Location = new System.Drawing.Point(179, -1);
			this.btn_log_save.Name = "btn_log_save";
			this.btn_log_save.Size = new System.Drawing.Size(58, 25);
			this.btn_log_save.TabIndex = 30;
			this.btn_log_save.Text = "Save";
			this.btn_log_save.UseVisualStyleBackColor = false;
			this.btn_log_save.Click += new System.EventHandler(this.btn_log_save_Click);
			// 
			// tb_LOG_Window
			// 
			this.tb_LOG_Window.BackColor = System.Drawing.Color.White;
			this.tb_LOG_Window.Font = new System.Drawing.Font("Yu Gothic", 8.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_LOG_Window.Location = new System.Drawing.Point(7, 24);
			this.tb_LOG_Window.Name = "tb_LOG_Window";
			this.tb_LOG_Window.ReadOnly = true;
			this.tb_LOG_Window.Size = new System.Drawing.Size(515, 242);
			this.tb_LOG_Window.TabIndex = 4;
			this.tb_LOG_Window.Tag = "LOG";
			this.tb_LOG_Window.Text = "";
			// 
			// label27
			// 
			this.label27.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.249999F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label27.ForeColor = System.Drawing.Color.Red;
			this.label27.Location = new System.Drawing.Point(114, 74);
			this.label27.Name = "label27";
			this.label27.Size = new System.Drawing.Size(180, 14);
			this.label27.TabIndex = 3;
			this.label27.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// groupBox4
			// 
			this.groupBox4.Controls.Add(this.label19);
			this.groupBox4.Controls.Add(this.Lab_Boot);
			this.groupBox4.Controls.Add(this.Lab_Setup);
			this.groupBox4.Location = new System.Drawing.Point(858, 332);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(112, 69);
			this.groupBox4.TabIndex = 17;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "릴 [보드입력]";
			// 
			// label19
			// 
			this.label19.AutoSize = true;
			this.label19.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label19.Location = new System.Drawing.Point(3, 41);
			this.label19.Name = "label19";
			this.label19.Size = new System.Drawing.Size(57, 12);
			this.label19.TabIndex = 33;
			this.label19.Text = "부트,설정";
			// 
			// Lab_Boot
			// 
			this.Lab_Boot.BackColor = System.Drawing.Color.Gray;
			this.Lab_Boot.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Lab_Boot.Location = new System.Drawing.Point(9, 17);
			this.Lab_Boot.Name = "Lab_Boot";
			this.Lab_Boot.Size = new System.Drawing.Size(17, 18);
			this.Lab_Boot.TabIndex = 1;
			// 
			// Lab_Setup
			// 
			this.Lab_Setup.BackColor = System.Drawing.Color.Gray;
			this.Lab_Setup.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Lab_Setup.Location = new System.Drawing.Point(31, 17);
			this.Lab_Setup.Name = "Lab_Setup";
			this.Lab_Setup.Size = new System.Drawing.Size(17, 18);
			this.Lab_Setup.TabIndex = 0;
			// 
			// groupBox15
			// 
			this.groupBox15.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox15.Controls.Add(this.Lab_ImuInterval);
			this.groupBox15.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox15.Location = new System.Drawing.Point(754, 158);
			this.groupBox15.Name = "groupBox15";
			this.groupBox15.Size = new System.Drawing.Size(111, 40);
			this.groupBox15.TabIndex = 18;
			this.groupBox15.TabStop = false;
			this.groupBox15.Text = "릴 IMU센서간격ms";
			// 
			// Lab_ImuInterval
			// 
			this.Lab_ImuInterval.AutoSize = true;
			this.Lab_ImuInterval.Font = new System.Drawing.Font("굴림", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Lab_ImuInterval.Location = new System.Drawing.Point(35, 18);
			this.Lab_ImuInterval.Name = "Lab_ImuInterval";
			this.Lab_ImuInterval.Size = new System.Drawing.Size(34, 16);
			this.Lab_ImuInterval.TabIndex = 0;
			this.Lab_ImuInterval.Text = "000";
			this.Lab_ImuInterval.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// groupBox12
			// 
			this.groupBox12.Controls.Add(this.label26);
			this.groupBox12.Controls.Add(this.Btn_Bbn_In_Stop);
			this.groupBox12.Controls.Add(this.label31);
			this.groupBox12.Controls.Add(this.Cont_Pwr_Sw);
			this.groupBox12.Controls.Add(this.Cont_usb5V);
			this.groupBox12.Controls.Add(this.Btn_Bbn_In_Start);
			this.groupBox12.Controls.Add(this.Cont_Boot_Sw);
			this.groupBox12.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox12.Location = new System.Drawing.Point(614, 397);
			this.groupBox12.Name = "groupBox12";
			this.groupBox12.Size = new System.Drawing.Size(140, 70);
			this.groupBox12.TabIndex = 18;
			this.groupBox12.TabStop = false;
			this.groupBox12.Text = "입력확인";
			// 
			// label26
			// 
			this.label26.AutoSize = true;
			this.label26.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label26.Location = new System.Drawing.Point(68, 0);
			this.label26.Name = "label26";
			this.label26.Size = new System.Drawing.Size(65, 12);
			this.label26.TabIndex = 37;
			this.label26.Text = "[보드입력]";
			// 
			// Btn_Bbn_In_Stop
			// 
			this.Btn_Bbn_In_Stop.BackColor = System.Drawing.Color.Gainsboro;
			this.Btn_Bbn_In_Stop.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Btn_Bbn_In_Stop.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_Bbn_In_Stop.Location = new System.Drawing.Point(11, 39);
			this.Btn_Bbn_In_Stop.Name = "Btn_Bbn_In_Stop";
			this.Btn_Bbn_In_Stop.Size = new System.Drawing.Size(40, 25);
			this.Btn_Bbn_In_Stop.TabIndex = 32;
			this.Btn_Bbn_In_Stop.Text = "정지";
			this.Btn_Bbn_In_Stop.UseVisualStyleBackColor = false;
			this.Btn_Bbn_In_Stop.Click += new System.EventHandler(this.Btn_Bbn_In_Stop_Click);
			// 
			// label31
			// 
			this.label31.AutoSize = true;
			this.label31.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label31.Location = new System.Drawing.Point(57, 41);
			this.label31.Name = "label31";
			this.label31.Size = new System.Drawing.Size(85, 12);
			this.label31.TabIndex = 31;
			this.label31.Text = "부트,USB,전원";
			// 
			// Cont_Pwr_Sw
			// 
			this.Cont_Pwr_Sw.BackColor = System.Drawing.Color.Gray;
			this.Cont_Pwr_Sw.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Cont_Pwr_Sw.Location = new System.Drawing.Point(115, 17);
			this.Cont_Pwr_Sw.Name = "Cont_Pwr_Sw";
			this.Cont_Pwr_Sw.Size = new System.Drawing.Size(18, 18);
			this.Cont_Pwr_Sw.TabIndex = 2;
			// 
			// Cont_usb5V
			// 
			this.Cont_usb5V.BackColor = System.Drawing.Color.Gray;
			this.Cont_usb5V.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Cont_usb5V.Location = new System.Drawing.Point(91, 17);
			this.Cont_usb5V.Name = "Cont_usb5V";
			this.Cont_usb5V.Size = new System.Drawing.Size(18, 18);
			this.Cont_usb5V.TabIndex = 1;
			// 
			// Btn_Bbn_In_Start
			// 
			this.Btn_Bbn_In_Start.BackColor = System.Drawing.Color.Gainsboro;
			this.Btn_Bbn_In_Start.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Btn_Bbn_In_Start.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_Bbn_In_Start.Location = new System.Drawing.Point(11, 14);
			this.Btn_Bbn_In_Start.Name = "Btn_Bbn_In_Start";
			this.Btn_Bbn_In_Start.Size = new System.Drawing.Size(40, 25);
			this.Btn_Bbn_In_Start.TabIndex = 31;
			this.Btn_Bbn_In_Start.Text = "시작";
			this.Btn_Bbn_In_Start.UseVisualStyleBackColor = false;
			this.Btn_Bbn_In_Start.Click += new System.EventHandler(this.Btn_Bbn_In_Start_Click);
			// 
			// Cont_Boot_Sw
			// 
			this.Cont_Boot_Sw.BackColor = System.Drawing.Color.Gray;
			this.Cont_Boot_Sw.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.Cont_Boot_Sw.Location = new System.Drawing.Point(66, 17);
			this.Cont_Boot_Sw.Name = "Cont_Boot_Sw";
			this.Cont_Boot_Sw.Size = new System.Drawing.Size(19, 18);
			this.Cont_Boot_Sw.TabIndex = 0;
			// 
			// groupBox17
			// 
			this.groupBox17.Controls.Add(this.label32);
			this.groupBox17.Controls.Add(this.BbnFg);
			this.groupBox17.Controls.Add(this.BbnEncB);
			this.groupBox17.Controls.Add(this.BbnEncA);
			this.groupBox17.Location = new System.Drawing.Point(754, 397);
			this.groupBox17.Name = "groupBox17";
			this.groupBox17.Size = new System.Drawing.Size(86, 70);
			this.groupBox17.TabIndex = 19;
			this.groupBox17.TabStop = false;
			this.groupBox17.Text = "[모터 입력]";
			// 
			// label32
			// 
			this.label32.AutoSize = true;
			this.label32.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label32.Location = new System.Drawing.Point(10, 41);
			this.label32.Name = "label32";
			this.label32.Size = new System.Drawing.Size(75, 12);
			this.label32.TabIndex = 32;
			this.label32.Text = "EnA EnB FG";
			// 
			// BbnFg
			// 
			this.BbnFg.BackColor = System.Drawing.Color.Gray;
			this.BbnFg.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.BbnFg.Location = new System.Drawing.Point(60, 17);
			this.BbnFg.Name = "BbnFg";
			this.BbnFg.Size = new System.Drawing.Size(18, 18);
			this.BbnFg.TabIndex = 2;
			// 
			// BbnEncB
			// 
			this.BbnEncB.BackColor = System.Drawing.Color.Gray;
			this.BbnEncB.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.BbnEncB.Location = new System.Drawing.Point(37, 17);
			this.BbnEncB.Name = "BbnEncB";
			this.BbnEncB.Size = new System.Drawing.Size(18, 18);
			this.BbnEncB.TabIndex = 1;
			// 
			// BbnEncA
			// 
			this.BbnEncA.BackColor = System.Drawing.Color.Gray;
			this.BbnEncA.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.BbnEncA.Location = new System.Drawing.Point(12, 17);
			this.BbnEncA.Name = "BbnEncA";
			this.BbnEncA.Size = new System.Drawing.Size(19, 18);
			this.BbnEncA.TabIndex = 0;
			// 
			// Btn_SubAc_Off
			// 
			this.Btn_SubAc_Off.BackColor = System.Drawing.Color.Gainsboro;
			this.Btn_SubAc_Off.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Btn_SubAc_Off.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_SubAc_Off.Location = new System.Drawing.Point(8, 36);
			this.Btn_SubAc_Off.Name = "Btn_SubAc_Off";
			this.Btn_SubAc_Off.Size = new System.Drawing.Size(50, 25);
			this.Btn_SubAc_Off.TabIndex = 31;
			this.Btn_SubAc_Off.Text = "OFF";
			this.Btn_SubAc_Off.UseVisualStyleBackColor = false;
			this.Btn_SubAc_Off.Click += new System.EventHandler(this.Btn_SubAc_Off_Click);
			// 
			// PowerSwitch
			// 
			this.PowerSwitch.Controls.Add(this.Btn_SubAc_On);
			this.PowerSwitch.Controls.Add(this.Btn_SubAc_Off);
			this.PowerSwitch.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.PowerSwitch.Location = new System.Drawing.Point(451, 397);
			this.PowerSwitch.Name = "PowerSwitch";
			this.PowerSwitch.Size = new System.Drawing.Size(72, 68);
			this.PowerSwitch.TabIndex = 32;
			this.PowerSwitch.TabStop = false;
			this.PowerSwitch.Text = " AC전원";
			// 
			// Btn_SubAc_On
			// 
			this.Btn_SubAc_On.BackColor = System.Drawing.Color.Gainsboro;
			this.Btn_SubAc_On.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.Btn_SubAc_On.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.Btn_SubAc_On.Location = new System.Drawing.Point(8, 11);
			this.Btn_SubAc_On.Name = "Btn_SubAc_On";
			this.Btn_SubAc_On.Size = new System.Drawing.Size(50, 25);
			this.Btn_SubAc_On.TabIndex = 32;
			this.Btn_SubAc_On.Text = "ON";
			this.Btn_SubAc_On.UseVisualStyleBackColor = false;
			this.Btn_SubAc_On.Click += new System.EventHandler(this.Btn_SubAc_On_Click);
			// 
			// setupCmd_Group
			// 
			this.setupCmd_Group.Controls.Add(this.label34);
			this.setupCmd_Group.Controls.Add(this.tb_ReelRegi_Result);
			this.setupCmd_Group.Controls.Add(this.btn_Reel_Regist);
			this.setupCmd_Group.Controls.Add(this.groupBox2);
			this.setupCmd_Group.Location = new System.Drawing.Point(272, 11);
			this.setupCmd_Group.Name = "setupCmd_Group";
			this.setupCmd_Group.Size = new System.Drawing.Size(168, 140);
			this.setupCmd_Group.TabIndex = 33;
			this.setupCmd_Group.TabStop = false;
			this.setupCmd_Group.Text = "셋업 명령";
			// 
			// label34
			// 
			this.label34.AutoSize = true;
			this.label34.Location = new System.Drawing.Point(82, 22);
			this.label34.Name = "label34";
			this.label34.Size = new System.Drawing.Size(29, 12);
			this.label34.TabIndex = 111;
			this.label34.Text = "결과";
			// 
			// tb_ReelRegi_Result
			// 
			this.tb_ReelRegi_Result.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_ReelRegi_Result.Location = new System.Drawing.Point(115, 16);
			this.tb_ReelRegi_Result.Name = "tb_ReelRegi_Result";
			this.tb_ReelRegi_Result.Size = new System.Drawing.Size(50, 21);
			this.tb_ReelRegi_Result.TabIndex = 37;
			this.tb_ReelRegi_Result.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// btn_Reel_Regist
			// 
			this.btn_Reel_Regist.Location = new System.Drawing.Point(4, 16);
			this.btn_Reel_Regist.Name = "btn_Reel_Regist";
			this.btn_Reel_Regist.Size = new System.Drawing.Size(75, 23);
			this.btn_Reel_Regist.TabIndex = 78;
			this.btn_Reel_Regist.Text = "릴 등록";
			this.btn_Reel_Regist.UseVisualStyleBackColor = true;
			this.btn_Reel_Regist.Click += new System.EventHandler(this.button2_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.label35);
			this.groupBox2.Controls.Add(this.tb_MainAutoSet_result);
			this.groupBox2.Controls.Add(this.btn_Mmot_Init);
			this.groupBox2.Controls.Add(this.label24);
			this.groupBox2.Controls.Add(this.tb_MainMot_Default);
			this.groupBox2.Controls.Add(this.btn_MainTorqAutoSet);
			this.groupBox2.Controls.Add(this.btn_Mmot_Write);
			this.groupBox2.Controls.Add(this.btn_Mmot_Read);
			this.groupBox2.Location = new System.Drawing.Point(3, 46);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(165, 82);
			this.groupBox2.TabIndex = 36;
			this.groupBox2.TabStop = false;
			// 
			// label35
			// 
			this.label35.AutoSize = true;
			this.label35.Location = new System.Drawing.Point(75, 59);
			this.label35.Name = "label35";
			this.label35.Size = new System.Drawing.Size(29, 12);
			this.label35.TabIndex = 112;
			this.label35.Text = "결과";
			// 
			// tb_MainAutoSet_result
			// 
			this.tb_MainAutoSet_result.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_MainAutoSet_result.Location = new System.Drawing.Point(107, 55);
			this.tb_MainAutoSet_result.Name = "tb_MainAutoSet_result";
			this.tb_MainAutoSet_result.Size = new System.Drawing.Size(55, 21);
			this.tb_MainAutoSet_result.TabIndex = 86;
			this.tb_MainAutoSet_result.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// btn_Mmot_Init
			// 
			this.btn_Mmot_Init.Location = new System.Drawing.Point(43, 24);
			this.btn_Mmot_Init.Name = "btn_Mmot_Init";
			this.btn_Mmot_Init.Size = new System.Drawing.Size(37, 23);
			this.btn_Mmot_Init.TabIndex = 84;
			this.btn_Mmot_Init.Text = "초기";
			this.btn_Mmot_Init.UseVisualStyleBackColor = true;
			this.btn_Mmot_Init.Click += new System.EventHandler(this.btn_Mmot_Init_Click);
			// 
			// label24
			// 
			this.label24.AutoSize = true;
			this.label24.Font = new System.Drawing.Font("굴림", 8.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label24.Location = new System.Drawing.Point(6, 11);
			this.label24.Name = "label24";
			this.label24.Size = new System.Drawing.Size(138, 12);
			this.label24.TabIndex = 37;
			this.label24.Text = "메인모터 최소값[40~99]";
			// 
			// tb_MainMot_Default
			// 
			this.tb_MainMot_Default.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_MainMot_Default.Location = new System.Drawing.Point(3, 26);
			this.tb_MainMot_Default.Name = "tb_MainMot_Default";
			this.tb_MainMot_Default.Size = new System.Drawing.Size(39, 21);
			this.tb_MainMot_Default.TabIndex = 83;
			this.tb_MainMot_Default.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// btn_MainTorqAutoSet
			// 
			this.btn_MainTorqAutoSet.Font = new System.Drawing.Font("굴림", 8.5F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_MainTorqAutoSet.Location = new System.Drawing.Point(3, 53);
			this.btn_MainTorqAutoSet.Name = "btn_MainTorqAutoSet";
			this.btn_MainTorqAutoSet.Size = new System.Drawing.Size(69, 23);
			this.btn_MainTorqAutoSet.TabIndex = 86;
			this.btn_MainTorqAutoSet.Text = "자동설정";
			this.btn_MainTorqAutoSet.UseVisualStyleBackColor = true;
			this.btn_MainTorqAutoSet.Click += new System.EventHandler(this.btn_MainMotAutoSet_Click);
			// 
			// btn_Mmot_Write
			// 
			this.btn_Mmot_Write.Location = new System.Drawing.Point(124, 24);
			this.btn_Mmot_Write.Name = "btn_Mmot_Write";
			this.btn_Mmot_Write.Size = new System.Drawing.Size(37, 23);
			this.btn_Mmot_Write.TabIndex = 79;
			this.btn_Mmot_Write.Text = "쓰기";
			this.btn_Mmot_Write.UseVisualStyleBackColor = true;
			this.btn_Mmot_Write.Click += new System.EventHandler(this.btn_Mmot_Write_Click);
			// 
			// btn_Mmot_Read
			// 
			this.btn_Mmot_Read.Location = new System.Drawing.Point(88, 24);
			this.btn_Mmot_Read.Name = "btn_Mmot_Read";
			this.btn_Mmot_Read.Size = new System.Drawing.Size(37, 23);
			this.btn_Mmot_Read.TabIndex = 78;
			this.btn_Mmot_Read.Text = "읽기";
			this.btn_Mmot_Read.UseVisualStyleBackColor = true;
			this.btn_Mmot_Read.Click += new System.EventHandler(this.btn_Mmot_Read_Click);
			// 
			// ManufInfo_Group
			// 
			this.ManufInfo_Group.Controls.Add(this.domainUpDown1);
			this.ManufInfo_Group.Controls.Add(this.dud_SerialNumber);
			this.ManufInfo_Group.Controls.Add(this.domainUpDown2);
			this.ManufInfo_Group.Controls.Add(this.tb_Manuf_No);
			this.ManufInfo_Group.Controls.Add(this.domainUpDown3);
			this.ManufInfo_Group.Controls.Add(this.label37);
			this.ManufInfo_Group.Controls.Add(this.dud_National);
			this.ManufInfo_Group.Controls.Add(this.dud_Service);
			this.ManufInfo_Group.Controls.Add(this.dud_Spec);
			this.ManufInfo_Group.Controls.Add(this.label36);
			this.ManufInfo_Group.Controls.Add(this.dud_config);
			this.ManufInfo_Group.Controls.Add(this.dud_person);
			this.ManufInfo_Group.Controls.Add(this.label29);
			this.ManufInfo_Group.Controls.Add(this.Btn_Manuf_Init);
			this.ManufInfo_Group.Controls.Add(this.label20);
			this.ManufInfo_Group.Controls.Add(this.tb_Manuf_Date);
			this.ManufInfo_Group.Controls.Add(this.tb_Manuf_SerialNo);
			this.ManufInfo_Group.Controls.Add(this.tb_Manuf_Model);
			this.ManufInfo_Group.Controls.Add(this.btn_Manuf_Read);
			this.ManufInfo_Group.Controls.Add(this.btn_Manuf_Erase);
			this.ManufInfo_Group.Controls.Add(this.btn_Manuf_Write);
			this.ManufInfo_Group.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.ManufInfo_Group.Location = new System.Drawing.Point(272, 158);
			this.ManufInfo_Group.Name = "ManufInfo_Group";
			this.ManufInfo_Group.Size = new System.Drawing.Size(168, 262);
			this.ManufInfo_Group.TabIndex = 34;
			this.ManufInfo_Group.TabStop = false;
			this.ManufInfo_Group.Text = "제조정보(모델, 제번, SN)";
			// 
			// domainUpDown1
			// 
			this.domainUpDown1.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.domainUpDown1.Location = new System.Drawing.Point(101, 125);
			this.domainUpDown1.Name = "domainUpDown1";
			this.domainUpDown1.Size = new System.Drawing.Size(37, 21);
			this.domainUpDown1.TabIndex = 96;
			this.domainUpDown1.Text = "L1";
			this.domainUpDown1.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// dud_SerialNumber
			// 
			this.dud_SerialNumber.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.dud_SerialNumber.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_SerialNumber.Location = new System.Drawing.Point(93, 187);
			this.dud_SerialNumber.Name = "dud_SerialNumber";
			this.dud_SerialNumber.Size = new System.Drawing.Size(64, 21);
			this.dud_SerialNumber.TabIndex = 90;
			this.dud_SerialNumber.Text = "000010";
			this.dud_SerialNumber.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// domainUpDown2
			// 
			this.domainUpDown2.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.domainUpDown2.Location = new System.Drawing.Point(61, 125);
			this.domainUpDown2.Name = "domainUpDown2";
			this.domainUpDown2.Size = new System.Drawing.Size(40, 21);
			this.domainUpDown2.TabIndex = 95;
			this.domainUpDown2.Text = "SN";
			this.domainUpDown2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// tb_Manuf_No
			// 
			this.tb_Manuf_No.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Manuf_No.Location = new System.Drawing.Point(26, 147);
			this.tb_Manuf_No.Name = "tb_Manuf_No";
			this.tb_Manuf_No.Size = new System.Drawing.Size(135, 21);
			this.tb_Manuf_No.TabIndex = 83;
			// 
			// domainUpDown3
			// 
			this.domainUpDown3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.domainUpDown3.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.domainUpDown3.Location = new System.Drawing.Point(28, 125);
			this.domainUpDown3.Name = "domainUpDown3";
			this.domainUpDown3.Size = new System.Drawing.Size(25, 21);
			this.domainUpDown3.TabIndex = 93;
			this.domainUpDown3.Text = "K";
			this.domainUpDown3.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label37
			// 
			this.label37.AutoSize = true;
			this.label37.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label37.Location = new System.Drawing.Point(2, 112);
			this.label37.Name = "label37";
			this.label37.Size = new System.Drawing.Size(165, 12);
			this.label37.TabIndex = 94;
			this.label37.Text = "제번:국가,  공장,  라인,  로트";
			// 
			// dud_National
			// 
			this.dud_National.Anchor = System.Windows.Forms.AnchorStyles.Top;
			this.dud_National.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_National.Location = new System.Drawing.Point(55, 187);
			this.dud_National.Name = "dud_National";
			this.dud_National.Size = new System.Drawing.Size(41, 21);
			this.dud_National.TabIndex = 89;
			this.dud_National.Text = "kr";
			this.dud_National.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// dud_Service
			// 
			this.dud_Service.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.dud_Service.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_Service.Location = new System.Drawing.Point(26, 187);
			this.dud_Service.Name = "dud_Service";
			this.dud_Service.Size = new System.Drawing.Size(27, 21);
			this.dud_Service.TabIndex = 87;
			this.dud_Service.Text = "i";
			this.dud_Service.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// dud_Spec
			// 
			this.dud_Spec.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_Spec.Location = new System.Drawing.Point(113, 62);
			this.dud_Spec.Name = "dud_Spec";
			this.dud_Spec.Size = new System.Drawing.Size(50, 21);
			this.dud_Spec.TabIndex = 86;
			this.dud_Spec.Text = "300";
			this.dud_Spec.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// label36
			// 
			this.label36.AutoSize = true;
			this.label36.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label36.Location = new System.Drawing.Point(3, 173);
			this.label36.Name = "label36";
			this.label36.Size = new System.Drawing.Size(146, 12);
			this.label36.TabIndex = 88;
			this.label36.Text = "SN:구분, 국가,   일련번호";
			// 
			// dud_config
			// 
			this.dud_config.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_config.Location = new System.Drawing.Point(65, 62);
			this.dud_config.Name = "dud_config";
			this.dud_config.Size = new System.Drawing.Size(48, 21);
			this.dud_config.TabIndex = 85;
			this.dud_config.Text = "SF";
			this.dud_config.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.dud_config.SelectedItemChanged += new System.EventHandler(this.dud_config_SelectedItemChanged);
			// 
			// dud_person
			// 
			this.dud_person.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.dud_person.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.dud_person.Location = new System.Drawing.Point(32, 62);
			this.dud_person.Name = "dud_person";
			this.dud_person.Size = new System.Drawing.Size(26, 21);
			this.dud_person.TabIndex = 84;
			this.dud_person.Text = "1";
			this.dud_person.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// label29
			// 
			this.label29.AutoSize = true;
			this.label29.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label29.Location = new System.Drawing.Point(2, 49);
			this.label29.Name = "label29";
			this.label29.Size = new System.Drawing.Size(149, 12);
			this.label29.TabIndex = 84;
			this.label29.Text = "모델:릴개수, 구성,     성능";
			// 
			// Btn_Manuf_Init
			// 
			this.Btn_Manuf_Init.Location = new System.Drawing.Point(20, 16);
			this.Btn_Manuf_Init.Name = "Btn_Manuf_Init";
			this.Btn_Manuf_Init.Size = new System.Drawing.Size(37, 23);
			this.Btn_Manuf_Init.TabIndex = 84;
			this.Btn_Manuf_Init.Text = "초기";
			this.Btn_Manuf_Init.UseVisualStyleBackColor = true;
			this.Btn_Manuf_Init.Click += new System.EventHandler(this.button1_Click_1);
			// 
			// label20
			// 
			this.label20.AutoSize = true;
			this.label20.Location = new System.Drawing.Point(4, 237);
			this.label20.Name = "label20";
			this.label20.Size = new System.Drawing.Size(53, 12);
			this.label20.TabIndex = 36;
			this.label20.Text = "제조날짜";
			// 
			// tb_Manuf_Date
			// 
			this.tb_Manuf_Date.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Manuf_Date.Location = new System.Drawing.Point(61, 233);
			this.tb_Manuf_Date.Name = "tb_Manuf_Date";
			this.tb_Manuf_Date.Size = new System.Drawing.Size(93, 21);
			this.tb_Manuf_Date.TabIndex = 83;
			// 
			// tb_Manuf_SerialNo
			// 
			this.tb_Manuf_SerialNo.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Manuf_SerialNo.Location = new System.Drawing.Point(24, 212);
			this.tb_Manuf_SerialNo.Name = "tb_Manuf_SerialNo";
			this.tb_Manuf_SerialNo.Size = new System.Drawing.Size(135, 21);
			this.tb_Manuf_SerialNo.TabIndex = 82;
			// 
			// tb_Manuf_Model
			// 
			this.tb_Manuf_Model.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Manuf_Model.Location = new System.Drawing.Point(26, 83);
			this.tb_Manuf_Model.Name = "tb_Manuf_Model";
			this.tb_Manuf_Model.Size = new System.Drawing.Size(135, 21);
			this.tb_Manuf_Model.TabIndex = 81;
			// 
			// btn_Manuf_Read
			// 
			this.btn_Manuf_Read.Location = new System.Drawing.Point(56, 16);
			this.btn_Manuf_Read.Name = "btn_Manuf_Read";
			this.btn_Manuf_Read.Size = new System.Drawing.Size(37, 23);
			this.btn_Manuf_Read.TabIndex = 78;
			this.btn_Manuf_Read.Text = "읽기";
			this.btn_Manuf_Read.UseVisualStyleBackColor = true;
			this.btn_Manuf_Read.Click += new System.EventHandler(this.btn_Manuf_Read_Click);
			// 
			// btn_Manuf_Erase
			// 
			this.btn_Manuf_Erase.Location = new System.Drawing.Point(127, 16);
			this.btn_Manuf_Erase.Name = "btn_Manuf_Erase";
			this.btn_Manuf_Erase.Size = new System.Drawing.Size(37, 23);
			this.btn_Manuf_Erase.TabIndex = 80;
			this.btn_Manuf_Erase.Text = "삭제";
			this.btn_Manuf_Erase.UseVisualStyleBackColor = true;
			this.btn_Manuf_Erase.Click += new System.EventHandler(this.btn_Manuf_Erase_Click);
			// 
			// btn_Manuf_Write
			// 
			this.btn_Manuf_Write.Location = new System.Drawing.Point(92, 16);
			this.btn_Manuf_Write.Name = "btn_Manuf_Write";
			this.btn_Manuf_Write.Size = new System.Drawing.Size(37, 23);
			this.btn_Manuf_Write.TabIndex = 79;
			this.btn_Manuf_Write.Text = "쓰기";
			this.btn_Manuf_Write.UseVisualStyleBackColor = true;
			this.btn_Manuf_Write.Click += new System.EventHandler(this.btn_Manuf_Write_Click);
			// 
			// InstallGroup
			// 
			this.InstallGroup.Controls.Add(this.Btn_Install_Init);
			this.InstallGroup.Controls.Add(this.label23);
			this.InstallGroup.Controls.Add(this.tb_Install_Date);
			this.InstallGroup.Controls.Add(this.btn_Install_Delete);
			this.InstallGroup.Controls.Add(this.btn_Install_Write);
			this.InstallGroup.Controls.Add(this.btn_Install_Read);
			this.InstallGroup.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.InstallGroup.Location = new System.Drawing.Point(272, 415);
			this.InstallGroup.Name = "InstallGroup";
			this.InstallGroup.Size = new System.Drawing.Size(168, 57);
			this.InstallGroup.TabIndex = 35;
			this.InstallGroup.TabStop = false;
			// 
			// Btn_Install_Init
			// 
			this.Btn_Install_Init.Location = new System.Drawing.Point(16, 28);
			this.Btn_Install_Init.Name = "Btn_Install_Init";
			this.Btn_Install_Init.Size = new System.Drawing.Size(37, 23);
			this.Btn_Install_Init.TabIndex = 84;
			this.Btn_Install_Init.Text = "초기";
			this.Btn_Install_Init.UseVisualStyleBackColor = true;
			this.Btn_Install_Init.Click += new System.EventHandler(this.Btn_Install_Init_Click);
			// 
			// label23
			// 
			this.label23.AutoSize = true;
			this.label23.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.label23.Location = new System.Drawing.Point(5, 11);
			this.label23.Name = "label23";
			this.label23.Size = new System.Drawing.Size(53, 12);
			this.label23.TabIndex = 37;
			this.label23.Text = "설치날짜";
			// 
			// tb_Install_Date
			// 
			this.tb_Install_Date.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Install_Date.Location = new System.Drawing.Point(56, 7);
			this.tb_Install_Date.Name = "tb_Install_Date";
			this.tb_Install_Date.Size = new System.Drawing.Size(104, 21);
			this.tb_Install_Date.TabIndex = 83;
			// 
			// btn_Install_Delete
			// 
			this.btn_Install_Delete.Location = new System.Drawing.Point(123, 28);
			this.btn_Install_Delete.Name = "btn_Install_Delete";
			this.btn_Install_Delete.Size = new System.Drawing.Size(37, 23);
			this.btn_Install_Delete.TabIndex = 80;
			this.btn_Install_Delete.Text = "삭제";
			this.btn_Install_Delete.UseVisualStyleBackColor = true;
			this.btn_Install_Delete.Click += new System.EventHandler(this.btn_Install_Delete_Click);
			// 
			// btn_Install_Write
			// 
			this.btn_Install_Write.Location = new System.Drawing.Point(88, 28);
			this.btn_Install_Write.Name = "btn_Install_Write";
			this.btn_Install_Write.Size = new System.Drawing.Size(37, 23);
			this.btn_Install_Write.TabIndex = 79;
			this.btn_Install_Write.Text = "쓰기";
			this.btn_Install_Write.UseVisualStyleBackColor = true;
			this.btn_Install_Write.Click += new System.EventHandler(this.btn_Install_Write_Click_1);
			// 
			// btn_Install_Read
			// 
			this.btn_Install_Read.Location = new System.Drawing.Point(52, 28);
			this.btn_Install_Read.Name = "btn_Install_Read";
			this.btn_Install_Read.Size = new System.Drawing.Size(37, 23);
			this.btn_Install_Read.TabIndex = 78;
			this.btn_Install_Read.Text = "읽기";
			this.btn_Install_Read.UseVisualStyleBackColor = true;
			this.btn_Install_Read.Click += new System.EventHandler(this.btn_Install_Read_Click);
			// 
			// pnl_reelPart
			// 
			this.pnl_reelPart.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
			this.pnl_reelPart.Location = new System.Drawing.Point(747, 11);
			this.pnl_reelPart.Name = "pnl_reelPart";
			this.pnl_reelPart.Size = new System.Drawing.Size(2, 313);
			this.pnl_reelPart.TabIndex = 86;
			// 
			// panel1
			// 
			this.panel1.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
			this.panel1.Location = new System.Drawing.Point(443, 11);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(2, 403);
			this.panel1.TabIndex = 87;
			// 
			// panel2
			// 
			this.panel2.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
			this.panel2.Location = new System.Drawing.Point(845, 323);
			this.panel2.Name = "panel2";
			this.panel2.Size = new System.Drawing.Size(2, 78);
			this.panel2.TabIndex = 90;
			// 
			// panel3
			// 
			this.panel3.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
			this.panel3.Location = new System.Drawing.Point(747, 323);
			this.panel3.Name = "panel3";
			this.panel3.Size = new System.Drawing.Size(99, 2);
			this.panel3.TabIndex = 91;
			// 
			// groupBox18
			// 
			this.groupBox18.Controls.Add(this.btn_MainEnc_Init);
			this.groupBox18.Controls.Add(this.btn_MainEncoder_StartStop);
			this.groupBox18.Controls.Add(this.lb_MainEncDir);
			this.groupBox18.Controls.Add(this.lb_MainEncCount);
			this.groupBox18.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox18.Location = new System.Drawing.Point(521, 397);
			this.groupBox18.Name = "groupBox18";
			this.groupBox18.Size = new System.Drawing.Size(107, 68);
			this.groupBox18.TabIndex = 92;
			this.groupBox18.TabStop = false;
			this.groupBox18.Text = "엔코더 회수";
			// 
			// btn_MainEnc_Init
			// 
			this.btn_MainEnc_Init.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_MainEnc_Init.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_MainEnc_Init.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_MainEnc_Init.Location = new System.Drawing.Point(46, 39);
			this.btn_MainEnc_Init.Name = "btn_MainEnc_Init";
			this.btn_MainEnc_Init.Size = new System.Drawing.Size(40, 25);
			this.btn_MainEnc_Init.TabIndex = 110;
			this.btn_MainEnc_Init.Text = "초기";
			this.btn_MainEnc_Init.UseVisualStyleBackColor = false;
			this.btn_MainEnc_Init.Click += new System.EventHandler(this.btn_MainEnc_Init_Click);
			// 
			// btn_MainEncoder_StartStop
			// 
			this.btn_MainEncoder_StartStop.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_MainEncoder_StartStop.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_MainEncoder_StartStop.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_MainEncoder_StartStop.Location = new System.Drawing.Point(47, 13);
			this.btn_MainEncoder_StartStop.Name = "btn_MainEncoder_StartStop";
			this.btn_MainEncoder_StartStop.Size = new System.Drawing.Size(40, 25);
			this.btn_MainEncoder_StartStop.TabIndex = 96;
			this.btn_MainEncoder_StartStop.Text = "시작";
			this.btn_MainEncoder_StartStop.UseVisualStyleBackColor = false;
			this.btn_MainEncoder_StartStop.Click += new System.EventHandler(this.btn_MainEncoder_StartStop_Click);
			// 
			// lb_MainEncDir
			// 
			this.lb_MainEncDir.AutoSize = true;
			this.lb_MainEncDir.Font = new System.Drawing.Font("굴림", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lb_MainEncDir.Location = new System.Drawing.Point(5, -41);
			this.lb_MainEncDir.Name = "lb_MainEncDir";
			this.lb_MainEncDir.Size = new System.Drawing.Size(31, 14);
			this.lb_MainEncDir.TabIndex = 1;
			this.lb_MainEncDir.Text = "CW";
			this.lb_MainEncDir.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// lb_MainEncCount
			// 
			this.lb_MainEncCount.AutoSize = true;
			this.lb_MainEncCount.Font = new System.Drawing.Font("굴림", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.lb_MainEncCount.Location = new System.Drawing.Point(6, 41);
			this.lb_MainEncCount.Name = "lb_MainEncCount";
			this.lb_MainEncCount.Size = new System.Drawing.Size(39, 16);
			this.lb_MainEncCount.TabIndex = 0;
			this.lb_MainEncCount.Text = "0000";
			this.lb_MainEncCount.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// panel4
			// 
			this.panel4.BackColor = System.Drawing.SystemColors.InactiveCaptionText;
			this.panel4.Location = new System.Drawing.Point(975, 9);
			this.panel4.Name = "panel4";
			this.panel4.Size = new System.Drawing.Size(2, 650);
			this.panel4.TabIndex = 93;
			// 
			// groupBox20
			// 
			this.groupBox20.Controls.Add(this.nud_AgingCounter);
			this.groupBox20.Controls.Add(this.btn_AgingCnt_Save);
			this.groupBox20.Controls.Add(this.btn_AgingCnt_Clear0);
			this.groupBox20.Controls.Add(this.btn_AgingCnt_Read);
			this.groupBox20.Location = new System.Drawing.Point(979, 12);
			this.groupBox20.Name = "groupBox20";
			this.groupBox20.Size = new System.Drawing.Size(322, 44);
			this.groupBox20.TabIndex = 95;
			this.groupBox20.TabStop = false;
			this.groupBox20.Text = "에이징 회수";
			// 
			// nud_AgingCounter
			// 
			this.nud_AgingCounter.BackColor = System.Drawing.SystemColors.Window;
			this.nud_AgingCounter.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_AgingCounter.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_AgingCounter.Location = new System.Drawing.Point(12, 14);
			this.nud_AgingCounter.Maximum = new decimal(new int[] {
            999999999,
            0,
            0,
            0});
			this.nud_AgingCounter.Name = "nud_AgingCounter";
			this.nud_AgingCounter.Size = new System.Drawing.Size(117, 22);
			this.nud_AgingCounter.TabIndex = 108;
			this.nud_AgingCounter.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_AgingCounter.ThousandsSeparator = true;
			// 
			// btn_AgingCnt_Save
			// 
			this.btn_AgingCnt_Save.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_AgingCnt_Save.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_AgingCnt_Save.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_AgingCnt_Save.Location = new System.Drawing.Point(193, 13);
			this.btn_AgingCnt_Save.Name = "btn_AgingCnt_Save";
			this.btn_AgingCnt_Save.Size = new System.Drawing.Size(51, 25);
			this.btn_AgingCnt_Save.TabIndex = 107;
			this.btn_AgingCnt_Save.Text = "쓰기";
			this.btn_AgingCnt_Save.UseVisualStyleBackColor = false;
			this.btn_AgingCnt_Save.Click += new System.EventHandler(this.btn_AgingCntSave_Click);
			// 
			// btn_AgingCnt_Clear0
			// 
			this.btn_AgingCnt_Clear0.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_AgingCnt_Clear0.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_AgingCnt_Clear0.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_AgingCnt_Clear0.Location = new System.Drawing.Point(244, 13);
			this.btn_AgingCnt_Clear0.Name = "btn_AgingCnt_Clear0";
			this.btn_AgingCnt_Clear0.Size = new System.Drawing.Size(73, 25);
			this.btn_AgingCnt_Clear0.TabIndex = 105;
			this.btn_AgingCnt_Clear0.Text = "클리어(0)";
			this.btn_AgingCnt_Clear0.UseVisualStyleBackColor = false;
			this.btn_AgingCnt_Clear0.Click += new System.EventHandler(this.btn_AgingCntClear_Click);
			// 
			// btn_AgingCnt_Read
			// 
			this.btn_AgingCnt_Read.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_AgingCnt_Read.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_AgingCnt_Read.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_AgingCnt_Read.Location = new System.Drawing.Point(135, 13);
			this.btn_AgingCnt_Read.Name = "btn_AgingCnt_Read";
			this.btn_AgingCnt_Read.Size = new System.Drawing.Size(52, 25);
			this.btn_AgingCnt_Read.TabIndex = 95;
			this.btn_AgingCnt_Read.Text = "읽기";
			this.btn_AgingCnt_Read.UseVisualStyleBackColor = false;
			this.btn_AgingCnt_Read.Click += new System.EventHandler(this.btn_AgingCntRead_Click);
			// 
			// groupBox19
			// 
			this.groupBox19.Controls.Add(this.groupBox22);
			this.groupBox19.Controls.Add(this.btn_WireInitValue);
			this.groupBox19.Controls.Add(this.groupBox21);
			this.groupBox19.Controls.Add(this.lb_AgingResult);
			this.groupBox19.Controls.Add(this.tb_WireAgingResult);
			this.groupBox19.Controls.Add(this.tb_AgingResult);
			this.groupBox19.Controls.Add(this.btn_wireAgingStop);
			this.groupBox19.Controls.Add(this.btn_wireAgingStart);
			this.groupBox19.Location = new System.Drawing.Point(977, 58);
			this.groupBox19.Name = "groupBox19";
			this.groupBox19.Size = new System.Drawing.Size(324, 488);
			this.groupBox19.TabIndex = 109;
			this.groupBox19.TabStop = false;
			this.groupBox19.Text = "에이징 테스트";
			// 
			// groupBox22
			// 
			this.groupBox22.Controls.Add(this.nud_bldc_duty2);
			this.groupBox22.Controls.Add(this.nud_bldc_duty1);
			this.groupBox22.Controls.Add(this.label40);
			this.groupBox22.Controls.Add(this.label39);
			this.groupBox22.Controls.Add(this.label41);
			this.groupBox22.Controls.Add(this.nud_mmot_cw_time2);
			this.groupBox22.Controls.Add(this.nud_mmot_cw_duty2);
			this.groupBox22.Controls.Add(this.label42);
			this.groupBox22.Controls.Add(this.nud_mmot_cw_time1);
			this.groupBox22.Controls.Add(this.nud_mmot_cw_duty1);
			this.groupBox22.Font = new System.Drawing.Font("굴림", 8.5F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.groupBox22.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.groupBox22.Location = new System.Drawing.Point(14, 52);
			this.groupBox22.Name = "groupBox22";
			this.groupBox22.Size = new System.Drawing.Size(310, 81);
			this.groupBox22.TabIndex = 112;
			this.groupBox22.TabStop = false;
			this.groupBox22.Text = "낚시줄  [토크]  [BLDC]   [on시간]";
			// 
			// nud_bldc_duty2
			// 
			this.nud_bldc_duty2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_bldc_duty2.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_bldc_duty2.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_bldc_duty2.Location = new System.Drawing.Point(101, 45);
			this.nud_bldc_duty2.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_bldc_duty2.Name = "nud_bldc_duty2";
			this.nud_bldc_duty2.Size = new System.Drawing.Size(58, 22);
			this.nud_bldc_duty2.TabIndex = 99;
			this.nud_bldc_duty2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_bldc_duty2.ValueChanged += new System.EventHandler(this.nud_bldc_duty2_ValueChanged);
			// 
			// nud_bldc_duty1
			// 
			this.nud_bldc_duty1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_bldc_duty1.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_bldc_duty1.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_bldc_duty1.Location = new System.Drawing.Point(101, 20);
			this.nud_bldc_duty1.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_bldc_duty1.Name = "nud_bldc_duty1";
			this.nud_bldc_duty1.Size = new System.Drawing.Size(58, 22);
			this.nud_bldc_duty1.TabIndex = 98;
			this.nud_bldc_duty1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_bldc_duty1.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
			this.nud_bldc_duty1.ValueChanged += new System.EventHandler(this.nud_bldc_duty1_ValueChanged);
			// 
			// label40
			// 
			this.label40.AutoSize = true;
			this.label40.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label40.Location = new System.Drawing.Point(6, 48);
			this.label40.Name = "label40";
			this.label40.Size = new System.Drawing.Size(35, 12);
			this.label40.TabIndex = 101;
			this.label40.Text = "감기2";
			// 
			// label39
			// 
			this.label39.AutoSize = true;
			this.label39.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label39.Location = new System.Drawing.Point(7, 25);
			this.label39.Name = "label39";
			this.label39.Size = new System.Drawing.Size(35, 12);
			this.label39.TabIndex = 100;
			this.label39.Text = "감기1";
			// 
			// label41
			// 
			this.label41.AutoSize = true;
			this.label41.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label41.Location = new System.Drawing.Point(240, 52);
			this.label41.Name = "label41";
			this.label41.Size = new System.Drawing.Size(23, 12);
			this.label41.TabIndex = 99;
			this.label41.Text = "ms";
			// 
			// nud_mmot_cw_time2
			// 
			this.nud_mmot_cw_time2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_mmot_cw_time2.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_mmot_cw_time2.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_mmot_cw_time2.Location = new System.Drawing.Point(170, 46);
			this.nud_mmot_cw_time2.Maximum = new decimal(new int[] {
            9999,
            0,
            0,
            0});
			this.nud_mmot_cw_time2.Name = "nud_mmot_cw_time2";
			this.nud_mmot_cw_time2.Size = new System.Drawing.Size(69, 22);
			this.nud_mmot_cw_time2.TabIndex = 98;
			this.nud_mmot_cw_time2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_mmot_cw_time2.Value = new decimal(new int[] {
            3000,
            0,
            0,
            0});
			this.nud_mmot_cw_time2.ValueChanged += new System.EventHandler(this.nud_mmot_cw_time2_ValueChanged);
			// 
			// nud_mmot_cw_duty2
			// 
			this.nud_mmot_cw_duty2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_mmot_cw_duty2.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_mmot_cw_duty2.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_mmot_cw_duty2.Location = new System.Drawing.Point(47, 45);
			this.nud_mmot_cw_duty2.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_mmot_cw_duty2.Name = "nud_mmot_cw_duty2";
			this.nud_mmot_cw_duty2.Size = new System.Drawing.Size(58, 22);
			this.nud_mmot_cw_duty2.TabIndex = 97;
			this.nud_mmot_cw_duty2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_mmot_cw_duty2.Value = new decimal(new int[] {
            80,
            0,
            0,
            0});
			this.nud_mmot_cw_duty2.ValueChanged += new System.EventHandler(this.nud_mmot_cw_duty2_ValueChanged);
			// 
			// label42
			// 
			this.label42.AutoSize = true;
			this.label42.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label42.Location = new System.Drawing.Point(239, 28);
			this.label42.Name = "label42";
			this.label42.Size = new System.Drawing.Size(23, 12);
			this.label42.TabIndex = 95;
			this.label42.Text = "ms";
			// 
			// nud_mmot_cw_time1
			// 
			this.nud_mmot_cw_time1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_mmot_cw_time1.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_mmot_cw_time1.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_mmot_cw_time1.Location = new System.Drawing.Point(170, 21);
			this.nud_mmot_cw_time1.Maximum = new decimal(new int[] {
            9999,
            0,
            0,
            0});
			this.nud_mmot_cw_time1.Name = "nud_mmot_cw_time1";
			this.nud_mmot_cw_time1.Size = new System.Drawing.Size(69, 22);
			this.nud_mmot_cw_time1.TabIndex = 15;
			this.nud_mmot_cw_time1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_mmot_cw_time1.Value = new decimal(new int[] {
            2000,
            0,
            0,
            0});
			this.nud_mmot_cw_time1.ValueChanged += new System.EventHandler(this.nud_mmot_cw_time1_ValueChanged);
			// 
			// nud_mmot_cw_duty1
			// 
			this.nud_mmot_cw_duty1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_mmot_cw_duty1.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_mmot_cw_duty1.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.nud_mmot_cw_duty1.Location = new System.Drawing.Point(47, 20);
			this.nud_mmot_cw_duty1.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_mmot_cw_duty1.Name = "nud_mmot_cw_duty1";
			this.nud_mmot_cw_duty1.Size = new System.Drawing.Size(58, 22);
			this.nud_mmot_cw_duty1.TabIndex = 13;
			this.nud_mmot_cw_duty1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_mmot_cw_duty1.Value = new decimal(new int[] {
            150,
            0,
            0,
            0});
			this.nud_mmot_cw_duty1.ValueChanged += new System.EventHandler(this.nud_mmot_cw_duty1_ValueChanged);
			// 
			// btn_WireInitValue
			// 
			this.btn_WireInitValue.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_WireInitValue.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_WireInitValue.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_WireInitValue.Location = new System.Drawing.Point(13, 20);
			this.btn_WireInitValue.Name = "btn_WireInitValue";
			this.btn_WireInitValue.Size = new System.Drawing.Size(60, 25);
			this.btn_WireInitValue.TabIndex = 96;
			this.btn_WireInitValue.Text = "초기값";
			this.btn_WireInitValue.UseVisualStyleBackColor = false;
			this.btn_WireInitValue.Click += new System.EventHandler(this.btn_WireInitValue_Click);
			// 
			// groupBox21
			// 
			this.groupBox21.ForeColor = System.Drawing.SystemColors.AppWorkspace;
			this.groupBox21.Location = new System.Drawing.Point(4, 138);
			this.groupBox21.Name = "groupBox21";
			this.groupBox21.Size = new System.Drawing.Size(318, 343);
			this.groupBox21.TabIndex = 110;
			this.groupBox21.TabStop = false;
			this.groupBox21.Text = "에이징 명령 시퀀스";
			// 
			// lb_AgingResult
			// 
			this.lb_AgingResult.AutoSize = true;
			this.lb_AgingResult.Location = new System.Drawing.Point(224, 25);
			this.lb_AgingResult.Name = "lb_AgingResult";
			this.lb_AgingResult.Size = new System.Drawing.Size(29, 12);
			this.lb_AgingResult.TabIndex = 110;
			this.lb_AgingResult.Text = "결과";
			// 
			// tb_WireAgingResult
			// 
			this.tb_WireAgingResult.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_WireAgingResult.Location = new System.Drawing.Point(259, 21);
			this.tb_WireAgingResult.Name = "tb_WireAgingResult";
			this.tb_WireAgingResult.Size = new System.Drawing.Size(65, 21);
			this.tb_WireAgingResult.TabIndex = 96;
			this.tb_WireAgingResult.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// tb_AgingResult
			// 
			this.tb_AgingResult.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.tb_AgingResult.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_AgingResult.Location = new System.Drawing.Point(212, -90);
			this.tb_AgingResult.Name = "tb_AgingResult";
			this.tb_AgingResult.Size = new System.Drawing.Size(124, 21);
			this.tb_AgingResult.TabIndex = 108;
			this.tb_AgingResult.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// btn_wireAgingStop
			// 
			this.btn_wireAgingStop.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_wireAgingStop.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_wireAgingStop.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_wireAgingStop.Location = new System.Drawing.Point(152, 18);
			this.btn_wireAgingStop.Name = "btn_wireAgingStop";
			this.btn_wireAgingStop.Size = new System.Drawing.Size(60, 25);
			this.btn_wireAgingStop.TabIndex = 6;
			this.btn_wireAgingStop.Text = "정지";
			this.btn_wireAgingStop.UseVisualStyleBackColor = false;
			this.btn_wireAgingStop.Click += new System.EventHandler(this.btn_wireAgingStop_Click);
			// 
			// btn_wireAgingStart
			// 
			this.btn_wireAgingStart.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_wireAgingStart.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_wireAgingStart.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_wireAgingStart.Location = new System.Drawing.Point(90, 18);
			this.btn_wireAgingStart.Name = "btn_wireAgingStart";
			this.btn_wireAgingStart.Size = new System.Drawing.Size(58, 25);
			this.btn_wireAgingStart.TabIndex = 14;
			this.btn_wireAgingStart.Text = "시작";
			this.btn_wireAgingStart.UseVisualStyleBackColor = false;
			this.btn_wireAgingStart.Click += new System.EventHandler(this.btn_wireAgingStart_Click);
			// 
			// tb_Error_OkMsg
			// 
			this.tb_Error_OkMsg.Location = new System.Drawing.Point(983, 641);
			this.tb_Error_OkMsg.Name = "tb_Error_OkMsg";
			this.tb_Error_OkMsg.Size = new System.Drawing.Size(318, 21);
			this.tb_Error_OkMsg.TabIndex = 110;
			// 
			// rTb_Error_ErrMsg
			// 
			this.rTb_Error_ErrMsg.Location = new System.Drawing.Point(983, 542);
			this.rTb_Error_ErrMsg.Name = "rTb_Error_ErrMsg";
			this.rTb_Error_ErrMsg.Size = new System.Drawing.Size(318, 93);
			this.rTb_Error_ErrMsg.TabIndex = 111;
			this.rTb_Error_ErrMsg.Text = "";
			// 
			// groupBox23
			// 
			this.groupBox23.Controls.Add(this.tb_Bldc_Off_time);
			this.groupBox23.Controls.Add(this.tb_Torq_Off_time);
			this.groupBox23.Controls.Add(this.label44);
			this.groupBox23.Controls.Add(this.nud_BM_together);
			this.groupBox23.Controls.Add(this.label45);
			this.groupBox23.Controls.Add(this.label43);
			this.groupBox23.Controls.Add(this.nud_time_together);
			this.groupBox23.Controls.Add(this.btn_on_motor_together);
			this.groupBox23.Controls.Add(this.btn_off_motor_together);
			this.groupBox23.Controls.Add(this.nud_TM_together);
			this.groupBox23.Controls.Add(this.label33);
			this.groupBox23.Location = new System.Drawing.Point(449, 177);
			this.groupBox23.Name = "groupBox23";
			this.groupBox23.Size = new System.Drawing.Size(298, 66);
			this.groupBox23.TabIndex = 112;
			this.groupBox23.TabStop = false;
			this.groupBox23.Text = "모터 동시 제어";
			// 
			// tb_Bldc_Off_time
			// 
			this.tb_Bldc_Off_time.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.tb_Bldc_Off_time.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Bldc_Off_time.Location = new System.Drawing.Point(227, 40);
			this.tb_Bldc_Off_time.Name = "tb_Bldc_Off_time";
			this.tb_Bldc_Off_time.Size = new System.Drawing.Size(65, 21);
			this.tb_Bldc_Off_time.TabIndex = 20;
			this.tb_Bldc_Off_time.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// tb_Torq_Off_time
			// 
			this.tb_Torq_Off_time.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.tb_Torq_Off_time.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.tb_Torq_Off_time.Location = new System.Drawing.Point(227, 13);
			this.tb_Torq_Off_time.Name = "tb_Torq_Off_time";
			this.tb_Torq_Off_time.Size = new System.Drawing.Size(65, 21);
			this.tb_Torq_Off_time.TabIndex = 19;
			this.tb_Torq_Off_time.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			// 
			// label44
			// 
			this.label44.AutoSize = true;
			this.label44.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label44.Location = new System.Drawing.Point(5, 41);
			this.label44.Name = "label44";
			this.label44.Size = new System.Drawing.Size(37, 12);
			this.label44.TabIndex = 18;
			this.label44.Text = "BLDC";
			// 
			// nud_BM_together
			// 
			this.nud_BM_together.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_BM_together.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_BM_together.Location = new System.Drawing.Point(45, 38);
			this.nud_BM_together.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_BM_together.Name = "nud_BM_together";
			this.nud_BM_together.Size = new System.Drawing.Size(55, 22);
			this.nud_BM_together.TabIndex = 17;
			this.nud_BM_together.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_BM_together.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
			// 
			// label45
			// 
			this.label45.AutoSize = true;
			this.label45.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label45.Location = new System.Drawing.Point(224, 0);
			this.label45.Name = "label45";
			this.label45.Size = new System.Drawing.Size(70, 12);
			this.label45.TabIndex = 16;
			this.label45.Text = "OFF시간ms";
			// 
			// label43
			// 
			this.label43.AutoSize = true;
			this.label43.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label43.Location = new System.Drawing.Point(101, 22);
			this.label43.Name = "label43";
			this.label43.Size = new System.Drawing.Size(49, 12);
			this.label43.TabIndex = 16;
			this.label43.Text = "On(ms)";
			// 
			// nud_time_together
			// 
			this.nud_time_together.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_time_together.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_time_together.Location = new System.Drawing.Point(104, 39);
			this.nud_time_together.Maximum = new decimal(new int[] {
            9999,
            0,
            0,
            0});
			this.nud_time_together.Name = "nud_time_together";
			this.nud_time_together.Size = new System.Drawing.Size(69, 22);
			this.nud_time_together.TabIndex = 15;
			this.nud_time_together.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			// 
			// btn_on_motor_together
			// 
			this.btn_on_motor_together.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_on_motor_together.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_on_motor_together.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_on_motor_together.Location = new System.Drawing.Point(173, 12);
			this.btn_on_motor_together.Name = "btn_on_motor_together";
			this.btn_on_motor_together.Size = new System.Drawing.Size(51, 25);
			this.btn_on_motor_together.TabIndex = 14;
			this.btn_on_motor_together.Text = "ON";
			this.btn_on_motor_together.UseVisualStyleBackColor = false;
			this.btn_on_motor_together.Click += new System.EventHandler(this.btn_on_motor_together_Click);
			// 
			// btn_off_motor_together
			// 
			this.btn_off_motor_together.BackColor = System.Drawing.Color.Gainsboro;
			this.btn_off_motor_together.FlatStyle = System.Windows.Forms.FlatStyle.System;
			this.btn_off_motor_together.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.btn_off_motor_together.Location = new System.Drawing.Point(173, 36);
			this.btn_off_motor_together.Name = "btn_off_motor_together";
			this.btn_off_motor_together.Size = new System.Drawing.Size(50, 25);
			this.btn_off_motor_together.TabIndex = 6;
			this.btn_off_motor_together.Text = "OFF";
			this.btn_off_motor_together.UseVisualStyleBackColor = false;
			this.btn_off_motor_together.Click += new System.EventHandler(this.btn_off_motor_together_Click);
			// 
			// nud_TM_together
			// 
			this.nud_TM_together.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.nud_TM_together.Font = new System.Drawing.Font("굴림", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.nud_TM_together.Location = new System.Drawing.Point(45, 12);
			this.nud_TM_together.Maximum = new decimal(new int[] {
            255,
            0,
            0,
            0});
			this.nud_TM_together.Name = "nud_TM_together";
			this.nud_TM_together.Size = new System.Drawing.Size(55, 22);
			this.nud_TM_together.TabIndex = 13;
			this.nud_TM_together.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
			this.nud_TM_together.Value = new decimal(new int[] {
            70,
            0,
            0,
            0});
			// 
			// label33
			// 
			this.label33.AutoSize = true;
			this.label33.Font = new System.Drawing.Font("굴림", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.label33.Location = new System.Drawing.Point(4, 16);
			this.label33.Name = "label33";
			this.label33.Size = new System.Drawing.Size(39, 12);
			this.label33.TabIndex = 3;
			this.label33.Text = "TORQ";
			// 
			// btn_Download
			// 
			this.btn_Download.Location = new System.Drawing.Point(9, 373);
			this.btn_Download.Name = "btn_Download";
			this.btn_Download.Size = new System.Drawing.Size(108, 28);
			this.btn_Download.TabIndex = 114;
			this.btn_Download.Text = "보드 다운로드";
			this.btn_Download.UseVisualStyleBackColor = true;
			this.btn_Download.Click += new System.EventHandler(this.btn_BoardDownload_Click);
			// 
			// DFTMmainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 11F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(1305, 742);
			this.Controls.Add(this.btn_Download);
			this.Controls.Add(this.groupBox23);
			this.Controls.Add(this.rTb_Error_ErrMsg);
			this.Controls.Add(this.tb_Error_OkMsg);
			this.Controls.Add(this.groupBox19);
			this.Controls.Add(this.groupBox12);
			this.Controls.Add(this.groupBox20);
			this.Controls.Add(this.panel4);
			this.Controls.Add(this.groupBox18);
			this.Controls.Add(this.panel3);
			this.Controls.Add(this.panel2);
			this.Controls.Add(this.panel1);
			this.Controls.Add(this.pnl_reelPart);
			this.Controls.Add(this.InstallGroup);
			this.Controls.Add(this.ManufInfo_Group);
			this.Controls.Add(this.setupCmd_Group);
			this.Controls.Add(this.PowerSwitch);
			this.Controls.Add(this.groupBox17);
			this.Controls.Add(this.groupBox11);
			this.Controls.Add(this.groupBox16);
			this.Controls.Add(this.SendCmdGroup);
			this.Controls.Add(this.groupBox14);
			this.Controls.Add(this.label14);
			this.Controls.Add(this.groupBox8);
			this.Controls.Add(this.groupBox10);
			this.Controls.Add(this.TextBox_Error_Content);
			this.Controls.Add(this.Textbox_Error_Code);
			this.Controls.Add(this.groupBox9);
			this.Controls.Add(this.groupBox13);
			this.Controls.Add(this.groupBox15);
			this.Controls.Add(this.groupBox7);
			this.Controls.Add(this.groupBox4);
			this.Controls.Add(this.groupBox6);
			this.Controls.Add(this.groupBox5);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.sendCmd_Group);
			this.Controls.Add(this.groupBox1);
			this.Font = new System.Drawing.Font("굴림", 8.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.Name = "DFTMmainForm";
			this.Text = "DFTM";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DFTM_FormClosing);
			this.Load += new System.EventHandler(this.DFTM_Load);
			((System.ComponentModel.ISupportInitialize)(this.Tb_Drag_TorqVal)).EndInit();
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.sendCmd_Group.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_Torque)).EndInit();
			this.groupBox6.ResumeLayout(false);
			this.groupBox7.ResumeLayout(false);
			this.groupBox7.PerformLayout();
			this.groupBox9.ResumeLayout(false);
			this.groupBox9.PerformLayout();
			this.groupBox10.ResumeLayout(false);
			this.groupBox10.PerformLayout();
			this.groupBox11.ResumeLayout(false);
			this.groupBox11.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedRead)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedControlTime)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedColorNo)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedPosNo)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.numUpDown_LedModeNo)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.updown_Bobbin_Duty)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.updown_Bobbin_timems)).EndInit();
			this.groupBox5.ResumeLayout(false);
			this.groupBox5.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.Tb_BLDC_OnTime)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.Tb_BLDC_Duty)).EndInit();
			this.groupBox13.ResumeLayout(false);
			this.groupBox13.PerformLayout();
			this.groupBox8.ResumeLayout(false);
			this.groupBox8.PerformLayout();
			this.groupBox14.ResumeLayout(false);
			this.SendCmdGroup.ResumeLayout(false);
			this.SendCmdGroup.PerformLayout();
			this.groupBox16.ResumeLayout(false);
			this.groupBox16.PerformLayout();
			this.groupBox4.ResumeLayout(false);
			this.groupBox4.PerformLayout();
			this.groupBox15.ResumeLayout(false);
			this.groupBox15.PerformLayout();
			this.groupBox12.ResumeLayout(false);
			this.groupBox12.PerformLayout();
			this.groupBox17.ResumeLayout(false);
			this.groupBox17.PerformLayout();
			this.PowerSwitch.ResumeLayout(false);
			this.setupCmd_Group.ResumeLayout(false);
			this.setupCmd_Group.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			this.ManufInfo_Group.ResumeLayout(false);
			this.ManufInfo_Group.PerformLayout();
			this.InstallGroup.ResumeLayout(false);
			this.InstallGroup.PerformLayout();
			this.groupBox18.ResumeLayout(false);
			this.groupBox18.PerformLayout();
			this.groupBox20.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.nud_AgingCounter)).EndInit();
			this.groupBox19.ResumeLayout(false);
			this.groupBox19.PerformLayout();
			this.groupBox22.ResumeLayout(false);
			this.groupBox22.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.nud_bldc_duty2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_bldc_duty1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_time2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_duty2)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_time1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_mmot_cw_duty1)).EndInit();
			this.groupBox23.ResumeLayout(false);
			this.groupBox23.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.nud_BM_together)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_time_together)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nud_TM_together)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

        }

#endregion

        private System.Windows.Forms.Button Btn_Port_Open;
        private System.Windows.Forms.ComboBox Cb_Ports;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button Btn_VersionRead;
        private System.Windows.Forms.Button Btn_GameStart;
        private System.Windows.Forms.TrackBar Tb_Drag_TorqVal;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox sendCmd_Group;
        private System.Windows.Forms.GroupBox groupBox3;
        //private System.Windows.Forms.TrackBar Tb_Break_Motor;
        private System.Windows.Forms.GroupBox groupBox6;
        private System.Windows.Forms.Label Lab_Btn_Right;
        private System.Windows.Forms.Label Lab_Btn_Left;
        private System.Windows.Forms.GroupBox groupBox7;
        private System.Windows.Forms.Label Lab_Rotate_Cnt;
        private System.Windows.Forms.GroupBox groupBox9;
        private System.Windows.Forms.TextBox Tb_Yaw;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.TextBox Tb_Pitch;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.TextBox Tb_Roll;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.TextBox Tb_Az;
        private System.Windows.Forms.Label label15;
        private System.Windows.Forms.TextBox Tb_Ay;
        private System.Windows.Forms.Label label16;
        private System.Windows.Forms.TextBox Tb_Ax;
        private System.Windows.Forms.Label label17;
        private System.Windows.Forms.Button Btn_IMU_On;
        private System.Windows.Forms.Button Btn_IMU_Off;
        private System.Windows.Forms.GroupBox groupBox10;
        private System.Windows.Forms.Label Lab_Battery;
        private System.Windows.Forms.NumericUpDown numUpDown_Torque;
        //private System.Windows.Forms.NumericUpDown numUpDown_Break;
        private System.Windows.Forms.GroupBox groupBox11;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numUpDown_LedModeNo;
        private System.Windows.Forms.NumericUpDown updown_Bobbin_Duty;
        private System.Windows.Forms.NumericUpDown updown_Bobbin_timems;
        private System.Windows.Forms.RadioButton radio_BLDC_CW;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.RadioButton radio_BLDC_CCW;
        private System.Windows.Forms.Button btn_BLDC_On;
        private System.Windows.Forms.GroupBox groupBox5;
        private System.Windows.Forms.Button Btn_Port_Close;
        private System.Windows.Forms.TrackBar Tb_BLDC_OnTime;
        private System.Windows.Forms.TrackBar Tb_BLDC_Duty;
        private System.Windows.Forms.Button btn_BLDC_Off;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label lbl_CW_CCW;
        private System.Windows.Forms.Button btn_Torque_OFF;
        //private System.Windows.Forms.Button btn_Break_OFF;
        private System.Windows.Forms.Button btn_LED_Off;
        private System.Windows.Forms.Label lblLedPos;
        private System.Windows.Forms.Button btn_LED_On;
        private System.Windows.Forms.NumericUpDown numUpDown_LedPosNo;
        private System.Windows.Forms.NumericUpDown numUpDown_LedControlTime;
        private System.Windows.Forms.NumericUpDown numUpDown_LedColorNo;
        private System.Windows.Forms.Label label18;
        private System.Windows.Forms.Label lblLedColor;
        private System.Windows.Forms.Label lblLedMode;
        private System.Windows.Forms.NumericUpDown numUpDown_LedRead;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.Button btn_LED_Read;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btn_LED_Write;
        private System.Windows.Forms.GroupBox groupBox13;
        private System.Windows.Forms.Label label21;
        private System.Windows.Forms.Label label22;
        private System.Windows.Forms.Label Lab_IMU_Conn;
        private System.Windows.Forms.Label Lab_Slave_Conn;
        private System.Windows.Forms.Label label25;
        private System.Windows.Forms.Label Lab_Main_Conn;
        private System.Windows.Forms.GroupBox groupBox8;
        private System.Windows.Forms.TextBox Textbox_Imu_Ver;
        private System.Windows.Forms.TextBox Textbox_Main_Ver;
        private System.Windows.Forms.TextBox Textbox_Rod_Ver;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.TextBox Textbox_Error_Code;
        private System.Windows.Forms.TextBox TextBox_Error_Content;
		private System.Windows.Forms.GroupBox groupBox14;
		private System.Windows.Forms.Button btn_REEL_LED_right;
		private System.Windows.Forms.Button btn_REEL_LED_left;
		private System.Windows.Forms.Button btn_REEL_motor;
		private System.Windows.Forms.Button btn_REEL_OFF;
		private System.Windows.Forms.Button btn_REEL_ON;

        private System.Windows.Forms.GroupBox SendCmdGroup;
		private System.Windows.Forms.TextBox tb_CustomSend4;
		private System.Windows.Forms.TextBox tb_CustomSend3;
		private System.Windows.Forms.TextBox tb_CustomSend2;
		private System.Windows.Forms.TextBox tb_CustomSend1;
		private System.Windows.Forms.Button btn_Custom_send4;
		private System.Windows.Forms.Button btn_Custom_send3;
		private System.Windows.Forms.Button btn_Custom_send2;
		private System.Windows.Forms.Button btn_Custom_send1;
		private System.Windows.Forms.Label label30;

		private System.Windows.Forms.GroupBox groupBox16;
		private System.Windows.Forms.RichTextBox tb_LOG_Window;
		private System.Windows.Forms.Label label27;
		private System.Windows.Forms.Button btn_log_clear;
		private System.Windows.Forms.Button btn_log_save;
		private System.Windows.Forms.Button btn_log_autoscroll;
        private System.Windows.Forms.Button btn_Custom_send6;
        private System.Windows.Forms.Button btn_Custom_send5;
        private System.Windows.Forms.TextBox tb_CustomSend6;
        private System.Windows.Forms.TextBox tb_CustomSend5;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Label Lab_Boot;
        private System.Windows.Forms.Label Lab_Setup;
        private System.Windows.Forms.GroupBox groupBox15;
        private System.Windows.Forms.Label Lab_ImuInterval;
        private System.Windows.Forms.Label label28;
        private System.Windows.Forms.GroupBox groupBox12;
        private System.Windows.Forms.Label Cont_Pwr_Sw;
        private System.Windows.Forms.Label Cont_usb5V;
        private System.Windows.Forms.Label Cont_Boot_Sw;
        private System.Windows.Forms.GroupBox groupBox17;
        private System.Windows.Forms.Label BbnFg;
        private System.Windows.Forms.Label BbnEncB;
        private System.Windows.Forms.Label BbnEncA;
        private System.Windows.Forms.Label label31;
        private System.Windows.Forms.Label label32;
        private System.Windows.Forms.Button Btn_Bbn_In_Start;
        private System.Windows.Forms.Button Btn_Bbn_In_Stop;
        private System.Windows.Forms.Button Btn_SubAc_Off;
        private System.Windows.Forms.GroupBox PowerSwitch;
        private System.Windows.Forms.Button Btn_SubAc_On;
        private System.Windows.Forms.Label label19;
        private System.Windows.Forms.Button Btn_Device_Check;
        private System.Windows.Forms.GroupBox setupCmd_Group;
        private System.Windows.Forms.Button btn_Reel_Regist;
        private System.Windows.Forms.GroupBox ManufInfo_Group;
        private System.Windows.Forms.Button btn_Manuf_Erase;
        private System.Windows.Forms.Button btn_Manuf_Write;
        private System.Windows.Forms.Button btn_Manuf_Read;
        private System.Windows.Forms.TextBox tb_Manuf_Date;
        private System.Windows.Forms.TextBox tb_Manuf_SerialNo;
        private System.Windows.Forms.TextBox tb_Manuf_Model;
        private System.Windows.Forms.GroupBox InstallGroup;
        private System.Windows.Forms.TextBox tb_Install_Date;
        private System.Windows.Forms.Button btn_Install_Delete;
        private System.Windows.Forms.Button btn_Install_Write;
        private System.Windows.Forms.Button btn_Install_Read;
        private System.Windows.Forms.Label label20;
        private System.Windows.Forms.Label label23;
        private System.Windows.Forms.Button Btn_Manuf_Init;
        private System.Windows.Forms.Button Btn_Install_Init;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button btn_Mmot_Init;
        private System.Windows.Forms.Label label24;
        private System.Windows.Forms.TextBox tb_MainMot_Default;
        private System.Windows.Forms.Button btn_Mmot_Write;
        private System.Windows.Forms.Button btn_Mmot_Read;
        private System.Windows.Forms.Button btn_Torue_On;
        private System.Windows.Forms.TextBox tb_ReelRegi_Result;
        private System.Windows.Forms.TextBox tb_Bat_Charging;
        private System.Windows.Forms.Label label26;
        private System.Windows.Forms.TextBox tb_Manuf_No;
        private System.Windows.Forms.Label label29;
        private System.Windows.Forms.DomainUpDown dud_person;
        private System.Windows.Forms.DomainUpDown dud_Spec;
        private System.Windows.Forms.DomainUpDown dud_config;
        private System.Windows.Forms.Button btn_MainTorqAutoSet;
        private System.Windows.Forms.DomainUpDown dud_SerialNumber;
        private System.Windows.Forms.DomainUpDown dud_National;
        private System.Windows.Forms.DomainUpDown dud_Service;
        private System.Windows.Forms.Label label36;
        private System.Windows.Forms.TextBox tb_MainAutoSet_result;
        private System.Windows.Forms.Panel pnl_reelPart;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label lb_BoardVer;
        private System.Windows.Forms.TextBox tb_MainBoardVer;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.GroupBox groupBox18;
        private System.Windows.Forms.Label lb_MainEncCount;
        private System.Windows.Forms.DomainUpDown domainUpDown1;
        private System.Windows.Forms.DomainUpDown domainUpDown2;
        private System.Windows.Forms.DomainUpDown domainUpDown3;
        private System.Windows.Forms.Label label37;
        public System.Windows.Forms.CheckBox cb_DTR;
        public System.Windows.Forms.CheckBox cb_RTS;
        private System.Windows.Forms.Panel panel4;
        private System.Windows.Forms.GroupBox groupBox20;
        private System.Windows.Forms.Button btn_AgingCnt_Clear0;
        private System.Windows.Forms.Button btn_AgingCnt_Read;
        private System.Windows.Forms.NumericUpDown nud_AgingCounter;
        private System.Windows.Forms.Button btn_AgingCnt_Save;
        private System.Windows.Forms.Label lb_MainEncDir;
        private System.Windows.Forms.Button btn_MainEncoder_StartStop;
        private System.Windows.Forms.GroupBox groupBox19;
        private System.Windows.Forms.TextBox tb_AgingResult;
        private System.Windows.Forms.Label lb_AgingResult;
        private System.Windows.Forms.GroupBox groupBox21;
        private System.Windows.Forms.Button btn_MainEnc_Init;
        private System.Windows.Forms.Label label34;
        private System.Windows.Forms.Label label35;
        private System.Windows.Forms.TextBox tb_Error_OkMsg;
        private System.Windows.Forms.RichTextBox rTb_Error_ErrMsg;
        private System.Windows.Forms.Label label38;
        public System.Windows.Forms.TextBox tb_LogSaveMin;      // LOG저장 시간 간격 설정
        private System.Windows.Forms.GroupBox groupBox22;
        private System.Windows.Forms.NumericUpDown nud_bldc_duty2;
        private System.Windows.Forms.NumericUpDown nud_bldc_duty1;
        private System.Windows.Forms.Label label40;
        private System.Windows.Forms.Label label39;
        private System.Windows.Forms.Label label41;
        private System.Windows.Forms.NumericUpDown nud_mmot_cw_time2;
        private System.Windows.Forms.NumericUpDown nud_mmot_cw_duty2;
        private System.Windows.Forms.Label label42;
        private System.Windows.Forms.NumericUpDown nud_mmot_cw_time1;
        private System.Windows.Forms.NumericUpDown nud_mmot_cw_duty1;
        private System.Windows.Forms.Button btn_WireInitValue;
        private System.Windows.Forms.TextBox tb_WireAgingResult;
        private System.Windows.Forms.Button btn_wireAgingStop;
        private System.Windows.Forms.Button btn_wireAgingStart;
        private System.Windows.Forms.GroupBox groupBox23;
        private System.Windows.Forms.Button btn_on_motor_together;
        private System.Windows.Forms.Button btn_off_motor_together;
        private System.Windows.Forms.NumericUpDown nud_TM_together;
        private System.Windows.Forms.Label label33;
        private System.Windows.Forms.Label label44;
        private System.Windows.Forms.NumericUpDown nud_BM_together;
        private System.Windows.Forms.Label label43;
        private System.Windows.Forms.NumericUpDown nud_time_together;
        private System.Windows.Forms.TextBox tb_Bldc_Off_time;
        private System.Windows.Forms.TextBox tb_Torq_Off_time;
        private System.Windows.Forms.Label label45;
		public System.Windows.Forms.TextBox tb_Poll_Interval;
		private System.Windows.Forms.Label label46;
		private System.Windows.Forms.Button btn_Download;
	}
}

