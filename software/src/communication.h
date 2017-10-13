/* industrial-counter-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants
#define INDUSTRIAL_COUNTER_PIN_0 0
#define INDUSTRIAL_COUNTER_PIN_1 1
#define INDUSTRIAL_COUNTER_PIN_2 2
#define INDUSTRIAL_COUNTER_PIN_3 3

#define INDUSTRIAL_COUNTER_COUNT_EDGE_RISING 0
#define INDUSTRIAL_COUNTER_COUNT_EDGE_FALLING 1
#define INDUSTRIAL_COUNTER_COUNT_EDGE_BOTH 2

#define INDUSTRIAL_COUNTER_COUNT_DIRECTION_UP 0
#define INDUSTRIAL_COUNTER_COUNT_DIRECTION_DOWN 1
#define INDUSTRIAL_COUNTER_COUNT_DIRECTION_EXTERNAL_UP 2
#define INDUSTRIAL_COUNTER_COUNT_DIRECTION_EXTERNAL_DOWN 3

#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_1 0
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_2 1
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_4 2
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_8 3
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_16 4
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_32 5
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_64 6
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_128 7
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_256 8
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_512 9
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_1024 10
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_2048 11
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_4096 12
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_8192 13
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_16384 14
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_32768 15
#define INDUSTRIAL_COUNTER_DUTY_CYCLE_PRESCALER_AUTO 255

#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_128_MS 0
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_256_MS 1
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_1024_MS 2
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_2048_MS 3
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_4096_MS 4
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_8192_MS 5
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_16384_MS 6
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_32768_MS 7
#define INDUSTRIAL_COUNTER_FREQUENCY_INTEGRATION_TIME_AUTO 255

#define INDUSTRIAL_COUNTER_BOOTLOADER_MODE_BOOTLOADER 0
#define INDUSTRIAL_COUNTER_BOOTLOADER_MODE_FIRMWARE 1
#define INDUSTRIAL_COUNTER_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define INDUSTRIAL_COUNTER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define INDUSTRIAL_COUNTER_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_OK 0
#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_INVALID_MODE 1
#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_NO_CHANGE 2
#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define INDUSTRIAL_COUNTER_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define INDUSTRIAL_COUNTER_STATUS_LED_CONFIG_OFF 0
#define INDUSTRIAL_COUNTER_STATUS_LED_CONFIG_ON 1
#define INDUSTRIAL_COUNTER_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define INDUSTRIAL_COUNTER_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_GET_COUNTER 1
#define FID_GET_ALL_COUNTER 2
#define FID_SET_COUNTER 3
#define FID_SET_ALL_COUNTER 4
#define FID_GET_SIGNAL_DATA 5
#define FID_GET_ALL_SIGNAL_DATA 6
#define FID_SET_COUNTER_ACTIVE 7
#define FID_SET_ALL_COUNTER_ACTIVE 8
#define FID_GET_COUNTER_ACTIVE 9
#define FID_GET_ALL_COUNTER_ACTIVE 10
#define FID_SET_COUNTER_CONFIGURATION 11
#define FID_GET_COUNTER_CONFIGURATION 12
#define FID_SET_ALL_COUNTER_CALLBACK_CONFIGURATION 13
#define FID_GET_ALL_COUNTER_CALLBACK_CONFIGURATION 14
#define FID_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION 15
#define FID_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION 16

#define FID_CALLBACK_ALL_COUNTER 17
#define FID_CALLBACK_ALL_SIGNAL_DATA 18

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
} __attribute__((__packed__)) GetCounter;

typedef struct {
	TFPMessageHeader header;
	int64_t counter;
} __attribute__((__packed__)) GetCounter_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAllCounter;

typedef struct {
	TFPMessageHeader header;
	int64_t counter[4];
} __attribute__((__packed__)) GetAllCounter_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
	int64_t counter;
} __attribute__((__packed__)) SetCounter;

typedef struct {
	TFPMessageHeader header;
	int64_t counter[4];
} __attribute__((__packed__)) SetAllCounter;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
} __attribute__((__packed__)) GetSignalData;

typedef struct {
	TFPMessageHeader header;
	uint16_t duty_cycle;
	uint64_t period;
	uint32_t frequency;
	bool pin_value;
} __attribute__((__packed__)) GetSignalData_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAllSignalData;

typedef struct {
	TFPMessageHeader header;
	uint16_t duty_cycle[4];
	uint64_t period[4];
	uint32_t frequency[4];
	bool pin_value[4];
} __attribute__((__packed__)) GetAllSignalData_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
	bool active;
} __attribute__((__packed__)) SetCounterActive;

typedef struct {
	TFPMessageHeader header;
	bool active[4];
} __attribute__((__packed__)) SetAllCounterActive;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
} __attribute__((__packed__)) GetCounterActive;

typedef struct {
	TFPMessageHeader header;
	bool active;
} __attribute__((__packed__)) GetCounterActive_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAllCounterActive;

typedef struct {
	TFPMessageHeader header;
	bool active[4];
} __attribute__((__packed__)) GetAllCounterActive_Response;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
	uint8_t count_edge;
	uint8_t count_direction;
	uint8_t duty_cylce_prescaler;
	uint8_t frequency_integration_time;
} __attribute__((__packed__)) SetCounterConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t pin;
} __attribute__((__packed__)) GetCounterConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint8_t count_edge;
	uint8_t count_direction;
	uint8_t duty_cylce_prescaler;
	uint8_t frequency_integration_time;
} __attribute__((__packed__)) GetCounterConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) SetAllCounterCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAllCounterCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) GetAllCounterCallbackConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) SetAllSignalDataCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetAllSignalDataCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) GetAllSignalDataCallbackConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	int64_t counter[4];
} __attribute__((__packed__)) AllCounter_Callback;

typedef struct {
	TFPMessageHeader header;
	uint16_t duty_cycle[4];
	uint64_t period[4];
	uint32_t frequency[4];
	bool pin_value[4];
} __attribute__((__packed__)) AllSignalData_Callback;


// Function prototypes
BootloaderHandleMessageResponse get_counter(const GetCounter *data, GetCounter_Response *response);
BootloaderHandleMessageResponse get_all_counter(const GetAllCounter *data, GetAllCounter_Response *response);
BootloaderHandleMessageResponse set_counter(const SetCounter *data);
BootloaderHandleMessageResponse set_all_counter(const SetAllCounter *data);
BootloaderHandleMessageResponse get_signal_data(const GetSignalData *data, GetSignalData_Response *response);
BootloaderHandleMessageResponse get_all_signal_data(const GetAllSignalData *data, GetAllSignalData_Response *response);
BootloaderHandleMessageResponse set_counter_active(const SetCounterActive *data);
BootloaderHandleMessageResponse set_all_counter_active(const SetAllCounterActive *data);
BootloaderHandleMessageResponse get_counter_active(const GetCounterActive *data, GetCounterActive_Response *response);
BootloaderHandleMessageResponse get_all_counter_active(const GetAllCounterActive *data, GetAllCounterActive_Response *response);
BootloaderHandleMessageResponse set_counter_configuration(const SetCounterConfiguration *data);
BootloaderHandleMessageResponse get_counter_configuration(const GetCounterConfiguration *data, GetCounterConfiguration_Response *response);
BootloaderHandleMessageResponse set_all_counter_callback_configuration(const SetAllCounterCallbackConfiguration *data);
BootloaderHandleMessageResponse get_all_counter_callback_configuration(const GetAllCounterCallbackConfiguration *data, GetAllCounterCallbackConfiguration_Response *response);
BootloaderHandleMessageResponse set_all_signal_data_callback_configuration(const SetAllSignalDataCallbackConfiguration *data);
BootloaderHandleMessageResponse get_all_signal_data_callback_configuration(const GetAllSignalDataCallbackConfiguration *data, GetAllSignalDataCallbackConfiguration_Response *response);

// Callbacks
bool handle_all_counter_callback(void);
bool handle_all_signal_data_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 2
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_all_counter_callback, \
	handle_all_signal_data_callback, \


#endif
