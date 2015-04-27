<?xml version="1.0"?>
<?CDF VERSION="4.0"?>
<INSTALLATION>
<SOFTPKG NAME="{06EF075D-271C-4585-B6DC-5756552CEFB1}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>Variable Legacy Protocol Support</TITLE>
	<ABSTRACT>Allows auto discovery of this target by variable browsers such as Distributed System Manager released prior to LabVIEW 2012, and allows this target to access network-published variables hosted by LabVIEW 8.6 or earlier.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/lksock_impl.out"/>
		<FEATURE SELECT="TARGET">
			<SOFTPKG NAME="{0304F75C-8202-42CA-99D4-C58297721FBF}" VERSION="5.5.0">
				<TITLE>Variable Legacy Server Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{AEC09CA3-9E70-4A06-9DF4-2AF031BFDB40}" VERSION="5.5.0">
				<TITLE>LOGOSXT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{CFFE2487-CD45-40DC-9ED7-10140BBE4467}" VERSION="5.5.0">
				<TITLE>LOGOS</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
lksock_impl.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{076E75BF-A643-415F-9E91-22EB4742763B}" VERSION="13.0.0" TYPE="VISIBLE" OLDESTCOMPATIBLEVERSION="3.0.0">
	<TITLE>Remote Panel Server for LabVIEW RT</TITLE>
	<ABSTRACT>Allows accessing VIs as remote panels. This version of the Remote Front Panel Server requires LabVIEW Real-Time 2010.  This component does not automatically downgrade.  Refer to the RT 2010 Known Issues for a workaround.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/LVModules/lvauthmodule.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/LVModules/lvrfpmodule.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="12.0.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{6B8CDAFE-4D77-4DF8-BB8F-EF0B0CF70EDE}" VERSION="13.0.0">
				<TITLE>Appweb</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
lvrt.out=libappweb.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{08C44C56-99A9-43E1-B37D-7994CE71CFE3}" VERSION="4.0.0" TYPE="VISIBLE">
	<TITLE>NI-Serial RT</TITLE>
	<ABSTRACT>NI-Serial support for LabVIEW Real-Time</ABSTRACT>
	<IMPLEMENTATION>
		<PROCESSOR VALUE="729D"/>
		<PROCESSOR VALUE="71C7"/>
		<PROCESSOR VALUE="718F"/>
		<PROCESSOR VALUE="7319"/>
		<PROCESSOR VALUE="7373"/>
		<PROCESSOR VALUE="73ED"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/niserial.ini"/>
		<CODEBASE FILENAME="/ni-rt/system/niserial.dbs"/>
		<FEATURE SELECT="NO">
			<SOFTPKG NAME="{00BEEB6C-7A65-4B8C-A03C-4B8D41247A13}" VERSION="4.0.0">
				<TITLE>NI-Serial 9870 and 9871 Scan Engine Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{3FA926C6-E783-11DB-9706-00E08161165F}" VERSION="4.0.0">
				<TITLE>NI-Serial RT OS-Dependent Layer</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{18108842-F963-4FE1-B709-1C79F745FF71}" VERSION="5.4.0" TYPE="VISIBLE" HIDEVERSION="YES" OLDESTCOMPATIBLEVERSION="5.0.0">
	<TITLE>NI-VISA ENET Passport</TITLE>
	<ABSTRACT>NI-VISA Passport for TCPIP resources</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="FIELDPOINT"/>
		<DEVICECLASS VALUE="SMART CAMERA"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/NiViEnet.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt/system/vxipnp/VxWorks/NIvisa/Passport/NiViEnet.ini">[PASSPORTS]
NumberOfPassports=1
passportEnabled0=1
LibName0=NiViEnet.out
LibDescription0=NI-VISA Passport for TCPIP and VXI-11

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
NiViEnet.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{37638B3C-551E-46D4-9503-2DBBB5B1132D}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>Hardware Configuration Web Support</TITLE>
	<ABSTRACT>Hardware Configuration Web Support</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/HardwareConfig.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/en/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/de/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/fr/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/ja/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/ko/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Help/zh-Hans/HardwareConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/HardwareConfig/Images/HardwareConfig.png"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{25D93C94-D8AB-4DEE-930D-7CC4B0EBB92B}" VERSION="2.0">
				<TITLE>NI Web-based Configuration and Monitoring</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.5.0">
				<TITLE>WIF core dependencies</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.5.0">
				<TITLE>NI System API Silverlight client</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{8946D3FB-8D14-4BFC-B401-2827F86364EB}" VERSION="5.5.0">
				<TITLE>NI System Configuration Remote Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{3B87558E-D9A4-49CC-9377-BEB8659CFD50}" VERSION="13.0.0.3.4" TYPE="VISIBLE" COMPATIBILITY="13.0.0">
	<TITLE>NI Vision RT</TITLE>
	<ABSTRACT>The core components required to use NI Vision with LabVIEW Real-Time.</ABSTRACT>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="736B"/>
		<EXCEPTPROCESSOR VALUE="73DE"/>
		<OS VALUE="VxWorks-PPC603"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{CAFF132A-40E3-4D34-9C2B-1664CB9E6AD5}" VERSION="13.0.0.3.4">
				<TITLE>NIVISSVC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{3FA926C6-E783-11DB-9706-00E08161165F}" VERSION="4.0.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="3.3.0">
	<TITLE>NI-Serial RT for VxWorks 6.3</TITLE>
	<ABSTRACT>NI-Serial RT OS-Dependent Layer</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/niserial.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.6.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
niserial.out=6.3

[LVRT]
StartupDLLs=niserial.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{497A2398-F021-46D5-B02E-4DB25E9080D6}" VERSION="3.1.0" TYPE="VISIBLE">
	<TITLE>System State Publisher</TITLE>
	<ABSTRACT>Publishes CPU and memory usage to Distributed System Manager</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/sysstatepublisher.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{6DEB7291-8EB1-4EB9-BB23-7745D8E122A8}" VERSION="1.0.0">
				<TITLE>Light-weight PSP control environment</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{39864828-E46E-461E-8780-9C50ADA29115}" VERSION="6.0">
				<TITLE>Base System</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
sysstatepublisher.out=6.3

[DEPENDENCIES]
sysstatepublisher.out=nilwpce.out;

[LVRT]
StartupDLLs=sysstatepublisher.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{5A0FD458-F336-4749-8548-9326219BB596}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>Software Management Web Support</TITLE>
	<ABSTRACT>Software Management Web Support</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/SoftwareManagement.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/en/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/de/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/fr/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/ja/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/ko/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Help/zh-Hans/SoftwareManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/SoftwareManagement/Images/SoftwareManagementIcon.png"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{25D93C94-D8AB-4DEE-930D-7CC4B0EBB92B}" VERSION="2.0">
				<TITLE>NI Web-based Configuration and Monitoring</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.5.0">
				<TITLE>WIF core dependencies</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.5.0">
				<TITLE>NI System API Silverlight client</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{6DEB7291-8EB1-4EB9-BB23-7745D8E122A8}" VERSION="2.2.0" TYPE="HIDDEN">
	<TITLE>Light-weight PSP control environment</TITLE>
	<ABSTRACT>Allows publishing information via NI-PSP</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nilwpce.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{CFFE2487-CD45-40DC-9ED7-10140BBE4467}" VERSION="5.4.0">
				<TITLE>LOGOS</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{AEC09CA3-9E70-4A06-9DF4-2AF031BFDB40}" VERSION="5.4.0">
				<TITLE>LOGOSXT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nilwpce.out=6.3

