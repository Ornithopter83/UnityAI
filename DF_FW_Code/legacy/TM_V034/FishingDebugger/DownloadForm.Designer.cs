namespace DF_TM
{
	partial class DownloadForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.txtFilePath = new System.Windows.Forms.TextBox();
			this.btn_FileSelect = new System.Windows.Forms.Button();
			this.btn_Download = new System.Windows.Forms.Button();
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
			this.label2 = new System.Windows.Forms.Label();
			this.panelTop = new System.Windows.Forms.Panel();
			this.chkBoxSub = new System.Windows.Forms.CheckBox();
			this.chkBoxMain = new System.Windows.Forms.CheckBox();
			this.btn_Refresh = new System.Windows.Forms.Button();
			this.panelGrid = new System.Windows.Forms.Panel();
			this.panelTop.SuspendLayout();
			this.SuspendLayout();
			// 
			// txtFilePath
			// 
			this.txtFilePath.Location = new System.Drawing.Point(52, 6);
			this.txtFilePath.Name = "txtFilePath";
			this.txtFilePath.Size = new System.Drawing.Size(490, 21);
			this.txtFilePath.TabIndex = 3;
			// 
			// btn_FileSelect
			// 
			this.btn_FileSelect.Location = new System.Drawing.Point(548, 4);
			this.btn_FileSelect.Name = "btn_FileSelect";
			this.btn_FileSelect.Size = new System.Drawing.Size(42, 22);
			this.btn_FileSelect.TabIndex = 4;
			this.btn_FileSelect.Text = "파일";
			this.btn_FileSelect.UseVisualStyleBackColor = true;
			this.btn_FileSelect.Click += new System.EventHandler(this.btn_FileSelect_Click);
			// 
			// btn_Download
			// 
			this.btn_Download.Location = new System.Drawing.Point(448, 30);
			this.btn_Download.Name = "btn_Download";
			this.btn_Download.Size = new System.Drawing.Size(84, 28);
			this.btn_Download.TabIndex = 5;
			this.btn_Download.Text = "다운로드";
			this.btn_Download.UseVisualStyleBackColor = true;
			this.btn_Download.Click += new System.EventHandler(this.btn_Download_Click);
			// 
			// openFileDialog1
			// 
			this.openFileDialog1.FileName = "openFileDialog1";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(3, 10);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(41, 12);
			this.label2.TabIndex = 2;
			this.label2.Text = "파일명";
			// 
			// panelTop
			// 
			this.panelTop.Controls.Add(this.chkBoxSub);
			this.panelTop.Controls.Add(this.chkBoxMain);
			this.panelTop.Controls.Add(this.btn_Refresh);
			this.panelTop.Controls.Add(this.btn_Download);
			this.panelTop.Controls.Add(this.btn_FileSelect);
			this.panelTop.Controls.Add(this.txtFilePath);
			this.panelTop.Controls.Add(this.label2);
			this.panelTop.Dock = System.Windows.Forms.DockStyle.Top;
			this.panelTop.Location = new System.Drawing.Point(0, 0);
			this.panelTop.Name = "panelTop";
			this.panelTop.Size = new System.Drawing.Size(602, 60);
			this.panelTop.TabIndex = 6;
			// 
			// chkBoxSub
			// 
			this.chkBoxSub.AutoSize = true;
			this.chkBoxSub.Location = new System.Drawing.Point(148, 38);
			this.chkBoxSub.Name = "chkBoxSub";
			this.chkBoxSub.Size = new System.Drawing.Size(60, 16);
			this.chkBoxSub.TabIndex = 7;
			this.chkBoxSub.Text = "릴보드";
			this.chkBoxSub.UseVisualStyleBackColor = true;
			this.chkBoxSub.CheckedChanged += new System.EventHandler(this.chkBoxSub_CheckedChanged);
			// 
			// chkBoxMain
			// 
			this.chkBoxMain.AutoSize = true;
			this.chkBoxMain.Location = new System.Drawing.Point(34, 38);
			this.chkBoxMain.Name = "chkBoxMain";
			this.chkBoxMain.Size = new System.Drawing.Size(72, 16);
			this.chkBoxMain.TabIndex = 6;
			this.chkBoxMain.Text = "메인보드";
			this.chkBoxMain.UseVisualStyleBackColor = true;
			this.chkBoxMain.CheckedChanged += new System.EventHandler(this.chkBoxMain_CheckedChanged);
			// 
			// btn_Refresh
			// 
			this.btn_Refresh.Location = new System.Drawing.Point(334, 30);
			this.btn_Refresh.Name = "btn_Refresh";
			this.btn_Refresh.Size = new System.Drawing.Size(84, 28);
			this.btn_Refresh.TabIndex = 5;
			this.btn_Refresh.Text = "새로고침";
			this.btn_Refresh.UseVisualStyleBackColor = true;
			this.btn_Refresh.Click += new System.EventHandler(this.btn_Refresh_Click);
			// 
			// panelGrid
			// 
			this.panelGrid.Dock = System.Windows.Forms.DockStyle.Fill;
			this.panelGrid.Location = new System.Drawing.Point(0, 60);
			this.panelGrid.Name = "panelGrid";
			this.panelGrid.Size = new System.Drawing.Size(602, 175);
			this.panelGrid.TabIndex = 7;
			// 
			// DownloadForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(602, 235);
			this.Controls.Add(this.panelGrid);
			this.Controls.Add(this.panelTop);
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "DownloadForm";
			this.Text = "보드 다운로드";
			this.Load += new System.EventHandler(this.DownloadForm_Load);
			this.Shown += this.DownloadForm_Shown;

			this.panelTop.ResumeLayout(false);
			this.panelTop.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.TextBox txtFilePath;
		private System.Windows.Forms.Button btn_FileSelect;
		private System.Windows.Forms.Button btn_Download;
		private System.Windows.Forms.OpenFileDialog openFileDialog1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Panel panelTop;
		private System.Windows.Forms.Panel panelGrid;
		private System.Windows.Forms.Button btn_Refresh;
		private System.Windows.Forms.CheckBox chkBoxSub;
		private System.Windows.Forms.CheckBox chkBoxMain;
	}
}