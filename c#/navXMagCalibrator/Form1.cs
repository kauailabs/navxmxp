using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO.Ports;
using System.Threading;
using System.IO;
using System.Globalization;
using navXComUtilities;

namespace compass_calibrator
{

    public partial class Form1 : Form
    {
        static Object bufferLock = new Object();
        static Byte[] bytes_from_usart = null;
        static int num_bytes_from_usart = 0;
        static int bytes_from_usart_offset = 0;
        static Boolean port_close_flag;
        double X_serial_value, Y_serial_value, Z_serial_value;
        int empty_serial_data_counter;
        string curDir;
        string symbol_mask = "1234567890.-";
#if DEBUG
        StreamWriter writer;
#endif
        Int16 last_mag_x;
        Int16 last_mag_y;
        Int16 last_mag_z;
        public Form1()
        {
            InitializeComponent();
#if DEBUG
            writer = new StreamWriter("nav6datalog.txt", true);
#endif
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //change regional standard to en-US
            CultureInfo en = new CultureInfo("en-US");
            Thread.CurrentThread.CurrentCulture = en;

            //current dir name
            curDir = System.IO.Path.GetDirectoryName(
            System.Reflection.Assembly.GetExecutingAssembly().GetModules()[0].FullyQualifiedName);

            stop_button.Enabled = false;
            point_buttons_enable(false);
            navXComHelper.InitPortComboBox(comboBox1);
        }

        private void point_buttons_enable(Boolean b_enable)
        {
            if (b_enable)
            {
                buttonXplus_0.Enabled = true;
                buttonXplus_180.Enabled = true;
                buttonXminus_0.Enabled = true;
                buttonXminus_180.Enabled = true;
                buttonYplus_0.Enabled = true;
                buttonYplus_180.Enabled = true;
                buttonYminus_0.Enabled = true;
                buttonYminus_180.Enabled = true;
                buttonZplus_0.Enabled = true;
                buttonZplus_180.Enabled = true;
                buttonZminus_0.Enabled = true;
                buttonZminus_180.Enabled = true;
            }
            else
            {
                buttonXplus_0.Enabled = false;
                buttonXplus_180.Enabled = false;
                buttonXminus_0.Enabled = false;
                buttonXminus_180.Enabled = false;
                buttonYplus_0.Enabled = false;
                buttonYplus_180.Enabled = false;
                buttonYminus_0.Enabled = false;
                buttonYminus_180.Enabled = false;
                buttonZplus_0.Enabled = false;
                buttonZplus_180.Enabled = false;
                buttonZminus_0.Enabled = false;
                buttonZminus_180.Enabled = false;
            }
        }

        private void start_button_Click(object sender, EventArgs e)
        {
            try
            {
                port.PortName = comboBox1.SelectedItem.ToString();
                //port opening
                port_close_flag = false;
                // Set the read/write timeouts
                port.ReadTimeout = 500;
                port.WriteTimeout = 500;
                port.Open();
                stop_button.Enabled = true;
                point_buttons_enable(true);
                start_button.Enabled = false;
                comboBox1.Enabled = false;
                port.DiscardInBuffer();
                send_board_identity_request();
                System.Threading.Thread.Sleep(100);
                send_ahrs_stream_request();
                //for the handler
                port.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                //timer activation
                timer1.Enabled = true;
            }
            catch
            {
                close_port();
                MessageBox.Show("Empty port name.", "Warning!");
            }
        }

        private void stop_button_Click(object sender, EventArgs e)
        {
            close_port();
        }

        void close_port()
        {
            //timet deactivation
            timer1.Enabled = false;
            //port closing
            port_close_flag = true;
            stop_button.Enabled = false;
            point_buttons_enable(false);
            Thread.Sleep(500);
            try
            {
                port.Close();
            }
            catch (Exception)
            {
            }
            start_button.Enabled = true;
            comboBox1.Enabled = true;
            empty_serial_data_counter = 0;
            bytes_from_usart = null;
            num_bytes_from_usart = 0;
            X_serial_value = 0;
            Y_serial_value = 0;
            Z_serial_value = 0;
            Xlabel.Text = "X = " + X_serial_value.ToString("0.###");
            Ylabel.Text = "Y = " + Y_serial_value.ToString("0.###");
            Zlabel.Text = "Z = " + Z_serial_value.ToString("0.###");
        }

        private static void DataReceivedHandler(object sender, SerialDataReceivedEventArgs e)
        {
            if (!port_close_flag)
            {
                SerialPort sp = (SerialPort)sender;
                try
                {
                    lock (Form1.bufferLock)
                    {
                        int bytes_available = sp.BytesToRead;
                        Byte[] buf = new Byte[bytes_available];
                        sp.Read(buf, 0, bytes_available);
                        if ( bytes_from_usart != null ) 
                        {
                            /* older, unprocessed data still exists. Append new data */
                            Byte[] bufexpanded = new Byte[num_bytes_from_usart + bytes_available];
                            System.Buffer.BlockCopy(bytes_from_usart, 0, bufexpanded, 0, num_bytes_from_usart);
                            System.Buffer.BlockCopy(buf, 0, bufexpanded, num_bytes_from_usart, bytes_available);
                            bytes_available = num_bytes_from_usart + bytes_available;
                            buf = bufexpanded;
                        }
                        for (int i = 0; i < bytes_available; i++)
                        {
                            if (buf[i] == Convert.ToByte('!'))
                            {
                                //string indata = sp.ReadLine();
                                //string_from_usart = indata;
                                bytes_from_usart = buf;
                                num_bytes_from_usart = bytes_available;
                                bytes_from_usart_offset = i;
                                break;
                            }
                        }
                    }
                }
                catch (Exception)
                {
                }
            }
        }