[DEPENDENCIES]
nilwpce.out=logosrt.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{78036FC0-F244-4AB4-9816-0A5415E3F22C}" VERSION="5.4.0" TYPE="VISIBLE" HIDEVERSION="YES" OLDESTCOMPATIBLEVERSION="5.0.0">
	<TITLE>NI-VISA ENET-ASRL Passport</TITLE>
	<ABSTRACT>NI-VISA Passport for ENET-Serial resources</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="FIELDPOINT"/>
		<DEVICECLASS VALUE="SMART CAMERA"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/NiEnAsrl.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt/system/vxipnp/VxWorks/NIvisa/Passport/NiEnAsrl.ini">[PASSPORTS]
NumberOfPassports=1
passportEnabled0=1
LibName0=NiEnAsrl.out
LibDescription0=NI-VISA Passport for Enet-Serial

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
NiEnAsrl.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{9992C06F-FBAC-47DE-912F-3F4A1CF6A284}" VERSION="5.4.0" TYPE="VISIBLE" HIDEVERSION="YES" OLDESTCOMPATIBLEVERSION="5.0.0">
	<TITLE>NI-VISA USB Passport</TITLE>
	<ABSTRACT>NI-VISA Passport for USB resources</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="FIELDPOINT"/>
		<DEVICECLASS VALUE="SMART CAMERA"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{9237F056-76AE-4B90-B914-58C7B8B32162}" VERSION="5.4.0">
				<TITLE>NI-VISA USB Passport Implementation</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{9237F056-76AE-4B90-B914-58C7B8B32162}" VERSION="5.4.0.0.2011" TYPE="HIDDEN">
	<TITLE>NI-VISA USB Passport Implemenation</TITLE>
	<ABSTRACT>USB passport based on the Jungo USB stack.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/NiViUsbj.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="11.0.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt/system/vxipnp/VxWorks/NIvisa/Passport/NiViUsbj.ini">[PASSPORTS]
NumberOfPassports=1
passportEnabled0=1
LibName0=NiViUsbj.out
LibDescription0=NI-VISA Passport for USB

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
NiViUsbj.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{A7E4BD0D-3E71-4ECD-B9FC-1D360BED0769}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>Time Configuration Web Support</TITLE>
	<ABSTRACT>Time Configuration Web Support</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/TimeConfig.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/en/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/de/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/fr/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/ja/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/ko/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Help/zh-Hans/TimeConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/TimeConfig/Images/TimeConfigIcon.png"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{25D93C94-D8AB-4DEE-930D-7CC4B0EBB92B}" VERSION="1.0.1">
				<TITLE>NI Web-based Configuration and Monitoring</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.3.0">
				<TITLE>WIF core dependencies</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.3.0">
				<TITLE>NI System API Silverlight client</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{B7C27E68-5D07-4679-8610-6D67381E9EFB}" VERSION="5.4.0" TYPE="VISIBLE" HIDEVERSION="YES" OLDESTCOMPATIBLEVERSION="5.0.0">
	<TITLE>NI-VISA Remote Passport</TITLE>
	<ABSTRACT>NI-VISA Passport for Remote resources</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="FIELDPOINT"/>
		<DEVICECLASS VALUE="SMART CAMERA"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/NiViRpc.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="4.0.0">
				<TITLE>NI-RPC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt/system/vxipnp/VxWorks/NIvisa/Passport/NiViRpc.ini">[PASSPORTS]
NumberOfPassports=1
passportEnabled0=1
LibName0=NiViRpc.out
LibDescription0=NI-VISA Passport for Remote NI-VISA

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
NiViRpc.out=nirpcs.out;

[MODULE VERSIONS]
NiViRpc.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{BE793622-69E8-4F0F-B3AB-DB06517FF276}" VERSION="13.0" TYPE="VISIBLE">
	<TITLE>CompactRIO Support</TITLE>
	<ABSTRACT>CompactRIO FPGA Support</ABSTRACT>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="7527"/>
		<EXCEPTPROCESSOR VALUE="758B"/>
		<EXCEPTPROCESSOR VALUE="75C8"/>
		<EXCEPTPROCESSOR VALUE="75C7"/>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.const.zh-CN.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nirio.HardSwitch.def.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/nicrio.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/nicrio.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/nicrio.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/nicrio.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/nicrio.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/criomdk.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/criomdk.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/criomdk.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/criomdk.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/criomdk.err"/>
		<CODEBASE FILENAME="/ni-rt/system/criosd.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{AB0CCF83-3A5E-402E-BB66-889B00C3ED9D}" VERSION="13.0.0">
				<TITLE>NI-RIO Mite Board Driver</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="4.2">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E62A7BA2-8ED9-4C4A-AFA1-0DE33A188D61}" VERSION="4.2">
				<TITLE>NI-VISA Server</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{0D328313-47CD-48C8-9A83-0CCCD96D2919}" VERSION="12.1.0">
				<TITLE>cRIO Icons</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[LVRT]

[MODULE VERSIONS]
criosd.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{0D328313-47CD-48C8-9A83-0CCCD96D2919}" VERSION="13.0" TYPE="HIDDEN">
	<TITLE>cRIO Icons</TITLE>
	<ABSTRACT>Icons for cRIO targets for Network Browser display</ABSTRACT>
	<IMPLEMENTATION>
		<PROCESSOR VALUE="71C7"/>
		<PROCESSOR VALUE="729D"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/907x.png"/>
		<MERGEINI TARGET="/ni-rt.ini">[MDNSRESPONDER]
