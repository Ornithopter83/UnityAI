using System.Drawing;
using System.Globalization;
using System.Text;

namespace DFTestModule;

internal sealed class MainForm : Form
{
    private readonly SerialConnection serial = new();
    private readonly FrameDecoder decoder = new();
    private readonly FirmwareDownloader downloader;
    private readonly EspToolWiredUpdater wiredUpdater = new();
    private readonly RodWirelessUpdater wirelessUpdater;
    private readonly Dictionary<string, Panel> pages = new();
    private readonly Dictionary<string, Label> statusLabels = new();
    private readonly ComboBox portBox = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 100 };
    private readonly Button connectButton = new() { Text = "연결", Width = 78 };
    private readonly Label connectionState = new() { Text = "연결 안 됨", AutoSize = true, ForeColor = Color.DimGray };
    private readonly Label mainVersion = ValueLabel("Vm -");
    private readonly Label rodVersion = ValueLabel("Vr -");
    private readonly Label imuVersion = ValueLabel("IMU -");
    private readonly RichTextBox logBox = new() { Dock = DockStyle.Fill, ReadOnly = true, BackColor = Color.White, Font = new Font("Consolas", 9F) };
    private readonly CheckBox autoScroll = new() { Text = "Auto Scroll", Checked = true, AutoSize = true };
    private readonly Panel pageHost = new() { Dock = DockStyle.Fill, Padding = new Padding(14), BackColor = Color.FromArgb(247, 248, 250) };
    private readonly FlowLayoutPanel commandRows = new() { Dock = DockStyle.Top, Height = 206, FlowDirection = FlowDirection.TopDown, WrapContents = false };
    private readonly System.Windows.Forms.Timer pollTimer = new() { Interval = 2000 };
    private readonly ProgressBar firmwareProgress = new() { Width = 440, Height = 24 };
    private readonly Label firmwareInfo = new() { AutoSize = true, MaximumSize = new Size(600, 0), Text = "업데이트 종류를 선택하세요." };
    private readonly Button mainUpdateButton = UpdateButton("MAIN 펌웨어 업데이트");
    private readonly Button rodWirelessButton = UpdateButton("ROD 펌웨어 업데이트 (무선)");
    private readonly Button rodWiredButton = UpdateButton("ROD 펌웨어 업데이트 (유선)");
    private readonly Button rodRegisterButton = UpdateButton("ROD 등록");
    private readonly Label registrationState = ValueLabel("등록 대기 안 함");
    private readonly Label leftButtonState = Indicator("왼쪽");
    private readonly Label rightButtonState = Indicator("오른쪽");
    private readonly Label setupButtonState = Indicator("설정");
    private readonly Label bootButtonState = Indicator("부트");
    private readonly Label reelConnectionState = Indicator("릴 연결 대기");
    private readonly Label imuConnectionState = Indicator("IMU 연결 대기");
    private readonly ImuOrientationView imuOrientation = new() { Width = 300, Height = 150, Margin = new Padding(18, 0, 0, 0) };
    private readonly Button imuZeroButton = SmallButton("0점 보정", 100);
    private bool firmwareBusy;
    private bool rodConnected;
    private bool imuConnected;
    private bool imuDataEnabled;
    private bool encoderMonitorEnabled;
    private bool powerOffRequestObserved;
    private bool registrationWaiting;

    public MainForm()
    {
        downloader = new FirmwareDownloader(serial);
        wirelessUpdater = new RodWirelessUpdater(serial);
        Text = Application.ProductName;
        StartPosition = FormStartPosition.CenterScreen;
        MinimumSize = new Size(1280, 770);
        ClientSize = new Size(1500, 980);
        WindowState = FormWindowState.Maximized;
        Font = new Font("맑은 고딕", 9F);
        BuildLayout();
        serial.BytesReceived += OnBytesReceived;
        pollTimer.Tick += (_, _) => { if (serial.IsOpen && !TrySend("$00%", false)) pollTimer.Stop(); };
        Shown += (_, _) => RefreshPorts();
        FormClosing += (_, _) => { pollTimer.Stop(); serial.Dispose(); };
    }

    private void BuildLayout()
    {
        Controls.Add(BuildBody());
        Controls.Add(BuildConnectionBar());
        CreatePages();
        ShowPage("모터 제어");
    }

    private Control BuildConnectionBar()
    {
        Panel top = new() { Dock = DockStyle.Top, Height = 76, BackColor = Color.White, Padding = new Padding(16, 13, 16, 10) };
        FlowLayoutPanel row = new() { Dock = DockStyle.Fill, WrapContents = false, FlowDirection = FlowDirection.LeftToRight };
        Label title = new() { Text = Application.ProductName, AutoSize = true, Font = new Font("맑은 고딕", 15F, FontStyle.Bold), Margin = new Padding(0, 5, 30, 0) };
        Button refresh = new() { Text = "재검색", Width = 76, Height = 30 };
        refresh.Click += (_, _) => RefreshPorts();
        connectButton.Height = 30;
        connectButton.Click += async (_, _) => await ToggleConnectionAsync();
        row.Controls.Add(title); row.Controls.Add(Caption("시리얼")); row.Controls.Add(portBox); row.Controls.Add(connectButton); row.Controls.Add(refresh);
        row.Controls.Add(connectionState); row.Controls.Add(Spacer(20)); row.Controls.Add(Caption("버전"));
        row.Controls.Add(mainVersion); row.Controls.Add(rodVersion); row.Controls.Add(imuVersion);
        top.Controls.Add(row);
        return top;
    }

    private Control BuildBody()
    {
        TableLayoutPanel body = new() { Dock = DockStyle.Fill, ColumnCount = 3, BackColor = Color.FromArgb(232, 235, 239) };
        body.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 160));
        body.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        body.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 500));
        body.Controls.Add(BuildNavigation(), 0, 0);
        body.Controls.Add(pageHost, 1, 0);
        body.Controls.Add(BuildRightRail(), 2, 0);
        return body;
    }

    private Control BuildNavigation()
    {
        Panel panel = new() { Dock = DockStyle.Fill, BackColor = Color.FromArgb(38, 45, 56), Padding = new Padding(10, 18, 10, 10) };
        FlowLayoutPanel buttons = new() { Dock = DockStyle.Top, FlowDirection = FlowDirection.TopDown, WrapContents = false, Height = 400 };
        foreach (string name in new[] { "모터 제어", "릴 / IMU", "입력 / 전원", "LED", "LM JIG", "업데이트" }) {
            Button button = new() { Text = name, Width = 138, Height = 48, FlatStyle = FlatStyle.Flat, ForeColor = Color.White, BackColor = Color.FromArgb(51, 61, 75), Margin = new Padding(0, 0, 0, 8) };
            button.FlatAppearance.BorderSize = 0;
            button.Click += (_, _) => ShowPage(name);
            buttons.Controls.Add(button);
        }
        panel.Controls.Add(buttons);
        return panel;
    }

    private Control BuildRightRail()
    {
        Panel rail = new() { Dock = DockStyle.Fill, BackColor = Color.White, Padding = new Padding(12) };
        TableLayoutPanel grid = new() { Dock = DockStyle.Fill, RowCount = 4 };
        grid.RowStyles.Add(new RowStyle(SizeType.Absolute, 46));
        grid.RowStyles.Add(new RowStyle(SizeType.Absolute, 222));
        grid.RowStyles.Add(new RowStyle(SizeType.Absolute, 40));
        grid.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        grid.Controls.Add(SectionTitle("수동 명령"), 0, 0);
        for (int i = 0; i < 6; i++) commandRows.Controls.Add(CreateCommandRow());
        grid.Controls.Add(commandRows, 0, 1);
        FlowLayoutPanel tools = new() { Dock = DockStyle.Fill, WrapContents = false };
        Button save = SmallButton("저장"); save.Click += (_, _) => SaveLog();
        Button clear = SmallButton("지우기"); clear.Click += (_, _) => logBox.Clear();
        tools.Controls.Add(autoScroll); tools.Controls.Add(save); tools.Controls.Add(clear);
        grid.Controls.Add(tools, 0, 2); grid.Controls.Add(logBox, 0, 3);
        rail.Controls.Add(grid);
        return rail;
    }

    private Control CreateCommandRow()
    {
        FlowLayoutPanel row = new() { Width = 470, Height = 32, WrapContents = false, Margin = new Padding(0, 1, 0, 1) };
        TextBox input = new() { Width = 380, Text = "$00%" };
        Button send = new() { Text = "SEND", Width = 78, Height = 27, BackColor = Color.FromArgb(35, 137, 230), ForeColor = Color.White, FlatStyle = FlatStyle.Flat };
        send.Click += (_, _) => SendManual(input.Text);
        input.KeyDown += (_, e) => { if (e.KeyCode == Keys.Enter) { SendManual(input.Text); e.SuppressKeyPress = true; } };
        row.Controls.Add(input); row.Controls.Add(send);
        return row;
    }

    private void CreatePages()
    {
        pages["모터 제어"] = BuildMotorPage();
        pages["릴 / IMU"] = BuildReelPage();
        pages["입력 / 전원"] = BuildInputPage();
        pages["LED"] = BuildLedPage();
        pages["LM JIG"] = BuildLmJigPage();
        pages["업데이트"] = BuildFirmwarePage();
        foreach (Panel page in pages.Values) { page.Visible = false; pageHost.Controls.Add(page); }
    }

    private Panel BuildMotorPage()
    {
        Panel page = Page();
        FlowLayoutPanel content = Column();
        content.Controls.Add(SectionTitle("메인 모터"));
        NumericUpDown torque = Number(0, 255, 70);
        TrackBar torqueSlider = DutySlider(70, 210);
        LinkDutyControls(torque, torqueSlider);
        FlowLayoutPanel torqueRow = new() { Width = 720, Height = 48, WrapContents = false };
        Button torqueOutput = MotorButton("출력", Color.FromArgb(27, 126, 96), 82);
        Button torqueStop = MotorButton("정지", Color.Firebrick, 82);
        torqueOutput.Click += (_, _) => TrySend(DeviceProtocol.Torque((int)torque.Value, 0));
        torqueStop.Click += (_, _) => { torque.Value = 0; TrySend(DeviceProtocol.TorqueOff()); };
        torqueRow.Controls.Add(new Label { Text = "메인 모터 Duty", Width = 110, Height = 36, TextAlign = ContentAlignment.MiddleLeft });
        torque.Width = 65; torqueRow.Controls.Add(torque); torqueRow.Controls.Add(SliderBorder(torqueSlider));
        content.Controls.Add(torqueRow);
        FlowLayoutPanel torqueActions = MotorActionRow();
        torqueActions.Controls.Add(torqueOutput); torqueActions.Controls.Add(torqueStop);
        content.Controls.Add(torqueActions);

        content.Controls.Add(SectionTitle("BLDC 모터"));
        RadioButton cw = new() { Text = "CW", Checked = true, AutoSize = true, Margin = new Padding(3, 7, 3, 0) };
        RadioButton ccw = new() { Text = "CCW", AutoSize = true, Margin = new Padding(3, 7, 8, 0) };
        NumericUpDown bldc = Number(0, 255, 20);
        TrackBar bldcSlider = DutySlider(20, 210);
        NumericUpDown bldcTime = Number(0, 9999, 3000);
        LinkDutyControls(bldc, bldcSlider);
        Button bldcOutput = MotorButton("출력", Color.FromArgb(27, 126, 96), 76);
        Button bldcStop = MotorButton("정지", Color.Firebrick, 76);
        bldcOutput.Click += (_, _) => TrySend(DeviceProtocol.Bldc(cw.Checked, (int)bldc.Value, (int)bldcTime.Value));
        bldcStop.Click += (_, _) => { bldc.Value = 0; TrySend(DeviceProtocol.BldcOff()); };
        FlowLayoutPanel bldcDirectionRow = FieldRow("방향", cw);
        bldcDirectionRow.Controls.Add(ccw);
        content.Controls.Add(bldcDirectionRow);
        FlowLayoutPanel bldcDutyRow = new() { Width = 560, Height = 48, WrapContents = false };
        bldcDutyRow.Controls.Add(new Label { Text = "Duty", Width = 125, Height = 36, TextAlign = ContentAlignment.MiddleLeft });
        bldc.Width = 65; bldcDutyRow.Controls.Add(bldc); bldcDutyRow.Controls.Add(SliderBorder(bldcSlider));
        content.Controls.Add(bldcDutyRow);
        bldcTime.Width = 80;
        content.Controls.Add(FieldRow("시간(ms)", bldcTime));

        FlowLayoutPanel bldcActions = MotorActionRow();
        bldcActions.Controls.Add(bldcOutput); bldcActions.Controls.Add(bldcStop);
        content.Controls.Add(bldcActions);

        Button simultaneousOutput = MotorButton("동시 출력", Color.FromArgb(35, 105, 190), 120);
        simultaneousOutput.Margin = new Padding(0, 2, 0, 2);
        simultaneousOutput.Click += (_, _) => {
            if (!RequireConnection()) return;
            TrySend(DeviceProtocol.Torque((int)torque.Value, 0));
            TrySend(DeviceProtocol.Bldc(cw.Checked, (int)bldc.Value, (int)bldcTime.Value));
        };
        FlowLayoutPanel simultaneousActions = MotorActionRow();
        simultaneousActions.Margin = new Padding(0, 0, 0, 6);
        simultaneousActions.Controls.Add(simultaneousOutput);
        content.Controls.Add(simultaneousActions);
        content.Controls.Add(Note("정지 버튼은 해당 Duty를 0으로 바꾸고 0 출력 명령을 보냅니다. Duty 범위는 0~255입니다."));
        page.Controls.Add(content); return page;
    }

    private Panel BuildReelPage()
    {
        Panel page = Page(); FlowLayoutPanel content = Column();
        content.Controls.Add(SectionTitle("릴 / IMU 연결 상태"));
        FlowLayoutPanel connections = new() { Width = 600, Height = 64 };
        connections.Controls.Add(reelConnectionState); connections.Controls.Add(imuConnectionState);
        content.Controls.Add(connections);
        content.Controls.Add(SectionTitle("릴 상태"));
        statusLabels["배터리"] = ValueLabel("-");
        statusLabels["릴 엔코더"] = ValueLabel("-");
        content.Controls.Add(FieldRow("배터리", statusLabels["배터리"]));
        content.Controls.Add(FieldRow("릴 엔코더", statusLabels["릴 엔코더"]));
        content.Controls.Add(SectionTitle("릴 버튼 상태"));
        FlowLayoutPanel indicators = new() { Width = 600, Height = 64 };
        indicators.Controls.Add(leftButtonState); indicators.Controls.Add(rightButtonState); indicators.Controls.Add(setupButtonState); indicators.Controls.Add(bootButtonState);
        content.Controls.Add(indicators);
        content.Controls.Add(SectionTitle("릴 출력"));
        content.Controls.Add(ToggleRow("진동 모터", DeviceProtocol.ReelVibration));
        content.Controls.Add(ToggleRow("왼쪽 버튼 LED", DeviceProtocol.ReelLeftLed));
        content.Controls.Add(ToggleRow("오른쪽 버튼 LED", DeviceProtocol.ReelRightLed));
        content.Controls.Add(SectionTitle("IMU 센서"));
        imuZeroButton.Enabled = false;
        imuZeroButton.Click += (_, _) => {
            if (!imuDataEnabled) return;
            if (!imuOrientation.SetCurrentAsZero()) {
                MessageBox.Show("IMU 자세값이 수신된 뒤 0점 보정을 실행하세요.", "IMU 0점 보정");
            }
        };
        FlowLayoutPanel imuToggle = ToggleRow("IMU 데이터 출력", DeviceProtocol.Imu, enabled => {
            imuDataEnabled = enabled;
            imuZeroButton.Enabled = enabled;
        });
        imuToggle.Width = 665;
        imuToggle.Controls.Add(imuZeroButton);
        content.Controls.Add(imuToggle);
        FlowLayoutPanel imuArea = new() { Width = 665, Height = 155, WrapContents = false };
        TableLayoutPanel imu = new() { Width = 315, Height = 110, ColumnCount = 4, RowCount = 3 };
        string[] keys = { "Roll", "Pitch", "Yaw", "GX", "GY", "GZ" };
        for (int i = 0; i < keys.Length; i++) {
            Label name = Caption(keys[i]); Label value = ValueLabel("-");
            statusLabels["IMU_" + keys[i]] = value;
            imu.Controls.Add(name, (i % 2) * 2, i / 2); imu.Controls.Add(value, (i % 2) * 2 + 1, i / 2);
        }
        imuArea.Controls.Add(imu);
        imuArea.Controls.Add(imuOrientation);
        content.Controls.Add(imuArea);
        page.Controls.Add(content); return page;
    }

    private Panel BuildInputPage()
    {
        Panel page = Page(); FlowLayoutPanel content = Column();
        content.Controls.Add(SectionTitle("본체 입력 감시"));
        content.Controls.Add(ToggleRow("보드 입력 수신", on => DeviceProtocol.InputMonitor("부트", on)));
        content.Controls.Add(ToggleRow("엔코더 / FG 수신", on => DeviceProtocol.InputMonitor("메인", on)));
        content.Controls.Add(ToggleRow("전원 / USB 수신", on => DeviceProtocol.InputMonitor("릴", on)));

        string[] inputKeys = { "부트", "EncA", "EncB", "FG", "USB", "전원" };
        string[] inputCaptions = { "BOOT 스위치", "엔코더 A", "엔코더 B", "BLDC FG", "USB 5V", "전원 스위치" };
        for (int i = 0; i < inputKeys.Length; i++) {
            statusLabels[inputKeys[i]] = ValueLabel("-");
            content.Controls.Add(FieldRow(inputCaptions[i], statusLabels[inputKeys[i]]));
        }

        content.Controls.Add(SectionTitle("본체 엔코더 회전"));
        FlowLayoutPanel encoderMonitor = ToggleRow("회전값 수신", on => DeviceProtocol.InputMonitor("메인엔코더", on), enabled => {
            encoderMonitorEnabled = enabled;
            if (!enabled) SetStatus("본체 엔코더", "-", Color.LightGray);
        });
        content.Controls.Add(encoderMonitor);
        statusLabels["본체 엔코더"] = ValueLabel("-");
        FlowLayoutPanel encoderValue = FieldRow("방향 / 회전 카운트", statusLabels["본체 엔코더"]);
        Button clearEncoder = SmallButton("표시 초기화", 100);
        clearEncoder.Click += (_, _) => SetStatus("본체 엔코더", "-", Color.LightGray);
        encoderValue.Controls.Add(clearEncoder);
        content.Controls.Add(encoderValue);
        page.Controls.Add(content); return page;
    }

    private Panel BuildLedPage()
    {
        Panel page = Page(); FlowLayoutPanel content = Column(); content.Controls.Add(SectionTitle("본체 LED 제어"));
        ComboBox position = Choice(new[] { "꺼짐 (0)", "중앙 (1)", "왼쪽 (2) - 비활성", "오른쪽 (3) - 비활성" }, 1);
        ComboBox action = Choice(new[] { "꺼짐 (0)", "켜짐 (1)", "점멸 (2)", "디밍 (3)" }, 1);
        ComboBox color = Choice(new[] {
            "00 - White", "01 - Red", "02 - Lime", "03 - Blue", "04 - Yellow", "05 - Cyan", "06 - Magenta", "07 - Maroon",
            "08 - Olive", "09 - Green", "10 - Purple", "11 - Teal", "12 - Navy", "13 - Silver", "14 - Gray", "15 - Black"
        }, 1);
        NumericUpDown time = Number(10, 9999, 2000), brightness = Number(10, 100, 100);
        content.Controls.Add(FieldRow("위치", position));
        content.Controls.Add(FieldRow("표시 액션", action));
        content.Controls.Add(FieldRow("색상", color));
        content.Controls.Add(FieldRow("표시 시간(ms)", time));
        Button apply = MotorButton("적용", Color.FromArgb(27, 126, 96), 110);
        apply.Margin = new Padding(125, 10, 0, 10);
        position.SelectedIndexChanged += (_, _) => apply.Enabled = position.SelectedIndex <= 1;
        apply.Click += (_, _) => TrySend(DeviceProtocol.LedControl(position.SelectedIndex, action.SelectedIndex, color.SelectedIndex, (int)time.Value));
        content.Controls.Add(apply);
        content.Controls.Add(Note("위치 2(왼쪽)와 3(오른쪽)은 현재 본체에서 비활성이라 명령을 전송하지 않습니다."));
        content.Controls.Add(FieldRow("밝기(%)", brightness));
        FlowLayoutPanel brightActions = new() { Width = 520, Height = 45, Margin = new Padding(125, 2, 0, 0) };
        Button read = MotorButton("읽기", Color.FromArgb(35, 105, 190), 90); Button write = MotorButton("쓰기", Color.FromArgb(27, 126, 96), 90);
        read.Click += (_, _) => TrySend(DeviceProtocol.LedBrightnessRead()); write.Click += (_, _) => TrySend(DeviceProtocol.LedBrightnessWrite((int)brightness.Value));
        brightActions.Controls.Add(read); brightActions.Controls.Add(write); content.Controls.Add(brightActions);
        statusLabels["Brightness"] = ValueLabel("-"); content.Controls.Add(FieldRow("수신 밝기", statusLabels["Brightness"]));
        page.Controls.Add(content); return page;
    }
    private Panel BuildLmJigPage()
    {
        Panel page = Page(); FlowLayoutPanel content = Column();
        content.Controls.Add(SectionTitle("LM JIG"));
        content.Controls.Add(Note("MAIN 보드가 LM JIG 구성으로 판별된 경우에만 $07 명령이 동작합니다. 위치 이동은 센서 감지 또는 3초 안전 타임아웃에서 정지합니다."));

        content.Controls.Add(SectionTitle("LM 모터 직접 제어"));
        NumericUpDown duty = Number(0, 255, 13), time = Number(0, 9999, 0);
        RadioButton cw = new() { Text = "CW / 왼쪽", Checked = true, AutoSize = true };
        RadioButton ccw = new() { Text = "CCW / 오른쪽", AutoSize = true };
        content.Controls.Add(FieldRow("Duty", duty));
        content.Controls.Add(FieldRow("동작 시간(ms)", time));
        FlowLayoutPanel direction = FieldRow("방향", cw); direction.Controls.Add(ccw); content.Controls.Add(direction);
        FlowLayoutPanel motorActions = new() { Width = 560, Height = 44, Margin = new Padding(125, 8, 0, 0) };
        Button startMotor = MotorButton("모터 실행", Color.FromArgb(27, 126, 96), 110), stopMotor = MotorButton("모터 정지", Color.Firebrick, 110);
        startMotor.Click += (_, _) => TrySend(DeviceProtocol.LmMotor(cw.Checked, (int)duty.Value, (int)time.Value));
        stopMotor.Click += (_, _) => TrySend(DeviceProtocol.LmMotorOff());
        motorActions.Controls.Add(startMotor); motorActions.Controls.Add(stopMotor); content.Controls.Add(motorActions);

        content.Controls.Add(SectionTitle("LM 위치 제어"));
        ComboBox fishLevel = Choice(new[] { "어종 레벨 1", "어종 레벨 2", "어종 레벨 3" }, 0);
        ComboBox powerLevel = Choice(new[] { "출력 a (1)", "출력 b (2)", "출력 c (3)" }, 0);
        NumericUpDown returnDuty = Number(0, 255, 35);
        content.Controls.Add(FieldRow("어종 레벨", fishLevel));
        content.Controls.Add(FieldRow("출력 레벨", powerLevel));
        content.Controls.Add(FieldRow("복귀 Duty", returnDuty));
        FlowLayoutPanel positionActions = new() { Width = 650, Height = 78, Margin = new Padding(0, 8, 0, 0), WrapContents = true };
        Button allOff = MotorButton("전체 정지", Color.Firebrick, 100), home = MotorButton("홈", Color.FromArgb(35, 105, 190), 90), left = MotorButton("왼쪽 이동", Color.FromArgb(35, 105, 190), 100);
        Button right = MotorButton("오른쪽 이동", Color.FromArgb(35, 105, 190), 110), returnButton = MotorButton("홈 복귀", Color.FromArgb(35, 105, 190), 100);
        allOff.Click += (_, _) => TrySend(DeviceProtocol.LmAllOff());
        home.Click += (_, _) => TrySend(DeviceProtocol.LmHome());
        left.Click += (_, _) => TrySend(DeviceProtocol.LmMove(true, fishLevel.SelectedIndex + 1, powerLevel.SelectedIndex + 1));
        right.Click += (_, _) => TrySend(DeviceProtocol.LmMove(false, fishLevel.SelectedIndex + 1, powerLevel.SelectedIndex + 1));
        returnButton.Click += (_, _) => TrySend(DeviceProtocol.LmReturn((int)returnDuty.Value));
        positionActions.Controls.Add(allOff); positionActions.Controls.Add(home); positionActions.Controls.Add(left);
        positionActions.Controls.Add(right); positionActions.Controls.Add(returnButton); content.Controls.Add(positionActions);
        content.Controls.Add(Note("명령: $071dddtttt% / $072dddtttt% / $070% / $0750~$0754 계열"));
        page.Controls.Add(content); return page;
    }
    private Panel BuildFirmwarePage()
    {
        Panel page = Page(); FlowLayoutPanel content = Column(); content.Controls.Add(SectionTitle("펌웨어 업데이트"));
        content.Controls.Add(Note("MAIN과 ROD 유선 업데이트는 bootloader / partitions / boot_app0 / application 네 파일을 내장 esptool로 기록합니다. ROD 무선은 application을 전송합니다."));
        mainUpdateButton.Click += async (_, _) => await UpdateMainAsync();
        rodWirelessButton.Click += async (_, _) => await UpdateRodWirelessAsync();
        rodWiredButton.Click += async (_, _) => await UpdateRodWiredAsync();
        rodRegisterButton.Click += (_, _) => ToggleRodRegistration();
        content.Controls.Add(mainUpdateButton); content.Controls.Add(rodWirelessButton); content.Controls.Add(rodWiredButton);
        FlowLayoutPanel registration = new() { Width = 620, Height = 48, WrapContents = false };
        registration.Controls.Add(rodRegisterButton); registration.Controls.Add(registrationState); content.Controls.Add(registration);
        content.Controls.Add(firmwareInfo); content.Controls.Add(firmwareProgress);
        page.Controls.Add(content); return page;
    }

    private FlowLayoutPanel ToggleRow(string caption, Func<bool, string> command, Action<bool>? changed = null)
    {
        FlowLayoutPanel row = new() { Width = 560, Height = 40, WrapContents = false };
        Label name = new() { Text = caption, Width = 180, TextAlign = ContentAlignment.MiddleLeft, Height = 30 };
        RadioButton on = ToggleButton("ON");
        RadioButton off = ToggleButton("OFF"); off.Checked = true;
        bool ready = false;
        on.CheckedChanged += (_, _) => {
            SetToggleButtonState(on, on.Checked, Color.FromArgb(27, 126, 96));
            if (ready && on.Checked && TrySend(command(true))) changed?.Invoke(true);
        };
        off.CheckedChanged += (_, _) => {
            SetToggleButtonState(off, off.Checked, Color.Firebrick);
            if (ready && off.Checked && TrySend(command(false))) changed?.Invoke(false);
        };
        SetToggleButtonState(on, false, Color.FromArgb(27, 126, 96));
        SetToggleButtonState(off, true, Color.Firebrick);
        row.Controls.Add(name); row.Controls.Add(on); row.Controls.Add(off); ready = true;
        return row;
    }

    private void RefreshPorts()
    {
        string previous = portBox.SelectedItem?.ToString() ?? string.Empty;
        portBox.Items.Clear(); portBox.Items.AddRange(SerialConnection.FindPorts());
        if (portBox.Items.Contains(previous)) portBox.SelectedItem = previous;
        else if (portBox.Items.Contains("COM4")) portBox.SelectedItem = "COM4";
        else if (portBox.Items.Count > 0) portBox.SelectedIndex = 0;
        AppendLog("SYSTEM", portBox.Items.Count + "개 시리얼 포트를 찾았습니다.");
    }

    private async Task ToggleConnectionAsync()
    {
        if (serial.IsOpen) {
            DisconnectMain();
            AppendLog("SYSTEM", "연결을 종료했습니다."); return;
        }
        if (portBox.SelectedItem == null) { MessageBox.Show("연결할 시리얼 포트를 선택하세요."); return; }
        connectButton.Enabled = false;
        try {
            decoder.Reset();
            serial.Open(portBox.SelectedItem.ToString()!);
            connectButton.Text = "끊음";
            connectionState.Text = serial.PortName + " 초기화 중"; connectionState.ForeColor = Color.DarkOrange;
            AppendLog("SYSTEM", serial.PortName + " 연결됨, 초기 명령 대기");
            bool initialized = await InitializeConnectedMainAsync();
            if (!initialized) {
                if (serial.IsOpen) DisconnectMain();
                AppendLog("WARNING", "연결 초기화를 완료하지 못했습니다.");
                return;
            }
            if (serial.IsOpen) {
                connectionState.Text = serial.PortName + " 연결됨"; connectionState.ForeColor = Color.SeaGreen;
                pollTimer.Start();
                AppendLog("SYSTEM", "초기 명령 송신 완료, 2초 polling 시작");
            }
        }
        catch (Exception ex) {
            if (serial.IsOpen) DisconnectMain();
            MessageBox.Show("시리얼 포트를 열거나 초기화할 수 없습니다.\r\n" + ex.Message, "연결 실패");
        }
        finally { connectButton.Enabled = !firmwareBusy; }
    }
    private void OnBytesReceived(byte[] data)
    {
        IList<string> frames = decoder.Push(data);
        foreach (string frame in frames) {
            downloader.AcceptFrame(frame);
            wirelessUpdater.AcceptFrame(frame);
            BeginInvoke(new Action(() => ProcessFrame(frame)));
        }
    }

    private void ProcessFrame(string frame)
    {
        if (!frame.StartsWith("$OR", StringComparison.Ordinal)) AppendLog("RECV", frame);
        if (frame == "$1800%" && !powerOffRequestObserved) {
            powerOffRequestObserved = true;
            AppendLog("WARNING", "MAIN이 AP 종료를 요청 중입니다. TestModule은 자동 종료 ACK를 보내지 않습니다.");
        }
        if (frame.StartsWith("$10", StringComparison.Ordinal)) {
            string[] values = frame.Substring(3).TrimEnd('%').Split(',');
            foreach (string value in values) {
                if (value.StartsWith("Vm", StringComparison.OrdinalIgnoreCase)) mainVersion.Text = value;
                else if (value.StartsWith("Vr", StringComparison.OrdinalIgnoreCase)) rodVersion.Text = value;
                else if (value.Length > 0) imuVersion.Text = value;
            }
        } else if (frame.StartsWith("$09", StringComparison.Ordinal)) {
            string[] values = frame.Substring(3).TrimStart(',').TrimEnd('%').Split(',');
            string[] keys = { "Roll", "Pitch", "Yaw", "GX", "GY", "GZ" };
            for (int i = 0; i < Math.Min(keys.Length, values.Length); i++) SetStatus("IMU_" + keys[i], values[i], null);
            if (imuDataEnabled && values.Length >= 3 &&
                float.TryParse(values[0], NumberStyles.Float, CultureInfo.InvariantCulture, out float roll) &&
                float.TryParse(values[1], NumberStyles.Float, CultureInfo.InvariantCulture, out float pitch) &&
                float.TryParse(values[2], NumberStyles.Float, CultureInfo.InvariantCulture, out float yaw)) {
                imuOrientation.SetAngles(roll, pitch, yaw);
            }
        } else if (frame.StartsWith("$13", StringComparison.Ordinal) && frame.Length >= 5) {
            string code = frame.Substring(3, 2);
            if (code == "10") SetButtonIndicator(leftButtonState, true, Color.Blue); else if (code == "11") SetButtonIndicator(leftButtonState, false, Color.Gray);
            else if (code == "20") SetButtonIndicator(rightButtonState, true, Color.Red); else if (code == "21") SetButtonIndicator(rightButtonState, false, Color.Gray);
            else if (code == "80") SetButtonIndicator(setupButtonState, true, Color.Red); else if (code == "81") SetButtonIndicator(setupButtonState, false, Color.LightYellow);
            else if (code == "90") SetButtonIndicator(bootButtonState, true, Color.Red); else if (code == "91") SetButtonIndicator(bootButtonState, false, Color.LightYellow);
        } else if (frame.StartsWith("$0608", StringComparison.Ordinal)) {
            SetStatus("Brightness", frame.Substring(5).TrimEnd('%'), null);
        } else if (frame.StartsWith("$27", StringComparison.Ordinal)) {
            ParseInput(frame);
        } else if (frame.StartsWith("$20", StringComparison.Ordinal) && frame.Length >= 5) {
            rodConnected = frame.Substring(3, 2) == "01";
            SetConnectionIndicator(reelConnectionState, rodConnected, "릴");
        } else if (frame.StartsWith("$21", StringComparison.Ordinal) && frame.Length >= 5) {
            imuConnected = frame.Substring(3, 2) == "01";
            SetConnectionIndicator(imuConnectionState, imuConnected, "IMU");
        } else if (frame.StartsWith("$12", StringComparison.Ordinal)) {
            SetStatus("배터리", frame.Substring(3).TrimEnd('%'), Color.LightGreen);
        } else if (frame.StartsWith("$14", StringComparison.Ordinal)) {
            SetStatus("릴 엔코더", frame.Substring(3).TrimEnd('%'), Color.LightGreen);
            imuOrientation.AdvanceHandleCounterClockwise();
        } else if (frame.StartsWith("$30", StringComparison.Ordinal) && frame.Length >= 5) {
            string code = frame.Substring(3, 2);
            if (code == "03") FinishRegistration("등록 취소", Color.LightGray);
            else if (code == "04") { registrationWaiting = true; rodRegisterButton.Text = "ROD 등록 취소"; registrationState.Text = "등록 진행 중"; registrationState.BackColor = Color.LightPink; }
            else if (code == "05") FinishRegistration("등록 완료", Color.LightBlue);
        }
    }

    private void ParseInput(string frame)
    {
        string body = frame.Substring(3).TrimEnd('%');
        int comma = body.IndexOf(','); if (comma < 0) return;
        string code = body.Substring(0, comma); string value = body.Substring(comma + 1);
        if (code == "03-03") {
            if (!encoderMonitorEnabled) return;
            string[] encoder = value.Split(',');
            if (encoder.Length < 2) return;
            string direction = encoder[0] == "1" ? "CW" : encoder[0] == "2" ? "CCW" : "방향 오류";
            SetStatus("본체 엔코더", direction + " / " + encoder[1], Color.LightGreen);
            return;
        }
        string? key = code switch { "01-02" => "부트", "03-01" => "EncA", "03-02" => "EncB", "03-05" => "FG", "05-02" => "USB", "05-01" => "전원", _ => null };
        if (key != null) SetStatus(key, value, value == "0" ? Color.LightGray : Color.LightGreen);
    }

    private void SendManual(string message)
    {
        message = message.Trim();
        if (!message.StartsWith('$') || !message.EndsWith('%') || message.Any(c => c > 0x7f)) { MessageBox.Show("명령은 ASCII $로 시작하고 %로 끝나야 합니다."); return; }
        TrySend(message);
    }

    private bool TrySend(string message, bool showError = true)
    {
        if (!serial.IsOpen) { if (showError) MessageBox.Show("먼저 시리얼 포트를 연결하세요."); return false; }
        try { serial.WriteText(message); AppendLog("SEND", message); return true; }
        catch (Exception ex) { if (showError) MessageBox.Show("명령 송신 실패\r\n" + ex.Message); return false; }
    }

    private bool RequireConnection() { if (serial.IsOpen) return true; MessageBox.Show("먼저 시리얼 포트를 연결하세요."); return false; }

    internal static string[][] InitialCommandGroups()
    {
        return new string[][] {
            new string[] { "$00%", "$290101%", "$1101%" },
            new string[] { "$00%", "$080%" },
            new string[] { "$00%", "$0112%", "$0110%", "$0115%", "$0117%", "$1501%" }
        };
    }
    private async Task<bool> InitializeConnectedMainAsync()
    {
        rodConnected = false; imuConnected = false; powerOffRequestObserved = false; registrationWaiting = false;
        rodRegisterButton.Text = "ROD 등록"; registrationState.Text = "등록 대기 안 함";
        registrationState.BackColor = Color.FromArgb(239, 241, 244);
        SetPeripheralConnectionPending();

        string[][] commandGroups = InitialCommandGroups();

        for (int group = 0; group < commandGroups.Length; group++) {
            await Task.Delay(500);
            if (!serial.IsOpen) return false;
            foreach (string command in commandGroups[group]) {
                if (!TrySend(command, false)) {
                    AppendLog("WARNING", "초기 명령 송신 중단: " + command);
                    return false;
                }
                await Task.Delay(60);
                if (!serial.IsOpen) return false;
            }
        }
        int connectionWait = 0;
        while (serial.IsOpen && connectionWait < 20 && (!rodConnected || !imuConnected)) {
            await Task.Delay(100);
            connectionWait++;
        }
        if (!serial.IsOpen) return false;
        if (!TrySend("$10%", false)) {
            AppendLog("WARNING", "초기 버전 조회 송신 실패");
            return false;
        }
        AppendLog("SYSTEM", "초기 연결: TM/Game 상태와 IMU OFF, 장치 상태 확인 후 버전을 조회했습니다.");
        return true;
    }
    private void DisconnectMain()
    {
        pollTimer.Stop(); serial.Close(); connectButton.Text = "연결"; connectionState.Text = "연결 안 됨"; connectionState.ForeColor = Color.DimGray;
        rodConnected = false; imuConnected = false; imuDataEnabled = false; imuZeroButton.Enabled = false; imuOrientation.ClearZero(); powerOffRequestObserved = false; registrationWaiting = false; rodRegisterButton.Text = "ROD 등록";
        mainVersion.Text = "Vm -"; rodVersion.Text = "Vr -"; imuVersion.Text = "IMU -";
        SetConnectionIndicator(reelConnectionState, false, "릴"); SetConnectionIndicator(imuConnectionState, false, "IMU");
    }

    private FirmwarePackage? SelectFirmwareFolder(byte board)
    {
        using FolderBrowserDialog dialog = new() { Description = (board == (byte)'M' ? "MAIN" : "ROD") + " 펌웨어 출력 폴더 선택", UseDescriptionForTitle = true, ShowNewFolderButton = false };
        if (dialog.ShowDialog(this) != DialogResult.OK) return null;
        try {
            FirmwarePackage package = FirmwarePackage.LoadFromFolder(dialog.SelectedPath, board);
            firmwareInfo.Text = "대상: " + package.TargetName + "\r\n버전: " + package.Version + "\r\n파일: " + package.FileName +
                "\r\n크기: " + package.Data.Length.ToString("N0") + " bytes\r\nSHA-256: " + package.Sha256;
            firmwareProgress.Value = 0; AppendLog("SYSTEM", "펌웨어 폴더 검증 완료: " + dialog.SelectedPath);
            return package;
        } catch (Exception ex) { firmwareInfo.Text = "폴더 오류: " + ex.Message; MessageBox.Show(ex.Message, "올바르지 않은 펌웨어 폴더"); return null; }
    }

    private async Task UpdateMainAsync()
    {
        if (firmwareBusy) return;
        string preferredPort = "COM4";
        if (serial.IsOpen) {
            if (!mainVersion.Text.StartsWith("Vm", StringComparison.OrdinalIgnoreCase)) { MessageBox.Show("현재 연결이 MAIN인지 확인할 수 없습니다."); return; }
            if (MessageBox.Show("MAIN 업데이트 전에 현재 시리얼 연결을 끊습니다. 계속할까요?", "MAIN 연결 해제 확인", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) != DialogResult.OK) return;
            DisconnectMain(); AppendLog("SYSTEM", "MAIN 업데이트 전 시리얼 연결을 종료했습니다.");
        }
        using PortSelectionDialog portDialog = new("MAIN 통신포트 선택", "USB로 연결한 MAIN의 통신포트를 선택하세요.", string.Empty, preferredPort);
        if (portDialog.ShowDialog(this) != DialogResult.OK) return;
        string port = portDialog.SelectedPort;
        if (portBox.Items.Contains(port)) portBox.SelectedItem = port;
        WiredFirmwarePackage? package = SelectWiredFirmwareFolder((byte)'M'); if (package == null) return;
        SetFirmwareBusy(true, "MAIN 업데이트 준비");
        try {
            Progress<int> progress = new(value => firmwareProgress.Value = value);
            Progress<string> toolOutput = new(value => AppendLog("FLASH", value));
            connectionState.Text = port + " MAIN 업데이트"; connectionState.ForeColor = Color.DarkOrange;
            AppendLog("SYSTEM", "MAIN USB 업데이트 시작: " + package.Version + ", " + port);
            await wiredUpdater.UpdateAsync(port, package, progress, toolOutput, CancellationToken.None);
            AppendLog("SYSTEM", "MAIN 네 영역 기록 및 검증 완료");
            connectionState.Text = "연결 안 됨"; connectionState.ForeColor = Color.DimGray;
            MessageBox.Show("MAIN 펌웨어 업데이트를 완료했습니다. 보드는 자동 재부팅됩니다. 부팅 후 연결 버튼으로 버전을 확인하세요.");
        } catch (Exception ex) { DisconnectMain(); AppendLog("ERROR", ex.Message); MessageBox.Show(ex.Message, "MAIN 업데이트 실패"); }
        finally { SetFirmwareBusy(false, firmwareInfo.Text); }
    }

    private WiredFirmwarePackage? SelectWiredFirmwareFolder(byte boardId)
    {
        string target = boardId == (byte)'M' ? "MAIN" : "ROD";
        using FolderBrowserDialog dialog = new() { Description = target + " 펌웨어 네 파일이 있는 출력 폴더 선택", UseDescriptionForTitle = true, ShowNewFolderButton = false };
        if (dialog.ShowDialog(this) != DialogResult.OK) return null;
        try {
            WiredFirmwarePackage package = WiredFirmwarePackage.Load(dialog.SelectedPath, boardId);
            firmwareInfo.Text = "대상: " + package.TargetName + " (USB 직접 업데이트)\r\n버전: " + package.Version +
                "\r\n파일: bootloader / partitions / boot_app0 / application" +
                "\r\n크기: " + package.TotalSize.ToString("N0") + " bytes\r\nAPP SHA-256: " + package.ApplicationSha256;
            firmwareProgress.Value = 0;
            AppendLog("SYSTEM", target + " 유선 업데이트 폴더 검증 완료: " + dialog.SelectedPath);
            return package;
        }
        catch (Exception ex) {
            firmwareInfo.Text = "폴더 오류: " + ex.Message;
            MessageBox.Show(ex.Message, "올바르지 않은 " + target + " 펌웨어 폴더");
            return null;
        }
    }

    private async Task UpdateRodWirelessAsync()
    {
        if (firmwareBusy) return;
        if (!serial.IsOpen || !mainVersion.Text.StartsWith("Vm", StringComparison.OrdinalIgnoreCase)) { MessageBox.Show("ROD 무선 업데이트는 MAIN 시리얼 연결이 필요합니다."); return; }
        if (!rodConnected) { MessageBox.Show("MAIN에 ROD가 연결된 상태인지 확인하세요."); return; }
        FirmwarePackage? package = SelectFirmwareFolder((byte)'R'); if (package == null) return;
        SetFirmwareBusy(true, "ROD 무선 업데이트 중"); pollTimer.Stop();
        try {
            Progress<int> progress = new(value => firmwareProgress.Value = value);
            await wirelessUpdater.DownloadAsync(package, progress, CancellationToken.None);
            AppendLog("SYSTEM", "ROD 무선 펌웨어 전송 완료");
            rodConnected = false;
            SetConnectionIndicator(reelConnectionState, false, "릴");
            ResumeMainPolling();
            int reconnectWait = 0;
            while (serial.IsOpen && !rodConnected && reconnectWait < 120) {
                await Task.Delay(100);
                reconnectWait++;
            }
            if (serial.IsOpen) TrySend("$10%", false);
            string reconnectResult = rodConnected ? "ROD 재연결도 확인했습니다." : "ROD 재연결은 아직 확인되지 않았습니다.";
            AppendLog("SYSTEM", reconnectResult);
            MessageBox.Show("ROD 무선 업데이트를 완료했습니다.\r\n" + reconnectResult, "ROD 무선 업데이트 완료", MessageBoxButtons.OK, MessageBoxIcon.Information);
        } catch (Exception ex) { AppendLog("ERROR", ex.Message); MessageBox.Show(ex.Message, "ROD 무선 업데이트 실패"); }
        finally { ResumeMainPolling(); SetFirmwareBusy(false, firmwareInfo.Text); }
    }

    private void ResumeMainPolling()
    {
        if (!serial.IsOpen) return;
        TrySend("$00%", false);
        pollTimer.Start();
    }

    private async Task UpdateRodWiredAsync()
    {
        if (firmwareBusy) return;
        using PortSelectionDialog portDialog = new("ROD 유선 통신포트 선택", "USB로 연결한 ROD의 통신포트를 선택하세요.", serial.IsOpen ? serial.PortName : string.Empty, string.Empty);
        if (portDialog.ShowDialog(this) != DialogResult.OK) return;
        string rodPort = portDialog.SelectedPort;
        WiredFirmwarePackage? package = SelectWiredFirmwareFolder((byte)'R'); if (package == null) return;
        if (MessageBox.Show(rodPort + "에 USB로 연결한 ROD의 네 펌웨어 영역을 업데이트합니다.", "ROD 유선 업데이트", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) != DialogResult.OK) return;
        SetFirmwareBusy(true, "ROD 유선 업데이트 중");
        try {
            Progress<int> progress = new(value => firmwareProgress.Value = value);
            Progress<string> toolOutput = new(value => AppendLog("FLASH", value));
            AppendLog("SYSTEM", "ROD USB 업데이트 시작: " + package.Version + ", " + rodPort);
            await wiredUpdater.UpdateAsync(rodPort, package, progress, toolOutput, CancellationToken.None);
            AppendLog("SYSTEM", "ROD 네 영역 기록 및 검증 완료: " + rodPort);
            MessageBox.Show("ROD 유선 펌웨어 업데이트를 완료했습니다. ROD는 자동 재부팅됩니다.");
        } catch (Exception ex) { AppendLog("ERROR", ex.Message); MessageBox.Show(ex.Message, "ROD 유선 업데이트 실패"); }
        finally { SetFirmwareBusy(false, firmwareInfo.Text); }
    }

    private void ToggleRodRegistration()
    {
        if (!RequireConnection()) return;
        if (!mainVersion.Text.StartsWith("Vm", StringComparison.OrdinalIgnoreCase)) { MessageBox.Show("ROD 등록은 MAIN 연결에서만 수행할 수 있습니다."); return; }
        if (!registrationWaiting) {
            if (!TrySend("$3001%")) return;
            registrationWaiting = true; rodRegisterButton.Text = "ROD 등록 취소";
            registrationState.Text = "ROD 왼쪽 버튼을 길게 누르세요"; registrationState.BackColor = Color.LightYellow;
        } else {
            if (!TrySend("$3002%")) return;
            FinishRegistration("취소 요청", Color.LightGray);
        }
    }

    private void FinishRegistration(string text, Color color)
    {
        registrationWaiting = false; rodRegisterButton.Text = "ROD 등록";
        registrationState.Text = text; registrationState.BackColor = color;
    }

    private void SetFirmwareBusy(bool busy, string text)
    {
        firmwareBusy = busy; mainUpdateButton.Enabled = !busy; rodWirelessButton.Enabled = !busy; rodWiredButton.Enabled = !busy; rodRegisterButton.Enabled = !busy;
        connectButton.Enabled = !busy; if (busy) AppendLog("SYSTEM", text);
    }
    private void AppendLog(string direction, string text)
    {
        if (InvokeRequired) { BeginInvoke(new Action(() => AppendLog(direction, text))); return; }
        logBox.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " " + direction.PadRight(6) + " " + text + Environment.NewLine);
        if (autoScroll.Checked) { logBox.SelectionStart = logBox.TextLength; logBox.ScrollToCaret(); }
    }

    private void SaveLog()
    {
        using SaveFileDialog dialog = new() { Filter = "Text log (*.txt)|*.txt", FileName = "DFLOG[" + DateTime.Now.ToString("yyyyMMddHHmm") + "].txt" };
        if (dialog.ShowDialog(this) == DialogResult.OK) File.WriteAllText(dialog.FileName, logBox.Text, Encoding.UTF8);
    }

    private void ShowPage(string name)
    {
        foreach (Panel page in pages.Values) page.Visible = false;
        Panel selected = pages[name]; selected.Visible = true; selected.BringToFront();
    }

    private void SetStatus(string key, string text, Color? color)
    {
        if (!statusLabels.TryGetValue(key, out Label? label)) return;
        label.Text = text; if (color.HasValue) label.BackColor = color.Value;
    }

    private void SetPeripheralConnectionPending()
    {
        reelConnectionState.Text = "릴 연결 시도"; reelConnectionState.BackColor = Color.LightYellow; reelConnectionState.ForeColor = Color.Black;
        imuConnectionState.Text = "IMU 연결 시도"; imuConnectionState.BackColor = Color.LightYellow; imuConnectionState.ForeColor = Color.Black;
    }

    private static void SetConnectionIndicator(Label label, bool connected, string name)
    {
        label.Text = name + (connected ? " 연결됨" : " 연결 안 됨");
        label.BackColor = connected ? Color.LightGreen : Color.FromArgb(230, 230, 230);
        label.ForeColor = Color.FromArgb(35, 35, 35);
    }
    private static void SetButtonIndicator(Label label, bool pressed, Color color)
    {
        label.BackColor = pressed ? color : Color.FromArgb(230, 230, 230); label.ForeColor = pressed ? Color.White : Color.Black;
    }

    private static Panel Page() => new() { Dock = DockStyle.Fill, BackColor = Color.White };
    private static FlowLayoutPanel Column() => new() { Dock = DockStyle.Fill, AutoScroll = true, FlowDirection = FlowDirection.TopDown, WrapContents = false, Padding = new Padding(22, 22, 22, 70) };
    private static Label SectionTitle(string text) => new() { Text = text, Font = new Font("맑은 고딕", 13F, FontStyle.Bold), AutoSize = true, Margin = new Padding(0, 6, 0, 14) };
    private static Label Caption(string text) => new() { Text = text, AutoSize = true, TextAlign = ContentAlignment.MiddleLeft, Margin = new Padding(7, 7, 7, 0) };
    private static Label ValueLabel(string text) => new() { Text = text, AutoSize = true, BackColor = Color.FromArgb(239, 241, 244), Padding = new Padding(8, 5, 8, 5), Margin = new Padding(4, 2, 4, 0) };
    private static Label Indicator(string text) => new() { Text = text, Width = 100, Height = 42, TextAlign = ContentAlignment.MiddleCenter, BackColor = Color.FromArgb(230, 230, 230), ForeColor = Color.FromArgb(35, 35, 35), Margin = new Padding(4) };
    private static Label Note(string text) => new() { Text = text, AutoSize = true, MaximumSize = new Size(620, 0), ForeColor = Color.DimGray, Margin = new Padding(0, 12, 0, 12) };
    private static Control Spacer(int width) => new Panel { Width = width, Height = 1 };
    private static ComboBox Choice(string[] items, int selectedIndex)
    {
        ComboBox box = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 180 }; box.Items.AddRange(items); box.SelectedIndex = selectedIndex; return box;
    }
    private static NumericUpDown Number(int min, int max, int value) => new() { Minimum = min, Maximum = max, Value = value, Width = 110 };
    private static TrackBar DutySlider(int value, int width) => new() { Minimum = 0, Maximum = 255, Value = value, Width = width, Height = 38, TickFrequency = 25, SmallChange = 1, LargeChange = 10, AutoSize = false };
    private static Panel SliderBorder(TrackBar slider)
    {
        Panel border = new() { Width = slider.Width + 4, Height = 40, BorderStyle = BorderStyle.FixedSingle, Padding = new Padding(1), Margin = new Padding(3, 0, 6, 0) };
        slider.Dock = DockStyle.Fill; slider.Margin = Padding.Empty;
        border.Controls.Add(slider);
        return border;
    }
    private static void LinkDutyControls(NumericUpDown number, TrackBar slider)
    {
        number.ValueChanged += (_, _) => { if (slider.Value != (int)number.Value) slider.Value = (int)number.Value; };
        slider.ValueChanged += (_, _) => { if (number.Value != slider.Value) number.Value = slider.Value; };
    }
    private static FlowLayoutPanel MotorActionRow() => new() { Width = 560, Height = 40, WrapContents = false, Margin = new Padding(0, 0, 0, 2) };
    private static Button MotorButton(string text, Color color, int width) => new() { Text = text, Width = width, Height = 34, BackColor = color, ForeColor = Color.White, FlatStyle = FlatStyle.Flat, Margin = new Padding(0, 2, 8, 2) };
    private static Button SmallButton(string text, int width = 72) => MotorButton(text, Color.FromArgb(35, 105, 190), width);
    private static Button UpdateButton(string text)
    {
        Button button = MotorButton(text, Color.FromArgb(35, 105, 190), 290); button.Height = 40; button.Margin = new Padding(0, 3, 0, 3); return button;
    }
    private static RadioButton ToggleButton(string text) => new() { Text = text, Width = 70, Height = 34, AutoSize = false, Appearance = Appearance.Button, FlatStyle = FlatStyle.Flat, TextAlign = ContentAlignment.MiddleCenter, Margin = new Padding(0, 2, 8, 2) };
    private static void SetToggleButtonState(RadioButton button, bool selected, Color activeColor)
    {
        button.BackColor = selected ? activeColor : Color.FromArgb(230, 230, 230);
        button.ForeColor = selected ? Color.White : Color.FromArgb(35, 35, 35);
    }
    private static FlowLayoutPanel FieldRow(string caption, Control control)
    {
        FlowLayoutPanel row = new() { Width = 560, Height = 38, WrapContents = false };
        row.Controls.Add(new Label { Text = caption, Width = 125, Height = 28, TextAlign = ContentAlignment.MiddleLeft });
        row.Controls.Add(control); return row;
    }
}























