using System;
using System.IO;
using System.Windows.Forms;

namespace DF_TM
{
    public partial class DFTMmainForm
    {
		public LinearMotorValues level1, level2, level3;
		private bool[] 	  bResOK_lv1, bResOK_lv2, bResOK_lv3;
		private bool[] 	  bValueOK_lv1, bValueOK_lv2, bValueOK_lv3;
		private bool	  bRecvEnd_lv1, bRecvEnd_lv2, bRecvEnd_lv3;

		private void LM_JIG_Init()
		{
			level1	=	new LinearMotorValues();
			level2	=	new LinearMotorValues();
			level3	=	new LinearMotorValues();
			ClearLMValue_Validate(1);
			ClearLMValue_Validate(2);
			ClearLMValue_Validate(3);
		}

		private void LM_JIG_SetActivate(bool bActive)
		{
#if (false) // LM_CMD_BTN              
			btn_LM_alloff.Enabled		=	bActive;
			btn_LM_home.Enabled			=	bActive;
			btn_LM_left.Enabled			=	bActive;
			cb_lmLeft_fishLevel.Enabled	=	bActive;
			cb_lmLeft_motorPower.Enabled=	bActive;
			btn_LM_Right.Enabled		=	bActive;
			cb_lmRight_fishLevel.Enabled=	bActive;
			cb_lmRight_motorPower.Enabled=	bActive;
			btn_LM_Return.Enabled		=	bActive;

            btn_load.Enabled			=	bActive;
            tb_LM_loadMsg.Enabled		=	bActive;
            btn_save.Enabled			=	bActive;

            btn_send_lv1.Enabled		=	bActive;
            btn_send_lv2.Enabled		=	bActive;
            btn_send_lv3.Enabled		=	bActive;
            tb_lv1_msg.Enabled			=	bActive;
            tb_lv2_msg.Enabled			=	bActive;
             tb_lv3_msg.Enabled			=	bActive;
#endif
        }
        private void ClearLMValue_Validate(int lv)
		{
			switch(lv)
			{
			case 1: 
				bValueOK_lv1	= new bool[3];
				bResOK_lv1		= new bool[3];
				bValueOK_lv1[0] = false; bValueOK_lv1[1] = false; bValueOK_lv1[2] = false;
				bResOK_lv1[0]	= false; bResOK_lv1[1]	 = false; bResOK_lv1[2]	  = false;
				bRecvEnd_lv1	= false;
				//tb_lv1_msg.Text	=	"";
				break;
			case 2: 
				bValueOK_lv2	= new bool[3];
				bResOK_lv2		= new bool[3];
				bValueOK_lv2[0] = false; bValueOK_lv2[1] = false; bValueOK_lv2[2] = false;
				bResOK_lv2[0]	= false; bResOK_lv2[1]	 = false; bResOK_lv2[2]	  = false;
				bRecvEnd_lv2	= false;
				//tb_lv2_msg.Text	=	"";
				break;
			case 3: 
				bValueOK_lv3	= new bool[3];
				bResOK_lv3		= new bool[3];
				bValueOK_lv3[0] = false; bValueOK_lv3[1] = false; bValueOK_lv3[2] = false;
				bResOK_lv3[0]	= false; bResOK_lv3[1]	 = false; bResOK_lv3[2]	  = false;
				bRecvEnd_lv3	= false;
				//tb_lv3_msg.Text	=	"";
				break;
			}
		}
		private void SetLMValue_Response (int lv, int pow, bool bOK)
		{
			if(pow < 0)	return;
			if(pow > 2) return;
			

			switch(lv)
			{
			case 1:	
				bValueOK_lv1[pow] = bOK;	
				bResOK_lv1[pow]	= true;
				CheckLM_RecvEnd(1);
				break;
			case 2:	
				bValueOK_lv2[pow] = bOK;	
				bResOK_lv2[pow]	= true;
				CheckLM_RecvEnd(2);
				break;
			case 3:	
				bValueOK_lv3[pow] = bOK;	
				bResOK_lv3[pow]	= true;
				CheckLM_RecvEnd(3);
				break;
			}
		}

		private void CheckLM_RecvEnd(int lv)
		{
			switch(lv)
			{
			case 1: bRecvEnd_lv1 = bResOK_lv1[0] && bResOK_lv1[1] && bResOK_lv1[2];
				if(bRecvEnd_lv1) SetLM_Result(1);
				break;
			case 2: bRecvEnd_lv2 = bResOK_lv2[0] && bResOK_lv2[1] && bResOK_lv2[2];
				if(bRecvEnd_lv2) SetLM_Result(2);
				break;
			case 3: bRecvEnd_lv3 = bResOK_lv3[0] && bResOK_lv3[1] && bResOK_lv3[2];
				if(bRecvEnd_lv3) SetLM_Result(3);
				break;
			default :
				return;
			}
			
		}

		private void SetLM_Result(int lv)
		{
			/*
			switch(lv)
			{
			case 1: 
				tb_lv1_msg.Text = bValueOK_lv1[0] && bValueOK_lv1[1] && bValueOK_lv1[2] ? "OK" : "NG";
				break;
			case 2:	
				tb_lv2_msg.Text = bValueOK_lv2[0] && bValueOK_lv2[1] && bValueOK_lv2[2] ? "OK" : "NG";
				break;
			case 3:	
				tb_lv3_msg.Text = bValueOK_lv3[0] && bValueOK_lv3[1] && bValueOK_lv3[2] ? "OK" : "NG";
				break;
			}
			*/
		}

