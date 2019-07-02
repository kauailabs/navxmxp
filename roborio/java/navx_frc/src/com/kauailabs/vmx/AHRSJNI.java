/*----------------------------------------------------------------------------*/
/* Copyright (c) 2016-2018 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package com.kauailabs.vmx;

public class AHRSJNI {
  public static native void	Init(byte update_rate_hz);
  public static native void	ZeroYaw();
  public static native boolean	IsConnected();
  public static native double	GetByteCount();
  public static native double	GetUpdateCount();
  public static native void	ResetDisplacement();
  public static native boolean  BlockOnNewCurrentRegisterData(int timeout_ms, 
								byte[] first_reg_addr_out, 
								byte[] data_out, 
								byte requested_len, 
								byte[] len_out);
  public static native boolean  ReadConfigurationData(byte first_reg, 
							byte[] data_out, 
							byte requested_len);
}
