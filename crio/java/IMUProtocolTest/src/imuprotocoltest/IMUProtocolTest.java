/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package imuprotocoltest;

import com.kauailabs.nav6.IMUProtocol;
import java.nio.charset.Charset;

/**
 *
 * @author Scott
 */
public class IMUProtocolTest {

    private static final String ypr_update_message = "!y-000.10 006.64 001.92 352.11C8\r\n";
    private static final String raw_update_message = "!qEBDE0624EBA238E7FE5EDD522B02FF1DFFF8FE77 031.846B\r\n";
    private static final String stream_response_message = "!sr07D000020064 001.973FDC027FFCDDFEC8000274\r\n";

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here

        // Create a Stream Command
        byte[] protocol_buffer = new byte[IMUProtocol.IMU_PROTOCOL_MAX_MESSAGE_LENGTH];
        int length = IMUProtocol.encodeStreamCommand(protocol_buffer, (byte) IMUProtocol.STREAM_CMD_STREAM_TYPE_GYRO,(byte)100);
        if (length != 0) {
            System.out.println(protocol_buffer);
            IMUProtocol.StreamCommand c = new IMUProtocol.StreamCommand();
            int decode_length = IMUProtocol.decodeStreamCommand(protocol_buffer, 0, length, c);
            if (decode_length != 0) {
                System.out.print("Decoded Stream Command.  Stream type:  ");
                System.out.println((char) c.stream_type);
            } else {
                System.out.println("Error decoding Stream Command");
            }
        } else {
            System.out.println("Error encoding Stream Command");
        }

        // Decode YPR update message
        byte[] ypr_packet = ypr_update_message.getBytes(Charset.forName("UTF-8"));
        IMUProtocol.YPRUpdate ypr = new IMUProtocol.YPRUpdate();
        int decode_length = IMUProtocol.decodeYPRUpdate(ypr_packet, 0, ypr_packet.length, ypr);
        if (decode_length != 0) {
            System.out.print("Decoded YPR Update. ");
            System.out.print(" Yaw:");
            System.out.print(ypr.yaw);
            System.out.print(" Pitch:");
            System.out.print(ypr.pitch);
            System.out.print(" Roll:");
            System.out.print(ypr.roll);
            System.out.print(" Compass Heading:");
            System.out.print(ypr.compass_heading);
            System.out.println();
        } else {
            System.out.println("Error decoding YPR Update Message");
        }

        // Decode RAW update message
        byte[] raw_packet = raw_update_message.getBytes(Charset.forName("UTF-8"));
        IMUProtocol.QuaternionUpdate raw = new IMUProtocol.QuaternionUpdate();
        decode_length = IMUProtocol.decodeQuaternionUpdate(raw_packet, 0, raw_packet.length, raw);
        if (decode_length != 0) {
            System.out.print("Decoded Quaternion Update. ");
            System.out.print(" Q1:");
            System.out.print(raw.q1);
            System.out.print(" Q2:");
            System.out.print(raw.q2);
            System.out.print(" Q3:");
            System.out.print(raw.q3);
            System.out.print(" Q4:");
            System.out.print(raw.q4);
            System.out.print(" Accel X:");
            System.out.print(raw.accel_x);
            System.out.print(" Accel Y:");
            System.out.print(raw.accel_y);
            System.out.print(" Accel Z:");
            System.out.print(raw.accel_z);
            System.out.print(" Mag X:");
            System.out.print(raw.mag_x);
            System.out.print(" Mag Y:");
            System.out.print(raw.mag_y);
            System.out.print(" Mag Z:");
            System.out.print(raw.mag_z);
            System.out.println();
        } else {
            System.out.println("Error decoding Quaternion Update Message");
        }

        // Decode YPR update message
        byte[] stream_response_packet = stream_response_message.getBytes(Charset.forName("UTF-8"));
        IMUProtocol.StreamResponse response = new IMUProtocol.StreamResponse();
        decode_length = IMUProtocol.decodeStreamResponse(stream_response_packet, 0, stream_response_packet.length, response);
        if (decode_length != 0) {
            System.out.print("Decoded Stream Response. ");
            System.out.print(" Stream Type:  ");
            System.out.print((char) response.stream_type);
            System.out.print(" Gyro FSR (DPS):  ");
            System.out.print(response.gyro_fsr_dps);
            System.out.print(" Accel FSR (G):  ");
            System.out.print(response.accel_fsr_g);
            System.out.print(" Update Rate (Hz):  ");
            System.out.print(response.update_rate_hz);
            System.out.print(" Yaw Offset (Degrees):  ");
            System.out.print(response.yaw_offset_degrees);
            System.out.print(" Flags:  ");
            System.out.print(response.flags);
            System.out.print(" Offset Q1:  ");
            System.out.print(response.q1_offset);
            System.out.print(" Q2:  ");
            System.out.print(response.q2_offset);
            System.out.print(" Q3:  ");
            System.out.print(response.q3_offset);
            System.out.print(" Q4:  ");
            System.out.print(response.q4_offset);
            System.out.println();
        } else {
            System.out.println("Error decoding YPR Update Message");
        }

    }

}
