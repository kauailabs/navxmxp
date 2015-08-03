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

import edu.wpi.first.wpilibj.SPI;
import edu.wpi.first.wpilibj.Timer;

class RegisterIO_SPI implements IRegisterIO{

    SPI port;
    int bitrate;
    
    static final int   DEFAULT_SPI_BITRATE_HZ         = 500000;
    
    public RegisterIO_SPI( SPI spi_port ) {
        port = spi_port;
        bitrate = DEFAULT_SPI_BITRATE_HZ;
    }
    
    public RegisterIO_SPI( SPI spi_port, int bitrate ) {
        port = spi_port;
        this.bitrate = bitrate;
    }
    
    @Override
    public boolean init() {
        port.setClockRate(bitrate);
        port.setMSBFirst();
        port.setSampleDataOnFalling();
        port.setClockActiveLow();
        port.setChipSelectActiveLow();
        return true;
    }

    @Override
    public boolean write(byte address, byte value ) {
        byte[] cmd = new byte[3];
        cmd[0] = (byte) (address  | (byte)0x80);
        cmd[1] = value;
        cmd[2] = AHRSProtocol.getCRC(cmd, 2);
        if ( port.write(cmd, cmd.length) != cmd.length) {
            return false; // WRITE ERROR
        }
        return true;
    }

    @Override
    public boolean read(byte first_address, byte[] buffer) {
        byte[] cmd = new byte[3];
        cmd[0] = first_address;
        cmd[1] = (byte)buffer.length;
        cmd[2] = AHRSProtocol.getCRC(cmd, 2);
        if ( port.write(cmd, cmd.length) != cmd.length ) {
            return false; // WRITE ERROR
        }
        // delay 200 us /* TODO:  What is min. granularity of delay()? */
        Timer.delay(0.001);
        byte[] received_data = new byte[buffer.length+1];
        if ( port.read(true, received_data, received_data.length) != received_data.length ) {
            return false; // READ ERROR
        }
        byte crc = AHRSProtocol.getCRC(received_data, received_data.length - 1);
        if ( crc != received_data[received_data.length-1] ) {
            return false; // CRC ERROR
        }
        System.arraycopy(received_data, 0, buffer, 0, received_data.length - 1);
        return true;
    }

    @Override
    public boolean shutdown() {
        return true;
    }

}
