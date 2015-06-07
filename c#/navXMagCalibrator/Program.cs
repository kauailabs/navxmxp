using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace compass_calibrator
{
    static class Program
    {
        /// <summary>
        /// Main Program Class
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Form1());
        }
    }
}
