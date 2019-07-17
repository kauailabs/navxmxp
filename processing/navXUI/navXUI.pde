
// navX-MXP/navX-Micro Open-Source Robotics Sensors
// User Interface/Demonstration software demonstration
// 12/23/2014 by Kauai Labs (scott@kauailabs.com)
// 
// Based upon the MPU-6050 Demonstration software developed
// 6/20/2012 by Jeff Rowberg <jeff@rowberg.net>
/* ============================================
This softwware is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg
Additions Copyright (c) 2014 Scott Libert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

import processing.serial.*;
import processing.opengl.*; // Import not needed if compiling w/Processing 3.0.
import toxi.geom.*;
import toxi.processing.*;
import com.kauailabs.navx.*;

import java.util.*;
import g4p_controls.*;
import java.io.FileWriter;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;

GButton btnResetYaw;
GButton btnBoardInfo;
GButton btnFileSave;
GButton btnAdvanced;
GDropList dplComPorts;
GDropList dplUpdateRates;

String[] currComPortNames = null;
int num_curr_com_port_names = 0;

GWindow windowBoardInfo = null;
GWindow windowAdvanced = null;

String g_boardtype = "Unknown";

FileWriter outputStream = null;
boolean save_to_file = false;
String datafile_name;

int tray_height_px = 60;

// NOTE: requires ToxicLibs to be installed in order to run properly.
// 1. Download from http://toxiclibs.org/downloads
// 2. Extract into [userdir]/Processing/libraries
//    (location may be different on Mac/Linux)
// 3. Run and bask in awesomeness

// NOTE:  Also requires the navX-core library to be installed in order to run properly.
// 1. Download from https://github.com/kauailabs/navxmxp/tree/master/java/navx/jar
// 2. Place the navX.jar library in the libraries subdirectory 

ToxiclibsSupport gfx;

byte[] protocol_buffer = new byte[4096];  // Buffer for received packets
byte[] protocol_buffer2 = new byte[4096];
int serialCount = 0;                 // current packet byte position
int aligned = 0;
int interval = 0;

float[] q = new float[4];
Quaternion quat = new Quaternion(1, 0, 0, 0);

float[] gravity = new float[3];
float[] euler = new float[3];
float[] ypr = new float[3];

float current_temp_c = 0.0;

float tilt_compensated_heading_degrees = 0.0;
float fused_heading_degrees = 0.0;
float altitude = 0.0;
float yaw_degrees = 0.0;
float pitch_degrees = 0.0;
float roll_degrees = 0.0;

float linear_acceleration_x = 0.0;
float linear_acceleration_y = 0.0;
float linear_acceleration_z = 0.0;

float world_linear_acceleration_x;
float world_linear_acceleration_y;
float world_linear_acceleration_z;

float curr_velocity_x = 0.0;
float curr_velocity_y = 0.0;
float curr_displacement_x = 0.0;
float curr_displacement_y = 0.0;

long curr_sensor_timestamp = 0;
            
static final int ACCEL_HISTORY_SIZE = 10;
float world_acceleration_history[] = new float[ACCEL_HISTORY_SIZE];
int world_acceleration_history_index = 0;
float world_acceleration_recent_avg = 0.0;

/* navx-Model AHRS Update-specific items */

boolean navx_device_moving = false;
boolean yaw_stable = true;
int last_update_ms = 0;

int updateRateHz = 50;
int gyroFSRDPS = 0;
int accelFSRG = 0;
float yaw_offset_degrees = 0.0;
boolean initial_calibration_in_process = false;
boolean yaw_reset_supported = false;
boolean omnimount_supported = false;
boolean vel_and_disp_supported = false;
boolean ahrspos_timestamp_supported = false;
boolean mag_disturbance = true;
boolean altitude_valid = false;
boolean compass_calibrated = false;
boolean fused_heading_valid = false;
boolean multiple_serial_ports = false;
boolean serial_port_open_error = false;
String attempted_open_serial_port_name = "";
MySerial port;                         // The serial port
String opened_port_name = "";
int board_yaw_axis = 2;
boolean board_yaw_axis_up = true;
int lf = 10;      // ASCII linefeed
// this table is used to store all command line parameters
// in the form: name=value
static Hashtable params=new Hashtable();

byte stream_type = AHRSProtocol.MSGID_AHRS_UPDATE;
 
PFont smallTextFont;
PFont mediumTextFont;
PFont largeTextFont;
 
String[] valid_update_rates = new String[] {"4","5","6","7","8","9","10","11",
                                            "12","13","14","15","16","18","20","22",
                                            "25","28","33","40","50","66","100","200"};
 
// here we overwrite PApplet's main entry point (for application mode)
// we're parsing all commandline arguments and copy only the relevant ones
 
static public void main(String args[]) {
  String[] newArgs=new String[args.length+1];
  /*********************************************************
  /* IMPORTANT: replace this with the name of your sketch  *
  /*********************************************************/
  newArgs[0]="navXUI";
 
  for(int i=0; i<args.length; i++) {
    newArgs[i+1]=args[i];
    if (args[i].indexOf("=")!=-1) {
      String[] pair=split(args[i], '=');
      params.put(pair[0],pair[1]);
    }
  }
  // pass on to PApplet entry point
  PApplet.main(newArgs);
}
 
int last_port_open_attempt_timestamp = 0; 
boolean port_open_in_progress = false;
 
boolean reopen_serial_port( String portName )
{
  boolean success = false;
    if ( port != null )
    {
      try {
        port.stop();
      } catch (Exception ex) {
        println("Exception calling port.stop().  " + ex.toString() );
      }
      port = null;
    }
	curr_sensor_timestamp = 0;
    // open the serial port
    try {
      port_open_in_progress = true;
      last_port_open_attempt_timestamp = millis();
      println("Opening serial port " + portName + "..." );
      cursor(WAIT);
      port = new MySerial(this, portName, 57600);
      print("Opened serial port ");
      println(portName);
      port.setDTR(true);
      port.clear();
      opened_port_name = portName;
      port.bufferUntil(lf);
      success = true;
    }
    catch(Exception ex) {
      print("Error opening serial port ");
      println(portName);
      opened_port_name = null;
      attempted_open_serial_port_name = portName;     
    }  
    port_open_in_progress = false;
    requestBoardInfo();
    cursor(HAND);
    return success;
}
 
void requestBoardInfo()
{
    int length = AHRSProtocol.encodeDataGetRequest( protocol_buffer, 
                                                    AHRSProtocol.AHRS_DATA_TYPE.BOARD_IDENTITY,
                                                    (byte)0);
    if ( port == null ) return;
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      port.write(stream_command);
      println("Sending " + new String(protocol_buffer));
    }
    else {
      println("Error encoding integration control.");
    }
}
 
