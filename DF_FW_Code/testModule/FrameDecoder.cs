using System.Text;

namespace DFTestModule;

internal sealed class FrameDecoder
{
    private readonly StringBuilder current = new();
    private bool collecting;

    public void Reset() { current.Clear(); collecting = false; }

    public IList<string> Push(byte[] data)
    {
        List<string> frames = new();
        foreach (byte value in data) {
            if (value == (byte)'$') { current.Clear(); current.Append('$'); collecting = true; continue; }
            if (!collecting) continue;
            if (value == (byte)'%') { current.Append('%'); frames.Add(current.ToString()); current.Clear(); collecting = false; continue; }
            if (value >= 0x20 && value <= 0x7e) current.Append((char)value);
            if (current.Length > 8192) { current.Clear(); collecting = false; }
        }
        return frames;
    }
}

