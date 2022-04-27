// This example is not self-contained.
// It requires usage of the example driver specific to your platform.
// See the HAL documentation.

#include "src/bindings/hal_common.h"
#include "src/bindings/bricklet_industrial_counter.h"

void check(int rc, const char *msg);
void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);

// Callback function for all counter callback
static void all_counter_handler(TF_IndustrialCounter *device, int64_t counter[4],
                                void *user_data) {
	(void)device; (void)user_data; // avoid unused parameter warning

	tf_hal_printf("Counter (Channel 0): %I64d\n", counter[0]);
	tf_hal_printf("Counter (Channel 1): %I64d\n", counter[1]);
	tf_hal_printf("Counter (Channel 2): %I64d\n", counter[2]);
	tf_hal_printf("Counter (Channel 3): %I64d\n", counter[3]);
	tf_hal_printf("\n");
}

static TF_IndustrialCounter ic;

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_industrial_counter_create(&ic, NULL, hal), "create device object");

	// Register all counter callback to function all_counter_handler
	tf_industrial_counter_register_all_counter_callback(&ic,
	                                                    all_counter_handler,
	                                                    NULL);

	// Set period for all counter callback to 1s (1000ms)
	tf_industrial_counter_set_all_counter_callback_configuration(&ic, 1000, true);
}

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
