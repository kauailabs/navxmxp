/*
 * Quaternion.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include <Quaternion.h>
#include <math.h>

Quaternion::Quaternion() {
	Set(0,0,0,0);
}

Quaternion::Quaternion(Quaternion& src) {
	Set(src);
}

Quaternion::Quaternion(float w, float x, float y, float z) {
	Set(w,x,y,z);
}

void Quaternion::Set(float w, float x, float y, float z) {
	this->w = w;
	this->x = x;
	this->y = y;
	this->z = z;
}

void Quaternion::Set(Quaternion& src) {
	Set(src.w, src.x, src.y, src.z);
}

void Quaternion::GetGravity(FloatVectorStruct& v, const Quaternion& q) {
    v.x = 2 * (q.x*q.z - q.w*q.y);
    v.y = 2 * (q.w*q.x + q.y*q.z);
    v.z = q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z;
}

void Quaternion::GetYawPitchRoll(const Quaternion& q, const FloatVectorStruct& gravity, FloatVectorStruct& ypr) {
    // yaw: (about Z axis)
    ypr.x = (float)atan2(2*q.x*q.y - 2*q.w*q.z, 2*q.w*q.w + 2*q.x*q.x - 1);
    // pitch: (node up/down, about X axis)
    ypr.y = (float)atan(gravity.y / sqrt(gravity.x*gravity.x + gravity.z*gravity.z));
    // roll: (tilt left/right, about Y axis)
    ypr.z = (float)atan(gravity.x / sqrt(gravity.y*gravity.y + gravity.z*gravity.z));
}

void Quaternion::GetYawPitchRoll(FloatVectorStruct& ypr) {
	FloatVectorStruct gravity;
	GetGravity(ypr,*this);
	GetYawPitchRoll(*this,gravity,ypr);
}

float Quaternion::GetYaw() {
	FloatVectorStruct ypr;
	GetYawPitchRoll(ypr);
	return ypr.x;
}

float Quaternion::GetPitch() {
	FloatVectorStruct ypr;
	GetYawPitchRoll(ypr);
	return ypr.y;
}

float Quaternion::GetRoll() {
	FloatVectorStruct ypr;
	GetYawPitchRoll(ypr);
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
void Quaternion::slerp(const Quaternion& qa, const Quaternion& qb, double t, Quaternion& qm) {
	// Calculate angle between them.
	double cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (fabs(cosHalfTheta) >= 1.0){
		qm.w = qa.w;
		qm.x = qa.x;
		qm.y = qa.y;
		qm.z = qa.z;
		return;
	}
	// Calculate temporary values.
	double halfTheta = acos(cosHalfTheta);
	double sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001){
		qm.w = (qa.w * 0.5f + qb.w * 0.5f);
		qm.x = (qa.x * 0.5f + qb.x * 0.5f);
		qm.y = (qa.y * 0.5f + qb.y * 0.5f);
		qm.z = (qa.z * 0.5f + qb.z * 0.5f);
		return;
	}
	float ratioA = (float)(sin((1 - t) * halfTheta) / sinHalfTheta);
	float ratioB = (float)(sin(t * halfTheta) / sinHalfTheta);
	//calculate Quaternion.
	qm.w = (qa.w * ratioA + qb.w * ratioB);
	qm.x = (qa.x * ratioA + qb.x * ratioB);
	qm.y = (qa.y * ratioA + qb.y * ratioB);
	qm.z = (qa.z * ratioA + qb.z * ratioB);
	return;
}

