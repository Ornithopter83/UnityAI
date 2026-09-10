using UnityEngine;
using UnityEngine.UI;

namespace FishingGame.UI
{
    /// <summary>
    /// Mutable Title UI corresponding to the original Card, battery, power_lv,
    /// and clinet_version objects. Hardware adapters can update these values later.
    /// </summary>
    public sealed class TitleStatusView : MonoBehaviour
    {
        [SerializeField] private Text paymentText;
        [SerializeField] private Text tensionText;
        [SerializeField] private Text versionText;
        [SerializeField] private Image batteryImage;
        [SerializeField] private Sprite[] batterySprites;

        private int tensionLevel = 3;
        private int batteryIndex;

        public void Configure(Text payment, Text tension, Text version, Image battery, Sprite[] sprites)
        {
            paymentText = payment;
            tensionText = tension;
            versionText = version;
            batteryImage = battery;
            batterySprites = sprites;
        }

        private void Start()
        {
            SetPaymentMessage("카드 단말기에 카드를 꽂거나 터치하게 되면 결제 후 게임이 시작 됩니다.");
            SetTensionLevel(3);
            SetClientVersion("L 1.3.5");
            SetBatteryState(0);
        }

        private void Update()
        {
            if (RuntimeModeSettings.Current != RuntimeMode.Pc)
            {
                return;
            }

            if (Input.GetKeyDown(KeyCode.UpArrow))
            {
                SetTensionLevel(tensionLevel + 1);
            }
            else if (Input.GetKeyDown(KeyCode.DownArrow))
            {
                SetTensionLevel(tensionLevel - 1);
            }

            if (Input.GetKeyDown(KeyCode.B))
            {
                SetBatteryState(batteryIndex + 1);
            }
        }

        public void SetPaymentMessage(string message)
        {
            if (paymentText != null)
            {
                paymentText.text = message ?? string.Empty;
            }
        }

        public void SetTensionLevel(int level)
        {
            tensionLevel = Mathf.Clamp(level, 1, 5);
            if (tensionText != null)
            {
                tensionText.text = "장력 : " + tensionLevel;
            }
        }

        public void SetClientVersion(string version)
        {
            if (versionText != null)
            {
                versionText.text = version ?? string.Empty;
            }
        }

        public void SetBatteryState(int state)
        {
            if (batterySprites == null || batterySprites.Length == 0)
            {
                return;
            }

            batteryIndex = (state % batterySprites.Length + batterySprites.Length) % batterySprites.Length;
            if (batteryImage != null)
            {
                batteryImage.sprite = batterySprites[batteryIndex];
            }
        }
    }
}
