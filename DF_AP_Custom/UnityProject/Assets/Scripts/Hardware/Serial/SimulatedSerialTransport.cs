using System;
using System.Collections.Generic;

namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// PC-only transport. It never opens a COM port and records outgoing frames.
    /// </summary>
    public sealed class SimulatedSerialTransport : ISerialTransport
    {
        private readonly List<string> sentLines = new List<string>();

        public event Action<string> LineReceived;

        public bool IsOpen { get; private set; }

        public IReadOnlyList<string> SentLines => sentLines;

        public void Open()
        {
            IsOpen = true;
        }

        public void Close()
        {
            IsOpen = false;
        }

        public void SendLine(string frame)
        {
            if (!IsOpen)
            {
                throw new InvalidOperationException("Simulated serial transport is closed.");
            }
            sentLines.Add(frame);
        }

        public void InjectReceivedLine(string frame)
        {
            if (!IsOpen)
            {
                throw new InvalidOperationException("Simulated serial transport is closed.");
            }
            LineReceived?.Invoke(frame);
        }

        public void ClearSentLines()
        {
            sentLines.Clear();
        }

        public void Dispose()
        {
            Close();
            LineReceived = null;
        }
    }
}
