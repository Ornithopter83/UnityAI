#if FISHINGGAME_SYSTEM_IO_PORTS
using System;
using System.Collections.Concurrent;
using System.IO;
using System.IO.Ports;
using System.Threading;
using System.Threading.Tasks;

namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// Optional real COM transport using the settings recovered from MainSerial.
    /// Nothing creates or opens this transport automatically; hardware access must
    /// be explicitly enabled by a future composition root after device review.
    /// </summary>
    public sealed class SystemSerialTransport : ISerialTransport
    {
        private readonly string portName;
        private readonly ConcurrentQueue<string> outgoing = new ConcurrentQueue<string>();
        private SerialPort port;
        private CancellationTokenSource cancellation;
        private Task ioTask;
        private bool disposed;

        public SystemSerialTransport(string portName = null)
        {
            this.portName = string.IsNullOrWhiteSpace(portName) ? SerialProtocol.ResolveMainPort() : portName;
        }

        public event Action<string> LineReceived;

        public event Action<Exception> Faulted;

        public bool IsOpen => port != null && port.IsOpen;

        public string PortName => portName;

        public void Open()
        {
            ThrowIfDisposed();
            if (IsOpen)
            {
                return;
            }

            port = new SerialPort(portName, SerialProtocol.BaudRate, Parity.None, 8, StopBits.One)
            {
                Handshake = Handshake.None,
                ReadBufferSize = SerialProtocol.ReadBufferSize,
                ReadTimeout = SerialProtocol.ReadTimeoutMilliseconds,
                WriteTimeout = SerialProtocol.WriteTimeoutMilliseconds,
                DtrEnable = true,
                RtsEnable = true,
                NewLine = "\n"
            };
            port.Open();
            cancellation = new CancellationTokenSource();
            ioTask = Task.Run(() => RunIoLoop(cancellation.Token));
        }

        public void SendLine(string frame)
        {
            ThrowIfDisposed();
            if (!IsOpen)
            {
                throw new InvalidOperationException("Serial port is closed: " + portName);
            }
            outgoing.Enqueue(frame);
        }

        public void Close()
        {
            cancellation?.Cancel();
            cancellation?.Dispose();
            cancellation = null;
            ioTask = null;
            if (port != null)
            {
                try
                {
                    if (port.IsOpen)
                    {
                        port.Close();
                    }
                }
                finally
                {
                    port.Dispose();
                    port = null;
                }
            }
            while (outgoing.TryDequeue(out _))
            {
            }
        }

        public void Dispose()
        {
            if (disposed)
            {
                return;
            }
            Close();
            disposed = true;
            LineReceived = null;
            Faulted = null;
        }

        private async Task RunIoLoop(CancellationToken token)
        {
            try
            {
                while (!token.IsCancellationRequested && IsOpen)
                {
                    while (outgoing.TryDequeue(out string frame))
                    {
                        port.WriteLine(frame);
                    }

                    if (port.BytesToRead > 0)
                    {
                        string line = port.ReadLine();
                        LineReceived?.Invoke(line);
                    }
                    else
                    {
                        await Task.Delay(10, token);
                    }
                }
            }
            catch (OperationCanceledException)
            {
            }
            catch (TimeoutException)
            {
                // The original MainSerial treats a 50ms read/write timeout as a
                // retryable condition. Continue unless the port was closed.
                if (!token.IsCancellationRequested && IsOpen)
                {
                    ioTask = Task.Run(() => RunIoLoop(token));
                }
            }
            catch (IOException exception)
            {
                Faulted?.Invoke(exception);
            }
            catch (UnauthorizedAccessException exception)
            {
                Faulted?.Invoke(exception);
            }
            catch (InvalidOperationException exception)
            {
                Faulted?.Invoke(exception);
            }
        }

        private void ThrowIfDisposed()
        {
            if (disposed)
            {
                throw new ObjectDisposedException(nameof(SystemSerialTransport));
            }
        }
    }
}
#endif
