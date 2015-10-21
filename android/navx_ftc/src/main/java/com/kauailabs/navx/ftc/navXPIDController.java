/* ============================================
 NavX-MXP and NavX-Micro source code is placed under the MIT license
 Copyright (c) 2015 Kauai Labs

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ===============================================
 */
package com.kauailabs.navx.ftc;

import android.util.Log;

import java.security.Timestamp;

/**
 * The navXPIDController implements a timestamped PID controller (designed to deal
 * with the jitter which is typically present in a networked control system scenario).
 *
 * The navXPIDController can use an of the various data sources on a navX-Model device
 * as an input (process variable); when instantiating a navXPIDController simply
 * provide an AHRS class instance and specify which navX-Model device variable you
 * wish to use as the input.  Then, configure the navXPIDController's setPoint,
 * outputRange, whether it should operate in continuous mode or not, and the
 * P, I, D and F coefficients which will be used to calculate the output value.
 *
 * Using the navXPIDController w/the AHRS class is rather simple.  An example of using
 * the navXPIDController to rotate a FTC robot to a target angle is provided at:
 * http://pdocs.kauailabs.com/navx-micro/examples/rotate-to-angle/.
 *
 * The PID algorithm used herein is discussed in detail at
 * https://en.wikipedia.org/wiki/PID_controller
 *
 * In addition to the P,I,D terms, a FeedForward term is optionally available
 * which may be useful in cases where velocity is being controlled (e.g., to
 * achieve a continuous rotational velocity using a yaw rate gyro).  The FeedForward
 * concept is discussed at http://www.expertune.com/articles/UG2007/PIDControlsPLCEnviron.pdf
 *
 * This algorithm implements two features with respect to the integral gain calculated
 * based on the integral (i) coefficient:
 *
 * - Anti-Windup:  Ensures the integral gain doesn't exceed the min/max output range, as discussed
 *   at http://www.expertune.com/articles/UG2007/PIDControlsPLCEnviron.pdf
 * - Time-Correction:  Adjust the integral gain in cases when timestamps indicate that
 *   data samples were lost.
 *
 * This algorithm implements this feature with respect to the derivative gain, as discussed
 *   at http://www.diva-portal.org/smash/get/diva2:570067/FULLTEXT01.pdf
 */

public class navXPIDController implements IDataArrivalSubscriber {

    public enum TimestampType {SENSOR, SYSTEM};

    static public class PIDResult {
        public double output;
        public long timestamp;
        public boolean on_target;
        public PIDResult() {
            output = 0.0;
            timestamp = 0;
            on_target = false;
        }
        public long    getTimestamp() { return timestamp; }
        public boolean isOnTarget()   { return on_target; }
        public double  getOutput()    { return output; }
    }

    private Object sync_event = new Object();

    public enum navXTimestampedDataSource {
        YAW,
        PITCH,
        ROLL,
        COMPASS_HEADING,
        FUSED_HEADING,
        ALTITUDE,
        LINEAR_ACCEL_X,
        LINEAR_ACCEL_Y,
        LINEAR_ACCEL_Z
    }

    public enum navXUntimestampedDataSource {
        RAW_GYRO_X,
        RAW_GYRO_Y,
        RAW_GYRO_Z,
        RAW_ACCEL_X,
        RAW_ACCEL_Y,
        RAW_ACCEL_Z,
        RAW_MAG_X,
        RAW_MAG_Y,
        RAW_MAG_Z
    }

    private boolean timestamped = true;
    navXTimestampedDataSource timestamped_src;
    navXUntimestampedDataSource untimestamped_src;
    AHRS navx_device;
    long last_system_timestamp = 0;
    long last_sensor_timestamp = 0;

    /* Error statistics */
    private double error_current    = 0.0;
    private double error_previous   = 0.0;
    private double error_total      = 0.0;

    /* Coefficients */
    private double p;
    private double i;
    private double d;
    private double ff;

    /* Input/Output Clamps */
    private double max_input        = 0.0;
    private double min_input        = 0.0;
    private double max_output       = 1.0;
    private double min_output       = -1.0;

    /* on-target tolerance */
    public enum ToleranceType {
        NONE, PERCENT, ABSOLUTE
    }

    private ToleranceType tolerance_type;
    double tolerance_amount;

    /* Behavior */
    private boolean continuous      = false;
    private boolean enabled         = false;

    private double setpoint         = 0.0;
    private double result           = 0.0;

