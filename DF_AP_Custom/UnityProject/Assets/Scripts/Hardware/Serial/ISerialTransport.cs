using System;

namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// Main-board line transport. A real COM implementation may raise LineReceived
    /// off the Unity thread; SerialCommandBus queues it until PumpReceived is called.
    /// </summary>
    public interface ISerialTransport : IDisposable
    {
        event Action<string> LineReceived;

        bool IsOpen { get; }

        void Open();

        void Close();

        void SendLine(string frame);
    }
}
