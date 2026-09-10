using UnityEngine;
using UnityEngine.SceneManagement;

namespace FishingGame.UI
{
    /// <summary>
    /// PC adapter for the original Title card-wait state. The original flow enters
    /// the original Addressable SelectScene through Loading after payment/card input;
    /// Enter, Space, or a left click provides that input without cabinet hardware.
    /// </summary>
    public sealed class TitleController : MonoBehaviour
    {
        private bool transitionStarted;

        private void Awake()
        {
            RuntimeModeSettings.Current = RuntimeMode.Pc;
        }

        private void Start()
        {
            GameLog.Info("Original Title card-wait screen ready. Enter/Space/click simulates cabinet input in PC mode.");
        }

        private void Update()
        {
            if (Input.GetKeyDown(KeyCode.Return) || Input.GetKeyDown(KeyCode.Space) || Input.GetMouseButtonDown(0))
            {
                StartPcDemo();
            }
        }

        public void StartPcDemo()
        {
            if (transitionStarted)
            {
                return;
            }

            transitionStarted = true;
            SceneSession.NextSceneName = SceneSession.SelectSceneName;
            GameLog.Info("PC cabinet input accepted on Title; opening original SelectScene flow through Loading.");
            SceneManager.LoadScene(SceneSession.LoadingSceneName, LoadSceneMode.Single);
        }
    }
}
