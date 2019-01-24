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

	// Get current counter from channel 0.
	counter, _ := ic.GetCounter(industrial_counter_bricklet.Channel0)
	fmt.Printf("Counter (Channel 0): \n", counter)

	// Get current signal data from channel 0.
	dutyCycle, period, frequency, value, _ := ic.GetSignalData(industrial_counter_bricklet.Channel0)

	fmt.Printf("Duty Cycle (Channel 0): %f %\n", float64(dutyCycle)/100.0)
	fmt.Printf("Period (Channel 0):  ns\n", period)
	fmt.Printf("Frequency (Channel 0): %f Hz\n", float64(frequency)/1000.0)
	fmt.Printf("Value (Channel 0): \n", value)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