ImgPath=/WIF/907x.png

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{AB0CCF83-3A5E-402E-BB66-889B00C3ED9D}" VERSION="13.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="13.0.0">
	<TITLE>NI-RIO Mite Board Driver</TITLE>
	<ABSTRACT>NI-RIO Mite Board Driver</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/niriomtk.out"/>
		<CODEBASE FILENAME="/ni-rt/system/dnf/niriomtk.dnf"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{56032245-0F4B-4641-9D74-33D067EB32B3}" VERSION="13.0.0">
				<TITLE>NI-RIO</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{9BB7AF74-8292-4A50-9124-8C4BA7D60C88}" VERSION="1.0.1">
				<TITLE>NI-RTDM</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{56032245-0F4B-4641-9D74-33D067EB32B3}" VERSION="13.0" TYPE="VISIBLE" OLDESTCOMPATIBLEVERSION="4.0.0">
	<TITLE>NI-RIO</TITLE>
	<ABSTRACT>NI-RIO Driver for Reconfigurable I/O in LabVIEW FPGA</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="PXI"/>
		<DEVICECLASS VALUE="DESKTOP"/>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/nivisa/passport/nivirio.ini"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/niriofcf.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/niriofcf.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/niriofcf.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/niriofcf.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/niriofcf.err"/>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{BE793622-69E8-4F0F-B3AB-DB06517FF276}" VERSION="4.1">
				<TITLE>CompactRIO Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{09ACAAD5-7A7A-49C9-831C-F78936EE8EC1}" VERSION="13.0">
				<TITLE>NI-RIO Driver for VxWorks</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{742C82AD-223A-4CCE-AFEC-55D0C1A659E7}" VERSION="2.0.1">
				<TITLE>NI-APAL</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{450CA255-E9A6-43B4-B86D-2B76F7057D85}" VERSION="2.9.0">
				<TITLE>NI-PAL Errors</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7370B6F6-DEB9-4241-8C6C-A39804097C8B}" VERSION="7.1">
				<TITLE>LabVIEW FPGA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{09ACAAD5-7A7A-49C9-831C-F78936EE8EC1}" VERSION="13.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="2.3.0">
	<TITLE>NI-RIO Driver for VxWorks 6.3</TITLE>
	<ABSTRACT>NI-RIO Driver for VxWorks 6.3</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/nivirio.out"/>
		<CODEBASE FILENAME="/ni-rt/system/niriosrv.out"/>
		<CODEBASE FILENAME="/ni-rt/system/niriorpc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/NiFpga.out"/>
		<CODEBASE FILENAME="/ni-rt/system/NiFpgaLv.out"/>
		<CODEBASE FILENAME="/ni-rt/system/niriosae.out"/>
		<CODEBASE FILENAME="/ni-rt/system/dnf/NiRioSrv.dnf"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.6">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="3.4.1">
				<TITLE>NI-RPC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{39864828-E46E-461E-8780-9C50ADA29115}" VERSION="7.0">
				<TITLE>Base System</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{9BB7AF74-8292-4A50-9124-8C4BA7D60C88}" VERSION="1.0.1">
				<TITLE>NI-RTDM</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="1.1.0">
				<TITLE>NI System API</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[LVRT]
StartupDlls=NiRioRpc.out;

[MODULE VERSIONS]
nivirio.out=6.3
niriosrv.out=6.3
niriorpc.out=6.3
NiFpga.out=6.3
NiFpgaLv.out=6.3
niriosae.out=6.3

[DEPENDENCIES]
NiFpga.out=niriosrv.out;
NiRioSrv.out=libexpat.out;nirpcs.out;lvrt.out;
NiRioRpc.out=NiRioSrv.out;nisvcloc.out;
NiViRio.out=NiRioSrv.out;
niriosae.out=nisysapi.out;niriosrv.out;
NiFpgaLv.out=NiFpga.out;

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt/system/nisysapi.ini">[NIRIOSAE]
Path=niriosae.out
ExpertFactory=NiRio_createSysApiExpert

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{7370B6F6-DEB9-4241-8C6C-A39804097C8B}" VERSION="13.0" TYPE="HIDDEN">
	<TITLE>LabVIEW FPGA</TITLE>
	<ABSTRACT>Files necessary to use LabVIEW FPGA and the FPGA Interface on a RT system.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/lvfpga.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/lvfpga.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/lvfpga.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/lvfpga.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/lvfpga.err"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{742C82AD-223A-4CCE-AFEC-55D0C1A659E7}" VERSION="2.3.0" TYPE="HIDDEN">
	<TITLE>NI-APAL</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/niapal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/niapal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/niapal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/niapal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/korean/niapal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/niapal.err"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CAFF132A-40E3-4D34-9C2B-1664CB9E6AD5}" VERSION="13.0.0.3.4" TYPE="HIDDEN" COMPATIBILITY="13.0.0">
	<TITLE>NIVISSVC</TITLE>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="736B"/>
		<EXCEPTPROCESSOR VALUE="73DE"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{8BD4ED5E-A4AA-412E-AD58-F14178284102}" VERSION="13.0.0.3.4">
				<TITLE>NI Vision Common</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7626BCF0-C9F4-4BD2-A6B6-DAD808625076}" VERSION="13.0.0.3.4">
				<TITLE>NIVISSVC Target Options</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{7626BCF0-C9F4-4BD2-A6B6-DAD808625076}" VERSION="13.0.0.3.4" TYPE="HIDDEN" COMPATIBILITY="13.0.0">
	<TITLE>NIVISSVC Target Options</TITLE>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="73EA"/>
		<EXCEPTPROCESSOR VALUE="71EC"/>
		<EXCEPTPROCESSOR VALUE="736B"/>
		<EXCEPTPROCESSOR VALUE="73DF"/>
		<EXCEPTPROCESSOR VALUE="73DE"/>
		<OS VALUE="VxWorks-PPC603"/>
		<MERGEINI TARGET="/ni-rt.ini">[VISION]
WriteFileAllowed=TRUE

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{8BD4ED5E-A4AA-412E-AD58-F14178284102}" VERSION="13.0.0.3.4" TYPE="HIDDEN" COMPATIBILITY="13.0.0">
	<TITLE>NI Vision Common</TITLE>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="736B"/>
		<EXCEPTPROCESSOR VALUE="73DE"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nivissvc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/nivision.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/french/nivision.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/german/nivision.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/nivision.err"/>
		<CODEBASE FILENAME="/ni-rt/system/nivision.out"/>
		<CODEBASE FILENAME="/vision/ni_vision.ttf"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="3.4.0">
				<TITLE>NI-RPC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{84C09D5C-116F-40A1-81AE-7037903B98BD}" VERSION="2.1.0">
				<TITLE>NI-PAL</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nivision.out=6.3
nivissvc.out=6.3

[DEPENDENCIES]
nivision.out=nivissvc.out;
nivissvc.out=lvrt.out;nirpcs.out;nipals.out;

