namespace FishingGame
{
    public enum RuntimeMode
    {
        Pc = 0,
        Serial = 1
    }

    /// <summary>
    /// PC mode is the safe default during restoration. A Serial backend can replace
    /// the no-op device boundary later without changing Scene controllers.
    /// </summary>
    public static class RuntimeModeSettings
    {
        public static RuntimeMode Current { get; set; } = RuntimeMode.Pc;

        public static bool UsesHardware => Current == RuntimeMode.Serial;
    }
}