void resetYaw()
{
    if ( port == null ) return;
    AHRSProtocol.IntegrationControl integration_control = new AHRSProtocol.IntegrationControl();
    integration_control.action = (byte) AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_YAW; 
    int length = AHRSProtocol.encodeIntegrationControlCmd( protocol_buffer, integration_control );
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      port.write(stream_command);
      println("Sending " + new String(protocol_buffer));
    }
    else {
      println("Error encoding integration control.");
    }
}

void enableRawUpdateMode(int rate) {

    if ( port == null ) return;
    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) IMUProtocol.STREAM_CMD_STREAM_TYPE_QUATERNION,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      port.write(stream_command);
      println("Sending " + new String(protocol_buffer));
    }
    else {
      println("Error encoding stream command.");
    }
}

void enableAHRSUpdateMode(int rate) {
    if ( port == null ) return;
    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) AHRSProtocol.MSGID_AHRS_UPDATE,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      try
      {
        port.write(stream_command);
      }
      catch(RuntimeException ex)
      {
        println("Exception:  " + ex.toString() );
      }
      println("Sending " + new String(protocol_buffer,0,length));
    }
    else {
      println("Error encoding stream command.");
    }
}

void enableAHRSPosUpdateMode(int rate) {
    if ( port == null ) return;
    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) AHRSProtocol.MSGID_AHRSPOS_UPDATE,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      try
      {
        port.write(stream_command);
      }
      catch(RuntimeException ex)
      {
        println("Exception:  " + ex.toString() );
      }
      println("Sending " + new String(protocol_buffer,0,length));
    }
    else {
      println("Error encoding stream command.");
    }
}

void enableAHRSPosTSUpdateMode(int rate) {
    if ( port == null ) return;
    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) AHRSProtocol.MSGID_AHRSPOS_TS_UPDATE,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      try
      {
        port.write(stream_command);
      }
      catch(RuntimeException ex)
      {
        println("Exception:  " + ex.toString() );
      }
      println("Sending " + new String(protocol_buffer,0,length));
    }
    else {
      println("Error encoding stream command.");
    }
}

void updateUpdateRateListSelection() {
  int index = getIndexInUpdateRateList(updateRateHz);
  dplUpdateRates.setSelected(index);
}

int getIndexInUpdateRateList( int update_rate ) {
  int highest_matching_index = 0;
  for ( int i = 0; i < valid_update_rates.length; i++ ) {
    double update_rate_hz_f = Double.parseDouble(valid_update_rates[i]);
    int update_rate_hz_i = (int)update_rate_hz_f;
    if ( update_rate == update_rate_hz_i ) {
      return i;
    }
    if ( update_rate > update_rate_hz_i ) {
      highest_matching_index = i+1;
    }
  }
  if ( highest_matching_index >= valid_update_rates.length ) {
    highest_matching_index = valid_update_rates.length - 1;
  }
  return highest_matching_index;
}

int last_port_list_update_timestamp = 0;

int getIndexInStringList( String[] array, String str )
{
  int index = -1;
  for ( int i = 0; i < array.length; i++ ) 
  {
    if ( array[i].equals(str) ) {
      index = i;
      break;
    }
  }
  return index;
}

void update_port_list()
{
  String existing_port_name = null;
  try {
    existing_port_name = dplComPorts.getSelectedText();
  } catch (Exception ex) {
  }
  last_port_list_update_timestamp = millis();
  for ( int i = 0; i < num_curr_com_port_names; i++ ) {
    dplComPorts.removeItem(i);
  }
  currComPortNames = MySerialList.list();
  //println(currComPortNames);
  dplComPorts.setVisible(currComPortNames.length != 0);
  if ( dplUpdateRates != null ) {
    dplUpdateRates.setVisible(currComPortNames.length != 0);
  }
  int index = 0;
  if ( ( dplComPorts != null ) && ( currComPortNames.length > 0 ) && ( currComPortNames != null ) ){
    if ( opened_port_name != null ) {
      index = getIndexInStringList(currComPortNames, opened_port_name);
      if ( index == -1 ) {
        index = 0;
      }
      /* Index should be that of opened port name as long as it's present */
    } else if ( attempted_open_serial_port_name != null ) {
      index = getIndexInStringList(currComPortNames, attempted_open_serial_port_name);
      if ( index == -1 ) {
        index = 0;
      }
      /* Index should be that of last attemped open port name as long as it's present */
    }
    num_curr_com_port_names = currComPortNames.length;
    dplComPorts.setItems(currComPortNames,index);
    if ( ( existing_port_name != null ) && ( num_curr_com_port_names > 0 ) ) {
      if ( !existing_port_name.equals(dplComPorts.getSelectedText()) ) {
        /* Simulate a selection event */
        println("Simulate selection event.");
        handleDropListEvents(dplComPorts, GEvent.SELECTED);        
      }
    }
  }
}

void setup() {
    // 500px square viewport using OpenGL rendering
    size(500, 560, OPENGL);
    gfx = new ToxiclibsSupport(this);
    println("Sketch Path:  " + sketchPath(""));
    println("Data Path:  " + dataPath(""));
    try {
    smallTextFont = loadFont("ArialMT-16.vlw");
    mediumTextFont = loadFont("ArialMT-26.vlw");
    largeTextFont = loadFont("ArialMT-32.vlw");
    } catch (Exception ex) {
      println("Unable to load fonts from data directory.  " + ex.toString() );
      try {
        smallTextFont = createFont("ArialMT", 16);
        mediumTextFont = createFont("ArialMT", 26);
        largeTextFont = createFont("ArialMT", 32);
      } catch (Exception ex2) {
      println("Unable to load fonts system.  " + ex2.toString() );
      }
    }
    print("Display Height:  ");
    println(height);
    print("Display Width:  ");
    println(width);

    // setup lights and antialiasing
    lights();
    smooth();
  
    print("Enumerating Serial Ports...");
    // display serial port list for debugging/clarity
    try {
      println(MySerialList.list());
    } catch (Exception e){
      println("No valid serial ports.  Exiting...");
      exit();
    }    
    print("Done Enumerating Serial Ports.");

    String theportname = "";
    
    if ( MySerialList.list().length < 1 ) {
      println("No valid serial ports.");
      return;    
    }
    else if ( MySerialList.list().length > 1 ) {
      
      if ( params.size() < 1 ) { 
        multiple_serial_ports = true;
        theportname = MySerialList.list()[0];
//        println("Use command line to specify serial port to use");
//        println("Syntax:  port=<PortNumber>");
//        return;
      }
      else {
        // select the port based upon the provided parameter
        String[] portnames = MySerialList.list();
        String portname = (String)params.get("port");
        print("Using command-line Specified port ");
        println(portname);
        for ( int i = 0; i < portnames.length; i++ ) {
          if ( portnames[i].equals(portname) ) {
            theportname = portnames[i];
            break;
          }
        }
      }
      
    }
    else {
      theportname = MySerialList.list()[0];
    }
    
     print("Opening serial port ");
     println(theportname);
    // get the first available port (use EITHER this OR the specific port code below)
    String portName = theportname;
    
    // get a specific serial port (use EITHER this OR the first-available code above)
    //String portName = "COM4";
    
    // open the serial port
    if ( reopen_serial_port(portName) ) {
    } else {
      serial_port_open_error = true;
      //return;
    }
    
    G4P.setGlobalColorScheme(5);
    btnResetYaw = new GButton(this, 20, ((height-tray_height_px)+15), 100, 30, "ResetYaw");
    btnBoardInfo = new GButton(this, (width/2)-110, ((height-tray_height_px)+15), 100, 30, "BoardInfo...");
    btnFileSave  = new GButton(this, (width/2)+10, ((height-tray_height_px)+15), 100, 30, "Save Data...");
    btnAdvanced = new GButton(this, width-120, ((height-tray_height_px)+15), 100, 30, "Experimental...");
    
    dplComPorts = new GDropList(this,400,40,80,70);
    dplComPorts.setItems(new String[] {"<None>"}, 0 );
    num_curr_com_port_names = 1;
    update_port_list();

    dplUpdateRates = new GDropList(this,20,120,80,70);
    dplUpdateRates.setItems(valid_update_rates, valid_update_rates.length);
    dplUpdateRates.setSelected(20);
    if ( port != null ) {
      int lf=10;
      port.bufferUntil(lf);
      
      // Give the navX-Model device a few seconds to start up.
      delay(1000);
      
      // Send command to navX-Model device requesting streaming data in 'raw' format
      //enableRawUpdateMode(100);    
      //enableAHRSUpdateMode(50);
      //enableAHRSUpdateMode(100);
      //enableAHRSUpdateMode(100);
      requestUpdateModeAndRate();
    }    
}

