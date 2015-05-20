
import com.kauailabs.navx_mxp.*;

// navX-MXP Open-Source Robotics Sensor
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
import processing.opengl.*;
import toxi.geom.*;
import toxi.processing.*;
import com.kauailabs.navx_mxp.*;

import java.util.*;

// NOTE: requires ToxicLibs to be installed in order to run properly.
// 1. Download from http://toxiclibs.org/downloads
// 2. Extract into [userdir]/Processing/libraries
//    (location may be different on Mac/Linux)
// 3. Run and bask in awesomeness

// NOTE:  Also requires the navX MXP library to be installed in order to run properly.
// 1. Download from http://code.google.com/p/navx-mxp
// 2. Place the navXMXP.jar library in the libraries subdirectory 

ToxiclibsSupport gfx;

Serial port;                         // The serial port
byte[] protocol_buffer = new byte[256];  // Buffer for received packets
byte[] protocol_buffer2 = new byte[256];
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
            
static final int ACCEL_HISTORY_SIZE = 10;
float world_acceleration_history[] = new float[ACCEL_HISTORY_SIZE];
int world_acceleration_history_index = 0;
float world_acceleration_recent_avg = 0.0;

/* navx-MXP AHRS Update-specific items */

boolean navx_mxp_moving = false;
boolean yaw_stable = true;
int last_update_ms = 0;

int updateRateHz = 0;
int gyroFSRDPS = 0;
int accelFSRG = 0;
float yaw_offset_degrees = 0.0;
boolean initial_calibration_in_process = false;
boolean mag_disturbance = true;
boolean altitude_valid = false;
boolean compass_calibrated = false;
boolean fused_heading_valid = false;
boolean multiple_serial_ports = false;
boolean serial_port_open_error = false;
String attempted_open_serial_port_name = "";
String opened_port_name = "";

// this table is used to store all command line parameters
// in the form: name=value
static Hashtable params=new Hashtable();
 
// here we overwrite PApplet's main entry point (for application mode)
// we're parsing all commandline arguments and copy only the relevant ones
 
static public void main(String args[]) {
  String[] newArgs=new String[args.length+1];
  /*********************************************************
  /* IMPORTANT: replace this with the name of your sketch  *
  /*********************************************************/
  newArgs[0]="navXMXPUI";
 
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
 

void enableRawUpdateMode(int rate) {

    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) IMUProtocol.STREAM_CMD_STREAM_TYPE_QUATERNION,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      port.write(stream_command);
      println(new String(protocol_buffer));
    }
    else {
      println("Error encoding stream command.");
    }
}

void enableAHRSUpdateMode(int rate) {
    int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) AHRSProtocol.MSGID_AHRS_UPDATE,(byte)rate);
    if (length != 0) {
      byte[] stream_command = new byte[length];
      arrayCopy(protocol_buffer,0,stream_command,0,length);
      port.write(stream_command);
      println(new String(protocol_buffer));
    }
    else {
      println("Error encoding stream command.");
    }
}

void setup() {
    // 500px square viewport using OpenGL rendering
    size(500, 500, OPENGL);
    gfx = new ToxiclibsSupport(this);

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
      println(Serial.list());
    } catch (Exception e){
      println("No valid serial ports.  Exiting...");
      exit();
    }    
    print("Done Enumerating Serial Ports.");

    String theportname = "";
    
    if ( Serial.list().length < 1 ) {
      println("No valid serial ports.");
      return;    
    }
    else if ( Serial.list().length > 1 ) {
      
      if ( params.size() < 1 ) { 
        multiple_serial_ports = true;
        println("Use command line to specify serial port to use");
        println("Syntax:  port=<PortNumber>");
        return;
      }
      else {
        // select the port based upon the provided parameter
        String[] portnames = Serial.list();
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
      theportname = Serial.list()[0];
    }
    
     print("Opening serial port ");
     println(theportname);
    // get the first available port (use EITHER this OR the specific port code below)
    String portName = theportname;
    
    // get a specific serial port (use EITHER this OR the first-available code above)
    //String portName = "COM4";
    
    // open the serial port
    try {
      port = new Serial(this, portName, 57600);
      opened_port_name = portName;
      port.setDTR(true);
    }
    catch(Exception ex) {
      print("Error opening serial port ");
      println(portName);
      attempted_open_serial_port_name = portName;
      serial_port_open_error = true;
      return;
    }
    
    int lf=10;
    port.bufferUntil(lf);
    
    // Give the navX MXP a few seconds to start up.
    delay(3000);
    
    // Send command to navX MXP requesting streaming data in 'raw' format
    //enableRawUpdateMode(100);    
    enableAHRSUpdateMode(100);
    //enableAHRSUpdateMode(100);
    //enableAHRSUpdateMode(100);
}

