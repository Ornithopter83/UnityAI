namespace FishingGame
{
    /// <summary>
    /// In-memory scene handoff without dependencies on global managers.
    /// </summary>
    public static class SceneSession
    {
        public const string BootstrapSceneName = "Bootstrap";
        public const string LoadingSceneName = "LoadingScene";
        public const string TitleSceneName = "Title";
        public const string SelectSceneName = "SelectScene";

        public static string NextSceneName { get; set; }

        public static void Clear()
        {
            NextSceneName = string.Empty;
        }
    }
}
