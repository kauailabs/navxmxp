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

import edu.wpi.first.wpilibj.I2C;

class RegisterIO_I2C implements IRegisterIO{

    I2C port;
    
    public RegisterIO_I2C( I2C i2c_port ) {
        port = i2c_port;
    }
    
    @Override
    public boolean init() {
        return true;
    }

    @Override
    public boolean write(byte address, byte value ) {
        return port.write(address | 0x80, value);
    }

    final static int MAX_WPILIB_I2C_READ_BYTES = 127;
    
    @Override
    public boolean read(byte first_address, byte[] buffer) {
        int len = buffer.length;
        int buffer_offset = 0;
        while ( len > 0 ) {
            int read_len = (len > MAX_WPILIB_I2C_READ_BYTES) ? MAX_WPILIB_I2C_READ_BYTES : len;
            byte[] read_buffer = new byte[read_len];
            if (!port.write(first_address + buffer_offset, read_len) && 
                !port.readOnly(read_buffer, read_len) ) {
                System.arraycopy(read_buffer, 0,  buffer, buffer_offset, read_len);
                buffer_offset += read_len;
                len -= read_len;
            } else {
                break;
            }
        }
        return (len == 0);
    }

    @Override
    public boolean shutdown() {
        return true;
    }

}