    @Override
    public void untimestampedDataReceived(long curr_system_timestamp, Object kind) {
        if (!timestamped && (kind.getClass() == AHRS.DeviceDataType.class)) {
            double process_value;
            switch (untimestamped_src) {
                case RAW_GYRO_X:
                    process_value = navx_device.getRawGyroX();
                    break;
                case RAW_GYRO_Y:
                    process_value = navx_device.getRawGyroY();
                    break;
                case RAW_GYRO_Z:
                    process_value = navx_device.getRawGyroZ();
                    break;
                case RAW_ACCEL_X:
                    process_value = navx_device.getRawAccelX();
                    break;
                case RAW_ACCEL_Y:
                    process_value = navx_device.getRawAccelY();
                    break;
                case RAW_MAG_X:
                    process_value = navx_device.getRawMagX();
                    break;
                case RAW_MAG_Y:
                    process_value = navx_device.getRawMagY();
                    break;
                case RAW_MAG_Z:
                    process_value = navx_device.getRawMagZ();
                    break;
                default:
                    process_value = 0.0;
                    break;
            }
            int num_missed_samples = 0; /* TODO */
            last_system_timestamp = curr_system_timestamp;
            double output = this.stepController(process_value, num_missed_samples);
            synchronized (sync_event) {
                sync_event.notify();
            }
        }
    }

    @Override
    public void timestampedDataReceived(long curr_system_timestamp,
                                        long curr_sensor_timestamp, Object kind) {
        if (timestamped && (kind.getClass() == AHRS.DeviceDataType.class)) {
            double process_value;
            switch (timestamped_src) {
                case YAW:
                    process_value = navx_device.getYaw();
                    break;
                case PITCH:
                    process_value = navx_device.getPitch();
                    break;
                case ROLL:
                    process_value = navx_device.getRoll();
                    break;
                case COMPASS_HEADING:
                    process_value = navx_device.getCompassHeading();
                    break;
                case FUSED_HEADING:
                    process_value = navx_device.getFusedHeading();
                    break;
                case LINEAR_ACCEL_X:
                    process_value = navx_device.getWorldLinearAccelX();
                    break;
                case LINEAR_ACCEL_Y:
                    process_value = navx_device.getWorldLinearAccelY();
                    break;
                case LINEAR_ACCEL_Z:
                    process_value = navx_device.getWorldLinearAccelZ();
                    break;
                default:
                    process_value = 0.0;
                    break;
            }
            int num_missed_samples = 0; /* TODO */
            last_system_timestamp = curr_system_timestamp;
            last_sensor_timestamp = curr_sensor_timestamp;
            double output = this.stepController(process_value, num_missed_samples);
            synchronized (sync_event) {
                sync_event.notify();
            }
        }
    }

    public navXPIDController(AHRS navx_device, navXTimestampedDataSource src) {
        this.navx_device = navx_device;
        this.timestamped = true;
        this.timestamped_src = src;
        switch ( src ) {
            case YAW:
                this.setInputRange( -180.0, 180.0 );
                break;
            case PITCH:
            case ROLL:
                this.setInputRange( -90.0, 90.0 );
                break;
            case COMPASS_HEADING:
            case FUSED_HEADING:
                this.setInputRange( 0.0, 360.0 );
                break;
            case LINEAR_ACCEL_X:
            case LINEAR_ACCEL_Y:
            case LINEAR_ACCEL_Z:
                this.setInputRange (-2.0, 2.0 );
                break;
        }
        navx_device.registerCallback(this);
    }

    public navXPIDController(AHRS navx_device, navXUntimestampedDataSource src) {
        this.navx_device = navx_device;
        this.timestamped = false;
        this.untimestamped_src = src;
        navx_device.registerCallback(this);
    }

    public boolean isNewUpdateAvailable(PIDResult result) {
        return ((timestamped && (result.timestamp < this.last_sensor_timestamp) ||
                (result.timestamp < this.last_system_timestamp)));
    }

    public boolean waitForNewUpdate(PIDResult result, int timeout_ms) {
        boolean ready = isNewUpdateAvailable(result);
        if (!ready) {
            synchronized (sync_event) {
                try {
                    sync_event.wait(timeout_ms);
                } catch (InterruptedException ex) {
                    ex.printStackTrace();
                }
            }
            ready = isNewUpdateAvailable(result);
        }
        if (ready) {
            result.on_target = this.isOnTarget();
            result.output = this.get();
            if (timestamped) {
                result.timestamp = last_sensor_timestamp;
            } else {
                result.timestamp = last_system_timestamp;
            }
        }
        return ready;
    }

