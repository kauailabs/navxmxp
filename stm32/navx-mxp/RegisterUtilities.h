/*
 * RegisterUtilities.h
 *
 *  Created on: Aug 13, 2017
 *      Author: Scott
 */

#ifndef REGISTERUTILITIES_H_
#define REGISTERUTILITIES_H_

inline bool get_requested_region_intersection(uint8_t requested_first_offset, uint8_t requested_count,
		uint8_t region_first_offset, uint8_t region_count,
		uint8_t& intersection_first_relative_offset, uint8_t& intersection_count) {
	uint8_t requested_last_offset = requested_first_offset + requested_count - 1;
	uint8_t region_last_offset = region_first_offset + region_count - 1;

	uint8_t intersection_first_absolute_offset;
	uint8_t intersection_last_absolute_offset;

	if ((requested_first_offset <= region_last_offset) &&
		(requested_last_offset >= region_first_offset)) {
		if (requested_first_offset < region_first_offset) {
			intersection_first_absolute_offset = region_first_offset;
		} else {
			intersection_first_absolute_offset = requested_first_offset;
		}
		if (requested_last_offset >= region_last_offset) {
			intersection_last_absolute_offset = region_last_offset;
		} else {
			intersection_last_absolute_offset = requested_last_offset;
		}
		intersection_count = intersection_last_absolute_offset -
				intersection_first_absolute_offset + 1;
		intersection_first_relative_offset =
				intersection_first_absolute_offset - region_first_offset;
		return true;
	} else {
		return false;
	}
}

inline bool get_requested_array_intersection(uint8_t requested_first_offset, uint8_t requested_count,
		uint8_t region_first_offset, uint8_t region_count, uint8_t array_element_size,
		uint8_t& first_array_index, uint8_t& array_item_count) {
	uint8_t intersection_first_relative_offset;
	uint8_t intersection_count;
	if (get_requested_region_intersection(requested_first_offset, requested_count,
			region_first_offset, region_count,
			intersection_first_relative_offset, intersection_count) ) {
		first_array_index = intersection_first_relative_offset / array_element_size;
		array_item_count = intersection_count / array_element_size;
		return true;
	} else {
		return false;
	}
}

#endif /* REGISTERUTILITIES_H_ */