        const int navx_ahrs_update_length = 66;
        const char navx_ahrs_update_msg_type = 'a';
        const char navx_msg_start_char = '!';
        const char navx_binary_msg_indicator = '#';
        const char navx_mag_cal_data_msg_type = 'M';
        const int navx_mag_cal_data_msg_length = 55;
        const char navx_board_id_msg_type = 'i';
        const int navx_board_id_msg_length = 26;
        const char navx_tuning_var_msg_type = 'T';
        const int navx_tuning_var_msg_length = 14;

        private void var_refresh()
        {
            /* TODO:  This portion should be a critical section, to ensure */
            /* receival of new bytes during the following block is prohibited */
            byte[] usart_bytes;
            int usart_data_offset;
            int n_bytes_from_usart;
            lock (bufferLock)
            {
                if (bytes_from_usart == null) return;
                if (num_bytes_from_usart == 0) return;
                n_bytes_from_usart = num_bytes_from_usart;
                usart_data_offset = bytes_from_usart_offset;
                usart_bytes = new byte[n_bytes_from_usart];
                System.Buffer.BlockCopy(bytes_from_usart, 0, usart_bytes, 0, n_bytes_from_usart);
                bytes_from_usart = null;
            }
            /* End of proposed critical section */
            num_bytes_from_usart = 0;
            try
            {
                int valid_bytes_available = n_bytes_from_usart - bytes_from_usart_offset;
                bool end_of_data = false;
                while ( !end_of_data ) 
                {
                    if ((usart_bytes != null) && (valid_bytes_available >= 2))
                    {
                        if ( (usart_bytes[usart_data_offset] == Convert.ToByte(navx_msg_start_char) ) &&
                             (usart_bytes[usart_data_offset+1] == Convert.ToByte(navx_binary_msg_indicator) ) )
                        {
                            /* Valid packet start found */
                            if ( (usart_bytes[usart_data_offset + 2] == navx_ahrs_update_length-2) &&
                                 (usart_bytes[usart_data_offset + 3] == Convert.ToByte(navx_ahrs_update_msg_type)))
                            {
                                /* AHRS Update packet received */
                                byte[] bytes = new byte[navx_ahrs_update_length];
                                System.Buffer.BlockCopy(usart_bytes, usart_data_offset, bytes, 0, navx_ahrs_update_length);
                                valid_bytes_available -= navx_ahrs_update_length;
                                usart_data_offset += navx_ahrs_update_length;

                                float yawval            = signedHundredthsToFloat(bytes, 4);
                                float rollval           = signedHundredthsToFloat(bytes, 6);
                                float pitchval          = signedHundredthsToFloat(bytes, 8);
                                float heading           = unsignedHudredthsToFloat(bytes, 10);
                                float altitude          = text1616FloatToFloat(bytes, 12);
                                float fused_heading     = unsignedHudredthsToFloat(bytes, 16);
                                float linear_accel_x    = signedThousandthsToFloat(bytes, 18);
                                float linear_accel_y    = signedThousandthsToFloat(bytes, 20);
                                float linear_accel_z    = signedThousandthsToFloat(bytes, 22);
                                Int16 cal_mag_x         = BitConverter.ToInt16(bytes, 24);
                                Int16 cal_mag_y         = BitConverter.ToInt16(bytes, 26);
                                Int16 cal_mag_z         = BitConverter.ToInt16(bytes, 28);
                                float mag_norm_ratio    = unsignedHudredthsToFloat(bytes, 30);
                                float mag_norm_scalar   = text1616FloatToFloat(bytes, 32);
                                float mpu_temp          = signedHundredthsToFloat(bytes, 36);
                                Int16 raw_mag_x         = BitConverter.ToInt16(bytes, 38);
                                Int16 raw_mag_y         = BitConverter.ToInt16(bytes, 40);
                                Int16 raw_mag_z         = BitConverter.ToInt16(bytes, 42);
                                Int16 quat_w            = BitConverter.ToInt16(bytes, 44);
                                Int16 quat_x            = BitConverter.ToInt16(bytes, 46);
                                Int16 quat_y            = BitConverter.ToInt16(bytes, 48);
                                Int16 quat_z            = BitConverter.ToInt16(bytes, 50);
                                float baro_pressure     = text1616FloatToFloat(bytes, 52);
                                float baro_temp         = signedHundredthsToFloat(bytes, 56);
                                byte op_status          = bytes[58];
                                byte sensor_status      = bytes[59];
                                byte cal_status         = bytes[60];
                                byte selftest_status    = bytes[61];

                                X_serial_value = (double)raw_mag_x;
                                Y_serial_value = (double)raw_mag_y;
                                Z_serial_value = (double)raw_mag_z;

                                pitch.Text = pitchval.ToString();
                                roll.Text = rollval.ToString();
                                mag_field_norm_ratio.Text = mag_norm_ratio.ToString();

                                if ((last_mag_x != raw_mag_x) ||
                                     (last_mag_y != raw_mag_y) ||
                                     (last_mag_z != raw_mag_z))
                                {
                                    String row = raw_mag_x + "," + raw_mag_y + "," + raw_mag_z;
#if DEBUG                                    
                                    writer.WriteLine(row);
#endif
                                    last_mag_x = raw_mag_x;
                                    last_mag_y = raw_mag_y;
                                    last_mag_z = raw_mag_z;
                                }
                            }
                            else if ((usart_bytes[usart_data_offset + 2] == navx_mag_cal_data_msg_length - 2) &&
                                     (usart_bytes[usart_data_offset + 3] == Convert.ToByte(navx_mag_cal_data_msg_type)))
                                {
                                /* Mag Cal Data Response received */
                                byte[] bytes = new byte[navx_mag_cal_data_msg_length];
                                System.Buffer.BlockCopy(usart_bytes, usart_data_offset, bytes, 0, navx_mag_cal_data_msg_length);
                                valid_bytes_available -= navx_mag_cal_data_msg_length;
                                usart_data_offset += navx_mag_cal_data_msg_length;

                                Int16 bias_x = BitConverter.ToInt16(bytes, 5);
                                Int16 bias_y = BitConverter.ToInt16(bytes, 7);
                                Int16 bias_z = BitConverter.ToInt16(bytes, 9);
                                float xform_x_x = text1616FloatToFloat(bytes, 11);
                                float xform_x_y = text1616FloatToFloat(bytes, 15);
                                float xform_x_z = text1616FloatToFloat(bytes, 19);
                                float xform_y_x = text1616FloatToFloat(bytes, 23);
                                float xform_y_y = text1616FloatToFloat(bytes, 27);
                                float xform_y_z = text1616FloatToFloat(bytes, 31);
                                float xform_z_x = text1616FloatToFloat(bytes, 35);
                                float xform_z_y = text1616FloatToFloat(bytes, 39);
                                float xform_z_z = text1616FloatToFloat(bytes, 43);

                                textBox_biasX.Text = bias_x.ToString();
                                textBox_biasY.Text = bias_y.ToString();
                                textBox_biasZ.Text = bias_z.ToString();
                                textBox_matrixX_x.Text = xform_x_x.ToString();
                                textBox_matrixX_y.Text = xform_x_y.ToString();
                                textBox_matrixX_z.Text = xform_x_z.ToString();
                                textBox_matrixY_x.Text = xform_y_x.ToString();
                                textBox_matrixY_y.Text = xform_y_y.ToString();
                                textBox_matrixY_z.Text = xform_y_z.ToString();
                                textBox_matrixZ_x.Text = xform_z_x.ToString();
                                textBox_matrixZ_y.Text = xform_z_y.ToString();
                                textBox_matrixZ_z.Text = xform_z_z.ToString();
                            }
                            else if ((usart_bytes[usart_data_offset + 2] == navx_board_id_msg_length - 2) &&
                                     (usart_bytes[usart_data_offset + 3] == Convert.ToByte(navx_board_id_msg_type)))
                            {
                                /* Mag Cal Data Response received */
                                byte[] bytes = new byte[navx_board_id_msg_length];
                                System.Buffer.BlockCopy(usart_bytes, usart_data_offset, bytes, 0, navx_board_id_msg_length);
                                valid_bytes_available -= navx_board_id_msg_length;
                                usart_data_offset += navx_board_id_msg_length;
                                byte boardtype = bytes[4];
                                byte hwrev = bytes[5];
                                byte fw_major = bytes[6];
                                byte fw_minor = bytes[7];
                                UInt16 fw_revision = BitConverter.ToUInt16(bytes, 8);
                                byte[] unique_id = new byte[12];
                                for (int i = 0; i < 12; i++)
                                {
                                    unique_id[i] = bytes[10 + i];
                                }
                                string boardtype_string = "unknown";
                                if (hwrev == 33)
                                {
                                    boardtype_string = "navX-MXP";
                                }
                                else if (hwrev == 40) {
                                    boardtype_string = "navX-Micro";
                                }
                                string msg = "Board type:  " + boardtype_string + " (" + boardtype + ")\n" +
                                                 "H/W Rev:  " + hwrev + "\n" +
                                                 "F/W Rev:  " + fw_major + "." + fw_minor + "." + fw_revision + "\n" +
                                                 "Unique ID:  ";
                                msg += BitConverter.ToString(unique_id);
                                MessageBox.Show(msg, "Kauai Labs navX-Model Board ID");
                             }
                            else
                            {
                                // Start of packet found, but not wanted
                                valid_bytes_available -= 1;
                                usart_data_offset += 1;
                                // Keep scanning through the remainder of the buffer
                            }
                        }
                        else 
                        {
                            // Data available, but first char is not a valid start of message.
                            // Keep scanning through the remainder of the buffer
                            valid_bytes_available -= 1;
                            usart_data_offset += 1;
                        }
                    }
                    else 
                    {
                        // At end of buffer, stop scanning
                        end_of_data = true;
                    }
                    /*
                    string[] serial_values = string_from_usart.Split(',');
                    if (serial_values[0] != "" && serial_values[1] != "" && serial_values[2] != "")
                    {
                        X_serial_value = double.Parse(serial_values[0]);
                        Y_serial_value = double.Parse(serial_values[1]);
                        Z_serial_value = double.Parse(serial_values[2]);
                    }*/
                }
                //empty_serial_data_counter++;
                if (empty_serial_data_counter >= 10)
                {
                    close_port();
                    MessageBox.Show("No serial data.", "Warning!");
                }
            }
            catch ( Exception )
            {
                close_port();
                MessageBox.Show("Serial port error.", "Warning!");
            }
        }

