#if UNITY_EDITOR
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using FishingGame.UI;
using Spine.Unity;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.Rendering;
using UnityEngine.UI;
using UnityEngine.Video;

namespace FishingGame.Editor
{
    public static class SceneSetup
    {
        private const string BootstrapPath = "Assets/Scenes/Bootstrap/Bootstrap.unity";
        private const string LoadingPath = "Assets/Scenes/Loading/LoadingScene.unity";
        private const string SelectPath = "Assets/Scenes/Menu/SelectScene.unity";
        private const string TitlePath = "Assets/Scenes/Menu/Title.unity";
        private const string OldLoadingPath = "Assets/Scenes/Bootstrap/LoadingScene.unity";
        private const string UiRoot = "Assets/Art/UI/Source/";
        private const string AudioRoot = "Assets/Audio/BGM/";
        private const string TitleMeshRoot = "Assets/Models/Title/Mesh/";
        private const string TitleMaterialRoot = "Assets/Materials/Title/";
        private const string TitleGeneratedMaterialRoot = "Assets/Materials/Title/Generated/";
        private const string TitleTextureRoot = "Assets/Art/Title/Textures/";
        private const string TitleSpriteRoot = "Assets/Art/Title/Sprites/";
        private const string TitleIdleSpriteRoot = "Assets/Art/Title/Idle/Sprites/";
        private const string TitleIdleRenderTextureRoot = "Assets/Art/Title/Idle/RenderTextures/";
        private const string TitleVideoRoot = "Assets/Video/Title/";
        private const string TitleAudioRoot = "Assets/Audio/Title/";
        private const string FontRoot = "Assets/Art/Fonts/";
        private const string LoadingArtRoot = "Assets/Art/Loading/";
        private const string LoadingSpineRoot = LoadingArtRoot + "Spine/";
        private const string SelectionArtRoot = "Assets/Art/Selection/";

        [MenuItem("Fishing Game/Create Restored PC Scenes")]
        public static void CreateScenes()
        {
            EnsureDirectory(Path.GetDirectoryName(BootstrapPath));
            EnsureDirectory(Path.GetDirectoryName(LoadingPath));
            EnsureDirectory(Path.GetDirectoryName(SelectPath));
            AssetDatabase.Refresh();
            PrepareScenePaths();

            CreateBootstrapScene();
            CreateLoadingScene();
            CreateSelectScene();
            CreateTitleScene();

            AssetDatabase.SaveAssets();
            AssetDatabase.Refresh();
            UpdateBuildSettings();
            AssetDatabase.SaveAssets();
            ValidateScenes();
            Debug.Log("[FishingGame] Loading, Select, and Title PC scenes created.");
        }

        public static void ValidateScenes()
        {
            ValidateScene<BootstrapController>(BootstrapPath);
            ValidateScene<LoadingController>(LoadingPath);
            ValidateOriginalLoading();
            ValidateScene<SelectSceneController>(SelectPath);
            ValidateScene<TitleController>(TitlePath);
            ValidateReferenceTitle();

            string[] required = { TitlePath, LoadingPath, SelectPath, BootstrapPath };
            string[] enabledPaths = EditorBuildSettings.scenes
                .Where(scene => scene.enabled)
                .Select(scene => scene.path)
                .ToArray();
            if (required.Any(path => !enabledPaths.Contains(path)))
            {
                throw new InvalidOperationException("A restored Scene is missing from Build Settings.");
            }

            Debug.Log("[FishingGame] Scene validation passed: Bootstrap, Loading, SelectScene, Title.");
        }

        private static void ValidateReferenceTitle()
        {
            string[] requiredObjects =
            {
                "Gwangan Bridge", "ocean", "Ships", "yacht", "yacht (1)",
                "Cruiseship_RotatingRadars", "titleLogo", "symbol", "Card",
                "payment_text", "battery", "power_lv", "clinet_version"
            };
            if (requiredObjects.Any(name => GameObject.Find(name) == null))
            {
                throw new InvalidOperationException("A required original Title object is missing.");
            }

            if (UnityEngine.Object.FindObjectsOfType<TitleShipMotion>().Length < 3)
            {
                throw new InvalidOperationException("Title ship motion adapters were not found.");
            }

            foreach (string meshObjectName in new[] { "Gwangan Bridge", "Yacht_PBR", "Cruiseship_RotatingRadars", "Cruiseship_WithoutRadars" })
            {
                GameObject meshObject = GameObject.Find(meshObjectName);
                MeshFilter filter = meshObject == null ? null : meshObject.GetComponent<MeshFilter>();
                MeshRenderer renderer = meshObject == null ? null : meshObject.GetComponent<MeshRenderer>();
                if (filter == null || filter.sharedMesh == null || filter.sharedMesh.vertexCount == 0 || renderer == null || renderer.sharedMaterials.Length == 0)
                {
                    throw new InvalidOperationException("Title geometry was not restored correctly: " + meshObjectName);
                }
            }

            if (UnityEngine.Object.FindObjectOfType<TitleStatusView>() == null)
            {
                throw new InvalidOperationException("Mutable Title status UI was not found.");
            }

            TitleIdleLoop idleLoop = UnityEngine.Object.FindObjectOfType<TitleIdleLoop>();
            if (idleLoop == null || UnityEngine.Object.FindObjectsOfType<TitleRankRowView>(true).Length != 30)
            {
                throw new InvalidOperationException("The original 30-entry Title idle ranking was not restored.");
            }

            Transform[] allTitleTransforms = UnityEngine.Object.FindObjectsOfType<Transform>(true);
            string[] idleObjects = { "Rank", "GameSupport", "Hold", "Hook", "Store", "Point", "Mystic", "WaitAndHook", "LRCont", "Lv", "intro" };
            if (idleObjects.Any(name => allTitleTransforms.All(item => item.name != name)))
            {
                throw new InvalidOperationException("A required original Title idle-loop object is missing.");
            }

            VideoPlayer player = idleLoop.GetComponentInChildren<VideoPlayer>(true);
            if (player == null || player.clip == null || player.clip.name != "fishing")
            {
                throw new InvalidOperationException("The original fishing.webm Title intro was not restored.");
            }

            string[] removedPlaceholders = { "TitleReferenceScreen", "PcMenuPanel", "CharacterPreview", "StartButton", "ResultButton", "LoadingButton" };
            if (removedPlaceholders.Any(name => GameObject.Find(name) != null))
            {
                throw new InvalidOperationException("An invented Title placeholder is still present.");
            }
        }

        private static void ValidateOriginalLoading()
        {
            string[] removedPlaceholders =
            {
                "OriginalBackground", "CinematicTint", "FishingFamilyLogo", "LoadingCard",
                "StatusText", "LoadingDots", "ProgressBack", "ProgressFill", "PcModeNote"
            };
            if (removedPlaceholders.Any(name => GameObject.Find(name) != null))
            {
                throw new InvalidOperationException("An invented Loading placeholder is still present.");
            }

            SkeletonGraphic graphic = UnityEngine.Object.FindObjectOfType<SkeletonGraphic>();
            LoadingController controller = UnityEngine.Object.FindObjectOfType<LoadingController>();
            SerializedProperty holdSeconds = controller == null
                ? null
                : new SerializedObject(controller).FindProperty("minimumDisplaySeconds");
            if (GameObject.Find("back") == null || GameObject.Find("subBack") == null ||
                controller == null || controller.name != "loadingManager" || holdSeconds == null ||
                !Mathf.Approximately(holdSeconds.floatValue, 2f) ||
                graphic == null || graphic.name != "SkeletonGraphic (loding)" || graphic.skeletonDataAsset == null ||
                graphic.startingAnimation != "loding_x2" || !graphic.startingLoop ||
                !Mathf.Approximately(graphic.timeScale, 2f))
            {
                throw new InvalidOperationException("The original Loading Spine animation was not restored.");
            }

            RectTransform rect = graphic.rectTransform;
            if (Vector2.Distance(rect.sizeDelta, new Vector2(586.10004f, 633.6256f)) > 0.01f)
            {
                throw new InvalidOperationException("The original Loading SkeletonGraphic layout was not restored.");
            }
        }

