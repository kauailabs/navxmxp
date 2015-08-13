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
using NetDFULib;
using navXComUtilities;

namespace navXFirmwareUpdater
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
        string full_path_to_hex_file;

        static string statusText;

        static FirmwareUpdate firmwareUpdate = new FirmwareUpdate();
        static HEX2DFU hex2dfu = new HEX2DFU();
        static TextBox statusTextbox;
        static ProgressBar progressBar;
        static Form1 form1;

        const UInt16 theVid = 0x0483;
        const UInt16 thePid = 0x5740;
        const UInt16 theBcd = 0x0200;

        bool port_open_in_progress = false;
        bool dialog_in_progress = false;
        bool firmware_update_registered = false;

        public Form1()
        {
            InitializeComponent();
            button1.Enabled = false;
            statusTextbox = this.firmwareUpdateStatus;
            progressBar = progressBar1;
            form1 = this;
            progressBar1.Visible = false;
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
            label1.Enabled = false;
            label2.Enabled = false;
            label3.Enabled = false;
            label8.Enabled = false;
            label_ReadyToUpdate.Visible = false;
            label_NotReadyToUpdate.Visible = true;
            label_VCP_Mode_Not_Ready.Visible = true;
            label_VCP_Open_Ready.Visible = false;
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
                try
                {
                    this.UseWaitCursor = true;
                    Application.DoEvents();
                    port_open_in_progress = true;
                    port.Open();
                    port.DiscardInBuffer();
                    port_open_in_progress = false;
                    this.UseWaitCursor = false;
                }
                catch (Exception ex)
                {
                    port_open_in_progress = false;
                    this.UseWaitCursor = false;
                    Application.DoEvents();
                    dialog_in_progress = true;
                    MessageBox.Show("Error opening port.  " + ex.Message,
                                     "navXFirmwareUpdater",
                                     MessageBoxButtons.OK,
                                     MessageBoxIcon.Error);
                    dialog_in_progress = false;
                    return;
                }

                stop_button.Enabled = true;
                start_button.Enabled = false;
                comboBox1.Enabled = false;
                enable_controls(true);
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
                dialog_in_progress = true;
                MessageBox.Show("Empty port name.", "Warning!");
                dialog_in_progress = false;
            }

        }

        private void enable_controls(bool enable)
        {
        }

        private void refresh_settings()
        {
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
            firmwareVersion.Text = "";
            boardVersion.Text = "";
            boardID.Text = "";
            boardType.Text = "";
            label1.Enabled = false;
            label2.Enabled = false;
            label3.Enabled = false;
            label8.Enabled = false;
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
            Int16 integer = BitConverter.ToInt16(buf, index);
            float val = (float)BitConverter.ToUInt16(buf, index + 2);
            val /= 65535.0f;
            val += integer;
            return val;
        }

        private void floatTextTo1616Float(string float_text, Byte[] buf, int index)
        {
            float x = Convert.ToSingle(float_text);
            short x_i = (short)x;
            float x_d = (x - (float)x_i);
            x_d *= 65535.0f;
            if (x_d < 0.0f)
            {
                x_d *= -1;
            }
            short decimal_as_int = (short)x_d;
            buf[index + 0] = (byte)x_i;
            buf[index + 1] = (byte)(x_i >> 8);
            buf[index + 2] = (byte)decimal_as_int;
            buf[index + 3] = (byte)(decimal_as_int >> 8);
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
                            if ((usart_bytes[usart_data_offset + 2] == navx_board_id_msg_length - 2) &&
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

                                if (boardtype == 50)
                                {
                                    boardType.Text = "navX MXP";
                                }
                                else
                                {
                                    boardType.Text = boardtype.ToString();
                                }
                                boardVersion.Text = hwrev.ToString();
                                firmwareVersion.Text = fw_major.ToString() + "." + fw_minor.ToString() + "." + fw_revision.ToString();
                                boardID.Text = BitConverter.ToString(unique_id);
                                label1.Enabled = true;
                                label2.Enabled = true;
                                label3.Enabled = true;
                                label8.Enabled = true;
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
                }
                //empty_serial_data_counter++;
                if (empty_serial_data_counter >= 10)
                {
                    close_port();
                    dialog_in_progress = true;
                    MessageBox.Show("No serial data.", "Warning!");
                    dialog_in_progress = false;
                }
            }
            catch (Exception ex)
            {
                close_port();
                dialog_in_progress = true;
                MessageBox.Show("Serial port error.  " + ex.Message, "Warning!");
                dialog_in_progress = false;
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if ((!port_open_in_progress) && (!dialog_in_progress))
            {
                var_refresh();
            }
        }

        private void button_exit_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void selectHexFile_Click(object sender, EventArgs e)
        {
            // Create an instance of the open file dialog box.
            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            // Set filter options and filter index.
            String default_firmware_dir = System.Environment.GetFolderPath(System.Environment.SpecialFolder.Personal) + "\\navx-mxp\\firmware\\";
            openFileDialog1.InitialDirectory = default_firmware_dir;
            openFileDialog1.RestoreDirectory = false;
            openFileDialog1.Filter = "Hex Files (.hex)|*.hex";
            openFileDialog1.FilterIndex = 1;

            openFileDialog1.Multiselect = false;

            // Call the ShowDialog method to show the dialog box.
            dialog_in_progress = true;
            bool userClickedOK = (openFileDialog1.ShowDialog() == DialogResult.OK);
            dialog_in_progress = false;

            // Process input if the user clicked OK.
            if (userClickedOK == true)
            {
                navXHexFilePath.Text = Path.GetFileName(openFileDialog1.FileName);
                full_path_to_hex_file = openFileDialog1.FileName;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!firmware_update_registered)
            {
                firmwareUpdate.OnFirmwareUpdateProgress += new FirmwareUpdateProgressEventHandler(firmwareUpdate_OnFirmwareUpdateProgress);
                firmware_update_registered = true;
            }
            FirmwareUpdateProgressEventArgs fupea;
            progressBar1.Maximum = 100;
            progressBar1.Step = 1;
            progressBar1.Value = 0;

            statusText = "";
            statusTextbox.Clear();
            progressBar1.Visible = true;
            Application.DoEvents();
            fupea = new FirmwareUpdateProgressEventArgs(0, "Detecting DFU Interface...", false);
            firmwareUpdate_OnFirmwareUpdateProgress(this, fupea);

            if (port.IsOpen)
            {
                close_port();
            }

            bool is_dfu_available = false;
            Cursor.Current = Cursors.WaitCursor;
            Application.DoEvents();
            try
            {
                is_dfu_available = firmwareUpdate.IsDFUDeviceFound();
            }
            catch (Exception ex)
            {
                fupea = new FirmwareUpdateProgressEventArgs(0, ex.Message, true);
                firmwareUpdate_OnFirmwareUpdateProgress(this, fupea);
            }
            if (!is_dfu_available)
            {
                Cursor.Current = Cursors.Default;
                progressBar1.Visible = false;
                Application.DoEvents();
                dialog_in_progress = true;
                MessageBox.Show("To update navX Firmware, the navX board must be in Firmware Update Mode.\n\n" +
                                "1. Disconnect the board from the PC USB port.\n" +
                                "2. Ensure the board has completely powered down (the RED 3.3V LED must be off).\n" +
                                "3. While holding down the 'CAL' button, re-connect the board to the PC USB port.\n\n" +
                                "4. After the board has powered up, release the 'CAL' button.\n" + 
                                "When in DFU mode, only the RED power LED will be ON.",
                                "navX Firmware Update",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Warning);
                dialog_in_progress = false;
                return;
            }
            String dfu_file_name = Path.GetTempPath() + "navx.dfu";
            bool converted = hex2dfu.ConvertHexToDFU(full_path_to_hex_file,
                                    dfu_file_name,
                                    theVid,
                                    thePid,
                                    theBcd);
            if (!converted)
            {
                Cursor.Current = Cursors.Default;
                progressBar1.Visible = false;
                Application.DoEvents();
                dialog_in_progress = true;
                MessageBox.Show("Error converting " + navXHexFilePath.Text + " to DFU format.",
                                "navX Firmware Update",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                dialog_in_progress = false;
                return;
            }

            UInt16 VID;
            UInt16 PID;
            UInt16 Version;
 
            try
            {
                firmwareUpdate.ParseDFU_File(dfu_file_name, out VID, out PID, out Version);
            }
            catch (Exception ex)
            {
                Cursor.Current = Cursors.Default;
                progressBar1.Visible = false;
                Application.DoEvents();
                fupea = new FirmwareUpdateProgressEventArgs(0, "Error parsing DFU file. " + ex.Message, false);
                firmwareUpdate_OnFirmwareUpdateProgress(this, fupea);
                dialog_in_progress = true;
                MessageBox.Show("Error parsing DFU file. " + ex.Message,
                                "navX Firmware Update",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                dialog_in_progress = false;
                return;
            }

            fupea = new FirmwareUpdateProgressEventArgs(0, ("Found VID: " + VID.ToString("X4") + " PID: " + PID.ToString("X4") + " Version: " + Version.ToString("X4")), false);
            firmwareUpdate_OnFirmwareUpdateProgress(this, fupea);

            try
            {
                bool eraseEveything = false;
                bool exitDFUMode = true;

                firmwareUpdate.UpdateFirmware(dfu_file_name, eraseEveything, exitDFUMode);
            }
            catch (Exception ex)
            {
                Cursor.Current = Cursors.Default;
                progressBar1.Visible = false;
                Application.DoEvents();
                fupea = new FirmwareUpdateProgressEventArgs(0, "Error deploying DFU file. " + ex.Message, true);
                firmwareUpdate_OnFirmwareUpdateProgress(this,fupea);
                dialog_in_progress = true;
                MessageBox.Show("Error deploying DFU file. " + ex.Message,
                                "navX Firmware Update",
                                MessageBoxButtons.OK,
                                MessageBoxIcon.Error);
                dialog_in_progress = false;
                return;
            }
            progressBar1.Visible = false;
            Cursor.Current = Cursors.Default;
        }
        static void firmwareUpdate_OnFirmwareUpdateProgress(object sender, FirmwareUpdateProgressEventArgs e)
        {
            statusText += e.Message + " " + System.Environment.NewLine;
            statusTextbox.Invoke(new Action(() => statusTextbox.Text = statusText));
            statusTextbox.Invoke(new Action(() => statusTextbox.SelectionStart = statusTextbox.Text.Length));
            statusTextbox.Invoke(new Action(() => statusTextbox.ScrollToCaret()));
            progressBar.Invoke(new Action(() => progressBar.Value = (int)e.Percentage));
            if (e.Percentage == 100)
            {
                form1.Invoke(new Action(() => MessageBox.Show("Firmware Update Complete.\n\nYou can verify the version on the 'Currently-loaded Firmware Version' tab.","navX Firmware Updater",
                    MessageBoxButtons.OK, MessageBoxIcon.Information)));
            }
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void tabPage1_Click(object sender, EventArgs e)
        {

        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            if ( (!port_open_in_progress) && (!dialog_in_progress) )
            {
                if (tabControl1.SelectedTab == tabPage1)
                {
                    if (full_path_to_hex_file != null)
                    {
                        bool dfu_device_present = navXComHelper.IsDFUDevicePresent();
                        bool firmware_file_ready = (full_path_to_hex_file != null) &&
                                                    (full_path_to_hex_file.Length > 1);
                        button1.Enabled = (dfu_device_present && firmware_file_ready);
                        label_NotReadyToUpdate.Visible = !button1.Enabled;
                        label_ReadyToUpdate.Visible = button1.Enabled;
                    }
                }
                if (tabControl1.SelectedTab == tabPage2)
                {
                    bool vcp_device_present = (navXComHelper.GetnavXSerialPortNames().Length > 0);
                    label_VCP_Mode_Not_Ready.Visible = !vcp_device_present;
                    label_VCP_Open_Ready.Visible = (vcp_device_present && !port.IsOpen);
                    if (!vcp_device_present && (comboBox1.Items.Count > 0))
                    {
                        if (port.IsOpen)
                        {
                            close_port();
                        }
                        comboBox1.Items.Clear();
                    }
                    else
                    {
                        if (comboBox1.Items.Count == 0)
                        {
                            navXComHelper.InitPortComboBox(comboBox1);
                            start_button.Enabled = true;
                        }
                    }
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Form frmTroubleshootingTips = new TroubleshootingTips();
            frmTroubleshootingTips.ShowDialog();
        }
    }

}