		private void SaveSettings()
		{
		/*
            SaveFileDialog sfd  =   new SaveFileDialog();
            sfd.Title           =   "설정값 저장";
            sfd.Filter          =   "텍스트 파일 | *.txt; | 모든 파일 | *.*";
			sfd.DefaultExt		=	"*.txt";
            if(sfd.ShowDialog() == DialogResult.OK)
            {
				//ValidateData_lv1();
				//ValidateData_lv2();
				//ValidateData_lv3();

                string fileName =   sfd.FileName;
				if(false == fileName.Contains(".txt"))
				{
					fileName = fileName + ".txt";
				}
                using(StreamWriter sw = new StreamWriter(fileName))
                {
					string lv1a	=	string.Format("a,{0},{1},{2},{3},{4},{5}", level1.Lmot_aVal, level1.Lmot_aMs, level1.Bmot_aVal, level1.Bmot_aMs, level1.Tmot_aVal, level1.Tmot_aMs);
					string lv1b	=	string.Format("b,{0},{1},{2},{3},{4},{5}", level1.Lmot_bVal, level1.Lmot_bMs, level1.Bmot_bVal, level1.Bmot_bMs, level1.Tmot_bVal, level1.Tmot_bMs);
					string lv1c	=	string.Format("c,{0},{1},{2},{3},{4},{5}", level1.Lmot_cVal, level1.Lmot_cMs, level1.Bmot_cVal, level1.Bmot_cMs, level1.Tmot_cVal, level1.Tmot_cMs);

					string lv2a	=	string.Format("a,{0},{1},{2},{3},{4},{5}", level2.Lmot_aVal, level2.Lmot_aMs, level2.Bmot_aVal, level2.Bmot_aMs, level2.Tmot_aVal, level2.Tmot_aMs);
					string lv2b	=	string.Format("b,{0},{1},{2},{3},{4},{5}", level2.Lmot_bVal, level2.Lmot_bMs, level2.Bmot_bVal, level2.Bmot_bMs, level2.Tmot_bVal, level2.Tmot_bMs);
					string lv2c	=	string.Format("c,{0},{1},{2},{3},{4},{5}", level2.Lmot_cVal, level2.Lmot_cMs, level2.Bmot_cVal, level2.Bmot_cMs, level2.Tmot_cVal, level2.Tmot_cMs);

					string lv3a	=	string.Format("a,{0},{1},{2},{3},{4},{5}", level3.Lmot_aVal, level3.Lmot_aMs, level3.Bmot_aVal, level3.Bmot_aMs, level3.Tmot_aVal, level3.Tmot_aMs);
					string lv3b	=	string.Format("b,{0},{1},{2},{3},{4},{5}", level3.Lmot_bVal, level3.Lmot_bMs, level3.Bmot_bVal, level3.Bmot_bMs, level3.Tmot_bVal, level3.Tmot_bMs);
					string lv3c	=	string.Format("c,{0},{1},{2},{3},{4},{5}", level3.Lmot_cVal, level3.Lmot_cMs, level3.Bmot_cVal, level3.Bmot_cMs, level3.Tmot_cVal, level3.Tmot_cMs);

                    sw.WriteLine("level1");
					sw.WriteLine(lv1a);
					sw.WriteLine(lv1b);
					sw.WriteLine(lv1c);
					sw.WriteLine("level2");
					sw.WriteLine(lv2a);
					sw.WriteLine(lv2b);
					sw.WriteLine(lv2c);
					sw.WriteLine("level3");
					sw.WriteLine(lv3a);
					sw.WriteLine(lv3b);
					sw.WriteLine(lv3c);
					sw.WriteLine("#end:");
                }
				MessageBox.Show("LM JIG값 저장이 완료되었습니다.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
            }
			else
			{
				MessageBox.Show("올바른 경로를 지정해주세요.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
			}
		*/
		}

