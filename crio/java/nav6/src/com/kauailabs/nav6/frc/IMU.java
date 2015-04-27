/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2013. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the nav6_License.txt file*/ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/

package com.kauailabs.nav6.frc;

import com.kauailabs.nav6.IMUProtocol;
import com.sun.squawk.util.Arrays;
import edu.wpi.first.wpilibj.PIDSource;
import edu.wpi.first.wpilibj.SensorBase;
import edu.wpi.first.wpilibj.livewindow.LiveWindowSendable;
import edu.wpi.first.wpilibj.tables.ITable;
import edu.wpi.first.wpilibj.visa.VisaException;
import edu.wpi.first.wpilibj.Timer;

/**
 * The IMU class provides a simplified interface to the KauaiLabs nav6 IMU.
 * 
 * The IMU class enables access to basic connectivity and state information, 
 * as well as key orientation information (yaw, pitch, roll, compass heading).
 * 
 * Advanced capabilities of the nav6 IMU may be accessed via the IMUAdvanced 
 * class.
 * @author Scott
 */
public class IMU extends SensorBase implements PIDSource, LiveWindowSendable, Runnable {

    static final int    YAW_HISTORY_LENGTH      = 10;
    static final byte   DEFAULT_UPDATE_RATE_HZ  = 100;
    static final short  DEFAULT_ACCEL_FSR_G     = 2;
    static final short  DEFAULT_GYRO_FSR_DPS    = 2000;
    
    BufferingSerialPort serial_port;
    float yaw_history[];
    int next_yaw_history_index;
    double user_yaw_offset;
    ITable m_table;
    Thread m_thread;
    protected byte update_rate_hz;

    volatile float yaw;
    volatile float pitch;
    volatile float roll;
    volatile float compass_heading;
    volatile int update_count = 0;
    volatile int byte_count = 0;
    volatile float nav6_yaw_offset_degrees;
    volatile short accel_fsr_g;
    volatile short gyro_fsr_dps;
    volatile short flags;    

    double last_update_time;
    boolean stop = false;
    private IMUProtocol.YPRUpdate ypr_update_data;
    protected byte update_type = IMUProtocol.MSGID_YPR_UPDATE;
    
    /**
     * Constructs the IMU class, overriding the default update rate
     * with a custom rate which may be from 4 to 100, representing
     * the number of updates per second sent by the nav6 IMU.  
     * 
     * Note that increasing the update rate may increase the 
     * CPU utilization.
     * @param serial_port BufferingSerialPort object to use
     * @param update_rate_hz Custom Update Rate (Hz)
     */
    public IMU(BufferingSerialPort serial_port, byte update_rate_hz) {
        ypr_update_data = new IMUProtocol.YPRUpdate();
        this.update_rate_hz = update_rate_hz;
        flags = 0;
        accel_fsr_g = DEFAULT_ACCEL_FSR_G;
        gyro_fsr_dps = DEFAULT_GYRO_FSR_DPS;
        this.serial_port = serial_port;
        yaw_history = new float[YAW_HISTORY_LENGTH];
        yaw = (float) 0.0;
        pitch = (float) 0.0;
        roll = (float) 0.0;
        try {
            serial_port.reset();
        } catch (VisaException ex) {
            ex.printStackTrace();
        }
        initIMU();
        m_thread = new Thread(this);
        m_thread.start();        
    }
    
    /**
     * Constructs the IMU class, using the default update rate.  
     * 
     * @param serial_port BufferingSerialPort object to use
     */
    public IMU(BufferingSerialPort serial_port) {
        this(serial_port,DEFAULT_UPDATE_RATE_HZ);
    }

    protected void initIMU() {
        
        // The nav6 IMU serial port configuration is 8 data bits, no parity, one stop bit. 
        // No flow control is used.
        // Conveniently, these are the defaults used by the WPILib's SerialPort class.
        //
        // In addition, the WPILib's SerialPort class also defaults to:
        //
        // Timeout period of 5 seconds
        // Termination ('\n' character)
        // Transmit immediately

        initializeYawHistory();
        user_yaw_offset = 0;

        // set the nav6 into the desired update mode
	byte stream_command_buffer[] = new byte[256];
	int packet_length = IMUProtocol.encodeStreamCommand( stream_command_buffer, update_type, update_rate_hz ); 
        try {
            serial_port.write( stream_command_buffer, packet_length );
        } catch (VisaException ex) {
        }
    }

    protected void setStreamResponse( IMUProtocol.StreamResponse response ) {
        
        flags = response.flags;
        nav6_yaw_offset_degrees = response.yaw_offset_degrees;
        accel_fsr_g = response.accel_fsr_g;
        gyro_fsr_dps = response.gyro_fsr_dps;
        update_rate_hz = (byte)response.update_rate_hz;
    }
        
