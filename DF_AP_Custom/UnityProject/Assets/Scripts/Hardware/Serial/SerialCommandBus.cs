using System;
using System.Collections.Concurrent;

namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// Side-effect boundary between the protocol codec and a serial transport.
    /// Received lines are pumped explicitly so Unity consumers stay on main thread.
    /// </summary>
    public sealed class SerialCommandBus : IDisposable
    {
        private readonly ISerialTransport transport;
        private readonly ConcurrentQueue<string> receivedLines = new ConcurrentQueue<string>();
        private bool disposed;

        public SerialCommandBus(ISerialTransport transport)
        {
            this.transport = transport ?? throw new ArgumentNullException(nameof(transport));
            this.transport.LineReceived += OnLineReceived;
        }

        public event Action<SerialFrame> FrameReceived;

        public event Action<string, string> MalformedFrameReceived;

        public bool IsOpen => transport.IsOpen;

        public void Open()
        {
            ThrowIfDisposed();
            transport.Open();
        }

        public void Close()
        {
            if (!disposed)
            {
                transport.Close();
            }
        }

        public bool TrySend(string frame, out string error)
        {
            ThrowIfDisposed();
            if (!transport.IsOpen)
            {
                error = "Serial transport is closed.";
                return false;
            }
            if (!SerialProtocol.TryParseFrame(frame, out _, out error))
            {
                return false;
            }
            transport.SendLine(frame.TrimEnd('\r', '\n'));
            return true;
        }

        public int PumpReceived()
        {
            ThrowIfDisposed();
            int count = 0;
            while (receivedLines.TryDequeue(out string line))
            {
                count++;
                if (SerialProtocol.TryParseFrame(line, out SerialFrame frame, out string error))
                {
                    FrameReceived?.Invoke(frame);
                }
                else
                {
                    MalformedFrameReceived?.Invoke(line, error);
                }
            }
            return count;
        }

        public void Dispose()
        {
            if (disposed)
            {
                return;
            }
            disposed = true;
            transport.LineReceived -= OnLineReceived;
            transport.Dispose();
            FrameReceived = null;
            MalformedFrameReceived = null;
        }

        private void OnLineReceived(string line)
        {
            receivedLines.Enqueue(line ?? string.Empty);
        }

        private void ThrowIfDisposed()
        {
            if (disposed)
            {
                throw new ObjectDisposedException(nameof(SerialCommandBus));
            }
        }
    }
}
