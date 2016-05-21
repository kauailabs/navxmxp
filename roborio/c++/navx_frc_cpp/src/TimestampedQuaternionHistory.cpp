/*
 * TimestampedQuaternionHistory.cpp
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#include <TimestampedQuaternionHistory.h>
#include <limits>

TimestampedQuaternionHistory::TimestampedQuaternionHistory(int num_samples) {
	history_size = num_samples;
	history = new TimestampedQuaternion[history_size];
	curr_index = 0;
	num_valid_samples = 0;
}

void TimestampedQuaternionHistory::Add(float w, float x, float y, float z, long new_timestamp) {

	std::lock_guard<std::mutex> lock(history_mutex);

	history[curr_index].Set(w, x, y, z, new_timestamp);
	if ( curr_index < (history_size - 1)) {
		curr_index++;
	} else {
		curr_index = 0;
	}
	if ( num_valid_samples < history_size ) {
		num_valid_samples++;
	}
}

bool TimestampedQuaternionHistory::Get( long requested_timestamp, TimestampedQuaternion& out ) {

	bool success = false;
	int initial_index = curr_index;
	long lowest_timestamp = std::numeric_limits<long>::max();
	int lowest_timestamp_index = -1;
	long highest_timestamp = std::numeric_limits<long>::min();
	int highest_timestamp_index = -1;

	std::lock_guard<std::mutex> lock(history_mutex);

	for ( int i = 0; i < num_valid_samples; i++ ) {
		long entry_timestamp = history[initial_index].GetTimestamp();
		if ( entry_timestamp < lowest_timestamp ) {
			lowest_timestamp = entry_timestamp;
			lowest_timestamp_index = i;
		}
		if ( entry_timestamp > highest_timestamp ) {
			highest_timestamp = entry_timestamp;
			highest_timestamp_index = i;
		}
		if ( entry_timestamp == requested_timestamp ) {
			out.Set(history[initial_index]);
			success = true;
			break;
		}
		initial_index--;
		if ( initial_index < 0 ) {
			initial_index = history_size - 1;
		}
	}

	/* If a match was not found, and the requested timestamp falls
	 * within two entries in the history, interpolate a Quaternion
	 * using SLERP.
	 */
	if ( !success ) {
		double timestamp_delta = highest_timestamp - lowest_timestamp;
		double requested_timestamp_offset = requested_timestamp - lowest_timestamp;
		double requested_timestamp_ratio = requested_timestamp_offset / timestamp_delta;

		Quaternion::slerp(  history[lowest_timestamp_index],
							history[highest_timestamp_index],
							requested_timestamp_ratio,
							out);
		out.SetInterpolated(true);
		success = true;
	}

	return success;
}

