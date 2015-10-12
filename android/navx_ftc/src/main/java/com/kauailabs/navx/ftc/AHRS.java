/* ============================================
 NavX-MXP and NavX-Micro source code is placed under the MIT license
 Copyright (c) 2015 Kauai Labs

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
 package com.kauailabs.navx.ftc;

import com.kauailabs.navx.AHRSProtocol;
import com.kauailabs.navx.IMUProtocol;
import com.kauailabs.navx.IMURegisters;

import com.qualcomm.robotcore.hardware.DeviceInterfaceModule;
import com.qualcomm.robotcore.hardware.I2cController;
import com.qualcomm.robotcore.hardware.I2cDevice;

import java.util.Arrays;

/**
 * The AHRS class provides an interface to AHRS capabilities
 * of the KauaiLabs navX Robotics Navigation Sensor via I2C on the Android-
 * based FTC robotics control system, where communications occur via the
 * "Core Device Interface Module" produced by Modern Robotics, inc.
 *
 * The AHRS class enables access to basic connectivity and state information,
 * as well as key 6-axis and 9-axis orientation information (yaw, pitch, roll,
 * compass heading, fused (9-axis) heading and magnetic disturbance detection.
 *
 * Additionally, the ARHS class also provides access to extended information
 * including linear acceleration, motion detection, rotation detection and sensor
 * temperature.
 *
 * If used with navX-Aero-enabled devices, the AHRS class also provides access to
 * altitude, barometric pressure and pressure sensor temperature data
 * @author Scott
 */
public class AHRS {

    /**
     * Identifies one of the three sensing axes on the navX sensor board.  Note that these axes are
     * board-relative ("Board Frame"), and are not necessarily the same as the logical axes of the
     * chassis on which the sensor is mounted.
     *
     * For more information on sensor orientation, please see the navX sensor
     * <a href=http://navx-micro.kauailabs.com//installation/orientation/>Orientation</a> page.
     */
    public enum BoardAxis {
        kBoardAxisX(0),
        kBoardAxisY(1),
        kBoardAxisZ(2);

        private int value;

        private BoardAxis(int value) {
            this.value = value;
        }
        public int getValue() {
            return this.value;
        }
    };

    /**
     * Indicates which sensor board axis is used as the "yaw" (gravity) axis.
     *
     * This selection may be modified via the <a href=http://navx-micro.kauailabs.com/installation/omnimount/>Omnimount</a> feature.
     *
     */
    static public class BoardYawAxis
    {
        public BoardAxis board_axis;
        public boolean up;
    };

    /**
     * The DeviceDataType specifies the
     * type of data to be retrieved from the sensor.  Due to limitations in the
     * communication bandwidth, only a subset of all available data can be streamed
     * and still maintain a 50Hz update rate via the Core Device Interface Module,
     * since it is limited to a maximum of one 26-byte transfer every 10ms.
     * Note that if all data types are required,
     */
    public enum DeviceDataType {
        /**
         * (default):  6 and 9-axis processed data
         */
        kProcessedData(0),
        /**
         * unprocessed data from each individual sensor
         */
        kRawData(1),
        /**
         * Both processed and raw data
         */
        kBoth(2);

        private int value;

        private DeviceDataType(int value){
            this.value = value;
        }

        public int getValue(){
            return this.value;
        }
    };

    private class BoardState {
        public short capability_flags;
        public byte  update_rate_hz;
        public short accel_fsr_g;
        public short gyro_fsr_dps;
    }

    private static AHRS instance = null;
    private static final int NAVX_DEFAULT_UPDATE_RATE_HZ = 60;

    private DeviceInterfaceModule dim = null;
    private navXIOThread io_thread_obj;
    private Thread io_thread;
    private int update_rate_hz = NAVX_DEFAULT_UPDATE_RATE_HZ;

    AHRSProtocol.AHRSPosUpdate curr_data;
    BoardState board_state;
    AHRSProtocol.BoardID board_id;
    IMUProtocol.GyroUpdate raw_data_update;

    final int NAVX_I2C_DEV_ADDRESS = 0x32;

    protected AHRS(DeviceInterfaceModule dim, int dim_i2c_port, DeviceDataType data_type, int update_rate_hz) {
        this.dim = dim;
        this.update_rate_hz = update_rate_hz;
        this.curr_data = new AHRSProtocol.AHRSPosUpdate();
        this.board_state = new BoardState();
        this.board_id = new AHRSProtocol.BoardID();
        this.raw_data_update = new IMUProtocol.GyroUpdate();

        io_thread_obj   = new navXIOThread(dim_i2c_port, update_rate_hz, data_type, curr_data);
        io_thread_obj.start();

        io_thread       = new Thread(io_thread_obj);
        io_thread.start();
    }

