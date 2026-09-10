using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace FishingGame
{
    /// <summary>
    /// Safe project entry point used while gameplay systems are connected.
    /// It does not require Serial, payment, networking, or Addressables.
    /// </summary>
    public sealed class BootstrapController : MonoBehaviour
    {
        private bool isLoading;
        private string status = "Ready";

        private void Awake()
        {
            SceneSession.Clear();
            GameLog.Info("Bootstrap ready. Hardware and external services are disabled.");
        }

        private void OnGUI()
        {
            const float width = 420f;
            const float height = 230f;
            Rect panel = new Rect((Screen.width - width) * 0.5f, (Screen.height - height) * 0.5f, width, height);

            GUI.Box(panel, "Fishing Game");
            GUI.Label(new Rect(panel.x + 24f, panel.y + 42f, width - 48f, 24f), "Bootstrap/Test Scene");
            GUI.Label(new Rect(panel.x + 24f, panel.y + 72f, width - 48f, 42f),
                "Serial / Card / Network / Addressables: disabled");
            GUI.Label(new Rect(panel.x + 24f, panel.y + 110f, width - 48f, 24f), "Status: " + status);

            GUI.enabled = !isLoading;
            if (GUI.Button(new Rect(panel.x + 24f, panel.y + 148f, width - 48f, 36f), "Test Loading Scene"))
            {
                StartCoroutine(OpenLoadingScene());
            }

            GUI.enabled = true;
        }

        private IEnumerator OpenLoadingScene()
        {
            isLoading = true;
            status = "Opening LoadingScene...";
            SceneSession.NextSceneName = SceneSession.BootstrapSceneName;

            AsyncOperation operation = SceneManager.LoadSceneAsync(SceneSession.LoadingSceneName, LoadSceneMode.Single);
            if (operation == null)
            {
                status = "LoadingScene is unavailable.";
                GameLog.Error(status);
                SceneSession.Clear();
                isLoading = false;
                yield break;
            }

            while (!operation.isDone)
            {
                status = $"Loading {Mathf.RoundToInt(operation.progress * 100f)}%";
                yield return null;
            }
        }
    }
}