[LVRT]
StartupDLLs=nivissvc.out;nivision.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{84C09D5C-116F-40A1-81AE-7037903B98BD}" VERSION="2.9.1" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="2.1.0">
	<TITLE>NI-PAL</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nipals.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{450CA255-E9A6-43B4-B86D-2B76F7057D85}" VERSION="2.9">
				<TITLE>NI-PAL Errors</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nipals.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{450CA255-E9A6-43B4-B86D-2B76F7057D85}" VERSION="2.9.1" TYPE="HIDDEN">
	<TITLE>NI-PAL Errors</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/nipal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/nipal.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/nipal.err"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CBFAAEAA-D69C-4295-A511-2988A7CEC9B4}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>Network Configuration Web Support</TITLE>
	<ABSTRACT>Network Configuration Web Support</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/NetworkConfig.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/en/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/de/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/fr/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/ja/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/ko/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Help/zh-Hans/NetworkConfigHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkConfig/Images/NetworkConfigIcon.png"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{25D93C94-D8AB-4DEE-930D-7CC4B0EBB92B}" VERSION="1.0.1">
				<TITLE>NI Web-based Configuration and Monitoring</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.5.0">
				<TITLE>WIF core dependencies</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.5.0">
				<TITLE>NI System API Silverlight client</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{25D93C94-D8AB-4DEE-930D-7CC4B0EBB92B}" VERSION="13.0.0" TYPE="VISIBLE">
	<TITLE>NI Web-based Configuration and Monitoring</TITLE>
	<ABSTRACT>Enables remote configuration and monitoring of the system from a standard Web browser.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/wif_core/WebService.ini"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/wif_core/wif_core.out"/>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{37638B3C-551E-46D4-9503-2DBBB5B1132D}">
				<TITLE>Hardware Configuration Web Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{CBFAAEAA-D69C-4295-A511-2988A7CEC9B4}">
				<TITLE>Network Configuration Web Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="NO">
			<SOFTPKG NAME="{0B37CBE4-E653-4A61-B098-01E097D85E24}">
				<TITLE>Network Browser Web Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{5A0FD458-F336-4749-8548-9326219BB596}">
				<TITLE>Software Management Web Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{A7E4BD0D-3E71-4ECD-B9FC-1D360BED0769}">
				<TITLE>Time Configuration Web Support</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{AD22C016-BED0-4996-BA2C-FF6671BC4553}" VERSION="13.0.0">
				<TITLE>WIF Core</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{AD22C016-BED0-4996-BA2C-FF6671BC4553}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>WIF Core</TITLE>
	<ABSTRACT>Core WIF files shared by all platform installations</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF.html"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WIFCore.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Console.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/en/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/de/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/fr/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/ja/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/ko/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Help/zh-Hans/ConsoleHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Console/Images/ConsoleIcon.png"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/FileBrowser.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/en/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/de/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/fr/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/ja/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/ko/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Help/zh-Hans/FileBrowserHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/FileBrowser/Images/FileBrowserIcon.png"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/WIFNIAuth.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Images/WIFNIAuthIcon.png"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/en/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/de/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/fr/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/ja/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/ko/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NIAuth/Help/zh-Hans/WIFNIAuthHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/WebServerConfigExtension.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Images/WebServerConfigExtensionIcon.png"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/en/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/de/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/fr/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/ja/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/ko/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServerConfigExtension/Help/zh-Hans/WebServerConfigExtensionHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/ExtensionMetaData.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/WebServicesManagement.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/DisplayInfo.zh-Hans.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/en/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/de/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/fr/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/ja/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/ko/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Help/zh-Hans/WebServicesManagementHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/WebServicesManagement/Images/WebServicesManagementIcon.png"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/add.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/arraytools.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/arrow_close.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/arrow_open.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/blueleft.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/blueright.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/box.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/caution.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/checkinstalled.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/common.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/cssframes.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/diamond.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/domtools.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/dynamiccontent.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/dynamicjumps.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/dynamiclinks.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/expandable_tree.css"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/expandable_tree.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/feedbacklink.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/find.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/hyphen.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/initpagedyn.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/jump.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/launchhelp.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/linking.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/minibutton.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/minimal.css"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/noloc_env_wif_security.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/note.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/open.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/pdf.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/placeholder.txt"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/polyviselect.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/print.css"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/stylesheets.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/support.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/tip.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/top.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/unsupport.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/variables.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Help/web.js"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/de/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/de/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/en/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/en/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/fr/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/fr/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/ja/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/ja/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/ko/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/ko/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/zh-Hans/ExtensionsHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/Core/Help/zh-Hans/WIFHelp.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.DynamicNavigation.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.Security.Cryptography.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.ComponentModel.DataAnnotations.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Data.Input.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Data.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Input.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Layout.Toolkit.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Navigation.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.Toolkit.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Controls.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Windows.Data.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Xml.Linq.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Xml.Serialization.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/System.Xml.XPath.zip"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="13.0.0">
				<TITLE>Run-Time Engine for Web Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{DB292C60-9332-468F-8A47-43D66E33BB70}" VERSION="13.0.0">
				<TITLE>NI File System Web Service</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{DB69F6E1-DBDC-461E-A383-5052023409A7}" VERSION="13.0.0">
				<TITLE>RT Exec Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{DCE66C0D-AF78-4C84-90E5-A0729B911429}" VERSION="13.0.0">
				<TITLE>NIAuth Web Service</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{035BDBDB-A575-408A-B28D-7AEB597D1123}" VERSION="13.0.0">
				<TITLE>NI System Web Server</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{AAD95C0A-FD47-4CDD-821D-C4B4EF76DDDA}" VERSION="2.1.0">
				<TITLE>mDNSResponder</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.3.0">
				<TITLE>WIF core dependencies</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{75BE83A6-6097-4DD3-8E3E-5D6384A1D095}" VERSION="5.5.0" TYPE="HIDDEN">
	<TITLE>WIF core dependencies</TITLE>
	<ABSTRACT>WIF core dependencies</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.Config.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.Config.MAX.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.Config.SysConfig.zip"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NetworkBrowserControl.zip"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.5.0">
				<TITLE>NI System API Silverlight client</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{7E56CD7B-42D6-46E0-8638-DE2D819A3A9D}" VERSION="5.5.0" TYPE="HIDDEN">
	<TITLE>NI System API Silverlight client</TITLE>
	<ABSTRACT>NI System API Silverlight client</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/www/WIF/NationalInstruments.SystemApi.zip"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="5.5.0">
				<TITLE>NI System API</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{8946D3FB-8D14-4BFC-B401-2827F86364EB}" VERSION="5.5.0">
				<TITLE>NI System Configuration Remote Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{8946D3FB-8D14-4BFC-B401-2827F86364EB}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>NI System Configuration Remote Support</TITLE>
	<ABSTRACT>NI System Configuration Remote Support provides web services for network discovery and configuration. This component requires LabVIEW RT 2010.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/nisysapi/nisysapisvc.out"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/nisysapi/WebService.ini"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationExt.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.CalibrationInt.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.Device.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.DiagnosticReset.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.DiagnosticReset.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.DiagnosticSelfTest.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.DiagnosticSelfTest.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.FirmwareUpdate.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.NetworkDevice.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.PciPxiDevice.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.RenamableDevice.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.binding.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.de.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.fr.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.ja.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.ko.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.const.zh-CN.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.System.def.xml" VERSION="2.0"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.const.zh-CN.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.SystemRsrc.def.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.soft_dip_switch.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nisyscfg.soft_dip_switch.def.xml"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="5.5.0">
				<TITLE>NI System API</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="10.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{035BDBDB-A575-408A-B28D-7AEB597D1123}" VERSION="1.0">
				<TITLE>NI System Web Server</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="3.0">
				<TITLE>Run-Time Engine for Web Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{DCE66C0D-AF78-4C84-90E5-A0729B911429}" VERSION="1.0">
				<TITLE>NIAuth Web Service</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nisysapisvc.out=6.3

[DEPENDENCIES]
nisysapisvc.out=nisysrpc.out;nisysapi.out;nimdnsResponder.out;ws_runtime.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CFFE2487-CD45-40DC-9ED7-10140BBE4467}" VERSION="5.5.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="5.4.0">
	<TITLE>Logos</TITLE>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="7350"/>
		<EXCEPTPROCESSOR VALUE="7351"/>
		<EXCEPTPROCESSOR VALUE="72B4"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/logosrt.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{AEC09CA3-9E70-4A06-9DF4-2AF031BFDB40}" VERSION="5.5.0">
				<TITLE>LOGOSXT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="9.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{AAD95C0A-FD47-4CDD-821D-C4B4EF76DDDA}" VERSION="1.6.0">
				<TITLE>mDNSResponder</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
logosrt.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{AEC09CA3-9E70-4A06-9DF4-2AF031BFDB40}" VERSION="5.5.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="4.9.0">
	<TITLE>LogosXT</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nilxtcor.out"/>
		<CODEBASE FILENAME="/ni-rt/system/nipspxts.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{BB1E1CEC-68D6-4240-A8A4-8560C0CE3B4A}" VERSION="5.5.0">
				<TITLE>LogosXT Platform Specific INI Settings</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="9.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nilxtcor.out=6.3
