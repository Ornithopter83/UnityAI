namespace FishingGame.Hardware.Serial
{
    /// <summary>
    /// Parsed form of the original ASCII frame: '$' + two digit id + payload + '%'.
    /// </summary>
    public readonly struct SerialFrame
    {
        public SerialFrame(int commandId, string payload, string raw)
        {
            CommandId = commandId;
            Payload = payload ?? string.Empty;
            Raw = raw ?? string.Empty;
        }

        public int CommandId { get; }

        public string Payload { get; }

        public string Raw { get; }

        public override string ToString()
        {
            return Raw;
        }
    }
}
