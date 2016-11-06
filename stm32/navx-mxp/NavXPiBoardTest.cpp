/*
 * NavXPiBoardTest.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: Scott
 */

#include "NavXPiBoardTest.h"
#include "navx-mxp_hal.h"

NavXPiBoardTest::NavXPiBoardTest() {
	/* Configure PWM output frequency */
	/*
	for ( int i = 0; i < HAL_PWM_Get_Num_Channels(); i++ ) {
		HAL_PWM_Set_Rate(i, (i+1)*50,1000);
		HAL_PWM_Enable(i,1);
	}
	*/
	/* Enable PWM on those two channels not overlapping w/Quad Encoders */
	//HAL_PWM_Enable(2,1);
	//HAL_PWM_Enable(3,1);
	/* Enable all Quad Encoder interfaces */
    HAL_QuadEncoder_Enable(0,1);
	HAL_QuadEncoder_Enable(1,1);
	HAL_QuadEncoder_Enable(2,1);
	HAL_QuadEncoder_Enable(3,1);
	/* Enable the ADC */
	//HAL_ADC_Enable(1); /* Danger:  as of 8/27/2016, this was found to be overwriting */
	/* expected memory and causing a hard fault! */
}

NavXPiBoardTest::~NavXPiBoardTest() {
}

#define NUM_AVERAGED_SAMPLES 10

static ADC_Samples test_adc_samples;
uint32_t quad_encoder_count[4];
void NavXPiBoardTest::loop() {
	for ( int i = 0; i < 4; i++ ) {
		quad_encoder_count[i] = HAL_QuadEncoder_Get_Count(i);
	}
	HAL_ADC_Get_Samples(&test_adc_samples, NUM_AVERAGED_SAMPLES);
}
