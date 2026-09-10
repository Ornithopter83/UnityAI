using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Video;
using Debug = UnityEngine.Debug;
using Random = UnityEngine.Random;

namespace FishingGame.UI
{
    /// <summary>
    /// Evidence-backed Title attract loop from the original GameSupport component:
    /// wait -> 30 ranks -> two guidance images -> intro video -> Title.
    /// </summary>
    public sealed class TitleIdleLoop : MonoBehaviour
    {
        public enum Stage
        {
            Title,
            Ranking,
            Guidance,
            Video
        }

        [Header("Original timing")]
        [SerializeField] private float introWaitTime = 30f;
        [SerializeField] private float guidanceDuration = 3.5f;
        [SerializeField] private float rankPause = 3f;
        [SerializeField] private float rankMoveDuration = 1f;
        [SerializeField] private float rankFinalPause = 1.5f;

        [Header("Original hierarchy")]
        [SerializeField] private GameObject rankRoot;
        [SerializeField] private RectTransform rankContent;
        [SerializeField] private TitleRankRowView[] rankRows;
        [SerializeField] private Sprite[] characterSprites;
        [SerializeField] private GameObject[] guidanceImages;
        [SerializeField] private RawImage introRawImage;
        [SerializeField] private VideoPlayer introPlayer;
        [SerializeField] private VideoClip builtInIntro;
        [SerializeField] private AudioClip externalIntroAudio;
        [SerializeField] private AudioSource externalAudioSource;
        [SerializeField] private AudioSource titleBgm;
        [SerializeField] private AudioSource loadingBgm;

        private readonly List<int> remainingGuidance = new List<int>();
        private Coroutine loopCoroutine;
        private bool videoComplete;
        private bool verificationMode;

        public Stage CurrentStage { get; private set; } = Stage.Title;
        public int CompletedCycles { get; private set; }

        public void Configure(
            GameObject ranking,
            RectTransform content,
            TitleRankRowView[] rows,
            Sprite[] characters,
            GameObject[] guidance,
            RawImage rawImage,
            VideoPlayer player,
            VideoClip intro,
            AudioClip introAudio,
            AudioSource externalAudio,
            AudioSource titleMusic,
            AudioSource loadingMusic)
        {
            rankRoot = ranking;
            rankContent = content;
            rankRows = rows;
            characterSprites = characters;
            guidanceImages = guidance;
            introRawImage = rawImage;
            introPlayer = player;
            builtInIntro = intro;
            externalIntroAudio = introAudio;
            externalAudioSource = externalAudio;
            titleBgm = titleMusic;
            loadingBgm = loadingMusic;
        }

        private void Awake()
        {
            HideOverlays();
            if (introPlayer != null)
            {
                introPlayer.loopPointReached += OnVideoComplete;
            }
        }

        private void Start()
        {
            PopulateRankingFromOriginalPlayerPrefsContract();
            ConfigureIntroSource();
            loopCoroutine = StartCoroutine(RunLoop());
        }

        private void OnDestroy()
        {
            if (introPlayer != null)
            {
                introPlayer.loopPointReached -= OnVideoComplete;
                introPlayer.url = null;
            }

            if (externalAudioSource != null)
            {
                externalAudioSource.clip = null;
            }
        }

        public void StartVerificationCycle()
        {
            if (loopCoroutine != null)
            {
                StopCoroutine(loopCoroutine);
            }

            verificationMode = true;
            loopCoroutine = StartCoroutine(RunLoop());
        }

        private IEnumerator RunLoop()
        {
            while (true)
            {
                CurrentStage = Stage.Title;
                yield return new WaitForSeconds(verificationMode ? 0.05f : introWaitTime);
                yield return ShowRanking();
                yield return ShowGuidanceImages();
                yield return PlayIntroVideo();
                CompletedCycles++;

                if (verificationMode)
                {
                    verificationMode = false;
                    loopCoroutine = null;
                    yield break;
                }
            }
        }

        private IEnumerator ShowRanking()
        {
            CurrentStage = Stage.Ranking;
            rankContent.anchoredPosition = Vector2.zero;
            rankRoot.SetActive(true);

            float pause = verificationMode ? 0.05f : rankPause;
            float move = verificationMode ? 0.05f : rankMoveDuration;
            yield return new WaitForSeconds(pause);
            yield return MoveRankContent(765f, move);
            yield return new WaitForSeconds(pause);
            yield return MoveRankContent(1545f, move);
            yield return new WaitForSeconds(pause);
            yield return MoveRankContent(2120f, move);
            yield return new WaitForSeconds(pause);
            yield return MoveRankContent(0f, move);
            yield return new WaitForSeconds(verificationMode ? 0.05f : rankFinalPause);

            rankRoot.SetActive(false);
        }

        private IEnumerator MoveRankContent(float targetY, float duration)
        {
            Vector2 start = rankContent.anchoredPosition;
            Vector2 target = new Vector2(start.x, targetY);
            float elapsed = 0f;
            while (elapsed < duration)
            {
                yield return null;
                elapsed += Time.deltaTime;
                rankContent.anchoredPosition = Vector2.Lerp(start, target, Mathf.Clamp01(elapsed / duration));
            }
            rankContent.anchoredPosition = target;
        }

