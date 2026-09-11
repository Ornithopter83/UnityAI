using System.Globalization;

namespace DFTestModule;

internal static class DeviceProtocol
{
    public static string Torque(int duty, int timeMs)
    {
        Range(duty, 0, 255, "메인 모터 Duty"); Range(timeMs, 0, 9999, "동작 시간");
        return timeMs == 0 ? string.Format(CultureInfo.InvariantCulture, "$02{0:000}%", duty)
            : string.Format(CultureInfo.InvariantCulture, "$02{0:000}{1:0000}%", duty, timeMs);
    }

    public static string Bldc(bool clockwise, int duty, int timeMs)
    {
        Range(duty, 0, 255, "BLDC Duty"); Range(timeMs, 0, 9999, "동작 시간");
        return string.Format(CultureInfo.InvariantCulture, "$04{0}{1:000}{2:0000}%", clockwise ? 1 : 2, duty, timeMs);
    }

    public static string TorqueOff() { return "$02000%"; }
    public static string BldcOff() { return "$0400000000%"; }
    public static string LmMotor(bool clockwise, int duty, int timeMs)
    {
        Range(duty, 0, 255, "LM Duty"); Range(timeMs, 0, 9999, "LM 동작 시간");
        return string.Format(CultureInfo.InvariantCulture, "$07{0}{1:000}{2:0000}%", clockwise ? 1 : 2, duty, timeMs);
    }

    public static string LmMotorOff() { return "$070%"; }
    public static string LmAllOff() { return "$0750%"; }
    public static string LmHome() { return "$0751%"; }
    public static string LmMove(bool left, int fishLevel, int powerLevel)
    {
        Range(fishLevel, 1, 3, "어종 레벨"); Range(powerLevel, 1, 3, "출력 레벨");
        return string.Format(CultureInfo.InvariantCulture, "$075{0}{1}{2}%", left ? 2 : 3, fishLevel, powerLevel);
    }
    public static string LmReturn(int duty)
    {
        Range(duty, 0, 255, "복귀 Duty");
        return string.Format(CultureInfo.InvariantCulture, "$0754{0:000}%", duty);
    }
    public static string ReelVibration(bool on) { return on ? "$231%" : "$230%"; }
    public static string ReelLeftLed(bool on) { return on ? "$2411%" : "$2410%"; }
    public static string ReelRightLed(bool on) { return on ? "$2421%" : "$2420%"; }
    public static string Imu(bool on) { return on ? "$081%" : "$080%"; }
    public static string LedControl(int position, int action, int color, int timeMs)
    {
        Range(position, 0, 3, "LED 위치"); Range(action, 0, 3, "LED 표시 액션");
        Range(color, 0, 15, "LED 색상"); Range(timeMs, 10, 9999, "LED 시간");
        if (action == 0) return string.Format(CultureInfo.InvariantCulture, "$06{0}0%", position);
        if (action == 1) return string.Format(CultureInfo.InvariantCulture, "$06{0}1{1:00}%", position, color);
        return string.Format(CultureInfo.InvariantCulture, "$06{0}{1}{2:00}{3:0000}%", position, action, color, timeMs);
    }

    public static string LedBrightnessRead() { return "$0608%"; }
    public static string LedBrightnessWrite(int value) { Range(value, 10, 100, "LED 밝기"); return string.Format(CultureInfo.InvariantCulture, "$0609{0:000}%", value); }

    public static string InputMonitor(string input, bool on)
    {
        string code = input switch {
            "부트" => "01-00", "릴" => "05-00", "메인" => "03-00", "메인엔코더" => "03-03",
            _ => throw new ArgumentException("알 수 없는 입력 대상입니다.", nameof(input))
        };
        return "$27" + code + "," + (on ? "1" : "0") + "%";
    }

    public static byte[] DownloadFrame(byte boardId, int sequence, byte[]? data)
    {
        if (sequence < 0 || sequence > 999) throw new ArgumentOutOfRangeException(nameof(sequence));
        data ??= Array.Empty<byte>();
        if (data.Length > ushort.MaxValue) throw new ArgumentOutOfRangeException(nameof(data));
        ushort sum = 0;
        for (int i = 0; i < data.Length; i++) sum += data[i];
        byte[] frame = new byte[data.Length + 12];
        frame[0] = (byte)'$'; frame[1] = (byte)'D'; frame[2] = (byte)'N'; frame[3] = boardId;
        frame[4] = (byte)('0' + (sequence / 100) % 10); frame[5] = (byte)('0' + (sequence / 10) % 10); frame[6] = (byte)('0' + sequence % 10);
        frame[7] = (byte)(data.Length >> 8); frame[8] = (byte)data.Length;
        Buffer.BlockCopy(data, 0, frame, 9, data.Length);
        frame[9 + data.Length] = (byte)(sum >> 8); frame[10 + data.Length] = (byte)sum; frame[11 + data.Length] = (byte)'%';
        return frame;
    }

    private static void Range(int value, int min, int max, string name)
    {
        if (value < min || value > max) throw new ArgumentOutOfRangeException(name);
    }
}





