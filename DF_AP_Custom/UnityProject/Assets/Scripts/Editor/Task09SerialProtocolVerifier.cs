#if UNITY_EDITOR
using System;
using FishingGame.Hardware.Serial;
using UnityEditor;
using UnityEngine;

namespace FishingGame.Editor
{
    public static class Task09SerialProtocolVerifier
    {
        [MenuItem("Fishing Game/Verify Task 09 Serial Protocol")]
        public static void Run()
        {
            VerifyOutgoingCommands();
            VerifyIncomingFrames();
            VerifySimulatorBoundary();
            Debug.Log("[FishingGame] Task 09 Serial protocol verification passed: command formats, receive parsers, and PC simulator.");
        }

        private static void VerifyOutgoingCommands()
        {
            Equal("$00%", SerialProtocol.MainAlive(), "main alive");
            Equal("$02007%", SerialProtocol.Torque(7), "torque");
            Equal("$0410850120%", SerialProtocol.SubMotor(1, 85, 120), "sub motor");
            Equal("$05217%", SerialProtocol.Brake(217), "brake");
            Equal("$0632140050%", SerialProtocol.LedWithColor(3, 2, 14, 50), "LED with color");
            Equal("$0632050%", SerialProtocol.LedWithoutColor(3, 2, 50), "LED without color");
            Equal("$081%", SerialProtocol.ImuStreaming(true), "IMU start");
            Equal("$080%", SerialProtocol.ImuStreaming(false), "IMU stop");
            Equal("$10-%", SerialProtocol.VersionRequest(), "version request");
            Equal("$1101%", SerialProtocol.InitialBoardState(), "initial board state");
            Equal("$11532987%", SerialProtocol.BitePattern(2, "987"), "bite pattern");
            Equal("$11532999%", SerialProtocol.BiteStop(2), "bite stop");
            Equal("$1199%", SerialProtocol.BoardClose(), "board close");
            Equal("$1500%", SerialProtocol.DeviceAllOff(), "device all off");
            Equal("$1501%", SerialProtocol.DeviceStatusRequest(), "device status request");
            Equal("$1800%", SerialProtocol.ApStatusAcknowledge(), "AP status ACK");
            Equal("$23ABC%", SerialProtocol.ReelMotor("ABC"), "reel motor setting");
            Equal("$24XYZ%", SerialProtocol.ButtonLed("XYZ"), "button LED setting");
            Equal("$2512%", SerialProtocol.TensionState(12), "tension state");
            Equal("$251211%", SerialProtocol.TensionState(12, "11"), "tension state payload");
            Equal("$3001%", SerialProtocol.RodChangeStart(), "rod change start");
            Equal("$3002%", SerialProtocol.RodChangeCancel(), "rod change cancel");
        }