void handleDropListEvents(GDropList list, GEvent event)
{
  if ( list == dplComPorts ) {
    println("COM Port list event" + event.toString() );
    if ( event == GEvent.SELECTED  ) {
      println("Selected index:  " + list.getSelectedIndex() + " (" + list.getSelectedText() + ")" );
      if ( ( opened_port_name == null ) || ( list.getSelectedText() != opened_port_name ) ) {
        if ( reopen_serial_port( list.getSelectedText() ) ) {
        }
      }
    }
  }
  if ( list == dplUpdateRates ) {
    println("Update Rate list event" + event.toString() );
    if ( event == GEvent.SELECTED  ) {
      //println("Selected index:  " + list.getSelectedIndex() + " (" + list.getSelectedText() + ")" );
      double update_rate_hz_f = Double.parseDouble(list.getSelectedText());
      int update_rate_hz_i = (int)update_rate_hz_f;
      println("Selected update Rate:  " + update_rate_hz_i + " Hz");
      updateRateHz = update_rate_hz_i;
      requestUpdateModeAndRate();
    }    
  }
}

void draw() {
    
  
    boolean disconnected = false;
    // black background
    background(0);
    textAlign(LEFT);
    if ( smallTextFont != null ) textFont(smallTextFont); 
    
    int timeout_ms = 1000;
    
    if ( ( millis() - last_port_list_update_timestamp ) >= timeout_ms ) {
      update_port_list();
    }

    if ( port_open_in_progress ) {
        String str = "Opening port " + opened_port_name;
        float sw = textWidth(str);
        text(str,(width/2)-(sw/2),((height-tray_height_px)/2)-100);
        println("Outputting opening port message.");
    }    
    
    if ( !port_open_in_progress && ( ( millis() - last_update_ms ) >= timeout_ms ) ) {
      
      if ( mediumTextFont != null ) textFont(mediumTextFont); 
      textSize(26);
      fill(255,0,0);
      if ( opened_port_name != null ) {
        String str = "No response from device on " + opened_port_name;
        float sw = textWidth(str);
        text(str,(width/2)-(sw/2),((height-tray_height_px)/2)-100);
        if ( smallTextFont != null ) textFont(smallTextFont); 
        textSize(16);
        fill(0,255,0);
        text("Connected",width-100,30);
      } else {
        text("Disconnected",(width/2)-100,((height-tray_height_px)/2)-100);
        g_boardtype = "<Unknown>";
        gyroFSRDPS = 0;
        accelFSRG = 0;        
      }
      disconnected = true;
      if ( millis() - last_port_open_attempt_timestamp > timeout_ms ) {
        if ( opened_port_name != null ) {
          reopen_serial_port(opened_port_name);
        } else {
          reopen_serial_port(attempted_open_serial_port_name);
        }
        requestUpdateModeAndRate();
      }
    }
    else {
      if ( smallTextFont != null ) textFont(smallTextFont); 
      textSize(16);
      text("Connected",width-100,30);
    }    
    
    if ( port == null ) {
      if ( mediumTextFont != null ) textFont(mediumTextFont); 
      textSize(26);
      fill(255,0,0);
      if ( serial_port_open_error ) {
        text("Unable to open serial port " + attempted_open_serial_port_name,(width/2)-200,(height-tray_height_px)/2);
      }
      else {
        text("No Serial Ports available",(width/2)-150,(height-tray_height_px)/2);
      }
      return;
    }
    
    if ( disconnected || ( update_count == 0 ) ) return;
    
    if ( initial_calibration_in_process ) {
      text("Calibrating Gyro...",(width/2)-55,((height-tray_height_px)/2)-100);
    }
    if ( mag_disturbance ) {
      text("Mag Disturbance",(width/2)-55,((height-tray_height_px)/2)-120);
    }
    
    if ( largeTextFont != null ) textFont(largeTextFont); 
    textSize(32); 
    text(g_boardtype, 20, 30); 
    if ( smallTextFont != null ) textFont(smallTextFont);
    textSize(16);
    text("Robotics Navigation Sensor",20,50);
    String gyrofsr = "Gyro Range:  +/- " + gyroFSRDPS + " Deg/sec";
    text(gyrofsr,20,70);
    String accelfsr = "Accel Range:  +/- " + accelFSRG + " G";
    text(accelfsr,20,90);
    String updaterate = "Update Rate:  " + updateRateHz + " Hz";
    text(updaterate,20,110);

    fill(255,255,255);
    if ( curr_sensor_timestamp != 0 ) {
      textAlign(RIGHT);
      String timestamp = "Time:  " + curr_sensor_timestamp + " ms";
      text(timestamp,width-20,110);
    }
    
    textSize(16);

    textAlign(LEFT);
    text("Accel X:",20, (height-tray_height_px)-60);
    textAlign(RIGHT);
    text(nfp(world_linear_acceleration_x,1,2),135,(height-tray_height_px)-60);
    textAlign(LEFT);
    text("Accel Y:",20, (height-tray_height_px)-40);
    textAlign(RIGHT);
    text(nfp(world_linear_acceleration_y,1,2),135,(height-tray_height_px)-40);
    textAlign(LEFT);
    text("Temp:",20, (height-tray_height_px)-20);
    textAlign(RIGHT);
    text(nfp(current_temp_c,1,2),135,(height-tray_height_px)-20);
    if ( initial_calibration_in_process ) {
      fill(128,128,128);
    }
    textAlign(LEFT);
    text("Yaw (Z):",width-150,(height-tray_height_px)-80);
    textAlign(RIGHT);
    text(nfp(yaw_degrees,3,2),width-15, (height-tray_height_px)-80);
    fill(255,255,255);
    textAlign(LEFT);
    text("Roll (Y):",width-150,(height-tray_height_px)-100);
    textAlign(RIGHT);
    text(nfp(roll_degrees,3,2),width-15, (height-tray_height_px)-100);
    textAlign(LEFT);
    text("Pitch (X):",width-150,(height-tray_height_px)-120);
    textAlign(RIGHT);
    text(nfp(pitch_degrees,3,2),width-15, (height-tray_height_px)-120);
    textAlign(LEFT);
    if ( !compass_calibrated ) {
      fill(128,128,128);
    }
    text("Compass:",width-150,(height-tray_height_px)-60);
    textAlign(RIGHT);
    text(nfp(tilt_compensated_heading_degrees,3,2),width-15, (height-tray_height_px)-60);
    if ( !fused_heading_valid ) {
      fill(128,128,128);
    }
    textAlign(LEFT);
    text("Heading:",width-150,(height-tray_height_px)-40);
    textAlign(RIGHT);
    text(nfp(fused_heading_degrees,3,2),width-15, (height-tray_height_px)-40);
    textAlign(LEFT);
    if ( !altitude_valid ) {
      fill(255,0,0);
    }
    text("Altitude:",width-150,(height-tray_height_px)-20);
    textAlign(RIGHT);
    text(nfp(altitude,3,2),width-15, (height-tray_height_px)-20);
    
    fill(0,255,0);
    textAlign(LEFT);    
    String motion_state = "";
    if ( navx_device_moving || ( world_acceleration_recent_avg >= 0.01 ) ) {
      motion_state = "Moving";
    }
    text(motion_state,(width/2)-30,(height-tray_height_px)-20);
    String rotation_state = "";
    if ( !yaw_stable ) {
      rotation_state = "Rotating";
    }
    text(rotation_state,(width/2)-35,(height-tray_height_px)-40);
    
    if ( save_to_file ) {
      textSize(14);
      fill(255,255,255);
      String save_string = "<Saving to " + datafile_name + ">";
      textAlign(LEFT);
      text(save_string,20, height-tray_height_px);
    }
    
    // translate everything to the middle of the viewport
    pushMatrix();
    translate(width / 2, (height-tray_height_px) / 2);

    // 3-step rotation from yaw/pitch/roll angles (gimbal lock!)
    // ...and other weirdness I haven't figured out yet
    //rotateY(-ypr[0]);
    //rotateZ(-ypr[1]);
    //rotateX(-ypr[2]);

    // toxiclibs direct angle/axis rotation from quaternion (NO gimbal lock!)
    // (axis order [1, 3, 2] and inversion [-1, +1, +1] is a consequence of
    // different coordinate system orientation assumptions between Processing
    // and InvenSense DMP)
    float[] axis = quat.toAxisAngle();
    rotate(axis[0], -axis[1], axis[3], axis[2]);

    // draw main body in red
    fill(255, 0, 0, 200);
    box(10, 10, 200);
    
    // draw front-facing tip in blue
    fill(0, 0, 255, 200);
    pushMatrix();
    translate(0, 0, -120);
    rotateX(PI/2);
    drawCylinder(0, 20, 20, 8);
    popMatrix();
    
    // draw wings and tail fin in green
    fill(0, 255, 0, 200);
    beginShape(TRIANGLES);
    vertex(-100,  2, 30); vertex(0,  2, -80); vertex(100,  2, 30);  // wing top layer
    vertex(-100, -2, 30); vertex(0, -2, -80); vertex(100, -2, 30);  // wing bottom layer
    vertex(-2, 0, 98); vertex(-2, -30, 98); vertex(-2, 0, 70);  // tail left layer
    vertex( 2, 0, 98); vertex( 2, -30, 98); vertex( 2, 0, 70);  // tail right layer
    endShape();
    beginShape(QUADS);
    vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
    vertex( 100, 2, 30); vertex( 100, -2, 30); vertex(  0, -2, -80); vertex(  0, 2, -80);
    vertex(-100, 2, 30); vertex(-100, -2, 30); vertex(100, -2,  30); vertex(100, 2,  30);
    vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2, -30, 98); vertex(-2, -30, 98);
    vertex(-2,   0, 98); vertex(2,   0, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
    vertex(-2, -30, 98); vertex(2, -30, 98); vertex(2,   0, 70); vertex(-2,   0, 70);
    endShape();
    
    popMatrix();
}

