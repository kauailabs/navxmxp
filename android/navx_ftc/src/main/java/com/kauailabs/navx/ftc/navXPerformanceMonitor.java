package com.kauailabs.navx.ftc;

import android.os.SystemClock;

import com.qualcomm.robotcore.util.ElapsedTime;

/**
 * Created by Scott on 10/20/2015.
 */
public class navXPerformanceMonitor implements IDataArrivalSubscriber {

    private ElapsedTime runtime = new ElapsedTime();
    private AHRS navx_device;
    private long last_system_timestamp = 0;
    private long last_sensor_timestamp = 0;
    private long sensor_timestamp_delta = 0;
    private long system_timestamp_delta = 0;
    private byte sensor_update_rate_hz = 40;
    private int missing_sensor_sample_count = 0;
    private int estimated_missing_sensor_sample_count = 0;
    private boolean first_sample_received = false;
    private int hertz_counter = 0;
    private int last_second_hertz = 0;

    final int MS_PER_SEC = 1000;

    public navXPerformanceMonitor(AHRS navx_device) {
        this.navx_device = navx_device;
        reset();
    }

    public void reset() {
        last_system_timestamp = 0;
        last_sensor_timestamp = 0;
        sensor_timestamp_delta = 0;
        system_timestamp_delta = 0;
        sensor_update_rate_hz = 40;
        missing_sensor_sample_count = 0;
        first_sample_received = false;
        hertz_counter = 0;
        last_second_hertz = 0;
    }
    public int getDeliveredRateHz() {
        return hertz_counter;
    }
    public int getSensorRateHz() {
        return navx_device.getActualUpdateRate();
    }
    public int getDimTransferRateHz() {
        return navx_device.getCurrentTransferRate();
    }
    public int getNumMissedSensorTimestampedSamples() {
        return missing_sensor_sample_count;
    }
    public int getNumEstimatedMissedUntimestampedSamples() {
        return estimated_missing_sensor_sample_count;
    }
    public long getLastSensorTimestampDeltaMS() {
        return sensor_timestamp_delta;
    }
    public long getLastSystemTimestampDeltaMS() {
        return system_timestamp_delta;
    }


    private int NORMAL_DIM_TRANSFER_ITTER_MS = 10;
    @Override
    public void untimestampedDataReceived(long curr_system_timestamp, Object kind) {
        byte sensor_update_rate = navx_device.getActualUpdateRate();
        long num_dropped = 0;
         system_timestamp_delta = curr_system_timestamp - last_system_timestamp;
        int expected_sample_time_ms = MS_PER_SEC / (int)sensor_update_rate;

        if ( !navx_device.isConnected() ) {
            reset();
        } else {
            if ( ( curr_system_timestamp % 1000 ) < ( last_system_timestamp % 1000 ) ) {
                /* Second roll over.  Start the Hertz accumulator */
                last_second_hertz = hertz_counter;
                hertz_counter = 1;
            } else {
                hertz_counter++;
            }
            if ( !first_sample_received ) {
                last_sensor_timestamp = curr_system_timestamp;
                first_sample_received = true;
                estimated_missing_sensor_sample_count = 0;
            } else {
                if (system_timestamp_delta > (expected_sample_time_ms + NORMAL_DIM_TRANSFER_ITTER_MS) ) {
                    long estimated_dropped_samples = (system_timestamp_delta / expected_sample_time_ms) - 1;
                    if (estimated_dropped_samples > 0) {
                        estimated_missing_sensor_sample_count += estimated_dropped_samples;
                    }
                }
            }
        }

        last_system_timestamp = curr_system_timestamp;
    }

    final int NAVX_TIMESTAMP_JITTER_MS = 2;

    @Override
    public void timestampedDataReceived(long curr_system_timestamp,
                                        long curr_sensor_timestamp,
                                        Object kind) {
        long num_dropped = 0;
        byte sensor_update_rate = navx_device.getActualUpdateRate();
        sensor_timestamp_delta = curr_sensor_timestamp - last_sensor_timestamp;
        system_timestamp_delta = curr_system_timestamp - last_system_timestamp;
        int expected_sample_time_ms = MS_PER_SEC / (int)sensor_update_rate;

        if ( !navx_device.isConnected() ) {
            reset();
        } else {
            if ( ( curr_system_timestamp % 1000 ) < ( last_system_timestamp % 1000 ) ) {
                /* Second roll over.  Start the Hertz accumulator */
                last_second_hertz = hertz_counter;
                hertz_counter = 1;
            } else {
                hertz_counter++;
            }
            if ( !first_sample_received ) {
                last_sensor_timestamp = curr_sensor_timestamp;
                first_sample_received = true;
                missing_sensor_sample_count = 0;
            } else {
                if (sensor_timestamp_delta > (expected_sample_time_ms + NAVX_TIMESTAMP_JITTER_MS) ) {
                    long dropped_samples = (sensor_timestamp_delta / expected_sample_time_ms) - 1;
                    if (dropped_samples > 0) {
                        missing_sensor_sample_count += dropped_samples;
                    }
                }
            }
        }

        last_sensor_timestamp = curr_sensor_timestamp;
        last_system_timestamp = curr_system_timestamp;
    }
}
