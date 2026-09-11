using System;
using System.IO;
using System.IO.Ports;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;

namespace DF_TM
{

    public class SerialManager
    {

        private LogMgr logMgr;
        private readonly DFTMmainForm mainForm;



        private readonly SerialPort serial;
        private readonly object _lock = new object();

        private int reconnectAttempt = 0;
        private const int maxReconnectAttempts = 20;
        private System.Windows.Forms.Timer reconnectTimer;
        private const string pollMsg = "$00%";

		// == Serial 외부용===
		public SerialManager(DFTMmainForm form)
        {
            mainForm = form;
            logMgr = new LogMgr(mainForm);


            serial = new SerialPort();

            reconnectTimer = new System.Windows.Forms.Timer();
            reconnectTimer.Interval = 1000; // 1초
            reconnectTimer.Tick += ReconnectTimer_Tick;

            //reconnectTimer.Elapsed += ReconnectTimer_Elapsed;

            serial.DataReceived += Serial_DataReceived;
            serial.ErrorReceived += Serial_ErrorReceived;
            //serial.Disposed += Serial_Disposed;

        }

        public void Configure(string portName, int baudRate, bool rtsEnb, bool dtrEnb, int timeout = 100)
        {
            lock (_lock)
            {
                serial.PortName = portName;
                serial.BaudRate = baudRate;
                serial.RtsEnable = rtsEnb;
                serial.DtrEnable = dtrEnb;
                serial.ReadTimeout = timeout;
            }
        }

        public void Open()
        {
            lock (_lock)
            {
                if (serial.IsOpen)
                    serial.Close();

                serial.Open();
                reconnectAttempt = 0; // 성공하면 시도 횟수 초기화
            }
        }

        /*
        public void Close()
        {
            if (serial == null || !serial.IsOpen)
                return;
            serial.Close();
        }
        */
        public void Close()
        {
            lock (_lock)
            {
                if (serial != null && serial.IsOpen)
                {
                    serial.Close();
                }
            }
        }

        /*
        public void WriteMessage(byte[] buffer, int offset, int count)
        {
            lock (_lock)
            {
                if (serial.IsOpen)
                    serial.WriteMsg(buffer, offset, count);
            }
        }
        */

        public void WriteMessage(string text)
        {
            lock (_lock)
            {
                if (serial.IsOpen)
                    WriteMsg(text);         // 함수 변경
            }
        }

        public int Write(byte[] buffer, int offset, int count)
        {
            lock (_lock)
            {
                if (serial.IsOpen) {
					serial.Write(buffer, offset, count);
                    return count;
				}
				else
                    return 0;
            }
        }

		public int Read(byte[] buffer, int offset, int count)
		{
			lock (_lock)
			{
				if (serial.IsOpen)
					return serial.Read(buffer, offset, count);
				else
					return 0;
			}
		}

		public string ReadLine()
        {
            lock (_lock)
            {
                if (serial.IsOpen)
                    return serial.ReadLine();
                else
                    return string.Empty;
            }
        }

        /*
        public bool IsOpen()
        {
            if (serial == null)
                return false;
            return serial.IsOpen;
        }
        */
        public bool IsOpen
        {
            get
            {
                lock (_lock)
                {
                    if (serial == null) { return false; }
                    return serial.IsOpen;
                }
            }
        }

        public void SetEventHandlers(SerialDataReceivedEventHandler dataHandler, SerialErrorReceivedEventHandler errorHandler)
        {
            lock (_lock)
            {
                serial.DataReceived += dataHandler;
                serial.ErrorReceived += errorHandler;
            }
        }

    //=== 외부용 끝====

        //--------- LOG --------------
        //private LogMgr logMgr = new LogMgr();

        public delegate void ReceiveMessageCallback(string msg);
        public event ReceiveMessageCallback ReceiveMessage;
		public delegate void DownloadReceiveCallback(byte[] buffer, int len);
		public event DownloadReceiveCallback DownloadReceive;

		public void SetLogBox(ref RichTextBox tb)
        {
            logMgr.LogTextBox   =   tb;
        }

        public void ClearLogBox()
        {
            logMgr.LogTextBox.Clear();
        }

        public void ToggleAutoScroll()
        {
            logMgr.ToggleAutoScroll();
        }