		private void LoadSettings()
		{
			OpenFileDialog ofd	=	new OpenFileDialog();
			ofd.Title			=	"LM_JIG용 파일 선택";
			ofd.Filter			=	"텍스트 파일 | *.txt; | 모든 파일 | *.*";
			if(ofd.ShowDialog() != DialogResult.OK)
			{
				MessageBox.Show("올바른 경로를 지정해주세요.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
				return;
			}

			string fileName	=	ofd.FileName;
			if(!File.Exists(fileName))
			{
				MessageBox.Show("올바른 파일경로를 지정해주세요.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
				return;
			}

			var reader = new StreamReader(fileName);
			try
			{
				string lv1Title	=	reader.ReadLine();
				string lv1a		=	reader.ReadLine();
				string lv1b		=	reader.ReadLine();
				string lv1c		=	reader.ReadLine();
				string lv2Title	=	reader.ReadLine();
				string lv2a		=	reader.ReadLine();
				string lv2b		=	reader.ReadLine();
				string lv2c		=	reader.ReadLine();
				string lv3Title	=	reader.ReadLine();
				string lv3a		=	reader.ReadLine();
				string lv3b		=	reader.ReadLine();
				string lv3c		=	reader.ReadLine();

				var lv1aData	=	lv1a.Split(',');
				var lv1bData	=	lv1b.Split(',');
				var lv1cData	=	lv1c.Split(',');
				var lv2aData	=	lv2a.Split(',');
				var lv2bData	=	lv2b.Split(',');
				var lv2cData	=	lv2c.Split(',');
				var lv3aData	=	lv3a.Split(',');
				var lv3bData	=	lv3b.Split(',');
				var lv3cData	=	lv3c.Split(',');
/*
				tb_Lv1_Lmot_aVal.Text = lv1aData[1]; tb_Lv1_Lmot_aMs.Text = lv1aData[2]; tb_Lv1_Bmot_aVal.Text = lv1aData[3]; tb_Lv1_Bmot_aMs.Text = lv1aData[4]; tb_Lv1_Tmot_aVal.Text	= lv1aData[5]; tb_Lv1_Tmot_aMs.Text = lv1aData[6];
				tb_Lv1_Lmot_bVal.Text = lv1bData[1]; tb_Lv1_Lmot_bMs.Text = lv1bData[2]; tb_Lv1_Bmot_bVal.Text = lv1bData[3]; tb_Lv1_Bmot_bMs.Text = lv1bData[4]; tb_Lv1_Tmot_bVal.Text	= lv1bData[5]; tb_Lv1_Tmot_bMs.Text = lv1bData[6];
				tb_Lv1_Lmot_cVal.Text = lv1cData[1]; tb_Lv1_Lmot_cMs.Text = lv1cData[2]; tb_Lv1_Bmot_cVal.Text = lv1cData[3]; tb_Lv1_Bmot_cMs.Text = lv1cData[4]; tb_Lv1_Tmot_cVal.Text	= lv1cData[5]; tb_Lv1_Tmot_cMs.Text = lv1cData[6];

				tb_Lv2_Lmot_aVal.Text = lv2aData[1]; tb_Lv2_Lmot_aMs.Text = lv2aData[2]; tb_Lv2_Bmot_aVal.Text = lv2aData[3]; tb_Lv2_Bmot_aMs.Text = lv2aData[4]; tb_Lv2_Tmot_aVal.Text	= lv2aData[5]; tb_Lv2_Tmot_aMs.Text = lv2aData[6];
				tb_Lv2_Lmot_bVal.Text = lv2bData[1]; tb_Lv2_Lmot_bMs.Text = lv2bData[2]; tb_Lv2_Bmot_bVal.Text = lv2bData[3]; tb_Lv2_Bmot_bMs.Text = lv2bData[4]; tb_Lv2_Tmot_bVal.Text	= lv2bData[5]; tb_Lv2_Tmot_bMs.Text = lv2bData[6];
				tb_Lv2_Lmot_cVal.Text = lv2cData[1]; tb_Lv2_Lmot_cMs.Text = lv2cData[2]; tb_Lv2_Bmot_cVal.Text = lv2cData[3]; tb_Lv2_Bmot_cMs.Text = lv2cData[4]; tb_Lv2_Tmot_cVal.Text	= lv2cData[5]; tb_Lv2_Tmot_cMs.Text = lv2cData[6];

				tb_Lv3_Lmot_aVal.Text = lv3aData[1]; tb_Lv3_Lmot_aMs.Text = lv3aData[2]; tb_Lv3_Bmot_aVal.Text = lv3aData[3]; tb_Lv3_Bmot_aMs.Text = lv3aData[4]; tb_Lv3_Tmot_aVal.Text	= lv3aData[5]; tb_Lv3_Tmot_aMs.Text = lv3aData[6];
				tb_Lv3_Lmot_bVal.Text = lv3bData[1]; tb_Lv3_Lmot_bMs.Text = lv3bData[2]; tb_Lv3_Bmot_bVal.Text = lv3bData[3]; tb_Lv3_Bmot_bMs.Text = lv3bData[4]; tb_Lv3_Tmot_bVal.Text	= lv3bData[5]; tb_Lv3_Tmot_bMs.Text = lv3bData[6];
				tb_Lv3_Lmot_cVal.Text = lv3cData[1]; tb_Lv3_Lmot_cMs.Text = lv3cData[2]; tb_Lv3_Bmot_cVal.Text = lv3cData[3]; tb_Lv3_Bmot_cMs.Text = lv3cData[4]; tb_Lv3_Tmot_cVal.Text	= lv3cData[5]; tb_Lv3_Tmot_cMs.Text = lv3cData[6];
*/
				//ValidateData_lv1();
				//ValidateData_lv2();
				//ValidateData_lv3();
				//tb_LM_loadMsg.Text	=	"OK";
				reader.Close();
				MessageBox.Show("LM JIG값 불러오기가 완료되었습니다.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
			}
			catch(Exception e)
			{
				//tb_LM_loadMsg.Text	=	"NG";
				MessageBox.Show("LM JIG값 불러오기 실패.", "확인",MessageBoxButtons.OK, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1, MessageBoxOptions.DefaultDesktopOnly);
                Console.WriteLine("예외가 발생했습니다: " + e.Message);	// Add ?
            }
			
		}

		/*
		private void ValidateData_lv1()
		{
			level1.Lmot_aVal	=	textParse(tb_Lv1_Lmot_aVal.Text);			tb_Lv1_Lmot_aVal.Text	=	level1.Lmot_aVal.ToString();
			level1.Lmot_bVal	=	textParse(tb_Lv1_Lmot_bVal.Text);			tb_Lv1_Lmot_bVal.Text	=	level1.Lmot_bVal.ToString();
			level1.Lmot_cVal	=	textParse(tb_Lv1_Lmot_cVal.Text, 255, 60);	tb_Lv1_Lmot_cVal.Text	=	level1.Lmot_cVal.ToString();
			level1.Bmot_aVal	=	textParse(tb_Lv1_Bmot_aVal.Text);			tb_Lv1_Bmot_aVal.Text	=	level1.Bmot_aVal.ToString();
			level1.Bmot_bVal	=	textParse(tb_Lv1_Bmot_bVal.Text);			tb_Lv1_Bmot_bVal.Text	=	level1.Bmot_bVal.ToString();
			level1.Bmot_cVal	=	textParse(tb_Lv1_Bmot_cVal.Text, 255, 60);	tb_Lv1_Bmot_cVal.Text	=	level1.Bmot_cVal.ToString();
			level1.Tmot_aVal	=	textParse(tb_Lv1_Tmot_aVal.Text);			tb_Lv1_Tmot_aVal.Text	=	level1.Tmot_aVal.ToString();
			level1.Tmot_bVal	=	textParse(tb_Lv1_Tmot_bVal.Text);			tb_Lv1_Tmot_bVal.Text	=	level1.Tmot_bVal.ToString();
			level1.Tmot_cVal	=	textParse(tb_Lv1_Tmot_cVal.Text, 255, 60);	tb_Lv1_Tmot_cVal.Text	=	level1.Tmot_cVal.ToString();

			level1.Lmot_aMs		=	textParse(tb_Lv1_Lmot_aMs.Text, 999, 100);	tb_Lv1_Lmot_aMs.Text	=	level1.Lmot_aMs.ToString();
			level1.Lmot_bMs		=	textParse(tb_Lv1_Lmot_bMs.Text, 999, 100);	tb_Lv1_Lmot_bMs.Text	=	level1.Lmot_bMs.ToString();
			level1.Lmot_cMs		=	textParse(tb_Lv1_Lmot_cMs.Text, 999, 100);	tb_Lv1_Lmot_cMs.Text	=	level1.Lmot_cMs.ToString();
			level1.Bmot_aMs		=	textParse(tb_Lv1_Bmot_aMs.Text, 999, 100);	tb_Lv1_Bmot_aMs.Text	=	level1.Bmot_aMs.ToString();
			level1.Bmot_bMs		=	textParse(tb_Lv1_Bmot_bMs.Text, 999, 100);	tb_Lv1_Bmot_bMs.Text	=	level1.Bmot_bMs.ToString();
			level1.Bmot_cMs		=	textParse(tb_Lv1_Bmot_cMs.Text, 999, 100);	tb_Lv1_Bmot_cMs.Text	=	level1.Bmot_cMs.ToString();
			level1.Tmot_aMs		=	textParse(tb_Lv1_Tmot_aMs.Text, 999, 100);	tb_Lv1_Tmot_aMs.Text	=	level1.Tmot_aMs.ToString();
			level1.Tmot_bMs		=	textParse(tb_Lv1_Tmot_bMs.Text, 999, 100);	tb_Lv1_Tmot_bMs.Text	=	level1.Tmot_bMs.ToString();
			level1.Tmot_cMs		=	textParse(tb_Lv1_Tmot_cMs.Text, 999, 100);	tb_Lv1_Tmot_cMs.Text	=	level1.Tmot_cMs.ToString();
		}
		private void ValidateData_lv2()
		{
			level2.Lmot_aVal	=	textParse(tb_Lv2_Lmot_aVal.Text);			tb_Lv2_Lmot_aVal.Text	=	level2.Lmot_aVal.ToString();
			level2.Lmot_bVal	=	textParse(tb_Lv2_Lmot_bVal.Text);			tb_Lv2_Lmot_bVal.Text	=	level2.Lmot_bVal.ToString();
			level2.Lmot_cVal	=	textParse(tb_Lv2_Lmot_cVal.Text, 255, 60);	tb_Lv2_Lmot_cVal.Text	=	level2.Lmot_cVal.ToString();
			level2.Bmot_aVal	=	textParse(tb_Lv2_Bmot_aVal.Text);			tb_Lv2_Bmot_aVal.Text	=	level2.Bmot_aVal.ToString();
			level2.Bmot_bVal	=	textParse(tb_Lv2_Bmot_bVal.Text);			tb_Lv2_Bmot_bVal.Text	=	level2.Bmot_bVal.ToString();
			level2.Bmot_cVal	=	textParse(tb_Lv2_Bmot_cVal.Text, 255, 60);	tb_Lv2_Bmot_cVal.Text	=	level2.Bmot_cVal.ToString();
			level2.Tmot_aVal	=	textParse(tb_Lv2_Tmot_aVal.Text);			tb_Lv2_Tmot_aVal.Text	=	level2.Tmot_aVal.ToString();
			level2.Tmot_bVal	=	textParse(tb_Lv2_Tmot_bVal.Text);			tb_Lv2_Tmot_bVal.Text	=	level2.Tmot_bVal.ToString();
			level2.Tmot_cVal	=	textParse(tb_Lv2_Tmot_cVal.Text, 255, 60);	tb_Lv2_Tmot_cVal.Text	=	level2.Tmot_cVal.ToString();

			level2.Lmot_aMs		=	textParse(tb_Lv2_Lmot_aMs.Text, 999, 100);	tb_Lv2_Lmot_aMs.Text	=	level2.Lmot_aMs.ToString();
			level2.Lmot_bMs		=	textParse(tb_Lv2_Lmot_bMs.Text, 999, 100);	tb_Lv2_Lmot_bMs.Text	=	level2.Lmot_bMs.ToString();
			level2.Lmot_cMs		=	textParse(tb_Lv2_Lmot_cMs.Text, 999, 100);	tb_Lv2_Lmot_cMs.Text	=	level2.Lmot_cMs.ToString();
			level2.Bmot_aMs		=	textParse(tb_Lv2_Bmot_aMs.Text, 999, 100);	tb_Lv2_Bmot_aMs.Text	=	level2.Bmot_aMs.ToString();
			level2.Bmot_bMs		=	textParse(tb_Lv2_Bmot_bMs.Text, 999, 100);	tb_Lv2_Bmot_bMs.Text	=	level2.Bmot_bMs.ToString();
			level2.Bmot_cMs		=	textParse(tb_Lv2_Bmot_cMs.Text, 999, 100);	tb_Lv2_Bmot_cMs.Text	=	level2.Bmot_cMs.ToString();
			level2.Tmot_aMs		=	textParse(tb_Lv2_Tmot_aMs.Text, 999, 100);	tb_Lv2_Tmot_aMs.Text	=	level2.Tmot_aMs.ToString();
			level2.Tmot_bMs		=	textParse(tb_Lv2_Tmot_bMs.Text, 999, 100);	tb_Lv2_Tmot_bMs.Text	=	level2.Tmot_bMs.ToString();
			level2.Tmot_cMs		=	textParse(tb_Lv2_Tmot_cMs.Text, 999, 100);	tb_Lv2_Tmot_cMs.Text	=	level2.Tmot_cMs.ToString();

		}
		private void ValidateData_lv3()
		{
			level3.Lmot_aVal	=	textParse(tb_Lv3_Lmot_aVal.Text);			tb_Lv3_Lmot_aVal.Text	=	level3.Lmot_aVal.ToString();
			level3.Lmot_bVal	=	textParse(tb_Lv3_Lmot_bVal.Text);			tb_Lv3_Lmot_bVal.Text	=	level3.Lmot_bVal.ToString();
			level3.Lmot_cVal	=	textParse(tb_Lv3_Lmot_cVal.Text, 255, 60);	tb_Lv3_Lmot_cVal.Text	=	level3.Lmot_cVal.ToString();
			level3.Bmot_aVal	=	textParse(tb_Lv3_Bmot_aVal.Text);			tb_Lv3_Bmot_aVal.Text	=	level3.Bmot_aVal.ToString();
			level3.Bmot_bVal	=	textParse(tb_Lv3_Bmot_bVal.Text);			tb_Lv3_Bmot_bVal.Text	=	level3.Bmot_bVal.ToString();
			level3.Bmot_cVal	=	textParse(tb_Lv3_Bmot_cVal.Text, 255, 60);	tb_Lv3_Bmot_cVal.Text	=	level3.Bmot_cVal.ToString();
			level3.Tmot_aVal	=	textParse(tb_Lv3_Tmot_aVal.Text);			tb_Lv3_Tmot_aVal.Text	=	level3.Tmot_aVal.ToString();
			level3.Tmot_bVal	=	textParse(tb_Lv3_Tmot_bVal.Text);			tb_Lv3_Tmot_bVal.Text	=	level3.Tmot_bVal.ToString();
			level3.Tmot_cVal	=	textParse(tb_Lv3_Tmot_cVal.Text, 255, 60);	tb_Lv3_Tmot_cVal.Text	=	level3.Tmot_cVal.ToString();

			level3.Lmot_aMs		=	textParse(tb_Lv3_Lmot_aMs.Text, 999, 100);	tb_Lv3_Lmot_aMs.Text	=	level3.Lmot_aMs.ToString();
			level3.Lmot_bMs		=	textParse(tb_Lv3_Lmot_bMs.Text, 999, 100);	tb_Lv3_Lmot_bMs.Text	=	level3.Lmot_bMs.ToString();
			level3.Lmot_cMs		=	textParse(tb_Lv3_Lmot_cMs.Text, 999, 100);	tb_Lv3_Lmot_cMs.Text	=	level3.Lmot_cMs.ToString();
			level3.Bmot_aMs		=	textParse(tb_Lv3_Bmot_aMs.Text, 999, 100);	tb_Lv3_Bmot_aMs.Text	=	level3.Bmot_aMs.ToString();
			level3.Bmot_bMs		=	textParse(tb_Lv3_Bmot_bMs.Text, 999, 100);	tb_Lv3_Bmot_bMs.Text	=	level3.Bmot_bMs.ToString();
			level3.Bmot_cMs		=	textParse(tb_Lv3_Bmot_cMs.Text, 999, 100);	tb_Lv3_Bmot_cMs.Text	=	level3.Bmot_cMs.ToString();
			level3.Tmot_aMs		=	textParse(tb_Lv3_Tmot_aMs.Text, 999, 100);	tb_Lv3_Tmot_aMs.Text	=	level3.Tmot_aMs.ToString();
			level3.Tmot_bMs		=	textParse(tb_Lv3_Tmot_bMs.Text, 999, 100);	tb_Lv3_Tmot_bMs.Text	=	level3.Tmot_bMs.ToString();
			level3.Tmot_cMs		=	textParse(tb_Lv3_Tmot_cMs.Text, 999, 100);	tb_Lv3_Tmot_cMs.Text	=	level3.Tmot_cMs.ToString();
		}
		*/
		private int textParse(string str, int maxVal = 255, int defaultVal = 20)
		{
			int res	=	0;
			if(false == Int32.TryParse(str, out res)) return 0;
			if(res > maxVal)	res = defaultVal;
			if(res < 0)			res = 0;

			return res;
		}
		
		/*
		private void tb_Lv1_Lmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_aVal		=	textParse(tb_Lv1_Lmot_aVal.Text);
			tb_Lv1_Lmot_aVal.Text	=	level1.Lmot_aVal.ToString();
		}
		
		private void tb_Lv1_Lmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_aMs			=	textParse(tb_Lv1_Lmot_aMs.Text, 999, 100);
			tb_Lv1_Lmot_aMs.Text	=	level1.Lmot_aMs.ToString();
		}

		private void tb_Lv1_Lmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_bVal		=	textParse(tb_Lv1_Lmot_bVal.Text);
			tb_Lv1_Lmot_bVal.Text	=	level1.Lmot_bVal.ToString();
		}

		private void tb_Lv1_Lmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_bMs			=	textParse(tb_Lv1_Lmot_bMs.Text, 999, 100);
			tb_Lv1_Lmot_bMs.Text	=	level1.Lmot_bMs.ToString();
		}

		private void tb_Lv1_Lmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_cVal		=	textParse(tb_Lv1_Lmot_cVal.Text, 255, 60);
			tb_Lv1_Lmot_cVal.Text	=	level1.Lmot_cVal.ToString();
		}

		private void tb_Lv1_Lmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level1.Lmot_cMs			=	textParse(tb_Lv1_Lmot_cMs.Text, 999, 100);
			tb_Lv1_Lmot_cMs.Text	=	level1.Lmot_cMs.ToString();
		}

		private void tb_Lv1_Bmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_aVal		=	textParse(tb_Lv1_Bmot_aVal.Text);
			tb_Lv1_Bmot_aVal.Text	=	level1.Bmot_aVal.ToString();
		}

		private void tb_Lv1_Bmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_aMs			=	textParse(tb_Lv1_Bmot_aMs.Text, 999, 100);
			tb_Lv1_Bmot_aMs.Text	=	level1.Bmot_aMs.ToString();
		}

		private void tb_Lv1_Bmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_bVal		=	textParse(tb_Lv1_Bmot_bVal.Text);
			tb_Lv1_Bmot_bVal.Text	=	level1.Bmot_bVal.ToString();
		}

		private void tb_Lv1_Bmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_bMs			=	textParse(tb_Lv1_Bmot_bMs.Text, 999, 100);
			tb_Lv1_Bmot_bMs.Text	=	level1.Bmot_bMs.ToString();
		}

		private void tb_Lv1_Bmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_cVal		=	textParse(tb_Lv1_Bmot_cVal.Text, 255, 60);
			tb_Lv1_Bmot_cVal.Text	=	level1.Bmot_cVal.ToString();
		}

		private void tb_Lv1_Bmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level1.Bmot_cMs			=	textParse(tb_Lv1_Bmot_cMs.Text, 999, 100);
			tb_Lv1_Bmot_cMs.Text	=	level1.Bmot_cMs.ToString();
		}

		private void tb_Lv1_Tmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_aVal		=	textParse(tb_Lv1_Tmot_aVal.Text);
			tb_Lv1_Tmot_aVal.Text	=	level1.Tmot_aVal.ToString();
		}

		private void tb_Lv1_Tmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_aMs			=	textParse(tb_Lv1_Tmot_aMs.Text, 999, 100);
			tb_Lv1_Tmot_aMs.Text	=	level1.Tmot_aMs.ToString();
		}

		private void tb_Lv1_Tmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_bVal		=	textParse(tb_Lv1_Tmot_bVal.Text);
			tb_Lv1_Tmot_bVal.Text	=	level1.Tmot_bVal.ToString();
		}

		private void tb_Lv1_Tmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_bMs			=	textParse(tb_Lv1_Tmot_bMs.Text, 999, 100);
			tb_Lv1_Tmot_bMs.Text	=	level1.Tmot_bMs.ToString();
		}

		private void tb_Lv1_Tmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_cVal		=	textParse(tb_Lv1_Tmot_cVal.Text, 255, 60);
			tb_Lv1_Tmot_cVal.Text	=	level1.Tmot_cVal.ToString();

		}
		private void tb_Lv1_Tmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level1.Tmot_cMs			=	textParse(tb_Lv1_Tmot_cMs.Text, 999, 100);
			tb_Lv1_Tmot_cMs.Text	=	level1.Tmot_cMs.ToString();
		}

		private void tb_Lv2_Lmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_aVal		=	textParse(tb_Lv2_Lmot_aVal.Text);
			tb_Lv2_Lmot_aVal.Text	=	level2.Lmot_aVal.ToString();
		}

		private void tb_Lv2_Lmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_aMs			=	textParse(tb_Lv2_Lmot_aMs.Text, 999, 100);
			tb_Lv2_Lmot_aMs.Text	=	level2.Lmot_aMs.ToString();
		}

		private void tb_Lv2_Lmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_bVal		=	textParse(tb_Lv2_Lmot_bVal.Text);
			tb_Lv2_Lmot_bVal.Text	=	level2.Lmot_bVal.ToString();
		}

		private void tb_Lv2_Lmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_bMs			=	textParse(tb_Lv2_Lmot_bMs.Text, 999, 100);
			tb_Lv2_Lmot_bMs.Text	=	level2.Lmot_bMs.ToString();
		}

		private void tb_Lv2_Lmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_cVal		=	textParse(tb_Lv2_Lmot_cVal.Text, 255, 60);
			tb_Lv2_Lmot_cVal.Text	=	level2.Lmot_cVal.ToString();
		}

		private void tb_Lv2_Lmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level2.Lmot_cMs			=	textParse(tb_Lv2_Lmot_cMs.Text, 999, 100);
			tb_Lv2_Lmot_cMs.Text	=	level2.Lmot_cMs.ToString();
		}

		private void tb_Lv2_Bmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_aVal		=	textParse(tb_Lv2_Bmot_aVal.Text);
			tb_Lv2_Bmot_aVal.Text	=	level2.Bmot_aVal.ToString();
		}

		private void tb_Lv2_Bmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_aMs			=	textParse(tb_Lv2_Bmot_aMs.Text, 999, 100);
			tb_Lv2_Bmot_aMs.Text	=	level2.Bmot_aMs.ToString();
		}

		private void tb_Lv2_Bmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_bVal		=	textParse(tb_Lv2_Bmot_bVal.Text);
			tb_Lv2_Bmot_bVal.Text	=	level2.Bmot_bVal.ToString();
		}

		private void tb_Lv2_Bmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_bMs			=	textParse(tb_Lv2_Bmot_bMs.Text, 999, 100);
			tb_Lv2_Bmot_bMs.Text	=	level2.Bmot_bMs.ToString();
		}

		private void tb_Lv2_Bmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_cVal		=	textParse(tb_Lv2_Bmot_cVal.Text, 255, 60);
			tb_Lv2_Bmot_cVal.Text	=	level2.Bmot_cVal.ToString();
		}

		private void tb_Lv2_Bmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level2.Bmot_cMs			=	textParse(tb_Lv2_Bmot_cMs.Text, 999, 100);
			tb_Lv2_Bmot_cMs.Text	=	level2.Bmot_cMs.ToString();
		}

		private void tb_Lv2_Tmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_aVal		=	textParse(tb_Lv2_Tmot_aVal.Text);
			tb_Lv2_Tmot_aVal.Text	=	level2.Tmot_aVal.ToString();
		}

		private void tb_Lv2_Tmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_aMs			=	textParse(tb_Lv2_Tmot_aMs.Text, 999, 100);
			tb_Lv2_Tmot_aMs.Text	=	level2.Tmot_aMs.ToString();
		}

		private void tb_Lv2_Tmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_bVal		=	textParse(tb_Lv2_Tmot_bVal.Text);
			tb_Lv2_Tmot_bVal.Text	=	level2.Tmot_bVal.ToString();
		}

		private void tb_Lv2_Tmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_bMs			=	textParse(tb_Lv2_Tmot_bMs.Text, 999, 100);
			tb_Lv2_Tmot_bMs.Text	=	level2.Tmot_bMs.ToString();
		}

		private void tb_Lv2_Tmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_cVal		=	textParse(tb_Lv2_Tmot_cVal.Text, 255, 60);
			tb_Lv2_Tmot_cVal.Text	=	level2.Tmot_cVal.ToString();

		}
		private void tb_Lv2_Tmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level2.Tmot_cMs			=	textParse(tb_Lv2_Tmot_cMs.Text, 999, 100);
			tb_Lv2_Tmot_cMs.Text	=	level2.Tmot_cMs.ToString();
		}

		private void tb_Lv3_Lmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_aVal		=	textParse(tb_Lv3_Lmot_aVal.Text);
			tb_Lv3_Lmot_aVal.Text	=	level3.Lmot_aVal.ToString();
		}

		private void tb_Lv3_Lmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_aMs			=	textParse(tb_Lv3_Lmot_aMs.Text, 999, 100);
			tb_Lv3_Lmot_aMs.Text	=	level3.Lmot_aMs.ToString();
		}

		private void tb_Lv3_Lmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_bVal		=	textParse(tb_Lv3_Lmot_bVal.Text);
			tb_Lv3_Lmot_bVal.Text	=	level3.Lmot_bVal.ToString();
		}

		private void tb_Lv3_Lmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_bMs			=	textParse(tb_Lv3_Lmot_bMs.Text, 999, 100);
			tb_Lv3_Lmot_bMs.Text	=	level3.Lmot_bMs.ToString();
		}

		private void tb_Lv3_Lmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_cVal		=	textParse(tb_Lv3_Lmot_cVal.Text, 255, 60);
			tb_Lv3_Lmot_cVal.Text	=	level3.Lmot_cVal.ToString();
		}

		private void tb_Lv3_Lmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level3.Lmot_cMs			=	textParse(tb_Lv3_Lmot_cMs.Text, 999, 100);
			tb_Lv3_Lmot_cMs.Text	=	level3.Lmot_cMs.ToString();
		}

		private void tb_Lv3_Bmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_aVal		=	textParse(tb_Lv3_Bmot_aVal.Text);
			tb_Lv3_Bmot_aVal.Text	=	level3.Bmot_aVal.ToString();
		}

		private void tb_Lv3_Bmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_aMs			=	textParse(tb_Lv3_Bmot_aMs.Text, 999, 100);
			tb_Lv3_Bmot_aMs.Text	=	level3.Bmot_aMs.ToString();
		}

		private void tb_Lv3_Bmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_bVal		=	textParse(tb_Lv3_Bmot_bVal.Text);
			tb_Lv3_Bmot_bVal.Text	=	level3.Bmot_bVal.ToString();
		}

		private void tb_Lv3_Bmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_bMs			=	textParse(tb_Lv3_Bmot_bMs.Text, 999, 100);
			tb_Lv3_Bmot_bMs.Text	=	level3.Bmot_bMs.ToString();
		}

		private void tb_Lv3_Bmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_cVal		=	textParse(tb_Lv3_Bmot_cVal.Text, 255, 60);
			tb_Lv3_Bmot_cVal.Text	=	level3.Bmot_cVal.ToString();
		}

		private void tb_Lv3_Bmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level3.Bmot_cMs			=	textParse(tb_Lv3_Bmot_cMs.Text, 999, 100);
			tb_Lv3_Bmot_cMs.Text	=	level3.Bmot_cMs.ToString();
		}

		private void tb_Lv3_Tmot_aVal_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_aVal		=	textParse(tb_Lv3_Tmot_aVal.Text);
			tb_Lv3_Tmot_aVal.Text	=	level3.Tmot_aVal.ToString();
		}

		private void tb_Lv3_Tmot_aMs_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_aMs			=	textParse(tb_Lv3_Tmot_aMs.Text, 999, 100);
			tb_Lv3_Tmot_aMs.Text	=	level3.Tmot_aMs.ToString();
		}

		private void tb_Lv3_Tmot_bVal_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_bVal		=	textParse(tb_Lv3_Tmot_bVal.Text);
			tb_Lv3_Tmot_bVal.Text	=	level3.Tmot_bVal.ToString();
		}

		private void tb_Lv3_Tmot_bMs_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_bMs			=	textParse(tb_Lv3_Tmot_bMs.Text, 999, 100);
			tb_Lv3_Tmot_bMs.Text	=	level3.Tmot_bMs.ToString();
		}

		private void tb_Lv3_Tmot_cVal_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_cVal		=	textParse(tb_Lv3_Tmot_cVal.Text, 255, 60);
			tb_Lv3_Tmot_cVal.Text	=	level3.Tmot_cVal.ToString();

		}

		private void tb_Lv3_Tmot_cMs_TextChanged (object sender, EventArgs e)
		{
			level3.Tmot_cMs			=	textParse(tb_Lv3_Tmot_cMs.Text, 999, 100);
			tb_Lv3_Tmot_cMs.Text	=	level3.Tmot_cMs.ToString();
		}
		*/
		private void btn_save_Click (object sender, EventArgs e)
		{
			SaveSettings();
		}

		private void btn_load_Click (object sender, EventArgs e)
		{
			LoadSettings();
		}

		private void btn_Send_lv1_Click (object sender, EventArgs e)
		{
			//ValidateData_lv1();
			ClearLMValue_Validate(1);
			string dataFormA	=	$"$9911{level1.Lmot_aVal:000}{level1.Lmot_aMs:000}{level1.Bmot_aVal:000}{level1.Bmot_aMs:000}{level1.Tmot_aVal:000}{level1.Tmot_aMs:000}%";
			string dataFormB	=	$"$9912{level1.Lmot_bVal:000}{level1.Lmot_bMs:000}{level1.Bmot_bVal:000}{level1.Bmot_bMs:000}{level1.Tmot_bVal:000}{level1.Tmot_bMs:000}%";
			string dataFormC	=	$"$9913{level1.Lmot_cVal:000}{level1.Lmot_cMs:000}{level1.Bmot_cVal:000}{level1.Bmot_cMs:000}{level1.Tmot_cVal:000}{level1.Tmot_cMs:000}%";
			uartSerial.WriteMessage(dataFormA);
			uartSerial.WriteMessage(dataFormB);
			uartSerial.WriteMessage(dataFormC);
		}

		private void btn_Send_lv2_Click (object sender, EventArgs e)
		{
			//ValidateData_lv2();
			ClearLMValue_Validate(2);
			string dataFormA	=	$"$9921{level2.Lmot_aVal:000}{level2.Lmot_aMs:000}{level2.Bmot_aVal:000}{level2.Bmot_aMs:000}{level2.Tmot_aVal:000}{level2.Tmot_aMs:000}%";
			string dataFormB	=	$"$9922{level2.Lmot_bVal:000}{level2.Lmot_bMs:000}{level2.Bmot_bVal:000}{level2.Bmot_bMs:000}{level2.Tmot_bVal:000}{level2.Tmot_bMs:000}%";
			string dataFormC	=	$"$9923{level2.Lmot_cVal:000}{level2.Lmot_cMs:000}{level2.Bmot_cVal:000}{level2.Bmot_cMs:000}{level2.Tmot_cVal:000}{level2.Tmot_cMs:000}%";
			uartSerial.WriteMessage(dataFormA);
			uartSerial.WriteMessage(dataFormB);
			uartSerial.WriteMessage(dataFormC);
		}

		private void btn_Send_lv3_Click (object sender, EventArgs e)
		{
			//ValidateData_lv3();
			ClearLMValue_Validate(3);
			string dataFormA	=	$"$9931{level3.Lmot_aVal:000}{level3.Lmot_aMs:000}{level3.Bmot_aVal:000}{level3.Bmot_aMs:000}{level3.Tmot_aVal:000}{level3.Tmot_aMs:000}%";
			string dataFormB	=	$"$9932{level3.Lmot_bVal:000}{level3.Lmot_bMs:000}{level3.Bmot_bVal:000}{level3.Bmot_bMs:000}{level3.Tmot_bVal:000}{level3.Tmot_bMs:000}%";
			string dataFormC	=	$"$9933{level3.Lmot_cVal:000}{level3.Lmot_cMs:000}{level3.Bmot_cVal:000}{level3.Bmot_cMs:000}{level3.Tmot_cVal:000}{level3.Tmot_cMs:000}%";
			uartSerial.WriteMessage(dataFormA);
			uartSerial.WriteMessage(dataFormB);
			uartSerial.WriteMessage(dataFormC);
		}

#if (false) // LM_CMD_BTN              

		private void btn_LM_alloff_Click (object sender, EventArgs e)
		{
            string data = "$0750%";
            uartSerial.WriteMessage(data);
		}
		private void btn_LM_home_Click (object sender, EventArgs e)
		{
            string data = "$0751%";
            uartSerial.WriteMessage(data);
		}

		private void btn_LM_left_Click (object sender, EventArgs e)
		{
            int fishlv  =   Int32.Parse(cb_lmLeft_fishLevel.Text);
            int powType =   cb_lmLeft_motorPower.SelectedIndex + 1;
            
            string data = $"$0752{fishlv}{powType}%";
            uartSerial.WriteMessage(data);
		}

		private void btn_LM_Right_Click (object sender, EventArgs e)
		{
            int fishlv  =   Int32.Parse(cb_lmRight_fishLevel.Text);
            int powType =   cb_lmRight_motorPower.SelectedIndex + 1;
            
            string data = $"$0753{fishlv}{powType}%";
            uartSerial.WriteMessage(data);
		}

		private void btn_LM_Return_Click (object sender, EventArgs e)
		{
            int val     = (int)numUpdown_lm_Return.Value;
            string data = $"$0754{val:000}%";
            uartSerial.WriteMessage(data);
		}
#endif

    }
}