public void requestUpdateModeAndRate() 
{
  if ( stream_type == AHRSProtocol.MSGID_AHRS_UPDATE ) {
    print("Enabling AHRS Update Mode.");
    enableAHRSUpdateMode(updateRateHz);
  } else if ( stream_type == AHRSProtocol.MSGID_AHRSPOS_UPDATE ) {
    print("Enabling AHRS Pos Update Mode.");
    enableAHRSPosUpdateMode(updateRateHz);
  } else if ( stream_type == AHRSProtocol.MSGID_AHRSPOS_TS_UPDATE ) {
    print("Enabling AHRS Pos Timestamp Update Mode.");    
    enableAHRSPosTSUpdateMode(updateRateHz);
  }  
}

public void createBoardInfoWindow()
{
    if ( windowBoardInfo != null ) return;
    windowBoardInfo = new GWindow(this, "Board Info", 70+0*220, 160+0*50, 300, 200, false, JAVA2D);
    //window[i].addData(new MyWinData());
    windowBoardInfo.addDrawHandler(this, "boardInfoWindowDraw");  
    windowBoardInfo.addOnCloseHandler(this,"boardInfoWindowClose");
    windowBoardInfo.setActionOnClose(GWindow.CLOSE_WINDOW);
}

public void createAdvancedWindow()
{
    if ( windowAdvanced != null ) return;
    windowAdvanced = new GWindow(this, "Advanced", 70+0*220, 160+0*50, 300, 200, false, JAVA2D);
    //window[i].addData(new MyWinData());
    windowAdvanced.addDrawHandler(this, "advancedWindowDraw");  
    windowAdvanced.addOnCloseHandler(this,"advancedWindowClose");
    windowAdvanced.setActionOnClose(GWindow.CLOSE_WINDOW);
}

void updateBoardType( AHRSProtocol.BoardID board_id ) {
    String board_type = "<Unknown>";
    if ( board_id.type == 50 ) {
      board_type = "navX-MXP";
    }
    if ( board_id.hw_rev == 33 ) {
      board_type = "navX-MXP";
    } else if ( board_id.hw_rev == 40 ) {
      board_type = "navX-Micro";    
    } else if (( board_id.hw_rev >= 60 ) && ( board_id.hw_rev <= 69 )) {
      board_type = "VMX-pi";    
    }
    g_boardtype = board_type;
}

