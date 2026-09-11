using System.IO.Compression;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace DFTestModule;

internal sealed class FirmwarePackage
{
    public byte BoardId { get; private set; }
    public string TargetName => BoardId == (byte)'M' ? "Main" : "Rod";
    public string Version { get; private set; } = "패키지 미표기";
    public string FileName { get; private set; } = string.Empty;
    public byte[] Data { get; private set; } = Array.Empty<byte>();
    public string Sha256 { get; private set; } = string.Empty;

    public static FirmwarePackage Load(string zipPath)
    {
        if (!string.Equals(Path.GetExtension(zipPath), ".zip", StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("ZIP 압축파일만 선택할 수 있습니다.");
        using ZipArchive archive = ZipFile.OpenRead(zipPath);
        List<ZipArchiveEntry> bins = archive.Entries.Where(e => string.Equals(Path.GetExtension(e.Name), ".bin", StringComparison.OrdinalIgnoreCase)).ToList();
        if (bins.Count != 1) throw new InvalidDataException("압축파일에는 펌웨어 BIN 파일이 정확히 1개 있어야 합니다.");
        ZipArchiveEntry binary = bins[0];
        byte boardId;
        if (binary.Name.Contains("DF_MAIN", StringComparison.OrdinalIgnoreCase)) boardId = (byte)'M';
        else if (binary.Name.Contains("DF_ROD", StringComparison.OrdinalIgnoreCase)) boardId = (byte)'R';
        else throw new InvalidDataException("BIN 파일명에 DF_MAIN 또는 DF_ROD가 있어야 합니다.");
        if (binary.Length <= 0 || binary.Length > 16 * 1024 * 1024) throw new InvalidDataException("펌웨어 파일 크기가 올바르지 않습니다.");
        byte[] data;
        using (Stream stream = binary.Open()) { using MemoryStream memory = new(); stream.CopyTo(memory); data = memory.ToArray(); }
        string hash = Convert.ToHexString(SHA256.HashData(data)), version = "패키지 미표기";
        ZipArchiveEntry? manifestEntry = archive.Entries.FirstOrDefault(e => string.Equals(e.Name, "manifest.json", StringComparison.OrdinalIgnoreCase));
        if (manifestEntry != null) {
            using Stream stream = manifestEntry.Open(); using JsonDocument manifest = JsonDocument.Parse(stream); JsonElement root = manifest.RootElement;
            if (root.TryGetProperty("target", out JsonElement target)) {
                string expected = boardId == (byte)'M' ? "main" : "rod";
                if (!string.Equals(target.GetString(), expected, StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("manifest의 target과 BIN 파일 대상이 다릅니다.");
            }
            if (root.TryGetProperty("version", out JsonElement versionValue)) version = versionValue.GetString() ?? version;
            if (root.TryGetProperty("size", out JsonElement size) && size.GetInt64() != data.LongLength) throw new InvalidDataException("manifest의 파일 크기가 실제 BIN과 다릅니다.");
            if (root.TryGetProperty("sha256", out JsonElement sha) && !string.Equals(sha.GetString(), hash, StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("manifest의 SHA-256과 실제 BIN이 다릅니다.");
        }
        return new FirmwarePackage { BoardId = boardId, Version = version, FileName = binary.Name, Data = data, Sha256 = hash };
    }

    public static FirmwarePackage LoadFromFolder(string folderPath, byte expectedBoard)
    {
        if (!Directory.Exists(folderPath)) throw new DirectoryNotFoundException("선택한 펌웨어 폴더가 없습니다.");
        string expectedName = expectedBoard == (byte)'M' ? "DF_Main.ino.bin" : expectedBoard == (byte)'R' ? "DF_Rod.ino.bin" : throw new ArgumentOutOfRangeException(nameof(expectedBoard));
        string[] matches = Directory.GetFiles(folderPath, "*.bin", SearchOption.AllDirectories).Where(path => string.Equals(Path.GetFileName(path), expectedName, StringComparison.OrdinalIgnoreCase)).ToArray();
        if (matches.Length != 1) throw new InvalidDataException("폴더에는 " + expectedName + " 파일이 정확히 1개 있어야 합니다.");
        byte[] data = File.ReadAllBytes(matches[0]);
        if (data.Length <= 0 || data.Length > 16 * 1024 * 1024) throw new InvalidDataException("펌웨어 파일 크기가 올바르지 않습니다.");
        string pattern = expectedBoard == (byte)'M' ? @"Vm\d+\.\d+\.\d+\.\d+" : @"Vr\d+\.\d+\.\d+\.\d+";
        Match version = Regex.Match(Encoding.Latin1.GetString(data), pattern, RegexOptions.IgnoreCase);
        if (!version.Success) throw new InvalidDataException("BIN 내부에서 " + (char)expectedBoard + " 대상 버전을 찾지 못했습니다.");
        return new FirmwarePackage { BoardId = expectedBoard, Version = version.Value, FileName = Path.GetFileName(matches[0]), Data = data, Sha256 = Convert.ToHexString(SHA256.HashData(data)) };
    }
}
