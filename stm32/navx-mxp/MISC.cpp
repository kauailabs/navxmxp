#include "MISC.h"
#include "navx-mxp_hal.h"
#include "IOCXRegisters.h" /* For ANALOG_TRIGGER_NUM_TO_INT_BIT */
#include "MISCRegisters.h"
#include "stm32f4xx_hal.h"
#include "navx-mxp.h"

#include <stdlib.h>

static MISC_REGS misc_regs;
static uint32_t last_analog_processing_timestamp;

/* Analog Triggers (see MISC_Registers.h for more) */

typedef struct {
	ANALOG_TRIGGER_MODE mode;
	uint16_t low_threshold;
	uint16_t high_threshold;
	ANALOG_TRIGGER_STATE last_nonwindow_state;
	ANALOG_TRIGGER_STATE last_state;
} AnalogTrigger;

const uint16_t CMOS_LOW_THRESHOLD =  (uint16_t)((3.3f / 4096) * 0.8f);
const uint16_t CMOS_HIGH_THRESHOLD = (uint16_t)((3.3f / 4096) * 2.0f);

static AnalogTrigger analog_trigger[MISC_NUM_ANALOG_TRIGGERS] = {
	{ ANALOG_TRIGGER_DISABLED, CMOS_LOW_THRESHOLD, CMOS_HIGH_THRESHOLD, ANALOG_TRIGGER_LOW, ANALOG_TRIGGER_LOW },
	{ ANALOG_TRIGGER_DISABLED, CMOS_LOW_THRESHOLD, CMOS_HIGH_THRESHOLD, ANALOG_TRIGGER_LOW, ANALOG_TRIGGER_LOW },
	{ ANALOG_TRIGGER_DISABLED, CMOS_LOW_THRESHOLD, CMOS_HIGH_THRESHOLD, ANALOG_TRIGGER_LOW, ANALOG_TRIGGER_LOW },
	{ ANALOG_TRIGGER_DISABLED, CMOS_LOW_THRESHOLD, CMOS_HIGH_THRESHOLD, ANALOG_TRIGGER_LOW, ANALOG_TRIGGER_LOW },
};

typedef struct {
	volatile bool enable_request;
	volatile bool reset_request;
	volatile int8_t analog_channel_index; // -1 if not active
	bool enabled;
	uint64_t last_processed_sample;
} AnalogAccumulator;

static AnalogAccumulator analog_accumulator[MISC_NUM_ANALOG_ACCUMULATORS] = {
		{ true, false, false, 0, 0, },
		{ false, false, false, 0, 0, },
};

static void InitAnalogOversampleAndAverageConfiguration(int analog_input_index) {
	if (analog_input_index >= MISC_NUM_ANALOG_INPUTS) {
		return;
	}
	misc_regs.analog_input_average_bits[analog_input_index] = 3;
	misc_regs.analog_input_oversample_bits[analog_input_index] = 0;
	misc_regs.analog_input_average_value[analog_input_index] = 0;
	misc_regs.analog_input_oversample_value[analog_input_index] = 0;
}

static void InitAnalogAccumulator(int analog_accumulator_index) {
	if (analog_accumulator_index >= MISC_NUM_ANALOG_ACCUMULATORS) {
		return;
	}
	misc_regs.analog_accumulator_center[analog_accumulator_index] = 0;
	misc_regs.analog_accumulator_count[analog_accumulator_index] = 0;
	misc_regs.analog_accumulator_value[analog_accumulator_index] = 0;
	misc_regs.analog_accumulator_deadband[analog_accumulator_index] = 0;
	misc_regs.analog_accumulator_cfg[analog_accumulator_index].anin_channel = 0;
	misc_regs.analog_accumulator_cfg[analog_accumulator_index].enabled = 0;
	misc_regs.analog_accumulator_cfg[analog_accumulator_index].reset = 0;
}