nipspxts.out=6.3

[LOGOSXT]
Heartbeat_AbsenceDetectCount=10

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{BB1E1CEC-68D6-4240-A8A4-8560C0CE3B4A}" VERSION="5.5.0" TYPE="HIDDEN">
	<TITLE>LogosXT Platform Specific INI Settings</TITLE>
	<IMPLEMENTATION>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{DB292C60-9332-468F-8A47-43D66E33BB70}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>NI File System Web Service</TITLE>
	<ABSTRACT>RESTful web service API to a target's file system.  Allows for basic file manipulation as well as upload and download capabilities.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/fsws/FilesystemWebService.out"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/fsws/WebService.ini"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="13.0.0">
				<TITLE>Run-Time Engine for Web Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{DB69F6E1-DBDC-461E-A383-5052023409A7}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>RT Exec Services</TITLE>
	<ABSTRACT>RESTful web service API to a target's rt_exec functionality.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/rtexecsvc/rtexecsvc.out"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/rtexecsvc/WebService.ini"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{39864828-E46E-461E-8780-9C50ADA29115}" VERSION="9.0">
				<TITLE>Base System</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="13.0.0">
				<TITLE>Run-Time Engine for Web Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{DCE66C0D-AF78-4C84-90E5-A0729B911429}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>NIAuth Web Service</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/LVWSAuthSvc/niauthsvc.out"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/LVWSAuthSvc/WebService.ini"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{44ECF1A3-8172-437D-B955-31B46BD96F17}" VERSION="13.0.0">
				<TITLE>NIAuth</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="13.0.0">
				<TITLE>Run-Time Engine for Web Services</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{5125BB03-DF31-4690-A87C-CF17DEF35FEA}" VERSION="13.0.0" TYPE="VISIBLE">
	<TITLE>Run-Time Engine for Web Services</TITLE>
	<ABSTRACT>Allows LabVIEW VIs to be executed as Web Services.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/ws_runtime.out"/>
		<CODEBASE FILENAME="/tmp/webservices/WebServices.ini"/>
		<CODEBASE FILENAME="/tmp/tracelogs/ws_shared.cfg"/>
		<CODEBASE FILENAME="/tmp/tracelogs/ws_runtime.cfg"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/LVWSSysAdmin/WebService.ini"/>
		<CODEBASE FILENAME="/tmp/webservices/NI/LVWSSysAdmin/sysadminsvc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/services/LVWSDebugSvc/WebService.ini"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/services/LVWSDebugSvc/debugsvc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/ws_repl.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/niwsdebugserver.conf.template"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/ws_www/login.html"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/ws_www/LoginPage.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/ws_www/clientaccesspolicy.xml"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{A9AA9E46-3C43-4540-BDA0-5555C3AE8BD4}" VERSION="3.0.0">
				<TITLE>Trace Engine</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{035BDBDB-A575-408A-B28D-7AEB597D1123}" VERSION="13.0.0">
				<TITLE>NI System Web Server</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
ws_runtime.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{035BDBDB-A575-408A-B28D-7AEB597D1123}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>NI System Web Server</TITLE>
	<ABSTRACT>The NI System Web Server loads NI built web services.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/RTWebServer.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/NISystemWebServer.conf"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/NISystemWebServer.ini.defaults"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/modules/mod_niauth.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/modules/mod_niconf.out"/>
		<CODEBASE FILENAME="/ni-rt/system/www/login.html"/>
		<CODEBASE FILENAME="/ni-rt/system/www/LoginPage.xap"/>
		<CODEBASE FILENAME="/ni-rt/system/www/clientaccesspolicy.xml"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{44ECF1A3-8172-437D-B955-31B46BD96F17}" VERSION="13.0.0">
				<TITLE>NIAuth</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="10.0.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{6B8CDAFE-4D77-4DF8-BB8F-EF0B0CF70EDE}" VERSION="13.0.0">
				<TITLE>Appweb</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{1C8F8B62-ED51-4F34-857F-436D29168FBD}" VERSION="13.0.0">
				<TITLE>Service Locator</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{CE844AFA-EE70-4CAB-A33B-F0A90C76A707}" VERSION="13.0.0">
				<TITLE>GMP</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
RTWebServer.out=libappweb.out;nisvcloc.out;
mod_niauth.out=nigmp.out;

[STARTUP]
EarlyStartupLibraries=RTWebServer.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{44ECF1A3-8172-437D-B955-31B46BD96F17}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>NIAuth</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/NIAuth/niauth.out"/>
		<CODEBASE FILENAME="/ni-rt/system/NIAuth/niPortableRegistry.out"/>
		<CODEBASE FILENAME="/tmp/tracelogs/niauth.cfg"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{CE844AFA-EE70-4CAB-A33B-F0A90C76A707}" VERSION="13.0.0">
				<TITLE>GMP</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
niauth.out=nigmp.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{6B8CDAFE-4D77-4DF8-BB8F-EF0B0CF70EDE}" VERSION="13.0.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="13.0.0">
	<TITLE>Appweb</TITLE>
	<ABSTRACT>Installs libappweb, its modules (libcopymodule, libespmodule, libdirmodule) and mime.types.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/libpcre.out"/>
		<CODEBASE FILENAME="/ni-rt/system/libmpr.out"/>
		<CODEBASE FILENAME="/ni-rt/system/libhttp.out"/>
		<CODEBASE FILENAME="/ni-rt/system/libappweb.out"/>
		<CODEBASE FILENAME="/ni-rt/system/libappwebcore.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/modules/mod_niesp.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/modules/mod_nisessmgr.out"/>
		<CODEBASE FILENAME="/ni-rt/system/webserver/mime.types"/>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
libmpr.out=libpcre.out;
libhttp.out=libmpr.out;
libappwebcore.out=libhttp.out;
libappweb.out=libappwebcore.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{A9AA9E46-3C43-4540-BDA0-5555C3AE8BD4}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>Trace Engine</TITLE>
	<ABSTRACT>Trace Engine collates and outputs debug trace messages from any component that uses it.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/ni_traceengine.out"/>
		<CODEBASE FILENAME="/tmp/tracelogs/traceengine.ini"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
ni_traceengine.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{E62A7BA2-8ED9-4C4A-AFA1-0DE33A188D61}" VERSION="5.4.0" TYPE="VISIBLE" OLDESTCOMPATIBLEVERSION="4.2">
	<TITLE>NI-VISA Server</TITLE>
	<ABSTRACT>Remote VISA Server -- Shares VISA resources over the network.</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<DEVICECLASS VALUE="FIELDPOINT"/>
		<DEVICECLASS VALUE="SMART CAMERA"/>
		<DEVICECLASS VALUE="WSN GATEWAY"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/NiViSrvr.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="4.2">
				<TITLE>NI-VISA</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="4.0.0">
				<TITLE>NI-RPC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[LVRT]
StartupDlls=NiViSrvr.out;

[DEPENDENCIES]
NiViSrvr.out=nirpcs.out;

