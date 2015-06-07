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
    public partial class serial_port_help_from : Form
    {
        public string help_text;

        public serial_port_help_from()
        {
            InitializeComponent();
        }

        private void serial_port_help_from_Load(object sender, EventArgs e)
        {
            textBox1.Text = help_text;
            textBox1.AppendText("\r\n");
        }


    }
}
