/* industrial-counter-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * counter.c: Counter driver (with ccu4/ccu8)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "counter.h"

#include "configs/config_counter.h"

#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"

#include "communication.h"

#include "xmc_gpio.h"
#include "xmc_ccu4.h"
#include "xmc_ccu8.h"
#include "xmc1_ccu4_map.h"
#include "xmc_scu.h"
#include "xmc_eru.h"

#include <stdlib.h>

Counter counter;

// Keep the overflows as single global variables to be sure that the compiler
// does not have to dereference anything or similar.
// The following IRQs can be called with a very high frequency.
static uint32_t counter_overflow0 = 0;
static uint32_t counter_overflow1 = 0;
static uint32_t counter_overflow2 = 0;
static uint32_t counter_overflow3 = 0;

static int64_t counter_frequency_current0 = 0;
static int64_t counter_frequency_current1 = 0;
static int64_t counter_frequency_current2 = 0;
static int64_t counter_frequency_current3 = 0;
static int64_t counter_frequency_before0 = 0;
static int64_t counter_frequency_before1 = 0;
static int64_t counter_frequency_before2 = 0;
static int64_t counter_frequency_before3 = 0;

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_21(void) {
	counter_overflow0++;
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_24(void) {
	counter_overflow0--;
}


void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_8(void) {
	counter_overflow1++;
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_13(void) {
	counter_overflow1--;
}


void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_25(void) {
	counter_overflow2++;
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_26(void) {
	counter_overflow2--;
}


void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_0(void) {
	counter_overflow3++;
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_31(void) {
	counter_overflow3--;
}


void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_23(void) {
	counter_frequency_before0 = counter_frequency_current0;
	counter_frequency_current0 = counter_get_count(0);
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_12(void) {
	counter_frequency_before1 = counter_frequency_current1;
	counter_frequency_current1 = counter_get_count(1);
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_6(void) {
	counter_frequency_before2 = counter_frequency_current2;
	counter_frequency_current2 = counter_get_count(2);
}

void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) IRQ_Hdlr_16(void) {
	counter_frequency_before3 = counter_frequency_current3;
	counter_frequency_current3 = counter_get_count(3);
}

void counter_counter_init_0(const bool first) {
	// First we turn the slices off, so we can use this function to reconfigure
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE3);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE2);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE1);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE0);

	// For the first initialization after startup we start with counter = 0
	const int64_t counter_save = first ? 0 : counter_get_count(0);

	XMC_CCU4_DisableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE3_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE2_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE1_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE0_NUMBER);

	// Disable rising/falling count events (otherwise they would stay enabled through a reconfigure)
	XMC_CCU4_SLICE_DisableEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0);
	XMC_CCU4_SLICE_DisableEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1);

	// Set global CCU4 configuration
	XMC_CCU4_Init(COUNTER_IN0_MODULE, XMC_CCU4_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
	XMC_CCU4_StartPrescaler(COUNTER_IN0_MODULE);
	XMC_CCU4_SetModuleClock(COUNTER_IN0_MODULE, XMC_CCU4_CLOCK_SCU);

	// Set timer config
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer_config = {
		.timer_mode            = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable            = false,
		.prescaler_initval     = XMC_CCU4_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU4_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level         = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation   = false
	};
	XMC_CCU4_SLICE_CompareInit(COUNTER_IN0_SLICE0, &timer_config);


	// Set the period/compare values
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN0_SLICE0, 0xFFFF);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN0_SLICE0, 2);

	// Transfer configuration through shadow register
	XMC_CCU4_SetMultiChannelShadowTransferMode(COUNTER_IN0_MODULE, (uint32_t)XMC_CCU4_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE0);
	XMC_CCU4_SLICE_DisableCascadedShadowTransfer(COUNTER_IN0_SLICE0);

	XMC_CCU4_EnableShadowTransfer(COUNTER_IN0_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_0 | XMC_CCU4_SHADOW_TRANSFER_DITHER_SLICE_0 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_0);

	// Configure parameters for the event 0 (rising edge)
	XMC_CCU4_SLICE_EVENT_CONFIG_t event0_config0 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AB,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_EVENT_0, &event0_config0);

	// Configure parameters for the event 1 (falling edge)
	XMC_CCU4_SLICE_EVENT_CONFIG_t event1_config0 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AB,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_EVENT_1, &event1_config0);


	// Set capture config for calculation of duty cycle
	const XMC_CCU4_SLICE_CAPTURE_CONFIG_t capture_config = {
		.fifo_enable         = 0,
		.timer_clear_mode    = XMC_CCU4_SLICE_TIMER_CLEAR_MODE_ALWAYS,
		.same_event          = 0,
		.ignore_full_flag    = 0,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.prescaler_initval   = counter.config_duty_cylce_prescaler[0],
		.float_limit         = 15,
		.timer_concatenation = 0U
	};
	XMC_CCU4_SLICE_CaptureInit(COUNTER_IN0_SLICE0, &capture_config);
	XMC_CCU4_SLICE_Capture0Config(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_EVENT_0);
	XMC_CCU4_SLICE_Capture1Config(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_EVENT_1);


	// Use event0 (rising) for rising count in slice 1
	if((counter.config_count_edge[0] == INDUSTRIAL_COUNTER_COUNT_EDGE_RISING) || (counter.config_count_edge[0] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)) {
		XMC_CCU4_SLICE_EnableEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0);
		XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0, XMC_CCU4_SLICE_SR_ID_1);
	}

	// Use event1 (falling) for falling count in slice 1
	if((counter.config_count_edge[0] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) || (counter.config_count_edge[0] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)){
		XMC_CCU4_SLICE_EnableEvent(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1);
		XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN0_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1, XMC_CCU4_SLICE_SR_ID_1);
	}

	// Enable clock for slice 0
	XMC_CCU4_EnableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE0_NUMBER);


	// Set timer config
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer_config1 = {
		.timer_mode            = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable            = false,
		.prescaler_initval     = XMC_CCU4_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU4_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level         = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation   = false
	};
	XMC_CCU4_SLICE_CompareInit(COUNTER_IN0_SLICE1, &timer_config1);


	// Set the period/compare values
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN0_SLICE1, 0xFFFF);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN0_SLICE1, 0); // Will be overwritten with correct value at the end of initialization

	// Transfer configuration through shadow register
	XMC_CCU4_SetMultiChannelShadowTransferMode(COUNTER_IN0_MODULE, (uint32_t)XMC_CCU4_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE1);
	XMC_CCU4_SLICE_DisableCascadedShadowTransfer(COUNTER_IN0_SLICE1);

	XMC_CCU4_EnableShadowTransfer(COUNTER_IN0_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_DITHER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Configure parameters for the event 0
	// Event 0 corresponds to rising edge, falling edge or both, depending on configuration of slice 0
	XMC_CCU4_SLICE_EVENT_CONFIG_t event0_config1 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_BB,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_EVENT_0, &event0_config1);
	XMC_CCU4_SLICE_CountConfig(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_EVENT_0);


	// Configure direction event
	XMC_CCU4_SLICE_EVENT_CONFIG_t direction_event_config = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AA, // Direction = P3_0
		.edge = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_NONE,
		.level = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	if((counter.config_count_direction[0] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_EXTERNAL_DOWN) || (counter.config_count_direction[0] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN)) {
		direction_event_config.level = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_LOW;
	}

	if((counter.config_count_direction[0] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_UP) || (counter.config_count_direction[0] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN)) {
		direction_event_config.mapped_input = XMC_CCU4_SLICE_INPUT_BD; // Use not connected input for non-external direction control
	}

	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_EVENT_2, &direction_event_config);
	XMC_CCU4_SLICE_DirectionConfig(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_EVENT_2);

	// Use SR 0 and 3 for overflow/underflow handling (we overflow/underflow at timer value 0, see below)
	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_0);
	NVIC_EnableIRQ(21);
	NVIC_SetPriority(21, 0);
	XMC_SCU_SetInterruptControl(21, XMC_SCU_IRQCTRL_CCU41_SR0_IRQ21);

	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_IRQ_ID_ONE_MATCH);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN0_SLICE1, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU4_SLICE_SR_ID_3);
	NVIC_EnableIRQ(24);
	NVIC_SetPriority(24, 0);
	XMC_SCU_SetInterruptControl(24, XMC_SCU_IRQCTRL_CCU41_SR3_IRQ24);


	// Request shadow transfer for the slice 1
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN0_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Enable clock for slice 1
	XMC_CCU4_EnableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE1_NUMBER);


	// Slice 2: Count from 0 to 48000 (1ms)
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer0_config2 = {
		.timer_mode          = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable          = false,
		.prescaler_initval   = XMC_CCU4_SLICE_PRESCALER_2, // Use prescaler 2 to get mclk = fccu4
		.float_limit         = 0U,
		.dither_limit        = 0U,
		.passive_level       = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation = false
	};

	XMC_CCU4_SLICE_CompareInit(COUNTER_IN0_SLICE2, &timer0_config2);
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN0_SLICE2, 48000);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN0_SLICE2, 0);
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN0_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_2 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_2);

	XMC_CCU4_EnableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE2_NUMBER);
	XMC_CCU4_SLICE_ClearTimer(COUNTER_IN0_SLICE2);


	// Slice 3: Concatenate with Slice 2, count for every 1ms (10000 counts per seconds)
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer0_config3 = {
		.timer_mode          = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable          = false,
		.prescaler_initval   = XMC_CCU4_SLICE_PRESCALER_2,
		.float_limit         = 0U,
		.dither_limit        = 0U,
		.passive_level       = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation = true
	};

	XMC_CCU4_SLICE_CompareInit(COUNTER_IN0_SLICE3, &timer0_config3);
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN0_SLICE3, 1 << (counter.config_frequency_integration_time[0] + 7));
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN0_SLICE3, 0);
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN0_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_3 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_3);

	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN0_SLICE3, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN0_SLICE3, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU4_SLICE_SR_ID_2);
	NVIC_EnableIRQ(23);
	NVIC_SetPriority(23, 0);
	XMC_SCU_SetInterruptControl(23, XMC_SCU_IRQCTRL_CCU41_SR2_IRQ23);

	XMC_CCU4_EnableClock(COUNTER_IN0_MODULE, COUNTER_IN0_SLICE3_NUMBER);
	XMC_CCU4_SLICE_ClearTimer(COUNTER_IN0_SLICE3);


	XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE0);
	counter_set_count(0, counter_save);

	// Start the CCU4 Timers
	if(counter.config_active[0]) {
		XMC_CCU4_SLICE_StartTimer(COUNTER_IN0_SLICE0);
	}

	XMC_CCU4_SLICE_StartTimer(COUNTER_IN0_SLICE1);
	XMC_CCU4_SLICE_StartTimer(COUNTER_IN0_SLICE2);
	XMC_CCU4_SLICE_StartTimer(COUNTER_IN0_SLICE3);
}

void counter_counter_init_1(const bool first) {
	// First we turn the slices off, so we can use this function to reconfigure
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE3);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE2);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE1);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE0);

	// For the first initialization after startup we start with counter = 0
	const int64_t counter_save = first ? 0 : counter_get_count(1);

	XMC_CCU8_DisableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE3_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE2_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE1_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE0_NUMBER);

	// Disable rising/falling count events (otherwise they would stay enabled through a reconfigure)
	XMC_CCU8_SLICE_DisableEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0);
	XMC_CCU8_SLICE_DisableEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1);

	// Set global CCU8 configuration
	XMC_CCU8_Init(COUNTER_IN1_MODULE, XMC_CCU8_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
	XMC_CCU8_StartPrescaler(COUNTER_IN1_MODULE);
	XMC_CCU8_SetModuleClock(COUNTER_IN1_MODULE, XMC_CCU8_CLOCK_SCU);


	// Set timer config
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer_config = {
		.timer_mode            = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable        = false,
		.mcm_ch2_enable        = false,
		.slice_status          = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval     = XMC_CCU8_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU8_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level_out0    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm        = false,
		.selector_out0         = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1         = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2         = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3         = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation   = false
	};
	XMC_CCU8_SLICE_CompareInit(COUNTER_IN1_SLICE0, &timer_config);


	// Set the period/compare values
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN1_SLICE0, 0xFFFF);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN1_SLICE0, 2);

	// Transfer configuration through shadow register
	XMC_CCU8_SetMultiChannelShadowTransferMode(COUNTER_IN1_MODULE, XMC_CCU8_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE0);
	XMC_CCU8_SLICE_DisableCascadedShadowTransfer(COUNTER_IN1_SLICE0);

	XMC_CCU8_EnableShadowTransfer(COUNTER_IN1_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_0 | XMC_CCU8_SHADOW_TRANSFER_DITHER_SLICE_0 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_0);

	// Configure parameters for the event 0 (rising edge)
	XMC_CCU8_SLICE_EVENT_CONFIG_t event0_config0 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_EVENT_0, &event0_config0);

	// Configure parameters for the event 1 (falling edge)
	XMC_CCU8_SLICE_EVENT_CONFIG_t event1_config0 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_EVENT_1, &event1_config0);


	// Set capture config for calculation of duty cycle
	const XMC_CCU8_SLICE_CAPTURE_CONFIG_t capture_config = {
		.fifo_enable         = 0,
		.timer_clear_mode    = XMC_CCU8_SLICE_TIMER_CLEAR_MODE_ALWAYS,
		.same_event          = 0,
		.ignore_full_flag    = 0,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.prescaler_initval   = counter.config_duty_cylce_prescaler[1],
		.float_limit         = 15,
		.timer_concatenation = 0U
	};
	XMC_CCU8_SLICE_CaptureInit(COUNTER_IN1_SLICE0, &capture_config);
	XMC_CCU8_SLICE_Capture0Config(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_EVENT_0);
	XMC_CCU8_SLICE_Capture1Config(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_EVENT_1);


	// Use event0 (rising) for rising count in slice 1
	if((counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_RISING) || (counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)) {
		XMC_CCU8_SLICE_EnableEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0);
		XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0, XMC_CCU8_SLICE_SR_ID_2);
	}

	// Use event1 (falling) for falling count in slice 1
	if((counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) || (counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)){
		XMC_CCU8_SLICE_EnableEvent(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1);
		XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN1_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1, XMC_CCU8_SLICE_SR_ID_2);
	}


	// Enable clock for slice 0
	XMC_CCU8_EnableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE0_NUMBER);


	// Set timer config
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer_config1 = {
		.timer_mode            = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable        = false,
		.mcm_ch2_enable        = false,
		.slice_status          = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval     = XMC_CCU8_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU8_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level_out0    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm        = false,
		.selector_out0         = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1         = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2         = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3         = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation   = false
	};
	XMC_CCU8_SLICE_CompareInit(COUNTER_IN1_SLICE1, &timer_config1);


	// Set the period/compare values
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN1_SLICE1, 0xFFFF);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN1_SLICE1, 0); // Will be overwritten with correct value at the end of initialization

	// Transfer configuration through shadow register
	XMC_CCU8_SetMultiChannelShadowTransferMode(COUNTER_IN1_MODULE, (uint32_t)XMC_CCU8_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE1);
	XMC_CCU8_SLICE_DisableCascadedShadowTransfer(COUNTER_IN1_SLICE1);

	XMC_CCU8_EnableShadowTransfer(COUNTER_IN1_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_DITHER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Configure parameters for the event 0
	XMC_CCU8_SLICE_EVENT_CONFIG_t event0_config1 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};

	if(counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) {
		event0_config1.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE;
	} else if(counter.config_count_edge[1] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH) {
		event0_config1.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_DUAL_EDGE;
	}

	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_EVENT_0, &event0_config1);
	XMC_CCU8_SLICE_CountConfig(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_EVENT_0);


	// Configure direction event
	XMC_CCU8_SLICE_EVENT_CONFIG_t direction_event_config = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AZ, /// Use not connected input for non-external direction control
		.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_NONE,
		.level = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	if(counter.config_count_direction[1] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN) {
		direction_event_config.level = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_LOW;
	}

	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_EVENT_2, &direction_event_config);
	XMC_CCU8_SLICE_DirectionConfig(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_EVENT_2);

	// Use SR 0 and 3 for overflow/underflow handling (we overflow/underflow at timer value 2, see below)
	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_1);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_1, XMC_CCU8_SLICE_SR_ID_0);
	NVIC_EnableIRQ(8);
	NVIC_SetPriority(8, 0);
	XMC_SCU_SetInterruptControl(8, XMC_SCU_IRQCTRL_CCU80_SR0_IRQ8);

	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_IRQ_ID_ONE_MATCH);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN1_SLICE1, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_1);
	NVIC_EnableIRQ(13);
	NVIC_SetPriority(13, 0);
	XMC_SCU_SetInterruptControl(13, XMC_SCU_IRQCTRL_CCU80_SR1_IRQ13);


	// Request shadow transfer for the slice 1
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN1_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Enable clock for slice 1
	XMC_CCU8_EnableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE1_NUMBER);



	// Slice 2: Count from 0 to 4800 (1ms)
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer0_config2 = {
		.timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable      = false,
		.mcm_ch2_enable      = false,
		.slice_status        = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval   = XMC_CCU8_SLICE_PRESCALER_2, // Use prescaler 2 to get mclk = fCCU8
		.float_limit         = XMC_CCU8_SLICE_PRESCALER_1,
		.dither_limit        = 0,
		.passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm      = false,
		.selector_out0       = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1       = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2       = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3       = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation = false
	};

	XMC_CCU8_SLICE_CompareInit(COUNTER_IN1_SLICE2, &timer0_config2);
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN1_SLICE2, 48000);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN1_SLICE2, 0);
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN1_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_2 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_2);

	XMC_CCU8_EnableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE2_NUMBER);
	XMC_CCU8_SLICE_ClearTimer(COUNTER_IN1_SLICE2);


	// Slice 3: Concatenate with Slice 2, count for every 1ms (10000 counts per seconds)
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer0_config3 = {
		.timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable      = false,
		.mcm_ch2_enable      = false,
		.slice_status        = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval   = XMC_CCU8_SLICE_PRESCALER_2,
		.float_limit         = XMC_CCU8_SLICE_PRESCALER_1,
		.dither_limit        = 0,
		.passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm      = false,
		.selector_out0       = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1       = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2       = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3       = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation = true
	};

	XMC_CCU8_SLICE_CompareInit(COUNTER_IN1_SLICE3, &timer0_config3);
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN1_SLICE3, 1 << (counter.config_frequency_integration_time[1] + 7));
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN1_SLICE3, 0);
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN1_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_3 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_3);

	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN1_SLICE3, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN1_SLICE3, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_3);
	XMC_CCU8_EnableClock(COUNTER_IN1_MODULE, COUNTER_IN1_SLICE3_NUMBER);
	XMC_CCU8_SLICE_ClearTimer(COUNTER_IN1_SLICE3);


	// Idea: CCU80.3 period match -> CCU80.SR3 -> ERU0.OGU33 -> ERU0.IDOUT -> Interrupt
	const XMC_ERU_OGU_CONFIG_t ogu_config = {
		.peripheral_trigger       = XMC_ERU_OGU_PERIPHERAL_TRIGGER3,
		.enable_pattern_detection = false,
		.service_request          = XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER,
		.pattern_detection_input  = 0 //XMC_ERU_OGU_PATTERN_DETECTION_INPUT0,
	};

	XMC_ERU_OGU_Init(XMC_ERU0, 3, &ogu_config);


	NVIC_SetPriority(12, 0);
	XMC_SCU_SetInterruptControl(12, XMC_SCU_IRQCTRL_ERU0_SR3_IRQ12);
	NVIC_ClearPendingIRQ(12);
	NVIC_EnableIRQ(12);


	XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE0);
	counter_set_count(1, counter_save);

	// Start the CCU8 Timers
	if(counter.config_active[1]) {
		XMC_CCU8_SLICE_StartTimer(COUNTER_IN1_SLICE0);
	}

	XMC_CCU8_SLICE_StartTimer(COUNTER_IN1_SLICE1);
	XMC_CCU8_SLICE_StartTimer(COUNTER_IN1_SLICE2);
	XMC_CCU8_SLICE_StartTimer(COUNTER_IN1_SLICE3);
}

void counter_counter_init_2(const bool first) {
	// First we turn the slices off, so we can use this function to reconfigure
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE3);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE2);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE1);
	XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE0);

	// For the first initialization after startup we start with counter = 0
	const int64_t counter_save = first ? 0 : counter_get_count(2);

	XMC_CCU8_DisableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE3_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE2_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE1_NUMBER);
	XMC_CCU8_DisableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE0_NUMBER);

	// Disable rising/falling count events (otherwise they would stay enabled through a reconfigure)
	XMC_CCU8_SLICE_DisableEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0);
	XMC_CCU8_SLICE_DisableEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1);

	// Set global CCU8 configuration
	XMC_CCU8_Init(COUNTER_IN2_MODULE, XMC_CCU8_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
	XMC_CCU8_StartPrescaler(COUNTER_IN2_MODULE);
	XMC_CCU8_SetModuleClock(COUNTER_IN2_MODULE, XMC_CCU8_CLOCK_SCU);


	// Set timer config
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer_config = {
		.timer_mode            = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable        = false,
		.mcm_ch2_enable        = false,
		.slice_status          = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval     = XMC_CCU8_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU8_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level_out0    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm        = false,
		.selector_out0         = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1         = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2         = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3         = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation   = false
	};
	XMC_CCU8_SLICE_CompareInit(COUNTER_IN2_SLICE0, &timer_config);


	// Set the period/compare values
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN2_SLICE0, 0xFFFF);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN2_SLICE0, 2);

	// Transfer configuration through shadow register
	XMC_CCU8_SetMultiChannelShadowTransferMode(COUNTER_IN2_MODULE, XMC_CCU8_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE0);
	XMC_CCU8_SLICE_DisableCascadedShadowTransfer(COUNTER_IN2_SLICE0);

	XMC_CCU8_EnableShadowTransfer(COUNTER_IN2_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_0 | XMC_CCU8_SHADOW_TRANSFER_DITHER_SLICE_0 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_0);

	// Configure parameters for the event 0 (rising edge)
	XMC_CCU8_SLICE_EVENT_CONFIG_t event0_config0 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_EVENT_0, &event0_config0);

	// Configure parameters for the event 1 (falling edge)
	XMC_CCU8_SLICE_EVENT_CONFIG_t event1_config0 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_EVENT_1, &event1_config0);


	// Set capture config for calculation of duty cycle
	const XMC_CCU8_SLICE_CAPTURE_CONFIG_t capture_config = {
		.fifo_enable         = 0,
		.timer_clear_mode    = XMC_CCU8_SLICE_TIMER_CLEAR_MODE_ALWAYS,
		.same_event          = 0,
		.ignore_full_flag    = 0,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.prescaler_initval   = counter.config_duty_cylce_prescaler[1],
		.float_limit         = 15,
		.timer_concatenation = 0U
	};
	XMC_CCU8_SLICE_CaptureInit(COUNTER_IN2_SLICE0, &capture_config);
	XMC_CCU8_SLICE_Capture0Config(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_EVENT_0);
	XMC_CCU8_SLICE_Capture1Config(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_EVENT_1);


	// Use event0 (rising) for rising count in slice 1
	if((counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_RISING) || (counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)) {
		XMC_CCU8_SLICE_EnableEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0);
		XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT0, XMC_CCU8_SLICE_SR_ID_2);
	}

	// Use event1 (falling) for falling count in slice 1
	if((counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) || (counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)){
		XMC_CCU8_SLICE_EnableEvent(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1);
		XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN2_SLICE0, XMC_CCU8_SLICE_IRQ_ID_EVENT1, XMC_CCU8_SLICE_SR_ID_2);
	}


	// Enable clock for slice 0
	XMC_CCU8_EnableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE0_NUMBER);


	// Set timer config
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer_config1 = {
		.timer_mode            = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable        = false,
		.mcm_ch2_enable        = false,
		.slice_status          = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval     = XMC_CCU8_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU8_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level_out0    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3    = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm        = false,
		.selector_out0         = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1         = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2         = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3         = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation   = false
	};
	XMC_CCU8_SLICE_CompareInit(COUNTER_IN2_SLICE1, &timer_config1);


	// Set the period/compare values
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN2_SLICE1, 0xFFFF);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN2_SLICE1, 0); // Will be overwritten with correct value at the end of initialization

	// Transfer configuration through shadow register
	XMC_CCU8_SetMultiChannelShadowTransferMode(COUNTER_IN2_MODULE, (uint32_t)XMC_CCU8_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE1);
	XMC_CCU8_SLICE_DisableCascadedShadowTransfer(COUNTER_IN2_SLICE1);

	XMC_CCU8_EnableShadowTransfer(COUNTER_IN2_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_DITHER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Configure parameters for the event 0
	XMC_CCU8_SLICE_EVENT_CONFIG_t event0_config1 = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AA,
		.edge         = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};

	if(counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) {
		event0_config1.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE;
	} else if(counter.config_count_edge[2] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH) {
		event0_config1.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_DUAL_EDGE;
	}

	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_EVENT_0, &event0_config1);
	XMC_CCU8_SLICE_CountConfig(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_EVENT_0);


	// Configure direction event
	XMC_CCU8_SLICE_EVENT_CONFIG_t direction_event_config = {
		.mapped_input = XMC_CCU8_SLICE_INPUT_AZ, /// Use not connected input for non-external direction control
		.edge = XMC_CCU8_SLICE_EVENT_EDGE_SENSITIVITY_NONE,
		.level = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration = XMC_CCU8_SLICE_EVENT_FILTER_DISABLED
	};
	if(counter.config_count_direction[2] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN) {
		direction_event_config.level = XMC_CCU8_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_LOW;
	}

	XMC_CCU8_SLICE_ConfigureEvent(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_EVENT_2, &direction_event_config);
	XMC_CCU8_SLICE_DirectionConfig(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_EVENT_2);

	// Use SR 0 and 1 for overflow/underflow handling (we overflow/underflow at timer value 2, see below)
	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_1);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_1, XMC_CCU8_SLICE_SR_ID_0);
	NVIC_EnableIRQ(25);
	NVIC_SetPriority(25, 0);
	XMC_SCU_SetInterruptControl(25, XMC_SCU_IRQCTRL_CCU81_SR0_IRQ25);

	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_IRQ_ID_ONE_MATCH);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN2_SLICE1, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_1);
	NVIC_EnableIRQ(26);
	NVIC_SetPriority(26, 0);
	XMC_SCU_SetInterruptControl(26, XMC_SCU_IRQCTRL_CCU81_SR1_IRQ26);


	// Request shadow transfer for the slice 1
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN2_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_1 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Enable clock for slice 1
	XMC_CCU8_EnableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE1_NUMBER);



	// Slice 2: Count from 0 to 4800 (1ms)
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer0_config2 = {
		.timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable      = false,
		.mcm_ch2_enable      = false,
		.slice_status        = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval   = XMC_CCU8_SLICE_PRESCALER_2, // Use prescaler 2 to get mclk = fCCU8
		.float_limit         = XMC_CCU8_SLICE_PRESCALER_1,
		.dither_limit        = 0,
		.passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm      = false,
		.selector_out0       = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1       = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2       = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3       = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation = false
	};

	XMC_CCU8_SLICE_CompareInit(COUNTER_IN2_SLICE2, &timer0_config2);
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN2_SLICE2, 48000);
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN2_SLICE2, 0);
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN2_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_2 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_2);

	XMC_CCU8_EnableClock(COUNTER_IN2_MODULE, COUNTER_IN2_SLICE2_NUMBER);
	XMC_CCU8_SLICE_ClearTimer(COUNTER_IN2_SLICE2);


	// Slice 3: Concatenate with Slice 2, count for every 1ms (10000 counts per seconds)
	XMC_CCU8_SLICE_COMPARE_CONFIG_t timer0_config3 = {
		.timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU8_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_ch1_enable      = false,
		.mcm_ch2_enable      = false,
		.slice_status        = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
		.prescaler_initval   = XMC_CCU8_SLICE_PRESCALER_2,
		.float_limit         = XMC_CCU8_SLICE_PRESCALER_1,
		.dither_limit        = 0,
		.passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm      = false,
		.selector_out0       = XMC_CCU8_SOURCE_OUT0_ST1,
		.selector_out1       = XMC_CCU8_SOURCE_OUT1_INV_ST1,
		.selector_out2       = XMC_CCU8_SOURCE_OUT2_ST2,
		.selector_out3       = XMC_CCU8_SOURCE_OUT3_INV_ST2,
		.timer_concatenation = true
	};

	XMC_CCU8_SLICE_CompareInit(COUNTER_IN2_SLICE3, &timer0_config3);
	XMC_CCU8_SLICE_SetTimerPeriodMatch(COUNTER_IN2_SLICE3, 1 << (counter.config_frequency_integration_time[2] + 7));
	XMC_CCU8_SLICE_SetTimerCompareMatchChannel1(COUNTER_IN2_SLICE3, 0);
	XMC_CCU8_EnableShadowTransfer(COUNTER_IN2_MODULE, XMC_CCU8_SHADOW_TRANSFER_SLICE_3 | XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_3);

	XMC_CCU8_SLICE_EnableEvent(COUNTER_IN2_SLICE3, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH);
	XMC_CCU8_SLICE_SetInterruptNode(COUNTER_IN2_SLICE3, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_3);
	XMC_CCU8_EnableClock(COUNTER_IN2_MODULE, COUNTER_IN1_SLICE3_NUMBER);
	XMC_CCU8_SLICE_ClearTimer(COUNTER_IN2_SLICE3);


	// Idea: CCU81.3 period match -> CCU81.SR3 -> ERU1.OGU33 -> ERU1.IDOUT -> Interrupt
	const XMC_ERU_OGU_CONFIG_t ogu_config = {
		.peripheral_trigger       = XMC_ERU_OGU_PERIPHERAL_TRIGGER3,
		.enable_pattern_detection = false,
		.service_request          = XMC_ERU_OGU_SERVICE_REQUEST_ON_TRIGGER,
		.pattern_detection_input  = 0 //XMC_ERU_OGU_PATTERN_DETECTION_INPUT0,
	};

	XMC_ERU_OGU_Init(XMC_ERU1, 3, &ogu_config);


	NVIC_SetPriority(6, 0);
	XMC_SCU_SetInterruptControl(6, XMC_SCU_IRQCTRL_ERU1_SR3_IRQ6);
	NVIC_ClearPendingIRQ(6);
	NVIC_EnableIRQ(6);


	XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE0);
	counter_set_count(2, counter_save);

	// Start the CCU8 Timers
	if(counter.config_active[2]) {
		XMC_CCU8_SLICE_StartTimer(COUNTER_IN2_SLICE0);
	}

	XMC_CCU8_SLICE_StartTimer(COUNTER_IN2_SLICE1);
	XMC_CCU8_SLICE_StartTimer(COUNTER_IN2_SLICE2);
	XMC_CCU8_SLICE_StartTimer(COUNTER_IN2_SLICE3);
}

void counter_counter_init_3(const bool first) {
	// First we turn the slices off, so we can use this function to reconfigure
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE3);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE2);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE1);
	XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE0);

	// For the first initialization after startup we start with counter = 0
	const int64_t counter_save = first ? 0 : counter_get_count(3);

	XMC_CCU4_DisableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE3_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE2_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE1_NUMBER);
	XMC_CCU4_DisableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE0_NUMBER);

	// Disable rising/falling count events (otherwise they would stay enabled through a reconfigure)
	XMC_CCU4_SLICE_DisableEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0);
	XMC_CCU4_SLICE_DisableEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1);

	// Set global CCU4 configuration
	XMC_CCU4_Init(COUNTER_IN3_MODULE, XMC_CCU4_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
	XMC_CCU4_StartPrescaler(COUNTER_IN3_MODULE);
	XMC_CCU4_SetModuleClock(COUNTER_IN3_MODULE, XMC_CCU4_CLOCK_SCU);

	// Set timer config
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer_config = {
		.timer_mode            = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable            = false,
		.prescaler_initval     = XMC_CCU4_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU4_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level         = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation   = false
	};
	XMC_CCU4_SLICE_CompareInit(COUNTER_IN3_SLICE0, &timer_config);


	// Set the period/compare values
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN3_SLICE0, 0xFFFF);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN3_SLICE0, 2);

	// Transfer configuration through shadow register
	XMC_CCU4_SetMultiChannelShadowTransferMode(COUNTER_IN3_MODULE, XMC_CCU4_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE0);
	XMC_CCU4_SLICE_DisableCascadedShadowTransfer(COUNTER_IN3_SLICE0);

	XMC_CCU4_EnableShadowTransfer(COUNTER_IN3_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_0 | XMC_CCU4_SHADOW_TRANSFER_DITHER_SLICE_0 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_0);

	// Configure parameters for the event 0 (rising edge)
	XMC_CCU4_SLICE_EVENT_CONFIG_t event0_config0 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AC,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_EVENT_0, &event0_config0);

	// Configure parameters for the event 1 (falling edge)
	XMC_CCU4_SLICE_EVENT_CONFIG_t event1_config0 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AC,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_FALLING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_EVENT_1, &event1_config0);


	// Set capture config for calculation of duty cycle
	const XMC_CCU4_SLICE_CAPTURE_CONFIG_t capture_config = {
		.fifo_enable         = 0,
		.timer_clear_mode    = XMC_CCU4_SLICE_TIMER_CLEAR_MODE_ALWAYS,
		.same_event          = 0,
		.ignore_full_flag    = 0,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.prescaler_initval   = counter.config_duty_cylce_prescaler[3],
		.float_limit         = 15,
		.timer_concatenation = 0U
	};
	XMC_CCU4_SLICE_CaptureInit(COUNTER_IN3_SLICE0, &capture_config);
	XMC_CCU4_SLICE_Capture0Config(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_EVENT_0);
	XMC_CCU4_SLICE_Capture1Config(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_EVENT_1);


	// Use event0 (rising) for rising count in slice 1
	if((counter.config_count_edge[3] == INDUSTRIAL_COUNTER_COUNT_EDGE_RISING) || (counter.config_count_edge[3] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)) {
		XMC_CCU4_SLICE_EnableEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0);
		XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT0, XMC_CCU4_SLICE_SR_ID_1);
	}

	// Use event1 (falling) for falling count in slice 1
	if((counter.config_count_edge[3] == INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING) || (counter.config_count_edge[3] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH)){
		XMC_CCU4_SLICE_EnableEvent(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1);
		XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN3_SLICE0, XMC_CCU4_SLICE_IRQ_ID_EVENT1, XMC_CCU4_SLICE_SR_ID_1);
	}

	// Enable clock for slice 0
	XMC_CCU4_EnableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE0_NUMBER);


	// Set timer config
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer_config1 = {
		.timer_mode            = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot              = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear     = false,
		.dither_timer_period   = false,
		.dither_duty_cycle     = false,
		.prescaler_mode        = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable            = false,
		.prescaler_initval     = XMC_CCU4_SLICE_PRESCALER_1,
		.float_limit           = XMC_CCU4_SLICE_PRESCALER_32768,
		.dither_limit          = 0,
		.passive_level         = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation   = false
	};
	XMC_CCU4_SLICE_CompareInit(COUNTER_IN3_SLICE1, &timer_config1);


	// Set the period/compare values
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN3_SLICE1, 0xFFFF);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN3_SLICE1, 0); // Will be overwritten with correct value at the end of initialization

	// Transfer configuration through shadow register
	XMC_CCU4_SetMultiChannelShadowTransferMode(COUNTER_IN3_MODULE, (uint32_t)XMC_CCU4_MULTI_CHANNEL_SHADOW_TRANSFER_SW_SLICE1);
	XMC_CCU4_SLICE_DisableCascadedShadowTransfer(COUNTER_IN3_SLICE1);

	XMC_CCU4_EnableShadowTransfer(COUNTER_IN3_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_DITHER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Configure parameters for the event 0
	// Event 0 corresponds to rising edge, falling edge or both, depending on configuration of slice 0
	XMC_CCU4_SLICE_EVENT_CONFIG_t event0_config1 = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_BB,
		.edge         = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_RISING_EDGE,
		.level        = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration     = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_EVENT_0, &event0_config1);
	XMC_CCU4_SLICE_CountConfig(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_EVENT_0);


	// Configure direction event
	XMC_CCU4_SLICE_EVENT_CONFIG_t direction_event_config = {
		.mapped_input = XMC_CCU4_SLICE_INPUT_AA, // Direction = P0_12
		.edge = XMC_CCU4_SLICE_EVENT_EDGE_SENSITIVITY_NONE,
		.level = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_HIGH,
		.duration = XMC_CCU4_SLICE_EVENT_FILTER_DISABLED
	};
	if((counter.config_count_direction[3] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_EXTERNAL_DOWN) || (counter.config_count_direction[3] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN)) {
		direction_event_config.level = XMC_CCU4_SLICE_EVENT_LEVEL_SENSITIVITY_ACTIVE_LOW;
	}

	if((counter.config_count_direction[3] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_UP) || (counter.config_count_direction[3] == INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN)) {
		direction_event_config.mapped_input = XMC_CCU4_SLICE_INPUT_BD; // Use not connected input for non-external direction control
	}

	XMC_CCU4_SLICE_ConfigureEvent(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_EVENT_2, &direction_event_config);
	XMC_CCU4_SLICE_DirectionConfig(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_EVENT_2);

	// Use SR 0 and 3 for overflow/underflow handling (we overflow/underflow at timer value 2, see below)
	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_0);
	NVIC_EnableIRQ(0);
	NVIC_SetPriority(0, 0);
	XMC_SCU_SetInterruptControl(0, XMC_SCU_IRQCTRL_CCU40_SR0_IRQ0);

	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_IRQ_ID_ONE_MATCH);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN3_SLICE1, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU4_SLICE_SR_ID_3);
	NVIC_EnableIRQ(31);
	NVIC_SetPriority(31, 0);
	XMC_SCU_SetInterruptControl(31, XMC_SCU_IRQCTRL_CCU40_SR3_IRQ24);


	// Request shadow transfer for the slice 1
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN3_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_1 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_1);

	// Enable clock for slice 1
	XMC_CCU4_EnableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE1_NUMBER);




	// Slice 2: Count from 0 to 4800 (1ms)
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer0_config2 = {
		.timer_mode          = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable          = false,
		.prescaler_initval   = XMC_CCU4_SLICE_PRESCALER_2, // Use prescaler 2 to get mclk = fccu4
		.float_limit         = 0U,
		.dither_limit        = 0U,
		.passive_level       = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation = false
	};

	XMC_CCU4_SLICE_CompareInit(COUNTER_IN3_SLICE2, &timer0_config2);
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN3_SLICE2, 48000);
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN3_SLICE2, 0);
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN3_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_2 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_2);

	XMC_CCU4_EnableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE2_NUMBER);
	XMC_CCU4_SLICE_ClearTimer(COUNTER_IN3_SLICE2);


	// Slice 3: Concatenate with Slice 2, count for every 1ms (10000 counts per seconds)
	XMC_CCU4_SLICE_COMPARE_CONFIG_t timer0_config3 = {
		.timer_mode          = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
		.shadow_xfer_clear   = false,
		.dither_timer_period = false,
		.dither_duty_cycle   = false,
		.prescaler_mode      = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
		.mcm_enable          = false,
		.prescaler_initval   = XMC_CCU4_SLICE_PRESCALER_2,
		.float_limit         = 0U,
		.dither_limit        = 0U,
		.passive_level       = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.timer_concatenation = true
	};

	XMC_CCU4_SLICE_CompareInit(COUNTER_IN3_SLICE3, &timer0_config3);
	XMC_CCU4_SLICE_SetTimerPeriodMatch(COUNTER_IN3_SLICE3, 1 << (counter.config_frequency_integration_time[3] + 7));
	XMC_CCU4_SLICE_SetTimerCompareMatch(COUNTER_IN3_SLICE3, 0);
	XMC_CCU4_EnableShadowTransfer(COUNTER_IN3_MODULE, XMC_CCU4_SHADOW_TRANSFER_SLICE_3 | XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_3);

	XMC_CCU4_SLICE_EnableEvent(COUNTER_IN3_SLICE3, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH);
	XMC_CCU4_SLICE_SetInterruptNode(COUNTER_IN3_SLICE3, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU4_SLICE_SR_ID_2);
	NVIC_EnableIRQ(16);
	NVIC_SetPriority(16, 0);
	XMC_SCU_SetInterruptControl(16, XMC_SCU_IRQCTRL_CCU40_SR2_IRQ16);

	XMC_CCU4_EnableClock(COUNTER_IN3_MODULE, COUNTER_IN3_SLICE3_NUMBER);
	XMC_CCU4_SLICE_ClearTimer(COUNTER_IN3_SLICE3);



	XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE0);
	counter_set_count(3, counter_save);

	// Start the CCU4 Timers
	if(counter.config_active[3]) {
		XMC_CCU4_SLICE_StartTimer(COUNTER_IN3_SLICE0);
	}

	XMC_CCU4_SLICE_StartTimer(COUNTER_IN3_SLICE1);
	XMC_CCU4_SLICE_StartTimer(COUNTER_IN3_SLICE2);
	XMC_CCU4_SLICE_StartTimer(COUNTER_IN3_SLICE3);
}

void counter_counter_init(const uint8_t pin, const bool first) {
	switch(pin) {
		case 0: counter_counter_init_0(first); break;
		case 1: counter_counter_init_1(first); break;
		case 2: counter_counter_init_2(first); break;
		case 3: counter_counter_init_3(first); break;
	}
}

void counter_init(void) {
	// LED pin configuration
	XMC_GPIO_CONFIG_t led_pin_config = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};

	// Counter input pin configuration
	XMC_GPIO_CONFIG_t counter_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	// Configure GPIO pins
	XMC_GPIO_Init(COUNTER_STATUS0_LED_PIN, &led_pin_config);
	XMC_GPIO_Init(COUNTER_STATUS1_LED_PIN, &led_pin_config);
	XMC_GPIO_Init(COUNTER_STATUS2_LED_PIN, &led_pin_config);
	XMC_GPIO_Init(COUNTER_STATUS3_LED_PIN, &led_pin_config);

	XMC_GPIO_Init(COUNTER_IN0_PIN, &counter_pin_config);
	XMC_GPIO_Init(COUNTER_IN1_PIN, &counter_pin_config);
	XMC_GPIO_Init(COUNTER_IN2_PIN, &counter_pin_config);
	XMC_GPIO_Init(COUNTER_IN3_PIN, &counter_pin_config);

	XMC_GPIO_Init(COUNTER_IN0_POSIF_PIN, &counter_pin_config);
	XMC_GPIO_Init(COUNTER_IN1_POSIF_PIN, &counter_pin_config);
	XMC_GPIO_Init(COUNTER_IN2_POSIF_PIN, &counter_pin_config);

	for(uint8_t pin = 0; pin < COUNTER_NUM; pin++) {
		counter.config_update[pin]                     = false;
		counter.config_count_edge[pin]                 = INDUSTRIAL_COUNTER_COUNT_EDGE_RISING;
		counter.config_count_direction[pin]            = INDUSTRIAL_COUNTER_COUNT_DIRECTION_UP;
		counter.config_duty_cylce_prescaler[pin]       = INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_1;
		counter.config_frequency_integration_time[pin] = INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_1024_MS;
		counter.config_active[pin]                     = true;
		counter.last_duty_cycle[pin]                   = 0;
		counter.last_period[pin]                       = 0;
		counter.last_cv1[pin]                          = 0;
		counter.last_cv3[pin]                          = 0;

		counter_counter_init(pin, true);
	}
}

void counter_set_active(const uint8_t pin_mask) {
	// TODO: Can we somehow do this atomically through shadow registers?

	uint8_t current = counter_get_active();
	for(uint8_t pin = 0; pin < COUNTER_NUM; pin++) {
		if((current & (1 << pin)) != (pin_mask & (1 << pin))) {
			if(pin_mask & (1 << pin)) {
				switch(pin) {
					case 0: XMC_CCU4_SLICE_StartTimer(COUNTER_IN0_SLICE1); break;
					case 1: XMC_CCU8_SLICE_StartTimer(COUNTER_IN1_SLICE1); break;
					case 2: XMC_CCU8_SLICE_StartTimer(COUNTER_IN2_SLICE1); break;
					case 3: XMC_CCU4_SLICE_StartTimer(COUNTER_IN3_SLICE1); break;
				}
			} else {
				switch(pin) {
					case 0: XMC_CCU4_SLICE_StopTimer(COUNTER_IN0_SLICE1); break;
					case 1: XMC_CCU8_SLICE_StopTimer(COUNTER_IN1_SLICE1); break;
					case 2: XMC_CCU8_SLICE_StopTimer(COUNTER_IN2_SLICE1); break;
					case 3: XMC_CCU4_SLICE_StopTimer(COUNTER_IN3_SLICE1); break;
				}
			}
		}
	}
}

uint8_t counter_get_active(void) {
	return ((COUNTER_IN0_SLICE1->TCST & CCU4_CC4_TCST_TRB_Msk) << 0) |
	       ((COUNTER_IN1_SLICE1->TCST & CCU8_CC8_TCST_TRB_Msk) << 1) |
	       ((COUNTER_IN2_SLICE1->TCST & CCU8_CC8_TCST_TRB_Msk) << 2) |
	       ((COUNTER_IN3_SLICE1->TCST & CCU4_CC4_TCST_TRB_Msk) << 3);
}

// Note: This function assumes that the slice is not currently active!
void counter_set_count(const uint8_t pin, const int64_t count) {
    // We start with a offset of 2, since it allows for easy overflow/underflow handling.
    // The CCU4 has an interrupt for the decrease from 2 to 1 and we can define a match for the
    // increase from 1 to 2. The overflow from 0xFFFF to 0 and vice versa can't be easily captured.
	uint16_t slice_value = (count+2) & 0xFFFF;
	uint32_t overflow_value = (int32_t)((count+2) >> 16);

	switch(pin) {
		case 0: XMC_CCU4_SLICE_SetTimerValue(COUNTER_IN0_SLICE1, slice_value); counter_overflow0 = overflow_value; break;
		case 1: XMC_CCU8_SLICE_SetTimerValue(COUNTER_IN1_SLICE1, slice_value); counter_overflow1 = overflow_value; break;
		case 2: XMC_CCU8_SLICE_SetTimerValue(COUNTER_IN2_SLICE1, slice_value); counter_overflow2 = overflow_value; break;
		case 3: XMC_CCU4_SLICE_SetTimerValue(COUNTER_IN3_SLICE1, slice_value); counter_overflow3 = overflow_value; break;
	}
}

int64_t counter_get_count(const uint8_t pin) {
	int32_t time_high;
	uint16_t time_low;
	switch(pin) {
		case 0: {
			time_high = counter_overflow0;
			time_low = XMC_CCU4_SLICE_GetTimerValue(COUNTER_IN0_SLICE1);

			while(time_high != (int32_t)counter_overflow0) {
				time_high = counter_overflow0;
				time_low = XMC_CCU4_SLICE_GetTimerValue(COUNTER_IN0_SLICE1);
			}

			break;
		}

		case 1: {
			time_high = counter_overflow1;
			time_low = XMC_CCU8_SLICE_GetTimerValue(COUNTER_IN1_SLICE1);

			while(time_high != (int32_t)counter_overflow1) {
				time_high = counter_overflow1;
				time_low = XMC_CCU8_SLICE_GetTimerValue(COUNTER_IN1_SLICE1);
			}

			break;
		}

		case 2: {
			time_high = counter_overflow2;
			time_low = XMC_CCU8_SLICE_GetTimerValue(COUNTER_IN2_SLICE1);

			while(time_high != (int32_t)counter_overflow2) {
				time_high = counter_overflow2;
				time_low = XMC_CCU8_SLICE_GetTimerValue(COUNTER_IN2_SLICE1);
			}

			break;
		}

		case 3: {
			time_high = counter_overflow3;
			time_low = XMC_CCU4_SLICE_GetTimerValue(COUNTER_IN3_SLICE1);

			while(time_high != (int32_t)counter_overflow3) {
				time_high = counter_overflow3;
				time_low = XMC_CCU4_SLICE_GetTimerValue(COUNTER_IN3_SLICE1);
			}

			break;
		}

		default: return 0;
	}

	// The overflow and starting value is set to 2. Remove it from counter as offset
	int64_t time = (((int64_t)time_high)*(1 << 16)) + time_low - 2;

	return time;
}


bool counter_get_pin_value(const uint8_t pin) {
	switch(pin) {
		case 0: return XMC_GPIO_GetInput(COUNTER_IN0_PIN);
		case 1: return XMC_GPIO_GetInput(COUNTER_IN1_PIN);
		case 2: return XMC_GPIO_GetInput(COUNTER_IN2_PIN);
		case 3: return XMC_GPIO_GetInput(COUNTER_IN3_PIN);
	}

	return false;
}

void counter_get_duty_cycle_and_period(const uint8_t pin, uint16_t *duty_cycle, uint64_t *period) {
	uint32_t cv1 = 0;
	uint32_t cv3 = 0;

	// Get newest CV values from correct slice
	switch(pin) {
		case 0: {
			cv1 = COUNTER_IN0_SLICE0->CV[1];
			cv3 = COUNTER_IN0_SLICE0->CV[3];
			break;
		}

		case 1: {
			cv1 = COUNTER_IN1_SLICE0->CV[1];
			cv3 = COUNTER_IN1_SLICE0->CV[3];
			break;
		}

		case 2: {
			cv1 = COUNTER_IN2_SLICE0->CV[1];
			cv3 = COUNTER_IN2_SLICE0->CV[3];
			break;
		}

		case 3: {
			cv1 = COUNTER_IN3_SLICE0->CV[1];
			cv3 = COUNTER_IN3_SLICE0->CV[3];
			break;
		}

		default: break;
	}

	// Take the latest low/high capture value (stored in CV[1] and CV[3])
	int32_t low = -1;
	int32_t high = -1;
	uint8_t prescaler = 0;

	// If the CV value is not valid we use the last valid one
	if(!(cv1 & CCU4_CC4_CV_FFL_Msk)) {
		cv1 = counter.last_cv1[pin];
	} else {
		counter.last_cv1[pin] = cv1;
	}

	if(!(cv3 & CCU4_CC4_CV_FFL_Msk)) {
		cv3 = counter.last_cv3[pin];
	} else {
		counter.last_cv3[pin] = cv3;
	}

	// Check if both CV values are valid and if the prescalers match
	if((cv1 & CCU4_CC4_CV_FFL_Msk) && (cv3 & CCU4_CC4_CV_FFL_Msk) && ((cv1 & CCU4_CC4_CV_FPCV_Msk) == (cv3 & CCU4_CC4_CV_FPCV_Msk))) {
		low = cv1 & CCU4_CC4_CV_CAPTV_Msk;
		high = cv3 & CCU4_CC4_CV_CAPTV_Msk;
		prescaler = (cv1 & CCU4_CC4_CV_FPCV_Msk) >> CCU4_CC4_CV_FPCV_Pos;
	}

	// If the CV values were not sane, we return the last known duty cycle and period
	if(((low+high) == 0) || (low == -1) || (high == -1)) {
		*duty_cycle = counter.last_duty_cycle[pin];
		*period = counter.last_period[pin];
		return;
	}

	// Calculate new duty cycle and period
	*duty_cycle = (high*10000LL + (low+high)/2)/(low+high);
	// *125/12 == 1000/96 (ns)
	*period = ((uint64_t)(low + high))*(1LL << prescaler)*125/12;

	// Save duty cycle and period
	counter.last_duty_cycle[pin] = *duty_cycle;
	counter.last_period[pin] = *period;
}

uint32_t counter_get_frequency(const uint8_t pin) {
	int64_t current = 0;
	int64_t before = 0;

	// TODO: Use correct irq num for 0, 1, 2
	switch(pin) {
		case 0: NVIC_DisableIRQ(23); current = counter_frequency_current0; before = counter_frequency_before0; NVIC_EnableIRQ(23); break;
		case 1: NVIC_DisableIRQ(12); current = counter_frequency_current1; before = counter_frequency_before1; NVIC_EnableIRQ(12); break;
		case 2: NVIC_DisableIRQ(6);  current = counter_frequency_current2; before = counter_frequency_before2; NVIC_EnableIRQ(6); break;
		case 3: NVIC_DisableIRQ(16); current = counter_frequency_current3; before = counter_frequency_before3; NVIC_EnableIRQ(16); break;
	}

	uint32_t frequency = ((llabs(current - before))*125*125) >> (counter.config_frequency_integration_time[pin] + 1);
	if(counter.config_count_edge[pin] == INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH) {
		frequency >>= 1;
	}

	return frequency;
}

void counter_tick(void) {
/*	bool in1 = XMC_GPIO_GetInput(COUNTER_IN0_PIN);
	if(in1) {
		XMC_GPIO_SetOutputLow(COUNTER_STATUS1_LED_PIN);
	} else {
		XMC_GPIO_SetOutputHigh(COUNTER_STATUS1_LED_PIN);
	}*/

	for(uint8_t pin = 0; pin < COUNTER_NUM; pin++) {
		if(counter.config_update[pin]) {
			counter_counter_init(pin, false);
			counter.config_update[pin] = false;
		}
	}
}
