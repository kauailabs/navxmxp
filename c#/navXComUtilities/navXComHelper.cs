using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Management;
using System.IO.Ports;
using System.Windows.Forms;

namespace navXComUtilities
{
    public class navXComHelper
    {
        enum STM32_USB_Interface_type
        {
            VCP,
            DFU,
        };

        static string crit_sec_lock = "CriticalSection";

        static public void InitPortComboBox(ComboBox comboBox1)
        {
            lock (crit_sec_lock)
            {
                string[] navx_port_names = navXComHelper.GetnavXSerialPortNames();
                int port_count = 0;
                foreach (string s in navx_port_names)
                {
                    comboBox1.Items.Add(s);
                    port_count++;
                }
                if (port_count > 0)
                {
                    comboBox1.SelectedIndex = 0;
                }
            }
        }

        static public void HandleComboBoxDropDownEvent(ComboBox comboBox1)
        {
            lock (crit_sec_lock)
            {
                int last_port_count = comboBox1.Items.Count;
                string last_selected_port = "";
                if (last_port_count > 0)
                {
                    last_selected_port = comboBox1.SelectedItem.ToString();
                }
                comboBox1.Items.Clear();
                int curr_index = 0;
                string[] navx_port_names = navXComHelper.GetnavXSerialPortNames();
                foreach (string s in navx_port_names)
                {
                    comboBox1.Items.Add(s);
                    if (s == last_selected_port)
                    {
                        comboBox1.SelectedIndex = curr_index;
                    }
                    curr_index++;
                }
            }
        }

        static public string[] GetnavXSerialPortNames()
        {
            lock (crit_sec_lock)
            {
                List<string> name_list = new List<string>();
                List<USBDeviceInfo> navx_usb_device_list = GetSTM32F4USBDevices(STM32_USB_Interface_type.VCP);
                string[] port_names = SerialPort.GetPortNames();
                foreach (USBDeviceInfo dev_info in navx_usb_device_list)
                {
                    foreach (string port_name in port_names)
                    {
                        if (dev_info.DeviceID == port_name)
                        {
                            name_list.Add(port_name);
                            break;
                        }
                    }
                }
                return name_list.ToArray<string>();
            }
        }

        static public bool IsDFUDevicePresent()
        {
            lock (crit_sec_lock)
            {
                bool dfu_device_present = false;
                List<USBDeviceInfo> navx_usb_device_list = GetSTM32F4USBDevices(STM32_USB_Interface_type.DFU);
                dfu_device_present = (navx_usb_device_list.Count() > 0);
                return dfu_device_present;
            }
        }

        static void GetMatchingDevicesInSearcher(ManagementObjectSearcher searcher, STM32_USB_Interface_type type, List<USBDeviceInfo> devices)
        {
            foreach (var device in searcher.Get())
            {
                try
                {
                    USBDeviceInfo devInfo = new USBDeviceInfo(
                    (string)device.GetPropertyValue("DeviceID"),
                    (string)device.GetPropertyValue("PNPDeviceID"),
                    (string)device.GetPropertyValue("Description"),
                    (string)device.GetPropertyValue("Name"),
                    (string)device.GetPropertyValue("Caption")
                    );
                    if (type == STM32_USB_Interface_type.VCP)
                    {
                        if (devInfo.PnpDeviceID.Contains("VID_0483") && /* ST Microelectronics */
                                devInfo.PnpDeviceID.Contains("PID_5740") /* STM32F407 */
                            )
                        {
                            devices.Add(devInfo);
                        }
                    }
                    if (type == STM32_USB_Interface_type.DFU)
                    {
                        if (devInfo.PnpDeviceID.Contains("VID_0483") && /* ST Microelectronics */
                                devInfo.PnpDeviceID.Contains("PID_DF11") /* STM Device in DFU Mode */
                            )
                        {
                            devices.Add(devInfo);
                        }
                    }
                }
                catch (Exception ex)
                {
                    string s = ex.Message;
                }
            }
        }

        static List<USBDeviceInfo> GetSTM32F4USBDevices(STM32_USB_Interface_type type)
        {
            lock (crit_sec_lock)
            {
                List<USBDeviceInfo> devices = new List<USBDeviceInfo>();

                ManagementObjectSearcher searcher = (ManagementObjectSearcher)null;

                if (type == STM32_USB_Interface_type.VCP)
                {
                    searcher = new ManagementObjectSearcher(@"Select * From Win32_SerialPort");
                }
                else
                {
                    searcher = new ManagementObjectSearcher(@"Select * From Win32_USBHub");
                }

                GetMatchingDevicesInSearcher(searcher, type, devices);

                // Starting in Windows 11, and with newer STM32 Bootloader installer,
                // The DFU device has been seen not present in Win32_USBHub devices;
                // however in this case it is present in Win32_PnpEntity; to handle this
                // case, search again in Win32_PnpEntity if not first found in Win32_USBHub.
                if ((devices.Count == 0) && (type == STM32_USB_Interface_type.DFU))
                {
                    searcher = new ManagementObjectSearcher(@"Select * From Win32_PnpEntity");
                    GetMatchingDevicesInSearcher(searcher, type, devices);
                }

                return devices;
            }
        }
    }

    public class USBDeviceInfo
    {
        public USBDeviceInfo(string deviceID,
                                string pnpDeviceID,
                                string description,
                                string name,
                                string caption)
        {
            this.DeviceID = deviceID;
            this.PnpDeviceID = pnpDeviceID;
            this.Description = description;
            this.Name = name;
            this.Caption = caption;
        }
        public string DeviceID { get; private set; }
        public string PnpDeviceID { get; private set; }
        public string Description { get; private set; }
        public string Name { get; private set; }
        public string Caption { get; private set; }
    }
}