        private static void VerifyIncomingFrames()
        {
            SerialFrame frame = Parse("$1310%\n");
            True(SerialProtocol.TryParseSwitch(frame, out SerialProtocol.SwitchSignal signal)
                && signal == SerialProtocol.SwitchSignal.LeftDown, "switch left down");

            frame = Parse("$1321%");
            True(SerialProtocol.TryParseSwitch(frame, out signal)
                && signal == SerialProtocol.SwitchSignal.RightUp, "switch right up");

            frame = Parse("$14-19%");
            True(SerialProtocol.TryParseRotary(frame, out int rotary) && rotary == -19, "signed rotary");

            frame = Parse("$091.5,-2,3,4,5,6%");
            True(SerialProtocol.TryParseImu(frame, out SerialProtocol.ImuSample imu)
                && Near(imu.Roll, 1.5f) && Near(imu.Pitch, -2f) && Near(imu.Z, 6f), "IMU sample");

            frame = Parse("$10M1,R2,I3%");
            True(SerialProtocol.TryParseVersion(frame, out SerialProtocol.VersionInfo version)
                && version.Main == "M1" && version.Rod == "R2" && version.Imu == "I3", "version tuple");

            frame = Parse("$12-1%");
            True(SerialProtocol.TryParseBattery(frame, out int battery) && battery == -1, "battery state");

            frame = Parse("$150000%");
            True(SerialProtocol.TryParseDeviceCode(frame, out int deviceCode) && deviceCode == 0, "device normal");

            frame = Parse("$1800%");
            True(SerialProtocol.TryParseApStatus(frame, out int apStatus) && apStatus == 0, "AP status");

            frame = Parse("$2001%");
            True(SerialProtocol.TryParseRodConnection(frame, out SerialProtocol.RodConnectionState rod)
                && rod == SerialProtocol.RodConnectionState.Connected, "rod connected");

            frame = Parse("$2199%");
            True(!SerialProtocol.TryParseImuConnection(frame, out _), "reject unknown IMU connection state");

            frame = Parse("$2100%");
            True(SerialProtocol.TryParseImuConnection(frame, out SerialProtocol.BinaryConnectionState imuConnection)
                && imuConnection == SerialProtocol.BinaryConnectionState.Disconnected, "IMU disconnected");

            frame = Parse("$3005%");
            True(SerialProtocol.TryParseRodChange(frame, out SerialProtocol.RodChangeState change)
                && change == SerialProtocol.RodChangeState.Success, "rod change success");

            True(!SerialProtocol.TryParseFrame("1310%", out _, out _), "reject missing STX");
            True(!SerialProtocol.TryParseFrame("$1310", out _, out _), "reject missing ETX");
            True(!SerialProtocol.TryParseFrame("$13$10%", out _, out _), "reject duplicate STX");
            True(!SerialProtocol.TryParseFrame("$AA10%", out _, out _), "reject non-decimal id");
            True(SerialProtocol.TryParseFrame("$9901%", out SerialFrame unknown, out _)
                && unknown.CommandId == 99 && unknown.Payload == "01", "preserve unknown valid frame");
        }

        private static void VerifySimulatorBoundary()
        {
            var transport = new SimulatedSerialTransport();
            using (var bus = new SerialCommandBus(transport))
            {
                int received = 0;
                int malformed = 0;
                SerialFrame last = default;
                bus.FrameReceived += frame =>
                {
                    received++;
                    last = frame;
                };
                bus.MalformedFrameReceived += (_, __) => malformed++;

                bus.Open();
                True(bus.TrySend(SerialProtocol.DeviceStatusRequest(), out string sendError), "simulator send: " + sendError);
                True(transport.SentLines.Count == 1 && transport.SentLines[0] == "$1501%", "simulator send record");

                transport.InjectReceivedLine("$1310%");
                transport.InjectReceivedLine("broken");
                True(received == 0 && malformed == 0, "receive is queued before main-thread pump");
                True(bus.PumpReceived() == 2, "pump count");
                True(received == 1 && malformed == 1 && last.CommandId == 13, "simulator receive events");

                bus.Close();
                True(!bus.TrySend("$00%", out _), "closed transport blocks send");
            }
        }

        private static SerialFrame Parse(string raw)
        {
            if (!SerialProtocol.TryParseFrame(raw, out SerialFrame frame, out string error))
            {
                throw new InvalidOperationException("Expected valid frame '" + raw + "': " + error);
            }
            return frame;
        }

        private static bool Near(float value, float expected)
        {
            return Mathf.Abs(value - expected) < 0.0001f;
        }

        private static void Equal(string expected, string actual, string label)
        {
            if (!string.Equals(expected, actual, StringComparison.Ordinal))
            {
                throw new InvalidOperationException(label + " mismatch. Expected " + expected + ", got " + actual);
            }
        }

        private static void True(bool value, string label)
        {
            if (!value)
            {
                throw new InvalidOperationException("Task 09 verification failed: " + label);
            }
        }
    }
}
#endif