        private IEnumerator ShowGuidanceImages()
        {
            CurrentStage = Stage.Guidance;
            if (titleBgm != null)
            {
                titleBgm.Stop();
            }
            if (loadingBgm != null)
            {
                loadingBgm.Play();
            }

            if (remainingGuidance.Count <= 0)
            {
                for (int index = 0; index < guidanceImages.Length; index++)
                {
                    remainingGuidance.Add(index);
                }
            }

            for (int count = 0; count < 2; count++)
            {
                int selection = Random.Range(0, remainingGuidance.Count);
                int imageIndex = remainingGuidance[selection];
                remainingGuidance.RemoveAt(selection);
                guidanceImages[imageIndex].SetActive(true);
                yield return new WaitForSeconds(verificationMode ? 0.05f : guidanceDuration);
                guidanceImages[imageIndex].SetActive(false);
            }
        }

        private IEnumerator PlayIntroVideo()
        {
            CurrentStage = Stage.Video;
            if (loadingBgm != null)
            {
                loadingBgm.Stop();
            }

            introPlayer.frame = 0;
            videoComplete = false;
            yield return FadeRawImage(Color.clear, Color.white, verificationMode ? 0.05f : 0.5f);

            introPlayer.playbackSpeed = 1f;
            introPlayer.Play();
            if (introPlayer.source == VideoSource.Url && externalAudioSource.clip != null)
            {
                externalAudioSource.volume = 1f;
                externalAudioSource.Play();
            }

            if (verificationMode)
            {
                yield return new WaitForSeconds(0.15f);
                videoComplete = true;
            }

            while (!videoComplete)
            {
                yield return null;
            }

            introPlayer.Stop();
            if (externalAudioSource.isPlaying)
            {
                while (externalAudioSource.volume > 0.1f)
                {
                    yield return null;
                    externalAudioSource.volume -= Time.deltaTime;
                }
                externalAudioSource.Stop();
            }

            if (titleBgm != null)
            {
                titleBgm.Play();
            }
            yield return FadeRawImage(Color.white, Color.clear, verificationMode ? 0.05f : 0.5f);
            CurrentStage = Stage.Title;
        }

        private IEnumerator FadeRawImage(Color from, Color to, float duration)
        {
            float elapsed = 0f;
            introRawImage.color = from;
            while (elapsed < duration)
            {
                yield return null;
                elapsed += Time.deltaTime;
                introRawImage.color = Color.Lerp(from, to, Mathf.Clamp01(elapsed / duration));
            }
            introRawImage.color = to;
        }

        private void ConfigureIntroSource()
        {
            string executableDirectory = Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName);
            string externalVideo = Path.Combine(executableDirectory ?? string.Empty, "Intro", "Intro.mp4");
            if (File.Exists(externalVideo))
            {
                introPlayer.source = VideoSource.Url;
                introPlayer.url = externalVideo;
                introPlayer.audioOutputMode = VideoAudioOutputMode.None;
                externalAudioSource.clip = externalIntroAudio;
            }
            else
            {
                introPlayer.source = VideoSource.VideoClip;
                introPlayer.clip = builtInIntro;
                introPlayer.audioOutputMode = VideoAudioOutputMode.Direct;
            }
        }

        private void PopulateRankingFromOriginalPlayerPrefsContract()
        {
            if (!PlayerPrefs.HasKey("nRank1"))
            {
                for (int rank = 1; rank <= 30; rank++)
                {
                    PlayerPrefs.SetString($"nRank{rank}", RandomIdentifier());
                    PlayerPrefs.SetInt($"sRank{rank}", 1500 - rank * 40);
                    PlayerPrefs.SetInt($"cRank{rank}", Random.Range(0, 9));
                    PlayerPrefs.SetString($"dRank{rank}", DateTime.Now.ToString("yyyy.MM.dd"));
                }
            }

            for (int index = 0; index < rankRows.Length; index++)
            {
                int rank = index + 1;
                int character = Mathf.Clamp(PlayerPrefs.GetInt($"cRank{rank}", 0), 0, characterSprites.Length - 1);
                rankRows[index].SetData(
                    rank,
                    characterSprites[character],
                    PlayerPrefs.GetInt($"sRank{rank}", 0),
                    PlayerPrefs.GetString($"dRank{rank}", "0000.00.00"),
                    PlayerPrefs.GetString($"nRank{rank}", "ABC"));
            }
        }

        private static string RandomIdentifier()
        {
            char[] value = new char[3];
            for (int index = 0; index < value.Length; index++)
            {
                value[index] = (char)Random.Range(65, 91);
            }
            return new string(value);
        }

        private void HideOverlays()
        {
            if (rankRoot != null)
            {
                rankRoot.SetActive(false);
            }
            if (guidanceImages != null)
            {
                foreach (GameObject guidance in guidanceImages)
                {
                    guidance.SetActive(false);
                }
            }
            if (introRawImage != null)
            {
                introRawImage.color = Color.clear;
            }
        }

        private void OnVideoComplete(VideoPlayer player)
        {
            videoComplete = true;
            player.Stop();
        }
    }
}
