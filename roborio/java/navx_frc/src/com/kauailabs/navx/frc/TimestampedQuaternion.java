/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2016. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
package com.kauailabs.navx.frc;

public class TimestampedQuaternion extends Quaternion {

    long timestamp;
    boolean valid;
    boolean interpolated;
    
    public TimestampedQuaternion() {
    	super();
    	timestamp = 0;
    	valid = false;
    	this.interpolated = false;
    }
    
    public TimestampedQuaternion(Quaternion src, long timestamp) {
    	super(src);
    	this.timestamp = timestamp;
    	valid = true;
    	this.interpolated = false;
    }
    
    public TimestampedQuaternion(TimestampedQuaternion src) {
    	super(src);
    	this.timestamp = src.timestamp;
    	this.valid = src.valid;
    	this.interpolated = src.interpolated;
    }
    
    public boolean isValid() {
    	return valid;
    }
    
    public long getTimestamp() {
    	return timestamp;
    }
    
    public void set(float w, float x, float y, float z, long timestamp) {
    	this.set(w,x,y,z);
    	this.timestamp = timestamp;
    	this.valid = true;
    }
    
    public void set(Quaternion src, long timestamp) {
    	this.set(src);
    	this.timestamp = timestamp;
    	this.valid = true;
    }
    
    public boolean getInterpolated() {
    	return interpolated;
    }
    
    public void setInterpolated( boolean interpolated ) {
    	this.interpolated = interpolated;
    }
}
