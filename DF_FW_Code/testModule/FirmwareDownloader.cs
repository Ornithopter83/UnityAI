namespace DFTestModule;

internal sealed class FirmwareDownloader
{
    private const int BlockSize = 0x4000;
    private readonly SerialConnection serial;
    private readonly object ackLock = new();
    private TaskCompletionSource<bool>? pendingAck;
    private int pendingSequence = -1;

    public FirmwareDownloader(SerialConnection serial) { this.serial = serial; }

    public void AcceptFrame(string frame)
    {
        if (!frame.StartsWith("$DN", StringComparison.Ordinal) || frame.Length < 9) return;
        if (!int.TryParse(frame.Substring(4, 3), out int sequence)) return;
        lock (ackLock) {
            if (pendingAck != null && sequence == pendingSequence) pendingAck.TrySetResult(frame[7] == '0');
        }
    }

    public async Task DownloadAsync(FirmwarePackage package, IProgress<int> progress, CancellationToken cancellationToken)
    {
        int count = (package.Data.Length + BlockSize - 1) / BlockSize;
        if (count >= 998) throw new InvalidOperationException("펌웨어 블록 수가 프로토콜 한도를 넘습니다.");
        try {
            for (int sequence = 0; sequence < count; sequence++) {
                cancellationToken.ThrowIfCancellationRequested();
                int offset = sequence * BlockSize; int length = Math.Min(BlockSize, package.Data.Length - offset);
                byte[] raw = new byte[length]; Buffer.BlockCopy(package.Data, offset, raw, 0, length);
                byte[] compressed = new byte[length]; int compressedLength = LzCodec.EncodeLZ(raw, compressed, length, length);
                bool useCompressed = compressedLength < length;
                byte[] payload = useCompressed ? compressed.Take(compressedLength).ToArray() : raw;
                byte board = (byte)(package.BoardId | (useCompressed ? 0x20 : 0));
                await SendAndWaitAsync(DeviceProtocol.DownloadFrame(board, sequence, payload), sequence, cancellationToken);
                progress.Report((sequence + 1) * 100 / count);
            }
            serial.WriteBytes(DeviceProtocol.DownloadFrame(package.BoardId, 999, null));
        }
        catch {
            try { serial.WriteBytes(DeviceProtocol.DownloadFrame(package.BoardId, 998, null)); } catch { }
            throw;
        }
    }

    private async Task SendAndWaitAsync(byte[] frame, int sequence, CancellationToken cancellationToken)
    {
        TaskCompletionSource<bool> source = new(TaskCreationOptions.RunContinuationsAsynchronously);
        lock (ackLock) { pendingSequence = sequence; pendingAck = source; }
        serial.WriteBytes(frame);
        try {
            Task completed = await Task.WhenAny(source.Task, Task.Delay(TimeSpan.FromSeconds(3), cancellationToken));
            cancellationToken.ThrowIfCancellationRequested();
            if (completed != source.Task) throw new TimeoutException("펌웨어 블록 응답 시간이 초과되었습니다.");
            if (!await source.Task) throw new InvalidOperationException("보드가 펌웨어 블록을 거부했습니다.");
        }
        finally {
            lock (ackLock) { if (ReferenceEquals(pendingAck, source)) { pendingAck = null; pendingSequence = -1; } }
        }
    }
}


