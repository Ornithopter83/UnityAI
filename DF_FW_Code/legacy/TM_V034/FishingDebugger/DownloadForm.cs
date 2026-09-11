using DF_TM.Controls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Management;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace DF_TM
{
	public partial class DownloadForm : Form
	{
		class UsbComPort
		{
			public SerialPort Serial;
			public string ComPort;
			public string DeviceId;
			public string Version;
			public string OldVer;
			public bool MainUse;
			public bool Selected;
			public byte BoardId;
			public int SendIdx;
			public int SendSeq;
			public int RecvSeq;
			public int Phase;
			public int Result;
			public int Progress;
			public string RecvStr;
			public Stopwatch StopWatch;
			public bool isMsgRecv;
			public int RecvLen;
			public byte[] RecvBuf;
		}
		//	private LabelProgressBar progressBar1;  // 디자이너 대신 코드로 생성
		private int recvResult;
		private DataGridView dgvSubBoards;
		//const string VID = "303A";
		//const string PID = "1001";
		const string CmdGetVersion = "$10%";
		const int MAX_BOARD = 16;
		private int DF_BoardCount = 0;
		UsbComPort[] DF_Board = new UsbComPort[MAX_BOARD];
		//			const int DOWNLOAD_SIZE = 0x1000;			// 8192바이트 전송
		const int DOWNLOAD_SIZE = 0x8000;           // 16바이트 전송(테스트)

		public class DnData
		{
			public bool ComFlag;      // false = RAW, true = COMP
			public int OrgSize;      // 원본 크기 (8192)
			public int OutSize;      // 압축 결과 크기
			public byte[] BufData;   // 압축 or 원본 데이터

			public DnData()
			{
				BufData = new byte[DOWNLOAD_SIZE];
			}
		}

		public DownloadForm()
		{
			InitializeComponent();
			this.FormClosing += DownloadForm_FormClosing;
			this.FormClosed += DownloadForm_FormClosed;
			for (int i = 0; i < DF_Board.Length; i++) {
				DF_Board[i] = new UsbComPort();
			}
		}
		private void DownloadForm_Load(object sender, EventArgs e)
		{
//			InitSubBoardGrid();   // ✅ 여기서 호출
//			LoadSubBoards();   // 🔥 이게 빠져 있었음
		}
		private async void DownloadForm_Shown(object sender, EventArgs e)
		{
			InitSubBoardGrid();
			await Task.Run(() => LoadSubBoards());
			//	await SearchCamerasAsync();
		}
		private void DownloadForm_FormClosed(object sender, FormClosedEventArgs e)
		{
			// 폼이 완전히 닫힌 후 실행
			// 리소스 정리, 로그 기록 등
		//	MessageBox.Show($"닫힌 이유: {e.CloseReason}");

			UsbSerialCloseAll();
		}

		private void DownloadForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			// 폼이 닫히기 직전 실행
			// e.Cancel = true; 로 닫기 취소 가능
/*
			DialogResult result = MessageBox.Show("정말 닫겠습니까?", "확인",
												 MessageBoxButtons.YesNo);
			if (result == DialogResult.No)
			{
				e.Cancel = true;  // 닫기 취소
				return;
			}
*/
			UsbSerialCloseAll();
		}

		private void UsbSerialCloseAll()
		{
			for (int i = 0; i < DF_BoardCount; i++) {
				if (DF_Board[i].MainUse) continue;
				DF_Board[i].Serial?.Close();
			}
		}

		private void MSleep(int miliSec)
		{
			Stopwatch sw = Stopwatch.StartNew();

			while (sw.ElapsedMilliseconds < miliSec)
			{
				Application.DoEvents();  // 메시지 펌프 처리
				Thread.Sleep(1);
			}
		}
		private void InitSubBoardGrid()
		{
			dgvSubBoards = new DataGridView
			{
				Dock = DockStyle.Fill,
				AllowUserToAddRows = false,
				RowHeadersVisible = false,
				SelectionMode = DataGridViewSelectionMode.FullRowSelect
			};

			dgvSubBoards.Columns.Add(new DataGridViewCheckBoxColumn
			{
				HeaderText = "선택",
				Width = 40
			});

			dgvSubBoards.Columns.Add("COM", "COM 포트");
			dgvSubBoards.Columns["COM"].Width = 80;
			dgvSubBoards.Columns.Add("NEW_VER", "현재 버전");
			dgvSubBoards.Columns["NEW_VER"].Width = 80;
			dgvSubBoards.Columns.Add("OLD_VER", "이전 버전");
			dgvSubBoards.Columns["OLD_VER"].Width = 80;
			dgvSubBoards.Columns.Add("State", "상태");
			dgvSubBoards.Columns["State"].Width = 124;

			dgvSubBoards.Columns.Add(new DataGridViewProgressBarColumn
			{
				HeaderText = "진행률",
				Width = 194
			});
			dgvSubBoards.DataError += (s, e) =>
			{
				e.ThrowException = false;
			};
			dgvSubBoards.ClearSelection();
			dgvSubBoards.SelectionMode = DataGridViewSelectionMode.CellSelect;
			dgvSubBoards.MultiSelect = false;
			dgvSubBoards.DefaultCellStyle.SelectionBackColor = dgvSubBoards.DefaultCellStyle.BackColor;
			dgvSubBoards.DefaultCellStyle.SelectionForeColor = dgvSubBoards.DefaultCellStyle.ForeColor;

			panelGrid.Controls.Add(dgvSubBoards);
		}

		List<UsbComPort> FindEsp32S3CdcPorts(string targetVid, string targetPid)
		{
			//			var mainForm = Owner as DFTMmainForm;
			var result = new List<UsbComPort>();

			var searcher = new ManagementObjectSearcher(
				"SELECT * FROM Win32_PnPEntity WHERE Name LIKE '%(COM%'"
			);

			foreach (ManagementObject device in searcher.Get())
			{
				string deviceId = device["DeviceID"]?.ToString();
				string name = device["Name"]?.ToString();

				if (deviceId == null || name == null)
					continue;

				if (!deviceId.Contains(targetVid) ||
					!deviceId.Contains(targetPid))
					continue;

				var match = Regex.Match(name, @"\(COM(\d+)\)");
				if (!match.Success)
					continue;

				result.Add(new UsbComPort
				{
					ComPort = "COM" + match.Groups[1].Value,
					DeviceId = deviceId,
					RecvBuf = new byte[1024]
				});
			}

			return result;
		}
		private void Config(SerialPort serial, bool rtsEnb, bool dtrEnb, int timeout = 100)
		{
			serial.Handshake = Handshake.None;  // 하드웨어 제어 끄기
			serial.RtsEnable = rtsEnb;
			serial.DtrEnable = dtrEnb;
			serial.ReadTimeout = timeout;
			serial.WriteTimeout = 1000;
		}

		private void RefreshCheckBox()
		{
			for (int i = 0; i < DF_BoardCount; i++)
			{
				bool check = ((DF_Board[i].Version != null) &&
							((chkBoxMain.Checked && (DF_Board[i].Version[1] == 'm')) ||
							(chkBoxSub.Checked && (DF_Board[i].Version[1] == 'r')))) ? true : false;
				dgvSubBoards.Rows[i].Cells[0].Value = check;
			}
		}
		private void UpdateProgress(int rowIndex, int progress, string state, int sec=0)
		{
			if (InvokeRequired)
			{
				Invoke(new Action(() => UpdateProgress(rowIndex, progress, state)));
				return;
			}

			dgvSubBoards.Rows[rowIndex].Cells[4].Value = " " + state + ((sec != 0) ? ( " : " + sec.ToString() + "초") : "");
			dgvSubBoards.Rows[rowIndex].Cells[5].Value = progress;
		}

		private void LoadSubBoards()
		{
			var mainForm = Owner as DFTMmainForm;
			bool isMainChecked = false;
			bool isSubChecked = false;
			Invoke(new Action(() =>
			{
				isMainChecked = chkBoxMain.Checked;
				isSubChecked = chkBoxSub.Checked;
			})); 
			DF_BoardCount = 0;
			foreach (var port in FindEsp32S3CdcPorts($"VID_{DFTMmainForm.VID}", $"PID_{DFTMmainForm.PID}"))
			{
				if ((mainForm.MainComPortName != null) && (mainForm.MainComPortName == port.ComPort)) {
					port.MainUse = true;
				}
				else {
					try {
						port.Serial = new SerialPort(port.ComPort, 115200);
						Config(port.Serial, true, false);
						port.Serial.DataReceived += Serial_DataReceived;
						port.Serial.Open();
					}
					catch (Exception ex)
					{
						Invoke(new Action(() =>
						{
							MessageBox.Show("시리얼 포트 오픈: " + ex.Message);
						}));
						continue;
					}
				}
				DF_Board[DF_BoardCount] = port;
				// 표시용: COM번호만
				DF_BoardCount++;
			}
			// Version Read 3회 시도
			for (int retry = 0; retry < 3; retry++)
			{
				for (int i = 0; i < DF_BoardCount; i++)				// 각 보드에 대해 
				{
					if (DF_Board[i].Version == null) {				// 버전이 안 읽혔으면
						DF_Board[i].Serial?.DiscardInBuffer();		// 이전 수신 데이터 버림
						SendMessage(i, CmdGetVersion);				// 버전 읽기 명령 송신
					}
				}
				Stopwatch sw = Stopwatch.StartNew();
				while (sw.ElapsedMilliseconds < 2000)				// 수신 대기 2초 동안
				{
					int cnt = 0;
					for (int i = 0; i < DF_BoardCount; i++)			// 각 보드 수 만큼
					{
						if (DF_Board[i].Version != null)			// 버전이 수신되었으면 카운트업
							cnt++;
					}
					if (cnt >= DF_BoardCount)						// 보드 수 만큼 다 읽혔으면 
						goto version_display;						// 지체없이 버전 뿌리러 가즈아
					MSleep(1);
				}
			}
		version_display:
			for (int i = 0; i < DF_BoardCount; i++)
			{
				bool check = ((DF_Board[i].Version != null) &&										// 버전이 수신되었고
							((isMainChecked && (DF_Board[i].Version[1] == 'm')) ||					// 메인 보드 체크된 상태에 버전이 메인보드이거나
							(isSubChecked && (DF_Board[i].Version[1] == 'r')))) ? true : false;		// 서브 보드 체크된 상태에 버전이 서브보드이면 트루
				AddSubBoardSafe(check, DF_Board[i].ComPort, DF_Board[i].Version, "");
			}
		}

		private void ItemAllEnable(bool enflag)
		{
			btn_FileSelect.Enabled = enflag;
			btn_Download.Enabled = enflag;
			btn_Refresh.Enabled = enflag;
			chkBoxMain.Enabled = enflag;
			chkBoxSub.Enabled = enflag;
			txtFilePath.Enabled = enflag;
		}
		private void btn_FileSelect_Click(object sender, EventArgs e)
		{
			openFileDialog1.Title = "다운로드 대상 파일 선택";
			openFileDialog1.Filter = "bin 파일 (*.bin)|*.bin";

			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				txtFilePath.Text = openFileDialog1.FileName;   // 전체 경로 표시
			}
		}

		private void AddSubBoard(bool check, string comPort, string vid, string pid)
		{
			dgvSubBoards.Rows.Add(
				check,           // 체크박스 기본 선택
				" " + comPort,
				" " + vid,
				" " + pid,
				" Ready",
				0               // 진행률
			);
		}
		private void AddSubBoardSafe(bool check, string comPort, string version, string etc)
		{
			if (InvokeRequired)
			{
				Invoke(new Action(() => AddSubBoard(check, comPort, version, etc)));
			}
			else
			{
				AddSubBoard(check, comPort, version, etc);
			}
		}
		private void btn_Download_Click(object sender, EventArgs e)
		{
			var mainForm = Owner as DFTMmainForm;
			int cIdx, sCnt, pCnt;
			byte bid;
			Stopwatch StartWatch;

			string localPath = txtFilePath.Text.Trim();

			if (string.IsNullOrWhiteSpace(localPath))
			{
				MessageBox.Show("다운로드할 파일을 선택하세요.");
				return;
			}
			string fileName = Path.GetFileName(localPath);
			if (fileName.IndexOf("DF_MAIN", StringComparison.OrdinalIgnoreCase) >= 0)
				bid = (byte)'M';
			else if (fileName.IndexOf("DF_ROD", StringComparison.OrdinalIgnoreCase) >= 0)
				bid = (byte)'R';
			else {
				MessageBox.Show("다운로드할 파일명이 적합하지 않습니다.");
				return;
			}
			sCnt = 0;
			for (cIdx = 0; cIdx < DF_BoardCount; cIdx++)
			{
				DF_Board[cIdx].Phase = 0;
				DF_Board[cIdx].SendSeq = 0;
				DF_Board[cIdx].SendIdx = 0;
				DF_Board[cIdx].Selected = Convert.ToBoolean(dgvSubBoards.Rows[cIdx].Cells[0].Value);
				if (DF_Board[cIdx].Selected) {
					if (DF_Board[cIdx].Version != null && (char.ToUpperInvariant(DF_Board[cIdx].Version[1]) != bid))
					{
						MessageBox.Show("다운로드할 파일과 보드가 일치하지 않습니다.");
						return;
					}
					sCnt++;
					DF_Board[cIdx].OldVer = DF_Board[cIdx].Version;
					dgvSubBoards.Rows[cIdx].Cells[3].Value = " " + DF_Board[cIdx].OldVer;
				}
			}
			if (sCnt == 0)
			{
				MessageBox.Show("다운로드할 대상을 선택하세요.");
				return;
			}
			ItemAllEnable(false);
			dgvSubBoards.ReadOnly = true;
			// 1. BIN 파일을 raw byte[]로 읽기 (변환 없음)
			byte[] firmwareData = File.ReadAllBytes(localPath);
			int totalSize = firmwareData.Length;

			int blockSize = DOWNLOAD_SIZE;
			int blockCount = (totalSize + blockSize - 1) / blockSize;

			DnData[] gDnData = new DnData[blockCount];
			for (int i = 0; i < blockCount; i++)
			{
				gDnData[i] = new DnData();

				int offset = i * blockSize;
				int remain = totalSize - offset;
				int currentSize = remain >= blockSize ? blockSize : remain;

				byte[] block = new byte[currentSize];
				Buffer.BlockCopy(firmwareData, offset, block, 0, currentSize);
#if (true)	// COMPRESS
				byte[] compressed = new byte[currentSize];

				int compressSize = LZ.EncodeLZ(
					block,
					compressed,
					currentSize,
					currentSize);   // targetLen = 8192 제한

				gDnData[i].OrgSize = currentSize;

				if (compressSize < currentSize)
				{
					// 압축 성공
					gDnData[i].ComFlag = true;
					gDnData[i].OutSize = compressSize;

					Buffer.BlockCopy(compressed, 0,
									 gDnData[i].BufData, 0,
									 compressSize);
				}
				else
#endif
				{
					// 압축 이득 없음 → RAW 저장
					gDnData[i].ComFlag = false;
					gDnData[i].OutSize = currentSize;

					Buffer.BlockCopy(block, 0,
									 gDnData[i].BufData, 0,
									 currentSize);
				}
			}
/*
		//	byte[] compressed = new byte[totalSize];
		//	int compressSize = LZEncoder.EncodeLZ(firmwareData, compressed, totalSize, totalSize);

			string writePath = localPath + ".out";
			using (FileStream fs = new FileStream(writePath, FileMode.Create, FileAccess.Write))
			{
				for (int i = 0; i < blockCount; i++)
				{
					//			fs.Write(compressed, 0, compressSize);
					fs.Write(BitConverter.GetBytes((ushort)gDnData[i].OutSize), 0, 2);
					fs.Write(gDnData[i].BufData, 0, gDnData[i].OutSize);
				}
			}
*/
//			bid = (byte)'M';

			StartWatch = Stopwatch.StartNew();
			pCnt = 0;
			cIdx = 0;
			while (pCnt < sCnt)
			{
				for (cIdx = 0; cIdx < DF_BoardCount; cIdx++)
				{
					if (DF_Board[cIdx].Selected)
					{
						switch (DF_Board[cIdx].Phase)
						{
							case 0:
								DF_Board[cIdx].Result = 0;
								//	int chunkSize = Math.Min(DOWNLOAD_SIZE, (int)(totalSize - DF_Board[cIdx].SendIdx));
								//	SendDownloadData(cIdx, bid, DF_Board[cIdx].SendSeq, firmwareData, DF_Board[cIdx].SendIdx, chunkSize);
								int sIdx = DF_Board[cIdx].SendSeq;

								SendDownloadData(cIdx, (byte)(bid | (gDnData[sIdx].ComFlag ? 0x20 : 0)), sIdx, gDnData[sIdx].BufData, 0, gDnData[sIdx].OutSize);

								// 진행률 표시
								DF_Board[cIdx].Progress = (int)((DF_Board[cIdx].SendIdx + gDnData[sIdx].OrgSize) * 100 / totalSize);
								UpdateProgress(cIdx, DF_Board[cIdx].Progress, "Downloading");

								// 작은 딜레이 (필요시)
								DF_Board[cIdx].StopWatch = Stopwatch.StartNew();
								DF_Board[cIdx].Phase = 1;
								break;
							case 1:
								if (DF_Board[cIdx].Result == 0)
								{
									if (DF_Board[cIdx].StopWatch.ElapsedMilliseconds >= 10000)
										DF_Board[cIdx].Result = -2;
									else
										break;
								}
								if (DF_Board[cIdx].Result < 0)
									DF_Board[cIdx].Phase = 2;
								else
								{
									DF_Board[cIdx].SendIdx += DOWNLOAD_SIZE;
									DF_Board[cIdx].SendSeq++;
									if (DF_Board[cIdx].SendIdx < totalSize)
										DF_Board[cIdx].Phase = 0;
									else
										DF_Board[cIdx].Phase = 2;
								}
								break;
							case 2:
								int EndSeq;
								if (DF_Board[cIdx].Result > 0)
								{
									EndSeq = 999;
									DF_Board[cIdx].StopWatch = Stopwatch.StartNew();
									DF_Board[cIdx].Phase = 3;
								}
								else
								{
									EndSeq = 998;
									DF_Board[cIdx].Phase = 8;
									UpdateProgress(cIdx, DF_Board[cIdx].Progress, "Fail");
								}
								SendDownloadData(cIdx, bid, EndSeq, null, 0, 0);
								break;
							case 3:
								if (DF_Board[cIdx].StopWatch.ElapsedMilliseconds >= 1800)
								{
									SendMessage(cIdx, "$10%");       // Ver읽기   // sVersion
									UpdateProgress(cIdx, 100, "Success", (int)StartWatch.ElapsedMilliseconds);
									DF_Board[cIdx].StopWatch = Stopwatch.StartNew();
									DF_Board[cIdx].Version = null;
									DF_Board[cIdx].Phase = 4;
								}
								break;
							case 4:
								if (DF_Board[cIdx].Version != null)
								{
									dgvSubBoards.Rows[cIdx].Cells[2].Value = " " + DF_Board[cIdx].Version;
									if (mainForm.GetMainVersion() != "")
									{
										mainForm.SetMainVersion(DF_Board[cIdx].Version);
									}
									DF_Board[cIdx].Phase = 8;
								}
								else if (DF_Board[cIdx].StopWatch.ElapsedMilliseconds >= 1800)
								{
									DF_Board[cIdx].Phase = 8;
								}
								break;
							case 8:
								pCnt++;
								DF_Board[cIdx].Phase = 9;
								break;
							default:
								break;
						}
					}
				}
				MSleep(1);
			}
		test_out:
			ItemAllEnable(true);
			dgvSubBoards.ReadOnly = false;
		}

		// $DNM(4) + SEQ(3) + OK(1) + %(1)
		private void DataReceive(int cIdx, byte[] buff, int len)
		{
			UsbComPort Ucp = DF_Board[cIdx];
			for (int i = 0; i < len; i++)
			{
				if (buff[i] == (byte)'$')
				{
					Ucp.RecvStr = "$";
					Ucp.isMsgRecv = true;
				}
				else
				{
					if (Ucp.isMsgRecv) {
						if (buff[i] == (byte)'%')
						{
							Ucp.isMsgRecv = false;
							if (Ucp.RecvStr.Length >= 8 && Ucp.RecvStr.Substring(0, 3) == "$DN")
							{
								Ucp.RecvSeq = int.Parse(Ucp.RecvStr.Substring(4, 3));
								if (Ucp.RecvStr[7] == '0')
								{
									Ucp.Result = 1;
								}
								else
								{
									Ucp.Result = -1;
								}
							}
							else if (Ucp.RecvStr.Length >= 4 && Ucp.RecvStr.Substring(0, 3) == "$10")
							{
								string msgVer = Ucp.RecvStr.Substring(3);
								msgVer = msgVer.Replace("%", "");
								string[] strVer = msgVer.Split(',');
								Ucp.Version = strVer[0];
							}
						}
						else
						{
							Ucp.RecvStr += (char)buff[i];
						}
					}
				}
			}
		}
		private void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
		{
			SerialPort sp = (SerialPort)sender;
			for (int i = 0; i < DF_BoardCount; i++) {
				if (DF_Board[i].Serial == sp) {
					//	try
					{
						byte[] recvbuff = new byte[2048];
						int rlen = sp.Read(recvbuff, 0, recvbuff.Length);
						DataReceive(i, recvbuff, rlen);
						return;
					}
				}
			}
		}


		public void DownloadReceiveData(byte[] buffer, int len)
		{
			for (int i = 0; i < DF_BoardCount; i++)
			{
				if (DF_Board[i].MainUse)
				{
					DataReceive(i, buffer, len);
					return;
				}
			}
		}
		public void DownloadReceiveMessage(string msg)
		{
			if (msg.StartsWith("$DN"))              // Download Message
			{
				if (msg[4] == '0')
					recvResult = 1;
				else
					recvResult = -1;
			}
		}
		public void SendMessage(int cIdx, string msg)
		{
			if (DF_Board[cIdx].MainUse)
			{
				var mainForm = Owner as DFTMmainForm;
				mainForm.WriteMessage(msg);
			}
			else
			{
				DF_Board[cIdx].Serial.WriteLine(msg);
			}
		}

		private void SendDownloadData(int cIdx, byte bid, int seq, byte[] buff, int offset, int len)
		{
			ushort csum = 0;
			byte[] SendBuffer = new byte[9 + len + 3];

			for (int i = 0; i < len; i++)
				csum += buff[offset + i];

			SendBuffer[0] = (byte)'$';
			SendBuffer[1] = (byte)'D';
			SendBuffer[2] = (byte)'N';
			SendBuffer[3] = (byte)bid;
			SendBuffer[4] = (byte)(((seq / 100) % 10) + 0x30);
			SendBuffer[5] = (byte)(((seq / 10) % 10) + 0x30);
			SendBuffer[6] = (byte)((seq % 10) + 0x30);
			SendBuffer[7] = (byte)(len >> 8);
			SendBuffer[8] = (byte)len;
			if (len > 0)
				Array.Copy(buff, offset, SendBuffer, 9, len);
			SendBuffer[9 + len] = (byte)(csum >> 8);
			SendBuffer[10 + len] = (byte)csum;
			SendBuffer[11 + len] = (byte)'%';
			if (DF_Board[cIdx].MainUse)
			{
				var mainForm = Owner as DFTMmainForm;
				mainForm.SendDownloadData(SendBuffer, 0, len + 9 + 3);
			}
			else {
				DF_Board[cIdx].Serial.Write(SendBuffer, 0, len + 9 + 3);
			}
		}

		private async void btn_Refresh_Click(object sender, EventArgs e)
		{
			dgvSubBoards.Rows.Clear();
			dgvSubBoards.ClearSelection();
			UsbSerialCloseAll();
			await Task.Run(() => LoadSubBoards());
		}

		private bool _isCbUpdating = false;
		private void chkBoxMain_CheckedChanged(object sender, EventArgs e)
		{
			if (_isCbUpdating) return;
			_isCbUpdating = true;
			if (chkBoxMain.Checked)
			{
				chkBoxSub.Checked = false;
			}
			RefreshCheckBox();
			_isCbUpdating = false;
		}

		private void chkBoxSub_CheckedChanged(object sender, EventArgs e)
		{
			if (_isCbUpdating) return;
			_isCbUpdating = true;
			if (chkBoxSub.Checked)
			{
				chkBoxMain.Checked = false;
			}
			RefreshCheckBox();
			_isCbUpdating = false;
		}
	}
}