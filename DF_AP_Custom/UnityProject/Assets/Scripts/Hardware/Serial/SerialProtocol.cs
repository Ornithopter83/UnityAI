using System;
using System.Globalization;

namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// Evidence-backed codec for the original main-board ASCII protocol.
    /// It intentionally contains no COM-port or gameplay side effects.
    /// </summary>
    public static class SerialProtocol
    {
        public const int BaudRate = 115200;
        public const int ReadBufferSize = 4096;
        public const int ReadTimeoutMilliseconds = 50;
        public const int WriteTimeoutMilliseconds = 50;
        public const string DefaultMainPort = "COM4";
        public const string DefaultCardPort = "COM10";
        public const string MainPortEnvironmentVariable = "MAINPORT";
        public const string CardPortEnvironmentVariable = "CARDPORT";

        public enum ReceiveId
        {
            MainAlive = 0,
            DeviceInfo = 1,
            Torque = 2,
            Bobbin = 4,
            RodBreak = 5,
            LedControl = 6,
            ImuSetup = 8,
            Imu = 9,
            Version = 10,
            GameStatus = 11,
            Battery = 12,
            Switch = 13,
            Rotary = 14,
            Device = 15,
            Address = 16,
            ApStatus = 18,
            MotorTest = 19,
            RodConnect = 20,
            ConnectImu = 21,
            RodChange = 30
        }

        public enum SwitchSignal
        {
            Unknown = 0,
            LeftDown = 10,
            LeftUp = 11,
            RightDown = 20,
            RightUp = 21
        }

        public enum BinaryConnectionState
        {
            Disconnected = 0,
            Connected = 1
        }

        public enum RodConnectionState
        {
            Unknown = -1,
            Disconnected = 0,
            Connected = 1,
            ChangeState11 = 11,
            ChangeState12 = 12,
            MacFail = 99
        }

        public enum RodChangeState
        {
            Unknown = -1,
            Cancelled = 3,
            Waiting = 4,
            Success = 5
        }

        public readonly struct ImuSample
        {
            public ImuSample(float roll, float pitch, float yaw, float x, float y, float z)
            {
                Roll = roll;
                Pitch = pitch;
                Yaw = yaw;
                X = x;
                Y = y;
                Z = z;
            }

            public float Roll { get; }
            public float Pitch { get; }
            public float Yaw { get; }
            public float X { get; }
            public float Y { get; }
            public float Z { get; }
        }

        public readonly struct VersionInfo
        {
            public VersionInfo(string main, string rod, string imu)
            {
                Main = main;
                Rod = rod;
                Imu = imu;
            }

            public string Main { get; }
            public string Rod { get; }
            public string Imu { get; }
        }

        public static string ResolveMainPort()
        {
            return ResolvePort(MainPortEnvironmentVariable, DefaultMainPort);
        }

        public static string ResolveCardPort()
        {
            return ResolvePort(CardPortEnvironmentVariable, DefaultCardPort);
        }

        public static bool TryParseFrame(string line, out SerialFrame frame, out string error)
        {
            frame = default;
            error = string.Empty;
            if (string.IsNullOrWhiteSpace(line))
            {
                error = "Frame is empty.";
                return false;
            }

            string raw = line.TrimEnd('\r', '\n');
            if (raw.Length < 4 || raw[0] != '$' || raw[raw.Length - 1] != '%')
            {
                error = "Frame must start with '$', end with '%', and contain a two-digit id.";
                return false;
            }
            if (raw.IndexOf('$', 1) >= 0 || raw.IndexOf('%') != raw.Length - 1)
            {
                error = "Frame contains an additional delimiter.";
                return false;
            }
            if (!int.TryParse(raw.Substring(1, 2), NumberStyles.None, CultureInfo.InvariantCulture, out int id))
            {
                error = "Command id is not a two-digit decimal value.";
                return false;
            }

            frame = new SerialFrame(id, raw.Substring(3, raw.Length - 4), raw);
            return true;
        }

        public static string MainAlive()
        {
            return "$00%";
        }

        public static string Torque(int value)
        {
            return "$02" + value.ToString("000", CultureInfo.InvariantCulture) + "%";
        }

        public static string SubMotor(int onOff, int power, int time)
        {
            return "$04" + onOff.ToString(CultureInfo.InvariantCulture)
                + power.ToString("000", CultureInfo.InvariantCulture)
                + time.ToString("0000", CultureInfo.InvariantCulture) + "%";
        }

        public static string Brake(int value)
        {
            return "$05" + value.ToString("000", CultureInfo.InvariantCulture) + "%";
        }

        public static string LedWithColor(int position, int controlDetail, int color, int waitTime)
        {
            return "$06" + position.ToString(CultureInfo.InvariantCulture)
                + controlDetail.ToString(CultureInfo.InvariantCulture)
                + color.ToString("00", CultureInfo.InvariantCulture)
                + waitTime.ToString("0000", CultureInfo.InvariantCulture) + "%";
        }

        public static string LedWithoutColor(int position, int controlDetail, int waitTime)
        {
            return "$06" + position.ToString(CultureInfo.InvariantCulture)
                + controlDetail.ToString(CultureInfo.InvariantCulture)
                + waitTime.ToString("000", CultureInfo.InvariantCulture) + "%";
        }

        public static string ImuStreaming(bool enabled)
        {
            return enabled ? "$081%" : "$080%";
        }

        public static string VersionRequest()
        {
            return "$10-%";
        }

        public static string InitialBoardState()
        {
            return "$1101%";
        }

        public static string BitePattern(int rank, string biteCommand)
        {
            return "$1153" + rank.ToString(CultureInfo.InvariantCulture) + (biteCommand ?? string.Empty) + "%";
        }

        public static string BiteStop(int rank)
        {
            return "$1153" + rank.ToString(CultureInfo.InvariantCulture) + "999%";
        }

        public static string BoardClose()
        {
            return "$1199%";
        }

        public static string DeviceAllOff()
        {
            return "$1500%";
        }

        public static string DeviceStatusRequest()
        {
            return "$1501%";
        }

        public static string ApStatusAcknowledge()
        {
            return "$1800%";
        }

        public static string ReelMotor(string settingCommand)
        {
            return "$23" + (settingCommand ?? string.Empty) + "%";
        }

        public static string ButtonLed(string settingCommand)
        {
            return "$24" + (settingCommand ?? string.Empty) + "%";
        }

        public static string TensionState(int state, string optionalPayload = null)
        {
            return "$25" + state.ToString("00", CultureInfo.InvariantCulture)
                + (optionalPayload ?? string.Empty) + "%";
        }

        public static string RodChangeStart()
        {
            return "$3001%";
        }

        public static string RodChangeCancel()
        {
            return "$3002%";
        }

        public static bool TryParseSwitch(SerialFrame frame, out SwitchSignal signal)
        {
            signal = SwitchSignal.Unknown;
            if (frame.CommandId != (int)ReceiveId.Switch || !TryParseExactInt(frame.Payload, 2, out int value))
            {
                return false;
            }
            if (!Enum.IsDefined(typeof(SwitchSignal), value) || value == 0)
            {
                return false;
            }
            signal = (SwitchSignal)value;
            return true;
        }

        public static bool TryParseRotary(SerialFrame frame, out int position)
        {
            position = 0;
            return frame.CommandId == (int)ReceiveId.Rotary
                && frame.Payload.Length == 3
                && int.TryParse(frame.Payload, NumberStyles.AllowLeadingSign, CultureInfo.InvariantCulture, out position);
        }

        public static bool TryParseImu(SerialFrame frame, out ImuSample sample)
        {
            sample = default;
            if (frame.CommandId != (int)ReceiveId.Imu)
            {
                return false;
            }
            string[] parts = frame.Payload.Split(',');
            if (parts.Length != 6)
            {
                return false;
            }
            var values = new float[6];
            for (int i = 0; i < parts.Length; i++)
            {
                if (!float.TryParse(parts[i], NumberStyles.Float, CultureInfo.InvariantCulture, out values[i]))
                {
                    return false;
                }
            }
            sample = new ImuSample(values[0], values[1], values[2], values[3], values[4], values[5]);
            return true;
        }

        public static bool TryParseVersion(SerialFrame frame, out VersionInfo version)
        {
            version = default;
            if (frame.CommandId != (int)ReceiveId.Version)
            {
                return false;
            }
            string[] parts = frame.Payload.Split(',');
            if (parts.Length != 3)
            {
                return false;
            }
            version = new VersionInfo(parts[0], parts[1], parts[2]);
            return true;
        }

        public static bool TryParseBattery(SerialFrame frame, out int value)
        {
            value = 0;
            return frame.CommandId == (int)ReceiveId.Battery && TryParseExactInt(frame.Payload, 2, out value);
        }

        public static bool TryParseDeviceCode(SerialFrame frame, out int code)
        {
            code = 0;
            return frame.CommandId == (int)ReceiveId.Device && TryParseExactInt(frame.Payload, 4, out code);
        }

        public static bool TryParseApStatus(SerialFrame frame, out int status)
        {
            status = -1;
            return frame.CommandId == (int)ReceiveId.ApStatus && TryParseExactInt(frame.Payload, 2, out status);
        }

        public static bool TryParseImuConnection(SerialFrame frame, out BinaryConnectionState state)
        {
            state = BinaryConnectionState.Disconnected;
            if (frame.CommandId != (int)ReceiveId.ConnectImu || !TryParseExactInt(frame.Payload, 2, out int value)
                || (value != 0 && value != 1))
            {
                return false;
            }
            state = (BinaryConnectionState)value;
            return true;
        }

        public static bool TryParseRodConnection(SerialFrame frame, out RodConnectionState state)
        {
            state = RodConnectionState.Unknown;
            if (frame.CommandId != (int)ReceiveId.RodConnect || !TryParseExactInt(frame.Payload, 2, out int value)
                || !Enum.IsDefined(typeof(RodConnectionState), value))
            {
                return false;
            }
            state = (RodConnectionState)value;
            return true;
        }

        public static bool TryParseRodChange(SerialFrame frame, out RodChangeState state)
        {
            state = RodChangeState.Unknown;
            if (frame.CommandId != (int)ReceiveId.RodChange || !TryParseExactInt(frame.Payload, 2, out int value)
                || !Enum.IsDefined(typeof(RodChangeState), value))
            {
                return false;
            }
            state = (RodChangeState)value;
            return true;
        }

        private static string ResolvePort(string environmentVariable, string fallback)
        {
            try
            {
                string value = Environment.GetEnvironmentVariable(environmentVariable);
                return string.IsNullOrEmpty(value) ? fallback : value;
            }
            catch
            {
                return fallback;
            }
        }

        private static bool TryParseExactInt(string payload, int length, out int value)
        {
            value = 0;
            return payload != null && payload.Length == length
                && int.TryParse(payload, NumberStyles.AllowLeadingSign, CultureInfo.InvariantCulture, out value);
        }
    }
}
