using System.IO.Ports;
using System.Text;

namespace DFTestModule;

internal sealed class SerialConnection : IDisposable
{
    private readonly object writeLock = new();
    private SerialPort? port;
    public event Action<byte[]>? BytesReceived;
    public bool IsOpen => port?.IsOpen == true;
    public string PortName => port?.PortName ?? string.Empty;

    public static string[] FindPorts()
    {
        string[] ports = SerialPort.GetPortNames();
        Array.Sort(ports, (left, right) => {
            int a, b;
            if (int.TryParse(left.Replace("COM", ""), out a) && int.TryParse(right.Replace("COM", ""), out b)) return a.CompareTo(b);
            return string.Compare(left, right, StringComparison.OrdinalIgnoreCase);
        });
        return ports;
    }

    public void Open(string portName)
    {
        Close();
        SerialPort next = new(portName, 115200, Parity.None, 8, StopBits.One) {
            Handshake = Handshake.None, RtsEnable = true, DtrEnable = false, ReadTimeout = 200, WriteTimeout = 500
        };
        try {
            next.Open();
            next.DiscardInBuffer();
            next.DiscardOutBuffer();
            port = next;
            next.DataReceived += OnDataReceived;
        }
        catch { next.DataReceived -= OnDataReceived; next.Dispose(); throw; }
    }

    public void WriteText(string message) { WriteBytes(Encoding.ASCII.GetBytes(message)); }
    public void WriteBytes(byte[] data)
    {
        const int WriteChunkSize = 1024;
        SerialPort current = port ?? throw new InvalidOperationException("시리얼 포트가 연결되지 않았습니다.");
        lock (writeLock) {
            for (int offset = 0; offset < data.Length; offset += WriteChunkSize) {
                int count = Math.Min(WriteChunkSize, data.Length - offset);
                current.Write(data, offset, count);
            }
        }
    }

    public void Close()
    {
        SerialPort? current = port; port = null;
        if (current == null) return;
        current.DataReceived -= OnDataReceived;
        try { if (current.IsOpen) current.Close(); } catch { }
        current.Dispose();
    }

    private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        SerialPort current = (SerialPort)sender;
        try {
            int count = current.BytesToRead; if (count <= 0) return;
            byte[] data = new byte[count]; int read = current.Read(data, 0, data.Length);
            if (read != data.Length) Array.Resize(ref data, read);
            BytesReceived?.Invoke(data);
        } catch { }
    }

    public void Dispose() { Close(); }
}




