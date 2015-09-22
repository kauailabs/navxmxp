using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
using System.Threading;
using System.IO;
using System.IO.Ports;
using System.Management;
using navXComUtilities;

namespace navXConfig
{
    public partial class Form1 : Form
    {
        static Object bufferLock = new Object();
        static Byte[] bytes_from_usart = null;
        static int num_bytes_from_usart = 0;
        static int bytes_from_usart_offset = 0;
        static Boolean port_close_flag;
        int empty_serial_data_counter;
        string curDir;
 
        public Form1()
        {
            InitializeComponent();
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
            enable_controls(false);
            navXComHelper.InitPortComboBox(comboBox1);
        }

        private void comboBox1_DropDown(object sender, EventArgs e)
        {
            navXComHelper.HandleComboBoxDropDownEvent(comboBox1);
        }

        const int tuning_var_id_unspecified = 0;
        const int tuning_var_id_motion_threshold = 1;
        const int tuning_var_id_yaw_stable_threshold = 2;
        const int tuning_var_id_mag_distrubance_threshold = 3;
        const int tuning_var_id_sea_level_pressure = 4;

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
                start_button.Enabled = false;
                comboBox1.Enabled = false;
                enable_controls(true);
                port.DiscardInBuffer();
                send_board_identity_request();
                System.Threading.Thread.Sleep(25);
                refresh_settings();
                //for the handler
                port.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                //timer activation
                timer1.Enabled = true;
            }
            catch
            {
                close_port();
                enable_controls(false);
                MessageBox.Show("Empty port name.", "Warning!");
            }

        }

        private void enable_controls(bool enable)
        {
            maskedTextBox1.Enabled = enable;
            maskedTextBox2.Enabled = enable;
            maskedTextBox3.Enabled = enable;
            maskedTextBox4.Enabled = enable;
            set_button1.Enabled = enable;
            button1.Enabled = enable;
            button2.Enabled = enable;
            button3.Enabled = enable;
            button_restore_default1.Enabled = enable;
            button_restore_default2.Enabled = enable;
            button_restore_default3.Enabled = enable;
            button_restore_default4.Enabled = enable;
            restore_all_factory_defaults_button.Enabled = enable;
        }

        private void refresh_settings()
        {
            request_tuning_variable(tuning_var_id_motion_threshold);
            Thread.Sleep(10);
            request_tuning_variable(tuning_var_id_yaw_stable_threshold);
            Thread.Sleep(10);
            request_tuning_variable(tuning_var_id_mag_distrubance_threshold);
            Thread.Sleep(10);
            request_tuning_variable(tuning_var_id_sea_level_pressure);
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
            enable_controls(false);
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

        private void send_board_data_request(int datatype)
        {
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[10]; /* HACK:  Must be 9 bytes to register to the navX MXP */

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
                        if (bytes_from_usart != null)
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

        /* Tuning Variable Get Request:  'D' (Data Request, type = tuning var, id = tuning var id) */
        /* Tuning Variable Get Response: 'T' (Tuning Variable Get/Set, action = GET, id = tuning var id, value = value  */
        /* Tuning Variable Set Request:  'T' (Tuning Variable Get/Set, action = SET, id = tuning var id, value = value */
        /* Tuning Variable Set Response: 'v' (Variable set response, type = tuning var, id = tuing var id, status = 0 if successful */

        const char navx_msg_start_char = '!';
        const char navx_binary_msg_indicator = '#';
        const int navx_tuning_get_request_msg_len = 10;
        const char navx_tuning_get_request_msg_id = 'D';    /* [type],[varid] */
        const int navx_tuning_getset_msg_len = 14;
        const char navx_tuning_getset_msg_id = 'T';        /* [type],[varid],[value (16:16)]*/
        const int navx_tuning_set_response_msg_len = 11;
        const char navx_tuning_set_response_msg_id = 'v';   /* [type],[varid],[status] */
        const char navx_board_id_msg_type = 'i';
        const int navx_board_id_msg_length = 26;

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
                while (!end_of_data)
                {
                    if ((usart_bytes != null) && (valid_bytes_available >= 2))
                    {
                        if ((usart_bytes[usart_data_offset] == Convert.ToByte(navx_msg_start_char)) &&
                             (usart_bytes[usart_data_offset + 1] == Convert.ToByte(navx_binary_msg_indicator)))
                        {
                            /* Valid packet start found */
                            if ((usart_bytes[usart_data_offset + 2] == navx_tuning_getset_msg_len - 2) &&
                                 (usart_bytes[usart_data_offset + 3] == Convert.ToByte(navx_tuning_getset_msg_id)))
                            {
                                /* AHRS Update packet received */
                                byte[] bytes = new byte[navx_tuning_getset_msg_len];
                                System.Buffer.BlockCopy(usart_bytes, usart_data_offset, bytes, 0, navx_tuning_getset_msg_len);
                                valid_bytes_available -= navx_tuning_getset_msg_len;
                                usart_data_offset += navx_tuning_getset_msg_len;

                                byte type = bytes[4];
                                byte varid = bytes[5];
                                float value = text1616FloatToFloat(bytes, 6);

                                if (varid == tuning_var_id_motion_threshold)
                                {
                                    maskedTextBox1.Text = String.Format("{0:##0.###}", value);
                                }
                                if (varid == tuning_var_id_yaw_stable_threshold)
                                {
                                    maskedTextBox2.Text = String.Format("{0:##0.###}", value);
                                }
                                if (varid == tuning_var_id_mag_distrubance_threshold)
                                {
                                    value *= 100; /* Convert from ratio to percentage */
                                    maskedTextBox3.Text = String.Format("{0:##0.#}", value);
                                }
                                if (varid == tuning_var_id_sea_level_pressure)
                                {
                                    maskedTextBox4.Text = String.Format("{0:##0.###}", value);
                                }
                            }
                            else if ((usart_bytes[usart_data_offset + 2] == navx_tuning_set_response_msg_len - 2) &&
                                     (usart_bytes[usart_data_offset + 3] == Convert.ToByte(navx_tuning_set_response_msg_id)))
                            {
                                byte[] bytes = new byte[navx_tuning_set_response_msg_len];
                                System.Buffer.BlockCopy(usart_bytes, usart_data_offset, bytes, 0, navx_tuning_set_response_msg_len);
                                valid_bytes_available -= navx_tuning_set_response_msg_len;
                                usart_data_offset += navx_tuning_set_response_msg_len;

                                byte type = bytes[4];
                                byte varid = bytes[5];
                                byte status = bytes[6];
                                string msg = "Data Set Response:  variable:  " + varid +
                                                ", status = " + status;
                                MessageBox.Show(msg, "Data Set");
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
            catch (Exception)
            {
                close_port();
                MessageBox.Show("Serial port error.", "Warning!");
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            var_refresh();
        }

        private void request_tuning_variable(int var_id)
        {
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[navx_tuning_get_request_msg_len]; /* HACK:  Must be 9 bytes to register to the navX MXP */

                // Header
                buf[0] = Convert.ToByte('!');
                buf[1] = Convert.ToByte('#');
                buf[2] = (byte)(buf.Length - 2);
                buf[3] = Convert.ToByte(navx_tuning_get_request_msg_id);
                // Data
                buf[4] = (byte)0; /* Tuning Variable data type */
                buf[5] = (byte)var_id;
                // Footer
                // Checksum is at 6;
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

        private void set_tuning_variable(int var_id, float val, bool restore_to_defaults)
        {
            if (port.IsOpen)
            {
                Byte[] buf = new Byte[navx_tuning_getset_msg_len]; /* HACK:  Must be 9 bytes to register to the navX MXP */

                // Header
                buf[0] = Convert.ToByte('!');
                buf[1] = Convert.ToByte('#');
                buf[2] = (byte)(buf.Length - 2);
                buf[3] = Convert.ToByte(navx_tuning_getset_msg_id);
                // Data
                buf[4] = (byte)(restore_to_defaults ? 2 : 1); /* DATA_SET_TO_DEFAULT : DATA_SET */
                buf[5] = (byte)var_id;
                floatTextTo1616Float(val.ToString(), buf, 6);
                // Footer
                // Checksum is at 10;
                byte checksum = (byte)0;
                for (int i = 0; i < 10; i++)
                {
                    checksum += (byte)buf[i];
                }
                CharToHex(checksum, buf, 10);

                // Terminator begins at 12;
                buf[12] = Convert.ToByte('\r');
                buf[13] = Convert.ToByte('\n');

                try
                {
                    port.Write(buf, 0, buf.Length);
                }
                catch (Exception)
                {
                }
            }
        }

        private void button_exit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void set_button1_Click(object sender, EventArgs e)
        {
            try
            {
                String val_string = maskedTextBox1.Text;
                float val = Convert.ToSingle(val_string);
                set_tuning_variable(tuning_var_id_motion_threshold, val, false);
                Thread.Sleep(25);
                refresh_settings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                String val_string = maskedTextBox2.Text;
                float val = Convert.ToSingle(val_string);
                set_tuning_variable(tuning_var_id_yaw_stable_threshold, val, false);
                Thread.Sleep(25);
                refresh_settings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                String val_string = maskedTextBox3.Text;
                float val = Convert.ToSingle(val_string);
                val /= 100; /* Convert from percentage to ratio */
                set_tuning_variable(tuning_var_id_mag_distrubance_threshold, val, false);
                Thread.Sleep(25);
                refresh_settings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                String val_string = maskedTextBox4.Text;
                float val = Convert.ToSingle(val_string);
                set_tuning_variable(tuning_var_id_sea_level_pressure, val, false);
                Thread.Sleep(25);
                refresh_settings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void button_restore_default1_Click(object sender, EventArgs e)
        {
            set_tuning_variable(tuning_var_id_motion_threshold, 0.0f, true);
            Thread.Sleep(25);
            refresh_settings();
        }

        private void button_restore_default2_Click(object sender, EventArgs e)
        {
            set_tuning_variable(tuning_var_id_yaw_stable_threshold, 0.0f, true);
            Thread.Sleep(25);
            refresh_settings();
        }

        private void button_restore_default3_Click(object sender, EventArgs e)
        {
            set_tuning_variable(tuning_var_id_mag_distrubance_threshold, 0.0f, true);
            Thread.Sleep(25);
            refresh_settings();
        }

        private void button_restore_default4_Click(object sender, EventArgs e)
        {
            set_tuning_variable(tuning_var_id_sea_level_pressure, 0.0f, true);
            Thread.Sleep(25);
            refresh_settings();
        }

        private void restore_all_factory_defaults_button_Click(object sender, EventArgs e)
        {
            set_tuning_variable(tuning_var_id_unspecified, 0.0f, true);
            Thread.Sleep(25);
            refresh_settings();
        }
    }
}
