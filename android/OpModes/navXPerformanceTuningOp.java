/* Copyright (c) 2014, 2015 Qualcomm Technologies Inc

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted (subject to the limitations in the disclaimer below) provided that
the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Qualcomm Technologies Inc nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS
LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

package com.qualcomm.ftcrobotcontroller.opmodes;

import android.os.SystemClock;

import com.kauailabs.navx.ftc.AHRS;
import com.qualcomm.robotcore.eventloop.opmode.OpMode;
import com.qualcomm.robotcore.util.ElapsedTime;

import java.text.DecimalFormat;

/**
 * navX-Micro Performance Tuning Op
 *
 * This opmode provides insight into the peformance of the communication
 * with the navX-Model sensor over the I2C bus via the Core Device Interface
 * Module.  Since the Android operating system is not a real-time
 * operating system, and since communication w/the navX-Model sensor is
 * occurring over a wifi-direct network which can be prone to interference,
 * the actual performance (update rate) achieved may be less than
 * one might expect.
 *
 * Since the navX-Model devices integrate sensor data onboard, to achieve
 * the best performance for device control methods like a PID controller
 * that work best with constant-time updates, the strategy is to:
 *
 * 1) Configure the navX-Model device to a high update rate (e.g., 50Hz)
 * 2) Using this performance-tuning Op-Mode (with all other
 * sensors connected, just as your robot will be configured for normal
 * use) observe the "effective" update rate, and the number of missed
 * samples over the last minute.
 * 3) Lower the navX-Model device update rate until the number of missed
 * samples over the last minute reaches zero.
 */
public class navXPerformanceTuningOp extends OpMode implements AHRS.Callback {

  /* This is the port on the Core Device Interace Module */
  /* in which the navX-Micro is connected.  Modify this  */
  /* depending upon which I2C port you are using.        */
  private final int NAVX_DIM_I2C_PORT = 0;

  private ElapsedTime runtime = new ElapsedTime();
  private AHRS navx_device;
  private long last_system_timestamp = 0;
  private long last_sensor_timestamp = 0;

  private long sensor_timestamp_delta = 0;
  private long system_timestamp_delta = 0;

  private byte sensor_update_rate_hz = 40;

  private int missing_sensor_sample_count = 0;
  private boolean first_sample_received = false;

  @Override
  public void init() {
    AHRS.setLogging(true);
    navx_device = AHRS.getInstance(hardwareMap.deviceInterfaceModule.get("dim"),
            NAVX_DIM_I2C_PORT,
            AHRS.DeviceDataType.kProcessedData,
            sensor_update_rate_hz);
  }

@Override
  public void start() {
    navx_device.registerCallback(this);
}

  @Override
  public void stop() {
    navx_device.close();
  }
  /*
     * Code to run when the op mode is first enabled goes here
     * @see com.qualcomm.robotcore.eventloop.opmode.OpMode#start()
     */
  @Override
  public void init_loop() {
    telemetry.addData("navX Op Init Loop", runtime.toString());
  }

  /*
   * This method will be called repeatedly in a loop
   * @see com.qualcomm.robotcore.eventloop.opmode.OpMode#loop()
   */
  @Override
  public void loop() {

      boolean connected = navx_device.isConnected();
      telemetry.addData("1 navX-Device...............:", connected ?
              "Connected" : "Disconnected" );
      String gyrocal, motion;
      DecimalFormat df = new DecimalFormat("#.##");

      telemetry.addData("2 Sensor Rate (Hz)...", Byte.toString(navx_device.getActualUpdateRate()));
      telemetry.addData("3 Transfer Rate (Hz).", Integer.toString(navx_device.getCurrentTransferRate()));
      telemetry.addData("4 Effective Rate (Hz)", Integer.toString(last_second_hertz));
      telemetry.addData("5 Dropped Samples....", Integer.toString(missing_sensor_sample_count));
      telemetry.addData("6 Duplicate Samples..", Integer.toString(navx_device.getDuplicateDataCount()));
      telemetry.addData("7 Sensor deltaT (ms).", Long.toString(sensor_timestamp_delta));
      telemetry.addData("8 System deltaT (ms).", Long.toString(system_timestamp_delta));
  }

  public int hertz_counter = 0;
  public int last_second_hertz = 0;

  /* This callback is invoked by the AHRS class whenever new data is
     received from the sensor.  Note that this callback is occurring
     within the context of the AHRS class IO thread, and it may
     interrupt the thread running this opMode.  Therefore, it is
     very important to use thread synchronization techniques when
     communicating between this callback and the rest of the
     code in this opMode.

     The difference between the current linear acceleration data in
     the X and Y axes and that in the last sample is compared.  If
     the absolute value of that difference is greater than the
     "Collision Detection Threshold", a collision event is declared.
     */
  public void newProcessedDataAvailable(long curr_sensor_timestamp) {

      final int MS_PER_SEC = 1000;
      final int NAVX_TIMESTAMP_JITTER_MS = 2;
      long num_dropped = 0;

      long curr_system_timestamp = SystemClock.elapsedRealtime();
      byte sensor_update_rate = navx_device.getActualUpdateRate();

      sensor_timestamp_delta = curr_sensor_timestamp - last_sensor_timestamp;
      system_timestamp_delta = curr_system_timestamp - last_system_timestamp;
      int expected_sample_time_ms = MS_PER_SEC / (int)sensor_update_rate;

      if ( !navx_device.isConnected() ) {
          last_second_hertz = 0;
          hertz_counter = 0;
          first_sample_received = false;
      } else {
          if ( ( curr_system_timestamp % 1000 ) < ( last_system_timestamp % 1000 ) ) {
              /* Second roll over.  Start the Hertz accumulator */
              last_second_hertz = hertz_counter;
              hertz_counter = 1;
          } else {
              hertz_counter++;
          }
          if ( !first_sample_received ) {
              last_sensor_timestamp = curr_sensor_timestamp;
              first_sample_received = true;
              missing_sensor_sample_count = 0;
          } else {
              if (sensor_timestamp_delta > (expected_sample_time_ms + NAVX_TIMESTAMP_JITTER_MS) ) {
                  long dropped_samples = (sensor_timestamp_delta / expected_sample_time_ms) - 1;
                  if (dropped_samples > 0) {
                      if ( dropped_samples > 10 )  {
                          num_dropped = dropped_samples;
                      }
                      missing_sensor_sample_count += dropped_samples;
                  }
              }
          }
      }

      last_sensor_timestamp = curr_sensor_timestamp;
      last_system_timestamp = curr_system_timestamp;

  }

  public void newQuatAndRawDataAvailable() {

  }

}
