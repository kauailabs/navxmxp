/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2015. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
package com.kauailabs.navx.frc;

import java.util.Arrays;

class OffsetTracker {
    float value_history[];
    int next_value_history_index;
    int history_len;
    double value_offset;

    public OffsetTracker(int history_length) {
        history_len = history_length;
        value_history = new float[history_len];        
        Arrays.fill(value_history,0);
        next_value_history_index = 0;
        value_offset = 0; 
    }
    
    public void updateHistory(float curr_value) {
        if (next_value_history_index >= history_len) {
            next_value_history_index = 0;
        }
        value_history[next_value_history_index] = curr_value;
        next_value_history_index++;
    }

    public double getAverageFromHistory() {
        double value_history_sum = 0.0;
        for (int i = 0; i < history_len; i++) {
            value_history_sum += value_history[i];
        }
        double value_history_avg = value_history_sum / history_len;
        return value_history_avg;
    }
    
    public void setOffset() {
        value_offset = getAverageFromHistory();
    }
    
    public double getOffset() {
        return value_offset;
    }
    
    public double applyOffset( double value ) {
        float offseted_value = (float) (value - value_offset);
        if (offseted_value < -180) {
            offseted_value += 360;
        }
        if (offseted_value > 180) {
            offseted_value -= 360;
        }        
        return offseted_value;
    }
}
