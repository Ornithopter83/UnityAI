namespace DFTestModule;

internal sealed class PortSelectionDialog : Form
{
    private readonly ComboBox ports = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 150 };
    private readonly string excludedPort;
    private readonly string preferredPort;
    public string SelectedPort => ports.SelectedItem?.ToString() ?? string.Empty;

    public PortSelectionDialog(string title, string prompt, string excluded, string preferred)
    {
        excludedPort = excluded;
        preferredPort = preferred;
        Text = title;
        StartPosition = FormStartPosition.CenterParent;
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MinimizeBox = false; MaximizeBox = false; ShowInTaskbar = false;
        ClientSize = new Size(390, 145);
        FlowLayoutPanel content = new() { Dock = DockStyle.Fill, Padding = new Padding(18), FlowDirection = FlowDirection.TopDown, WrapContents = false };
        content.Controls.Add(new Label { Text = prompt, AutoSize = true });
        FlowLayoutPanel row = new() { Width = 350, Height = 38 };
        Button refresh = new() { Text = "재검색", Width = 80, Height = 29 };
        refresh.Click += (_, _) => LoadPorts();
        row.Controls.Add(ports); row.Controls.Add(refresh); content.Controls.Add(row);
        FlowLayoutPanel actions = new() { Width = 350, Height = 38, FlowDirection = FlowDirection.RightToLeft };
        Button cancel = new() { Text = "취소", DialogResult = DialogResult.Cancel, Width = 80 };
        Button ok = new() { Text = "선택", DialogResult = DialogResult.OK, Width = 80 };
        actions.Controls.Add(cancel); actions.Controls.Add(ok); content.Controls.Add(actions);
        Controls.Add(content); AcceptButton = ok; CancelButton = cancel;
        LoadPorts();
    }

    protected override void OnFormClosing(FormClosingEventArgs e)
    {
        if (DialogResult == DialogResult.OK && ports.SelectedItem == null) { MessageBox.Show(this, "통신포트를 선택하세요."); e.Cancel = true; }
        base.OnFormClosing(e);
    }

    private void LoadPorts()
    {
        string previous = SelectedPort;
        ports.Items.Clear();
        foreach (string port in SerialConnection.FindPorts()) if (!string.Equals(port, excludedPort, StringComparison.OrdinalIgnoreCase)) ports.Items.Add(port);
        if (ports.Items.Contains(previous)) ports.SelectedItem = previous;
        else if (ports.Items.Contains(preferredPort)) ports.SelectedItem = preferredPort;
        else if (ports.Items.Count > 0) ports.SelectedIndex = 0;
    }
}
