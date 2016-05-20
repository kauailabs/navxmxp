/*----------------------------------------------------------------------------*/
/* Copyright (c) Kauai Labs 2016. All Rights Reserved.                        */
/*                                                                            */
/* Created in support of Team 2465 (Kauaibots).  Go Purple Wave!              */
/*                                                                            */
/* Open Source Software - may be modified and shared by FRC teams. Any        */
/* modifications to this code must be accompanied by the \License.txt file    */ 
/* in the root directory of the project.                                      */
/*----------------------------------------------------------------------------*/
package com.kauailabs.navx.frc;

import java.lang.Long;

class TimestampedQuaternionHistory {

    TimestampedQuaternion[] history;
    int history_size;
    int curr_index;
    int num_valid_samples;
    
    public TimestampedQuaternionHistory(int num_samples) {
    	history_size = num_samples;
    	history = new TimestampedQuaternion[history_size];
    	for ( int i = 0; i < history_size; i++ ) {
    		history[i] = new TimestampedQuaternion();
    	}
    	curr_index = 0;
    	num_valid_samples = 0;
    }

    public void add(float w, float x, float y, float z, long new_timestamp) {
    	synchronized(this) {
			history[curr_index].set(w, x, y, z, new_timestamp);
	    	if ( curr_index < (history_size - 1)) {
	    		curr_index++;
	    	} else {
	    		curr_index = 0;
	    	}
	    	if ( num_valid_samples < history_size ) {
	    		num_valid_samples++;
	    	}			
    	}
    }
    
    public TimestampedQuaternion get( long requested_timestamp ) {
    	TimestampedQuaternion match = null;
    	int initial_index = curr_index;
    	long lowest_timestamp = Long.MAX_VALUE;
    	int lowest_timestamp_index = -1;
    	long highest_timestamp = Long.MIN_VALUE;
    	int highest_timestamp_index = -1;
    	synchronized(this) {
	    	for ( int i = 0; i < num_valid_samples; i++ ) {
	    		long entry_timestamp = history[initial_index].getTimestamp();
	    		if ( entry_timestamp < lowest_timestamp ) {
	    			lowest_timestamp = entry_timestamp;
	    			lowest_timestamp_index = i;
	    		}
	    		if ( entry_timestamp > highest_timestamp ) {
	    			highest_timestamp = entry_timestamp;
	    			highest_timestamp_index = i;
	    		}
	    		if ( entry_timestamp == requested_timestamp ) {
	    			match = history[initial_index];
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
	    	if ( match == null ) {   	
		    	double timestamp_delta = highest_timestamp - lowest_timestamp;
		    	double requested_timestamp_offset = requested_timestamp - lowest_timestamp;
		    	double requested_timestamp_ratio = requested_timestamp_offset / timestamp_delta;
		    	
		    	Quaternion interpolated_quaternion = Quaternion.slerp(  history[lowest_timestamp_index],
		    															history[highest_timestamp_index],
		    															requested_timestamp_ratio);
		    	    	
		    	match = new TimestampedQuaternion(interpolated_quaternion, requested_timestamp);
		    	match.setInterpolated(true);
	    	}
    	}
    	
    	if ( match != null ) {
    		/* Make a copy of the quaternion, so that caller does not directly reference
    		 * a quaternion within this history, which is volatile.
    		 */
    		return new TimestampedQuaternion(match);
    	}
    	
    	return null;
    }
}
