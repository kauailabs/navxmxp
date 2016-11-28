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

import java.lang.Math;

public class Quaternion {

    private float w;
    private float x;
    private float y;
    private float z;

    class FloatVectorStruct {
        float x;
        float y;
        float z;
    };    
    
    public Quaternion() {
    	set(0,0,0,0);
    }
    
    public Quaternion(Quaternion src) {
    	set(src);
    }
    
    public Quaternion(float w, float x, float y, float z) {
    	set(w, x, y, z);
    }
    
    public void set(float w, float x, float y, float z) {
        this.w = w;
        this.x = x;
        this.y = y;
        this.z = z;    	
    }
    
    public void set(Quaternion src) {
    	set(src.w, src.x, src.y, src.z);  	
    }

    static void getGravity(FloatVectorStruct v, Quaternion q) {
        v.x = 2 * (q.x*q.z - q.w*q.y);
        v.y = 2 * (q.w*q.x + q.y*q.z);
        v.z = q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z;
    }

    static void getYawPitchRoll(Quaternion q, FloatVectorStruct gravity, FloatVectorStruct ypr) {
        // yaw: (about Z axis)
        ypr.x = (float)Math.atan2(2*q.x*q.y - 2*q.w*q.z, 2*q.w*q.w + 2*q.x*q.x - 1);
        // pitch: (node up/down, about X axis)
        ypr.y = (float)Math.atan(gravity.y / Math.sqrt(gravity.x*gravity.x + gravity.z*gravity.z));
        // roll: (tilt left/right, about Y axis)
        ypr.z = (float)Math.atan(gravity.x / Math.sqrt(gravity.y*gravity.y + gravity.z*gravity.z));
    }
    
    void getYawPitchRoll(FloatVectorStruct ypr) {
    	FloatVectorStruct gravity = new FloatVectorStruct();    	
    	getGravity(ypr,this);
    	getYawPitchRoll(this,gravity,ypr);    	
    }
    
    public float getYaw() {
    	FloatVectorStruct ypr = new FloatVectorStruct();
    	getYawPitchRoll(ypr);
    	return ypr.x;
    }
    
    public float getPitch() {
    	FloatVectorStruct ypr = new FloatVectorStruct();
    	getYawPitchRoll(ypr);
    	return ypr.y;
    }
    
    public float getRoll() {
    	FloatVectorStruct ypr = new FloatVectorStruct();
    	getYawPitchRoll(ypr);
    	return ypr.z;
    }
    
    /* Estimates an intermediate Quaternion given Quaternions representing each end of the path,
     * and an interpolation ratio from 0.0 t0 1.0.
     * 
     * Uses Quaternion SLERP (Spherical Linear Interpolation), an algorithm
     * originally introduced by Ken Shoemake in the context of quaternion interpolation for the 
     * purpose of animating 3D rotation. This estimation is based upon the assumption of
     * constant-speed motion along a unit-radius great circle arc.
     * 
     * For more info:
     * 
     * http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
     */
    public static Quaternion slerp(Quaternion qa, Quaternion qb, double t) {
    	// quaternion to return
    	Quaternion qm = new Quaternion();
    	// Calculate angle between them.
    	double cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
    	// if qa=qb or qa=-qb then theta = 0 and we can return qa
    	if (Math.abs(cosHalfTheta) >= 1.0){
    		qm.w = qa.w;
    		qm.x = qa.x;
    		qm.y = qa.y;
    		qm.z = qa.z;
    		return qm;
    	}
    	// Calculate temporary values.
    	double halfTheta = Math.acos(cosHalfTheta);
    	double sinHalfTheta = Math.sqrt(1.0 - cosHalfTheta*cosHalfTheta);
    	// if theta = 180 degrees then result is not fully defined
    	// we could rotate around any axis normal to qa or qb
    	if (Math.abs(sinHalfTheta) < 0.001){ 
    		qm.w = (qa.w * 0.5f + qb.w * 0.5f);
    		qm.x = (qa.x * 0.5f + qb.x * 0.5f);
    		qm.y = (qa.y * 0.5f + qb.y * 0.5f);
    		qm.z = (qa.z * 0.5f + qb.z * 0.5f);
    		return qm;
    	}
    	float ratioA = (float)(Math.sin((1 - t) * halfTheta) / sinHalfTheta);
    	float ratioB = (float)(Math.sin(t * halfTheta) / sinHalfTheta); 
    	//calculate Quaternion.
    	qm.w = (qa.w * ratioA + qb.w * ratioB);
    	qm.x = (qa.x * ratioA + qb.x * ratioB);
    	qm.y = (qa.y * ratioA + qb.y * ratioB);
    	qm.z = (qa.z * ratioA + qb.z * ratioB);
    	return qm;
    }    
}
