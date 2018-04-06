#include "MISC.h"
#include "navx-mxp_hal.h"
#include "IOCXRegisters.h" /* For ANALOG_TRIGGER_NUM_TO_INT_BIT */
#include "MISCRegisters.h"
#include "stm32f4xx_hal.h"
#include "navx-mxp.h"

static MISC_REGS misc_regs;
static uint32_t last_loop_timestamp;

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

_EXTERN_ATTRIB void MISC_init()
{
	/* Enable power to all external IOs */
	HAL_IOCX_Ext_Power_Enable(1);
	/* Since this powers the ADCs, delay for this to complete. */
	HAL_Delay(5);
	HAL_IOCX_ADC_Enable(1);
	misc_regs.capability_flags.powerctl = 1;
	misc_regs.capability_flags.analog_trigger = 1;
	misc_regs.capability_flags.rtc = 1;
	misc_regs.capability_flags.analog_in = 1;
	/* By default, enable power currentlimit mode */
	misc_regs.ext_pwr_ctl_cfg.ext_pwr_currentlimit_mode = 1;
}

#define NUM_MS_BETWEEN_SUCCESSIVE_LOOPS 2
#define NUM_MS_BETWEEN_SUCCESSIVE_OVERCURRENT_CHECKS  10
#define NUM_OVERCURRENT_POWER_OFF_PERIODS             20
#define NUM_MS_BETWEEN_SUCCESSIVE_UNDERVOLTAGE_CHECKS 20
#define NUM_ANALOG_INPUT_OVERSAMPLE_BITS 3
#define NUM_ANALOG_INPUT_AVERAGE_BITS NUM_ANALOG_INPUT_OVERSAMPLE_BITS

bool ext_power_overcurrent = false;
int  ext_power_num_power_off_periods = 0;
bool ext_power_undervoltage = false;
static uint32_t last_overcurrent_check_timestamp;
static uint32_t last_undervoltage_check_timestamp;
static uint32_t analog_oversamples[MISC_NUM_ANALOG_INPUTS];
static uint32_t analog_averages[MISC_NUM_ANALOG_INPUTS];

const float undervoltage_min = 5.75f;
const float undervoltage_max = 5.85;

/* Analog Triggers (see IOCX_Registers.h for more) */

_EXTERN_ATTRIB void MISC_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();
	if ( curr_loop_timestamp < last_loop_timestamp) {
		/* Timestamp rollover */
		last_loop_timestamp = curr_loop_timestamp;
	} else {
		if ((curr_loop_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_LOOPS){
			last_loop_timestamp = curr_loop_timestamp;
			misc_regs.capability_flags.an_in_5V = HAL_IOCX_ADC_Voltage_5V();
			misc_regs.ext_pwr_voltage_value = HAL_IOCX_Get_ExtPower_Voltage();

			/* Process Analog Inputs/Analog Triggers */
		    HAL_IOCX_ADC_Get_Latest_Samples(0, MISC_NUM_ANALOG_INPUTS,
		    	&analog_oversamples[0], NUM_ANALOG_INPUT_OVERSAMPLE_BITS,
		    	&analog_averages[0], NUM_ANALOG_INPUT_AVERAGE_BITS);
		    for ( int i = 0; i < MISC_NUM_ANALOG_INPUTS; i++) {
		    	misc_regs.analog_input_average_value[i] = analog_averages[i];
		    	misc_regs.analog_input_oversample_value[i] = analog_oversamples[i];
		    	if(analog_trigger[i].mode != ANALOG_TRIGGER_DISABLED) {
		    		ANALOG_TRIGGER_STATE curr_state;
		    		if ( uint16_t(analog_averages[i]) <= analog_trigger[i].low_threshold) {
		    			curr_state = ANALOG_TRIGGER_LOW;
		    		} else if ( uint16_t(analog_averages[i]) >= analog_trigger[i].high_threshold) {
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
		    		analog_trigger[i].last_state = curr_state;
		    	}
		    }
		}

		/* External Power Output Current monitoring */
		if ((last_overcurrent_check_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_OVERCURRENT_CHECKS){
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

		/* External Power Input Voltage monitoring */
		if ((last_undervoltage_check_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_UNDERVOLTAGE_CHECKS){
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
	    if ( register_offset >= offsetof(struct MISC_REGS, end_of_bank) ) {
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
	    *size = offsetof(struct MISC_REGS, end_of_bank) - register_offset;
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