    public void close() {
        io_thread_obj.stop();
        try {
            io_thread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        instance = null;
    }

    public static AHRS getInstance(DeviceInterfaceModule dim, int dim_i2c_port, DeviceDataType data_type) {
        if (instance == null) {
            instance = new AHRS(dim, dim_i2c_port, data_type, NAVX_DEFAULT_UPDATE_RATE_HZ);
        }
        return instance;
    }

    /**
     * Returns the current pitch value (in degrees, from -180 to 180)
     * reported by the sensor.  Pitch is a measure of rotation around
     * the X Axis.
     * @return The current pitch value in degrees (-180 to 180).
     */
    public float getPitch() {
        return curr_data.pitch;
    }

    /**
     * Returns the current roll value (in degrees, from -180 to 180)
     * reported by the sensor.  Roll is a measure of rotation around
     * the X Axis.
     * @return The current roll value in degrees (-180 to 180).
     */
    public float getRoll() {
        return curr_data.roll;
    }

    /**
     * Returns the current yaw value (in degrees, from -180 to 180)
     * reported by the sensor.  Yaw is a measure of rotation around
     * the Z Axis (which is perpendicular to the earth).
     *<p>
     * Note that the returned yaw value will be offset by a user-specified
     * offset value; this user-specified offset value is set by
     * invoking the zeroYaw() method.
     * @return The current yaw value in degrees (-180 to 180).
     */
    public float getYaw() {
        return curr_data.yaw;
    }

    /**
     * Returns the current tilt-compensated compass heading
     * value (in degrees, from 0 to 360) reported by the sensor.
     *<p>
     * Note that this value is sensed by a magnetometer,
     * which can be affected by nearby magnetic fields (e.g., the
     * magnetic fields generated by nearby motors).
     *<p>
     * Before using this value, ensure that (a) the magnetometer
     * has been calibrated and (b) that a magnetic disturbance is
     * not taking place at the instant when the compass heading
     * was generated.
     * @return The current tilt-compensated compass heading, in degrees (0-360).
     */
    public float getCompassHeading() {
        return curr_data.compass_heading;
    }

    /**
     * Sets the user-specified yaw offset to the current
     * yaw value reported by the sensor.
     *<p>
     * This user-specified yaw offset is automatically
     * subtracted from subsequent yaw values reported by
     * the getYaw() method.
     */
    public void zeroYaw() {
        io_thread_obj.zeroYaw();
    }

    /**
     * Returns true if the sensor is currently performing automatic
     * gyro/accelerometer calibration.  Automatic calibration occurs
     * when the sensor is initially powered on, during which time the
     * sensor should be held still, with the Z-axis pointing up
     * (perpendicular to the earth).
     *<p>
     * NOTE:  During this automatic calibration, the yaw, pitch and roll
     * values returned may not be accurate.
     *<p>
     * Once calibration is complete, the sensor will automatically remove
     * an internal yaw offset value from all reported values.
     *<p>
     * @return Returns true if the sensor is currently automatically
     * calibrating the gyro and accelerometer sensors.
     */

    public boolean isCalibrating() {
        return !((curr_data.cal_status &
                AHRSProtocol.NAVX_CAL_STATUS_IMU_CAL_STATE_MASK) ==
                AHRSProtocol.NAVX_CAL_STATUS_IMU_CAL_COMPLETE);
    }

    /**
     * Indicates whether the sensor is currently connected
     * to the host computer.  A connection is considered established
     * whenever communication with the sensor has occurred recently.
     *<p>
     * @return Returns true if a valid update has been recently received
     * from the sensor.
     */

    public boolean isConnected() {
        return io_thread_obj.isConnected();
    }

    /**
     * Returns the count in bytes of data received from the
     * sensor.  This could can be useful for diagnosing
     * connectivity issues.
     *<p>
     * If the byte count is increasing, but the update count
     * (see getUpdateCount()) is not, this indicates a software
     * misconfiguration.
     * @return The number of bytes received from the sensor.
     */
    public double getByteCount() {
        return io_thread_obj.getByteCount();
    }

    /**
     * Returns the count of valid updates which have
     * been received from the sensor.  This count should increase
     * at the same rate indicated by the configured update rate.
     * @return The number of valid updates received from the sensor.
     */
    public double getUpdateCount() {
        return io_thread_obj.getUpdateCount();
    }

    /**
     * Returns the current linear acceleration in the X-axis (in G).
     *<p>
     * World linear acceleration refers to raw acceleration data, which
     * has had the gravity component removed, and which has been rotated to
     * the same reference frame as the current yaw value.  The resulting
     * value represents the current acceleration in the x-axis of the
     * body (e.g., the robot) on which the sensor is mounted.
     *<p>
     * @return Current world linear acceleration in the X-axis (in G).
     */
    public float getWorldLinearAccelX()
    {
        return curr_data.linear_accel_x;
    }

    /**
     * Returns the current linear acceleration in the Y-axis (in G).
     *<p>
     * World linear acceleration refers to raw acceleration data, which
     * has had the gravity component removed, and which has been rotated to
     * the same reference frame as the current yaw value.  The resulting
     * value represents the current acceleration in the Y-axis of the
     * body (e.g., the robot) on which the sensor is mounted.
     *<p>
     * @return Current world linear acceleration in the Y-axis (in G).
     */
    public float getWorldLinearAccelY()
    {
        return curr_data.linear_accel_y;
    }

    /**
     * Returns the current linear acceleration in the Z-axis (in G).
     *<p>
     * World linear acceleration refers to raw acceleration data, which
     * has had the gravity component removed, and which has been rotated to
     * the same reference frame as the current yaw value.  The resulting
     * value represents the current acceleration in the Z-axis of the
     * body (e.g., the robot) on which the sensor is mounted.
     *<p>
     * @return Current world linear acceleration in the Z-axis (in G).
     */
    public float getWorldLinearAccelZ()
    {
        return curr_data.linear_accel_z;
    }

    /**
     * Indicates if the sensor is currently detecting motion,
     * based upon the X and Y-axis world linear acceleration values.
     * If the sum of the absolute values of the X and Y axis exceed
     * a "motion threshold", the motion state is indicated.
     *<p>
     * @return Returns true if the sensor is currently detecting motion.
     */
    public boolean isMoving()
    {
        return (((curr_data.sensor_status &
                AHRSProtocol.NAVX_SENSOR_STATUS_MOVING) != 0)
                ? true : false);
    }

    /**
     * Indicates if the sensor is currently detecting yaw rotation,
     * based upon whether the change in yaw over the last second
     * exceeds the "Rotation Threshold."
     *<p>
     * Yaw Rotation can occur either when the sensor is rotating, or
     * when the sensor is not rotating AND the current gyro calibration
     * is insufficiently calibrated to yield the standard yaw drift rate.
     *<p>
     * @return Returns true if the sensor is currently detecting motion.
     */
    public boolean isRotating()
    {
        return (((curr_data.sensor_status &
                AHRSProtocol.NAVX_SENSOR_STATUS_YAW_STABLE) != 0)
                ? false : true);
    }

    /**
     * Returns the current altitude, based upon calibrated readings
     * from a barometric pressure sensor, and the currently-configured
     * sea-level barometric pressure [navX Aero only].  This value is in units of meters.
     *<p>
     * NOTE:  This value is only valid sensors including a pressure
     * sensor.  To determine whether this value is valid, see
     * isAltitudeValid().
     *<p>
     * @return Returns current altitude in meters (as long as the sensor includes
     * an installed on-board pressure sensor).
     */
    public float getAltitude()
    {
        return curr_data.altitude;
    }

    /**
     * Indicates whether the current altitude (and barometric pressure) data is
     * valid. This value will only be true for a sensor with an onboard
     * pressure sensor installed.
     *<p>
     * If this value is false for a board with an installed pressure sensor,
     * this indicates a malfunction of the onboard pressure sensor.
     *<p>
     * @return Returns true if a working pressure sensor is installed.
     */
    public boolean isAltitudeValid()
    {
        return (((curr_data.sensor_status &
                AHRSProtocol.NAVX_SENSOR_STATUS_ALTITUDE_VALID) != 0)
                ? true : false);
    }

    /**
     * Returns the "fused" (9-axis) heading.
     *<p>
     * The 9-axis heading is the fusion of the yaw angle, the tilt-corrected
     * compass heading, and magnetic disturbance detection.  Note that the
     * magnetometer calibration procedure is required in order to
     * achieve valid 9-axis headings.
     *<p>
     * The 9-axis Heading represents the sensor's best estimate of current heading,
     * based upon the last known valid Compass Angle, and updated by the change in the
     * Yaw Angle since the last known valid Compass Angle.  The last known valid Compass
     * Angle is updated whenever a Calibrated Compass Angle is read and the sensor
     * has recently rotated less than the Compass Noise Bandwidth (~2 degrees).
     * @return Fused Heading in Degrees (range 0-360)
     */
    public float getFusedHeading()
    {
        return curr_data.fused_heading;
    }

    /**
     * Indicates whether the current magnetic field strength diverges from the
     * calibrated value for the earth's magnetic field by more than the currently-
     * configured Magnetic Disturbance Ratio.
     *<p>
     * This function will always return false if the sensor's magnetometer has
     * not yet been calibrated; see isMagnetometerCalibrated().
     * @return true if a magnetic disturbance is detected (or the magnetometer is uncalibrated).
     */
    public boolean isMagneticDisturbance()
    {
        return (((curr_data.sensor_status &
                AHRSProtocol.NAVX_SENSOR_STATUS_MAG_DISTURBANCE) != 0)
                ? true : false);    }

    /**
     * Indicates whether the magnetometer has been calibrated.
     *<p>
     * Magnetometer Calibration must be performed by the user.
     *<p>
     * Note that if this function does indicate the magnetometer is calibrated,
     * this does not necessarily mean that the calibration quality is sufficient
     * to yield valid compass headings.
     *<p>
     * @return Returns true if magnetometer calibration has been performed.
     */
    public boolean isMagnetometerCalibrated()
    {
        return (((curr_data.cal_status &
                AHRSProtocol.NAVX_CAL_STATUS_MAG_CAL_COMPLETE) != 0)
                ? true : false);
    }

    /* Unit Quaternions */

    /**
     * Returns the imaginary portion (W) of the Orientation Quaternion which
     * fully describes the current sensor orientation with respect to the
     * reference angle defined as the angle at which the yaw was last "zeroed".
     *<p>
     * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
     * to 2.  This total range (4) can be associated with a unit circle, since
     * each circle is comprised of 4 PI Radians.
     * <p>
     * For more information on Quaternions and their use, please see this <a href=https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>definition</a>.
     * @return Returns the imaginary portion (W) of the quaternion.
     */
    public float getQuaternionW() {
        return ((float)curr_data.quat_w / 16384.0f);
    }
    /**
     * Returns the real portion (X axis) of the Orientation Quaternion which
     * fully describes the current sensor orientation with respect to the
     * reference angle defined as the angle at which the yaw was last "zeroed".
     * <p>
     * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
     * to 2.  This total range (4) can be associated with a unit circle, since
     * each circle is comprised of 4 PI Radians.
     * <p>
     * For more information on Quaternions and their use, please see this <a href=https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation>description</a>.
     * @return Returns the real portion (X) of the quaternion.
     */
    public float getQuaternionX() {
        return ((float)curr_data.quat_x / 16384.0f);
    }
    /**
     * Returns the real portion (X axis) of the Orientation Quaternion which
     * fully describes the current sensor orientation with respect to the
     * reference angle defined as the angle at which the yaw was last "zeroed".
     *
     * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
     * to 2.  This total range (4) can be associated with a unit circle, since
     * each circle is comprised of 4 PI Radians.
     *
     * For more information on Quaternions and their use, please see:
     *
     *   https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
     *
     * @return Returns the real portion (X) of the quaternion.
     */
    public float getQuaternionY() {
        return ((float)curr_data.quat_y / 16384.0f);
    }
    /**
     * Returns the real portion (X axis) of the Orientation Quaternion which
     * fully describes the current sensor orientation with respect to the
     * reference angle defined as the angle at which the yaw was last "zeroed".
     *
     * Each quaternion value (W,X,Y,Z) is expressed as a value ranging from -2
     * to 2.  This total range (4) can be associated with a unit circle, since
     * each circle is comprised of 4 PI Radians.
     *
     * For more information on Quaternions and their use, please see:
     *
     *   https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
     *
     * @return Returns the real portion (X) of the quaternion.
     */
    public float getQuaternionZ() {
        return ((float)curr_data.quat_z / 16384.0f);
    }

    /**
     * Returns the current temperature (in degrees centigrade) reported by
     * the sensor's gyro/accelerometer circuit.
     *<p>
     * This value may be useful in order to perform advanced temperature-
     * correction of raw gyroscope and accelerometer values.
     *<p>
     * @return The current temperature (in degrees centigrade).
     */
    public float getTempC()
    {
        return curr_data.mpu_temp;
    }

    /**
     * Returns information regarding which sensor board axis (X,Y or Z) and
     * direction (up/down) is currently configured to report Yaw (Z) angle
     * values.   NOTE:  If the board firmware supports Omnimount, the board yaw
     * axis/direction are configurable.
     *<p>
     * For more information on Omnimount, please see:
     *<p>
     * http://navx-mxp.kauailabs.com/navx-mxp/installation/omnimount/
     *<p>
     * @return The currently-configured board yaw axis/direction.
     */
    public BoardYawAxis getBoardYawAxis() {
        BoardYawAxis yaw_axis = new BoardYawAxis();
        short yaw_axis_info = (short)(board_state.capability_flags >> 3);
        yaw_axis_info &= 7;
        if ( yaw_axis_info == AHRSProtocol.OMNIMOUNT_DEFAULT) {
            yaw_axis.up = true;
            yaw_axis.board_axis = BoardAxis.kBoardAxisZ;
        } else {
            yaw_axis.up = (((yaw_axis_info & 0x01) != 0) ? true : false);
            yaw_axis_info >>= 1;
            switch ( (byte)yaw_axis_info ) {
                case 0:
                    yaw_axis.board_axis = BoardAxis.kBoardAxisX;
                    break;
                case 1:
                    yaw_axis.board_axis = BoardAxis.kBoardAxisY;
                    break;
                case 2:
                default:
                    yaw_axis.board_axis = BoardAxis.kBoardAxisZ;
                    break;
            }
        }
        return yaw_axis;
    }

    /**
     * Returns the version number of the firmware currently executing
     * on the sensor.
     *<p>
     * To update the firmware to the latest version, please see:
     *<p>
     *   http://navx-mxp.kauailabs.com/navx-mxp/support/updating-firmware/
     *<p>
     * @return The firmware version in the format [MajorVersion].[MinorVersion]
     */
    public String getFirmwareVersion() {
        double version_number = (double)board_id.fw_ver_major;
        version_number += ((double)board_id.fw_ver_minor / 10);
        String fw_version = Double.toString(version_number);
        return fw_version;
    }

    private final float DEV_UNITS_MAX = 32768.0f;

    /**
     * Returns the current raw (unprocessed) X-axis gyro rotation rate (in degrees/sec).  NOTE:  this
     * value is un-processed, and should only be accessed by advanced users.
     * Typically, rotation about the X Axis is referred to as "Pitch".  Calibrated
     * and Integrated Pitch data is accessible via the {@link #getPitch()} method.
     *<p>
     * @return Returns the current rotation rate (in degrees/sec).
     */
    public float getRawGyroX() {
        return this.raw_data_update.gyro_x / (DEV_UNITS_MAX / (float)this.board_state.gyro_fsr_dps);
    }

    /**
     * Returns the current raw (unprocessed) Y-axis gyro rotation rate (in degrees/sec).  NOTE:  this
     * value is un-processed, and should only be accessed by advanced users.
     * Typically, rotation about the T Axis is referred to as "Roll".  Calibrated
     * and Integrated Pitch data is accessible via the {@link #getRoll()} method.
     *<p>
     * @return Returns the current rotation rate (in degrees/sec).
     */
    public float getRawGyroY() {
        return this.raw_data_update.gyro_y / (DEV_UNITS_MAX / (float)this.board_state.gyro_fsr_dps);
    }

    /**
     * Returns the current raw (unprocessed) Z-axis gyro rotation rate (in degrees/sec).  NOTE:  this
     * value is un-processed, and should only be accessed by advanced users.
     * Typically, rotation about the T Axis is referred to as "Yaw".  Calibrated
     * and Integrated Pitch data is accessible via the {@link #getYaw()} method.
     *<p>
     * @return Returns the current rotation rate (in degrees/sec).
     */
    public float getRawGyroZ() {
        return this.raw_data_update.gyro_z / (DEV_UNITS_MAX / (float)this.board_state.gyro_fsr_dps);
    }

    /**
     * Returns the current raw (unprocessed) X-axis acceleration rate (in G).  NOTE:  this
     * value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not had acceleration due to gravity removed from it, and has not been rotated to
     * the world reference frame.  Gravity-corrected, world reference frame-corrected
     * X axis acceleration data is accessible via the {@link #getWorldLinearAccelX()} method.
     *<p>
     * @return Returns the current acceleration rate (in G).
     */
    public float getRawAccelX() {
        return this.raw_data_update.accel_x / (DEV_UNITS_MAX / (float)this.board_state.accel_fsr_g);
    }

    /**
     * Returns the current raw (unprocessed) Y-axis acceleration rate (in G).  NOTE:  this
     * value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not had acceleration due to gravity removed from it, and has not been rotated to
     * the world reference frame.  Gravity-corrected, world reference frame-corrected
     * Y axis acceleration data is accessible via the {@link #getWorldLinearAccelY()} method.
     *<p>
     * @return Returns the current acceleration rate (in G).
     */
    public float getRawAccelY() {
        return this.raw_data_update.accel_y / (DEV_UNITS_MAX / (float)this.board_state.accel_fsr_g);
    }

    /**
     * Returns the current raw (unprocessed) Z-axis acceleration rate (in G).  NOTE:  this
     * value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not had acceleration due to gravity removed from it, and has not been rotated to
     * the world reference frame.  Gravity-corrected, world reference frame-corrected
     * Z axis acceleration data is accessible via the {@link #getWorldLinearAccelZ()} method.
     *<p>
     * @return Returns the current acceleration rate (in G).
     */
    public float getRawAccelZ() {
        return this.raw_data_update.accel_z / (DEV_UNITS_MAX / (float)this.board_state.accel_fsr_g);
    }

    private final float UTESLA_PER_DEV_UNIT = 0.15f;

    /**
     * Returns the current raw (unprocessed) X-axis magnetometer reading (in uTesla).  NOTE:
     * this value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not been tilt-corrected, and has not been combined with the other magnetometer axis
     * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
     * via the {@link #getCompassHeading()} method.
     *<p>
     * @return Returns the mag field strength (in uTesla).
     */
    public float getRawMagX() {
        return this.raw_data_update.mag_x / UTESLA_PER_DEV_UNIT;
    }

    /**
     * Returns the current raw (unprocessed) Y-axis magnetometer reading (in uTesla).  NOTE:
     * this value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not been tilt-corrected, and has not been combined with the other magnetometer axis
     * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
     * via the {@link #getCompassHeading()} method.
     *<p>
     * @return Returns the mag field strength (in uTesla).
     */
    public float getRawMagY() {
        return this.raw_data_update.mag_y / UTESLA_PER_DEV_UNIT;
    }

    /**
     * Returns the current raw (unprocessed) Z-axis magnetometer reading (in uTesla).  NOTE:
     * this value is unprocessed, and should only be accessed by advanced users.  This raw value
     * has not been tilt-corrected, and has not been combined with the other magnetometer axis
     * data to yield a compass heading.  Tilt-corrected compass heading data is accessible
     * via the {@link #getCompassHeading()} method.
     *<p>
     * @return Returns the mag field strength (in uTesla).
     */
    public float getRawMagZ() {
        return this.raw_data_update.mag_z / UTESLA_PER_DEV_UNIT;
    }

    /**
     * Returns the current barometric pressure (in millibar) [navX Aero only].
     *<p>
     *This value is valid only if a barometric pressure sensor is onboard.
     *
     * @return Returns the current barometric pressure (in millibar).
     */
    public float getPressure() {
        // TODO implement for navX-Aero.
        return 0;
    }

    class navXIOThread implements Runnable {

        int dim_port;
        int update_rate_hz;
        protected boolean keep_running;
        boolean request_zero_yaw;
        boolean is_connected;
        int byte_count;
        int update_count;
        DeviceDataType data_type;
        AHRSProtocol.AHRSPosUpdate ahrspos_update;

        public navXIOThread( int port, int update_rate_hz, DeviceDataType data_type,
                             AHRSProtocol.AHRSPosUpdate ahrspos_update) {
            this.dim_port = port;
            this.keep_running = false;
            this.update_rate_hz = update_rate_hz;
            this.request_zero_yaw = false;
            this.is_connected = false;
            this.byte_count = 0;
            this.update_count = 0;
            this.ahrspos_update = ahrspos_update;
            this.data_type = data_type;
        }

        public void start() {
            keep_running = true;
        }
        public void stop() {
            keep_running = false;
        }

        public void zeroYaw() {
            request_zero_yaw = true;
        }

        public int getByteCount() {
            return byte_count;
        }

        public int getUpdateCount() {
            return update_count;
        }

        public boolean isConnected() {
            return is_connected;
        }

        @Override
        public void run() {

            final int DIM_MAX_I2C_READ_LEN     = 26;
            final int NAVX_REGISTER_FIRST      = IMURegisters.NAVX_REG_WHOAMI;
            final int NAVX_REGISTER_RAW_FIRST  = IMURegisters.NAVX_REG_GYRO_X_L;
            DimI2cDeviceReader navxReader[] = new DimI2cDeviceReader[3];
            DimI2cDeviceWriter navxWriter  = null;
            I2cDevice navXDevice        = null;
            int iteration_count = 0;

            byte[][] navx_data = new byte[3][];
            byte[] write_buffer = new byte[1];
            byte[] empty_data = new byte[DIM_MAX_I2C_READ_LEN];
            write_buffer[0] = (byte)update_rate_hz;

            navXDevice = new I2cDevice(dim, dim_port);
            navxWriter = new DimI2cDeviceWriter(navXDevice,NAVX_I2C_DEV_ADDRESS << 1,
                                                0x80 | IMURegisters.NAVX_REG_UPDATE_RATE_HZ, write_buffer);

            while (!navxWriter.isDone()) {
                try {
                    Thread.sleep(5);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            while ( keep_running ) {
                try {
                    int new_update_bytes = 0;
                    if ( request_zero_yaw ) {
                        write_buffer[0] = AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_YAW;
                        navxWriter = new DimI2cDeviceWriter(navXDevice,NAVX_I2C_DEV_ADDRESS << 1,
                                0x80 | IMURegisters.NAVX_REG_INTEGRATION_CTL, write_buffer);
                        while (!navxWriter.isDone()) {
                            try {
                                Thread.sleep(5);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                        request_zero_yaw = false;
                    }
                    while (!navXDevice.isI2cPortReady()) {
                        Thread.sleep(5);
                    }

                    /* Read 1 Data (always) */

                    navxReader[0] = new DimI2cDeviceReader(navXDevice, NAVX_I2C_DEV_ADDRESS << 1,
                            NAVX_REGISTER_FIRST, DIM_MAX_I2C_READ_LEN);
                    while (!navxReader[0].isDone()) {
                        Thread.sleep(5);
                    }
                    navx_data[0] = navxReader[0].getReadBuffer();
                    if ( navx_data[0] != null ) {
                        new_update_bytes += navx_data[0].length;
                    }

                    if ( ( data_type == DeviceDataType.kProcessedData ) ||
                            (data_type == DeviceDataType.kBoth )) {
                        navxReader[1] = new DimI2cDeviceReader(navXDevice, NAVX_I2C_DEV_ADDRESS << 1,
                                NAVX_REGISTER_FIRST + DIM_MAX_I2C_READ_LEN, DIM_MAX_I2C_READ_LEN);
                        while (!navxReader[1].isDone()) {
                            Thread.sleep(5);
                        }
                        navx_data[1] = navxReader[1].getReadBuffer();
                        if (navx_data[1] != null) {
                            new_update_bytes += navx_data[1].length;
                        }
                    } else {
                        navx_data[1] = empty_data;
                    }

                    if ( ( data_type == DeviceDataType.kRawData ) ||
                            (data_type == DeviceDataType.kBoth )) {
                        navxReader[2] = new DimI2cDeviceReader(navXDevice, NAVX_I2C_DEV_ADDRESS << 1,
                                NAVX_REGISTER_RAW_FIRST, DIM_MAX_I2C_READ_LEN);
                        while (!navxReader[2].isDone()) {
                            Thread.sleep(5);
                        }
                        navx_data[2] = navxReader[2].getReadBuffer();
                        if (navx_data[2] != null) {
                            new_update_bytes += navx_data[2].length;
                        }
                    } else {
                        navx_data[2] = empty_data;
                    }

                    this.byte_count += new_update_bytes;
                    if ( new_update_bytes > 0 ) {
                        this.update_count++;
                        if ( navx_data[0][IMURegisters.NAVX_REG_WHOAMI] == 50 ) {
                            this.is_connected = true;
                        } else {
                            navx_data[0] =
                                navx_data[1] =
                                navx_data[2] = empty_data;
                            this.is_connected = false;
                        }
                        decodeNavxBank1Data(navx_data[0], 0, navx_data[0].length);
                        decodeNavxBank2Data(navx_data[1], DIM_MAX_I2C_READ_LEN, navx_data[1].length);
                        decodeNavxBank3Data(navx_data[2], NAVX_REGISTER_RAW_FIRST, navx_data[2].length);
                    }
                } catch (Exception ex) {
                    navx_data[0] = empty_data;
                    navx_data[1] = empty_data;
                    navx_data[2] = empty_data;
                }
            }

            navXDevice.close();
        }

        void decodeNavxBank1Data(byte[] curr_data, int first_address, int len) {
            board_id.hw_rev                 = curr_data[IMURegisters.NAVX_REG_HW_REV - first_address];
            board_id.fw_ver_major           = curr_data[IMURegisters.NAVX_REG_FW_VER_MAJOR - first_address];
            board_id.fw_ver_minor           = curr_data[IMURegisters.NAVX_REG_FW_VER_MINOR - first_address];
            board_id.type                   = curr_data[IMURegisters.NAVX_REG_WHOAMI - first_address];

            board_state.gyro_fsr_dps        = AHRSProtocol.decodeBinaryUint16(curr_data,IMURegisters.NAVX_REG_GYRO_FSR_DPS_L - first_address);
            board_state.accel_fsr_g         = (short)curr_data[IMURegisters.NAVX_REG_ACCEL_FSR_G - first_address];
            board_state.update_rate_hz      = curr_data[IMURegisters.NAVX_REG_UPDATE_RATE_HZ - first_address];
            board_state.capability_flags    = AHRSProtocol.decodeBinaryUint16(curr_data,IMURegisters.NAVX_REG_CAPABILITY_FLAGS_L - first_address);

            long timestamp_low, timestamp_high;
            long sensor_timestamp;

            timestamp_low = (long)AHRSProtocol.decodeBinaryUint16(curr_data, IMURegisters.NAVX_REG_TIMESTAMP_L_L-first_address);
            timestamp_high = (long)AHRSProtocol.decodeBinaryUint16(curr_data, IMURegisters.NAVX_REG_TIMESTAMP_H_L-first_address);
            sensor_timestamp               = (timestamp_high << 16) + timestamp_low;
            ahrspos_update.op_status       = curr_data[IMURegisters.NAVX_REG_OP_STATUS - first_address];
            ahrspos_update.selftest_status = curr_data[IMURegisters.NAVX_REG_SELFTEST_STATUS - first_address];
            ahrspos_update.cal_status      = curr_data[IMURegisters.NAVX_REG_CAL_STATUS - first_address];
            ahrspos_update.sensor_status   = curr_data[IMURegisters.NAVX_REG_SENSOR_STATUS_L - first_address];
            ahrspos_update.yaw             = AHRSProtocol.decodeProtocolSignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_YAW_L-first_address);
            ahrspos_update.pitch           = AHRSProtocol.decodeProtocolSignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_PITCH_L-first_address);
        }

        void decodeNavxBank2Data(byte[] curr_data, int first_address, int len) {
            ahrspos_update.roll            = AHRSProtocol.decodeProtocolSignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_ROLL_L-first_address);
            ahrspos_update.compass_heading = AHRSProtocol.decodeProtocolUnsignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_HEADING_L-first_address);
            ahrspos_update.mpu_temp        = AHRSProtocol.decodeProtocolSignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_MPU_TEMP_C_L - first_address);
            ahrspos_update.linear_accel_x  = AHRSProtocol.decodeProtocolSignedThousandthsFloat(curr_data, IMURegisters.NAVX_REG_LINEAR_ACC_X_L-first_address);
            ahrspos_update.linear_accel_y  = AHRSProtocol.decodeProtocolSignedThousandthsFloat(curr_data, IMURegisters.NAVX_REG_LINEAR_ACC_Y_L-first_address);
            ahrspos_update.linear_accel_z  = AHRSProtocol.decodeProtocolSignedThousandthsFloat(curr_data, IMURegisters.NAVX_REG_LINEAR_ACC_Z_L-first_address);
            ahrspos_update.altitude        = AHRSProtocol.decodeProtocol1616Float(curr_data, IMURegisters.NAVX_REG_ALTITUDE_D_L - first_address);
            ahrspos_update.fused_heading   = AHRSProtocol.decodeProtocolUnsignedHundredthsFloat(curr_data, IMURegisters.NAVX_REG_FUSED_HEADING_L-first_address);
            ahrspos_update.quat_w          = AHRSProtocol.decodeBinaryInt16(curr_data, IMURegisters.NAVX_REG_QUAT_W_L-first_address);
            ahrspos_update.quat_x          = AHRSProtocol.decodeBinaryInt16(curr_data, IMURegisters.NAVX_REG_QUAT_X_L-first_address);
            ahrspos_update.quat_y          = AHRSProtocol.decodeBinaryInt16(curr_data, IMURegisters.NAVX_REG_QUAT_Y_L-first_address);
            ahrspos_update.quat_z          = AHRSProtocol.decodeBinaryInt16(curr_data, IMURegisters.NAVX_REG_QUAT_Z_L-first_address);
        }

        void decodeNavxBank3Data( byte[] curr_data, int first_address, int len ) {
            raw_data_update.gyro_x      = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_GYRO_X_L-first_address);
            raw_data_update.gyro_y      = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_GYRO_Y_L-first_address);
            raw_data_update.gyro_z      = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_GYRO_Z_L-first_address);
            raw_data_update.accel_x     = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_ACC_X_L-first_address);
            raw_data_update.accel_y     = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_ACC_Y_L-first_address);
            raw_data_update.accel_z     = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_ACC_Z_L-first_address);
            raw_data_update.mag_x       = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_MAG_X_L-first_address);
            raw_data_update.mag_y       = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_MAG_Y_L-first_address);
            raw_data_update.mag_z       = AHRSProtocol.decodeBinaryInt16(curr_data,  IMURegisters.NAVX_REG_MAG_Z_L-first_address);
        }

    }

    public class DimI2cDeviceWriter {
        private final I2cDevice device;
        private final int dev_address;
        private final int mem_address;
        private boolean done;

        public DimI2cDeviceWriter(I2cDevice i2cDevice, int i2cAddress, int memAddress, byte[] data) {
            this.device = i2cDevice;
            this.dev_address = i2cAddress;
            this.mem_address = memAddress;
            done = false;
            i2cDevice.copyBufferIntoWriteBuffer(data);
            i2cDevice.enableI2cWriteMode(i2cAddress, memAddress, data.length);
            i2cDevice.setI2cPortActionFlag();
            i2cDevice.writeI2cCacheToController();
            i2cDevice.registerForI2cPortReadyCallback(new I2cController.I2cPortReadyCallback() {
                public void portIsReady(int port) {
                    DimI2cDeviceWriter.this.portDone();
                }
            });
        }

        public boolean isDone() {
            return this.done;
        }

        private void portDone() {
            if ( device.isI2cPortReady() ) {
                device.deregisterForPortReadyCallback();
                done = true;
            }
        }
    }

    public class DimI2cDeviceReader {
        private final I2cDevice device;
        private final int dev_address;
        private final int mem_address;
        private boolean i2c_transaction_complete;
        private boolean buffer_read_complete;
        private byte[] device_data;

        public DimI2cDeviceReader(I2cDevice i2cDevice, int i2cAddress, int memAddress, int num_bytes) {
            this.device = i2cDevice;
            this.dev_address = i2cAddress;
            this.mem_address = memAddress;
            device_data = null;
            i2c_transaction_complete = false;
            buffer_read_complete = false;
            i2cDevice.enableI2cReadMode(i2cAddress, memAddress, num_bytes);
            i2cDevice.setI2cPortActionFlag();
            i2cDevice.writeI2cCacheToController();
            i2cDevice.registerForI2cPortReadyCallback(new I2cController.I2cPortReadyCallback() {
                public void portIsReady(int port) {
                    DimI2cDeviceReader.this.portDone();
                }
            });
        }

        public boolean isDone() {
            return this.i2c_transaction_complete && buffer_read_complete;
        }

        private void portDone() {
            if (!i2c_transaction_complete && device.isI2cPortReady()) {
                i2c_transaction_complete = true;
                device.readI2cCacheFromController();
            }
            else if (i2c_transaction_complete) {
                device_data = this.device.getCopyOfReadBuffer();
                device.deregisterForPortReadyCallback();
                buffer_read_complete = true;
            }
        }

        public byte[] getReadBuffer() {
            return device_data;
        }
    }
}
