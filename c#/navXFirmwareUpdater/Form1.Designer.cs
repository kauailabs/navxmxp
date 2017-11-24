namespace VMXFirmwareUpdater
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.button_exit = new System.Windows.Forms.Button();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.start_button = new System.Windows.Forms.Button();
            this.stop_button = new System.Windows.Forms.Button();
            this.port = new System.IO.Ports.SerialPort(this.components);
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.boardType = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.boardID = new System.Windows.Forms.Label();
            this.boardVersion = new System.Windows.Forms.Label();
            this.firmwareVersion = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.selectHexFile = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.label_NotReadyToUpdate = new System.Windows.Forms.Label();
            this.hexFilePath = new System.Windows.Forms.Label();
            this.firmwareUpdateStatus = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.button2 = new System.Windows.Forms.Button();
            this.label_ReadyToUpdate = new System.Windows.Forms.Label();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.label_VCP_Mode_Not_Ready = new System.Windows.Forms.Label();
            this.label_VCP_Open_Ready = new System.Windows.Forms.Label();
            this.timer2 = new System.Windows.Forms.Timer(this.components);
            this.groupBox2.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.SuspendLayout();
            // 
            // button_exit
            // 
            this.button_exit.Location = new System.Drawing.Point(633, 469);
            this.button_exit.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button_exit.Name = "button_exit";
            this.button_exit.Size = new System.Drawing.Size(89, 26);
            this.button_exit.TabIndex = 0;
            this.button_exit.Text = "E&xit";
            this.button_exit.UseVisualStyleBackColor = true;
            this.button_exit.Click += new System.EventHandler(this.button_exit_Click);
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(27, 26);
            this.comboBox1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(148, 24);
            this.comboBox1.TabIndex = 1;
            this.comboBox1.DropDown += new System.EventHandler(this.comboBox1_DropDown);
            this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
            // 
            // start_button
            // 
            this.start_button.Location = new System.Drawing.Point(215, 26);
            this.start_button.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.start_button.Name = "start_button";
            this.start_button.Size = new System.Drawing.Size(87, 28);
            this.start_button.TabIndex = 2;
            this.start_button.Text = "Open";
            this.start_button.UseVisualStyleBackColor = true;
            this.start_button.Click += new System.EventHandler(this.start_button_Click);
            // 
            // stop_button
            // 
            this.stop_button.Location = new System.Drawing.Point(309, 26);
            this.stop_button.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.stop_button.Name = "stop_button";
            this.stop_button.Size = new System.Drawing.Size(87, 27);
            this.stop_button.TabIndex = 3;
            this.stop_button.Text = "Close";
            this.stop_button.UseVisualStyleBackColor = true;
            this.stop_button.Click += new System.EventHandler(this.stop_button_Click);
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Interval = 500;
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.boardType);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.boardID);
            this.groupBox2.Controls.Add(this.boardVersion);
            this.groupBox2.Controls.Add(this.firmwareVersion);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Location = new System.Drawing.Point(12, 62);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox2.Size = new System.Drawing.Size(648, 151);
            this.groupBox2.TabIndex = 21;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Current Version";
            this.groupBox2.Enter += new System.EventHandler(this.groupBox2_Enter);
            // 
            // boardType
            // 
            this.boardType.AutoSize = true;
            this.boardType.Location = new System.Drawing.Point(143, 116);
            this.boardType.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.boardType.Name = "boardType";
            this.boardType.Size = new System.Drawing.Size(0, 17);
            this.boardType.TabIndex = 1;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(15, 116);
            this.label8.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(86, 17);
            this.label8.TabIndex = 0;
            this.label8.Text = "Board Type:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 86);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(67, 17);
            this.label3.TabIndex = 0;
            this.label3.Text = "Board ID:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(15, 54);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(102, 17);
            this.label2.TabIndex = 0;
            this.label2.Text = "Board Version:";
            // 
            // boardID
            // 
            this.boardID.AutoSize = true;
            this.boardID.Location = new System.Drawing.Point(143, 86);
            this.boardID.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.boardID.Name = "boardID";
            this.boardID.Size = new System.Drawing.Size(0, 17);
            this.boardID.TabIndex = 0;
            // 
            // boardVersion
            // 
            this.boardVersion.AutoSize = true;
            this.boardVersion.Location = new System.Drawing.Point(143, 54);
            this.boardVersion.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.boardVersion.Name = "boardVersion";
            this.boardVersion.Size = new System.Drawing.Size(0, 17);
            this.boardVersion.TabIndex = 0;
            // 
            // firmwareVersion
            // 
            this.firmwareVersion.AutoSize = true;
            this.firmwareVersion.Location = new System.Drawing.Point(143, 25);
            this.firmwareVersion.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.firmwareVersion.Name = "firmwareVersion";
            this.firmwareVersion.Size = new System.Drawing.Size(0, 17);
            this.firmwareVersion.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 25);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(121, 17);
            this.label1.TabIndex = 0;
            this.label1.Text = "Firmware Version:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(8, 238);
            this.label4.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(52, 17);
            this.label4.TabIndex = 2;
            this.label4.Text = "Status:";
            // 
            // selectHexFile
            // 
            this.selectHexFile.Location = new System.Drawing.Point(9, 15);
            this.selectHexFile.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.selectHexFile.Name = "selectHexFile";
            this.selectHexFile.Size = new System.Drawing.Size(276, 28);
            this.selectHexFile.TabIndex = 22;
            this.selectHexFile.Text = "Select VMX-Model Firmware to load...";
            this.selectHexFile.UseVisualStyleBackColor = true;
            this.selectHexFile.Click += new System.EventHandler(this.selectHexFile_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(8, 196);
            this.button1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(219, 28);
            this.button1.TabIndex = 23;
            this.button1.Text = "Update VMX-Model Firmware";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // label_NotReadyToUpdate
            // 
            this.label_NotReadyToUpdate.AutoSize = true;
            this.label_NotReadyToUpdate.Location = new System.Drawing.Point(35, 58);
            this.label_NotReadyToUpdate.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label_NotReadyToUpdate.Name = "label_NotReadyToUpdate";
            this.label_NotReadyToUpdate.Size = new System.Drawing.Size(545, 136);
            this.label_NotReadyToUpdate.TabIndex = 24;
            this.label_NotReadyToUpdate.Text = resources.GetString("label_NotReadyToUpdate.Text");
            this.label_NotReadyToUpdate.Click += new System.EventHandler(this.label5_Click);
            // 
            // hexFilePath
            // 
            this.hexFilePath.AutoSize = true;
            this.hexFilePath.Location = new System.Drawing.Point(293, 22);
            this.hexFilePath.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.hexFilePath.Name = "hexFilePath";
            this.hexFilePath.Size = new System.Drawing.Size(0, 17);
            this.hexFilePath.TabIndex = 1;
            // 
            // firmwareUpdateStatus
            // 
            this.firmwareUpdateStatus.Location = new System.Drawing.Point(8, 257);
            this.firmwareUpdateStatus.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.firmwareUpdateStatus.Multiline = true;
            this.firmwareUpdateStatus.Name = "firmwareUpdateStatus";
            this.firmwareUpdateStatus.ReadOnly = true;
            this.firmwareUpdateStatus.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.firmwareUpdateStatus.Size = new System.Drawing.Size(679, 149);
            this.firmwareUpdateStatus.TabIndex = 25;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(35, 79);
            this.label6.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(175, 17);
            this.label6.TabIndex = 24;
            this.label6.Text = "While VMX-Model device is powered on:";
            this.label6.Click += new System.EventHandler(this.label5_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(247, 199);
            this.progressBar1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(251, 25);
            this.progressBar1.TabIndex = 26;
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Location = new System.Drawing.Point(16, 15);
            this.tabControl1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(707, 447);
            this.tabControl1.TabIndex = 27;
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.button2);
            this.tabPage1.Controls.Add(this.progressBar1);
            this.tabPage1.Controls.Add(this.selectHexFile);
            this.tabPage1.Controls.Add(this.label4);
            this.tabPage1.Controls.Add(this.button1);
            this.tabPage1.Controls.Add(this.firmwareUpdateStatus);
            this.tabPage1.Controls.Add(this.hexFilePath);
            this.tabPage1.Controls.Add(this.label_ReadyToUpdate);
            this.tabPage1.Controls.Add(this.label_NotReadyToUpdate);
            this.tabPage1.Controls.Add(this.label6);
            this.tabPage1.Location = new System.Drawing.Point(4, 25);
            this.tabPage1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.tabPage1.Size = new System.Drawing.Size(699, 418);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Firmware Update";
            this.tabPage1.UseVisualStyleBackColor = true;
            this.tabPage1.Click += new System.EventHandler(this.tabPage1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(533, 15);
            this.button2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(155, 28);
            this.button2.TabIndex = 27;
            this.button2.Text = "Troubleshooting Tips";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // label_ReadyToUpdate
            // 
            this.label_ReadyToUpdate.AutoSize = true;
            this.label_ReadyToUpdate.Location = new System.Drawing.Point(33, 58);
            this.label_ReadyToUpdate.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label_ReadyToUpdate.Name = "label_ReadyToUpdate";
            this.label_ReadyToUpdate.Size = new System.Drawing.Size(647, 136);
            this.label_ReadyToUpdate.TabIndex = 24;
            this.label_ReadyToUpdate.Text = resources.GetString("label_ReadyToUpdate.Text");
            this.label_ReadyToUpdate.Click += new System.EventHandler(this.label5_Click);
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.label_VCP_Mode_Not_Ready);
            this.tabPage2.Controls.Add(this.label_VCP_Open_Ready);
            this.tabPage2.Controls.Add(this.stop_button);
            this.tabPage2.Controls.Add(this.groupBox2);
            this.tabPage2.Controls.Add(this.comboBox1);
            this.tabPage2.Controls.Add(this.start_button);
            this.tabPage2.Location = new System.Drawing.Point(4, 25);
            this.tabPage2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.tabPage2.Size = new System.Drawing.Size(699, 418);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Currently-loaded VMX-Model Firmware Version";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // label_VCP_Mode_Not_Ready
            // 
            this.label_VCP_Mode_Not_Ready.AutoSize = true;
            this.label_VCP_Mode_Not_Ready.Location = new System.Drawing.Point(27, 241);
            this.label_VCP_Mode_Not_Ready.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label_VCP_Mode_Not_Ready.Name = "label_VCP_Mode_Not_Ready";
            this.label_VCP_Mode_Not_Ready.Size = new System.Drawing.Size(627, 51);
            this.label_VCP_Mode_Not_Ready.TabIndex = 25;
            this.label_VCP_Mode_Not_Ready.Text = "VMX-Model device must be in Operational Mode to retrieve the currently-loaded Fir" +
    "mware Version.\r\n\r\nWhen in Operational Mode, the GREEN S1 and S2 LEDs should be O" +
    "N.";
            // 
            // label_VCP_Open_Ready
            // 
            this.label_VCP_Open_Ready.AutoSize = true;
            this.label_VCP_Open_Ready.Location = new System.Drawing.Point(23, 241);
            this.label_VCP_Open_Ready.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label_VCP_Open_Ready.Name = "label_VCP_Open_Ready";
            this.label_VCP_Open_Ready.Size = new System.Drawing.Size(541, 17);
            this.label_VCP_Open_Ready.TabIndex = 25;
            this.label_VCP_Open_Ready.Text = "Press the \'Open\' button to retrieve the currently-loaded firmware version informa" +
    "tion.";
            // 
            // timer2
            // 
            this.timer2.Enabled = true;
            this.timer2.Interval = 1000;
            this.timer2.Tick += new System.EventHandler(this.timer2_Tick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(749, 510);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.button_exit);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "Form1";
            this.Text = "VMXFirmware Updater";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.tabControl1.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.tabPage1.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button button_exit;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Button start_button;
        private System.Windows.Forms.Button stop_button;
        private System.IO.Ports.SerialPort port;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label boardID;
        private System.Windows.Forms.Label boardVersion;
        private System.Windows.Forms.Label firmwareVersion;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label boardType;
        private System.Windows.Forms.Button selectHexFile;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Label label_NotReadyToUpdate;
        private System.Windows.Forms.Label hexFilePath;
        private System.Windows.Forms.TextBox firmwareUpdateStatus;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.Label label_VCP_Open_Ready;
        private System.Windows.Forms.Timer timer2;
        private System.Windows.Forms.Label label_ReadyToUpdate;
        private System.Windows.Forms.Label label_VCP_Mode_Not_Ready;
        private System.Windows.Forms.Button button2;
    }
}

