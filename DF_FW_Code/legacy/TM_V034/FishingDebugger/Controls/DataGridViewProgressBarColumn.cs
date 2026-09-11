using System;
using System.Drawing;
using System.Windows.Forms;

namespace DF_TM.Controls
{
	public class DataGridViewProgressBarColumn : DataGridViewTextBoxColumn
	{
		public DataGridViewProgressBarColumn()
		{
			CellTemplate = new DataGridViewProgressBarCell();
			ValueType = typeof(int);
		}
	}

	public class DataGridViewProgressBarCell : DataGridViewTextBoxCell
	{
		public override Type ValueType => typeof(int);

		protected override void Paint(
			Graphics graphics,
			Rectangle clipBounds,
			Rectangle cellBounds,
			int rowIndex,
			DataGridViewElementStates cellState,
			object value,
			object formattedValue,
			string errorText,
			DataGridViewCellStyle cellStyle,
			DataGridViewAdvancedBorderStyle advancedBorderStyle,
			DataGridViewPaintParts paintParts)
		{
			// 기본 셀 배경 / 테두리
			base.Paint(graphics, clipBounds, cellBounds, rowIndex,
					   cellState, null, null, errorText,
					   cellStyle, advancedBorderStyle,
					   DataGridViewPaintParts.Background |
					   DataGridViewPaintParts.Border);

			int progress = 0;
			if (value != null && int.TryParse(value.ToString(), out int p))
				progress = Math.Max(0, Math.Min(100, p));

			Rectangle bar = new Rectangle(
				cellBounds.X + 2,
				cellBounds.Y + 2,
				(cellBounds.Width - 4) * progress / 100,
				cellBounds.Height - 4);

			using (Brush back = new SolidBrush(Color.LightGray))
				graphics.FillRectangle(back, cellBounds);

			using (Brush fore = new SolidBrush(Color.DodgerBlue))
				graphics.FillRectangle(fore, bar);

			TextRenderer.DrawText(
				graphics,
				$"{progress}%",
				cellStyle.Font,
				cellBounds,
				Color.Black,
				TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter
			);
		}
	}
}