[MODULE VERSIONS]
NiViSrvr.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{2C9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0" TYPE="VISIBLE">
	<TITLE>NI-VISA</TITLE>
	<ABSTRACT>VISA provides an API for instrument control. This component includes VISA core and the Passport for Serial resources. You may select/deselect the components below to configure other Passports.</ABSTRACT>
	<IMPLEMENTATION>
		<DEVICECLASS VALUE="CRIO"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/NIvisa/Visaconf.ini"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/English/VISA.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/German/VISA.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/French/VISA.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/Japanese/VISA.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/Korean/VISA.err"/>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{78036FC0-F244-4AB4-9816-0A5415E3F22C}" VERSION="5.4.0">
				<TITLE>NI-VISA ENET-ASRL Passport</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{18108842-F963-4FE1-B709-1C79F745FF71}" VERSION="5.4.0">
				<TITLE>NI-VISA ENET Passport</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="NO">
			<SOFTPKG NAME="{1B4AB45D-F85F-4BDA-A150-3B27A608E9FB}" VERSION="5.4.0">
				<TITLE>NI-VISA PXI Passport</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{9992C06F-FBAC-47DE-912F-3F4A1CF6A284}" VERSION="5.4.0">
				<TITLE>NI-VISA USB Passport</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{B7C27E68-5D07-4679-8610-6D67381E9EFB}" VERSION="5.4.0">
				<TITLE>NI-VISA Passport for Remote Resources</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="YESDEPS">
			<SOFTPKG NAME="{E62A7BA2-8ED9-4C4A-AFA1-0DE33A188D61}" VERSION="5.4.0">
				<TITLE>NI-VISA Server</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{3D9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0">
				<TITLE>VISA RT for VxWorks</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{3D9F0C7B-DB26-40B1-A078-ABFB614C013E}" VERSION="5.4.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="4.2.0">
	<TITLE>NI-VISA for VxWorks 6.3</TITLE>
	<ABSTRACT>VISA provides an API for instrument control. This component includes VISA core and the Passport for Serial resources. You may select/deselect the components below to configure other Passports.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/NiViSv32.out"/>
		<CODEBASE FILENAME="/ni-rt/system/visa32.out"/>
		<CODEBASE FILENAME="/ni-rt/system/NiViSys.out"/>
		<CODEBASE FILENAME="/ni-rt/system/vxipnp/VxWorks/bin/NiViAsrl.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="1.1.0">
				<TITLE>NI System API</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{89E9C488-FBB0-4934-ABF7-627E8E37CF40}" VERSION="5.4.0">
				<TITLE>NI-VISA UI XML</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
visa32.out=NiViSv32.out;
NiViSys.out=nisysapi.out;visa32.out;

[MODULE VERSIONS]
NiViSv32.out=6.3
NiViAsrl.out=6.3
visa32.out=6.3
NiViSys.out=6.3

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt/system/vxipnp/VxWorks/NIvisa/Passport/NiViAsrl.ini">[PASSPORTS]
NumberOfPassports=1
passportEnabled0=1
LibName0=NiViAsrl.out
LibDescription0=NI-VISA Passport for Serial

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt/system/nisysapi.ini">[NI-VISA]
Path=/ni-rt/system/NiViSys.out
ExpertFactory=createNIVISASysApiExpert

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{89E9C488-FBB0-4934-ABF7-627E8E37CF40}" VERSION="5.4.0" TYPE="HIDDEN">
	<TITLE>NI-VISA UI XML</TITLE>
	<ABSTRACT>UI XML for NI-VISA</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="PharLap"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.def.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.device.const.zh-CN.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.def.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.serial.const.zh-CN.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.binding.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.def.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.de.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.fr.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.ja.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.ko.xml"/>
		<CODEBASE FILENAME="/ni-rt/system/uixml/sysconfig/nivisa.usb.const.zh-CN.xml"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{F45FF3C8-2940-4E60-87ED-2BC3D2301D21}" VERSION="13.0.0" TYPE="VISIBLE">
	<TITLE>HTTP Client</TITLE>
	<ABSTRACT>This is used by the LabVIEW HTTP Client palette and does not include support for HTTPS connections.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/ni_httpClient_nossl.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="13.0.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{CE844AFA-EE70-4CAB-A33B-F0A90C76A707}" VERSION="13.0.0">
				<TITLE>GMP</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[DEPENDENCIES]
ni_httpClient_nossl.out=nigmp.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CE844AFA-EE70-4CAB-A33B-F0A90C76A707}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>GMP</TITLE>
	<ABSTRACT>NI Implementation of GMP</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nigmp.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nigmp.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{F8064116-D2A9-4463-9CD7-F9C464E360BB}" VERSION="5.5.0" TYPE="VISIBLE">
	<TITLE>NI System Configuration</TITLE>
	<ABSTRACT>NI System Configuration API provides libraries and other files necessary to support applications that discover and configure systems and devices.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nisyscfg.out"/>
		<CODEBASE FILENAME="/ni-rt/system/mxRmCfg.out"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/nisyscfg.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/japanese/nisyscfg.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/chineses/nisyscfg.err"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="5.5.0">
				<TITLE>NI System API</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nisyscfg.out=6.3

[DEPENDENCIES]
nisyscfg.out=nisysapi.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{5FEB7A50-4EFF-416F-9EE9-EC94006BAA66}" VERSION="5.5.0" TYPE="HIDDEN">
	<TITLE>NI System API</TITLE>
	<ABSTRACT>NI System API</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nisysapi.out"/>
		<CODEBASE FILENAME="/ni-rt/system/nisysrpc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/nisyscfgExpert.out"/>
		<CODEBASE FILENAME="/ni-rt/system/timezone.ini"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="10.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{79BBD504-B94E-4D53-A2D0-28983B2BD2C4}" VERSION="1.0">
				<TITLE>Target Configuration</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="4.2.0">
				<TITLE>NI-RPC</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[LVRT]
StartupDLLs=nisysrpc.out;

[DEPENDENCIES]
nisysrpc.out=nirpcs.out;nisysapi.out;
nisyscfgExpert.out=nisysapi.out;nitargetcfg.out;

[MODULE VERSIONS]
nisysapi.out=6.3
nisysrpc.out=6.3
nisyscfgExpert.out=6.3

		</MERGEINI>
		<MERGEINI TARGET="/ni-rt/system/nisysapi.ini">[NISYSCFGEXPERT]
Path=nisyscfgExpert.out
ExpertFactory=createSystemConfigBuiltInExpert

		</MERGEINI>
		<MERGEINI TARGET="/tmp/systemsettings/scs_imagemetadata.ini">[IMAGEDESCRIPTION]
Attr=219516928
ChangeRequiresReboot=False
Type=string
DefaultValue=

[IMAGEID]
Attr=219521024
ChangeRequiresReboot=False
Type=string
DefaultValue=

[IMAGETITLE]
Attr=219525120
ChangeRequiresReboot=False
Type=string
DefaultValue=

[IMAGEVERSION]
Attr=219529216
ChangeRequiresReboot=False
Type=string
DefaultValue=

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{79BBD504-B94E-4D53-A2D0-28983B2BD2C4}" VERSION="1.0" TYPE="HIDDEN">
	<TITLE>TargetConfiguration</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nitargetcfg.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="10.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nitargetcfg.out=6.3

[DEPENDENCIES]
nitargetcfg.out=ni_emb.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{E96F7666-FB58-455A-921A-66CB6714EFAF}" VERSION="4.4.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="3.4.0">
	<TITLE>NI-RPC</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nirpcs.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW Real-Time</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nirpcs.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{39864828-E46E-461E-8780-9C50ADA29115}" VERSION="10.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="4.0">
	<TITLE>VxWorks Base System</TITLE>
	<IMPLEMENTATION>
		<PROCESSOR VALUE="71C7"/>
		<PROCESSOR VALUE="729D"/>
		<PROCESSOR VALUE="7373"/>
		<PROCESSOR VALUE="74E8"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxWorks"/>
		<CODEBASE FILENAME="/ni-rt/system/target.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{CCB467C9-9FD0-4A0B-A7E5-C44DD4AB60F8}" VERSION="10.0">
				<TITLE>Base System Common</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="13.0.0" TYPE="VISIBLE" OLDESTCOMPATIBLEVERSION="8.5">
	<TITLE>LabVIEW Real-Time</TITLE>
	<ABSTRACT>LabVIEW Real-Time provides the core Real-Time Engine. You must have this component installed to be able to use any other software component.</ABSTRACT>
	<IMPLEMENTATION>
		<PROCESSOR VALUE="71C7"/>
		<PROCESSOR VALUE="729D"/>
		<PROCESSOR VALUE="7373"/>
		<PROCESSOR VALUE="74E8"/>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/lvanlys.out"/>
		<CODEBASE FILENAME="/ni-rt/system/lvblas.out"/>
		<CODEBASE FILENAME="/ni-rt/system/ts_rtc.out"/>
		<CODEBASE FILENAME="/ni-rt/system/ts_dio.out"/>
		<CODEBASE FILENAME="/ni-rt/system/ts_sntp.out"/>
		<FEATURE SELECT="TARGET">
			<SOFTPKG NAME="{4D706F57-FD55-445B-9FB9-840F9F38274B}" VERSION="13.0.0">
				<TITLE>LabVIEW PID Control Toolkit</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="TARGET">
			<SOFTPKG NAME="{C2C6AE73-0028-4750-B472-51C2E65DB3F9}" VERSION="13.0.0">
				<TITLE>Language Support for Japanese</TITLE>
			</SOFTPKG>
		</FEATURE>
		<FEATURE SELECT="TARGET">
			<SOFTPKG NAME="{2B13BEDE-7BA9-4A80-B2AD-334A942510B1}" VERSION="13.0.0">
				<TITLE>Language Support for Simplified Chinese</TITLE>
			</SOFTPKG>
		</FEATURE>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{CAEAE9D7-F7D3-4EA5-957E-6F09F764EE3B}" VERSION="13.0.0">
				<TITLE>LabVIEW RT Common</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
lvanlys.out=6.3
lvblas.out=6.3
ts_rtc.out=6.3
ts_dio.out=6.3
ts_sntp.out=6.3

[TIME SYNC]
source.rtc.enable=True

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CAEAE9D7-F7D3-4EA5-957E-6F09F764EE3B}" VERSION="13.0.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="8.5">
	<TITLE>LabVIEW RT VxWorks Common</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/lvalarms.out"/>
		<CODEBASE FILENAME="/ni-rt/system/lvuste.out"/>
		<CODEBASE FILENAME="/ni-rt/system/english/rtapp.rsc"/>
		<CODEBASE FILENAME="/ni-rt/system/lvrt.out"/>
		<CODEBASE FILENAME="/ni-rt/system/tsengine.out"/>
		<CODEBASE FILENAME="/ni-rt/system/tdtable.tdr"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/labview.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/analysis.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/measure.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/reports.err"/>
		<CODEBASE FILENAME="/ni-rt/system/errors/english/Services.err"/>
		<CODEBASE FILENAME="/ni-rt/system/www/beyond.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/docs.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/index.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/overview.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/services.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/webtool.htm"/>
		<CODEBASE FILENAME="/ni-rt/system/www/www.css"/>
		<CODEBASE FILENAME="/ni-rt/system/www/favicon.ico"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/home.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/homed.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/lvwebsrv.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/next.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/nextd.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/panel.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/panldiag.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/prev.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/prevd.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/top.gif"/>
		<CODEBASE FILENAME="/ni-rt/system/www/images/up.gif"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{32BC46CD-BF4A-4A57-AD90-E9A56ADD55CB}" VERSION="3.1">
				<TITLE>Deterministic Shared Variable Support for LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{B040DA53-2C8C-4FA6-A313-26036C633C98}" VERSION="1.4">
				<TITLE>NI CPU Info</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{53483209-4DF5-4E65-A426-9E022BBEEFDB}" VERSION="1.0">
				<TITLE>Real-Time Target Clock Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{AE3C452B-0C83-463C-9F5E-571E412F07F3}" VERSION="3.1">
				<TITLE>Deterministic FIFO Support for LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{39864828-E46E-461E-8780-9C50ADA29115}" VERSION="10.0">
				<TITLE>Base System</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{1C8F8B62-ED51-4F34-857F-436D29168FBD}" VERSION="1.0">
				<TITLE>Service Locator</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{F94E7627-BDFA-47F8-90A6-9C7C84670AC6}" VERSION="1.0">
				<TITLE>TDMS File Format Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{38A0C71C-CC2E-4F78-BDD3-9D2F034C8071}" VERSION="1.0.0.5">
				<TITLE>Language Support for LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[SUPPORTED LOCALES]
