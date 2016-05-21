/*
 * TimestampedQuaternionHistory.h
 *
 *  Created on: Jul 29, 2015
 *      Author: Scott
 */

#ifndef SRC_TIMESTAMPED_QUATERNION_HISTORY_H_
#define SRC_TIMESTAMPED_QUATERNION_HISTORY_H_

#include <stdint.h>
#include <mutex>
#include <TimestampedQuaternion.h>

class TimestampedQuaternionHistory {

protected:
    TimestampedQuaternion* history;
    int history_size;
    int curr_index;
    int num_valid_samples;
    std::mutex history_mutex;

public:
    TimestampedQuaternionHistory(int num_samples);

    void Add(float w, float x, float y, float z, long new_timestamp);
    bool Get( long requested_timestamp, TimestampedQuaternion& out );
};

#endif /* SRC_TIMESTAMPED_QUATERNION_HISTORY_H_ */
