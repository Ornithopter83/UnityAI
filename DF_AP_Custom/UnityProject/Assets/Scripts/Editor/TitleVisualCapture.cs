#if UNITY_EDITOR
using System.IO;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.UI;

namespace FishingGame.Editor
{
    public static class TitleVisualCapture
    {
        private const string TitlePath = "Assets/Scenes/Menu/Title.unity";

        [MenuItem("Fishing Game/Capture Title Reference View")]
        public static void Capture()
        {
            EditorSceneManager.OpenScene(TitlePath, OpenSceneMode.Single);
            Camera camera = Camera.main;
            Canvas canvas = Object.FindObjectOfType<Canvas>();
            if (camera == null || canvas == null)
            {
                throw new MissingReferenceException("Title capture requires a Main Camera and Canvas.");
            }

            const int width = 1920;
            const int height = 1080;
            var renderTexture = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32);
            var capture = new Texture2D(width, height, TextureFormat.RGB24, false);
            RenderTexture previousTarget = camera.targetTexture;
            RenderTexture previousActive = RenderTexture.active;
            RenderMode previousMode = canvas.renderMode;
            Camera previousCamera = canvas.worldCamera;
            float previousDistance = canvas.planeDistance;

            try
            {
                canvas.renderMode = RenderMode.ScreenSpaceCamera;
                canvas.worldCamera = camera;
                canvas.planeDistance = 1f;
                Canvas.ForceUpdateCanvases();
                camera.aspect = (float)width / height;
                camera.targetTexture = renderTexture;
                camera.Render();

                RenderTexture.active = renderTexture;
                capture.ReadPixels(new Rect(0f, 0f, width, height), 0, 0);
                capture.Apply();
                string output = Path.GetFullPath(Path.Combine(Application.dataPath, "../Logs/TitleObjectRender.png"));
                File.WriteAllBytes(output, capture.EncodeToPNG());
                Debug.Log("[FishingGame] Title visual capture saved: " + output);

                Transform rank = null;
                foreach (Transform candidate in Object.FindObjectsOfType<Transform>(true))
                {
                    if (candidate.name == "Rank")
                    {
                        rank = candidate;
                        break;
                    }
                }
                if (rank == null)
                {
                    throw new MissingReferenceException("Title capture requires the original Rank overlay.");
                }

                rank.gameObject.SetActive(true);
                RectTransform content = null;
                foreach (RectTransform candidate in rank.GetComponentsInChildren<RectTransform>(true))
                {
                    if (candidate.name == "Content")
                    {
                        content = candidate;
                        break;
                    }
                }
                if (content != null)
                {
                    LayoutRebuilder.ForceRebuildLayoutImmediate(content);
                }
                Canvas.ForceUpdateCanvases();
                camera.Render();
                RenderTexture.active = renderTexture;
                capture.ReadPixels(new Rect(0f, 0f, width, height), 0, 0);
                capture.Apply();
                string rankOutput = Path.GetFullPath(Path.Combine(Application.dataPath, "../Logs/TitleRankRender.png"));
                File.WriteAllBytes(rankOutput, capture.EncodeToPNG());
                rank.gameObject.SetActive(false);
                Debug.Log("[FishingGame] Title rank capture saved: " + rankOutput);
            }
            finally
            {
                camera.targetTexture = previousTarget;
                RenderTexture.active = previousActive;
                canvas.renderMode = previousMode;
                canvas.worldCamera = previousCamera;
                canvas.planeDistance = previousDistance;
                Object.DestroyImmediate(capture);
                Object.DestroyImmediate(renderTexture);
            }
        }

    }
}
#endif
