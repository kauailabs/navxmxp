/*
 * ContinuousAngleTracker.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include "ContinuousAngleTracker.h"

ContinuousAngleTracker::ContinuousAngleTracker() {
	Init();
    angleAdjust = 0.0f;
}

void ContinuousAngleTracker::Init() {
    gyro_prevVal = 0.0;
    ctrRollOver  = 0;
    fFirstUse = true;
    last_yaw_angle = 0.0f;
    curr_yaw_angle = 0.0f;
}

void ContinuousAngleTracker::NextAngle( float newAngle ) {
	std::unique_lock<std::mutex> sync(tracker_mutex);
	last_yaw_angle = curr_yaw_angle;
	curr_yaw_angle = newAngle;
}

/* Invoked (internally) whenever yaw reset occurs. */
void ContinuousAngleTracker::Reset() {
	std::unique_lock<std::mutex> sync(tracker_mutex);
	Init();
}


double ContinuousAngleTracker::GetAngle() {
	// First case
	// Old reading: +150 degrees
	// New reading: +170 degrees
	// Difference:  (170 - 150) = +20 degrees

	// Second case
	// Old reading: -20 degrees
	// New reading: -50 degrees
	// Difference : (-50 - -20) = -30 degrees

	// Third case
	// Old reading: +179 degrees
	// New reading: -179 degrees
	// Difference:  (-179 - 179) = -358 degrees

	// Fourth case
	// Old reading: -179  degrees
	// New reading: +179 degrees
	// Difference:  (+179 - -179) = +358 degrees

	double difference;
	double gyroVal;
	double yawVal;

	{
		std::unique_lock<std::mutex> sync(tracker_mutex);

		yawVal = curr_yaw_angle;

		// Has gyro_prevVal been previously set?
		// If not, return do not calculate, return current value
		if( !fFirstUse )
		{
			// Determine count for rollover counter
			difference = yawVal - gyro_prevVal;

			/* Clockwise past +180 degrees
			 * If difference > 180*, increment rollover counter */
			if( difference < -180.0 ) {
				ctrRollOver++;

			/* Counter-clockwise past -180 degrees:
			 * If difference > 180*, decrement rollover counter */
			}
			else if ( difference > 180.0 ) {
				ctrRollOver--;
			}
		}

		// Mark gyro_prevVal as being used
		fFirstUse = false;

		// Calculate value to return back to calling function
		// e.g. +720 degrees or -360 degrees
		gyroVal = yawVal + (360.0 * ctrRollOver);
		gyro_prevVal = yawVal;

		return gyroVal + angleAdjust;
	}
}

void ContinuousAngleTracker::SetAngleAdjustment(double adjustment) {
	angleAdjust = adjustment;
}

double ContinuousAngleTracker::GetAngleAdjustment() {
	return angleAdjust;
}

double ContinuousAngleTracker::GetRate() {
	float difference;
	{
		std::unique_lock<std::mutex> sync(tracker_mutex);
		difference = curr_yaw_angle - last_yaw_angle;
	}
	if ( difference > 180.0f) {
		/* Clockwise past +180 degrees */
		difference = 360.0f - difference;
	} else if ( difference < -180.0f) {
		/* Counter-clockwise past -180 degrees */
		difference = 360.0f + difference;
	}
	return difference;
}
