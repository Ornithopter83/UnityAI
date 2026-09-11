using System;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace DF_TM
{
    class LogMgr
    {
        // 통신로그을 파일에 저장
        private static string ComlogFilePath = Path.Combine(Application.StartupPath, "logs", "log_com");
        //, "comlog" + DateTime.Now.ToString("yyyy-MM-dd_HH:mm") + ".txt");
        
        // 통신로그를 로그창(리치박스)에 뿌리기.
        public RichTextBox LogTextBox;
        private bool bAutoScroll = true;

        //private DFTMmainForm mainForm;
        private readonly DFTMmainForm mainForm;
        public LogMgr(DFTMmainForm form)
        {
            mainForm = form;
        }

        private short logTimeMin = 5;   // 5분 간격 저장
        public void Append(string str)
        {
            try
            {
				// 1) 통신로그를 파일에 저장.
#if (false)
                //string timeKey;
                string comlogFileName = ComlogFilePath + GetCurrentTime_SliceMin(2) + ".txt";
                string folder = Path.GetDirectoryName(comlogFileName);

                if (!Directory.Exists(folder))
                {
                    Directory.CreateDirectory(folder);
                }

                string timenow = DateTime.Now.ToString("yyyy-MM-dd_HH:mm:fff");
                // comLog를 파일에 저장	// 덮어쓰기
                File.AppendAllText(comlogFileName, (timenow + " " + str + Environment.NewLine));
#endif
#if (false)
                // 1) 통신로그를 파일에 저장.
                string fullPath = ComlogFilePath + GetCurrentTime_SliceMin(logTimeMin) + ".txt";
                string folder = Path.GetDirectoryName(fullPath);

                string timenow = DateTime.Now.ToString("yyyy-MM-dd_HH:mm:ss:fff");

                try
                {
                    if (!Directory.Exists(folder))
                    {
                        Directory.CreateDirectory(folder);
                    }

                    using (var sw = new StreamWriter(fullPath, append: true, encoding: Encoding.UTF8))
                    {
                        sw.WriteLine(timenow + " " + str);
                    }
                }
                catch (IOException ex)
                {
                    //MessageBox.Show("로그 파일을 사용할 수 없습니다:\n" + ex.Message);
                    mainForm.ShowErrorSafeRTB("Exception: " + ex.Message);
                }
#else
				string fullPath = ComlogFilePath + GetCurrentTime_SliceMin(logTimeMin) + ".txt";
				string folder = Path.GetDirectoryName(fullPath);
				string timenow = DateTime.Now.ToString("yyyy-MM-dd_HH:mm:ss:fff");

				try
				{
					if (!Directory.Exists(folder))
					{
						Directory.CreateDirectory(folder);
					}

					// ✅ FileShare.ReadWrite로 수정 + 중복 파일 방지
					using (var fs = new FileStream(fullPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
					using (var sw = new StreamWriter(fs, Encoding.UTF8))
					{
						sw.WriteLine(timenow + " " + str);
					}
				}
				catch (IOException ex)
				{
					// ✅ 대체 로그 파일 생성 (밀리초 추가로 충돌 방지)
					try
					{
						string altPath = fullPath.Replace(".txt", $"_alt_{DateTime.Now:HHmmssfff}.txt");
						using (var fs = new FileStream(altPath, FileMode.Append, FileAccess.Write, FileShare.ReadWrite))
						using (var sw = new StreamWriter(fs, Encoding.UTF8))
						{
							sw.WriteLine($"[{timenow}] ALT: " + str);
						}
						mainForm.ShowErrorSafeRTB("대체 로그 생성: " + Path.GetFileName(altPath));
					}
					catch (Exception altEx)
					{
						mainForm.ShowErrorSafeRTB("로그 완전 실패: " + altEx.Message);
					}
				}
#endif

				// 2) 통신로그창에 출력
#if (true)
				if (null == LogTextBox)
                {
                    return;
                }

                //comLog를 로그창에 출력
                /*
                LogTextBox.AppendText(timenow +" "+ str + Environment.NewLine);
				LogTextBox.SelectionStart = LogTextBox.TextLength;
				if (bAutoScroll) 
                {
                    LogTextBox.ScrollToCaret();
                }
                */
                if (LogTextBox.InvokeRequired)
                {
                    LogTextBox.BeginInvoke(new Action(() =>
                    {
                        LogTextBox.AppendText(timenow + " " + str + Environment.NewLine);
                        LogTextBox.SelectionStart = LogTextBox.TextLength;
                        if (bAutoScroll)
                        {
                            LogTextBox.ScrollToCaret();
                        }
                    }));
                }
                else
                {
                    LogTextBox.AppendText(timenow + " " + str + Environment.NewLine);
                    LogTextBox.SelectionStart = LogTextBox.TextLength;
                    if (bAutoScroll)
                    {
                        LogTextBox.ScrollToCaret();
                    }
                }
        #endif

            }
            catch (Exception ex)
            {
                LogText.Log("에러 로그창 어펜드1: " + ex.ToString());
                //MessageBox.Show("에러  로그창 어펜드1: " + ex.Message);
                if (mainForm != null)
                    //mainForm.ShowError("Exception: " + ex.Message);
                    mainForm.ShowErrorSafeRTB("Exception: " + ex.Message);
                // 또는 로그로 저장
            }
        }

        private static string GetCurrentTime_SliceMin(short sliceMin)
        {
            DateTime now = DateTime.Now;

            // 현재 시각에서 분을 10분 단위로 내림
            int roundedMinute = (now.Minute / sliceMin) * sliceMin;

            // 새 DateTime 만들기 (초는 0으로)
            DateTime logTime = new DateTime(now.Year, now.Month, now.Day, now.Hour, roundedMinute, 0);

            return logTime.ToString("yyyy-MMdd_HHmm");
        }

        public void ToggleAutoScroll()
		{
			bAutoScroll	=	!bAutoScroll;
		}
	}
}