        private static void ValidateScene<T>(string path) where T : Component
        {
            EditorSceneManager.OpenScene(path, OpenSceneMode.Single);
            if (UnityEngine.Object.FindObjectOfType<T>() == null)
            {
                throw new InvalidOperationException(typeof(T).Name + " component was not found in " + path);
            }

            Canvas canvas = UnityEngine.Object.FindObjectOfType<Canvas>();
            if (typeof(T) != typeof(BootstrapController) && canvas == null)
            {
                throw new InvalidOperationException("Canvas was not found in " + path);
            }
        }

        private static void PrepareScenePaths()
        {
            if (AssetDatabase.LoadAssetAtPath<SceneAsset>(OldLoadingPath) == null)
            {
                return;
            }

            if (AssetDatabase.LoadAssetAtPath<SceneAsset>(LoadingPath) == null)
            {
                string error = AssetDatabase.MoveAsset(OldLoadingPath, LoadingPath);
                if (!string.IsNullOrEmpty(error))
                {
                    throw new InvalidOperationException(error);
                }
            }
            else
            {
                AssetDatabase.DeleteAsset(OldLoadingPath);
            }
        }

        private static void CreateBootstrapScene()
        {
            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            CreateCamera(new Color(0.035f, 0.08f, 0.12f));
            new GameObject("Bootstrap").AddComponent<BootstrapController>();
            EditorSceneManager.SaveScene(scene, BootstrapPath);
        }

        private static void CreateLoadingScene()
        {
            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            Camera camera = CreateOriginalLoadingCamera();
            Canvas canvas = CreateCanvas();
            canvas.renderMode = RenderMode.ScreenSpaceCamera;
            canvas.worldCamera = camera;
            canvas.planeDistance = 0.4f;
            canvas.pixelPerfect = true;
            canvas.vertexColorAlwaysGammaSpace = true;
            canvas.additionalShaderChannels = AdditionalCanvasShaderChannels.TexCoord1 |
                                              AdditionalCanvasShaderChannels.Normal |
                                              AdditionalCanvasShaderChannels.Tangent;
            CanvasScaler scaler = canvas.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ConstantPixelSize;
            scaler.referencePixelsPerUnit = 100f;
            scaler.referenceResolution = new Vector2(800f, 600f);
            scaler.matchWidthOrHeight = 0f;

            Image back = CreateFullImage("back", canvas.transform, null,
                new Color(0.3773585f, 0.3773585f, 0.3773585f, 1f));
            CreateFullImage("subBack", back.transform, LoadingBackgroundSprite(), Color.white);
            CreateOriginalLoadingSkeleton(back.transform);

            GameObject manager = new GameObject("loadingManager");
            manager.transform.position = new Vector3(1820f, 100f, 0f);
            manager.AddComponent<LoadingController>();

            canvas.transform.SetSiblingIndex(0);
            manager.transform.SetSiblingIndex(1);
            camera.transform.SetSiblingIndex(2);
            GameObject.Find("EventSystem")?.transform.SetSiblingIndex(3);
            EditorSceneManager.SaveScene(scene, LoadingPath);
        }

        private static Camera CreateOriginalLoadingCamera()
        {
            var cameraObject = new GameObject("Main Camera");
            cameraObject.tag = "MainCamera";
            cameraObject.transform.position = new Vector3(0f, 1f, -10f);
            Camera camera = cameraObject.AddComponent<Camera>();
            camera.clearFlags = CameraClearFlags.Skybox;
            camera.backgroundColor = Color.black;
            camera.fieldOfView = 60f;
            camera.nearClipPlane = 0.3f;
            camera.farClipPlane = 1000f;
            camera.depth = -1f;
            cameraObject.AddComponent<AudioListener>();
            return camera;
        }

        private static void CreateOriginalLoadingSkeleton(Transform parent)
        {
            var skeletonObject = new GameObject("SkeletonGraphic (loding)", typeof(RectTransform));
            skeletonObject.transform.SetParent(parent, false);
            RectTransform rect = skeletonObject.GetComponent<RectTransform>();
            rect.anchorMin = new Vector2(0.5f, 0.5f);
            rect.anchorMax = new Vector2(0.5f, 0.5f);
            rect.anchoredPosition = Vector2.zero;
            rect.sizeDelta = new Vector2(586.10004f, 633.6256f);
            rect.pivot = new Vector2(0.4979526f, 0.46096623f);

            SkeletonGraphic graphic = skeletonObject.AddComponent<SkeletonGraphic>();
            graphic.raycastTarget = false;
            graphic.maskable = false;
            graphic.material = LoadingMaterialAt("SkeletonGraphicDefault-Straight.mat");
            graphic.skeletonDataAsset = LoadingSkeletonData();
            graphic.additiveMaterial = LoadingMaterialAt("SkeletonGraphicAdditive.mat");
            graphic.multiplyMaterial = LoadingMaterialAt("SkeletonGraphicMultiply.mat");
            graphic.screenMaterial = LoadingMaterialAt("SkeletonGraphicScreen.mat");
            graphic.initialSkinName = "default";
            graphic.startingAnimation = "loding_x2";
            graphic.startingLoop = true;
            graphic.timeScale = 2f;
        }

        private static void CreateSelectScene()
        {
            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            CreateCamera(Color.black);
            Canvas canvas = CreateCanvas();

            Sprite safety = SelectionSpriteAt("Warning/precautionsBg3.asset");
            Sprite controls = SelectionSpriteAt("Warning/rod_controller.asset");
            Image warning = CreateFullImage("Warning", canvas.transform, safety, Color.white);
            var firstPageTexts = new GameObject("FirstPageTexts", typeof(RectTransform));
            firstPageTexts.transform.SetParent(warning.transform, false);
            Stretch(firstPageTexts.GetComponent<RectTransform>());

            CreateSelectText("Text", firstPageTexts.transform, "주의사항", 70, TextAnchor.MiddleCenter, Color.red,
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0f, 126f), new Vector2(360.3f, 86.9f));
            Text warningText = CreateSelectText("Text (1)", firstPageTexts.transform,
                "낚싯대를 휘두르거나 무리하게 당기면\n기기 파손으로 인해 다칠 수 있습니다.", 47,
                TextAnchor.MiddleCenter, new Color(1f, 1f, 1f, 0.404f),
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(0f, -19f), new Vector2(798.2f, 121.8f));
            CreateSelectText("Text (2)", firstPageTexts.transform,
                "※이용자의 무리한 게임 동작으로 인해 발생한 사고에 대한 \n    책임은 본인에게 있음을 알려드립니다.", 33,
                TextAnchor.MiddleLeft, Color.white,
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(8.3f, -178.2f), new Vector2(771.5f, 111.1f));