    public double getError() {
        return error_current;
    }

    public synchronized void setTolerance(ToleranceType tolerance_type, double tolerance_amount) {
        this.tolerance_amount = tolerance_amount;
        this.tolerance_type = tolerance_type;
    }

    public boolean isOnTarget() {
        boolean on_target = false;
        switch (tolerance_type) {
            case NONE:
                on_target = (getError() == 0);
                break;
            case PERCENT:
                on_target = (Math.abs(getError()) <
                        (tolerance_amount / 100 * (max_input - min_input)));
                break;
            case ABSOLUTE:
                on_target = (Math.abs(getError()) < tolerance_amount);
                break;
        }
        return on_target;
    }

    public double stepController(double process_variable, int num_missed_samples) {
        double local_result;
        if (enabled) {
            double d_adj;

            synchronized (this) {

                error_current = setpoint - process_variable;

                /* If a continuous controller, if > 1/2 way from the target */
                /* modify the error to ensure the output doesn't change     */
                /* direction, but processed onward until error is zero.     */
                if (continuous) {
                    if (Math.abs(error_current) > (max_input - min_input) / 2) {
                        if (error_current > 0) {
                            error_current -= max_input + min_input;
                        } else {
                            error_current += max_input - min_input;
                        }
                    }
                }

                /* Process integral term.  Perform "anti-windup" processing */
                /* by preventing the integral term from accumulating when   */
                /* the output reaches it's minimum or maximum limits.       */
                /* TODO:  adjust for dropped samples as described at:       */
                /* http://www.diva-portal.org/smash/get/diva2:570067/FULLTEXT01.pdf */
                if (i != 0) {
                    double estimated_i = (error_total + error_current) * i;
                    if (estimated_i < max_output) {
                        if (estimated_i > min_output) {
                            error_total += error_current;
                        } else {
                            error_total = min_output / i;
                        }
                    } else {
                        error_total = max_output / i;
                    }
                }

                /* If samples were missed, reduce the d gain by dividing */
                /* by the total number of samples since the last update. */
                if ( num_missed_samples > 0 ) {
                    d_adj = d / (1 + num_missed_samples);
                } else {
                    d_adj = d;
                }

                /* Calculate result w/P, I, D & F terms */
                result = p      * error_current +
                        i      * error_total +
                        d_adj  * (error_current - error_previous) +
                        ff     * setpoint;
                error_previous = error_current;

                /* Clamp result to output range */
                if (result > max_output) {
                    result = max_output;
                } else if (result < min_output) {
                    result = min_output;
                }

                local_result = result;
            }
        } else {
            local_result = 0.0;
        }
        return local_result;
    }

    public synchronized void setPID(double p, double i, double d) {
        this.p = p;
        this.i = i;
        this.d = d;
    }

    public synchronized void setPID(double p, double i, double d, double ff) {
        this.p = p;
        this.i = i;
        this.d = d;
        this.ff = ff;
    }

    public synchronized void setContinuous(boolean continuous) {
        this.continuous = continuous;
    }

    public synchronized double get() {
        return result;
    }

    public synchronized void setOutputRange(double min_output, double max_output) {
        if (min_output <= max_output) {
            this.min_output = min_output;
            this.max_output = max_output;
        }
    }

    public synchronized void setInputRange(double min_input, double max_input) {
        if (min_input <= max_input) {
            this.min_input = min_input;
            this.max_input = max_input;
            setSetpoint(setpoint);
        }
    }

    public synchronized void setSetpoint(double setpoint) {
        if (max_input > min_input) {
            if (setpoint > max_input) {
                this.setpoint = max_input;
            } else if (setpoint < min_input) {
                this.setpoint = min_input;
            } else {
                this.setpoint = setpoint;
            }
        } else {
            this.setpoint = setpoint;
        }
    }

    public synchronized double getSetpoint() {
        return setpoint;
    }

    public synchronized void enable(boolean enabled) {
        this.enabled = enabled;
    }

    public synchronized boolean isEnabled() {
        return this.enabled;
    }

    public synchronized void reset() {
        enable(false);
        error_previous = 0;
        error_total = 0;
        result = 0;
    }
}
