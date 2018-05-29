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

#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/utility/led_flicker.h"

#define COUNTER_MAX_VALUE (((int64_t)(UINT16_MAX)*(int64_t)(INT32_MAX) + (int64_t)(UINT16_MAX)-(int64_t)(1)))
#define COUNTER_MIN_VALUE ((int64_t)(UINT16_MAX)*(int64_t)(INT32_MIN))

#define COUNTER_NUM 4

typedef struct {
	uint8_t config;
	LEDFlickerState info_led_flicker_state;
} INFO_LED_CONFIG_t;

typedef struct {
	bool config_update[COUNTER_NUM];
	uint8_t config_count_edge[COUNTER_NUM];
	uint8_t config_count_direction[COUNTER_NUM];
	uint8_t config_duty_cycle_prescaler[COUNTER_NUM];
	uint8_t config_frequency_integration_time[COUNTER_NUM];
	bool config_active[COUNTER_NUM];

	uint16_t last_duty_cycle[COUNTER_NUM];
	uint64_t last_period[COUNTER_NUM];
	uint32_t last_cv1[COUNTER_NUM];
	uint32_t last_cv3[COUNTER_NUM];

	INFO_LED_CONFIG_t info_leds[COUNTER_NUM];
} Counter;

void counter_init(void);
void counter_tick(void);

void counter_set_active(const uint8_t channel_mask);
uint8_t counter_get_active(void);
bool counter_get_value(const uint8_t channel);
void counter_set_count(const uint8_t channel, const int64_t count);
int64_t counter_get_count(const uint8_t channel);
void counter_get_duty_cycle_and_period(const uint8_t channel, uint16_t *duty_cycle, uint64_t *period);
uint32_t counter_get_frequency(const uint8_t channel);

#endif