void boardInfoWindowDraw(GWinApplet appc, GWinData data) {
    appc.background(0);
    appc.textAlign(LEFT);
    if ( smallTextFont != null ) appc.textFont(smallTextFont); 
    appc.textSize(16); 
    appc.text("BoardType:  " + g_boardtype,20,20);
    appc.text("Board Version:  " + Byte.toString(board_id.hw_rev),20,40);
    appc.text("Firmware Version:  " + Byte.toString(board_id.fw_ver_major)+ "." + Byte.toString(board_id.fw_ver_minor) + "." + Short.toString(board_id.fw_revision), 20,60);
    String board_yaw_axis_string = "Board Yaw Axis:  ";
    if ( board_yaw_axis == 0 ) {
      board_yaw_axis_string += "X";
    } else if ( board_yaw_axis == 1 ) {
      board_yaw_axis_string += "Y";
    } else {
      board_yaw_axis_string += "Z";
    }
    board_yaw_axis_string += (board_yaw_axis_up ? " (Up)" : " (Down)");
    appc.text(board_yaw_axis_string, 20, 80);
} 

void boardInfoWindowClose(GWindow window) {
  windowBoardInfo = null;
  println("In boardInfoWindowClose");
}

void advancedWindowDraw(GWinApplet appc, GWinData data) {
    appc.background(0);
    appc.textAlign(LEFT);
    if ( smallTextFont != null ) appc.textFont(smallTextFont); 
    appc.textSize(16); 
    appc.text("Velocity X    :  ", 20, 20 );
    appc.text("Velocity Y    :  ", 20, 40 );
    appc.text("Displacement X:  ", 20, 60 );
    appc.text("Displacement Y:  ", 20, 80 );
    appc.text(String.valueOf(curr_velocity_x), 160, 20 );
    appc.text(String.valueOf(curr_velocity_y), 160, 40 );
    appc.text(String.valueOf(curr_displacement_x), 160, 60 );
    appc.text(String.valueOf(curr_displacement_y), 160, 80 );
} 

void advancedWindowClose(GWindow window ) {
  windowAdvanced = null;
  println("In advancedWindowClose");
}

public void handleButtonEvents(GButton button, GEvent event)
{
  if ( button == btnResetYaw )
  {
    if ( event.getType() == GEvent.CLICKED.toString() )
    {
      resetYaw();      
    }
  }
  if ( button == btnBoardInfo )
  {
    if ( event.getType() == GEvent.CLICKED.toString() )
    {
      if ( windowBoardInfo == null ) {
        createBoardInfoWindow();      
      }
      requestBoardInfo();      
    }
  } 
  if ( button == btnAdvanced )
  {
    if ( event.getType() == GEvent.CLICKED.toString() )
    {
      if ( windowAdvanced == null ) {
        createAdvancedWindow();      
      }
    }
  } 
  if ( button == btnFileSave ) 
  {
    save_to_file = !save_to_file;
    if ( save_to_file ) {
      println("Opening data file.");
      try {
        String user_dir = System.getProperty("user.home");
        Date date = new Date();
        SimpleDateFormat sdf = new SimpleDateFormat("MM_dd_yyyy_h_mm_ss");
        String formattedDate = sdf.format(date);
        datafile_name = user_dir + File.separator + "navXData_" + formattedDate + ".csv";
        println("Datafile name:  " + datafile_name);
        outputStream = new FileWriter(datafile_name);
        String header = "Timestamp,QuatW,QuatX,QuatY,QuatZ,Yaw,Pitch,Roll,LinearAccelX,LinearAccelY,VelocityX,VelocityY,DisplacementX,DisplacementY\r\n";
        outputStream.write(header);
        btnFileSave.setText("Stop Saving Data", GAlign.CENTER, GAlign.MIDDLE );
        btnFileSave.setLocalColorScheme(3);
      } catch (IOException ex) {
        println("IOException opening file.");
        ex.printStackTrace();
      }
    } else {
      if (outputStream != null) {
          println("Closing data file.");
          btnFileSave.setText("Save Data...", GAlign.CENTER, GAlign.MIDDLE );
          btnFileSave.setLocalColorScheme(5);
        try {
          outputStream.close();
          outputStream = null;
        } catch (IOException ex) {
          println("Exception closing data file.");
          ex.printStackTrace();
        }
      }
    }
  }
}

// The serialEvent() method will be invoked whenever one or more
// serial characters are received at the moment when a 
// line feed character (which terminates a navX-Model device message)
// is received.

IMUProtocol.QuaternionUpdate raw_update = new IMUProtocol.QuaternionUpdate();
IMUProtocol.YPRUpdate ypr_update = new IMUProtocol.YPRUpdate();
IMUProtocol.StreamResponse stream_response = new IMUProtocol.StreamResponse();
AHRSProtocol.AHRSUpdate ahrs_update = new AHRSProtocol.AHRSUpdate();
AHRSProtocol.AHRSPosTSUpdate ahrs_pos_update = new AHRSProtocol.AHRSPosTSUpdate();
AHRSProtocol.BoardID board_id = new AHRSProtocol.BoardID();

long update_count = 0;
long data_received_count = 0;
long serial_event_count = 0;

int ahrs_update_msg_len = 56;
int last_msg_start_received_timestamp = 0;

int last_partial_packet_expected_length = 0;
int last_partial_packet_bytes_received = 0;

