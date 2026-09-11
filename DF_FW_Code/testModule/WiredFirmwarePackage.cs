using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;

namespace DFTestModule;

internal sealed class WiredFirmwarePackage
{
    public byte BoardId { get; private set; }
    public string TargetName => BoardId == (byte)'M' ? "Main" : "Rod";
    public string FolderPath { get; private set; } = string.Empty;
    public string Version { get; private set; } = string.Empty;
    public string ApplicationPath { get; private set; } = string.Empty;
    public string BootloaderPath { get; private set; } = string.Empty;
    public string PartitionsPath { get; private set; } = string.Empty;
    public string BootApp0Path { get; private set; } = string.Empty;
    public long TotalSize { get; private set; }
    public string ApplicationSha256 { get; private set; } = string.Empty;

    public static WiredFirmwarePackage Load(string folderPath, byte boardId)
    {
        if (boardId != (byte)'M' && boardId != (byte)'R') throw new ArgumentOutOfRangeException(nameof(boardId));
        if (!Directory.Exists(folderPath)) throw new DirectoryNotFoundException("선택한 펌웨어 폴더가 없습니다.");
        string boardName = boardId == (byte)'M' ? "Main" : "Rod";
        string application = RequiredFile(folderPath, "DF_" + boardName + ".ino.bin");
        string bootloader = RequiredFile(folderPath, "DF_" + boardName + ".ino.bootloader.bin");
        string partitions = RequiredFile(folderPath, "DF_" + boardName + ".ino.partitions.bin");
        string bootApp0 = RequiredFile(folderPath, "boot_app0.bin");
        byte[] applicationData = File.ReadAllBytes(application);
        string pattern = boardId == (byte)'M' ? @"Vm\d+\.\d+\.\d+\.\d+" : @"Vr\d+\.\d+\.\d+\.\d+";
        Match version = Regex.Match(Encoding.Latin1.GetString(applicationData), pattern, RegexOptions.IgnoreCase);
        if (!version.Success) throw new InvalidDataException(Path.GetFileName(application) + " 내부에서 " + boardName + " 버전을 찾지 못했습니다.");
        return new WiredFirmwarePackage {
            BoardId = boardId, FolderPath = Path.GetFullPath(folderPath), Version = version.Value,
            ApplicationPath = application, BootloaderPath = bootloader,
            PartitionsPath = partitions, BootApp0Path = bootApp0,
            TotalSize = new FileInfo(application).Length + new FileInfo(bootloader).Length + new FileInfo(partitions).Length + new FileInfo(bootApp0).Length,
            ApplicationSha256 = Convert.ToHexString(SHA256.HashData(applicationData))
        };
    }

    private static string RequiredFile(string folderPath, string name)
    {
        string[] matches = Directory.GetFiles(folderPath, name, SearchOption.TopDirectoryOnly);
        if (matches.Length != 1 || new FileInfo(matches[0]).Length <= 0)
            throw new InvalidDataException("선택 폴더에 " + name + " 파일이 정확히 1개 있어야 합니다.");
        return Path.GetFullPath(matches[0]);
    }
}
