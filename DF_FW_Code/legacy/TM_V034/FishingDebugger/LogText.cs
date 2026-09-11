using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;

namespace DF_TM
{
    public class LogText
    {
        private static string logFileName = Path.Combine(Application.StartupPath, "logs", "log"+ DateTime.Now.ToString("yyyy-MMdd_HH")+".txt");
        //private static string ComlogFileName = Path.Combine(Application.StartupPath, "logs", "comlog" + DateTime.Now.ToString("yyyy-MM-dd_HH:mm") + ".txt");

        private readonly DFTMmainForm mainForm;
        public LogText(DFTMmainForm form)
        {
            mainForm = form;
        }

        public static void Log(string message)
        {
            try
            {
                string folder = Path.GetDirectoryName(logFileName);
                if (!Directory.Exists(folder))
                    Directory.CreateDirectory(folder);

                string timeStamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
                string fullMessage = $"[{timeStamp}] {message}{Environment.NewLine}";
                File.AppendAllText(logFileName, fullMessage);
            }
            catch (Exception ex)
            {
                Console.WriteLine("에러 로그 기록: " + ex.Message);
            }
        }


        /*
        public static void comLog(string message)
        {
            try
            {
                string folder = Path.GetDirectoryName(ComlogFileName);
                if (!Directory.Exists(folder))
                    Directory.CreateDirectory(folder);

                string timeStamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
                string fullMessage = $"[{timeStamp}] {message}{Environment.NewLine}";
                File.AppendAllText(ComlogFileName, fullMessage);
            }
            catch (Exception ex)
            {
                Console.WriteLine("에러 통신로그 기록: " + ex.Message);
            }
        }
        */

    }
}