        private void indication()
        {
            Xlabel.Text = "X = " + X_serial_value.ToString("0.###");
            Ylabel.Text = "Y = " + Y_serial_value.ToString("0.###");
            Zlabel.Text = "Z = " + Z_serial_value.ToString("0.###");
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            var_refresh();
            indication();
        }

        private void comboBox1_DropDown(object sender, EventArgs e)
        {
            navXComHelper.HandleComboBoxDropDownEvent(comboBox1);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            close_port();
        }

        private void buttonXplus_0_Click(object sender, EventArgs e)
        {
            textBoxXplus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxXplus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxXplus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonXplus_180_Click(object sender, EventArgs e)
        {
            textBoxXplus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxXplus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxXplus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonXminus_0_Click(object sender, EventArgs e)
        {
            textBoxXminus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxXminus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxXminus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonXminus_180_Click(object sender, EventArgs e)
        {
            textBoxXminus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxXminus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxXminus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonYplus_0_Click(object sender, EventArgs e)
        {
            textBoxYplus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxYplus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxYplus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonYplus_180_Click(object sender, EventArgs e)
        {
            textBoxYplus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxYplus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxYplus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonZplus_0_Click(object sender, EventArgs e)
        {
            textBoxZplus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxZplus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxZplus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonZplus_180_Click(object sender, EventArgs e)
        {
            textBoxZplus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxZplus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxZplus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonYminus_0_Click(object sender, EventArgs e)
        {
            textBoxYminus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxYminus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxYminus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonYminus_180_Click(object sender, EventArgs e)
        {
            textBoxYminus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxYminus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxYminus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonZminus_0_Click(object sender, EventArgs e)
        {
            textBoxZminus_X_0.Text = X_serial_value.ToString("0.###");
            textBoxZminus_Y_0.Text = Y_serial_value.ToString("0.###");
            textBoxZminus_Z_0.Text = Z_serial_value.ToString("0.###");
        }

        private void buttonZminus_180_Click(object sender, EventArgs e)
        {
            textBoxZminus_X_180.Text = X_serial_value.ToString("0.###");
            textBoxZminus_Y_180.Text = Y_serial_value.ToString("0.###");
            textBoxZminus_Z_180.Text = Z_serial_value.ToString("0.###");
        }

        private void CalculateButton_Click(object sender, EventArgs e)
        {
            try
            {
                calculate_transformation_matrix();
            }
            catch
            {
                textBox_matrixX_x.Clear();
                textBox_matrixX_y.Clear();
                textBox_matrixX_z.Clear();
                textBox_matrixY_x.Clear();
                textBox_matrixY_y.Clear();
                textBox_matrixY_z.Clear();
                textBox_matrixZ_x.Clear();
                textBox_matrixZ_y.Clear();
                textBox_matrixZ_z.Clear();
                textBox_biasX.Clear();
                textBox_biasY.Clear();
                textBox_biasZ.Clear();
                MessageBox.Show("Wrong input data!", "Warning!");          
            }
        }

        private void calculate_transformation_matrix()
        {
            //Axis X--------------------------------------------------------------------------------------------------
            double[] Xplus_center = new double[3];
            //Centers of the circles
            Xplus_center[0] = (double.Parse(textBoxXplus_X_0.Text) + double.Parse(textBoxXplus_X_180.Text)) / 2;
            Xplus_center[1] = (double.Parse(textBoxXplus_Y_0.Text) + double.Parse(textBoxXplus_Y_180.Text)) / 2;
            Xplus_center[2] = (double.Parse(textBoxXplus_Z_0.Text) + double.Parse(textBoxXplus_Z_180.Text)) / 2;
            //Centers of the circles
            double[] Xminus_center = new double[3];
            Xminus_center[0] = (double.Parse(textBoxXminus_X_0.Text) + double.Parse(textBoxXminus_X_180.Text)) / 2;
            Xminus_center[1] = (double.Parse(textBoxXminus_Y_0.Text) + double.Parse(textBoxXminus_Y_180.Text)) / 2;
            Xminus_center[2] = (double.Parse(textBoxXminus_Z_0.Text) + double.Parse(textBoxXminus_Z_180.Text)) / 2;
            //Vector from the center of minus circle to the center of plus circle
            double[] Xvector = new double[3];
            Xvector[0] = Xplus_center[0] - Xminus_center[0];
            Xvector[1] = Xplus_center[1] - Xminus_center[1];
            Xvector[2] = Xplus_center[2] - Xminus_center[2];

            //Axis Y--------------------------------------------------------------------------------------------------
            double[] Yplus_center = new double[3];
            //Centers of the circles
            Yplus_center[0] = (double.Parse(textBoxYplus_X_0.Text) + double.Parse(textBoxYplus_X_180.Text)) / 2;
            Yplus_center[1] = (double.Parse(textBoxYplus_Y_0.Text) + double.Parse(textBoxYplus_Y_180.Text)) / 2;
            Yplus_center[2] = (double.Parse(textBoxYplus_Z_0.Text) + double.Parse(textBoxYplus_Z_180.Text)) / 2;
            //Centers of the circles
            double[] Yminus_center = new double[3];
            Yminus_center[0] = (double.Parse(textBoxYminus_X_0.Text) + double.Parse(textBoxYminus_X_180.Text)) / 2;
            Yminus_center[1] = (double.Parse(textBoxYminus_Y_0.Text) + double.Parse(textBoxYminus_Y_180.Text)) / 2;
            Yminus_center[2] = (double.Parse(textBoxYminus_Z_0.Text) + double.Parse(textBoxYminus_Z_180.Text)) / 2;
            //Vector from the center of minus circle to the center of plus circle
            double[] Yvector = new double[3];
            Yvector[0] = Yplus_center[0] - Yminus_center[0];
            Yvector[1] = Yplus_center[1] - Yminus_center[1];
            Yvector[2] = Yplus_center[2] - Yminus_center[2];

            //Axis Z--------------------------------------------------------------------------------------------------
            double[] Zplus_center = new double[3];
            //Centers of the circles
            Zplus_center[0] = (double.Parse(textBoxZplus_X_0.Text) + double.Parse(textBoxZplus_X_180.Text)) / 2;
            Zplus_center[1] = (double.Parse(textBoxZplus_Y_0.Text) + double.Parse(textBoxZplus_Y_180.Text)) / 2;
            Zplus_center[2] = (double.Parse(textBoxZplus_Z_0.Text) + double.Parse(textBoxZplus_Z_180.Text)) / 2;
            //Centers of the circles
            double[] Zminus_center = new double[3];
            Zminus_center[0] = (double.Parse(textBoxZminus_X_0.Text) + double.Parse(textBoxZminus_X_180.Text)) / 2;
            Zminus_center[1] = (double.Parse(textBoxZminus_Y_0.Text) + double.Parse(textBoxZminus_Y_180.Text)) / 2;
            Zminus_center[2] = (double.Parse(textBoxZminus_Z_0.Text) + double.Parse(textBoxZminus_Z_180.Text)) / 2;
            //Vector from the center of minus circle to the center of plus circle
            double[] Zvector = new double[3];
            Zvector[0] = Zplus_center[0] - Zminus_center[0];
            Zvector[1] = Zplus_center[1] - Zminus_center[1];
            Zvector[2] = Zplus_center[2] - Zminus_center[2];

            // Rotation matrix--------------------------------------------------------------------------------------
            // rotation_matrix[a][b], a - number of the rows, b - number of the columbs
            double[][] rotation_matrix = new double[3][];
            rotation_matrix[0] = new double[3];
            rotation_matrix[1] = new double[3];
            rotation_matrix[2] = new double[3];
            //Deviding by main value, for example for X axis - deviding by X coordinate, for Y axis by Y coordinate, for Z axis by Z cordinate
            rotation_matrix[0][0] = Xvector[0] / Xvector[0]; rotation_matrix[0][1] = Yvector[0] / Yvector[1]; rotation_matrix[0][2] = Zvector[0] / Zvector[2];
            rotation_matrix[1][0] = Xvector[1] / Xvector[0]; rotation_matrix[1][1] = Yvector[1] / Yvector[1]; rotation_matrix[1][2] = Zvector[1] / Zvector[2];
            rotation_matrix[2][0] = Xvector[2] / Xvector[0]; rotation_matrix[2][1] = Yvector[2] / Yvector[1]; rotation_matrix[2][2] = Zvector[2] / Zvector[2];
            //Matrix inversion
            rotation_matrix = InvertMatrix(rotation_matrix);

            //Determinating of the corrected by ratation matrix centers of the circles 
            Xplus_center = MatrixVectorMultiply(rotation_matrix, Xplus_center);
            Xminus_center = MatrixVectorMultiply(rotation_matrix, Xminus_center);
            Yplus_center = MatrixVectorMultiply(rotation_matrix, Yplus_center);
            Yminus_center = MatrixVectorMultiply(rotation_matrix, Yminus_center);
            Zplus_center = MatrixVectorMultiply(rotation_matrix, Zplus_center);
            Zminus_center = MatrixVectorMultiply(rotation_matrix, Zminus_center);

            //Determinating of the elipsoid center---------------------------------------------------------------------------
            double[] center = new double[3];
            center[0] = (Xplus_center[0] + Xminus_center[0] + Yplus_center[0] + Yminus_center[0] + Zplus_center[0] + Zminus_center[0]) / 6;
            center[1] = (Xplus_center[1] + Xminus_center[1] + Yplus_center[1] + Yminus_center[1] + Zplus_center[1] + Zminus_center[1]) / 6;
            center[2] = (Xplus_center[2] + Xminus_center[2] + Yplus_center[2] + Yminus_center[2] + Zplus_center[2] + Zminus_center[2]) / 6;

            //Determinating of the radius of the future sphere-----------------------------------------------------------------------
            double x_length = Math.Abs(Xplus_center[0] - Xminus_center[0])/2;
            double y_length = Math.Abs(Yplus_center[1] - Yminus_center[1])/2;
            double z_length = Math.Abs(Zplus_center[2] - Zminus_center[2])/2;
            double[] Xplus_0 = new double[3];
            Xplus_0[0] = double.Parse(textBoxXplus_X_0.Text); 
            Xplus_0[1] = double.Parse(textBoxXplus_Y_0.Text); 
            Xplus_0[2] = double.Parse(textBoxXplus_Z_0.Text);
            Xplus_0 = MatrixVectorMultiply(rotation_matrix, Xplus_0);
            double[] Yplus_0 = new double[3];
            Yplus_0[0] = double.Parse(textBoxYplus_X_0.Text);
            Yplus_0[1] = double.Parse(textBoxYplus_Y_0.Text);
            Yplus_0[2] = double.Parse(textBoxYplus_Z_0.Text);
            Yplus_0 = MatrixVectorMultiply(rotation_matrix, Yplus_0);
            double[] Zplus_0 = new double[3];
            Zplus_0[0] = double.Parse(textBoxZplus_X_0.Text);
            Zplus_0[1] = double.Parse(textBoxZplus_Y_0.Text);
            Zplus_0[2] = double.Parse(textBoxZplus_Z_0.Text);
            Zplus_0 = MatrixVectorMultiply(rotation_matrix, Zplus_0);
            double x_abs = Math.Sqrt(x_length * x_length + Xplus_0[1] * Xplus_0[1] + Xplus_0[2] * Xplus_0[2]);
            double y_abs = Math.Sqrt(Yplus_0[0] * Yplus_0[0] + y_length * y_length + Yplus_0[2] * Yplus_0[2]);
            double z_abs = Math.Sqrt(Zplus_0[0] * Zplus_0[0] + Zplus_0[1] * Zplus_0[1] + z_length * z_length);
            //sphere radius
            double sphere_radius = (x_abs + y_abs + z_abs) / 3;

            //Scales for the each axis------------------------------------------------
            //Diameter of the sphere
            double diameter = sphere_radius * 2;
            double kx = Math.Abs(diameter / (Xplus_center[0] - Xminus_center[0]));
            double ky = Math.Abs(diameter / (Yplus_center[1] - Yminus_center[1]));
            double kz = Math.Abs(diameter / (Zplus_center[2] - Zminus_center[2]));

            //Multiplying elements of matrix by scales
            rotation_matrix[0][0] = rotation_matrix[0][0] * kx; rotation_matrix[0][1] = rotation_matrix[0][1] * ky; rotation_matrix[0][2] = rotation_matrix[0][2] * kz;
            rotation_matrix[1][0] = rotation_matrix[1][0] * kx; rotation_matrix[1][1] = rotation_matrix[1][1] * ky; rotation_matrix[1][2] = rotation_matrix[1][2] * kz;
            rotation_matrix[2][0] = rotation_matrix[2][0] * kx; rotation_matrix[2][1] = rotation_matrix[2][1] * ky; rotation_matrix[2][2] = rotation_matrix[2][2] * kz;
            
            //Bias
            double[] bias = new double[3];
            bias[0] = center[0];
            bias[1] = center[1];
            bias[2] = center[2];

            //Indication
            //Transformation matrix
            textBox_matrixX_x.Text = rotation_matrix[0][0].ToString("0.###"); textBox_matrixY_x.Text = rotation_matrix[0][1].ToString("0.###"); textBox_matrixZ_x.Text = rotation_matrix[0][2].ToString("0.###");
            textBox_matrixX_y.Text = rotation_matrix[1][0].ToString("0.###"); textBox_matrixY_y.Text = rotation_matrix[1][1].ToString("0.###"); textBox_matrixZ_y.Text = rotation_matrix[1][2].ToString("0.###");
            textBox_matrixX_z.Text = rotation_matrix[2][0].ToString("0.###"); textBox_matrixY_z.Text = rotation_matrix[2][1].ToString("0.###"); textBox_matrixZ_z.Text = rotation_matrix[2][2].ToString("0.###");
            //Bias
            textBox_biasX.Text = bias[0].ToString("0.###");
            textBox_biasY.Text = bias[1].ToString("0.###");
            textBox_biasZ.Text = bias[2].ToString("0.###");
        }

        public static double[] MatrixVectorMultiply(double[][] matrixA, double[] vectorB)
        {
            int aRows = matrixA.Length; int aCols = matrixA[0].Length;
            int bRows = vectorB.Length;
            if (aCols != bRows)
                throw new Exception("Non-conformable matrices in MatrixProduct");
            double[] result = new double[aRows];
            for (int i = 0; i < aRows; ++i) // each row of A
                for (int k = 0; k < aCols; ++k)
                    result[i] += matrixA[i][k] * vectorB[k];
            return result;
        }

        public static double[][] InvertMatrix(double[][] A)
        {
            int n = A.Length;
            //e will represent each column in the identity matrix
            double[] e;
            //x will hold the inverse matrix to be returned
            double[][] x = new double[n][];
            for (int i = 0; i < n; i++)
            {
                x[i] = new double[A[i].Length];
            }
            /*
            * solve will contain the vector solution for the LUP decomposition as we solve
            * for each vector of x.  We will combine the solutions into the double[][] array x.
            * */
            double[] solve;

            //Get the LU matrix and P matrix (as an array)
            Tuple<double[][], int[]> results = LUPDecomposition(A);

            double[][] LU = results.Item1;
            int[] P = results.Item2;

            /*
            * Solve AX = e for each column ei of the identity matrix using LUP decomposition
            * */
            for (int i = 0; i < n; i++)
            {
                e = new double[A[i].Length];
                e[i] = 1;
                solve = LUPSolve(LU, P, e);
                for (int j = 0; j < solve.Length; j++)
                {
                    x[j][i] = solve[j];
                }
            }
            return x;
        }

        public static double[] LUPSolve(double[][] LU, int[] pi, double[] b)
        {
            int n = LU.Length - 1;
            double[] x = new double[n + 1];
            double[] y = new double[n + 1];
            double suml = 0;
            double sumu = 0;
            double lij = 0;

            /*
            * Solve for y using formward substitution
            * */
            for (int i = 0; i <= n; i++)
            {
                suml = 0;
                for (int j = 0; j <= i - 1; j++)
                {
                    /*
                    * Since we've taken L and U as a singular matrix as an input
                    * the value for L at index i and j will be 1 when i equals j, not LU[i][j], since
                    * the diagonal values are all 1 for L.
                    * */
                    if (i == j)
                    {
                        lij = 1;
                    }
                    else
                    {
                        lij = LU[i][j];
                    }
                    suml = suml + (lij * y[j]);
                }
                y[i] = b[pi[i]] - suml;
            }
            //Solve for x by using back substitution
            for (int i = n; i >= 0; i--)
            {
                sumu = 0;
                for (int j = i + 1; j <= n; j++)
                {
                    sumu = sumu + (LU[i][j] * x[j]);
                }
                x[i] = (y[i] - sumu) / LU[i][i];
            }
            return x;
        }

        public static Tuple<double[][], int[]> LUPDecomposition(double[][] A)
        {
            int n = A.Length - 1;
            /*
            * pi represents the permutation matrix.  We implement it as an array
            * whose value indicates which column the 1 would appear.  We use it to avoid 
            * dividing by zero or small numbers.
            * */
            int[] pi = new int[n + 1];
            double p = 0;
            int kp = 0;
            int pik = 0;
            int pikp = 0;
            double aki = 0;
            double akpi = 0;

            //Initialize the permutation matrix, will be the identity matrix
            for (int j = 0; j <= n; j++)
            {
                pi[j] = j;
            }

            for (int k = 0; k <= n; k++)
            {
                /*
                * In finding the permutation matrix p that avoids dividing by zero
                * we take a slightly different approach.  For numerical stability
                * We find the element with the largest 
                * absolute value of those in the current first column (column k).  If all elements in
                * the current first column are zero then the matrix is singluar and throw an
                * error.
                * */
                p = 0;
                for (int i = k; i <= n; i++)
                {
                    if (Math.Abs(A[i][k]) > p)
                    {
                        p = Math.Abs(A[i][k]);
                        kp = i;
                    }
                }
                if (p == 0)
                {
                    throw new Exception("singular matrix");
                }
                /*
                * These lines update the pivot array (which represents the pivot matrix)
                * by exchanging pi[k] and pi[kp].
                * */
                pik = pi[k];
                pikp = pi[kp];
                pi[k] = pikp;
                pi[kp] = pik;

                /*
                * Exchange rows k and kpi as determined by the pivot
                * */
                for (int i = 0; i <= n; i++)
                {
                    aki = A[k][i];
                    akpi = A[kp][i];
                    A[k][i] = akpi;
                    A[kp][i] = aki;
                }

                /*
                    * Compute the Schur complement
                    * */
                for (int i = k + 1; i <= n; i++)
                {
                    A[i][k] = A[i][k] / A[k][k];
                    for (int j = k + 1; j <= n; j++)
                    {
                        A[i][j] = A[i][j] - (A[i][k] * A[k][j]);
                    }
                }
            }
            return Tuple.Create(A, pi);
        }

        private void help_image_form(string help_image_massge, string help_image_link)
        {
            axis_image_view frm_op = new axis_image_view();
            frm_op.Owner = this;
            frm_op.text = help_image_massge;
            frm_op.image_link = help_image_link;
            frm_op.ShowDialog();
        }

        private void button9_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "X_plus_0.png");
        }

        private void button10_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "X_plus_180.png");
        }

        private void button11_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Y_plus_0.png");
        }

        private void button12_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Y_plus_180.png");
        }

        private void button13_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Z_plus_0.png");
        }

        private void button14_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Z_plus_180.png");
        }

        private void button3_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "X_minus_0.png");
        }