_EXTERN_ATTRIB void MISC_init()
{
	/* Enable power to all external IOs */
	HAL_IOCX_Ext_Power_Enable(1);
	/* Since this powers the ADCs, delay for this to complete. */
	HAL_Delay(5);
	HAL_IOCX_ADC_Enable(1);

    for ( int i = 0; i < MISC_NUM_ANALOG_INPUTS; i++) {
    	InitAnalogOversampleAndAverageConfiguration(i);
    }

    for (int i = 0; i < MISC_NUM_ANALOG_ACCUMULATORS; i++) {
    	InitAnalogAccumulator(i);
    }

	misc_regs.capability_flags.powerctl = 1;
	misc_regs.capability_flags.analog_trigger = 1;
	misc_regs.capability_flags.rtc = 1;
	misc_regs.capability_flags.analog_in = 1;
	/* By default, enable power currentlimit mode */
	misc_regs.ext_pwr_ctl_cfg.ext_pwr_currentlimit_mode = 1;
}

#define NUM_MS_BETWEEN_SUCCESSIVE_ANALOG_INPUT_PROCESSING	2
#define NUM_MS_BETWEEN_SUCCESSIVE_OVERCURRENT_CHECKS		10
#define NUM_OVERCURRENT_POWER_OFF_PERIODS					20
#define NUM_MS_BETWEEN_SUCCESSIVE_UNDERVOLTAGE_CHECKS		20

bool ext_power_overcurrent = false;
int  ext_power_num_power_off_periods = 0;
bool ext_power_undervoltage = false;
static uint32_t last_overcurrent_check_timestamp;
static uint32_t last_undervoltage_check_timestamp;

const float undervoltage_min = 5.75f;
const float undervoltage_max = 5.85;

int8_t enabled_analog_accumulator_by_analog_input[MISC_NUM_ANALOG_INPUTS] = {-1, -1, -1, -1};

uint32_t accumulator_overrun_error_count = 0;
uint64_t accumulator_lost_sample_count = 0;

void AddToAccumulator(uint8_t analog_accumulator_index, uint32_t newOvsAvgValue) {
	// Accumulating data with negative/positive values, where the configured value is the center.
	if (misc_regs.analog_accumulator_center[analog_accumulator_index] != 0) {
		int16_t centered_avg = newOvsAvgValue - misc_regs.analog_accumulator_center[analog_accumulator_index];
		if (abs(centered_avg) <= abs(misc_regs.analog_accumulator_deadband[analog_accumulator_index])) {
			centered_avg = 0;
		}
		misc_regs.analog_accumulator_value[analog_accumulator_index] += centered_avg;
	} else {
		misc_regs.analog_accumulator_value[analog_accumulator_index] += newOvsAvgValue;
	}
	misc_regs.analog_accumulator_count[analog_accumulator_index]++;
}

