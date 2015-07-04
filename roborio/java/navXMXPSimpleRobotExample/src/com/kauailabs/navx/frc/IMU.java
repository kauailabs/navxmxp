/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2013. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Thunderchicken!           */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the LICENSE file         */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/

package com.kauailabs.navx.frc;

import java.util.Arrays;

import com.kauailabs.navx.AHRSProtocol;
import com.kauailabs.navx.IMUProtocol;

import edu.wpi.first.wpilibj.PIDSource;
import edu.wpi.first.wpilibj.SensorBase;
import edu.wpi.first.wpilibj.SerialPort;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.livewindow.LiveWindowSendable;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;
import edu.wpi.first.wpilibj.tables.ITable;

/**
 * The IMU class provides a simplified interface to the KauaiLabs navX IMU.
 * 
 * The IMU class enables access to basic connectivity and state information, 
 * as well as key orientation information (yaw, pitch, roll, compass heading).
 * 
 * Advanced capabilities of the navX IMU may be accessed via the IMUAdvanced 
 * class.
 * @author Scott
 */
public class IMU extends SensorBase implements PIDSource, LiveWindowSendable, Runnable {

    static final int    YAW_HISTORY_LENGTH      = 10;
    static final byte   DEFAULT_UPDATE_RATE_HZ  = 100;
    static final short  DEFAULT_ACCEL_FSR_G     = 2;
    static final short  DEFAULT_GYRO_FSR_DPS    = 2000;

    SerialPort.Port serial_port_id;
    SerialPort serial_port;
    float yaw_history[];
    int next_yaw_history_index;
    double user_yaw_offset;
    ITable m_table;
    Thread m_thread;
    protected byte update_rate_hz;

    protected volatile float yaw;
    protected volatile float pitch;
    protected volatile float roll;
    protected volatile float compass_heading;
    protected volatile int yaw_crossing_count;
    protected volatile int yaw_last_direction;
    protected volatile float last_yaw_rate;
    volatile int update_count = 0;
    volatile int byte_count = 0;
    volatile float navX_yaw_offset_degrees;
    volatile short accel_fsr_g;
    volatile short gyro_fsr_dps;
    volatile short flags;    

    double last_update_time;
    boolean stop = false;
    private IMUProtocol.YPRUpdate ypr_update_data;
    protected byte update_type = IMUProtocol.MSGID_YPR_UPDATE;

    private byte next_integration_control_action = 0;
    private boolean signal_transmit_integration_control = false;
    private boolean signal_retransmit_stream_config = false;

    static public class BoardAxis
    {
        public static final byte BOARD_AXIS_X = 0;
        public static final byte BOARD_AXIS_Y = 1;
        public static final byte BOARD_AXIS_Z = 2;
    }

    static public class BoardYawAxis
    {
        public byte board_axis;
        public boolean up;
    };

    protected SerialPort resetSerialPort()
    {
        if (serial_port != null) {
            try {
                serial_port.free();
            } catch (Exception ex) {
                // This has been seen to happen before....
            }
            serial_port = null;
        }
        serial_port = getMaybeCreateSerialPort();
        return serial_port;
    }

    protected SerialPort getMaybeCreateSerialPort()
    {
        if (serial_port == null) {
            try {
                serial_port = new SerialPort(57600, serial_port_id);
                serial_port.setReadBufferSize(256);
                serial_port.setTimeout(1.0);
                serial_port.enableTermination('\n');
                serial_port.reset();
            } catch (Exception ex) {
                /* Error opening serial port. Perhaps it doesn't exist... */
                serial_port = null;
            }
        }
        return serial_port;
    }

