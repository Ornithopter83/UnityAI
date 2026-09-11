using System.Diagnostics;
using System.Globalization;
using System.Reflection;
using System.Text.RegularExpressions;

namespace DFTestModule;

internal sealed class EspToolWiredUpdater
{
    private const string ResourceName = "DFTestModule.esptool_V4.5.1.exe";
    private static readonly Regex WritingPattern = new(@"Writing at 0x([0-9a-fA-F]+).*\(([0-9]+)\s*%\)", RegexOptions.Compiled);

    public async Task UpdateAsync(string portName, WiredFirmwarePackage package, IProgress<int> progress, IProgress<string> output, CancellationToken cancellationToken)
    {
        string tempFolder = Path.Combine(Path.GetTempPath(), "DFTestModule-esptool-" + Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempFolder);
        string toolPath = Path.Combine(tempFolder, "esptool_V4.5.1.exe");
        try {
            ExtractTool(toolPath);
            using Process process = new();
            process.StartInfo = CreateStartInfo(toolPath, portName, package);
            process.OutputDataReceived += (_, e) => HandleLine(e.Data, package, progress, output);
            process.ErrorDataReceived += (_, e) => HandleLine(e.Data, package, progress, output);
            progress.Report(1);
            if (!process.Start()) throw new InvalidOperationException("내장 esptool을 시작하지 못했습니다.");
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            await process.WaitForExitAsync(cancellationToken);
            process.WaitForExit();
            if (process.ExitCode != 0)
                throw new InvalidOperationException("esptool 기록 실패 (종료 코드 " + process.ExitCode + "). 보드를 USB로 연결하고 필요하면 BOOT 버튼을 누른 상태에서 다시 시도하세요.");
            progress.Report(100);
        }
        finally { try { Directory.Delete(tempFolder, true); } catch { } }
    }

    internal static ProcessStartInfo CreateStartInfo(string toolPath, string portName, WiredFirmwarePackage package)
    {
        ProcessStartInfo start = new(toolPath) {
            UseShellExecute = false, CreateNoWindow = true,
            RedirectStandardOutput = true, RedirectStandardError = true,
            WorkingDirectory = package.FolderPath
        };
        string[] args = {
            "--chip", "esp32s3", "--port", portName, "--baud", "921600",
            "--before", "default_reset", "--after", "hard_reset", "write_flash", "-z",
            "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "4MB",
            "0x0", package.BootloaderPath, "0x8000", package.PartitionsPath,
            "0xe000", package.BootApp0Path, "0x10000", package.ApplicationPath
        };
        foreach (string arg in args) start.ArgumentList.Add(arg);
        return start;
    }

    private static void ExtractTool(string destination)
    {
        using Stream source = Assembly.GetExecutingAssembly().GetManifestResourceStream(ResourceName)
            ?? throw new InvalidOperationException("배포 EXE에 esptool 리소스가 없습니다.");
        using FileStream target = new(destination, FileMode.CreateNew, FileAccess.Write, FileShare.None);
        source.CopyTo(target);
    }

    private static void HandleLine(string? line, WiredFirmwarePackage package, IProgress<int> progress, IProgress<string> output)
    {
        if (string.IsNullOrWhiteSpace(line)) return;
        output.Report(line.Trim());
        Match match = WritingPattern.Match(line);
        if (!match.Success) return;
        long address = long.Parse(match.Groups[1].Value, NumberStyles.HexNumber, CultureInfo.InvariantCulture);
        int percent = Math.Clamp(int.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture), 0, 100);
        long bootSize = new FileInfo(package.BootloaderPath).Length;
        long partitionSize = new FileInfo(package.PartitionsPath).Length;
        long bootAppSize = new FileInfo(package.BootApp0Path).Length;
        long appSize = new FileInfo(package.ApplicationPath).Length;
        long completed;
        long segmentSize;
        if (address >= 0x10000) { completed = bootSize + partitionSize + bootAppSize; segmentSize = appSize; }
        else if (address >= 0xe000) { completed = bootSize + partitionSize; segmentSize = bootAppSize; }
        else if (address >= 0x8000) { completed = bootSize; segmentSize = partitionSize; }
        else { completed = 0; segmentSize = bootSize; }
        int totalPercent = (int)((completed + segmentSize * percent / 100L) * 100L / package.TotalSize);
        progress.Report(Math.Clamp(totalPercent, 1, 99));
    }
}
