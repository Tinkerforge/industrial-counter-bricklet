/* industrial-counter-bricklet
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/hal/system_timer/system_timer.h"

#include "counter.h"

extern Counter counter;

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_GET_COUNTER: return get_counter(message, response);
		case FID_GET_ALL_COUNTER: return get_all_counter(message, response);
		case FID_SET_COUNTER: return set_counter(message);
		case FID_SET_ALL_COUNTER: return set_all_counter(message);
		case FID_GET_SIGNAL_DATA: return get_signal_data(message, response);
		case FID_GET_ALL_SIGNAL_DATA: return get_all_signal_data(message, response);
		case FID_SET_COUNTER_ACTIVE: return set_counter_active(message);
		case FID_SET_ALL_COUNTER_ACTIVE: return set_all_counter_active(message);
		case FID_GET_COUNTER_ACTIVE: return get_counter_active(message, response);
		case FID_GET_ALL_COUNTER_ACTIVE: return get_all_counter_active(message, response);
		case FID_SET_COUNTER_CONFIGURATION: return set_counter_configuration(message);
		case FID_GET_COUNTER_CONFIGURATION: return get_counter_configuration(message, response);
		case FID_SET_ALL_COUNTER_CALLBACK_CONFIGURATION: return set_all_counter_callback_configuration(message);
		case FID_GET_ALL_COUNTER_CALLBACK_CONFIGURATION: return get_all_counter_callback_configuration(message, response);
		case FID_SET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION: return set_all_signal_data_callback_configuration(message);
		case FID_GET_ALL_SIGNAL_DATA_CALLBACK_CONFIGURATION: return get_all_signal_data_callback_configuration(message, response);
		case FID_SET_CHANNEL_LED_CONFIG: return set_channel_led_config(message);
		case FID_GET_CHANNEL_LED_CONFIG: return get_channel_led_config(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse get_counter(const GetCounter *data, GetCounter_Response *response) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->header.length = sizeof(GetCounter_Response);
	response->counter = counter_get_count(data->channel);


	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_all_counter(const GetAllCounter *data, GetAllCounter_Response *response) {
	response->header.length = sizeof(GetAllCounter_Response);

	for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
		response->counter[channel] = counter_get_count(channel);
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_counter(const SetCounter *data) {
	if((data->counter > COUNTER_MAX_VALUE) || (data->counter < COUNTER_MIN_VALUE)) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	uint8_t active = counter_get_active();
	if(active & (1 << data->channel)) {
		counter_set_active((active & (~(1 << data->channel))));
	}
	counter_set_count(data->channel, data->counter);

	if(active & (1 << data->channel)) {
		counter_set_active(active);
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse set_all_counter(const SetAllCounter *data) {
	if((data->counter[0] > COUNTER_MAX_VALUE) || (data->counter[0] < COUNTER_MIN_VALUE) ||
	   (data->counter[1] > COUNTER_MAX_VALUE) || (data->counter[1] < COUNTER_MIN_VALUE) ||
	   (data->counter[2] > COUNTER_MAX_VALUE) || (data->counter[2] < COUNTER_MIN_VALUE) ||
	   (data->counter[3] > COUNTER_MAX_VALUE) || (data->counter[3] < COUNTER_MIN_VALUE)) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	uint8_t active = counter_get_active();
	counter_set_active(0);

	for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
		counter_set_count(channel, data->counter[channel]);
	}

	counter_set_active(active);

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_signal_data(const GetSignalData *data, GetSignalData_Response *response) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->header.length = sizeof(GetSignalData_Response);

	uint16_t duty_cycle;
	uint64_t period;
	counter_get_duty_cycle_and_period(data->channel, &duty_cycle, &period);
	response->duty_cycle = duty_cycle;
	response->period = period;

	response->frequency = counter_get_frequency(data->channel);
	response->value = counter_get_value(data->channel);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_all_signal_data(const GetAllSignalData *data, GetAllSignalData_Response *response) {
	response->header.length = sizeof(GetAllSignalData_Response);
	response->value = 0;

	for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
		uint16_t duty_cycle;
		uint64_t period;

		counter_get_duty_cycle_and_period(channel, &duty_cycle, &period);

		response->duty_cycle[channel] = duty_cycle;
		response->period[channel] = period;
		response->frequency[channel] = counter_get_frequency(channel);
		response->value |= counter_get_value(channel) << channel;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_counter_active(const SetCounterActive *data) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	counter.config_active[data->channel] = data->active;

	uint8_t mask = counter_get_active();
	if(data->active) {
		mask |=  (1 << data->channel);
	} else {
		mask &= ~(1 << data->channel);
	}

	counter_set_active(mask);

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse set_all_counter_active(const SetAllCounterActive *data) {
	for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
		counter.config_active[channel] = data->active & (1 << channel);
	}

	counter_set_active(data->active);

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_counter_active(const GetCounterActive *data, GetCounterActive_Response *response) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->header.length = sizeof(GetCounterActive_Response);
	response->active = counter_get_active() & (1 << data->channel);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_all_counter_active(const GetAllCounterActive *data, GetAllCounterActive_Response *response) {
	response->header.length = sizeof(GetAllCounterActive_Response);
	response->active = counter_get_active();

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_counter_configuration(const SetCounterConfiguration *data) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	counter.config_count_edge[data->channel] = data->count_edge;
	counter.config_count_direction[data->channel] = data->count_direction;
	counter.config_duty_cycle_prescaler[data->channel] = data->duty_cycle_prescaler;
	counter.config_frequency_integration_time[data->channel] = data->frequency_integration_time;
	counter.config_update[data->channel] = true;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_counter_configuration(const GetCounterConfiguration *data, GetCounterConfiguration_Response *response) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->header.length = sizeof(GetCounterConfiguration_Response);
	response->count_edge = counter.config_count_edge[data->channel];
	response->count_direction = counter.config_count_direction[data->channel];
	response->duty_cycle_prescaler = counter.config_duty_cycle_prescaler[data->channel];
	response->frequency_integration_time = counter.config_frequency_integration_time[data->channel];

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_all_counter_callback_configuration(const SetAllCounterCallbackConfiguration *data) {
	counter.cb_counter_period              = data->period;
	counter.cb_counter_value_has_to_change = data->value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_all_counter_callback_configuration(const GetAllCounterCallbackConfiguration *data, GetAllCounterCallbackConfiguration_Response *response) {
	response->header.length       = sizeof(GetAllCounterCallbackConfiguration_Response);
	response->period              = counter.cb_counter_period;
	response->value_has_to_change = counter.cb_counter_value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_all_signal_data_callback_configuration(const SetAllSignalDataCallbackConfiguration *data) {
	counter.cb_signal_period              = data->period;
	counter.cb_signal_value_has_to_change = data->value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_all_signal_data_callback_configuration(const GetAllSignalDataCallbackConfiguration *data, GetAllSignalDataCallbackConfiguration_Response *response) {
	response->header.length       = sizeof(GetAllSignalDataCallbackConfiguration_Response);
	response->period              = counter.cb_signal_period;
	response->value_has_to_change = counter.cb_signal_value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_channel_led_config(const SetChannelLEDConfig *data) {
	if(data->channel >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	counter.info_leds[data->channel].config = data->config;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_channel_led_config(const GetChannelLEDConfig *data, GetChannelLEDConfig_Response *response) {
	response->header.length = sizeof(GetChannelLEDConfig_Response);

	if(data->led >= COUNTER_NUM) {
		return HANDLE_MESSAGE_RESPONSE_INVALID_PARAMETER;
	}

	response->config = counter.info_leds[data->led].config;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

bool handle_all_counter_callback(void) {
	static bool is_buffered = false;
	static AllCounter_Callback cb;

	static uint32_t last_time = 0;
	static int64_t last_counter[4] = {0, 0, 0, 0};

	if(!is_buffered) {
		if(counter.cb_counter_period != 0) {
			if(system_timer_is_time_elapsed_ms(last_time, counter.cb_counter_period)) {
				uint64_t new_counter[4];
				for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
					new_counter[channel] = counter_get_count(channel);
				}

				if((!counter.cb_counter_value_has_to_change) ||
				   (last_counter[0] != new_counter[0])       ||
				   (last_counter[1] != new_counter[1])       ||
				   (last_counter[2] != new_counter[2])       ||
				   (last_counter[3] != new_counter[3])) {
					last_time = system_timer_get_ms();
					tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(AllCounter_Callback), FID_CALLBACK_ALL_COUNTER);
					for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
						cb.counter[channel] = new_counter[channel];
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(AllCounter_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

bool handle_all_signal_data_callback(void) {
	static bool is_buffered = false;
	static AllSignalData_Callback cb;

	static uint32_t last_time = 0;

	static uint16_t duty_cycle[4] = {0, 0, 0, 0};
	static uint64_t period[4] = {0, 0, 0, 0};
	static uint32_t frequency[4] = {0, 0, 0, 0};
	static uint8_t value = 0;

	if(!is_buffered) {
		if(counter.cb_signal_period != 0) {
			if(system_timer_is_time_elapsed_ms(last_time, counter.cb_signal_period)) {
				uint16_t new_duty_cycle[4];
				uint64_t new_period[4];
				uint32_t new_frequency[4];
				uint8_t new_value = 0;

				for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
					uint16_t duty_cycle;
					uint64_t period;

					counter_get_duty_cycle_and_period(channel, &duty_cycle, &period);

					new_duty_cycle[channel] = duty_cycle;
					new_period[channel] = period;
					new_frequency[channel] = counter_get_frequency(channel);
					new_value |= counter_get_value(channel) << channel;
				}

				if((!counter.cb_signal_value_has_to_change) ||
				   (duty_cycle[0] != new_duty_cycle[0])     ||
				   (period[0]     != new_period[0])         ||
				   (frequency[0]  != new_frequency[0])      ||
				   (duty_cycle[1] != new_duty_cycle[1])     ||
				   (period[1]     != new_period[1])         ||
				   (frequency[1]  != new_frequency[1])      ||
				   (duty_cycle[2] != new_duty_cycle[2])     ||
				   (period[2]     != new_period[2])         ||
				   (frequency[2]  != new_frequency[2])      ||
				   (duty_cycle[3] != new_duty_cycle[3])     ||
				   (period[3]     != new_period[3])         ||
				   (frequency[3]  != new_frequency[3])      ||
				   (value         != new_value)) {
					last_time = system_timer_get_ms();
					tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(AllSignalData_Callback), FID_CALLBACK_ALL_SIGNAL_DATA);
					for(uint8_t channel = 0; channel < COUNTER_NUM; channel++) {
						cb.duty_cycle[channel] = new_duty_cycle[channel];
						cb.period[channel]     = new_period[channel];
						cb.frequency[channel]  = new_frequency[channel];
						cb.value               = new_value;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(AllSignalData_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