english=L1

[LVRT]
memoryChecking=False
LABVIEWRTDir=/c/ni-rt/system
PATH=/c/ni-rt/system/;/c/ni-rt/;
CDIntervalTicks=55
WebServer.Enabled=FALSE
RTTarget.VIPath=/c/ni-rt/startup
RTTarget.IPAccess=+*
RTEnetRcvMode=2
RTCPULoadMonitoringEnabled=True

[MODULE VERSIONS]
goopsup.out=6.3
rendezvs.out=6.3
semaphor.out=6.3
nbfifo.out=6.3
settime.out=6.3
lvalarms.out=6.3
lvuste.out=6.3
rtvarsup.out=6.3
lvrt.out=6.3
tsengine.out=6.3

[STARTUP]
MainExe=/c/ni-rt/system/lvrt.out
EarlyStartupLibraries=tsengine.out;

[DEPENDENCIES]
lvrt.out=ni_emb.out;libexpat.out;niCPULib.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{CCB467C9-9FD0-4A0B-A7E5-C44DD4AB60F8}" VERSION="10.0" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="10.0">
	<TITLE>Base System Common</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vx_exec.out"/>
		<CODEBASE FILENAME="/ni-rt/system/ni_emb.out"/>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{BA091EA4-AF3A-421C-855C-748682F5E9ED}" VERSION="1.1">
				<TITLE>Legacy Support for LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{A8CD9DBC-42EE-47A3-A77C-BA15B30A32DE}" VERSION="1.0">
				<TITLE>Real-Time Registry API Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY TYPE="STRICT">
			<SOFTPKG NAME="{8EACF39D-C4DD-4395-9D07-2EEB67E6D595}" VERSION="1.2.1">
				<TITLE>FTP Server</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{9BB7AF74-8292-4A50-9124-8C4BA7D60C88}" VERSION="1.0.0">
				<TITLE>NI Real-Time Device Manager</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{F05BB4F0-E3DF-4C7D-AD39-0B9938D49C9C}" VERSION="1.1.0">
				<TITLE>RTDM USB bus driver dependency</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{38A0C71C-CC2E-4F78-BDD3-9D2F034C8071}" VERSION="1.0.0.3">
				<TITLE>Language Support for LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{E8793E15-21E4-47BB-B1FD-F4975A1C00E5}" VERSION="2.2">
				<TITLE>VxWorks Floating Point Support</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<DEPENDENCY>
			<SOFTPKG NAME="{AAD95C0A-FD47-4CDD-821D-C4B4EF76DDDA}" VERSION="1.2.0">
				<TITLE>mDNSResponder</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
