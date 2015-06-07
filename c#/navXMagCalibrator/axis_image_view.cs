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
    public partial class axis_image_view : Form
    {
        public string text;
        public string image_link;

        public axis_image_view()
        {
            InitializeComponent();
        }

        private void axis_image_view_Load(object sender, EventArgs e)
        {
            message_label.Text = text;
            Image out_image = Image.FromFile(image_link);
            pictureBox1.Image = out_image;
        }
    }
}
