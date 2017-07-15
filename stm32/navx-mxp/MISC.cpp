#include "MISC.h"
#include "navx-mxp_hal.h"
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
}

#define NUM_MS_BETWEEN_SUCCESSIVE_LOOPS 2

_EXTERN_ATTRIB void MISC_loop()
{
	uint32_t curr_loop_timestamp = HAL_GetTick();
	if ( curr_loop_timestamp < last_loop_timestamp) {
		/* Timestamp rollover */
		last_loop_timestamp = curr_loop_timestamp;
	} else {
		if ((curr_loop_timestamp - last_loop_timestamp) >= NUM_MS_BETWEEN_SUCCESSIVE_LOOPS){
			last_loop_timestamp = curr_loop_timestamp;
			/* TODO:  Implement */
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

	    /* Requested data includes rtc timstamp; update w/latest value */
	    if((first_offset <= offsetof(struct MISC_REGS, rtc_time)) &&
	       (last_offset >=
	    		   (offsetof(struct MISC_REGS, rtc_time) +
	    			sizeof(misc_regs.rtc_time)))) {
	    	HAL_RTC_Get_Time(&misc_regs.rtc_time.hours,
	    			&misc_regs.rtc_time.minutes,
	    			&misc_regs.rtc_time.seconds,
	    			&misc_regs.rtc_time.subseconds);
	    }
	    if((first_offset <= offsetof(struct MISC_REGS, rtc_date)) &&
	 	       (last_offset >=
	 	    		   (offsetof(struct MISC_REGS, rtc_date) +
	 	    			sizeof(misc_regs.rtc_date)))) {
	    	HAL_RTC_Get_Date(&misc_regs.rtc_date.weekday,
	    			&misc_regs.rtc_date.date,
	    			&misc_regs.rtc_date.month,
	    			&misc_regs.rtc_date.year);
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
}

static void rtc_time_set_modified(uint8_t first_offset, uint8_t count) {
}

static void rtc_date_set_modified(uint8_t first_offset, uint8_t count) {
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