    private void initializeYawHistory() {

        Arrays.fill(yaw_history,0);
        next_yaw_history_index = 0;
        last_update_time = 0.0;
    }

    private void setYawPitchRoll(float yaw, float pitch, float roll, float compass_heading) {

        this.yaw = yaw;
        this.pitch = pitch;
        this.roll = roll;
        this.compass_heading = compass_heading;

        updateYawHistory(this.yaw);
    }

    protected void updateYawHistory(float curr_yaw) {

        if (next_yaw_history_index >= YAW_HISTORY_LENGTH) {
            next_yaw_history_index = 0;
        }
        yaw_history[next_yaw_history_index] = curr_yaw;
        last_update_time = Timer.getFPGATimestamp();
        next_yaw_history_index++;
    }

    private double getAverageFromYawHistory() {

        double yaw_history_sum = 0.0;
        for (int i = 0; i < YAW_HISTORY_LENGTH; i++) {
            yaw_history_sum += yaw_history[i];
        }
        double yaw_history_avg = yaw_history_sum / YAW_HISTORY_LENGTH;
        return yaw_history_avg;
    }

    /**
     * Returns the current pitch value (in degrees, from -180 to 180)
     * reported by the nav6 IMU.
     * @return The current pitch value in degrees (-180 to 180).
     */
        public float getPitch() {
        return pitch;
    }

    /**
     * Returns the current roll value (in degrees, from -180 to 180)
     * reported by the nav6 IMU.
     * @return The current roll value in degrees (-180 to 180).
     */
    public float getRoll() {
        return roll;
    }

    /**
     * Returns the current yaw value (in degrees, from -180 to 180)
     * reported by the nav6 IMU.
     * 
     * Note that the returned yaw value will be offset by a user-specified
     * offset value; this user-specified offset value is set by 
     * invoking the zeroYaw() method.
     * @return The current yaw value in degrees (-180 to 180).
     */
    public float getYaw() {
        float calculated_yaw = (float) (this.yaw - user_yaw_offset);
        if (calculated_yaw < -180) {
            calculated_yaw += 360;
        }
        if (calculated_yaw > 180) {
            calculated_yaw -= 360;
        }
        return calculated_yaw;
    }

    /**
     * Returns the current tilt-compensated compass heading 
     * value (in degrees, from 0 to 360) reported by the nav6 IMU.
     * 
     * Note that this value is sensed by the nav6 magnetometer,
     * which can be affected by nearby magnetic fields (e.g., the
     * magnetic fields generated by nearby motors).
     * @return The current tilt-compensated compass heading, in degrees (0-360).
     */
    public float getCompassHeading() {
        return compass_heading;
    }

    /**
     * Sets the user-specified yaw offset to the current
     * yaw value reported by the nav6 IMU.
     * 
     * This user-specified yaw offset is automatically
     * subtracted from subsequent yaw values reported by
     * the getYaw() method.
     */
    public void zeroYaw() {
        user_yaw_offset = getAverageFromYawHistory();
    }

    /**
     * Indicates whether the nav6 IMU is currently connected
     * to the host computer.  A connection is considered established
     * whenever a value update packet has been received from the
     * nav6 IMU within the last second.
     * @return Returns true if a valid update has been received within the last second.
     */
    public boolean isConnected() {
        double time_since_last_update = Timer.getFPGATimestamp() - this.last_update_time;
        return time_since_last_update <= 1.0;
    }

    /**
     * Returns the count in bytes of data received from the
     * nav6 IMU.  This could can be useful for diagnosing 
     * connectivity issues.
     * 
     * If the byte count is increasing, but the update count
     * (see getUpdateCount()) is not, this indicates a software
     * misconfiguration.
     * @return The number of bytes received from the nav6 IMU.
     */
    public double getByteCount() {
        return byte_count;
    }

    /**
     * Returns the count of valid update packets which have
     * been received from the nav6 IMU.  This count should increase
     * at the same rate indicated by the configured update rate.
     * @return The number of valid updates received from the nav6 IMU.
     */
    public double getUpdateCount() {
        return update_count;
    }

