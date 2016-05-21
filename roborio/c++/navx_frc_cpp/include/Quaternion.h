/*
 * Quaternion.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_QUATERNION_H_
#define SRC_QUATERNION_H_

#include <stdint.h>

class Quaternion {

protected:
    float w;
    float x;
    float y;
    float z;

public:

    class FloatVectorStruct {

    public:

    	float x;
        float y;
        float z;
    };

    Quaternion();
    Quaternion(Quaternion& src);
    Quaternion(float w, float x, float y, float z);

    void Set(float w, float x, float y, float z);
    void Set(Quaternion& src);

    static void GetGravity(FloatVectorStruct& v, const Quaternion& q);
    static void GetYawPitchRoll(const Quaternion& q, const FloatVectorStruct& gravity, FloatVectorStruct& ypr);

    void GetYawPitchRoll(FloatVectorStruct& ypr);

    float GetYaw();
    float GetPitch();
    float GetRoll();

    /* Estimates an intermediate Quaternion given Quaternions representing each end of the path,
     * and an interpolation ratio from 0.0 t0 1.0.
     */
    static void slerp(const Quaternion& qa, const Quaternion& qb, double t, Quaternion& out);

};

#endif /* SRC_QUATERNION_H_ */
