#if UNITY_EDITOR
using System;
using System.IO;
using FishingGame.UI;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace FishingGame.Editor
{
    /// <summary>
    /// Verifies the PC payment adapter flow and the two original warning states
    /// before exercising the original mode-selection positions.
    /// </summary>
    [InitializeOnLoad]
    public static class Task08SelectVerifier
    {
        private const string TitlePath = "Assets/Scenes/Menu/Title.unity";
        private const string ActiveKey = "FishingGame.Task08.Active";
        private const string PhaseKey = "FishingGame.Task08.Phase";
        private const string StartedKey = "FishingGame.Task08.Started";
        private const string PassedKey = "FishingGame.Task08.Passed";
        private const string MessageKey = "FishingGame.Task08.Message";

        static Task08SelectVerifier()
        {
            EditorApplication.playModeStateChanged -= OnPlayModeStateChanged;
            EditorApplication.playModeStateChanged += OnPlayModeStateChanged;
            if (SessionState.GetBool(ActiveKey, false) && EditorApplication.isPlaying)
            {
                AttachUpdate();
            }
        }

        [MenuItem("Fishing Game/Verify Task 08 Warning and Mode Flow")]
        public static void StartVerification()
        {
            if (EditorApplication.isPlayingOrWillChangePlaymode)
            {
                throw new InvalidOperationException("Exit Play Mode before starting Task 08 verification.");
            }

            SessionState.SetBool(ActiveKey, true);
            SessionState.SetInt(PhaseKey, 0);
            SessionState.SetBool(PassedKey, false);
            SessionState.SetString(MessageKey, "Verification did not finish.");
            SessionState.SetString(StartedKey, DateTime.UtcNow.Ticks.ToString());
            SceneSession.Clear();
            EditorSceneManager.OpenScene(TitlePath, OpenSceneMode.Single);
            EditorApplication.EnterPlaymode();
        }

        private static void OnPlayModeStateChanged(PlayModeStateChange state)
        {
            if (!SessionState.GetBool(ActiveKey, false))
            {
                return;
            }

            if (state == PlayModeStateChange.EnteredPlayMode)
            {
                AttachUpdate();
                return;
            }

            if (state != PlayModeStateChange.EnteredEditMode)
            {
                return;
            }

            EditorApplication.update -= VerifyUpdate;
            bool passed = SessionState.GetBool(PassedKey, false);
            string message = SessionState.GetString(MessageKey, "Verification ended without a result.");
            SessionState.SetBool(ActiveKey, false);
            if (passed)
            {
                Debug.Log("[FishingGame] Task 08 verification passed: " + message);
            }
            else
            {
                Debug.LogError("[FishingGame] Task 08 verification failed: " + message);
            }

            if (Application.isBatchMode)
            {
                EditorApplication.Exit(passed ? 0 : 1);
            }
        }

        private static void AttachUpdate()
        {
            EditorApplication.update -= VerifyUpdate;
            EditorApplication.update += VerifyUpdate;
        }

        private static void VerifyUpdate()
        {
            if (!EditorApplication.isPlaying)
            {
                return;
            }

            if (ElapsedSeconds() > 12d)
            {
                Finish(false, "Timed out while following Title -> Loading -> SelectScene.");
                return;
            }

            int phase = SessionState.GetInt(PhaseKey, 0);
            string sceneName = SceneManager.GetActiveScene().name;
            if (phase == 0)
            {
                TitleController title = UnityEngine.Object.FindObjectOfType<TitleController>();
                if (sceneName != SceneSession.TitleSceneName || title == null)
                {
                    return;
                }
                title.StartPcDemo();
                SessionState.SetInt(PhaseKey, 1);
                return;
            }

            if (phase == 1)
            {
                if (sceneName == SceneSession.LoadingSceneName)
                {
                    return;
                }
                if (sceneName != SceneSession.SelectSceneName)
                {
                    Finish(false, "Payment adapter did not arrive at SelectScene through LoadingScene.");
                    return;
                }

                SelectSceneController controller = UnityEngine.Object.FindObjectOfType<SelectSceneController>();
                if (controller == null || controller.Stage != SelectSceneController.SelectStage.WarningSafety)
                {
                    Finish(false, "SelectScene did not start on the first warning page.");
                    return;
                }
                CaptureStage("Task08WarningPage1.png");
                controller.SkipCurrentWarning();
                if (controller.Stage != SelectSceneController.SelectStage.WarningControls)
                {
                    Finish(false, "The first warning page was not skippable by the cabinet-button adapter.");
                    return;
                }
                CaptureStage("Task08WarningPage2.png");
                SessionState.SetInt(PhaseKey, 2);
                return;
            }

            SelectSceneController select = UnityEngine.Object.FindObjectOfType<SelectSceneController>();
            if (select == null)
            {
                Finish(false, "SelectSceneController disappeared during warning verification.");
                return;
            }

            if (phase == 2)
            {
                select.SkipCurrentWarning();
                if (select.Stage != SelectSceneController.SelectStage.ModeSelect || select.SelectedMode != 0)
                {
                    Finish(false, "The second warning did not enter SINGLE-first mode selection.");
                    return;
                }
                CaptureStage("Task08ModeSelect.png");
                SessionState.SetInt(PhaseKey, 3);
                return;
            }

            RectTransform outline = GameObject.Find("select_outline(1)")?.GetComponent<RectTransform>();
            RectTransform block = GameObject.Find("block")?.GetComponent<RectTransform>();
            if (outline == null || block == null || !Near(outline.anchoredPosition.x, -364f) || !Near(block.anchoredPosition.x, 362f))
            {
                Finish(false, "Initial SINGLE outline/block positions differ from the original Scene.");
                return;
            }

            select.MoveSelection();
            if (select.SelectedMode != 1 || !Near(outline.anchoredPosition.x, 364f) || !Near(block.anchoredPosition.x, -362f))
            {
                Finish(false, "BATTLE toggle did not exchange the original outline/block positions.");
                return;
            }

            select.ConfirmSelection();
            if (!select.SelectionConfirmed)
            {
                Finish(false, "The current mode could not be confirmed with the PC adapter.");
                return;
            }

            Finish(true, "Title -> Loading -> warning 1 -> warning 2 -> mode selection; skip, toggle, and confirm passed.");
        }

        private static bool Near(float value, float expected)
        {
            return Mathf.Abs(value - expected) < 0.1f;
        }

        private static void CaptureStage(string fileName)
        {
            Camera camera = Camera.main;
            Canvas canvas = UnityEngine.Object.FindObjectOfType<Canvas>();
            if (camera == null || canvas == null)
            {
                throw new MissingReferenceException("Task 08 capture requires a Main Camera and Canvas.");
            }

            const int width = 1920;
            const int height = 1080;
            var renderTexture = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32);
            var capture = new Texture2D(width, height, TextureFormat.RGB24, false);
            RenderMode previousMode = canvas.renderMode;
            Camera previousCanvasCamera = canvas.worldCamera;
            float previousPlaneDistance = canvas.planeDistance;
            RenderTexture previousTarget = camera.targetTexture;
            RenderTexture previousActive = RenderTexture.active;
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
                string output = Path.GetFullPath(Path.Combine(Application.dataPath, "../Logs/" + fileName));
                File.WriteAllBytes(output, capture.EncodeToPNG());
                Debug.Log("[FishingGame] Task 08 visual capture saved: " + output);
            }
            finally
            {
                canvas.renderMode = previousMode;
                canvas.worldCamera = previousCanvasCamera;
                canvas.planeDistance = previousPlaneDistance;
                camera.targetTexture = previousTarget;
                RenderTexture.active = previousActive;
                UnityEngine.Object.DestroyImmediate(capture);
                UnityEngine.Object.DestroyImmediate(renderTexture);
            }
        }

        private static double ElapsedSeconds()
        {
            string rawTicks = SessionState.GetString(StartedKey, "0");
            return long.TryParse(rawTicks, out long ticks)
                ? TimeSpan.FromTicks(DateTime.UtcNow.Ticks - ticks).TotalSeconds
                : 99d;
        }

        private static void Finish(bool passed, string message)
        {
            EditorApplication.update -= VerifyUpdate;
            SessionState.SetBool(PassedKey, passed);
            SessionState.SetString(MessageKey, message);
            EditorApplication.ExitPlaymode();
        }
    }
}
#endif
