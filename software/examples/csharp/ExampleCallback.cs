using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Industrial Counter Bricklet

	// Callback function for all counter callback
	static void AllCounterCB(BrickletIndustrialCounter sender, long[] counter)
	{
		Console.WriteLine("Counter (Channel 0): " + counter[0]);
		Console.WriteLine("Counter (Channel 1): " + counter[1]);
		Console.WriteLine("Counter (Channel 2): " + counter[2]);
		Console.WriteLine("Counter (Channel 3): " + counter[3]);
		Console.WriteLine("");
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletIndustrialCounter ic =
		  new BrickletIndustrialCounter(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register all counter callback to function AllCounterCB
		ic.AllCounterCallback += AllCounterCB;

		// Set period for all counter callback to 1s (1000ms)
		ic.SetAllCounterCallbackConfiguration(1000, true);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