ni_emb.out=6.3

[DEPENDENCIES]
ni_emb.out=lvuste.out

[STARTUP]
DisplayStartupLibProgress=TRUE

[SYSTEMSETTINGS]
EnableVectorTableProtection=FALSE

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{E8793E15-21E4-47BB-B1FD-F4975A1C00E5}" VERSION="2.2" TYPE="HIDDEN" OLDESTCOMPATIBLEVERSION="2.2">
	<TITLE>VxWorks Floating-point support</TITLE>
	<ABSTRACT>Fix a vulnerability in VxWorks 6.3 where sporadic data corruption could happen</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/vxfpsup.out"/>
		<DEPENDENCY>
			<SOFTPKG NAME="{899452D2-C085-430B-B76D-7FDB33BB324A}" VERSION="8.5.0">
				<TITLE>LabVIEW RT</TITLE>
			</SOFTPKG>
		</DEPENDENCY>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
vxfpsup.out=6.3

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{1C8F8B62-ED51-4F34-857F-436D29168FBD}" VERSION="13.0.0" TYPE="HIDDEN">
	<TITLE>Service Locator</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nisvcloc.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nisvcloc.out=6.3

[STARTUP]
EarlyStartupLibraries=nisvcloc.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{32BC46CD-BF4A-4A57-AD90-E9A56ADD55CB}" VERSION="3.1" TYPE="HIDDEN">
	<TITLE>Deterministic Shared Variable Support for LabVIEW RT</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/rtvarsup.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{38A0C71C-CC2E-4F78-BDD3-9D2F034C8071}" VERSION="1.0.0.5" TYPE="VISIBLE" OLDESTCOMPATIBLEVERSION="1.0.0.3">
	<TITLE>Language Support for LabVIEW RT</TITLE>
	<ABSTRACT>This component adds support for non-ASCII languages on the real-time target. This component is required when using localized strings in your real-time application.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/libiconv.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{53483209-4DF5-4E65-A426-9E022BBEEFDB}" VERSION="1.0" TYPE="HIDDEN">
	<TITLE>Real-Time Target Clock Support</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/settime.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{8EACF39D-C4DD-4395-9D07-2EEB67E6D595}" VERSION="1.2.1" TYPE="HIDDEN">
	<TITLE>FTP Server</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/ftpserve.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
ftpserve.out=6.3

[STARTUP]
DisplayStartupLibProgress=TRUE

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{9BB7AF74-8292-4A50-9124-8C4BA7D60C88}" VERSION="1.2.0" TYPE="HIDDEN">
	<TITLE>NI-RTDM</TITLE>
	<ABSTRACT>NI Real-Time Device Manager</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nirtdm.out"/>
		<CODEBASE FILENAME="/ni-rt/system/nipci.out"/>
		<CODEBASE FILENAME="/ni-rt/system/dnf/nipci.dnf"/>
		<CODEBASE FILENAME="/ni-rt/system/rtdmRoot/pciRoot"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
nirtdm.out=6.3
nipci.out=6.3

[DEPENDENCIES]
nipci.out=nirtdm.out;

[STARTUP]
EarlyStartupLibraries=nirtdm.out;

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{A8CD9DBC-42EE-47A3-A77C-BA15B30A32DE}" VERSION="1.0" TYPE="HIDDEN">
	<TITLE>Real-Time Registry API Support</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/registry.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
registry.out=6.3

[STARTUP]
DisplayStartupLibProgress=TRUE

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{AAD95C0A-FD47-4CDD-821D-C4B4EF76DDDA}" VERSION="2.2.0" TYPE="HIDDEN">
	<TITLE>mDNSResponder</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nimdnsResponder.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[STARTUP]
EarlyStartupLibraries=nimdnsResponder.out;

[MDNSRESPONDER]
mDNSDisableAnnouncement=0
mDNSDisableDiscovery=0

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{AE3C452B-0C83-463C-9F5E-571E412F07F3}" VERSION="3.1" TYPE="HIDDEN">
	<TITLE>Deterministic FIFO Support for LabVIEW RT</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/nbfifo.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{B040DA53-2C8C-4FA6-A313-26036C633C98}" VERSION="1.4" TYPE="HIDDEN">
	<TITLE>NI CPU Info</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/niCPULib.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{BA091EA4-AF3A-421C-855C-748682F5E9ED}" VERSION="1.1" TYPE="HIDDEN" COMPATIBILITY="1.0">
	<TITLE>Legacy Support for LabVIEW RT</TITLE>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/libexpat.out"/>
		<MERGEINI TARGET="/ni-rt.ini">[MODULE VERSIONS]
libexpat.out=6.3

[STARTUP]
DisplayStartupLibProgress=TRUE

		</MERGEINI>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{F05BB4F0-E3DF-4C7D-AD39-0B9938D49C9C}" VERSION="1.1.0" TYPE="HIDDEN">
	<TITLE>RTDM USB bus driver dependency</TITLE>
	<ABSTRACT>Load USB Support for the NI Real-Time Device Manager only on targets with usb ports</ABSTRACT>
	<IMPLEMENTATION>
		<EXCEPTPROCESSOR VALUE="718F"/>
		<EXCEPTPROCESSOR VALUE="7319"/>
		<EXCEPTPROCESSOR VALUE="72B4"/>
		<EXCEPTPROCESSOR VALUE="73D2"/>
		<EXCEPTPROCESSOR VALUE="7459"/>
		<EXCEPTPROCESSOR VALUE="74D2"/>
		<EXCEPTPROCESSOR VALUE="7527"/>
		<EXCEPTPROCESSOR VALUE="758B"/>
		<EXCEPTPROCESSOR VALUE="75BE"/>
		<OS VALUE="VxWorks-PPC603"/>
	</IMPLEMENTATION>
</SOFTPKG>
<SOFTPKG NAME="{F94E7627-BDFA-47F8-90A6-9C7C84670AC6}" VERSION="2.5.0.0" TYPE="HIDDEN">
	<TITLE>TDMS File Format</TITLE>
	<ABSTRACT>This component adds support for the TDMS file format. TDMS is used to share measured data amongst NI software applications.</ABSTRACT>
	<IMPLEMENTATION>
		<OS VALUE="VxWorks-PPC603"/>
		<CODEBASE FILENAME="/ni-rt/system/tdms.out"/>
	</IMPLEMENTATION>
</SOFTPKG>
</INSTALLATION>