void draw() {
    
  
    // black background
    background(0);
    textAlign(LEFT);
    if ( port == null ) {
      textSize(26);
      fill(255,0,0);
      if ( multiple_serial_ports ) {
        text("Multiple Serial ports available.",(width/2)-150,height/2);
        text(" Specify port on command line.",(width/2)-150,height/2+30);
        text("   port=<PORTNAME>",(width/2)-150,height/2+60);
      }
      else if ( serial_port_open_error ) {
        text("Unable to open serial port " + attempted_open_serial_port_name,(width/2)-200,height/2);
      }
      else {
        text("No Serial Ports available",(width/2)-150,height/2);
      }
      return;
    }
    
    if ( initial_calibration_in_process ) {
      text("Calibrating Gyro...",(width/2)-55,(height/2)-100);
    }
    if ( mag_disturbance ) {
      text("Mag Disturbance",(width/2)-55,(height/2)-120);
    }
    
    textSize(36); 
    text("navX MXP", 20, 30); 
    textSize(16);
    text("Robotics Navigation Sensor",20,50);
    String gyrofsr = "Gyro Range:  +/- " + gyroFSRDPS + " Deg/sec";
    text(gyrofsr,20,70);
    String accelfsr = "Accel Range:  +/- " + accelFSRG + " G";
    text(accelfsr,20,90);
    String updaterate = "Update Rate:  " + updateRateHz + " Hz";
    text(updaterate,20,110);
    
    int timeout_ms = 2000;
    if ( ( millis() - last_update_ms ) >= timeout_ms ) {
      textSize(26);
      fill(255,0,0);
      text("Disconnected",(width/2)-100,(height/2)-100);
    }
    else {
      textSize(16);
      text("Connected",width-100,30);
      text(opened_port_name,width-100,50);
    }
    
    textSize(15);
    fill(255,255,255);

    textAlign(LEFT);
    text("Accel X:",20, height-80);
    textAlign(RIGHT);
    text(nfp(world_linear_acceleration_x,1,2),135,height-80);
    textAlign(LEFT);
    text("Accel Y:",20, height-60);
    textAlign(RIGHT);
    text(nfp(world_linear_acceleration_y,1,2),135,height-60);
    textAlign(LEFT);
    text("Accel Z:",20, height-40);
    textAlign(RIGHT);
    text(nfp(world_linear_acceleration_z,1,2),135,height-40);
    textAlign(LEFT);
    text("Temp:",20, height-20);
    textAlign(RIGHT);
    text(nfp(current_temp_c,1,2),135,height-20);
    if ( initial_calibration_in_process ) {
      fill(128,128,128);
    }
    textAlign(LEFT);
    text("Yaw (Z):",width-150,height-80);
    textAlign(RIGHT);
    text(nfp(yaw_degrees,3,2),width-15, height-80);
    fill(255,255,255);
    textAlign(LEFT);
    text("Pitch (Y):",width-150,height-100);
    textAlign(RIGHT);
    text(nfp(pitch_degrees,3,2),width-15, height-100);
    textAlign(LEFT);
    text("Roll (X):",width-150,height-120);
    textAlign(RIGHT);
    text(nfp(roll_degrees,3,2),width-15, height-120);
    textAlign(LEFT);
    if ( !compass_calibrated ) {
      fill(128,128,128);
    }
    text("Compass:",width-150,height-60);
    textAlign(RIGHT);
    text(nfp(tilt_compensated_heading_degrees,3,2),width-15, height-60);
    if ( !fused_heading_valid ) {
      fill(128,128,128);
    }
    textAlign(LEFT);
    text("Heading:",width-150,height-40);
    textAlign(RIGHT);
    text(nfp(fused_heading_degrees,3,2),width-15, height-40);
    textAlign(LEFT);
    if ( !altitude_valid ) {
      fill(255,0,0);
    }
    text("Altitude:",width-150,height-20);
    textAlign(RIGHT);
    text(nfp(altitude,3,2),width-15, height-20);
    
    fill(0,255,0);
    textAlign(LEFT);    
    String motion_state = "";
    if ( navx_mxp_moving || ( world_acceleration_recent_avg >= 0.01 ) ) {
      motion_state = "Moving";
    }
    text(motion_state,(width/2)-30,height-20);
    String rotation_state = "";
    if ( !yaw_stable ) {
      rotation_state = "Rotating";
    }
    text(rotation_state,(width/2)-35,height-40);
    
    // translate everything to the middle of the viewport
    pushMatrix();
    translate(width / 2, height / 2);

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

// The serialEvent() method will be invoked whenever one or more
// serial characters are received at the moment when a 
// line feed character (which terminates a navX MXP message)
// is received.

IMUProtocol.QuaternionUpdate raw_update = new IMUProtocol.QuaternionUpdate();
IMUProtocol.YPRUpdate ypr_update = new IMUProtocol.YPRUpdate();
IMUProtocol.StreamResponse stream_response = new IMUProtocol.StreamResponse();
AHRSProtocol.AHRSUpdate ahrs_update = new AHRSProtocol.AHRSUpdate();

long update_count = 0;

int ahrs_update_msg_len = 56;
int last_msg_start_received_timestamp = 0;
void serialEvent(Serial port) {

  try {

    int initial_bytes_received = 0;
    while ( port.available() > 0 ) {
      initial_bytes_received = port.readBytesUntil((int)'!',protocol_buffer);
      if ( initial_bytes_received > 0 ) {
        last_msg_start_received_timestamp = millis();
        int msg_len = port.readBytesUntil((int) '\n',protocol_buffer);
        if (msg_len > 0 ) {
          if ( ( protocol_buffer[0] == AHRSProtocol.BINARY_PACKET_INDICATOR_CHAR ) && 
               ( msg_len > 1 ) &&
               ( protocol_buffer[msg_len-2] != '\r' ) ) {
            int indicated_message_length = protocol_buffer[1];
            while ( msg_len < indicated_message_length-1 ) {
              int remainder_len = port.readBytes(protocol_buffer2);
              if ( remainder_len > 0 ) {
                for ( int x = 0; x < remainder_len; x++ ) {
                  protocol_buffer[msg_len+x] = protocol_buffer2[x];
                }
                msg_len += remainder_len;
              } else {
                /* Nothing in the buffer.  Wait for more data, w/a timeout. */
                  if ( ( millis() - last_msg_start_received_timestamp ) > 10 ) {
                    println("Timeout.");
                    return;
                  }
              }
            }
          }
          println(msg_len);
          byte[] full_message = new byte[msg_len+1];
          full_message[0] = '!';
          arrayCopy(protocol_buffer,0,full_message,1,msg_len);
          
          print(new String(full_message));
          
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
              enableAHRSUpdateMode(100);
            }
            else {
              decode_length = IMUProtocol.decodeStreamResponse(full_message,0, full_message.length,stream_response);
              if ( decode_length > 0 ) {
                updateRateHz = stream_response.update_rate_hz;
                gyroFSRDPS = stream_response.gyro_fsr_dps;
                accelFSRG = stream_response.accel_fsr_g;
                yaw_offset_degrees = stream_response.yaw_offset_degrees;
                short calibration_state = (short)(stream_response.flags & IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
                initial_calibration_in_process = (calibration_state != IMUProtocol.NAV6_CALIBRATION_STATE_COMPLETE);
              } else {
                decode_length = AHRSProtocol.decodeAHRSUpdate(full_message, 0, full_message.length, ahrs_update);
                if (decode_length != 0) {
                  update_count++;
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
                  
                  q[0] = ((float)ahrs_update.quat_w) / 16384.0f;
                  q[1] = ((float)ahrs_update.quat_x) / 16384.0f;
                  q[2] = ((float)ahrs_update.quat_y) / 16384.0f;
                  q[3] = ((float)ahrs_update.quat_z) / 16384.0f;
                  for (int i = 0; i < 4; i++) if (q[i] >= 2) q[i] = -4 + q[i]; // Range-check quaternion values
                  
                  // set our toxilibs quaternion to new data
                  quat.set(q[0], q[1], q[2], q[3]);
                  navx_mxp_moving = (( ahrs_update.sensor_status & 0x01 ) != 0);
                  initial_calibration_in_process = (( ahrs_update.cal_status & 0x02)==0);
                  compass_calibrated = (( ahrs_update.cal_status & 0x04)!=0);
                  fused_heading_valid = (( ahrs_update.sensor_status & 0x20)!=0);
                  mag_disturbance = (( ahrs_update.sensor_status & 0x04)!=0);
                  altitude_valid = (( ahrs_update.sensor_status & 0x08) != 0);
                  yaw_stable = (( ahrs_update.sensor_status & 0x02)!= 0);
                  println(ahrs_update.sensor_status);
                  float mag_field_norm = sqrt( 
                    ((float)ahrs_update.raw_mag_x * (float)ahrs_update.raw_mag_x) +
                    ((float)ahrs_update.raw_mag_y * (float)ahrs_update.raw_mag_y) +
                    ((float)ahrs_update.raw_mag_z * (float)ahrs_update.raw_mag_z) );
                  println(ahrs_update.raw_mag_x + ", " + ahrs_update.raw_mag_y + ", " + ahrs_update.raw_mag_z + ", ", mag_field_norm);
                } else {
                  print("<Unrecognized Message.  Len:  " + full_message.length + ">");
                }
              }
            }
          }        
        }
      }
      else {
        byte[] buffer = port.readBytes();
        println(new String(buffer));
        port.clear();
      }
    }
  }
  catch(Exception ex ) {
    println("Exception during serialEvent()");
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