        public void SaveLog()
        {
            try
            {
                SaveFileDialog sfd = new SaveFileDialog();
                sfd.Title = "로그 저장";
                sfd.Filter = "텍스트 파일 | *.txt; | 모든 파일 | *.*";
                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    string fileName = sfd.FileName;
                    if (false == fileName.Contains(".txt"))
                    {
                        fileName = fileName + ".txt";
                    }

                    using (StreamWriter sw = new StreamWriter(fileName))
                    {
                        sw.Write(logMgr.LogTextBox.Text);
                    }
                    MessageBox.Show("로그가 저장되었습니다.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                }
                else
                {
                    MessageBox.Show("올바른 경로를 지정해주세요.", "확인", MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                }
            }
            catch (Exception ex)
            {
                LogText.Log("에러 통신로그창 저장: " + ex.ToString());
                MessageBox.Show("에러 통신로그창 저장: " + ex.Message);
                // 또는 로그로 저장
            }

        }

        //------ SERIAL

        public Action<string> OnErrorMessage;       // msg전달

        //private static SerialPort serial;               // SERIAL DEFINE
 
        // UI 쓰레드에서 체크박스 값 , 인수 추가
        public async Task<bool> Open(string comPort, int baudrate, bool rtsEnb, bool dtrEnb)
        {
            //bool rts = false, dtr = false;

            //this.Invoke(new Action(() =>
            //{
            //    rts = cb_RTS.Checked;
            //    dtr = cb_DTR.Checked;
            //}
            //));

            if (comPort.Equals("COM1") || comPort.Equals("COM2"))
                return false;

            await Task.Run(() =>
            {
                try
                {
                    // 1)시리얼 있고 열려있으면(?) : 어떤 포트 인지 모르니,
                    if (serial != null && serial.IsOpen)    // Serial이 NULL이 아니고, 오픈되어 있으면 Skip
                    {
                        //return;
                        serial.Close();
                    }

                    // 
                    //serial = new SerialPort();

#if (true)
//                    Config(comPort, baudrate, true, true, 50);      // 50ms T/O
					Config(comPort, baudrate, rtsEnb, dtrEnb, 50);      // 50ms T/O
#else
                    //{

                    //serial.ReadBufferSize = 8196;   // 4K => 8K
                    //serial.WriteBufferSize = 8196;  // 4K => 8K
                    serial.PortName = comPort;
                        serial.BaudRate = baudrate;
                        serial.ReadTimeout = 50;

                        //serial.RtsEnable = rtsEnb;
                        //serial.DtrEnable = dtrEnb;

                        serial.RtsEnable = true;
                        serial.DtrEnable = true;

                    // Profram Start : - COM Open:-, Com Close : Main Reset, Program Exit: Main Reset
                    //serial.RtsEnable = true; serial.DtrEnable = false;    // COM Open: Main Reset, Com Close : -
                    //serial.DtrEnable = false;  serial.RtsEnable = true;   // COM Open : -, Com Close: -
                    //serial.DtrEnable = false;  serial.RtsEnable = false;   // COM Open : -, Com Close: -

                    //== RTS, DTR 지정 UI(RB)추가
                    // serial.RtsEnable = rb_RTS.Checked;
                    // serial.DtrEnable = rb_DTR.Checked;

                    //};
#endif

					//---- 포트 오픈-----
					try
					{
                        serial.Open();
                        //var type = serial.GetType();
                        reconnectAttempt = 0; // 성공하면 시도 횟수 초기화
                    }
                    catch (UnauthorizedAccessException)
                    {
                        //MessageBox.Show("포트가 이미 다른 프로그램에서 사용 중입니다.", "접근 거부");
                        OnErrorMessage?.Invoke("포트가 이미 다른 프로그램에서 사용 중입니다. 접근 거부");
                    }
                    catch (IOException)
                    {
                        OnErrorMessage?.Invoke("포트를 열 수 없습니다. 포트가 존재하지 않거나 하드웨어 오류. I/O 오류");
                    }
                    catch (ArgumentException)
                    {
                        OnErrorMessage?.Invoke("포트 이름이 잘못되었습니다. 존재하지 않는 포트일 수 있습니다. 잘못된 포트");
                    }
                    catch (InvalidOperationException)
                    {
                        OnErrorMessage?.Invoke("포트가 이미 열려 있습니다. 중복 열기");
                    }
                    catch (Exception ex)
                    {
                        LogText.Log("에러 통신포트 오픈: " + ex.ToString());
                        OnErrorMessage?.Invoke("에러 통신포트 오픈" + ex.Message);
                    }
                    //----------------------

                    // 오픈상태 확인, LOG버퍼 클리어
                    if (serial.IsOpen)
                    {

                        logMgr.Append(String.Format("Com Port Connected : {0}", serial.IsOpen));
                        // Clear Buffer
                        int bufSize = serial.ReadBufferSize;
                        byte[] buf = new byte[bufSize];
                        serial.Read(buf, 0, bufSize);

                        //logMgr.Init();

                        //Serial_Open_SendCmd();        // 통신포트 오픈후, 메뉴 클릭등이 안됨, 프로그램 종료도 안됨!! => 삭제

                        //return true;

                    }
                    else
                    {
                        // NA, "오픈이 안되네요. COMx확인해주세요.
                        Console.WriteLine("포트를 열 수 없습니다. 다른 프로그램에서 포트를 사용 중일 수 있습니다.");
                        //return false;
                    }

                }

                catch (UnauthorizedAccessException)
                {
                    // 포트를 열 수 없을 때 (다른 프로그램에서 사용 중일 경우 등)
                    Console.WriteLine("포트를 열 수 없습니다. 다른 프로그램에서 포트를 사용 중일 수 있습니다.");
                }

                //catch (Exception e)
                //{
                //    Console.WriteLine(e.StackTrace);
                //}

                catch (Exception ex)
                {
                    // 기타 예외 처리
                    Console.WriteLine($"포트를 여는 중에 오류가 발생했습니다: {ex.Message}");
                }
            }

            );
            //Console.WriteLine("Com Port Connected : {0}", serial.IsOpen);

            return (serial.IsOpen);
        }

        private void Config(string portName, int baudRate, bool rtsEnb, bool dtrEnb, int timeout = 100)
        {
                serial.PortName = portName;
                serial.BaudRate = baudRate;
                serial.Handshake = Handshake.None;  // 하드웨어 제어 끄기
                serial.RtsEnable = rtsEnb;
                serial.DtrEnable = dtrEnb;
                serial.ReadTimeout = timeout;
		    	serial.WriteTimeout = 1000;
		}
		public void SignalConfig(bool rtsEnb, bool dtrEnb)
		{
			serial.RtsEnable = rtsEnb;
			serial.DtrEnable = dtrEnb;
		}


#if (false)   // 통신포트 오픈후, 메뉴 클릭등이 안됨, 프로그램 종료도 안됨!! => 삭제
        private void Serial_Open_SendCmd()
        {
                // 자동 송신
                WriteMessage("$290101%");   // TM통지
                WriteMessage("$1101%");     // 게임시작 통지
                WriteMessage("$10DFTM_V0250%");       // Ver읽기   // sVersion
                WriteMessage("$081%");     // IMU Data ON
        }
#endif

		private void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                if (null != DownloadReceive) {
                    byte[] recvbuff = new byte[1024];
                    int rlen = serial.Read(recvbuff, 0, recvbuff.Length);
                    DownloadReceive(recvbuff, rlen);
                    return;
				}
                //string recvData = serial.ReadExisting().Trim();
                string recvData = serial.ReadLine().Trim();
                //recvData = recvData.Substring(0, str.Length - 1);       // Delete Last "%"
                //string timenow  = $"{DateTime.Now:yyyyMMdd_HHmmss}";
                //string timenow = $"{DateTime.Now:yyMMdd_HHmmss_fff}";
                if (null == ReceiveMessage)
                {
                    Console.WriteLine("데이터 NULL 수신");
                }
                else {
					if (!(mainForm.Poll_Filter && (recvData == pollMsg)))
						logMgr.Append($"<-FW] {recvData}");
                }
                //Console.WriteLine("< Recv > : {0}", recvData);
                if (null != ReceiveMessage)
                    ReceiveMessage(recvData);
            }

            //catch (TimeoutException)
            //{
            //    // 데이터 수신 타임아웃 발생 시 처리
            //    Console.WriteLine("데이터 수신 타임아웃");
            //}

            catch (IOException ex)
            {
                // 포트가 닫혔을 때 발생하는 예외
                Console.WriteLine($"데이터 수신 중 포트 오류 발생: {ex.Message}");

                // 포트가 닫혔음을 감지하고, 다시 열도록 시도
                ReOpenSerialPort();
            }

            catch (InvalidOperationException ex)
            {
                // 포트가 닫혔을 때 발생할 수 있는 예외
                Console.WriteLine($"포트가 닫혀서 수신할 수 없습니다: {ex.Message}");

                // 포트가 닫혔음을 감지하고, 다시 열도록 시도
                ReOpenSerialPort();
            }

            catch (Exception ep)
            {
                string timenow  = $"{DateTime.Now:yyMMdd_HHmmss_fff}";
                logMgr.Append($"{timenow} ER] {ep.StackTrace}");

                //Console.WriteLine(ep.StackTrace);
            }

        }