        private void button4_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "X_minus_180.png");
        }

        private void button5_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Y_minus_0.png");
        }

        private void button6_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Y_minus_180.png");
        }

        private void button7_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 0°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Z_minus_0.png");
        }

        private void button8_Click(object sender, EventArgs e)
        {
            help_image_form("and click \"Point 180°\" button or enter data manually.", curDir + "\\MagMaster Files\\images\\" + "Z_minus_180.png");
        }

        private void help_button_Click(object sender, EventArgs e)
        {
            serial_port_help_from frm_op = new serial_port_help_from();
            frm_op.Owner = this;
            frm_op.help_text = System.IO.File.ReadAllText(curDir + "\\MagMaster Files\\texts\\" + "sphelp.txt"); 
            frm_op.ShowDialog();
        }

        private void floatTextToint16_buf(string float_text, Byte[] buf, int index)
        {
            short int16_val = (short)Convert.ToSingle(float_text);
            buf[index + 0] = (byte)int16_val;
            buf[index + 1] = (byte)(int16_val >> 8);
        }

        private float unsignedHudredthsToFloat(Byte[] buf, int index)
        {
            UInt16 integer = BitConverter.ToUInt16(buf, index);
            float val = integer;
            val /= 100.0f;
            return val;
        }

        private float signedHundredthsToFloat(Byte[] buf, int index)
        {
            Int16 integer = BitConverter.ToInt16(buf, index);
            float val = integer;
            val /= 100.0f;
            return val;
        }

        private float signedThousandthsToFloat(Byte[] buf, int index)
        {
            Int16 integer = BitConverter.ToInt16(buf, index);
            float val = integer;
            val /= 1000.0f;
            return val;
        }

        private float text1616FloatToFloat(Byte[] buf, int index)
        {
            Int32 integer = BitConverter.ToInt32(buf, index);
            float val = (float)integer;
            val /= 65536.0f;
            return val;
        }

        private void floatTextTo1616Float(string float_text, Byte[] buf, int index)
        {
            float x_d = Convert.ToSingle(float_text);
            x_d *= 65536.0f;
            int decimal_as_int = (int)x_d;
            if (BitConverter.IsLittleEndian)
            {
                buf[index + 3] = (byte)(decimal_as_int >> 24);
                buf[index + 2] = (byte)(decimal_as_int >> 16);
                buf[index + 1] = (byte)(decimal_as_int >> 8);
                buf[index + 0] = (byte)(decimal_as_int >> 0);
            }
            else
            {
                buf[index + 0] = (byte)(decimal_as_int >> 24);
                buf[index + 1] = (byte)(decimal_as_int >> 16);
                buf[index + 2] = (byte)(decimal_as_int >> 8);
                buf[index + 3] = (byte)(decimal_as_int >> 0);
            }
        }

        public void CharToHex(byte b, byte[] buf, int index)
        {
            String hex = b.ToString("X2");
            byte[] b2 = System.Text.Encoding.ASCII.GetBytes(hex);
            buf[index] = b2[0];
            buf[index + 1] = b2[1];
        }

        private void send_board_identity_request()
        {
            send_board_data_request(2);
        }

        private void send_magcal_data_request()
        {
            send_board_data_request(1);
        }

        private void send_board_data_request(int datatype)
        {
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[10];

                // Header
                buf[0] = Convert.ToByte('!');
                buf[1] = Convert.ToByte('#');
                buf[2] = (byte)(buf.Length - 2);
                buf[3] = Convert.ToByte('D');
                // Data
                buf[4] = (byte)datatype;
                buf[5] = 0; /* Subtype = 0 (not used) */
                // Footer
                // Checksum is at 4;
                byte checksum = (byte)0;
                for (int i = 0; i < 6; i++)
                {
                    checksum += (byte)buf[i];
                }
                CharToHex(checksum, buf, 6);

                // Terminator begins at 8;
                buf[8] = Convert.ToByte('\r');
                buf[9] = Convert.ToByte('\n');

                try
                {
                    port.Write(buf, 0, buf.Length);
                }
                catch (Exception)
                {
                }
            }
        }

        private void send_ahrs_stream_request()
        {
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[10];

                // Header
                buf[0] = Convert.ToByte('!');
                buf[1] = Convert.ToByte('S');
                // Data
                buf[2] = Convert.ToByte('a');
                byte update_rate_hz = 50;
                CharToHex(update_rate_hz, buf, 3);
                // Footer
                // Checksum is at 5;
                byte checksum = (byte)0;
                for (int i = 0; i < 5; i++)
                {
                    checksum += (byte)buf[i];
                }
                CharToHex(checksum, buf, 5);

                // Terminator begins at 7;
                buf[7] = Convert.ToByte('\r');
                buf[8] = Convert.ToByte('\n');
                try
                {
                    port.Write(buf, 0, buf.Length);
                }
                catch (Exception)
                {
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            DialogResult d = MessageBox.Show("Are you sure you want to apply the current calibration?  This will over-write any calibration data already on the device.", "Apply Calibration", MessageBoxButtons.OKCancel);
            if (d != System.Windows.Forms.DialogResult.OK) return;

            /* TODO:  Button should be disabled unless both port is open and */
            /* a valid set of bias/matrix data is available */
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[55];

                // Header
                buf[0] = Convert.ToByte('!');
                buf[1] = Convert.ToByte('#');
                buf[2] = (byte)(buf.Length - 2);
                buf[3] = Convert.ToByte('M');

                // Data
                buf[4] = 1; /* 1 = Set */
                floatTextToint16_buf(textBox_biasX.Text,buf,5);
                floatTextToint16_buf(textBox_biasY.Text, buf, 7);
                floatTextToint16_buf(textBox_biasZ.Text, buf, 9);
                floatTextTo1616Float(textBox_matrixX_x.Text, buf, 11);
                floatTextTo1616Float(textBox_matrixX_y.Text, buf, 15);
                floatTextTo1616Float(textBox_matrixX_z.Text, buf, 19);
                floatTextTo1616Float(textBox_matrixY_x.Text, buf, 23);
                floatTextTo1616Float(textBox_matrixY_y.Text, buf, 27);
                floatTextTo1616Float(textBox_matrixY_z.Text, buf, 31);
                floatTextTo1616Float(textBox_matrixZ_x.Text, buf, 35);
                floatTextTo1616Float(textBox_matrixZ_y.Text, buf, 39);
                floatTextTo1616Float(textBox_matrixZ_z.Text, buf, 43);

                float x, y, z;
                float bias_x, bias_y, bias_z;
                float mag_field_norm_total = 0;
                bias_x = Convert.ToSingle(textBox_biasX.Text);
                bias_y = Convert.ToSingle(textBox_biasY.Text);
                bias_z = Convert.ToSingle(textBox_biasZ.Text);
                x = Convert.ToSingle(textBoxXplus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxXplus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxXplus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                x = Convert.ToSingle(textBoxXminus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxXminus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxXminus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                x = Convert.ToSingle(textBoxYplus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxYplus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxYplus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                x = Convert.ToSingle(textBoxYminus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxYminus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxYminus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                x = Convert.ToSingle(textBoxZplus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxZplus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxZplus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                x = Convert.ToSingle(textBoxZminus_X_0.Text) - bias_x;
                y = Convert.ToSingle(textBoxZminus_Y_0.Text) - bias_y;
                z = Convert.ToSingle(textBoxZminus_Z_0.Text) - bias_z;
                mag_field_norm_total += (float)Math.Sqrt((x * x) + (y * y) + (z * z));
                float mag_field_norm_avg = mag_field_norm_total / 6.0f;
                string earth_mag_field_norm = Convert.ToString(mag_field_norm_avg);

                floatTextTo1616Float(earth_mag_field_norm, buf, 47);

                // Footer
                // Checksum is at 51;
                byte checksum = (byte)0;
                for (int i = 0; i < 51; i++)
                {
                    checksum += (byte)buf[i];
                }
                CharToHex(checksum, buf, 51);

                // Terminator begins at 53;
                buf[53] = Convert.ToByte('\r');
                buf[54] = Convert.ToByte('\n');

                port.Write(buf, 0, buf.Length);
            }
        }

        private void textBoxXplus_X_0_KeyPress(object sender, KeyPressEventArgs e)
        {
            TextBox tx = (TextBox)sender;
            if ((symbol_mask.IndexOf(e.KeyChar) != -1) || (e.KeyChar == 8))
            {
                if ((e.KeyChar == '.') && (tx.Text.IndexOf(".") != -1))
                    e.Handled = true;
                if ((e.KeyChar == '-') && (tx.Text.IndexOf("-") != -1))
                    e.Handled = true;
            }
            else
                e.Handled = true;       
        }

        private void textBoxXplus_X_0_TextChanged(object sender, EventArgs e)
        {
            TextBox tx = (TextBox)sender;
            string strbox = tx.Text;

            string symbol_st = ".";
            if (strbox.Length >= symbol_st.Length)
            {
                string substrbox = strbox.Substring(0, symbol_st.Length);
                if (substrbox == symbol_st) strbox = "" + strbox.Substring(symbol_st.Length, strbox.Length - symbol_st.Length);
            }

            symbol_st = "-.";
            if (strbox.Length >= symbol_st.Length)
            {
                string substrbox = strbox.Substring(0, symbol_st.Length);
                if (substrbox == symbol_st) strbox = "-" + strbox.Substring(symbol_st.Length, strbox.Length - symbol_st.Length);
            }

            symbol_st = "-";
            if (strbox.Length >= symbol_st.Length)
            {
                string substrbox = strbox.Substring(0, symbol_st.Length);
                if (substrbox != symbol_st) strbox = strbox.Replace("-", "");
            }

            tx.Text = strbox;

        }

        private void button2_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Process.Start("http://www.pdocs.kauailabs.com/navx-mxp/software/tools/magnetometer-calibration/");
        }

        private void button15_Click(object sender, EventArgs e)
        {
            send_magcal_data_request();
        }

        private void label34_Click(object sender, EventArgs e)
        {

        }

    }
}