void serialEvent(MySerial port) {

  serial_event_count++;
  while ( port.available() > 0 ) {
    try {
  
      int initial_bytes_received = 0;
      
      if ( last_partial_packet_bytes_received != 0 ) {
        int now = millis();
        if ( now < last_msg_start_received_timestamp ) {
          /* Rollover */
          last_msg_start_received_timestamp = now;
        }
        /* If timeout occurs, clear the partial packet */
        if ( now - last_msg_start_received_timestamp > 50 ) {
          last_partial_packet_bytes_received = 0;
          println("Discarded Partial Packet after timeout.  Now = " + String.valueOf(now) );
        }
      }
      
      initial_bytes_received = 0;
      try {
        if ( last_partial_packet_bytes_received == 0 ) {        
          initial_bytes_received = port.readBytesUntil((int)'!',protocol_buffer);
        } else {
          for ( int i = 0; i < last_partial_packet_bytes_received; i++ ) {
            protocol_buffer[i] = protocol_buffer2[i];
          }
          boolean end_of_packet = false;
          initial_bytes_received = 0;
          while ( ( port.available() > 0 ) && !end_of_packet ) {
            protocol_buffer2[initial_bytes_received] = (byte)port.read();
            if ( protocol_buffer2[initial_bytes_received] == (byte)lf ) {
              end_of_packet = true;
            }
            initial_bytes_received++;
          }
          //initial_bytes_received = port.readBytesUntil((int)'n',protocol_buffer2);
          if ( end_of_packet ) {
            println("Got end of partial packet.");            
          } else {
            println("Got more bytes.  More Len = " + String.valueOf(initial_bytes_received) + " Remaining available:  " + String.valueOf(port.available()) );
          }
          if ( initial_bytes_received == 0 ) return;
          int x = 0;
          for ( int i = last_partial_packet_bytes_received; i < last_partial_packet_bytes_received + initial_bytes_received; i++, x++ ) {
            protocol_buffer[i] = protocol_buffer2[x];
          }
        }
      } catch( Exception ex) {
        println("Exception calling port readBytesUntil.  " + ex.toString() );
      }
      if ( initial_bytes_received > 0 ) {
        int msg_len = 0;
        try {
          if ( last_partial_packet_bytes_received == 0 ) {
            msg_len = port.readBytesUntil((int) '\n',protocol_buffer);
          } else {
            msg_len = last_partial_packet_bytes_received + initial_bytes_received;
            last_partial_packet_bytes_received = 0;
          }
        } catch( Exception ex) {
          println("Exception calling port readBytesUntil.  " + ex.toString() );
        }
        if (msg_len > 0 ) {
          if ( ( protocol_buffer[0] == AHRSProtocol.BINARY_PACKET_INDICATOR_CHAR ) && 
               ( msg_len > 1 ) &&
               ( protocol_buffer[msg_len-1] != '\r' ) ) {
              int indicated_message_length = protocol_buffer[1];
              if ( msg_len != indicated_message_length + 1 ) {
                last_msg_start_received_timestamp = millis();             
                println( "Partial:  Len Received = " + String.valueOf(msg_len) + ", Expected Len = " + String.valueOf(indicated_message_length) + ", Now = " + String.valueOf(last_msg_start_received_timestamp));
                last_partial_packet_expected_length = indicated_message_length;
                last_partial_packet_bytes_received = msg_len;
                for ( int i = 0; i < msg_len; i++ ) {
                  protocol_buffer2[i] = protocol_buffer[i];
                }
                continue;
              } else {
                //println("Got complete binary packet.  Len = " + String.valueOf(msg_len));
              }
          } else {
            println("Received packet, but not a valid binary packet.");
          }
        }
        //println(msg_len);
        byte[] full_message = new byte[msg_len+1];
        full_message[0] = '!';
        arrayCopy(protocol_buffer,0,full_message,1,msg_len);
        
        //print(new String(full_message));
        
        int decode_length = IMUProtocol.decodeQuaternionUpdate(full_message, 0, full_message.length, raw_update);
        if (decode_length != 0) {
          
          update_count++;
          last_update_ms = millis();
          
          q[0] = ((float)raw_update.q1) / 16384.0f;
          q[1] = ((float)raw_update.q2) / 16384.0f;
          q[2] = ((float)raw_update.q3) / 16384.0f;
          q[3] = ((float)raw_update.q4) / 16384.0f;
          for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i]; // Range-check quaternion values
          
          // set our toxilibs quaternion to new data
          quat.set(q[0], q[1], q[2], q[3]);
          
          current_temp_c = raw_update.temp_c;
          
          // below calculations are necessary for calculation of yaw/pitch/roll, 
          // and tilt-compensated compass heading
          
          // calculate gravity vector
          gravity[0] = 2 * (q[1]*q[3] - q[0]*q[2]);
          gravity[1] = 2 * (q[0]*q[1] + q[2]*q[3]);
          gravity[2] = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];
  
          // calculate Euler angles
          euler[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
          euler[1] = -asin(2*q[1]*q[3] + 2*q[0]*q[2]);
          euler[2] = atan2(2*q[2]*q[3] - 2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3] - 1);
  
          // calculate yaw/pitch/roll angles
          ypr[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
          ypr[1] = atan(gravity[0] / sqrt(gravity[1]*gravity[1] + gravity[2]*gravity[2]));
          ypr[2] = atan(gravity[1] / sqrt(gravity[0]*gravity[0] + gravity[2]*gravity[2]));
           
          yaw_degrees = ypr[0] * (180.0/3.1415926); 
          pitch_degrees = ypr[1] * (180.0/3.1415926); 
          roll_degrees = ypr[2] * (180.0/3.1415926); 
           
          // Subtract offset, and handle potential 360 degree wrap-around
          yaw_degrees -= yaw_offset_degrees;
          if ( yaw_degrees < -180 ) yaw_degrees += 360;
          if ( yaw_degrees > 180 ) yaw_degrees -= 360;
           
          // calculate linear acceleration by 
          // removing the gravity component (+1g = +4096 in standard DMP FIFO packet)
           
          linear_acceleration_x = (((float)raw_update.accel_x) / (32768.0/accelFSRG)) - gravity[0];
          linear_acceleration_y = (((float)raw_update.accel_y) / (32768.0/accelFSRG)) - gravity[1];
          linear_acceleration_z = (((float)raw_update.accel_z) / (32768.0/accelFSRG)) - gravity[2]; 
          
          // Calculate world-frame acceleration
          
          float q2[] = new float[4];
          q2[0] = 0;
          q2[1] = linear_acceleration_x;
          q2[2] = linear_acceleration_y;
          q2[3] = linear_acceleration_z;
          
          // Rotate linear acceleration so that it's relative to the world reference frame
          
          // http://www.cprogramming.com/tutorial/3d/quaternions.html
          // http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
          // http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
          // ^ or: http://webcache.googleusercontent.com/search?q=cache:xgJAp3bDNhQJ:content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation&hl=en&gl=us&strip=1
      
          // P_out = q * P_in * conj(q)
          // - P_out is the output vector
          // - q is the orientation quaternion
          // - P_in is the input vector (a*aReal)
          // - conj(q) is the conjugate of the orientation quaternion (q=[w,x,y,z], q*=[w,-x,-y,-z])
  
          
          float[] q_product = new float[4];
          
          // calculate quaternion product
          // Quaternion multiplication is defined by:
          //     (Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
          //     (Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
          //     (Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
          //     (Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2
          
          q_product[0] = q[0]*q2[0] - q[1]*q2[1] - q[2]*q2[2] - q[3]*q2[3];  // new w
          q_product[1] = q[0]*q2[1] + q[1]*q2[0] + q[2]*q2[3] - q[3]*q2[2];  // new x
          q_product[2] = q[0]*q2[2] - q[1]*q2[3] + q[2]*q2[0] + q[3]*q2[1];  // new y 
          q_product[3] = q[0]*q2[3] + q[1]*q2[2] - q[2]*q2[1] + q[3]*q2[0];  // new z
  
          float[] q_conjugate = new float[4];
          
          q_conjugate[0] = q[0];            
          q_conjugate[1] = -q[1];            
          q_conjugate[2] = -q[2];            
          q_conjugate[3] = -q[3];            
  
          float[] q_final = new float[4];
          
          q_final[0] = q_product[0]*q_conjugate[0] - q_product[1]*q_conjugate[1] - q_product[2]*q_conjugate[2] - q_product[3]*q_conjugate[3];  // new w
          q_final[1] = q_product[0]*q_conjugate[1] + q_product[1]*q_conjugate[0] + q_product[2]*q_conjugate[3] - q_product[3]*q_conjugate[2];  // new x
          q_final[2] = q_product[0]*q_conjugate[2] - q_product[1]*q_conjugate[3] + q_product[2]*q_conjugate[0] + q_product[3]*q_conjugate[1];  // new y 
          q_final[3] = q_product[0]*q_conjugate[3] + q_product[1]*q_conjugate[2] - q_product[2]*q_conjugate[1] + q_product[3]*q_conjugate[0];  // new z
  
          world_linear_acceleration_x = q_final[1];
          world_linear_acceleration_y = q_final[2];
          world_linear_acceleration_z = q_final[3];
           
          world_acceleration_history[world_acceleration_history_index] = abs(world_linear_acceleration_x) + abs(world_linear_acceleration_y);
          world_acceleration_history_index = (world_acceleration_history_index + 1) % ACCEL_HISTORY_SIZE;
          float world_acceleration_history_sum = 0;
          for ( int i = 0; i < ACCEL_HISTORY_SIZE; i++ ) {
            world_acceleration_history_sum += world_acceleration_history[i];
          }  
          world_acceleration_recent_avg = world_acceleration_history_sum / ACCEL_HISTORY_SIZE;
           
          // Calculate tilt-compensated compass heading
          
          float inverted_pitch = -ypr[1];
          float roll = ypr[2];
          
          float cos_roll = cos(roll);
          float sin_roll = sin(roll);
          float cos_pitch = cos(inverted_pitch);
          float sin_pitch = sin(inverted_pitch);
          
          float MAG_X = raw_update.mag_x * cos_pitch + raw_update.mag_z * sin_pitch;
          float MAG_Y = raw_update.mag_x * sin_roll * sin_pitch + raw_update.mag_y * cos_roll - raw_update.mag_z * sin_roll * cos_pitch;
          float tilt_compensated_heading_radians = atan2(MAG_Y,MAG_X);
          tilt_compensated_heading_degrees = tilt_compensated_heading_radians * (180.0 / 3.1415926);
          
          // Adjust compass for board orientation,
          // and modify range from -180-180 to
          // 0-360 degrees
        
          tilt_compensated_heading_degrees -= 90.0;
          if ( tilt_compensated_heading_degrees < 0 ) {
            tilt_compensated_heading_degrees += 360; 
          }
        }
        else {
          // If a YPR Update was received, send a stream command to switch to Raw Update Mode.
          // This case can happen if the nav6 IMU is reset after this application has completed
          // initialization.
          decode_length = IMUProtocol.decodeYPRUpdate(full_message, 0, full_message.length, ypr_update);
          if ( decode_length > 0 ) {
            requestUpdateModeAndRate();
          }
          else {
            decode_length = IMUProtocol.decodeStreamResponse(full_message,0, full_message.length,stream_response);
            if ( decode_length > 0 ) {
              updateRateHz = stream_response.update_rate_hz;
              updateUpdateRateListSelection();
              gyroFSRDPS = stream_response.gyro_fsr_dps;
              accelFSRG = stream_response.accel_fsr_g;
              yaw_offset_degrees = stream_response.yaw_offset_degrees;
              short calibration_state = (short)(stream_response.flags & IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
              initial_calibration_in_process = (calibration_state != IMUProtocol.NAV6_CALIBRATION_STATE_COMPLETE);
              yaw_reset_supported = ((stream_response.flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_YAW_RESET) != 0);
              if ( btnResetYaw != null ) {
                btnResetYaw.setVisible(yaw_reset_supported);
              }
              vel_and_disp_supported = ((stream_response.flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_VEL_AND_DISP) != 0);
              ahrspos_timestamp_supported = ((stream_response.flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_AHRSPOS_TS) != 0);
              println("Stream Response:  Type:  " + stream_response.stream_type + ", Vel/Disp:  " + vel_and_disp_supported + ", Timestamp:  " + ahrspos_timestamp_supported);
              if ( btnAdvanced != null ) {
                btnAdvanced.setVisible(vel_and_disp_supported);
                if ( ahrspos_timestamp_supported ) {
                  if ( stream_response.stream_type != AHRSProtocol.MSGID_AHRSPOS_TS_UPDATE ) {
                    stream_type = AHRSProtocol.MSGID_AHRSPOS_TS_UPDATE;
                    requestUpdateModeAndRate();
                  }
                }
                else {
                  if ( vel_and_disp_supported ) {
                    if ( stream_response.stream_type != AHRSProtocol.MSGID_AHRSPOS_UPDATE ) {
                      stream_type = AHRSProtocol.MSGID_AHRSPOS_UPDATE;
                      requestUpdateModeAndRate();
                    }
                  }
                }
              }
              omnimount_supported = ((stream_response.flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_OMNIMOUNT) != 0);
              println("Flags:  " + Integer.toHexString(stream_response.flags & 0xffff));
              short omnimount_config = (short)((stream_response.flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_OMNIMOUNT_CONFIG_MASK) >> 3);
              if ( !omnimount_supported || ( omnimount_config == 0 ) ) {
                board_yaw_axis = 2; /* Z Axis */
                board_yaw_axis_up = true;
              } else {
                println("Omnimount config:  " + String.valueOf(omnimount_config));
                board_yaw_axis_up = (((omnimount_config & 0x0001) != 0) ? true : false);
                board_yaw_axis = omnimount_config >> 1;
              }
              
            } else {
              decode_length = AHRSProtocol.decodeAHRSUpdate(full_message, 0, full_message.length, ahrs_update);
              if (decode_length != 0) {
                update_count++;
                println("AHRS Update Count:  " + String.valueOf(update_count));
                last_update_ms = millis();
          
                current_temp_c = ahrs_update.mpu_temp;
                
                tilt_compensated_heading_degrees = ahrs_update.compass_heading;
                
                yaw_degrees = ahrs_update.yaw;
                pitch_degrees = ahrs_update.pitch;
                roll_degrees = ahrs_update.roll;
                fused_heading_degrees = ahrs_update.fused_heading;
                
                world_linear_acceleration_x = ahrs_update.linear_accel_x;
                world_linear_acceleration_y = ahrs_update.linear_accel_y;
                world_linear_acceleration_z = ahrs_update.linear_accel_z;
                
                q[0] = ((float)ahrs_update.quat_w); // / 16384.0f; // This now occurs in decodeAHRSUpdate()
                q[1] = ((float)ahrs_update.quat_x); // / 16384.0f; // This now occurs in decodeAHRSUpdate()
                q[2] = ((float)ahrs_update.quat_y); // / 16384.0f; // This now occurs in decodeAHRSUpdate()
                q[3] = ((float)ahrs_update.quat_z); // / 16384.0f; // This now occurs in decodeAHRSUpdate()
                for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i]; // Range-check quaternion values
                
                // set our toxilibs quaternion to new data
                quat.set(q[0], q[1], q[2], q[3]);
                navx_device_moving = (( ahrs_update.sensor_status & 0x01 ) != 0);
                initial_calibration_in_process = (( ahrs_update.cal_status & 0x02)==0);
                compass_calibrated = (( ahrs_update.cal_status & 0x04)!=0);
                fused_heading_valid = (( ahrs_update.sensor_status & 0x20)!=0);
                mag_disturbance = (( ahrs_update.sensor_status & 0x04)!=0);
                altitude_valid = (( ahrs_update.sensor_status & 0x08) != 0);
                yaw_stable = (( ahrs_update.sensor_status & 0x02)!= 0);
                //println(ahrs_update.sensor_status);
                float mag_field_norm = sqrt( 
                  ((float)ahrs_update.raw_mag_x * (float)ahrs_update.raw_mag_x) +
                  ((float)ahrs_update.raw_mag_y * (float)ahrs_update.raw_mag_y) +
                  ((float)ahrs_update.raw_mag_z * (float)ahrs_update.raw_mag_z) );
                //println(ahrs_update.raw_mag_x + ", " + ahrs_update.raw_mag_y + ", " + ahrs_update.raw_mag_z + ", ", mag_field_norm);
              } else {
                decode_length = AHRSProtocol.decodeAHRSPosTSUpdate(full_message, 0, full_message.length, ahrs_pos_update);
                if ( decode_length == 0 ) {
                  decode_length = AHRSProtocol.decodeAHRSPosUpdate(full_message, 0, full_message.length, ahrs_pos_update);
                } else {
                  curr_sensor_timestamp = ahrs_pos_update.timestamp;
                }
                if (decode_length != 0) {
                  update_count++;
                  //println("AHRSPos Update Count:  " + String.valueOf(update_count) + ", time:  " + String.valueOf(curr_sensor_timestamp));
                  last_update_ms = millis();
            
                  current_temp_c = ahrs_pos_update.mpu_temp;
                  
                  tilt_compensated_heading_degrees = ahrs_pos_update.compass_heading;
                  
                  yaw_degrees = ahrs_pos_update.yaw;
                  pitch_degrees = ahrs_pos_update.pitch;
                  roll_degrees = ahrs_pos_update.roll;
                  fused_heading_degrees = ahrs_pos_update.fused_heading;
                  
                  world_linear_acceleration_x = ahrs_pos_update.linear_accel_x;
                  world_linear_acceleration_y = ahrs_pos_update.linear_accel_y;
                  world_linear_acceleration_z = ahrs_pos_update.linear_accel_z;
                  
                  q[0] = ((float)ahrs_pos_update.quat_w); // / 16384.0f; // This now occurs in decodeAHRSPosUpdate()
                  q[1] = ((float)ahrs_pos_update.quat_x); // / 16384.0f; // This now occurs in decodeAHRSPosUpdate()
                  q[2] = ((float)ahrs_pos_update.quat_y); // / 16384.0f; // This now occurs in decodeAHRSPosUpdate()
                  q[3] = ((float)ahrs_pos_update.quat_z); // / 16384.0f; // This now occurs in decodeAHRSPosUpdate()
                  for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i]; // Range-check quaternion values
                  
                  // set our toxilibs quaternion to new data
                  quat.set(q[0], q[1], q[2], q[3]);
                  navx_device_moving = (( ahrs_pos_update.sensor_status & 0x01 ) != 0);
                  initial_calibration_in_process = (( ahrs_pos_update.cal_status & 0x02)==0);
                  compass_calibrated = (( ahrs_pos_update.cal_status & 0x04)!=0);
                  fused_heading_valid = (( ahrs_pos_update.sensor_status & 0x20)!=0);
                  mag_disturbance = (( ahrs_pos_update.sensor_status & 0x04)!=0);
                  altitude_valid = (( ahrs_pos_update.sensor_status & 0x08) != 0);
                  yaw_stable = (( ahrs_pos_update.sensor_status & 0x02)!= 0);          
                  curr_velocity_x = ahrs_pos_update.vel_x;
                  curr_velocity_y = ahrs_pos_update.vel_y;
                  curr_displacement_x = ahrs_pos_update.disp_x;
                  curr_displacement_y = ahrs_pos_update.disp_y;

                  String file_data = new String();
                  file_data = curr_sensor_timestamp + "," + q[0] + "," + q[1] + "," + q[2] + "," + q[3] + "," + 
                                yaw_degrees + "," + pitch_degrees + "," + roll_degrees + "," + 
                                world_linear_acceleration_x + "," + world_linear_acceleration_y + "," + 
                                curr_velocity_x + "," + curr_velocity_y + "," + 
                                curr_displacement_x + "," + curr_displacement_y + "\r\n"; 
                  if ( outputStream != null ) {
                    outputStream.write(file_data);
                  }
                } else {               
                  decode_length = AHRSProtocol.decodeBoardIDGetResponse(full_message, 0, full_message.length, board_id);
                  if (decode_length != 0) {
                    // Got new board id data
                    updateBoardType(board_id);
                    if ( windowBoardInfo != null ) {
                      windowBoardInfo.invalidate();
                    }  
                  } else {
                    print("<Unrecognized Message.  Len:  " + full_message.length + ">:  " + new String(full_message));
                  }
                }
              }
            }
          }
        }
      }        
    }
    catch(Exception ex ) {
      println("Exception during serialEvent():  " + ex.toString());
    }