    /**
     * Constructs the IMU class, overriding the default update rate
     * with a custom rate which may be from 4 to 100, representing
     * the number of updates per second sent by the navX IMU.  
     * 
     * Note that increasing the update rate may increase the 
     * CPU utilization.
     * @param serial_port BufferingSerialPort object to use
     * @param update_rate_hz Custom Update Rate (Hz)
     */
    public IMU(SerialPort.Port serial_port_id, byte update_rate_hz) {
        ypr_update_data = new IMUProtocol.YPRUpdate();
        this.update_rate_hz = update_rate_hz;
        flags = 0;
        accel_fsr_g = DEFAULT_ACCEL_FSR_G;
        gyro_fsr_dps = DEFAULT_GYRO_FSR_DPS;
        this.serial_port_id = serial_port_id;
        yaw_history = new float[YAW_HISTORY_LENGTH];
        yaw = (float) 0.0;
        pitch = (float) 0.0;
        roll = (float) 0.0;
        yaw_crossing_count = 0;
        yaw_last_direction = 0;
        last_yaw_rate = 0.0f;
        try {
            serial_port = getMaybeCreateSerialPort();    		
            serial_port.reset();
        } catch (RuntimeException ex) {
            serial_port = null;
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
    public IMU(SerialPort.Port serial_port_id) {
        this(serial_port_id,DEFAULT_UPDATE_RATE_HZ);
    }

    protected void initIMU() {

        // The navX serial port configuration is 8 data bits, no parity, one stop bit. 
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

        // set the navX into the desired update mode
        byte stream_command_buffer[] = new byte[256];
        int packet_length = IMUProtocol.encodeStreamCommand( stream_command_buffer, update_type, update_rate_hz ); 
        try {
            serial_port.write( stream_command_buffer, packet_length );
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        }
    }

    protected void setStreamResponse( IMUProtocol.StreamResponse response )
    {    
        flags = response.flags;
        navX_yaw_offset_degrees = response.yaw_offset_degrees;
        accel_fsr_g = response.accel_fsr_g;
        gyro_fsr_dps = response.gyro_fsr_dps;
        update_rate_hz = (byte)response.update_rate_hz;
        /* If AHRSPOS is update type is request, but board doesn't support it, */
        /* retransmit the stream config, falling back to AHRS Update mode.     */
        if ( this.update_type == AHRSProtocol.MSGID_AHRSPOS_UPDATE ) {
            if ( !isDisplacementSupported() ) {
                this.update_type = AHRSProtocol.MSGID_AHRS_UPDATE;
                signal_retransmit_stream_config = true;
            }
        }
    }

    protected boolean isOmniMountSupported()
    {
        return (((flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_OMNIMOUNT) !=0) ? true : false);
    }

    boolean isBoardYawResetSupported()
    {
        return (((flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_YAW_RESET) != 0) ? true : false);
    }

    protected boolean isDisplacementSupported()
    {
        return (((flags & AHRSProtocol.NAVX_CAPABILITY_FLAG_VEL_AND_DISP) != 0) ? true : false);
    }

    protected void enqueueIntegrationControlMessage(byte action)
    {
        next_integration_control_action = action;
        signal_transmit_integration_control = true;
    }

    public void getBoardYawAxis( BoardYawAxis info)
    {
        short yaw_axis_info = (short)(flags >> 3);
        yaw_axis_info &= 7;
        if ( yaw_axis_info == AHRSProtocol.OMNIMOUNT_DEFAULT) {
            info.board_axis = BoardAxis.BOARD_AXIS_Z;
            info.up = true;
        } else {
            info.board_axis = (byte)yaw_axis_info;
            info.up = (((info.board_axis & 0x01) != 0) ? true : false);
            info.board_axis >>= 1;
        }
    }

    private void initializeYawHistory() {

        Arrays.fill(yaw_history,0);
        next_yaw_history_index = 0;
        last_update_time = 0.0;
    }

    protected void setYawPitchRoll(float yaw, float pitch, float roll, float compass_heading) {

        float last_offset_corrected_yaw = getYaw();

        float last_yaw = this.yaw + 180.0f;
        float curr_yaw = yaw + 180.0f;
        float delta_yaw = curr_yaw - last_yaw;
        this.last_yaw_rate = delta_yaw;

        yaw_last_direction = 0;
        if ( curr_yaw < last_yaw ) {
            if ( delta_yaw < 180.0f ) {
                yaw_last_direction = -1;
            } else {
                yaw_last_direction = 1;
            }
        } else if ( curr_yaw > last_yaw ) {
            if ( delta_yaw > 180.0f ) {
                yaw_last_direction = -1;
            } else {
                yaw_last_direction = 1;
            }
        }

        this.yaw = yaw;
        this.pitch = pitch;
        this.roll = roll;
        this.compass_heading = compass_heading;

        updateYawHistory(this.yaw);

        float curr_offset_corrected_yaw = getYaw();

        if ( yaw_last_direction < 0 ) {
            if ( ( curr_offset_corrected_yaw < 0.0f ) && ( last_offset_corrected_yaw >= 0.0f ) ) {
                yaw_crossing_count--;
            }    		
        } else if ( yaw_last_direction > 0 ) {
            if ( ( curr_offset_corrected_yaw >= 0.0f ) && ( last_offset_corrected_yaw < 0.0f ) ) {
                yaw_crossing_count++;
            }    		
        }


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
     * reported by the navX MXP.
     * @return The current pitch value in degrees (-180 to 180).
     */
    public float getPitch() {
        return pitch;
    }

    /**
     * Returns the current roll value (in degrees, from -180 to 180)
     * reported by the navX MXP.
     * @return The current roll value in degrees (-180 to 180).
     */
    public float getRoll() {
        return roll;
    }

    /**
     * Returns the current yaw value (in degrees, from -180 to 180)
     * reported by the navX MXP.
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
     * Returns the total accumulated yaw angle (Z Axis, in degrees)
     * reported by the navX MXP.
     * 
     * The angle is continuous, that is can go beyond 360 degrees. This make algorithms that wouldn't
     * want to see a discontinuity in the gyro output as it sweeps past 0 on the second time around.
     *
     * Note that the returned yaw value will be offset by a user-specified
     * offset value; this user-specified offset value is set by 
     * invoking the zeroYaw() method.
     *
     * @return the current heading of the robot in degrees. This heading is based on integration
     * of the returned rate from the gyro.
     */

    public double getAngle() {
        double accumulated_yaw_angle = (double)yaw_crossing_count * 360.0f;
        double curr_yaw = getYaw();
        if ( curr_yaw < 0.0f ) {
            curr_yaw += 360.0f;
        }
        accumulated_yaw_angle += curr_yaw;
        return accumulated_yaw_angle;
    }

    /**
     * Return the rate of rotation of the gyro.
     * 
     * The rate is based on the most recent reading of the gyro analog value.
     * 
     * @return the current rate in degrees per second
     */

    public double getRate() {
        return last_yaw_rate * (float)update_rate_hz;    	
    }

    /**
     * Returns the current tilt-compensated compass heading 
     * value (in degrees, from 0 to 360) reported by the navX MXP IMU.
     * 
     * Note that this value is sensed by the navX MXP magnetometer,
     * which can be affected by nearby magnetic fields (e.g., the
     * magnetic fields generated by nearby motors).
     * @return The current tilt-compensated compass heading, in degrees (0-360).
     */
    public float getCompassHeading() {
        return compass_heading;
    }

    /**
     * Sets the user-specified yaw offset to the current
     * yaw value reported by the navX MXP.
     * 
     * This user-specified yaw offset is automatically
     * subtracted from subsequent yaw values reported by
     * the getYaw() method.
     */
    public void zeroYaw() {

        if ( isBoardYawResetSupported() ) {
            /* navX MXP supports on-board yaw offset reset */
            enqueueIntegrationControlMessage( AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_YAW );
        } else {
            user_yaw_offset = getAverageFromYawHistory();
        }
        yaw_crossing_count = 0;
    }

    /**
     * Reset the gyro.
     *
     * Resets the gyro Z (Yaw) axis to a heading of zero. This can be used if there is significant 
     * drift in the gyro and it needs to be recalibrated after it has been running.
     */

    public void reset() {
        zeroYaw();
    }

    /**
     * Indicates whether the navX MXP is currently connected
     * to the host computer.  A connection is considered established
     * whenever a value update packet has been received from the
     * navX MXP within the last second.
     * @return Returns true if a valid update has been received within the last second.
     */
    public boolean isConnected() {
        double time_since_last_update = Timer.getFPGATimestamp() - this.last_update_time;
        return time_since_last_update <= 1.0;
    }

    /**
     * Returns the count in bytes of data received from the
     * navX MXP.  This could can be useful for diagnosing 
     * connectivity issues.
     * 
     * If the byte count is increasing, but the update count
     * (see getUpdateCount()) is not, this indicates a software
     * misconfiguration.
     * @return The number of bytes received from the navX MXP.
     */
    public double getByteCount() {
        return byte_count;
    }

    /**
     * Returns the count of valid update packets which have
     * been received from the navX MXP.  This count should increase
     * at the same rate indicated by the configured update rate.
     * @return The number of valid updates received from the navX MXP.
     */
    public double getUpdateCount() {
        return update_count;
    }

    /**
     * Returns true if the navX MXP is currently performing automatic
     * calibration.  Automatic calibration occurs when the navX MXP
     * is initially powered on, during which time the navX MXP should
     * be held still.
     * 
     * During this automatically calibration, the yaw, pitch and roll
     * values returned may not be accurate.
     * 
     * Once complete, the navX MXP will automatically remove an internal
     * yaw offset value from all reported values.
     * @return Returns true if the navX MXP is currently calibrating.
     */
    public boolean isCalibrating() {
        short calibration_state = (short)(this.flags & IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
        return (calibration_state != IMUProtocol.NAV6_CALIBRATION_STATE_COMPLETE);
    }

    /**
     * Returns the current yaw value reported by the navX IMU.  This
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
        double last_valid_packet_time = 0.0;
        int partial_binary_packet_count = 0;
        int stream_response_receive_count = 0;
        int timeout_count = 0;
        int discarded_bytes_count = 0;
        int port_reset_count = 0;
        double last_stream_command_sent_timestamp = 0.0;
        int updates_in_last_second = 0;
        double last_data_received_timestamp = 0;
        int integration_response_receive_count = 0;
        double last_second_start_time = 0;

        try {
            serial_port.setReadBufferSize(256);
            serial_port.setTimeout(1.0);
            serial_port.enableTermination('\n');
            serial_port.flush();
            serial_port.reset();
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        }

        IMUProtocol.StreamResponse response = new IMUProtocol.StreamResponse();

        byte[] stream_command = new byte[256];
        byte[] integration_control_command = new byte[256];
        AHRSProtocol.IntegrationControl integration_control = new AHRSProtocol.IntegrationControl();
        AHRSProtocol.IntegrationControl integration_control_response = new AHRSProtocol.IntegrationControl();

        int cmd_packet_length = IMUProtocol.encodeStreamCommand( stream_command, update_type, update_rate_hz ); 
        try {
            serial_port.reset();
            serial_port.write( stream_command, cmd_packet_length );
            serial_port.flush();
            port_reset_count++;
            SmartDashboard.putNumber("navX_PortResets", (double)port_reset_count);
            last_stream_command_sent_timestamp = Timer.getFPGATimestamp();
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        }

        int remainder_bytes = 0;
        byte[] remainder_data = null;

        while (!stop) {
            try {

                // Wait, with delays to conserve CPU resources, until
                // bytes have arrived.

                if ( signal_transmit_integration_control ) {
                    integration_control.action = next_integration_control_action;
                    signal_transmit_integration_control = false;
                    next_integration_control_action = 0;
                    cmd_packet_length = AHRSProtocol.encodeIntegrationControlCmd( integration_control_command, integration_control );
                    try {
                        serial_port.write( integration_control_command, cmd_packet_length );
                    } catch (RuntimeException ex2) {
                        ex2.printStackTrace();
                    }
                }            	

                if ( !stop && ( remainder_bytes == 0 ) && ( serial_port.getBytesReceived() < 1 ) ) {
                    Timer.delay(1.0/update_rate_hz);
                }

                int packets_received = 0;
                byte[] received_data = serial_port.read(256);
                int bytes_read = received_data.length;
                byte_count += bytes_read;

                /* If a partial packet remains from last iteration, place that at  */
                /* the start of the data buffer, and append any new data available */
                /* at the serial port.                                             */

                if ( remainder_bytes > 0 ) {
                    byte[] resized_array = new byte[remainder_bytes + bytes_read];
                    System.arraycopy(remainder_data, 0, resized_array, 0, remainder_bytes);
                    System.arraycopy(received_data, 0, resized_array, remainder_bytes, bytes_read);
                    received_data = resized_array;
                    bytes_read += remainder_bytes;
                    remainder_bytes = 0;
                    remainder_data = null;
                }

                if (bytes_read > 0) {
                    last_data_received_timestamp = Timer.getFPGATimestamp();
                    int i = 0;
                    // Scan the buffer looking for valid packets
                    while (i < bytes_read) {

                        // Attempt to decode a packet

                        int bytes_remaining = bytes_read - i;

                        if ( received_data[i] != IMUProtocol.PACKET_START_CHAR ) {
                            /* Skip over received bytes until a packet start is detected. */
                            i++;
                            discarded_bytes_count++;
                            SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                            continue;
                        } else {
                            if ( ( bytes_remaining > 2 ) && 
                                    ( received_data[i+1] == AHRSProtocol.BINARY_PACKET_INDICATOR_CHAR ) ) {
                                /* Binary packet received; next byte is packet length-2 */
                                byte total_expected_binary_data_bytes = received_data[i+2];
                                total_expected_binary_data_bytes += 2;
                                while ( bytes_remaining < total_expected_binary_data_bytes ) {

                                    /* This binary packet contains an embedded     */
                                    /* end-of-line character.  Continue to receive */
                                    /* more data until entire packet is received.  */
                                    byte[] additional_received_data = serial_port.read(256);
                                    byte_count += additional_received_data.length;
                                    bytes_remaining += additional_received_data.length;

                                    /* Resize array to hold existing and new data */
                                    byte[] c = new byte[received_data.length + additional_received_data.length];
                                    if ( c.length > 0 ) {
                                        System.arraycopy(received_data, 0, c, 0, received_data.length);
                                        System.arraycopy(additional_received_data, 0, c, received_data.length, additional_received_data.length);
                                        received_data = c;
                                    } else {
                                        /* Timeout waiting for remainder of binary packet */
                                        i++;
                                        bytes_remaining--;
                                        partial_binary_packet_count++;
                                        SmartDashboard.putNumber("navX Partial Binary Packets", (double)partial_binary_packet_count);
                                        continue;
                                    }
                                }
                            }
                        }

                        int packet_length = decodePacketHandler(received_data,i,bytes_remaining);
                        if (packet_length > 0) {
                            packets_received++;
                            update_count++;
                            last_valid_packet_time = Timer.getFPGATimestamp();
                            updates_in_last_second++;
                            if ((last_valid_packet_time - last_second_start_time ) > 1.0 ) {
                                SmartDashboard.putNumber("navX UpdatesPerSec", (double)updates_in_last_second);
                                updates_in_last_second = 0;
                                last_second_start_time = last_valid_packet_time;
                            }
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
                                stream_response_receive_count++;
                                SmartDashboard.putNumber("navX Stream Responses", (double)stream_response_receive_count);                                
                            }
                            else {
                                packet_length = AHRSProtocol.decodeIntegrationControlResponse( received_data, i, bytes_remaining,
                                        integration_control_response );
                                if ( packet_length > 0 ) {
                                    // Confirmation of integration control
                                    integration_response_receive_count++;
                                    SmartDashboard.putNumber("IntegrationControlRxCount", integration_response_receive_count);
                                    i += packet_length;
                                } else {
                                    /* Even though a start-of-packet indicator was found, the  */
                                    /* current index is not the start of a packet if interest. */
                                    /* Scan to the beginning of the next packet,               */
                                    boolean next_packet_start_found = false;
                                    int x;
                                    for ( x = 0; x < bytes_remaining; x++ ) {
                                        if ( received_data[i + x] != IMUProtocol.PACKET_START_CHAR) {
                                            x++;
                                        } else {
                                            i += x;
                                            bytes_remaining -= x;
                                            if ( x != 0 ) {
                                                next_packet_start_found = true;
                                            }
                                            break;
                                        }
                                    }
                                    boolean discard_remainder = false;
                                    if ( !next_packet_start_found && x == bytes_remaining ) {
                                        /* Remaining bytes don't include a start-of-packet */
                                        discard_remainder = true;
                                    }
                                    boolean partial_packet = false;
                                    if ( discard_remainder ) {
                                        /* Discard the remainder */
                                        i = bytes_remaining;
                                    } else {            						
                                        if ( !next_packet_start_found ) {
                                            /* This occurs when packets are received that are not decoded.   */
                                            /* Bump over this packet and prepare for the next.               */
                                            if ( ( bytes_remaining > 2 ) && 
                                                    ( received_data[i+1] == AHRSProtocol.BINARY_PACKET_INDICATOR_CHAR ) ) {
                                                /* Binary packet received; next byte is packet length-2 */
                                                int pkt_len = received_data[i+2];
                                                pkt_len += 2;
                                                if ( bytes_remaining >= pkt_len ) {
                                                    bytes_remaining -= pkt_len;
                                                    i += pkt_len;
                                                    discarded_bytes_count += pkt_len;
                                                    SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                                                } else {
                                                    /* This is the initial portion of a partial binary packet. */
                                                    /* Keep this data and attempt to acquire the remainder.    */
                                                    partial_packet = true;
                                                }
                                            } else {
                                                /* Ascii packet received. */
                                                /* Scan up to and including next end-of-packet character       */
                                                /* sequence, or the beginning of a new packet.                 */
                                                for ( x = 0; x < bytes_remaining; x++ ) {
                                                    if ( received_data[i+x] == (byte)'\r') {
                                                        i += x+1;
                                                        bytes_remaining -= (x+1);
                                                        discarded_bytes_count += x+1;
                                                        if ( ( bytes_remaining > 0 ) &&  received_data[i] == (byte)'\n') {
                                                            bytes_remaining--;
                                                            i++;
                                                            discarded_bytes_count++;
                                                        }
                                                        SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                                                        break;
                                                    }
                                                    /* If a new start-of-packet is found, discard */
                                                    /* the ascii packet bytes that precede it.    */
                                                    if ( received_data[i+x] == (byte)'!') {
                                                        if ( x > 0 ) {
                                                            i += x;
                                                            bytes_remaining -= x;
                                                            discarded_bytes_count += x;
                                                            break;
                                                        } else {
                                                            /* start of packet found, but no termination     */
                                                            /* Time to get some more data, unless the bytes  */
                                                            /* remaining are larger than a valid packet size */
                                                            if ( bytes_remaining < IMUProtocol.IMU_PROTOCOL_MAX_MESSAGE_LENGTH ) {
                                                                /* Get more data */
                                                                partial_packet = true;
                                                            } else {
                                                                i++;
                                                                bytes_remaining--;
                                                            }
                                                            break;
                                                        }
                                                    }
                                                }
                                                if ( x == bytes_remaining ) {
                                                    /* Partial ascii packet - keep the remainder */
                                                    partial_packet = true;
                                                }
                                            }
                                        }
                                    }
                                    if ( partial_packet ) {
                                        remainder_data = new byte[bytes_remaining];
                                        System.arraycopy(received_data,i,remainder_data,0,bytes_remaining);
                                        remainder_bytes = bytes_remaining;
                                        i = bytes_read;
                                    }
                                }
                            }
                        }
                    }

                    if ( ( packets_received == 0 ) && ( bytes_read == 256 ) ) {
                        // Workaround for issue found in SerialPort implementation:
                        // No packets received and 256 bytes received; this
                        // condition occurs in the SerialPort.  In this case,
                        // reset the serial port.
                        serial_port.flush();
                        serial_port.reset();
                        port_reset_count++;                        
                        SmartDashboard.putNumber("navX_PortResets", (double)port_reset_count);
                    }

                    boolean retransmit_stream_config = false;
                    if ( signal_retransmit_stream_config ) {
                        retransmit_stream_config = true;
                        signal_retransmit_stream_config = false;
                    }

                    // If a stream configuration response has not been received within three seconds
                    // of operation, (re)send a stream configuration request

                    if ( retransmit_stream_config ||
                            (!stream_response_received && ((Timer.getFPGATimestamp() - last_stream_command_sent_timestamp ) > 3.0 ) ) ) {
                        cmd_packet_length = IMUProtocol.encodeStreamCommand( stream_command, update_type, update_rate_hz ); 
                        try {
                            resetSerialPort();
                            last_stream_command_sent_timestamp = Timer.getFPGATimestamp();
                            serial_port.write( stream_command, cmd_packet_length );
                            serial_port.flush();
                        } catch (RuntimeException ex2) {
                            ex2.printStackTrace();
                        }                                                    
                    }
                    else {                        
                        // If no bytes remain in the buffer, and not awaiting a response, sleep a bit
                        if ( stream_response_received && ( serial_port.getBytesReceived() == 0 ) ) {
                            Timer.delay(1.0/update_rate_hz);
                        }        
                    }

                    /* If receiving data, but no valid packets have been received in the last second */
                    /* the navX MXP may have been reset, but no exception has been detected.         */
                    /* In this case , trigger transmission of a new stream_command, to ensure the    */
                    /* streaming packet type is configured correctly.                                */

                    if ( ( Timer.getFPGATimestamp() - last_valid_packet_time ) > 1.0 ) {
                        last_stream_command_sent_timestamp = 0.0;
                        stream_response_received = false;
                    }
                } else {
                    /* No data received this time around */
                    if ( Timer.getFPGATimestamp() - last_data_received_timestamp  > 1.0 ) {
                        resetSerialPort();
                    }                	
                }
            } catch (RuntimeException ex) {
                // This exception typically indicates a Timeout, but can also be a buffer overrun error.
                stream_response_received = false;
                timeout_count++;
                SmartDashboard.putNumber("navX Serial Port Timeouts", (double)timeout_count);
                ex.printStackTrace();
                SmartDashboard.putString("LastNavException", ex.getMessage() + "; " + ex.toString());
                resetSerialPort();
            }
        }
    }
}