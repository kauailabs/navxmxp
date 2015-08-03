/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2015. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
package com.kauailabs.navx.frc;

import com.kauailabs.navx.AHRSProtocol;
import com.kauailabs.navx.IMUProtocol;

import edu.wpi.first.wpilibj.SerialPort;
import edu.wpi.first.wpilibj.Timer;
import edu.wpi.first.wpilibj.smartdashboard.SmartDashboard;

class SerialIO implements IIOProvider {

    SerialPort.Port serial_port_id;
    SerialPort serial_port;
    private byte next_integration_control_action;
    private boolean signal_transmit_integration_control;
    private boolean signal_retransmit_stream_config;
    private boolean stop;
    private byte update_type; //IMUProtocol.MSGID_XXX
    private byte update_rate_hz;
    int byte_count;
    int update_count;
    private IMUProtocol.YPRUpdate ypr_update_data;
    private IMUProtocol.GyroUpdate gyro_update_data;
    private AHRSProtocol.AHRSUpdate ahrs_update_data;
    private AHRSProtocol.AHRSPosUpdate ahrspos_update_data;
    private AHRSProtocol.BoardID board_id;
    IIOCompleteNotification notify_sink;
    IIOCompleteNotification.BoardState board_state;
    IBoardCapabilities board_capabilities;
    double last_valid_packet_time;

    final boolean debug = false; /* Set to true to enable debug output (to smart dashboard) */
    
    public SerialIO( SerialPort.Port port_id, byte update_rate_hz, boolean processed_data, IIOCompleteNotification notify_sink, IBoardCapabilities board_capabilities ) {
        this.serial_port_id = port_id;
        ypr_update_data = new IMUProtocol.YPRUpdate();
        gyro_update_data = new IMUProtocol.GyroUpdate();
        ahrs_update_data = new AHRSProtocol.AHRSUpdate();
        ahrspos_update_data = new AHRSProtocol.AHRSPosUpdate();
        board_id = new AHRSProtocol.BoardID();
        board_state = new IIOCompleteNotification.BoardState();
        this.notify_sink = notify_sink;
        this.board_capabilities = board_capabilities;
        serial_port = getMaybeCreateSerialPort();
        this.update_rate_hz = update_rate_hz;
        if ( processed_data ) {
            update_type = AHRSProtocol.MSGID_AHRSPOS_UPDATE;
        } else {
            update_type = IMUProtocol.MSGID_GYRO_UPDATE;
        }
    }
    
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

    protected void enqueueIntegrationControlMessage(byte action)
    {
        next_integration_control_action = action;
        signal_transmit_integration_control = true;
    }
    
