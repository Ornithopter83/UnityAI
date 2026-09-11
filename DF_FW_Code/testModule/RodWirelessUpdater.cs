using System.Security.Cryptography;
using System.Text;

namespace DFTestModule;

internal sealed class RodWirelessUpdater
{
    private const int PayloadSize = 96, MaximumImageSize = 1250000;
    private readonly SerialConnection serial;
    private readonly object ackLock = new();
    private TaskCompletionSource<bool>? pendingAck;
    private byte pendingType; private uint pendingSession, pendingSequence, pendingNext, pendingWritten;

    public RodWirelessUpdater(SerialConnection connection) { serial = connection; }

    public void AcceptFrame(string frame)
    {
        if (!frame.StartsWith("$OR", StringComparison.Ordinal) || !frame.EndsWith('%')) return;
        byte[] ack;
        try { ack = Convert.FromHexString(frame.Substring(3, frame.Length - 4)); } catch { return; }
        lock (ackLock) {
            if (pendingAck == null) return;
            try { if (IsExpectedAck(ack, pendingType, pendingSession, pendingSequence, pendingNext, pendingWritten)) pendingAck.TrySetResult(true); }
            catch (Exception ex) { pendingAck.TrySetException(ex); }
        }
    }

    public async Task DownloadAsync(FirmwarePackage package, IProgress<int> progress, CancellationToken cancellationToken)
    {
        if (package.BoardId != (byte)'R') throw new InvalidOperationException("ROD 펌웨어만 무선 업데이트할 수 있습니다.");
        if (package.Data.Length <= 0 || package.Data.Length > MaximumImageSize) throw new InvalidDataException("ROD 무선 펌웨어는 1~1,250,000 bytes여야 합니다.");
        byte[] version = Encoding.ASCII.GetBytes(package.Version);
        if (version.Length == 0 || version.Length > 24 || version.Any(value => value > 0x7f)) throw new InvalidDataException("ROD 버전은 1~24자의 ASCII 문자열이어야 합니다.");
        uint session = unchecked((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds()) | 1U;
        byte[] digest = SHA256.HashData(package.Data);
        byte[] startPayload = new byte[38 + version.Length];
        startPayload[0] = 1; WriteU32(startPayload, 1, (uint)package.Data.Length);
        Buffer.BlockCopy(digest, 0, startPayload, 5, digest.Length);
        startPayload[37] = (byte)version.Length; Buffer.BlockCopy(version, 0, startPayload, 38, version.Length);

        bool started = false; uint sequence = 0, written = 0;
        try {
            await SendAndWaitAsync(BuildFrame(1, session, 0, startPayload), 1, session, 0, 0, 0, cancellationToken);
            started = true;
            while (written < package.Data.Length) {
                int count = Math.Min(PayloadSize, package.Data.Length - (int)written);
                byte[] payload = new byte[count]; Buffer.BlockCopy(package.Data, (int)written, payload, 0, count); written += (uint)count;
                await SendAndWaitAsync(BuildFrame(2, session, sequence, payload), 2, session, sequence, sequence + 1, written, cancellationToken);
                sequence++; progress.Report((int)(written * 100U / (uint)package.Data.Length));
            }
            await SendAndWaitAsync(BuildFrame(3, session, sequence, Array.Empty<byte>()), 3, session, sequence, sequence, written, cancellationToken);
            started = false;
        } catch {
            if (started && serial.IsOpen) try { serial.WriteText("$OR" + Convert.ToHexString(BuildFrame(4, session, sequence, Array.Empty<byte>())) + "%"); } catch { }
            throw;
        }
    }

    internal static byte[] BuildFrame(byte type, uint session, uint sequence, byte[] payload)
    {
        if (payload.Length > PayloadSize) throw new ArgumentOutOfRangeException(nameof(payload));
        byte[] frame = new byte[19 + payload.Length];
        frame[0] = (byte)'D'; frame[1] = (byte)'F'; frame[2] = (byte)'R'; frame[3] = (byte)'O'; frame[4] = 1; frame[5] = type;
        WriteU32(frame, 6, session); WriteU32(frame, 10, sequence); frame[14] = (byte)payload.Length;
        if (payload.Length > 0) Buffer.BlockCopy(payload, 0, frame, 15, payload.Length);
        WriteU32(frame, 15 + payload.Length, Crc32(frame, 15 + payload.Length)); return frame;
    }

    private async Task SendAndWaitAsync(byte[] frame, byte type, uint session, uint sequence, uint next, uint written, CancellationToken cancellationToken)
    {
        for (int attempt = 1; attempt <= 5; attempt++) {
            TaskCompletionSource<bool> source = new(TaskCreationOptions.RunContinuationsAsynchronously);
            lock (ackLock) { pendingType = type; pendingSession = session; pendingSequence = sequence; pendingNext = next; pendingWritten = written; pendingAck = source; }
            serial.WriteText("$OR" + Convert.ToHexString(frame) + "%");
            try {
                Task completed = await Task.WhenAny(source.Task, Task.Delay(1500, cancellationToken)); cancellationToken.ThrowIfCancellationRequested();
                if (completed == source.Task) { await source.Task; return; }
            } finally { lock (ackLock) { if (ReferenceEquals(pendingAck, source)) pendingAck = null; } }
        }
        throw new TimeoutException("ROD가 OTA 프레임에 응답하지 않았습니다.");
    }

    private static bool IsExpectedAck(byte[] ack, byte type, uint session, uint sequence, uint next, uint written)
    {
        if (ack.Length < 29 || ack[0] != 'D' || ack[1] != 'F' || ack[2] != 'R' || ack[3] != 'O' || ack[4] != 1 || ack[5] != 5 || ack[14] != 10) return false;
        if (Crc32(ack, ack.Length - 4) != ReadU32(ack, ack.Length - 4)) return false;
        if (ReadU32(ack, 6) != session || ReadU32(ack, 10) != sequence || ack[15] != type) return false;
        if (ack[16] != 0) throw new InvalidOperationException("ROD OTA 오류 상태 " + ack[16] + " (프레임 " + type + ", 순번 " + sequence + ")");
        return ReadU32(ack, 17) == next && ReadU32(ack, 21) == written;
    }

    private static uint Crc32(byte[] data, int length)
    {
        uint crc = uint.MaxValue;
        for (int index = 0; index < length; index++) { crc ^= data[index]; for (int bit = 0; bit < 8; bit++) crc = (crc & 1) != 0 ? (crc >> 1) ^ 0xedb88320U : crc >> 1; }
        return crc ^ uint.MaxValue;
    }
    private static void WriteU32(byte[] data, int offset, uint value) { data[offset] = (byte)(value >> 24); data[offset + 1] = (byte)(value >> 16); data[offset + 2] = (byte)(value >> 8); data[offset + 3] = (byte)value; }
    private static uint ReadU32(byte[] data, int offset) { return ((uint)data[offset] << 24) | ((uint)data[offset + 1] << 16) | ((uint)data[offset + 2] << 8) | data[offset + 3]; }
}
