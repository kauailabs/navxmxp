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
    public partial class help_how_to_use : Form
    {
        public string image_link;

        public help_how_to_use()
        {
            InitializeComponent();
        }

        private void help_how_to_use_Load(object sender, EventArgs e)
        {
            Image out_image = Image.FromFile(image_link);
            pictureBox1.Image = out_image;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            help_arduino_code frm_op = new help_arduino_code();
            frm_op.Owner = this;
            frm_op.ShowDialog();
        }
    }
}