    protected void dispatchStreamResponse(IMUProtocol.StreamResponse response) {
        board_state.cal_status = (byte) (response.flags & IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
        board_state.capability_flags = (short) (response.flags & ~IMUProtocol.NAV6_FLAG_MASK_CALIBRATION_STATE);
        board_state.op_status = 0x04; /* TODO:  Create a symbol for this */
        board_state.selftest_status = 0x07; /* TODO:  Create a symbol for this */
        board_state.accel_fsr_g = response.accel_fsr_g;
        board_state.gyro_fsr_dps = response.gyro_fsr_dps;
        board_state.update_rate_hz = (byte) response.update_rate_hz;
        notify_sink.setBoardState(board_state);
        /* If AHRSPOS is update type is request, but board doesn't support it, */
        /* retransmit the stream config, falling back to AHRS Update mode.     */
        if ( this.update_type == AHRSProtocol.MSGID_AHRSPOS_UPDATE ) {
            if ( !board_capabilities.isDisplacementSupported() ) {
                this.update_type = AHRSProtocol.MSGID_AHRS_UPDATE;
                signal_retransmit_stream_config = true;
            }
        }
    }
    
    protected int decodePacketHandler(byte[] received_data, int offset, int bytes_remaining) {

        int packet_length;
        
        if ( (packet_length = IMUProtocol.decodeYPRUpdate(received_data, offset, bytes_remaining, ypr_update_data)) > 0) {
            notify_sink.setYawPitchRoll(ypr_update_data);
        } else if ( ( packet_length = AHRSProtocol.decodeAHRSPosUpdate(received_data, offset, bytes_remaining, ahrspos_update_data)) > 0) {
            notify_sink.setAHRSPosData(ahrspos_update_data);
        } else if ( ( packet_length = AHRSProtocol.decodeAHRSUpdate(received_data, offset, bytes_remaining, ahrs_update_data)) > 0) {
            notify_sink.setAHRSData(ahrs_update_data);
        } else if ( ( packet_length = IMUProtocol.decodeGyroUpdate(received_data, offset, bytes_remaining, gyro_update_data)) > 0) {
            notify_sink.setRawData(gyro_update_data);
        } else if ( ( packet_length = AHRSProtocol.decodeBoardIDGetResponse(received_data, offset, bytes_remaining, board_id)) > 0) {
            notify_sink.setBoardID(board_id);
        } else {
            packet_length = 0;
        }
        return packet_length;
    }    
    
    public void run() {

        stop = false;
        boolean stream_response_received = false;
        double last_stream_command_sent_timestamp = 0.0;
        double last_data_received_timestamp = 0;
        double last_second_start_time = 0;

        int partial_binary_packet_count = 0;
        int stream_response_receive_count = 0;
        int timeout_count = 0;
        int discarded_bytes_count = 0;
        int port_reset_count = 0;
        int updates_in_last_second = 0;
        int integration_response_receive_count = 0;

        try {
            serial_port.setReadBufferSize(256);
            serial_port.setTimeout(1.0);
            serial_port.enableTermination('\n');
            serial_port.flush();
            serial_port.reset();
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        }

        byte[] stream_command = new byte[256];
        byte[] integration_control_command = new byte[256];
        IMUProtocol.StreamResponse response = new IMUProtocol.StreamResponse();
        AHRSProtocol.IntegrationControl integration_control = new AHRSProtocol.IntegrationControl();
        AHRSProtocol.IntegrationControl integration_control_response = new AHRSProtocol.IntegrationControl();

        int cmd_packet_length = IMUProtocol.encodeStreamCommand( stream_command, update_type, update_rate_hz ); 
        try {
            serial_port.reset();
            serial_port.write( stream_command, cmd_packet_length );
            cmd_packet_length = AHRSProtocol.encodeDataGetRequest( stream_command,  AHRSProtocol.AHRS_DATA_TYPE.BOARD_IDENTITY, (byte)0 ); 
            serial_port.write( stream_command, cmd_packet_length );
            serial_port.flush();
            port_reset_count++;
            if ( debug ) {
                SmartDashboard.putNumber("navX Port Resets", (double)port_reset_count);
            }
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
                            if ( debug ) {
                                SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                            }
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
                                        if ( debug ) {
                                            SmartDashboard.putNumber("navX Partial Binary Packets", (double)partial_binary_packet_count);
                                        }
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
                                if ( debug ) {
                                    SmartDashboard.putNumber("navX Updates Per Sec", (double)updates_in_last_second);
                                }
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
                                dispatchStreamResponse(response);
                                stream_response_received = true;
                                i += packet_length;
                                stream_response_receive_count++;
                                if ( debug ) {
                                    SmartDashboard.putNumber("navX Stream Responses", (double)stream_response_receive_count);                                
                                }
                            }
                            else {
                                packet_length = AHRSProtocol.decodeIntegrationControlResponse( received_data, i, bytes_remaining,
                                        integration_control_response );
                                if ( packet_length > 0 ) {
                                    // Confirmation of integration control
                                    integration_response_receive_count++;
                                    if ( debug ) {
                                        SmartDashboard.putNumber("navX Integration Control Response Count", integration_response_receive_count);
                                    }
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
                                                    if ( debug ) {
                                                        SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                                                    }
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
                                                        if ( debug ) {
                                                            SmartDashboard.putNumber("navX Discarded Bytes", (double)discarded_bytes_count);
                                                        }
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
                        if ( debug ) {
                            SmartDashboard.putNumber("navX Port Resets", (double)port_reset_count);
                        }
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
                            cmd_packet_length = AHRSProtocol.encodeDataGetRequest( stream_command,  AHRSProtocol.AHRS_DATA_TYPE.BOARD_IDENTITY, (byte)0 ); 
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
                if ( debug ) {
                    SmartDashboard.putNumber("navX Serial Port Timeout / Buffer Overrun", (double)timeout_count);
                    SmartDashboard.putString("navX Last Exception", ex.getMessage() + "; " + ex.toString());
                }
                ex.printStackTrace();
                resetSerialPort();
            }
        }
    }
    
    /**
     * Indicates whether the navX MXP is currently connected
     * to the host computer.  A connection is considered established
     * whenever a value update packet has been received from the
     * navX MXP within the last second.
     * @return Returns true if a valid update has been received within the last second.
     */
    public boolean isConnected() {
        double time_since_last_update = Timer.getFPGATimestamp() - this.last_valid_packet_time;
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

    @Override
    public void setUpdateRateHz(byte update_rate) {
        update_rate_hz = update_rate;
    }

    @Override
    public void zeroYaw() {
        enqueueIntegrationControlMessage(AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_YAW);
    }

    @Override
    public void zeroDisplacement() {
        enqueueIntegrationControlMessage( (byte)(AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_DISP_X |
                                                 AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_DISP_Y |
                                                 AHRSProtocol.NAVX_INTEGRATION_CTL_RESET_DISP_Z ) );
    }

    @Override
    public void stop() {
        stop = true;
    }
    
    
}
