/*
 * TimestampedQuaternion.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include <TimestampedQuaternion.h>

TimestampedQuaternion::TimestampedQuaternion() :
	Quaternion()
{
	this->timestamp = 0;
	this->valid = false;
	this->interpolated = false;
}

TimestampedQuaternion::TimestampedQuaternion(Quaternion& src, long timestamp) :
	Quaternion(src)
{
	this->timestamp = timestamp;
	this->valid = true;
	this->interpolated = false;
}

TimestampedQuaternion::TimestampedQuaternion(TimestampedQuaternion& src) :
	Quaternion(src)
{
	this->timestamp = src.timestamp;
	this->valid = src.valid;
	this->interpolated = src.interpolated;
}

bool TimestampedQuaternion::IsValid() {
	return valid;
}

long TimestampedQuaternion::GetTimestamp() {
	return timestamp;
}

void TimestampedQuaternion::Set(float w, float x, float y, float z, long timestamp) {
	Quaternion::Set(w,x,y,z);
	this->timestamp = timestamp;
	this->valid = true;
}

void TimestampedQuaternion::Set(Quaternion& src, long timestamp) {
	Quaternion::Set(src);
	this->timestamp = timestamp;
	this->valid = true;
}

void TimestampedQuaternion::Set(TimestampedQuaternion& src) {
	Quaternion::Set(src);
	this->timestamp = src.timestamp;
	this->valid = src.valid;
}

bool TimestampedQuaternion::GetInterpolated() {
	return interpolated;
}

void TimestampedQuaternion::SetInterpolated( bool interpolated ) {
	this->interpolated = interpolated;
}
