using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace FishingGame
{
    /// <summary>
    /// PC-safe scene transition behind the original Loading Scene presentation.
    /// Serial and Addressables remain optional backends.
    /// </summary>
    public sealed class LoadingController : MonoBehaviour
    {
        [SerializeField, Min(0f)]
        private float minimumDisplaySeconds = 2f;

        private void Start()
        {
            StartCoroutine(ContinueFlow());
        }

        private IEnumerator ContinueFlow()
        {
            string target = SceneSession.NextSceneName;
            if (string.IsNullOrWhiteSpace(target) || target == SceneSession.LoadingSceneName)
            {
                target = SceneSession.TitleSceneName;
            }

            GameLog.Info("Loading scene in PC mode: " + target);

            AsyncOperation operation = SceneManager.LoadSceneAsync(target, LoadSceneMode.Single);
            if (operation == null)
            {
                GameLog.Error("Failed to open scene in PC mode: " + target);
                yield break;
            }

            // Original LoadingSceneManager holds activation after the async load
            // reaches its 0.9 activation boundary, then exposes the loading scene
            // for two more seconds before switching scenes.
            operation.allowSceneActivation = false;
            while (operation.progress < 0.89f)
            {
                yield return null;
            }

            float elapsed = 0f;
            while (elapsed < minimumDisplaySeconds)
            {
                elapsed += Time.unscaledDeltaTime;
                yield return null;
            }

            operation.allowSceneActivation = true;
            while (!operation.isDone)
            {
                yield return null;
            }
        }

    }
}
