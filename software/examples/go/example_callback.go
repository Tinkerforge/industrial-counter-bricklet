package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/industrial_counter_bricklet"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your Industrial Counter Bricklet.

func main() {
	ipcon := ipconnection.New()
	defer ipcon.Close()
	ic, _ := industrial_counter_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
	defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	ic.RegisterAllCounterCallback(func(counter [4]int64) {
		fmt.Printf("Counter (Channel 0): %d\n", counter[0])
		fmt.Printf("Counter (Channel 1): %d\n", counter[1])
		fmt.Printf("Counter (Channel 2): %d\n", counter[2])
		fmt.Printf("Counter (Channel 3): %d\n", counter[3])
		fmt.Println()
	})

	// Set period for all counter callback to 1s (1000ms).
	ic.SetAllCounterCallbackConfiguration(1000, true)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
