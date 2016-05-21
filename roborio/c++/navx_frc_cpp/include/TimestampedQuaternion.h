/*
 * TimestampedQuaternion.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_TIMESTAMPED_QUATERNION_H_
#define SRC_TIMESTAMPED_QUATERNION_H_

#include <stdint.h>
#include <Quaternion.h>

class TimestampedQuaternion : public Quaternion{

protected:
    long timestamp;
    bool valid;
    bool interpolated;

public:
	TimestampedQuaternion();
    TimestampedQuaternion(Quaternion& src, long timestamp);
    TimestampedQuaternion(TimestampedQuaternion& src);

    bool IsValid();
    long GetTimestamp();

    void Set(float w, float x, float y, float z, long timestamp);
    void Set(Quaternion& src, long timestamp);
    void Set(TimestampedQuaternion& src);

    bool GetInterpolated();
    void SetInterpolated( bool interpolated );
};

#endif /* SRC_TIMESTAMPED_QUATERNION_H_ */