    /**
     * Returns true if the nav6 IMU is currently performing automatic
     * calibration.  Automatic calibration occurs when the nav6 IMU
     * is initially powered on, during which time the nav6 IMU should
     * be held still.
     * 
     * During this automatically calibration, the yaw, pitch and roll
     * values returned may not be accurate.
     * 
     * Once complete, the nav6 IMU will automatically remove an internal
     * yaw offset value from all reported values.
     * @return Returns true if the nav6 IMU is currently calibrating.
     */
    public boolean isCalibrating() {
        short calibration_state = (short)(this.flags & IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
        return (calibration_state != IMUProtocol.NAV6_CALIBRATION_STATE_COMPLETE);
    }

    /**
     * Returns the current yaw value reported by the nav6 IMU.  This
     * yaw value is useful for implementing features including "auto rotate 
     * to a known angle".
     * @return The current yaw angle in degrees (-180 to 180).
     */
    public double pidGet() {
        return getYaw();
    }

    public void updateTable() {
        if (m_table != null) {
            m_table.putNumber("Value", getYaw());
        }
    }

    public void startLiveWindowMode() {
    }

    public void stopLiveWindowMode() {
    }

    public void initTable(ITable itable) {
        m_table = itable;
        updateTable();
    }

    public ITable getTable() {
        return m_table;
    }

    public String getSmartDashboardType() {
        return "Gyro";
    }

    // Invoked when a new packet is received; returns the packet length if the packet 
    // is valid, based upon IMU Protocol definitions; otherwise, returns 0
    
    protected int decodePacketHandler(byte[] received_data, int offset, int bytes_remaining) {
        
        int packet_length = IMUProtocol.decodeYPRUpdate(received_data, offset, bytes_remaining, ypr_update_data);
        if (packet_length > 0) {
            setYawPitchRoll(ypr_update_data.yaw,ypr_update_data.pitch,ypr_update_data.roll,ypr_update_data.compass_heading);
        }
        return packet_length;
    }
    
    // IMU Class thread run method
    
    public void run() {

        stop = false;
        boolean stream_response_received = false;
        double last_stream_command_sent_timestamp = 0.0;
        try {
            serial_port.setReadBufferSize(512);
            serial_port.setTimeout(1.0);
            serial_port.enableTermination('\n');
            serial_port.flush();
            serial_port.reset();
        } catch (VisaException ex) {
            ex.printStackTrace();
        }
                
        IMUProtocol.StreamResponse response = new IMUProtocol.StreamResponse();

        byte[] stream_command = new byte[256];
        
	int cmd_packet_length = IMUProtocol.encodeStreamCommand( stream_command, update_type, update_rate_hz ); 
        try {
            serial_port.reset();
            serial_port.write( stream_command, cmd_packet_length );
            serial_port.flush();
            last_stream_command_sent_timestamp = Timer.getFPGATimestamp();
        } catch (VisaException ex) {
        }
        
        while (!stop) {
            try {

                // Wait, with delays to conserve CPU resources, until
                // bytes have arrived.
                
                while ( !stop && ( serial_port.getBytesReceived() < 1 ) ) {
                    Timer.delay(0.1);
                }

                int packets_received = 0;
                byte[] received_data = serial_port.read(256);
                int bytes_read = received_data.length;
                if (bytes_read > 0) {
                    byte_count += bytes_read;
                    int i = 0;
                    // Scan the buffer looking for valid packets
                    while (i < bytes_read) {
                                                
                        // Attempt to decode a packet
                        
                        int bytes_remaining = bytes_read - i;
                        int packet_length = decodePacketHandler(received_data,i,bytes_remaining);
                        if (packet_length > 0) {
                            packets_received++;
                            update_count++;
                            i += packet_length;
                        } 
                        else 
                        {
                            packet_length = IMUProtocol.decodeStreamResponse(received_data, i, bytes_remaining, response);
                            if (packet_length > 0) {
                                packets_received++;
                                setStreamResponse(response);
                                stream_response_received = true;
                                i += packet_length;
                            }
                            else {
                                // current index is not the start of a valid packet; increment
                                i++;
                            }
                        }
                    }
                
                    if ( ( packets_received == 0 ) && ( bytes_read == 256 ) ) {
                        // Workaround for issue found in Java SerialPort implementation:
                        // No packets received and 256 bytes received; this
                        // condition occurs in the Java SerialPort.  In this case,
                        // reset the serial port.
                        serial_port.reset();
                    }
                    
                    // If a stream configuration response has not been received within three seconds
                    // of operation, (re)send a stream configuration request
                    
                    if ( !stream_response_received && ((Timer.getFPGATimestamp() - last_stream_command_sent_timestamp ) > 3.0 ) ) {
                        cmd_packet_length = IMUProtocol.encodeStreamCommand( stream_command, update_type, update_rate_hz ); 
                        try {
                            last_stream_command_sent_timestamp = Timer.getFPGATimestamp();
                            serial_port.write( stream_command, cmd_packet_length );
                            serial_port.flush();
                        } catch (VisaException ex2) {
                        }                                                    
                    }
                    else {                        
                        // If no bytes remain in the buffer, and not awaiting a response, sleep a bit
                        if ( stream_response_received && ( serial_port.getBytesReceived() == 0 ) ) {
                            Timer.delay(1.0/update_rate_hz);
                        }        
                    }
                }
            } catch (VisaException ex) {
                // This exception typically indicates a Timeout
                stream_response_received = false;
            }
        }
    }
}