            var bottom = new GameObject("GameObject", typeof(RectTransform));
            bottom.transform.SetParent(warning.transform, false);
            RectTransform bottomRect = bottom.GetComponent<RectTransform>();
            bottomRect.anchorMin = new Vector2(0.5f, 0f);
            bottomRect.anchorMax = new Vector2(0.5f, 0f);
            bottomRect.anchoredPosition = new Vector2(0f, -21.2f);
            bottomRect.sizeDelta = new Vector2(983.3f, 100f);
            CreateSelectText("Text", bottom.transform, "<          또는          버튼을 눌러 다음 페이지로 가기 >", 40,
                TextAnchor.MiddleCenter, Color.white, Vector2.zero, Vector2.one, Vector2.zero, Vector2.zero);
            CreateImage("btn_l", bottom.transform, SpriteAt("btn_l.png"),
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(-330f, 0f), new Vector2(64f, 64f), Color.white, true);
            CreateImage("btn_r", bottom.transform, SpriteAt("btn_r.png"),
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(-180f, 0f), new Vector2(64f, 64f), Color.white, true);

            var modeRoot = new GameObject("ModeSelect", typeof(RectTransform));
            modeRoot.transform.SetParent(canvas.transform, false);
            Stretch(modeRoot.GetComponent<RectTransform>());
            CreateFullImage("bg", modeRoot.transform, LoadingBackgroundSprite(), Color.white);
            Image heading = CreateImage("LV_bg(1)", modeRoot.transform, SelectionSpriteAt("Mode/ui_diffiBox.asset"),
                new Vector2(0f, 0.5f), new Vector2(1f, 0.5f), new Vector2(0f, 455f), new Vector2(0f, 150f), Color.white, false);
            CreateSelectText("Lv_text", heading.transform, "게임 모드", 80, TextAnchor.MiddleCenter, Color.white,
                new Vector2(0f, 0.5f), new Vector2(1f, 0.5f), Vector2.zero, new Vector2(0f, 50f));

            CreateImage("mode_single", modeRoot.transform, SelectionSpriteAt("Mode/singelMod.asset"),
                new Vector2(0f, 0.5f), new Vector2(0f, 0.5f), new Vector2(345f, -25f), new Vector2(505f, 772f), Color.white, false)
                .rectTransform.pivot = new Vector2(0f, 0.5f);
            CreateImage("mode_multi", modeRoot.transform, SelectionSpriteAt("Mode/battleMod.asset"),
                new Vector2(1f, 0.5f), new Vector2(1f, 0.5f), new Vector2(-345f, -25f), new Vector2(505f, 772f), Color.white, false)
                .rectTransform.pivot = new Vector2(1f, 0.5f);
            Image outline = CreateImage("select_outline(1)", modeRoot.transform, SelectionSpriteAt("Mode/border_0.asset"),
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(-364f, -28f), new Vector2(505f, 793.3f), Color.white, false);
            outline.rectTransform.localScale = new Vector3(1f, 1.01f, 1f);
            Image block = CreateImage("block", modeRoot.transform, null,
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(362f, -25f), new Vector2(505f, 772f),
                new Color(0f, 0f, 0f, 0.807843f), false);
            Text readyText = CreateSelectText("Text", block.transform, "준비중", 50, TextAnchor.MiddleCenter, Color.white,
                Vector2.zero, Vector2.one, Vector2.zero, Vector2.zero);
            readyText.gameObject.SetActive(false);

            Image leftButton = CreateImage("lbtn(1)", modeRoot.transform, SpriteAt("btn_l.png"),
                new Vector2(0.5f, 0f), new Vector2(0.5f, 0f), new Vector2(-110f, 20f), new Vector2(100f, 100f), Color.white, true);
            CreateSelectText("Text", leftButton.transform, "이동", 38, TextAnchor.MiddleCenter, Color.white,
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(-99f, 0f), new Vector2(94.3f, 50f));
            Image rightButton = CreateImage("rbtn(1)", modeRoot.transform, SpriteAt("btn_r.png"),
                new Vector2(0.5f, 0f), new Vector2(0.5f, 0f), new Vector2(110f, 20f), new Vector2(100f, 100f), Color.white, true);
            CreateSelectText("Text", rightButton.transform, "선택", 38, TextAnchor.MiddleCenter, Color.white,
                new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), new Vector2(99f, 0f), new Vector2(94.3f, 50f));

