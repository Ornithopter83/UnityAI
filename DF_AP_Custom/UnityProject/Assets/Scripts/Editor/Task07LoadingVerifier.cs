#if UNITY_EDITOR
using System;
using System.IO;
using Spine.Unity;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace FishingGame.Editor
{
    /// <summary>
    /// Verifies the evidence-backed Loading scene in Play Mode and captures its
    /// real Spine-rendered frame without requiring serial or payment hardware.
    /// </summary>
    [InitializeOnLoad]
    public static class Task07LoadingVerifier
    {
        private const string LoadingPath = "Assets/Scenes/Loading/LoadingScene.unity";
        private const string ActiveKey = "FishingGame.Task07Loading.Active";
        private const string StartedKey = "FishingGame.Task07Loading.Started";
        private const string BoneRotationKey = "FishingGame.Task07Loading.BoneRotation";
        private const string SampledKey = "FishingGame.Task07Loading.Sampled";
        private const string PassedKey = "FishingGame.Task07Loading.Passed";
        private const string MessageKey = "FishingGame.Task07Loading.Message";

        static Task07LoadingVerifier()
        {
            EditorApplication.playModeStateChanged -= OnPlayModeStateChanged;
            EditorApplication.playModeStateChanged += OnPlayModeStateChanged;
            if (SessionState.GetBool(ActiveKey, false) && EditorApplication.isPlaying)
            {
                AttachUpdate();
            }
        }

        [MenuItem("Fishing Game/Verify Task 07 Loading Scene")]
        public static void StartVerification()
        {
            if (EditorApplication.isPlayingOrWillChangePlaymode)
            {
                throw new InvalidOperationException("Exit Play Mode before starting Task 07 verification.");
            }

            SessionState.SetBool(ActiveKey, true);
            SessionState.SetBool(SampledKey, false);
            SessionState.SetBool(PassedKey, false);
            SessionState.SetString(MessageKey, "Verification did not finish.");
            SessionState.SetString(StartedKey, DateTime.UtcNow.Ticks.ToString());
            SceneSession.NextSceneName = SceneSession.TitleSceneName;
            EditorSceneManager.OpenScene(LoadingPath, OpenSceneMode.Single);
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
                Debug.Log("[FishingGame] Task 07 Loading verification passed: " + message);
            }
            else
            {
                Debug.LogError("[FishingGame] Task 07 Loading verification failed: " + message);
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

            double elapsed = ElapsedSeconds();
            if (elapsed > 8d)
            {
                Finish(false, "Timed out before the original Loading animation could be verified.");
                return;
            }

            if (SceneManager.GetActiveScene().name != SceneSession.LoadingSceneName)
            {
                Finish(false, "Loading scene switched before its evidence-backed two-second hold completed.");
                return;
            }

            SkeletonGraphic graphic = UnityEngine.Object.FindObjectOfType<SkeletonGraphic>();
            Spine.Bone rotationBone = graphic == null || graphic.Skeleton == null
                ? null
                : graphic.Skeleton.FindBone("bone2");
            Spine.TrackEntry track = graphic == null || graphic.AnimationState == null
                ? null
                : graphic.AnimationState.GetCurrent(0);
            if (graphic == null || rotationBone == null || track == null || track.Animation == null || track.Animation.Name != "loding_x2")
            {
                Finish(false, "The original loding_x2 Spine track or bone2 was not active in Play Mode.");
                return;
            }

            if (!SessionState.GetBool(SampledKey, false) && elapsed >= 0.35d)
            {
                SessionState.SetFloat(BoneRotationKey, rotationBone.Rotation);
                SessionState.SetBool(SampledKey, true);
                return;
            }

            if (SessionState.GetBool(SampledKey, false) && elapsed >= 0.9d)
            {
                float initialRotation = SessionState.GetFloat(BoneRotationKey, rotationBone.Rotation);
                if (Mathf.Abs(Mathf.DeltaAngle(initialRotation, rotationBone.Rotation)) < 1f || track.TrackTime <= 0f)
                {
                    Finish(false, "loding_x2 was assigned but its rotating fish animation did not advance.");
                    return;
                }

                CaptureFrame();
                Finish(true, "loding_x2 loop advanced and LoadingOriginalRender.png was captured before transition.");
            }
        }

        private static void CaptureFrame()
        {
            Camera camera = Camera.main;
            if (camera == null)
            {
                throw new MissingReferenceException("Loading capture requires the original Main Camera.");
            }

            const int width = 1920;
            const int height = 1080;
            var renderTexture = new RenderTexture(width, height, 24, RenderTextureFormat.ARGB32);
            var capture = new Texture2D(width, height, TextureFormat.RGB24, false);
            RenderTexture previousTarget = camera.targetTexture;
            RenderTexture previousActive = RenderTexture.active;
            try
            {
                Canvas.ForceUpdateCanvases();
                camera.aspect = (float)width / height;
                camera.targetTexture = renderTexture;
                camera.Render();
                RenderTexture.active = renderTexture;
                capture.ReadPixels(new Rect(0f, 0f, width, height), 0, 0);
                capture.Apply();
                string output = Path.GetFullPath(Path.Combine(Application.dataPath, "../Logs/LoadingOriginalRender.png"));
                File.WriteAllBytes(output, capture.EncodeToPNG());
                Debug.Log("[FishingGame] Loading visual capture saved: " + output);
            }
            finally
            {
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