// Performs Analog Input and Trigger processing, and External Power Current and Voltage monitoring.
_EXTERN_ATTRIB void MISC_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();
	if ( curr_loop_timestamp < last_analog_processing_timestamp) {
		/* Timestamp rollover */
		last_analog_processing_timestamp =
				last_overcurrent_check_timestamp =
						last_undervoltage_check_timestamp = curr_loop_timestamp;
	} else {

		// Process analog accumulator state changes (this occurs first, on every
		// loop iteration to ensure responsiveness of the register control interface.
		for (uint8_t i = 0; i < MISC_NUM_ANALOG_ACCUMULATORS; i++)  {
			if (analog_accumulator[i].enable_request) {
				if (!analog_accumulator[i].enabled) {
					analog_accumulator[i].enabled = true;
					analog_accumulator[i].reset_request = true; // Always reset when first enabled

					enabled_analog_accumulator_by_analog_input[analog_accumulator[i].analog_channel_index] =
							i;
				}
			} else {
				if (analog_accumulator[i].enabled) {
					analog_accumulator[i].enabled = false;
					analog_accumulator[i].reset_request = true; // Always reset when disabled
					for (uint8_t j = 0; j < MISC_NUM_ANALOG_INPUTS; j++) {
						if (enabled_analog_accumulator_by_analog_input[j] == i) {
							enabled_analog_accumulator_by_analog_input[j] = -1;
							break;
						}
					}
				}
			}
			if (analog_accumulator[i].reset_request) {
				misc_regs.analog_accumulator_count[i] = 0;
				misc_regs.analog_accumulator_value[i] = 0;
				analog_accumulator[i].last_processed_sample = 0;
				analog_accumulator[i].reset_request = false;  // Clear register to indicate when done
			}
		}

		/* Analog Input Processing */

		if ((curr_loop_timestamp - last_analog_processing_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_ANALOG_INPUT_PROCESSING){
			last_analog_processing_timestamp = curr_loop_timestamp;

			// HARDWARE BUG:  Starting at board revision 5.35, the ability to detect whether 3.3V or 5V
			// is used for analog inputs no longer works.  This should be corrected in a later version
			// For now, hard-code the analog input voltage to 3.3V (this is the default)/
			//misc_regs.capability_flags.an_in_5V = HAL_IOCX_ADC_Voltage_5V();
			misc_regs.capability_flags.an_in_5V = 0;
			misc_regs.ext_pwr_voltage_value = HAL_IOCX_Get_ExtPower_Voltage();

			/* Process Analog Oversample & Average, Trigger and Accumulation */
			for ( int i = 0; i < MISC_NUM_ANALOG_INPUTS; i++) {

				uint8_t num_oversample_bits = misc_regs.analog_input_oversample_bits[i];
				uint8_t num_avg_bits = misc_regs.analog_input_average_bits[i];

				uint32_t full_dma_transfer_cycle_count;
				uint32_t num_completed_per_channel_transfers;
				uint64_t adc_cumulative_sample_count = HAL_IOCX_ADC_GetLastCompleteSampleCount(&full_dma_transfer_cycle_count, &num_completed_per_channel_transfers);

				// Round cumulative sample count to align with the # samples in the average.
				uint16_t num_avg_sample_sets;
				uint64_t last_sample_abs_position;
				uint16_t last_sample_in_final_avg_sample_set =
						HAL_IOCX_ADC_GetAverageSetFinalSamplePosition(
								adc_cumulative_sample_count,
								num_oversample_bits,
								num_avg_bits,
								&num_avg_sample_sets,
								&last_sample_abs_position);

				// As long as one complete ADC DMA Transfer Cycle is completed, process ADC data on this channel
				if (full_dma_transfer_cycle_count > 0) {

					/*************************/
					/* Process Analog Inputs */
					/*************************/

					// The following section is timing-variable & sensitive; the elapsed duration depends upon the
					// number of oversample and average bits.  For the maximum value (10 bits)
					// it takes an average of 310 microseconds to sample a single channel.  For the minimum
					// (0 bits) it takes 11 microsecond.  Based on these statistics, the per-sample
					// multiplier is 291 nanoseconds per sample.  And in the worst case (10-bits on
					// all 4 channels, this can take 4*310 microseconds = 1.24 milliseconds to execute
					// To ensure that sample data accessed by the oversample/average engine is not overwritten
					// during this process, the dma transfer buffer should hold at least 1.24 milliseconds of data
					// (e.g., 60 samples).  Therefore, the dma transfer buffer is sized to be somewhat larger than
					// the largest # of processed samples + the execution time to oversample/average the samples.

					uint32_t last_oversample;
					uint32_t avg;
					uint16_t last_sample_in_prev_avg_sample_set;
					HAL_IOCX_ADC_Get_OversampledAveraged_Request_Samples(i, last_sample_in_final_avg_sample_set, &last_oversample, num_oversample_bits, &avg, num_avg_bits, &last_sample_in_prev_avg_sample_set);

					misc_regs.analog_input_average_value[i] = avg;
					misc_regs.analog_input_oversample_value[i] = last_oversample;

					/***************************/
					/* Process Analog Triggers */
					/***************************/

					if(analog_trigger[i].mode != ANALOG_TRIGGER_DISABLED) {
						ANALOG_TRIGGER_STATE curr_state;
						if ( uint16_t(avg) <= analog_trigger[i].low_threshold) {
							curr_state = ANALOG_TRIGGER_LOW;
						} else if ( uint16_t(avg) >= analog_trigger[i].high_threshold) {
							curr_state = ANALOG_TRIGGER_HIGH;
						} else {
							curr_state = ANALOG_TRIGGER_IN_WINDOW;
						}
						if(curr_state != ANALOG_TRIGGER_IN_WINDOW) {
							if (curr_state != analog_trigger[i].last_nonwindow_state) {
								/* State Change has occurred */
								if (analog_trigger[i].last_state != ANALOG_TRIGGER_LOW) {
									analog_trigger[i].last_state = ANALOG_TRIGGER_LOW;
									/* Falling Edge Detected */
									if ( analog_trigger[i].mode == ANALOG_TRIGGER_MODE_FALLING_EDGE_PULSE) {
										HAL_IOCX_AssertInterrupt(ANALOG_TRIGGER_NUMBER_TO_INT_BIT(i));
									}
								}
								else {
									/* Rising Edge Detected */
									analog_trigger[i].last_state = ANALOG_TRIGGER_HIGH;
									if ( analog_trigger[i].mode == ANALOG_TRIGGER_MODE_RISING_EDGE_PULSE) {
										HAL_IOCX_AssertInterrupt(ANALOG_TRIGGER_NUMBER_TO_INT_BIT(i));
									}
								}
							}
						}
						if (analog_trigger[i].last_state != curr_state) {
							analog_trigger[i].last_state = curr_state;
							HAL_IOCX_ADC_SetCurrentAnalogTriggerState(i, curr_state);
						}
					}

					/*******************************/
					/* Process Analog Accumulators */
					/*******************************/

					int8_t analog_accumulator_index =
							enabled_analog_accumulator_by_analog_input[i];
					if (analog_accumulator_index != -1) {
						// Analog Trigger enabled for this channel
						// The most recent average was calculated above
						// The accumulator should now acquire the averages (if any) preceding
						// the most recent average, and following the last average added to the
						// accumulator.
						uint8_t num_samples_in_sample_average_set = (1 << (num_oversample_bits + num_avg_bits));
						uint64_t earliest_available_position =
								last_sample_abs_position -
									((num_avg_sample_sets - 1) * num_samples_in_sample_average_set);
						if (analog_accumulator[analog_accumulator_index].last_processed_sample == 0) {
							// No older samples to accumulate.  The most recent average (acquired above)
							// is the first average in the accumulator.
							misc_regs.analog_accumulator_count[analog_accumulator_index] = 0;
							AddToAccumulator(analog_accumulator_index, avg);
						} else {
							uint16_t delta_average_sample_sets;
							if (earliest_available_position > analog_accumulator[analog_accumulator_index].last_processed_sample) {
								// Samples were dropped during the accumulation process.
								// Set a debug flag with info about the droppage (which should be avoided)
								delta_average_sample_sets = (num_avg_sample_sets - 1);
								accumulator_overrun_error_count++;
								accumulator_lost_sample_count += (earliest_available_position - analog_accumulator[analog_accumulator_index].last_processed_sample);
							} else {
								// Sufficient samples exist are available to fulfill accumulator requirements
								uint16_t delta_samples = last_sample_abs_position -
										analog_accumulator[analog_accumulator_index].last_processed_sample;
								delta_average_sample_sets = delta_samples / num_samples_in_sample_average_set;
							}
							// Add the most recent average (acquired above) to the accumulator
							if (analog_accumulator[analog_accumulator_index].last_processed_sample < last_sample_abs_position) {
								AddToAccumulator(analog_accumulator_index, avg);
								delta_average_sample_sets--;
							}
							// Calculate and add preceding samples to the accumulator
							for (uint16_t j = 0; j < delta_average_sample_sets; j++) {
								uint16_t last_sample_in_curr_avg_sample_set = last_sample_in_prev_avg_sample_set;
								HAL_IOCX_ADC_Get_OversampledAveraged_Request_Samples(i, last_sample_in_curr_avg_sample_set, &last_oversample, num_oversample_bits, &avg, num_avg_bits, &last_sample_in_prev_avg_sample_set);
								AddToAccumulator(analog_accumulator_index, avg);
							}
						}
						analog_accumulator[analog_accumulator_index].last_processed_sample =
								last_sample_abs_position;
					}
				}
			}
		}

		/********************************************/
		/* External Power Output Current monitoring */
		/********************************************/

		if ((curr_loop_timestamp - last_overcurrent_check_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_OVERCURRENT_CHECKS){
			last_overcurrent_check_timestamp = curr_loop_timestamp;
			if(ext_power_overcurrent) {
				/* If Over-current no longer present, re-enable ext power switch */
				if(!HAL_IOCX_Ext_Power_Fault()) {
					ext_power_num_power_off_periods++;
					if (ext_power_num_power_off_periods > NUM_OVERCURRENT_POWER_OFF_PERIODS) {
						ext_power_overcurrent = false;
						ext_power_num_power_off_periods = 0;
						HAL_IOCX_Ext_Power_Enable(1);
						misc_regs.ext_pwr_ctl_status.ext_pwr_overcurrent = 0;
					}
				}
			} else {
				/* If Over-current is present, disable ext power switch. */
				if(HAL_IOCX_Ext_Power_Fault()) {
					/* NOTE:  Only disable power if current limit mode enabled */
					if (misc_regs.ext_pwr_ctl_cfg.ext_pwr_currentlimit_mode) {
						ext_power_overcurrent = true;
						ext_power_num_power_off_periods = 0;
						HAL_IOCX_Ext_Power_Enable(0);
						misc_regs.ext_pwr_ctl_status.ext_pwr_overcurrent = 1;
					}
				}
			}
		}

		/*******************************************/
		/* External Power Input Voltage monitoring */
		/*******************************************/

		if ((curr_loop_timestamp - last_undervoltage_check_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_UNDERVOLTAGE_CHECKS){
			last_undervoltage_check_timestamp = curr_loop_timestamp;
			float ext_power_volts = (((float)HAL_IOCX_Get_ExtPower_Voltage()) / 4096) * 3.3f;
			ext_power_volts *= (1.0f/EXTPOWER_INPUT_VDIV_RATIO);

			if(ext_power_undervoltage) {
				if (ext_power_volts >= undervoltage_max) {
					/* If voltage has risen above the upper threshold, */
					/* re-enable the ext power switch */
					ext_power_undervoltage = false;
					HAL_IOCX_Ext_Power_Enable(1);
					misc_regs.ext_pwr_ctl_status.ext_pwr_undervoltage = 0;
				}
			} else {
				if (ext_power_volts <= undervoltage_min) {
					/* If voltage falls below lower threshold, */
					/* disable the ext power switch. */
					ext_power_undervoltage = true;
					HAL_IOCX_Ext_Power_Enable(0);
					misc_regs.ext_pwr_ctl_status.ext_pwr_undervoltage = 1;
				}
			}
		}
	}
}

_EXTERN_ATTRIB uint8_t *MISC_get_reg_addr_and_max_size( uint8_t bank, uint8_t register_offset, uint8_t requested_count, uint16_t* size )
{
	if ( bank == MISC_REGISTER_BANK) {
	    if ( register_offset > sizeof(struct MISC_REGS) ) {
	        size = 0;
	        return 0;
	    }

	    uint8_t first_offset = register_offset;
	    uint8_t last_offset = register_offset + requested_count - 1;

	    bool date_read = false;
	    if((first_offset >= offsetof(struct MISC_REGS, rtc_time)) &&
	       (last_offset <
	    		   (offsetof(struct MISC_REGS, rtc_time) +
	    			sizeof(misc_regs.rtc_time)))) {
		    /* Requested data includes rtc timestamp; retrieve current value */
	    	HAL_RTC_Get_Time(&misc_regs.rtc_time.hours,
	    			&misc_regs.rtc_time.minutes,
	    			&misc_regs.rtc_time.seconds,
	    			&misc_regs.rtc_time.subseconds);
	    	date_read = true;
	    	/* The Date must always be read after reading the time */
	    	/* The STM32 Shadow register usage requires this. */
	    	HAL_RTC_Get_Date(&misc_regs.rtc_date.weekday,
	    			&misc_regs.rtc_date.date,
	    			&misc_regs.rtc_date.month,
	    			&misc_regs.rtc_date.year);
	    }
	    if((first_offset >= offsetof(struct MISC_REGS, rtc_date)) &&
	 	       (last_offset <
	 	    		   (offsetof(struct MISC_REGS, rtc_date) +
	 	    			sizeof(misc_regs.rtc_date)))) {
		    /* Requested data includes rtc datestamp; retrieve current value */
	    	if(!date_read) {
				HAL_RTC_Get_Date(&misc_regs.rtc_date.weekday,
						&misc_regs.rtc_date.date,
						&misc_regs.rtc_date.month,
						&misc_regs.rtc_date.year);
	    	}
	    }

	    uint8_t *register_base = (uint8_t *)&misc_regs;
	    *size = sizeof(struct MISC_REGS) - register_offset;
	    return register_base + register_offset;
	}
	return 0;
}

static void ext_pwr_ctl_cfg_modified(uint8_t first_offset, uint8_t count) {
}

static void rtc_cfg_modified(uint8_t first_offset, uint8_t count) {
	HAL_RTC_Set_DaylightSavings(misc_regs.rtc_cfg.daylight_savings);
}

static void rtc_time_set_modified(uint8_t first_offset, uint8_t count) {
	HAL_RTC_Set_Time(misc_regs.rtc_time_set.hours,
			misc_regs.rtc_time_set.minutes,
			misc_regs.rtc_time_set.seconds);
}

static void rtc_date_set_modified(uint8_t first_offset, uint8_t count) {
	HAL_RTC_Set_Date(misc_regs.rtc_date_set.weekday,
			misc_regs.rtc_date_set.date,
			misc_regs.rtc_date_set.month,
			misc_regs.rtc_date_set.year);
}

static void analog_trigger_cfg_modified(uint8_t first_offset, uint8_t count) {
}

static void analog_trigger_threshold_low_modified(uint8_t first_offset, uint8_t count) {
}

static void analog_trigger_threshold_high_modified(uint8_t first_offset, uint8_t count) {
}

static void analog_input_oversample_bits_modified(uint8_t first_offset, uint8_t count) {
}

static void analog_input_average_bits_modified(uint8_t first_offset, uint8_t count) {
}

static void analog_accumulator_cfg_modified(uint8_t first_offset, uint8_t count) {
	uint8_t first_accumulator_modified = first_offset / sizeof(misc_regs.analog_accumulator_cfg[0]);
	uint8_t last_accumulator_modified = ((first_offset + count)-1) / sizeof(misc_regs.analog_accumulator_cfg[0]);
	for ( uint8_t accumulator_index = first_accumulator_modified; accumulator_index <= last_accumulator_modified; accumulator_index++ ) {
		if (accumulator_index < MISC_NUM_ANALOG_ACCUMULATORS) {
			analog_accumulator[accumulator_index].enable_request = misc_regs.analog_accumulator_cfg[accumulator_index].enabled;
			analog_accumulator[accumulator_index].reset_request = misc_regs.analog_accumulator_cfg[accumulator_index].reset;
			if (analog_accumulator[accumulator_index].enable_request) {
				analog_accumulator[accumulator_index].analog_channel_index =
						misc_regs.analog_accumulator_cfg[accumulator_index].anin_channel;
			}
		}
	}
}

static WritableRegSet all_reg_sets[] =
{
	/* Contiguous registers, increasing order of offset  */
	{ offsetof(struct MISC_REGS, ext_pwr_ctl_cfg), sizeof(MISC_REGS::ext_pwr_ctl_cfg), ext_pwr_ctl_cfg_modified },
	{ offsetof(struct MISC_REGS, rtc_cfg), sizeof(MISC_REGS::rtc_cfg), rtc_cfg_modified },
	{ offsetof(struct MISC_REGS, rtc_time_set), sizeof(MISC_REGS::rtc_time_set), rtc_time_set_modified },
	{ offsetof(struct MISC_REGS, rtc_date_set), sizeof(MISC_REGS::rtc_date_set), rtc_date_set_modified },
	{ offsetof(struct MISC_REGS, analog_trigger_cfg), sizeof(MISC_REGS::analog_trigger_cfg), analog_trigger_cfg_modified },
	{ offsetof(struct MISC_REGS, analog_trigger_threshold_low), sizeof(MISC_REGS::analog_trigger_threshold_low), analog_trigger_threshold_low_modified },
	{ offsetof(struct MISC_REGS, analog_trigger_threshold_high), sizeof(MISC_REGS::analog_trigger_threshold_high), analog_trigger_threshold_high_modified },
	{ offsetof(struct MISC_REGS, analog_input_oversample_bits), sizeof(MISC_REGS::analog_input_oversample_bits), analog_input_oversample_bits_modified },
	{ offsetof(struct MISC_REGS, analog_input_average_bits), sizeof(MISC_REGS::analog_input_average_bits), analog_input_average_bits_modified },
	{ offsetof(struct MISC_REGS, analog_accumulator_cfg), sizeof(MISC_REGS::analog_accumulator_cfg), analog_accumulator_cfg_modified },
};

static WritableRegSetGroup writable_reg_set_groups[] =
{
	{ all_reg_sets[0].start_offset,
			all_reg_sets[SIZEOF_STRUCT(all_reg_sets)-1].start_offset + all_reg_sets[SIZEOF_STRUCT(all_reg_sets)-1].num_bytes,
			all_reg_sets,
		SIZEOF_STRUCT(all_reg_sets) },
};

_EXTERN_ATTRIB void MISC_banked_writable_reg_update_func(uint8_t bank, uint8_t reg_offset, uint8_t *p_reg, uint8_t count, uint8_t *p_new_values )
{
	if ( bank == MISC_REGISTER_BANK) {
		for ( size_t i = 0; i < SIZEOF_STRUCT(writable_reg_set_groups); i++) {
			if ( (reg_offset >= writable_reg_set_groups[i].first_offset) &&
				 (reg_offset <= writable_reg_set_groups[i].last_offset)) {
				for ( int j = 0; j < writable_reg_set_groups[i].set_count; j++) {
					WritableRegSet *p_set = &writable_reg_set_groups[i].p_sets[j];
					if ((reg_offset >= p_set->start_offset) &&
						(reg_offset < (p_set->start_offset + p_set->num_bytes))){
						int first_offset = (reg_offset - p_set->start_offset);
						int max_bytes_to_write_in_set = p_set->num_bytes-first_offset;
						int num_bytes_in_set_to_modify = (count < max_bytes_to_write_in_set) ? count : max_bytes_to_write_in_set;
						int num_bytes_in_set_changed = 0;
						while (num_bytes_in_set_changed < num_bytes_in_set_to_modify )  {
							*p_reg++ = *p_new_values++;
							reg_offset++;
							num_bytes_in_set_changed++;
							count--;
						}
						if (num_bytes_in_set_changed > 0){
							/* At least one byte in this set was modified. */
							p_set->changed(first_offset, num_bytes_in_set_changed);
						}
						if (count == 0) {
							break;
						}
					}
				}
			}
		}
	}
}