        // 포트가 닫혔을 때 포트를 다시 여는 메서드
        private void ReOpenSerialPort()
        {
            try
            {
                //Console.WriteLine("포트가 닫혔습니다. 다시 열려고 시도합니다...");

                // 포트가 열려있으면, 닫고 다시 열기
                if (serial.IsOpen)
                {
                    serial.Close();
                }

                serial.Open();
                Console.WriteLine("포트가 정상적으로 다시 열렸습니다.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"포트를 다시 여는 중 오류 발생: {ex.Message}");
            }
        }

        /*
        private void Serial_ErrorReceived(object sender, SerialErrorReceivedEventArgs ex)
        {
            //throw new NotImplementedException();
            //Console.WriteLine($"데이타 수신중에 오류가 발생했습니다: {ex.EventType}");
            // 발생한 오류 유형을 확인
            switch (ex.EventType)
            {
                case SerialError.Frame:
                    Console.WriteLine("프레임 오류 발생: 데이터 전송 중 비트가 손상됨.");
                    break;

                case SerialError.Overrun:
                    Console.WriteLine("오버런 오류 발생: 수신 버퍼 오버플로우.");
                    break;

                case SerialError.RXOver:
                    Console.WriteLine("수신 버퍼 초과 오류 발생.");
                    break;

                case SerialError.TXFull:
                    Console.WriteLine("송신 버퍼가 가득 차 오류 발생.");
                    break;
            
                //case SerialError.Break:
                //    Console.WriteLine("브레이크 신호 발생.");
                //    break;

                case SerialError.RXParity:
                    Console.WriteLine("패리티 오류 발생.");
                    break;
            
                default:
                    Console.WriteLine("알 수 없는 오류 발생.");
                    break;
            }
        }
        */

        private void Serial_ErrorReceived(object sender, SerialErrorReceivedEventArgs ex)
        {
            Console.WriteLine("시리얼 에러 발생: " + ex.EventType);

            // 발생한 오류 유형을 확인
            switch (ex.EventType)
            {
                case SerialError.Frame:
                    Console.WriteLine("프레임 오류 발생: 데이터 전송 중 비트가 손상됨.");
                    break;

                case SerialError.Overrun:
                    Console.WriteLine("오버런 오류 발생: 수신 버퍼 오버플로우.");
                    break;

                case SerialError.RXOver:
                    Console.WriteLine("수신 버퍼 초과 오류 발생.");
                    break;

                case SerialError.TXFull:
                    Console.WriteLine("송신 버퍼가 가득 차 오류 발생.");
                    break;

                //case SerialError.Break:
                //    Console.WriteLine("브레이크 신호 발생.");
                //    break;

                case SerialError.RXParity:
                    Console.WriteLine("패리티 오류 발생.");
                    break;

                default:
                    Console.WriteLine("알 수 없는 오류 발생.");
                    break;
            }

            lock (_lock)
            {
                if (!serial.IsOpen && !reconnectTimer.Enabled)
                {
                    Console.WriteLine("재연결 타이머 시작...");
                    reconnectAttempt = 0;
                    reconnectTimer.Start();
                }
            }
        }

        /*
        private void ReconnectTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            lock (_lock)
            {
                if (serial.IsOpen)
                {
                    reconnectTimer.Stop();
                    return;
                }

                if (reconnectAttempt >= maxReconnectAttempts)
                {
                    Console.WriteLine("최대 재연결 시도 횟수 초과. 포트 열기 중단.");
                    reconnectTimer.Stop();
                    return;
                }

                try
                {
                    Console.WriteLine($"[{reconnectAttempt + 1}/{maxReconnectAttempts}] 포트 재연결 시도...");
                    serial.Open();
                    Console.WriteLine("포트 재연결 성공!");
                    reconnectTimer.Stop();
                    reconnectAttempt = 0;
                }
                catch (Exception ex)
                {
                    reconnectAttempt++;
                    Console.WriteLine("포트 열기 실패: " + ex.Message);
                    // 타이머는 계속 동작 중
                }
            }
        }
        */

        private void ReconnectTimer_Tick(object sender, EventArgs e)
        {
            if (serial.IsOpen)
            {
                reconnectTimer.Stop();
                return;
            }

            if (reconnectAttempt >= maxReconnectAttempts)
            {
                reconnectTimer.Stop();
                Console.WriteLine("❌ 최대 재시도 초과");
                return;
            }

            try
            {
                Console.WriteLine($"⏳ 재연결 시도 {reconnectAttempt + 1}/{maxReconnectAttempts}");
                serial.Open();
                Console.WriteLine("✅ 포트 재연결 성공");
                reconnectTimer.Stop();
                reconnectAttempt = 0;
            }
            catch (Exception ex)
            {
                reconnectAttempt++;
                Console.WriteLine("⚠️ 재연결 실패: " + ex.Message);
            }
        }

        // 수신메시지 처리
        private string timenow;
        private void WriteMsg(string msg)
        {
            if (msg == null)
            {
                logMgr.Append($"ERR] 메세지 없음: {msg}");
                return;
            }

            if (serial == null || !serial.IsOpen)
            {
                logMgr.Append($"ERR] Failed to close serial:");
                return;
            }

            //string timenow;

            //string timenow  = $"{DateTime.Now:yyMMdd_HHmmss_fff}";
            //logMgr.Append($"{timenow} AP] {msg}");
            //serial.Write(msg + "\r\n");

            //Console.WriteLine("< Send > : {0}", msg);

            //----------25/06/19a TM행업발생 부분------------------------
            //== 예외처리가 안됨!!!
            // System.IO.IOException: '시스템에 부착된 장치가 작동하지 않습니다.

            try
            {
                //timenow = $"{DateTime.Now:yyMMdd_HHmmss_fff}";
                //logMgr.Append($"{timenow} AP] {msg}");
                if (!(mainForm.Poll_Filter && (msg == pollMsg)))
                    logMgr.Append($"AP->] {msg}");
                serial.Write(msg + "\r\n");
            }
            catch (InvalidOperationException ex)
            {
                //logMgr.Append($"{timenow} [ERR] InvalidOperationException: {ex.Message}");
                logMgr.Append($"ERR] InvalidOperationException: {ex.Message}");
                // 예: 포트가 닫힌 상태에서 Write 시도
            }
            catch (TimeoutException ex)
            {
                //logMgr.Append($"{timenow} [ERR] TimeoutException: {ex.Message}");
                logMgr.Append($"ERR] TimeoutException: {ex.Message}");
                // 예: 쓰기 제한 시간 초과
            }
            catch (IOException ex)
            {
                //logMgr.Append($"{timenow} [ERR] IOException: {ex.Message}");
                logMgr.Append($"ERR] IOException: {ex.Message}");
                // 예: 장치 연결 끊김, 케이블 제거 등
                // 필요하면 포트 닫기 또는 재연결 시도
                TryCloseSerial();
            }
            catch (Exception ex)
            {
                //logMgr.Append($"{timenow} [ERR] Unknown Exception: {ex.Message}");
                logMgr.Append($"ERR] Unknown Exception: {ex.Message}");
                // 모든 예외를 마지막에 한 번 더 잡아줌
            }
        }

        private void TryCloseSerial()
        {
            try
            {
                if (serial != null && serial.IsOpen)
                    serial.Close();
            }
            catch (Exception ex)
            {
                logMgr.Append($"ERR] Failed to close serial=> {ex.Message}");
            }
        }

    }
}
