// This example is not self-contained.
// It requres usage of the example driver specific to your platform.
// See the HAL documentation.

#include "bindings/hal_common.h"
#include "bindings/bricklet_industrial_counter.h"

#define UID "XYZ" // Change XYZ to the UID of your Industrial Counter Bricklet

void check(int rc, const char* msg);

void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);


static TF_IndustrialCounter ic;

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_industrial_counter_create(&ic, UID, hal), "create device object");

	// Get current counter from channel 0
	int64_t counter;
	check(tf_industrial_counter_get_counter(&ic, TF_INDUSTRIAL_COUNTER_CHANNEL_0,
	                                        &counter), "get counter from channel 0");

	tf_hal_printf("Counter (Channel 0): %I64d\n", counter);

	// Get current signal data from channel 0
	uint16_t duty_cycle; uint64_t period; uint32_t frequency; bool value;
	check(tf_industrial_counter_get_signal_data(&ic, TF_INDUSTRIAL_COUNTER_CHANNEL_0,
	                                            &duty_cycle, &period, &frequency,
	                                            &value), "get signal data from channel 0");

	tf_hal_printf("Duty Cycle (Channel 0): %d 1/%d %%\n", duty_cycle, 100);
	tf_hal_printf("Period (Channel 0): %I64u ns\n", period);
	tf_hal_printf("Frequency (Channel 0): %d 1/%d Hz\n", frequency, 1000);
	tf_hal_printf("Value (Channel 0): %s\n", value ? "true" : "false");
}

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
