namespace navXConfig
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
            this.maskedTextBox1 = new System.Windows.Forms.MaskedTextBox();
            this.maskedTextBox2 = new System.Windows.Forms.MaskedTextBox();
            this.maskedTextBox3 = new System.Windows.Forms.MaskedTextBox();
            this.maskedTextBox4 = new System.Windows.Forms.MaskedTextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.port = new System.IO.Ports.SerialPort(this.components);
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.set_button1 = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button_restore_default1 = new System.Windows.Forms.Button();
            this.button_restore_default2 = new System.Windows.Forms.Button();
            this.button_restore_default3 = new System.Windows.Forms.Button();
            this.button_restore_default4 = new System.Windows.Forms.Button();
            this.restore_all_factory_defaults_button = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label9 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // button_exit
            // 
            this.button_exit.Location = new System.Drawing.Point(432, 282);
            this.button_exit.Name = "button_exit";
            this.button_exit.Size = new System.Drawing.Size(67, 21);
            this.button_exit.TabIndex = 0;
            this.button_exit.Text = "E&xit";
            this.button_exit.UseVisualStyleBackColor = true;
            this.button_exit.Click += new System.EventHandler(this.button_exit_Click);
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(38, 20);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(112, 21);
            this.comboBox1.TabIndex = 1;
            this.comboBox1.DropDown += new System.EventHandler(this.comboBox1_DropDown);
            // 
            // start_button
            // 
            this.start_button.Location = new System.Drawing.Point(179, 20);
            this.start_button.Name = "start_button";
            this.start_button.Size = new System.Drawing.Size(65, 23);
            this.start_button.TabIndex = 2;
            this.start_button.Text = "Open";
            this.start_button.UseVisualStyleBackColor = true;
            this.start_button.Click += new System.EventHandler(this.start_button_Click);
            // 
            // stop_button
            // 
            this.stop_button.Location = new System.Drawing.Point(250, 20);
            this.stop_button.Name = "stop_button";
            this.stop_button.Size = new System.Drawing.Size(65, 22);
            this.stop_button.TabIndex = 3;
            this.stop_button.Text = "Close";
            this.stop_button.UseVisualStyleBackColor = true;
            this.stop_button.Click += new System.EventHandler(this.stop_button_Click);
            // 
            // maskedTextBox1
            // 
            this.maskedTextBox1.Location = new System.Drawing.Point(159, 19);
            this.maskedTextBox1.Name = "maskedTextBox1";
            this.maskedTextBox1.Size = new System.Drawing.Size(74, 20);
            this.maskedTextBox1.TabIndex = 5;
            // 
            // maskedTextBox2
            // 
            this.maskedTextBox2.Location = new System.Drawing.Point(159, 57);
            this.maskedTextBox2.Name = "maskedTextBox2";
            this.maskedTextBox2.Size = new System.Drawing.Size(74, 20);
            this.maskedTextBox2.TabIndex = 6;
            // 
            // maskedTextBox3
            // 
            this.maskedTextBox3.Location = new System.Drawing.Point(159, 96);
            this.maskedTextBox3.Name = "maskedTextBox3";
            this.maskedTextBox3.Size = new System.Drawing.Size(74, 20);
            this.maskedTextBox3.TabIndex = 7;
            // 
            // maskedTextBox4
            // 
            this.maskedTextBox4.Location = new System.Drawing.Point(159, 24);
            this.maskedTextBox4.Name = "maskedTextBox4";
            this.maskedTextBox4.Size = new System.Drawing.Size(74, 20);
            this.maskedTextBox4.TabIndex = 8;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(8, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(71, 13);
            this.label1.TabIndex = 9;
            this.label1.Text = "Linear Motion";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(8, 62);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(47, 13);
            this.label2.TabIndex = 10;
            this.label2.Text = "Rotation";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(8, 99);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(111, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "Magnetic Disturbance";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(35, 237);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(148, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "Sea-level Barometric Pressure";
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // set_button1
            // 
            this.set_button1.Location = new System.Drawing.Point(319, 66);
            this.set_button1.Name = "set_button1";
            this.set_button1.Size = new System.Drawing.Size(67, 23);
            this.set_button1.TabIndex = 12;
            this.set_button1.Text = "Set";
            this.set_button1.UseVisualStyleBackColor = true;
            this.set_button1.Click += new System.EventHandler(this.set_button1_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(319, 106);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(67, 23);
            this.button1.TabIndex = 13;
            this.button1.Text = "Set";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(319, 143);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(67, 23);
            this.button2.TabIndex = 14;
            this.button2.Text = "Set";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(319, 231);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(67, 23);
            this.button3.TabIndex = 15;
            this.button3.Text = "Set";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button_restore_default1
            // 
            this.button_restore_default1.Location = new System.Drawing.Point(409, 66);
            this.button_restore_default1.Name = "button_restore_default1";
            this.button_restore_default1.Size = new System.Drawing.Size(90, 23);
            this.button_restore_default1.TabIndex = 16;
            this.button_restore_default1.Text = "Restore Default";
            this.button_restore_default1.UseVisualStyleBackColor = true;
            this.button_restore_default1.Click += new System.EventHandler(this.button_restore_default1_Click);
            // 
            // button_restore_default2
            // 
            this.button_restore_default2.Location = new System.Drawing.Point(409, 106);
            this.button_restore_default2.Name = "button_restore_default2";
            this.button_restore_default2.Size = new System.Drawing.Size(90, 23);
            this.button_restore_default2.TabIndex = 17;
            this.button_restore_default2.Text = "Restore Default";
            this.button_restore_default2.UseVisualStyleBackColor = true;
            this.button_restore_default2.Click += new System.EventHandler(this.button_restore_default2_Click);
            // 
            // button_restore_default3
            // 
            this.button_restore_default3.Location = new System.Drawing.Point(409, 143);
            this.button_restore_default3.Name = "button_restore_default3";
            this.button_restore_default3.Size = new System.Drawing.Size(90, 23);
            this.button_restore_default3.TabIndex = 18;
            this.button_restore_default3.Text = "Restore Default";
            this.button_restore_default3.UseVisualStyleBackColor = true;
            this.button_restore_default3.Click += new System.EventHandler(this.button_restore_default3_Click);
            // 
            // button_restore_default4
            // 
            this.button_restore_default4.Location = new System.Drawing.Point(409, 231);
            this.button_restore_default4.Name = "button_restore_default4";
            this.button_restore_default4.Size = new System.Drawing.Size(90, 23);
            this.button_restore_default4.TabIndex = 19;
            this.button_restore_default4.Text = "Restore Default";
            this.button_restore_default4.UseVisualStyleBackColor = true;
            this.button_restore_default4.Click += new System.EventHandler(this.button_restore_default4_Click);
            // 
            // restore_all_factory_defaults_button
            // 
            this.restore_all_factory_defaults_button.Location = new System.Drawing.Point(38, 280);
            this.restore_all_factory_defaults_button.Name = "restore_all_factory_defaults_button";
            this.restore_all_factory_defaults_button.Size = new System.Drawing.Size(180, 23);
            this.restore_all_factory_defaults_button.TabIndex = 16;
            this.restore_all_factory_defaults_button.Text = "Restore All Factory Defaults";
            this.restore_all_factory_defaults_button.UseVisualStyleBackColor = true;
            this.restore_all_factory_defaults_button.Click += new System.EventHandler(this.restore_all_factory_defaults_button_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label9);
            this.groupBox1.Controls.Add(this.maskedTextBox4);
            this.groupBox1.Location = new System.Drawing.Point(27, 209);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(486, 56);
            this.groupBox1.TabIndex = 20;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Environment";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(239, 27);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(38, 13);
            this.label9.TabIndex = 15;
            this.label9.Text = "millibar";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.maskedTextBox3);
            this.groupBox2.Controls.Add(this.maskedTextBox2);
            this.groupBox2.Controls.Add(this.maskedTextBox1);
            this.groupBox2.Location = new System.Drawing.Point(27, 49);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(486, 154);
            this.groupBox2.TabIndex = 21;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Thresholds";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(239, 99);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(15, 13);
            this.label8.TabIndex = 14;
            this.label8.Text = "%";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(239, 62);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(49, 13);
            this.label7.TabIndex = 13;
            this.label7.Text = "Deg/sec";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(239, 22);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(15, 13);
            this.label6.TabIndex = 12;
            this.label6.Text = "G";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(8, 129);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(170, 13);
            this.label5.TabIndex = 11;
            this.label5.Text = "(Lower Values = Higher Sensitivity)";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(525, 317);
            this.Controls.Add(this.button_restore_default4);
            this.Controls.Add(this.button_restore_default3);
            this.Controls.Add(this.button_restore_default2);
            this.Controls.Add(this.restore_all_factory_defaults_button);
            this.Controls.Add(this.button_restore_default1);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.set_button1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.stop_button);
            this.Controls.Add(this.start_button);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.button_exit);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBox2);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Form1";
            this.Text = "navX-Model Device Configuration";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button_exit;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.Button start_button;
        private System.Windows.Forms.Button stop_button;
        private System.Windows.Forms.MaskedTextBox maskedTextBox1;
        private System.Windows.Forms.MaskedTextBox maskedTextBox2;
        private System.Windows.Forms.MaskedTextBox maskedTextBox3;
        private System.Windows.Forms.MaskedTextBox maskedTextBox4;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.IO.Ports.SerialPort port;
        private System.Windows.Forms.Timer timer1;
        private System.Windows.Forms.Button set_button1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Button button_restore_default1;
        private System.Windows.Forms.Button button_restore_default2;
        private System.Windows.Forms.Button button_restore_default3;
        private System.Windows.Forms.Button button_restore_default4;
        private System.Windows.Forms.Button restore_all_factory_defaults_button;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
    }
}

