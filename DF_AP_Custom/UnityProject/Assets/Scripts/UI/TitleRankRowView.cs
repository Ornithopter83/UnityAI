using UnityEngine;
using UnityEngine.UI;

namespace FishingGame.UI
{
    /// <summary>
    /// One row of the original 30-entry Title ranking table.
    /// </summary>
    public sealed class TitleRankRowView : MonoBehaviour
    {
        [SerializeField] private Text rankText;
        [SerializeField] private Image characterImage;
        [SerializeField] private Text scoreText;
        [SerializeField] private Text dateText;
        [SerializeField] private Text nameText;

        public void Configure(Text rank, Image character, Text score, Text date, Text playerName)
        {
            rankText = rank;
            characterImage = character;
            scoreText = score;
            dateText = date;
            nameText = playerName;
        }

        public void SetData(int rank, Sprite character, int score, string date, string playerName)
        {
            rankText.text = rank.ToString();
            characterImage.sprite = character;
            scoreText.text = score.ToString("#,0");
            dateText.text = date;
            nameText.text = playerName;
        }
    }
}
