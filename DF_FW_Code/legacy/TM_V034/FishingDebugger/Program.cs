using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;

namespace DF_TM
{
    // TM 종류 (권한 제한)
    public static class Config
    {
        public const bool IsDevelope = false;        // D-개발
        public const bool IsBoard = false;          // B-보드제작업체 (O)
        public const bool IsManuFeildService = true;     // MFS-생산용,필드,서비스 통합
        //public const bool IsField = false;          // F-필드설치용, 서비스용 겸용
    }

    static class Program
    {
        /// <summary>
        /// 해당 애플리케이션의 주 진입점입니다.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Application.ThreadException += new ThreadExceptionEventHandler(MyHandler);
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(MyDomainHandler);

            Application.Run(new DFTMmainForm());
        }


        static void MyHandler(object sender, ThreadExceptionEventArgs e)
        {
            MessageBox.Show("UI쓰레드 예외: " + e.Exception.Message);
        }

        static void MyDomainHandler(object sender, UnhandledExceptionEventArgs e)
        {
            Exception ex = e.ExceptionObject as Exception;
            MessageBox.Show("도메인 예외: " + ex?.Message);
        }
    }
}