            modeRoot.SetActive(false);
            SelectSceneController controller = new GameObject("SelectSceneManager").AddComponent<SelectSceneController>();
            controller.Configure(warning.gameObject, warning, firstPageTexts, warningText, modeRoot,
                outline.rectTransform, block.rectTransform, safety, controls);
            EditorSceneManager.SaveScene(scene, SelectPath);
        }

        private static void CreateTitleScene()
        {
            var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);
            CreateTitleCameraAndLighting();
            CreateTitleEnvironment();
            Canvas canvas = CreateCanvas();
            CreateTitleInterface(canvas);

            AudioSource titleBgm = AddLoopingAudio("Title BGM", AudioAt("bgm_title.ogg"), 0.32f);
            CreateTitleIdleLoop(canvas, titleBgm);
            new GameObject("TitleController").AddComponent<TitleController>();
            EditorSceneManager.SaveScene(scene, TitlePath);
        }

        private static void CreateTitleCameraAndLighting()
        {
            var cameraObject = new GameObject("Main Camera");
            cameraObject.tag = "MainCamera";
            cameraObject.transform.position = new Vector3(0f, 5f, 0f);
            cameraObject.transform.rotation = Quaternion.identity;
            Camera camera = cameraObject.AddComponent<Camera>();
            camera.clearFlags = CameraClearFlags.Skybox;
            camera.backgroundColor = Color.black;
            camera.fieldOfView = 60f;
            camera.nearClipPlane = 0.3f;
            camera.farClipPlane = 500f;
            camera.allowHDR = true;
            camera.allowMSAA = true;
            cameraObject.AddComponent<AudioListener>();

            Material sky = GetOrCreateMaterial("TitleSky", Shader.Find("Skybox/Procedural"));
            sky.SetFloat("_SunSize", 0.035f);
            sky.SetFloat("_SunSizeConvergence", 5f);
            sky.SetFloat("_AtmosphereThickness", 0.72f);
            sky.SetColor("_SkyTint", new Color(0.32f, 0.60f, 0.86f));
            sky.SetColor("_GroundColor", new Color(0.48f, 0.55f, 0.62f));
            sky.SetFloat("_Exposure", 1.28f);
            RenderSettings.skybox = sky;
            RenderSettings.ambientMode = AmbientMode.Skybox;
            RenderSettings.ambientIntensity = 1.15f;
            RenderSettings.reflectionIntensity = 1f;
            RenderSettings.fog = false;

            var lightObject = new GameObject("Sun Directional Light");
            lightObject.transform.rotation = new Quaternion(0.12674598f, 0.27890947f, -0.037164636f, 0.9511907f);
            Light light = lightObject.AddComponent<Light>();
            light.type = LightType.Directional;
            light.color = new Color(0.91764706f, 0.785153f, 0.6448997f);
            light.intensity = 1.869325f;
            light.shadows = LightShadows.Soft;
        }

        private static void CreateTitleEnvironment()
        {
            EnsureDirectory(TitleGeneratedMaterialRoot.TrimEnd('/'));
            Material[] bridgeMaterials = CreateBridgeMaterials();
            GameObject bridge = CreateMeshObject("Gwangan Bridge", MeshAt("Object427_0.asset"), bridgeMaterials, null);
            bridge.transform.position = new Vector3(193.14f, 26f, 333.71f);
            bridge.transform.rotation = new Quaternion(-0.70506066f, 0.053754278f, 0.053754278f, 0.70506066f);
            bridge.transform.localScale = Vector3.one * 226.04634f;

            GameObject ocean = GameObject.CreatePrimitive(PrimitiveType.Plane);
            ocean.name = "ocean";
            UnityEngine.Object.DestroyImmediate(ocean.GetComponent<Collider>());
            ocean.transform.position = Vector3.zero;
            ocean.transform.localScale = new Vector3(200f, 1f, 200f);
            Material water = GetOrCreateMaterial("TitleWater", Shader.Find("FishingGame/TitleWater"));
            water.SetTexture("_NormalMap", TextureAt("WaveNormals.png"));
            water.SetTexture("_FoamTex", TextureAt("foam.png"));
            water.SetColor("_DeepColor", new Color(0.08f, 0.30f, 0.58f));
            water.SetColor("_SurfaceColor", new Color(0.18f, 0.58f, 0.82f));
            water.SetColor("_SkyColor", new Color(0.72f, 0.88f, 1f));
            water.SetFloat("_Smoothness", 0.74f);
            water.SetFloat("_NormalStrength", 0.514f);
            water.SetFloat("_WaveSpeed", 1.15f);
            water.SetFloat("_HorizontalChop", 0.7f);
            ocean.GetComponent<MeshRenderer>().sharedMaterial = water;

            CreateTitleShips();
        }

        private static void CreateTitleShips()
        {
            var ships = new GameObject("Ships");
            Material yachtInterior = CreateStandardMaterial("YachtInterior", "Yacht_PBR_Interior_Albedo.png", "Yacht_PBR_Normal.png", "Yacht_PBR_Interior_Metallic.png");
            Material yachtGlassInterior1 = CreateGlassMaterial("YachtGlassInterior1", new Color(0.62f, 0.82f, 0.92f, 0.34f));
            Material yachtGlassInterior2 = CreateGlassMaterial("YachtGlassInterior2", new Color(0.52f, 0.76f, 0.9f, 0.28f));
            Material yachtExterior = CreateStandardMaterial("YachtExterior", "Yacht_PBR_Exterior_Albedo.png", "Yacht_PBR_Normal.png", "Yacht_PBR_Exterior_Metallic.png");
            Material yachtGlassExterior = CreateGlassMaterial("YachtGlassExterior", new Color(0.45f, 0.72f, 0.9f, 0.3f));
            Material[] yachtMaterials = { yachtInterior, yachtGlassInterior1, yachtGlassInterior2, yachtExterior, yachtGlassExterior };

            Vector3[] yachtPath =
            {
                new Vector3(86.02975f, 0f, 45.932545f), new Vector3(85.1f, 0f, 44.1f),
                new Vector3(71.93639f, 0.37373397f, 58.60563f), new Vector3(48.732887f, 0.08305197f, 74.88382f),
                new Vector3(20.761473f, 0f, 60.025272f), new Vector3(0.48549652f, 0f, 65.05113f),
                new Vector3(-25.134354f, 0f, 55.04867f), new Vector3(-48.01833f, 0f, 33.432434f),
                new Vector3(-35.573387f, 0f, 6.4971514f), new Vector3(-3.512126f, 0f, -2.4710615f),
                new Vector3(31.020163f, 0f, 4.3901916f), new Vector3(50.550083f, 0f, 33.027325f)
            };
            CreateYacht("yacht", ships.transform, yachtPath, yachtMaterials);

            Vector3[] yachtPath2 =
            {
                new Vector3(178.4f, 0f, 48.7f), new Vector3(88.9f, -1.4f, 81.5f),
                new Vector3(112.52072f, -1.2358797f, 126.61728f), new Vector3(77.6123f, -1.4f, 141.44955f),
                new Vector3(49.656464f, 1.9323479f, 121.60581f), new Vector3(80.33897f, -1.4f, 104.75801f),
                new Vector3(100.336624f, -1.4f, 129.00024f), new Vector3(51.409004f, -1.4f, 157.12097f),
                new Vector3(-13.598373f, -1.4f, 155.52061f), new Vector3(-26.470184f, -1.4f, 175.35977f),
                new Vector3(-12.942001f, -1.4f, 201.21443f), new Vector3(-42.956154f, -1.4f, 216.24826f),
                new Vector3(-69.31821f, -1.4f, 223.18964f), new Vector3(-51.957016f, -1.4f, 206.3981f),
                new Vector3(-54.28965f, -1.4f, 186.47067f), new Vector3(-1.3009129f, -1.4f, 186.51886f),
                new Vector3(20.772766f, -1.4f, 138.75443f), new Vector3(38.78747f, -1.4f, 130.22028f),
                new Vector3(90.03322f, -1.4f, 145.61313f), new Vector3(124.7611f, -1.4f, 121.169334f),
                new Vector3(109.12894f, -1.4f, 76.6598f), new Vector3(150.15536f, -1.4f, 42.509216f)
            };
            CreateYacht("yacht (1)", ships.transform, yachtPath2, yachtMaterials);

            Material cruiseMaterial = CreateStandardMaterial("Cruiseship", "Cruiseship_d.png", "Cruiseship_n.png", "Cruiseship_s.png");
            Vector3[] cruisePath =
            {
                new Vector3(944f, 4f, 864f), new Vector3(934.5083f, -1.4f, 869.53357f),
                new Vector3(478.97864f, -1.4980991f, 823.4402f), new Vector3(-119.40802f, -1.4834175f, 761.8942f),
                new Vector3(-718.96436f, -1.4420425f, 703.7671f), new Vector3(-1180.634f, -1.4f, 675.6229f),
                new Vector3(-682.9725f, -1.4f, 998.396f), new Vector3(216.27267f, -1.3999999f, 964.1922f),
                new Vector3(1116.5083f, -1.4f, 928.53357f)
            };
            GameObject cruise = CreateMeshObject("Cruiseship_RotatingRadars", MeshAt("Cruiseship.asset"), new[] { cruiseMaterial }, ships.transform);
            cruise.transform.position = cruisePath[0];
            cruise.transform.rotation = new Quaternion(1.5827206e-08f, -0.68962306f, -1.507219e-08f, 0.72416854f);
            cruise.transform.localScale = Vector3.one * 1.2917f;
            GameObject cruiseBody = CreateMeshObject("Cruiseship_WithoutRadars", MeshAt("Cruiseship_WithoutRadars.asset"), new[] { cruiseMaterial }, cruise.transform);
            cruiseBody.transform.localPosition = Vector3.zero;
            cruiseBody.transform.localRotation = Quaternion.identity;
            cruise.AddComponent<TitleShipMotion>().Configure(cruisePath, 8f, true);
        }

        private static void CreateYacht(string name, Transform parent, Vector3[] path, Material[] materials)
        {
            var root = new GameObject(name);
            root.transform.SetParent(parent, false);
            root.transform.position = path[0];
            root.transform.rotation = Quaternion.identity;
            GameObject body = CreateMeshObject("Yacht_PBR", MeshAt("Yacht_PBR.asset"), materials, root.transform);
            body.transform.localPosition = new Vector3(-0.6121979f, 1.8f, -0.306015f);
            body.transform.localRotation = new Quaternion(0f, 0.7071068f, 0f, 0.7071068f);
            body.transform.localScale = Vector3.one;
            root.AddComponent<TitleShipMotion>().Configure(path, 10f, false);
        }

        private static void CreateTitleInterface(Canvas canvas)
        {
            Image titleLogo = CreateFullImage("titleLogo", canvas.transform, TitleSpriteAt("strong-fisher-1.asset"), Color.white);
            titleLogo.preserveAspect = true;
            titleLogo.rectTransform.localScale = Vector3.one * 0.6682538f;

            Image symbol = CreateImage("symbol", canvas.transform, TitleSpriteAt("intro-fishing-family-logo.asset"),
                new Vector2(0f, 1f), new Vector2(0f, 1f), new Vector2(25f, -25f), new Vector2(2048f, 169f), Color.white, true);
            symbol.rectTransform.pivot = new Vector2(0f, 1f);
            symbol.rectTransform.localScale = Vector3.one * 0.23545343f;

            Image card = CreateImage("Card", canvas.transform, TitleSpriteAt("obj_arrowBox.asset"),
                new Vector2(0f, 0f), new Vector2(1f, 0f), new Vector2(0f, 20f), new Vector2(0f, 100f), Color.white, false);
            card.rectTransform.pivot = new Vector2(0.5f, 0f);
            Text payment = CreateText("payment_text", card.transform,
                "카드 단말기에 카드를 꽂거나 터치하게 되면 결제 후 게임이 시작 됩니다.", 40, FontStyle.Bold,
                TextAnchor.MiddleCenter, new Color(0.88f, 1f, 0.08f), Vector2.zero, Vector2.one, Vector2.zero, Vector2.zero);
            payment.font = TitleFont();
            AddOutline(payment.gameObject, Color.black, new Vector2(2f, -2f));

            Image battery = CreateImage("battery", canvas.transform, TitleSpriteAt("b_r.asset"),
                Vector2.one, Vector2.one, new Vector2(-5f, -9.5f), new Vector2(421f, 245f), Color.white, true);
            battery.rectTransform.pivot = Vector2.one;
            battery.rectTransform.localScale = Vector3.one * 0.29297724f;

            Text tension = CreateText("power_lv", canvas.transform, "장력 : 3", 25, FontStyle.Bold, TextAnchor.LowerLeft,
                Color.white, Vector2.zero, Vector2.zero, new Vector2(10f, 5f), new Vector2(200f, 31f));
            tension.rectTransform.pivot = Vector2.zero;
            tension.font = TitleFont();
            AddOutline(tension.gameObject, Color.black, new Vector2(1.5f, -1.5f));

            Text version = CreateText("clinet_version", canvas.transform, "L 1.3.5", 20, FontStyle.Bold, TextAnchor.LowerRight,
                Color.white, Vector2.zero, Vector2.zero, Vector2.zero, new Vector2(1920f, 50f));
            version.rectTransform.pivot = Vector2.zero;
            version.font = TitleFont();
            AddOutline(version.gameObject, new Color(0f, 0f, 0f, 0.7f), new Vector2(1f, -1f));

            Sprite[] batteries =
            {
                TitleSpriteAt("b_r.asset"), TitleSpriteAt("b_y.asset"), TitleSpriteAt("b_g_4.asset"),
                TitleSpriteAt("b_g_3.asset"), TitleSpriteAt("b_g_2.asset"), TitleSpriteAt("b_g_1.asset"), TitleSpriteAt("b_b.asset")
            };
            TitleStatusView status = new GameObject("TitleStatusView").AddComponent<TitleStatusView>();
            status.Configure(payment, tension, version, battery, batteries);
        }

        private static void CreateTitleIdleLoop(Canvas canvas, AudioSource titleBgm)
        {
            GameObject rankRoot = new GameObject("Rank", typeof(RectTransform));
            rankRoot.transform.SetParent(canvas.transform, false);
            Stretch(rankRoot.GetComponent<RectTransform>());

            CreateFullImage("bg", rankRoot.transform, null, new Color(0.33962262f, 0.33962262f, 0.33962262f, 1f));

            Image sub = CreateFullImage("sub", rankRoot.transform, TitleSpriteAt("strong-fisher-1.asset"), Color.white);
            sub.rectTransform.localScale = Vector3.one * 0.6682538f;

            CreateImage("line", rankRoot.transform, TitleIdleSpriteAt("ui_rankLine.asset"),
                new Vector2(0f, 1f), new Vector2(1f, 1f), new Vector2(0f, -100.3999f), new Vector2(0f, 100f), Color.white, false)
                .rectTransform.pivot = new Vector2(0.5f, 1f);

            CreateImage("title", rankRoot.transform, TitleIdleSpriteAt("ui_txtRank.asset"),
                new Vector2(0.5f, 1f), new Vector2(0.5f, 1f), new Vector2(0f, -54.629883f), new Vector2(763.5f, 107.6f), Color.white, false)
                .rectTransform.pivot = new Vector2(0.5f, 1f);

            Image view = CreateImage("view", rankRoot.transform, null,
                new Vector2(0f, 1f), new Vector2(0f, 1f), new Vector2(211.55005f, -229.46002f), new Vector2(1526f, 775f),
                new Color(0.18867922f, 0.18867922f, 0.18867922f, 0.85882354f), false);
            view.rectTransform.pivot = new Vector2(0f, 1f);

            Image viewport = CreateImage("Viewport", view.transform, null, Vector2.zero, Vector2.one, Vector2.zero,
                new Vector2(-17f, 0f), Color.white, false);
            viewport.rectTransform.pivot = new Vector2(0f, 1f);
            Mask mask = viewport.gameObject.AddComponent<Mask>();
            mask.showMaskGraphic = false;

            var contentObject = new GameObject("Content", typeof(RectTransform), typeof(VerticalLayoutGroup));
            contentObject.transform.SetParent(viewport.transform, false);
            RectTransform content = contentObject.GetComponent<RectTransform>();
            content.anchorMin = new Vector2(0.5f, 1f);
            content.anchorMax = new Vector2(0.5f, 1f);
            content.pivot = new Vector2(0.5f, 1f);
            content.anchoredPosition = Vector2.zero;
            content.sizeDelta = new Vector2(1508f, 300f);
            VerticalLayoutGroup layout = contentObject.GetComponent<VerticalLayoutGroup>();
            layout.padding = new RectOffset(10, 0, 0, 0);
            layout.spacing = 23f;
            layout.childAlignment = TextAnchor.UpperLeft;
            layout.childControlWidth = false;
            layout.childControlHeight = false;
            layout.childForceExpandWidth = false;
            layout.childForceExpandHeight = false;

            Sprite[] characters =
            {
                TitleIdleSpriteAt("ui_char1.asset"), TitleIdleSpriteAt("ui_char2.asset"), TitleIdleSpriteAt("ui_char3.asset"),
                TitleIdleSpriteAt("ui_char4.asset"), TitleIdleSpriteAt("ui_char5.asset"), TitleIdleSpriteAt("ui_char6.asset"),
                TitleIdleSpriteAt("ui_char7.asset"), TitleIdleSpriteAt("ui_char8.asset"), TitleIdleSpriteAt("ui_char9.asset")
            };
            var rows = new TitleRankRowView[30];
            for (int index = 0; index < rows.Length; index++)
            {
                rows[index] = CreateTitleRankRow(content, index + 1, characters[index % characters.Length]);
            }

            rankRoot.SetActive(false);

            var supportObject = new GameObject("GameSupport", typeof(RectTransform), typeof(AudioSource), typeof(TitleIdleLoop));
            supportObject.transform.SetParent(canvas.transform, false);
            Stretch(supportObject.GetComponent<RectTransform>());
            AudioSource externalAudio = supportObject.GetComponent<AudioSource>();
            externalAudio.playOnAwake = false;
            externalAudio.loop = false;

            string[] guidanceNames = { "Hold", "Hook", "Store", "Point", "Mystic", "WaitAndHook", "LRCont", "Lv" };
            string[] guidanceAssets = { "Hold.asset", "Hook.asset", "Store.asset", "point.asset", "mystic.asset", "WaitAndHook.asset", "LRControl.asset", "lv.asset" };
            var guidance = new GameObject[guidanceNames.Length];
            for (int index = 0; index < guidance.Length; index++)
            {
                guidance[index] = CreateFullImage(guidanceNames[index], supportObject.transform,
                    TitleIdleSpriteAt(guidanceAssets[index]), Color.white).gameObject;
                guidance[index].SetActive(false);
            }

            var introObject = new GameObject("intro", typeof(RectTransform), typeof(CanvasRenderer), typeof(RawImage), typeof(VideoPlayer));
            introObject.transform.SetParent(supportObject.transform, false);
            Stretch(introObject.GetComponent<RectTransform>());
            RawImage introRaw = introObject.GetComponent<RawImage>();
            RenderTexture introTexture = TitleIntroRenderTexture();
            introRaw.texture = introTexture;
            introRaw.color = Color.clear;
            introRaw.raycastTarget = false;
            VideoPlayer introPlayer = introObject.GetComponent<VideoPlayer>();
            introPlayer.playOnAwake = false;
            introPlayer.isLooping = false;
            introPlayer.waitForFirstFrame = false;
            introPlayer.skipOnDrop = false;
            introPlayer.renderMode = VideoRenderMode.RenderTexture;
            introPlayer.targetTexture = introTexture;
            introPlayer.clip = TitleVideoAt("fishing.webm");
            introPlayer.audioOutputMode = VideoAudioOutputMode.Direct;

            AudioSource loadingBgm = AddLoopingAudio("Title Idle Loading BGM", AudioAt("bgm_loading.ogg"), 0.32f);
            loadingBgm.playOnAwake = false;

            TitleIdleLoop idleLoop = supportObject.GetComponent<TitleIdleLoop>();
            idleLoop.Configure(rankRoot, content, rows, characters, guidance, introRaw, introPlayer,
                TitleVideoAt("fishing.webm"), TitleAudioAt("Intro_bg.ogg"), externalAudio, titleBgm, loadingBgm);
        }

        private static TitleRankRowView CreateTitleRankRow(RectTransform content, int rank, Sprite characterSprite)
        {
            Image background = CreateImage("rank_bg_" + rank, content, TitleIdleSpriteAt("ui_rankTxtBg.asset"),
                Vector2.zero, Vector2.zero, Vector2.zero, new Vector2(1506f, 74f), Color.white, false);
            background.rectTransform.pivot = new Vector2(0.5f, 0.5f);
            Mask rowMask = background.gameObject.AddComponent<Mask>();
            rowMask.showMaskGraphic = true;
            LayoutElement layout = background.gameObject.AddComponent<LayoutElement>();
            layout.preferredWidth = 1506f;
            layout.preferredHeight = 74f;

            Text rankText = CreateText("rank", background.transform, rank.ToString(), 95, FontStyle.Normal, TextAnchor.MiddleCenter,
                Color.white, new Vector2(0f, 0f), new Vector2(0f, 1f), new Vector2(10f, 0f), new Vector2(160f, 0f));
            rankText.rectTransform.pivot = new Vector2(0f, 0.5f);
            rankText.rectTransform.localScale = new Vector3(0.9f, 1f, 1f);

            Image character = CreateImage("character", background.transform, characterSprite,
                new Vector2(0f, 0.5f), new Vector2(0f, 0.5f), new Vector2(175f, 6.4199524f), new Vector2(86f, 92f), Color.white, true);
            character.rectTransform.pivot = new Vector2(0f, 0.5f);

            Text score = CreateText("Score", background.transform, "10,000", 93, FontStyle.Normal, TextAnchor.MiddleCenter,
                Color.white, new Vector2(0.5f, 0.5f), new Vector2(0.5f, 0.5f), Vector2.zero, new Vector2(651.2f, 74f));
            Text date = CreateText("date", background.transform, "2024.06.20", 42, FontStyle.Normal, TextAnchor.MiddleRight,
                Color.white, new Vector2(1f, 0.5f), new Vector2(1f, 0.5f), new Vector2(-87.79999f, 0f), new Vector2(250.4f, 74f));
            date.rectTransform.pivot = new Vector2(1f, 0.5f);
            Text playerName = CreateText("name", background.transform, "ABC", 65, FontStyle.Normal, TextAnchor.MiddleCenter,
                Color.white, new Vector2(0f, 0.5f), new Vector2(0f, 0.5f), new Vector2(323.9f, 0f), new Vector2(214.83f, 74f));
            playerName.rectTransform.pivot = new Vector2(0f, 0.5f);
            playerName.rectTransform.localScale = Vector3.one * 1.0054555f;

            Font font = TitleFont();
            rankText.font = font;
            score.font = font;
            date.font = font;
            playerName.font = font;

            TitleRankRowView row = background.gameObject.AddComponent<TitleRankRowView>();
            row.Configure(rankText, character, score, date, playerName);
            return row;
        }

        private static GameObject CreateMeshObject(string name, Mesh mesh, Material[] materials, Transform parent)
        {
            var target = new GameObject(name, typeof(MeshFilter), typeof(MeshRenderer));
            if (parent != null)
            {
                target.transform.SetParent(parent, false);
            }
            target.GetComponent<MeshFilter>().sharedMesh = mesh;
            target.GetComponent<MeshRenderer>().sharedMaterials = materials;
            return target;
        }

        private static Material[] CreateBridgeMaterials()
        {
            return new[]
            {
                CreateBridgeFallbackMaterial("BridgeFloor", "floor_low_floor_BaseMap_0.png"),
                CreateBridgeFallbackMaterial("BridgeBody", "bo_low_07___Default_AlbedoTransparency_0.png"),
                CreateBridgeFallbackMaterial("BridgeLine", "line_low_wire_008008136_AlbedoTransparency_0.png"),
                CreateBridgeFallbackMaterial("BridgeRoad", "Road_low_Road_AlbedoTransparency_0.png"),
                CreateBridgeFallbackMaterial("BridgeRebar", "rebarH_low_rebar_AlbedoTransparency_0.png"),
                CreateBridgeFallbackMaterial("BridgeLowRoad", "low_road_name_AlbedoTransparency_0.png")
            };
        }

        private static Material CreateBridgeFallbackMaterial(string name, string albedo)
        {
            Material material = GetOrCreateMaterial(name, Shader.Find("Standard"));
            material.SetTexture("_MainTex", TextureAt(albedo));
            material.SetTexture("_BumpMap", null);
            material.SetTexture("_MetallicGlossMap", null);
            material.SetTexture("_SpecGlossMap", null);
            material.DisableKeyword("_NORMALMAP");
            material.DisableKeyword("_METALLICGLOSSMAP");
            material.DisableKeyword("_SPECGLOSSMAP");
            material.SetFloat("_Metallic", 0f);
            material.SetFloat("_Glossiness", 0.16f);
            material.color = Color.white;
            return material;
        }

        private static Material CreateStandardMaterial(string name, string albedo, string normal, string surfaceMap)
        {
            bool isMetallicMap = surfaceMap.IndexOf("Metallic", StringComparison.OrdinalIgnoreCase) >= 0;
            bool isSpecularMap = surfaceMap.IndexOf("Specular", StringComparison.OrdinalIgnoreCase) >= 0 ||
                                 surfaceMap.EndsWith("_s.png", StringComparison.OrdinalIgnoreCase);
            Shader shader = Shader.Find(isSpecularMap ? "Standard (Specular setup)" : "Standard");
            Material material = GetOrCreateMaterial(name, shader);
            material.SetTexture("_MainTex", TextureAt(albedo));
            material.SetTexture("_BumpMap", TextureAt(normal));
            material.EnableKeyword("_NORMALMAP");

            material.SetTexture("_MetallicGlossMap", null);
            material.SetTexture("_SpecGlossMap", null);
            material.DisableKeyword("_METALLICGLOSSMAP");
            material.DisableKeyword("_SPECGLOSSMAP");
            material.SetFloat("_Metallic", 0f);
            material.SetColor("_SpecColor", new Color(0.2f, 0.2f, 0.2f, 1f));

            if (isMetallicMap)
            {
                material.SetTexture("_MetallicGlossMap", TextureAt(surfaceMap));
                material.EnableKeyword("_METALLICGLOSSMAP");
                material.SetFloat("_Metallic", 0.15f);
            }
            else if (isSpecularMap)
            {
                material.SetTexture("_SpecGlossMap", TextureAt(surfaceMap));
                material.EnableKeyword("_SPECGLOSSMAP");
            }

            // AssetRipper exported the bridge floor's URP MaskMap. Feeding it to
            // the Built-in metallic slot produces a false pipe/checker pattern,
            // so it is intentionally omitted until the original URP shader is available.
            material.SetFloat("_Glossiness", isSpecularMap ? 0.5f : 0.42f);
            return material;
        }

        private static Material CreateGlassMaterial(string name, Color color)
        {
            Material material = GetOrCreateMaterial(name, Shader.Find("Standard"));
            material.color = color;
            material.SetFloat("_Mode", 3f);
            material.SetInt("_SrcBlend", (int)BlendMode.SrcAlpha);
            material.SetInt("_DstBlend", (int)BlendMode.OneMinusSrcAlpha);
            material.SetInt("_ZWrite", 0);
            material.DisableKeyword("_ALPHATEST_ON");
            material.EnableKeyword("_ALPHABLEND_ON");
            material.DisableKeyword("_ALPHAPREMULTIPLY_ON");
            material.renderQueue = 3000;
            material.SetFloat("_Glossiness", 0.9f);
            return material;
        }

        private static Material GetOrCreateMaterial(string name, Shader shader)
        {
            if (shader == null)
            {
                throw new InvalidOperationException("Required shader was not found for Title material: " + name);
            }

            EnsureDirectory(TitleGeneratedMaterialRoot.TrimEnd('/'));
            string path = TitleGeneratedMaterialRoot + name + ".mat";
            Material material = AssetDatabase.LoadAssetAtPath<Material>(path);
            if (material == null)
            {
                material = new Material(shader) { name = name };
                AssetDatabase.CreateAsset(material, path);
            }
            else
            {
                material.shader = shader;
            }

            EditorUtility.SetDirty(material);
            return material;
        }

        private static Mesh MeshAt(string fileName)
        {
            Mesh mesh = AssetDatabase.LoadAssetAtPath<Mesh>(TitleMeshRoot + fileName);
            if (mesh == null)
            {
                throw new InvalidOperationException("Title Mesh was not imported: " + fileName);
            }
            return mesh;
        }

        private static Texture2D TextureAt(string fileName)
        {
            Texture2D texture = AssetDatabase.LoadAssetAtPath<Texture2D>(TitleTextureRoot + fileName);
            if (texture == null)
            {
                throw new InvalidOperationException("Title Texture was not imported: " + fileName);
            }
            return texture;
        }

        private static Sprite TitleSpriteAt(string fileName)
        {
            Sprite sprite = AssetDatabase.LoadAssetAtPath<Sprite>(TitleSpriteRoot + fileName);
            if (sprite == null)
            {
                throw new InvalidOperationException("Title Sprite was not imported: " + fileName);
            }
            return sprite;
        }

        private static Sprite TitleIdleSpriteAt(string fileName)
        {
            Sprite sprite = AssetDatabase.LoadAssetAtPath<Sprite>(TitleIdleSpriteRoot + fileName);
            if (sprite == null)
            {
                throw new InvalidOperationException("Title idle Sprite was not imported: " + fileName);
            }
            return sprite;
        }

        private static VideoClip TitleVideoAt(string fileName)
        {
            VideoClip clip = AssetDatabase.LoadAssetAtPath<VideoClip>(TitleVideoRoot + fileName);
            if (clip == null)
            {
                throw new InvalidOperationException("Title VideoClip was not imported: " + fileName);
            }
            return clip;
        }

        private static RenderTexture TitleIntroRenderTexture()
        {
            const string path = TitleIdleRenderTextureRoot + "intro_render_texture.renderTexture";
            RenderTexture texture = AssetDatabase.LoadAssetAtPath<RenderTexture>(path);
            if (texture == null)
            {
                throw new InvalidOperationException("Original Title intro RenderTexture was not imported.");
            }
            return texture;
        }

        private static AudioClip TitleAudioAt(string fileName)
        {
            AudioClip clip = AssetDatabase.LoadAssetAtPath<AudioClip>(TitleAudioRoot + fileName);
            if (clip == null)
            {
                throw new InvalidOperationException("Title AudioClip was not imported: " + fileName);
            }
            return clip;
        }

        private static Font TitleFont()
        {
            Font font = AssetDatabase.LoadAssetAtPath<Font>(FontRoot + "SCDream5.otf");
            if (font == null)
            {
                throw new InvalidOperationException("Original Title font was not imported: SCDream5.otf");
            }
            return font;
        }

        private static Sprite LoadingBackgroundSprite()
        {
            const string path = LoadingArtRoot + "Background/bg_gameDiffi _2.asset";
            Sprite sprite = AssetDatabase.LoadAssetAtPath<Sprite>(path);
            if (sprite == null)
            {
                throw new InvalidOperationException("Original Loading background Sprite was not imported.");
            }
            return sprite;
        }

        private static SkeletonDataAsset LoadingSkeletonData()
        {
            const string path = LoadingSpineRoot + "Data/loading_SkeletonData.asset";
            SkeletonDataAsset data = AssetDatabase.LoadAssetAtPath<SkeletonDataAsset>(path);
            if (data == null)
            {
                throw new InvalidOperationException("Original Loading SkeletonData was not imported.");
            }
            return data;
        }

        private static Material LoadingMaterialAt(string fileName)
        {
            Material material = AssetDatabase.LoadAssetAtPath<Material>(LoadingSpineRoot + "Materials/" + fileName);
            if (material == null)
            {
                throw new InvalidOperationException("Original Loading Spine material was not imported: " + fileName);
            }
            return material;
        }

        private static Canvas CreateCanvas()
        {
            var canvasObject = new GameObject("Canvas", typeof(RectTransform), typeof(Canvas), typeof(CanvasScaler), typeof(GraphicRaycaster));
            Canvas canvas = canvasObject.GetComponent<Canvas>();
            canvas.renderMode = RenderMode.ScreenSpaceOverlay;
            CanvasScaler scaler = canvasObject.GetComponent<CanvasScaler>();
            scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;
            scaler.referenceResolution = new Vector2(1920f, 1080f);
            scaler.screenMatchMode = CanvasScaler.ScreenMatchMode.MatchWidthOrHeight;
            scaler.matchWidthOrHeight = 0.5f;

            var eventObject = new GameObject("EventSystem", typeof(EventSystem), typeof(StandaloneInputModule));
            eventObject.transform.SetParent(canvas.transform.parent);
            return canvas;
        }

        private static Image CreateFullImage(string name, Transform parent, Sprite sprite, Color color)
        {
            return CreateImage(name, parent, sprite, Vector2.zero, Vector2.one, Vector2.zero, Vector2.zero, color, false);
        }

        private static void Stretch(RectTransform rect)
        {
            rect.anchorMin = Vector2.zero;
            rect.anchorMax = Vector2.one;
            rect.anchoredPosition = Vector2.zero;
            rect.sizeDelta = Vector2.zero;
            rect.pivot = new Vector2(0.5f, 0.5f);
        }

        private static Image CreateImage(string name, Transform parent, Sprite sprite, Vector2 anchorMin, Vector2 anchorMax,
            Vector2 position, Vector2 size, Color color, bool preserveAspect)
        {
            var target = new GameObject(name, typeof(RectTransform), typeof(CanvasRenderer), typeof(Image));
            target.transform.SetParent(parent, false);
            RectTransform rect = target.GetComponent<RectTransform>();
            rect.anchorMin = anchorMin;
            rect.anchorMax = anchorMax;
            rect.anchoredPosition = position;
            rect.sizeDelta = size;
            Image image = target.GetComponent<Image>();
            image.sprite = sprite;
            image.color = color;
            image.preserveAspect = preserveAspect;
            image.raycastTarget = false;
            return image;
        }

        private static Text CreateText(string name, Transform parent, string value, int fontSize, FontStyle style,
            TextAnchor alignment, Color color, Vector2 anchorMin, Vector2 anchorMax, Vector2 position, Vector2 size)
        {
            var target = new GameObject(name, typeof(RectTransform), typeof(CanvasRenderer), typeof(Text));
            target.transform.SetParent(parent, false);
            RectTransform rect = target.GetComponent<RectTransform>();
            rect.anchorMin = anchorMin;
            rect.anchorMax = anchorMax;
            rect.anchoredPosition = position;
            rect.sizeDelta = size;
            Text text = target.GetComponent<Text>();
            text.font = Resources.GetBuiltinResource<Font>("LegacyRuntime.ttf");
            text.text = value;
            text.fontSize = fontSize;
            text.fontStyle = style;
            text.alignment = alignment;
            text.color = color;
            text.raycastTarget = false;
            text.horizontalOverflow = HorizontalWrapMode.Overflow;
            text.verticalOverflow = VerticalWrapMode.Overflow;
            return text;
        }

        private static Text CreateSelectText(string name, Transform parent, string value, int fontSize,
            TextAnchor alignment, Color color, Vector2 anchorMin, Vector2 anchorMax, Vector2 position, Vector2 size)
        {
            Text text = CreateText(name, parent, value, fontSize, FontStyle.Normal, alignment, color,
                anchorMin, anchorMax, position, size);
            Font originalFont = AssetDatabase.LoadAssetAtPath<Font>(FontRoot + "SCDream5.otf");
            if (originalFont == null)
            {
                throw new InvalidOperationException("Original SCDream5 font was not imported.");
            }
            text.font = originalFont;
            return text;
        }

        private static Button CreateButton(string name, Transform parent, string label, Vector2 anchor, Vector2 size)
        {
            var target = new GameObject(name, typeof(RectTransform), typeof(CanvasRenderer), typeof(Image), typeof(Button));
            target.transform.SetParent(parent, false);
            RectTransform rect = target.GetComponent<RectTransform>();
            rect.anchorMin = anchor;
            rect.anchorMax = anchor;
            rect.anchoredPosition = Vector2.zero;
            rect.sizeDelta = size;
            Image image = target.GetComponent<Image>();
            image.color = new Color(0.08f, 0.26f, 0.52f, 0.95f);
            AddOutline(target, new Color(0.25f, 0.9f, 1f, 0.85f), new Vector2(2f, -2f));
            Button button = target.GetComponent<Button>();
            ColorBlock colors = button.colors;
            colors.normalColor = Color.white;
            colors.highlightedColor = new Color(0.72f, 0.95f, 1f);
            colors.pressedColor = new Color(0.45f, 0.75f, 1f);
            button.colors = colors;
            CreateText("Label", target.transform, label, 25, FontStyle.Bold, TextAnchor.MiddleCenter, Color.white,
                Vector2.zero, Vector2.one, Vector2.zero, Vector2.zero);
            return button;
        }

        private static Button CreateSpriteButton(string name, Transform parent, Sprite sprite, string label, Vector2 anchor, Vector2 size)
        {
            Button button = CreateButton(name, parent, string.Empty, anchor, size);
            Image image = button.GetComponent<Image>();
            image.sprite = sprite;
            image.color = Color.white;
            image.preserveAspect = true;
            CreateText("KeyLabel", button.transform, label, 17, FontStyle.Bold, TextAnchor.MiddleCenter, Color.white,
                new Vector2(0.5f, -0.12f), new Vector2(0.5f, -0.12f), Vector2.zero, new Vector2(180f, 34f));
            return button;
        }

        private static void AddOutline(GameObject target, Color color, Vector2 distance)
        {
            Outline outline = target.AddComponent<Outline>();
            outline.effectColor = color;
            outline.effectDistance = distance;
        }

        private static AudioSource AddLoopingAudio(string name, AudioClip clip, float volume)
        {
            var audioObject = new GameObject(name, typeof(AudioSource));
            AudioSource source = audioObject.GetComponent<AudioSource>();
            source.clip = clip;
            source.loop = true;
            source.playOnAwake = clip != null;
            source.volume = volume;
            return source;
        }

        private static Sprite SpriteAt(string fileName)
        {
            Sprite sprite = AssetDatabase.LoadAssetAtPath<Sprite>(UiRoot + fileName);
            if (sprite == null)
            {
                throw new InvalidOperationException("Sprite was not imported: " + fileName);
            }
            return sprite;
        }

        private static Sprite SelectionSpriteAt(string relativePath)
        {
            Sprite sprite = AssetDatabase.LoadAssetAtPath<Sprite>(SelectionArtRoot + relativePath);
            if (sprite == null)
            {
                throw new InvalidOperationException("Original SelectScene Sprite was not imported: " + relativePath);
            }
            return sprite;
        }

        private static AudioClip AudioAt(string fileName)
        {
            AudioClip clip = AssetDatabase.LoadAssetAtPath<AudioClip>(AudioRoot + fileName);
            if (clip == null)
            {
                throw new InvalidOperationException("AudioClip was not imported: " + fileName);
            }
            return clip;
        }

        private static void CreateCamera(Color background)
        {
            var cameraObject = new GameObject("Main Camera");
            cameraObject.tag = "MainCamera";
            Camera camera = cameraObject.AddComponent<Camera>();
            camera.clearFlags = CameraClearFlags.SolidColor;
            camera.backgroundColor = background;
            cameraObject.AddComponent<AudioListener>();
        }

        private static void UpdateBuildSettings()
        {
            string[] scenePaths = { TitlePath, LoadingPath, SelectPath, BootstrapPath };
            var scenes = scenePaths.Select(CreateBuildSettingsScene).ToList();
            scenes.AddRange(EditorBuildSettings.scenes.Where(scene =>
                !scenePaths.Contains(scene.path) && scene.path != "Assets/Scenes/Menu/LastScene.unity"));
            EditorBuildSettings.scenes = scenes.ToArray();
        }

        private static EditorBuildSettingsScene CreateBuildSettingsScene(string scenePath)
        {
            string guidText = AssetDatabase.AssetPathToGUID(scenePath);
            if (string.IsNullOrEmpty(guidText))
            {
                throw new InvalidOperationException("Scene GUID was not found: " + scenePath);
            }
            return new EditorBuildSettingsScene(new GUID(guidText), true);
        }

        private static void EnsureDirectory(string assetPath)
        {
            if (!string.IsNullOrEmpty(assetPath) && !Directory.Exists(assetPath))
            {
                Directory.CreateDirectory(assetPath);
            }
        }
    }
}
#endif
