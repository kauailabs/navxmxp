using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace compass_calibrator
{
    public partial class help_arduino_code : Form
    {
        public string help_text;
        string curDir;

        public help_arduino_code()
        {
            InitializeComponent();
        }

        private void help_arduino_code_Load(object sender, EventArgs e)
        {
            //про текущаю папку
            curDir = System.IO.Path.GetDirectoryName(
            System.Reflection.Assembly.GetExecutingAssembly().GetModules()[0].FullyQualifiedName);

            string help_text = System.IO.File.ReadAllText(curDir + "\\MagMaster Files\\texts\\" + "acode.txt");
            textBox1.Text = help_text;
            textBox1.AppendText("\r\n");

        }
    }
}