/*      
      else {
        byte[] buffer = port.readBytes();
        println(new String(buffer) + " [" + String.valueOf(data_received_count) + "]" + " - Length:  " + String.valueOf(buffer.length));
        data_received_count++;
        port.clear();
      }
      */
  }
  /*
  if ( update_count > 1000 ) {
    int new_rate;
    if ( updateRateHz == 100 ) {
      new_rate = 50;
    }
    else if ( updateRateHz == 50 ) {
      new_rate = 20;
    }
    else {
      new_rate = 100;
    }
    enableRawUpdateMode(new_rate);
    update_count = 0;
  }
  */
}

void drawCylinder(float topRadius, float bottomRadius, float tall, int sides) {
    float angle = 0;
    float angleIncrement = TWO_PI / sides;
    beginShape(QUAD_STRIP);
    for (int i = 0; i < sides + 1; ++i) {
        vertex(topRadius*cos(angle), 0, topRadius*sin(angle));
        vertex(bottomRadius*cos(angle), tall, bottomRadius*sin(angle));
        angle += angleIncrement;
    }
    endShape();
    
    // If it is not a cone, draw the circular top cap
    if (topRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);
        
        // Center point
        vertex(0, 0, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(topRadius * cos(angle), 0, topRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
  
    // If it is not a cone, draw the circular bottom cap
    if (bottomRadius != 0) {
        angle = 0;
        beginShape(TRIANGLE_FAN);
    
        // Center point
        vertex(0, tall, 0);
        for (int i = 0; i < sides + 1; i++) {
            vertex(bottomRadius * cos(angle), tall, bottomRadius * sin(angle));
            angle += angleIncrement;
        }
        endShape();
    }
}
