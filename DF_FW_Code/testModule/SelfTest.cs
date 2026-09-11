using System.IO.Compression;
using System.Text;

namespace DFTestModule;

internal static class SelfTest
{
    public static int Run()
    {
        try {
            Equal("$020230050%", DeviceProtocol.Torque(23, 50), "메인 모터 명령");
            Equal("$0410230050%", DeviceProtocol.Bldc(true, 23, 50), "BLDC 명령");
            Equal("$2411%", DeviceProtocol.ReelLeftLed(true), "릴 왼쪽 LED");

            Equal("$0710130050%", DeviceProtocol.LmMotor(true, 13, 50), "LM CW 모터");
            Equal("$0720130050%", DeviceProtocol.LmMotor(false, 13, 50), "LM CCW 모터");
            Equal("$075212%", DeviceProtocol.LmMove(true, 1, 2), "LM 왼쪽 이동");
            Equal("$075333%", DeviceProtocol.LmMove(false, 3, 3), "LM 오른쪽 이동");
            Equal("$0754035%", DeviceProtocol.LmReturn(35), "LM 홈 복귀");
            Equal("$0600%", DeviceProtocol.LedControl(0, 0, 1, 10), "본체 LED 끄기");
            Equal("$061100%", DeviceProtocol.LedControl(1, 1, 0, 10), "중앙 LED 흰색 켜기");
            Equal("$061115%", DeviceProtocol.LedControl(1, 1, 15, 10), "중앙 LED 검정 켜기");
            Equal("$0609010%", DeviceProtocol.LedBrightnessWrite(10), "LED 최소 밝기");
            Equal("$0609100%", DeviceProtocol.LedBrightnessWrite(100), "LED 최대 밝기");
            byte[] frame = DeviceProtocol.DownloadFrame((byte)'M', 7, new byte[] { 1, 2, 3 });
            if (frame.Length != 15 || frame[3] != (byte)'M' || frame[6] != (byte)'7' || frame[13] != 6 || frame[14] != (byte)'%')
                throw new InvalidOperationException("다운로드 프레임 오류");

            FrameDecoder decoder = new();
            IList<string> first = decoder.Push(Encoding.ASCII.GetBytes("noise$10Vm1."));
            IList<string> second = decoder.Push(Encoding.ASCII.GetBytes("0%$00%"));
            if (first.Count != 0 || second.Count != 2 || second[0] != "$10Vm1.0%" || second[1] != "$00%")
                throw new InvalidOperationException("수신 프레임 분할 오류");

            byte[] raw = Enumerable.Repeat((byte)0x5a, 2048).ToArray();
            byte[] compressed = new byte[raw.Length];
            int compressedLength = LzCodec.EncodeLZ(raw, compressed, raw.Length, raw.Length);
            if (compressedLength >= raw.Length) throw new InvalidOperationException("LZ 압축 오류");
            byte[] restored = new byte[raw.Length];
            byte[] compressedExact = compressed.Take(compressedLength).ToArray();
            int restoredLength = LzCodec.DecodeLZ(compressedExact, restored);
            if (restoredLength != raw.Length || !raw.SequenceEqual(restored)) throw new InvalidOperationException("LZ 복원 오류");

            string temp = Path.Combine(Path.GetTempPath(), "DFTestModule-selftest-" + Guid.NewGuid().ToString("N") + ".zip");
            try {
                using (ZipArchive archive = ZipFile.Open(temp, ZipArchiveMode.Create)) {
                    ZipArchiveEntry entry = archive.CreateEntry("DF_MAIN_test.bin");
                    using Stream stream = entry.Open(); stream.Write(raw, 0, raw.Length);
                }
                FirmwarePackage package = FirmwarePackage.Load(temp);
                if (package.BoardId != (byte)'M' || package.Data.Length != raw.Length) throw new InvalidOperationException("펌웨어 ZIP 검증 오류");
            } finally { try { File.Delete(temp); } catch { } }
            byte[] otaFrame = RodWirelessUpdater.BuildFrame(2, 0x01020304U, 7, new byte[] { 0xaa, 0x55 });
            if (otaFrame.Length != 21 || otaFrame[0] != (byte)'D' || otaFrame[3] != (byte)'O' || otaFrame[5] != 2 ||
                otaFrame[6] != 1 || otaFrame[7] != 2 || otaFrame[8] != 3 || otaFrame[9] != 4 || otaFrame[14] != 2)
                throw new InvalidOperationException("ROD OTA 프레임 오류");

            string folder = Path.Combine(Path.GetTempPath(), "DFTestModule-folder-" + Guid.NewGuid().ToString("N"));
            try {
                Directory.CreateDirectory(folder);
                byte[] rodImage = Encoding.ASCII.GetBytes("binary-prefix-Vr1.2.3.4-binary-suffix");
                File.WriteAllBytes(Path.Combine(folder, "DF_Rod.ino.bin"), rodImage);
                FirmwarePackage folderPackage = FirmwarePackage.LoadFromFolder(folder, (byte)'R');
                if (folderPackage.BoardId != (byte)'R' || folderPackage.Version != "Vr1.2.3.4" || folderPackage.Data.Length != rodImage.Length)
                    throw new InvalidOperationException("펌웨어 폴더 검증 오류");
            } finally { try { Directory.Delete(folder, true); } catch { } }

            string mainFolder = Path.Combine(Path.GetTempPath(), "DFTestModule-main-" + Guid.NewGuid().ToString("N"));
            try {
                Directory.CreateDirectory(mainFolder);
                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Main.ino.bin"), Encoding.ASCII.GetBytes("binary-Vm1.2.3.4-image"));
                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Main.ino.bootloader.bin"), new byte[] { 1 });
                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Main.ino.partitions.bin"), new byte[] { 2 });
                File.WriteAllBytes(Path.Combine(mainFolder, "boot_app0.bin"), new byte[] { 3 });
                WiredFirmwarePackage mainPackage = WiredFirmwarePackage.Load(mainFolder, (byte)'M');
                if (mainPackage.Version != "Vm1.2.3.4" || mainPackage.TotalSize <= 3)
                    throw new InvalidOperationException("MAIN 유선 패키지 검증 오류");
                System.Diagnostics.ProcessStartInfo start = EspToolWiredUpdater.CreateStartInfo("tool.exe", "COM4", mainPackage);
                string arguments = string.Join(" ", start.ArgumentList);
                if (!arguments.Contains("--baud 921600", StringComparison.Ordinal) ||
                    !arguments.Contains("0x0", StringComparison.Ordinal) ||
                    !arguments.Contains("0x8000", StringComparison.Ordinal) ||
                    !arguments.Contains("0xe000", StringComparison.Ordinal) ||
                    !arguments.Contains("0x10000", StringComparison.Ordinal))
                    throw new InvalidOperationException("esptool 인수 구성 오류");
                using Stream? resource = typeof(EspToolWiredUpdater).Assembly.GetManifestResourceStream("DFTestModule.esptool_V4.5.1.exe");
                if (resource == null || resource.Length != 6638312)
                    throw new InvalidOperationException("내장 esptool 리소스 오류");

                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Rod.ino.bin"), Encoding.ASCII.GetBytes("binary-Vr1.2.3.4-image"));
                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Rod.ino.bootloader.bin"), new byte[] { 4 });
                File.WriteAllBytes(Path.Combine(mainFolder, "DF_Rod.ino.partitions.bin"), new byte[] { 5 });
                WiredFirmwarePackage rodPackage = WiredFirmwarePackage.Load(mainFolder, (byte)'R');
                System.Diagnostics.ProcessStartInfo rodStart = EspToolWiredUpdater.CreateStartInfo("tool.exe", "COM7", rodPackage);
                if (rodPackage.Version != "Vr1.2.3.4" || !rodStart.ArgumentList.Contains(rodPackage.ApplicationPath))
                    throw new InvalidOperationException("ROD 유선 esptool 패키지 검증 오류");
            } finally { try { Directory.Delete(mainFolder, true); } catch { } }

            string[][] initialCommands = MainForm.InitialCommandGroups();
            if (initialCommands.Length != 3 || initialCommands[0][1] != "$290101%" ||
                initialCommands[0][2] != "$1101%" || initialCommands[1][1] != "$080%" ||
                initialCommands[2][3] != "$1501%")
                throw new InvalidOperationException("초기 연결 명령 단계 오류");
            using (MainForm form = new MainForm()) {
                form.CreateControl();
                if (form.Controls.Count == 0 || form.ClientSize.Height != 980) throw new InvalidOperationException("UI 구성 오류");
            }
            return 0;
        }
        catch (Exception ex) {
            Console.Error.WriteLine(ex.ToString());
            return 1;
        }
    }

    private static void Equal(string expected, string actual, string name)
    {
        if (!string.Equals(expected, actual, StringComparison.Ordinal))
            throw new InvalidOperationException(name + ": expected " + expected + ", actual " + actual);
    }
}













