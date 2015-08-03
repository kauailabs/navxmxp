/*
 * OffsetTracker.h
 *
 *  Created on: Jul 30, 2015
 *      Author: Scott
 */

#ifndef SRC_OFFSETTRACKER_H_
#define SRC_OFFSETTRACKER_H_

class OffsetTracker {
    float *value_history;
    int next_value_history_index;
    int history_len;
    double value_offset;

public:
    OffsetTracker(int history_length);
    void UpdateHistory(float curr_value);
    void SetOffset();
    double ApplyOffset( double value );

private:
    double GetAverageFromHistory();
    double GetOffset();
};

#endif /* SRC_OFFSETTRACKER_H_ */
