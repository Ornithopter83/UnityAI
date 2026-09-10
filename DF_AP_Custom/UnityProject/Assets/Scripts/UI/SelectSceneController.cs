using UnityEngine;
using UnityEngine.UI;

namespace FishingGame.UI
{
    /// <summary>
    /// Reconstructs the original SelectScene warning and mode-selection states.
    /// Keyboard input is the PC adapter for the cabinet's left/right buttons.
    /// </summary>
    public sealed class SelectSceneController : MonoBehaviour
    {
        public enum SelectStage
        {
            WarningSafety,
            WarningControls,
            ModeSelect
        }

        [SerializeField] private GameObject warningRoot;
        [SerializeField] private Image warningImage;
        [SerializeField] private GameObject firstPageTextRoot;
        [SerializeField] private Text warningText;
        [SerializeField] private GameObject modeSelectRoot;
        [SerializeField] private RectTransform selectionOutline;
        [SerializeField] private RectTransform unselectedBlock;
        [SerializeField] private Sprite safetySprite;
        [SerializeField] private Sprite controlsSprite;
        [SerializeField] private float firstPageSeconds = 5f;
        [SerializeField] private float secondPageSeconds = 10f;

        private float stageElapsed;
        private int selectedMode;

        public SelectStage Stage { get; private set; }
        public int SelectedMode => selectedMode;
        public bool SelectionConfirmed { get; private set; }

        public void Configure(
            GameObject warning,
            Image warningBackground,
            GameObject pageOneText,
            Text pulsingWarningText,
            GameObject modeRoot,
            RectTransform outline,
            RectTransform block,
            Sprite safety,
            Sprite controls)
        {
            warningRoot = warning;
            warningImage = warningBackground;
            firstPageTextRoot = pageOneText;
            warningText = pulsingWarningText;
            modeSelectRoot = modeRoot;
            selectionOutline = outline;
            unselectedBlock = block;
            safetySprite = safety;
            controlsSprite = controls;
        }

        private void Start()
        {
            ShowSafetyWarning();
        }

        private void Update()
        {
            stageElapsed += Time.unscaledDeltaTime;

            if (Stage == SelectStage.WarningSafety)
            {
                PulseWarningText();
                if (stageElapsed >= firstPageSeconds)
                {
                    ShowControlsWarning();
                }
            }
            else if (Stage == SelectStage.WarningControls && stageElapsed >= secondPageSeconds)
            {
                ShowModeSelect();
            }

            bool left = Input.GetKeyDown(KeyCode.LeftArrow) || Input.GetKeyDown(KeyCode.A);
            bool right = Input.GetKeyDown(KeyCode.RightArrow) || Input.GetKeyDown(KeyCode.D)
                || Input.GetKeyDown(KeyCode.Return) || Input.GetKeyDown(KeyCode.Space);

            if (Stage != SelectStage.ModeSelect)
            {
                if (left || right)
                {
                    SkipCurrentWarning();
                }
                return;
            }

            if (left)
            {
                MoveSelection();
            }
            else if (right)
            {
                ConfirmSelection();
            }
        }

        public void SkipCurrentWarning()
        {
            if (Stage == SelectStage.WarningSafety)
            {
                ShowControlsWarning();
            }
            else if (Stage == SelectStage.WarningControls)
            {
                ShowModeSelect();
            }
        }

        public void MoveSelection()
        {
            if (Stage != SelectStage.ModeSelect)
            {
                return;
            }

            selectedMode = selectedMode == 0 ? 1 : 0;
            ApplyModeSelection();
        }

        public void ConfirmSelection()
        {
            if (Stage != SelectStage.ModeSelect)
            {
                return;
            }

            SelectionConfirmed = true;
            string mode = selectedMode == 0 ? "SINGLE" : "BATTLE";
            GameLog.Info("SelectScene mode confirmed in PC adapter: " + mode
                + ". The original next-state backend is outside Task 08 scope.");
        }

        private void ShowSafetyWarning()
        {
            Stage = SelectStage.WarningSafety;
            stageElapsed = 0f;
            SelectionConfirmed = false;
            warningRoot.SetActive(true);
            modeSelectRoot.SetActive(false);
            warningImage.sprite = safetySprite;
            firstPageTextRoot.SetActive(true);
            SetWarningAlpha(0.8f);
        }

        private void ShowControlsWarning()
        {
            Stage = SelectStage.WarningControls;
            stageElapsed = 0f;
            warningRoot.SetActive(true);
            modeSelectRoot.SetActive(false);
            warningImage.sprite = controlsSprite;
            firstPageTextRoot.SetActive(false);
        }

        private void ShowModeSelect()
        {
            Stage = SelectStage.ModeSelect;
            stageElapsed = 0f;
            selectedMode = 0;
            SelectionConfirmed = false;
            warningRoot.SetActive(false);
            modeSelectRoot.SetActive(true);
            ApplyModeSelection();
        }

        private void PulseWarningText()
        {
            float alpha = Mathf.Lerp(0.4f, 0.8f, (Mathf.Sin(Time.unscaledTime * Mathf.PI) + 1f) * 0.5f);
            SetWarningAlpha(alpha);
        }

        private void SetWarningAlpha(float alpha)
        {
            if (warningText == null)
            {
                return;
            }

            Color color = warningText.color;
            color.a = alpha;
            warningText.color = color;
        }

        private void ApplyModeSelection()
        {
            selectionOutline.anchoredPosition = new Vector2(selectedMode == 0 ? -364f : 364f, -28f);
            unselectedBlock.anchoredPosition = new Vector2(selectedMode == 0 ? 362f : -362f, -25f);
        }
    }
}
