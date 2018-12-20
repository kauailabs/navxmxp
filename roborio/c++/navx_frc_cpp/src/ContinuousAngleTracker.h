/*
 * ContinuousAngleTracker.h
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#ifndef SRC_CONTINUOUSANGLETRACKER_H_
#define SRC_CONTINUOUSANGLETRACKER_H_

#include "wpi/priority_mutex.h"

using namespace wpi;

class ContinuousAngleTracker {
private:
    bool fFirstUse;
    double gyro_prevVal;
    int ctrRollOver;
    float curr_yaw_angle;
    float last_yaw_angle;
    double angleAdjust;
    std::mutex tracker_mutex;

    void Init();

public:
    ContinuousAngleTracker();
    void Reset();
    void NextAngle( float newAngle );
    double GetAngle();
    double GetRate();
	void SetAngleAdjustment(double adjustment);
	double GetAngleAdjustment();
};

#endif /* SRC_CONTINUOUSANGLETRACKER_H_ */
