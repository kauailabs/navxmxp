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

interface IIOProvider {
    public boolean  isConnected();
    public double   getByteCount();
    public double   getUpdateCount();
    public void     setUpdateRateHz(byte update_rate);
    public void     zeroYaw();
    public void     zeroDisplacement();
    public void     run();
    public void     stop();
}
