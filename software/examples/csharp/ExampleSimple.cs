using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Industrial Counter Bricklet

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletIndustrialCounter ic =
		  new BrickletIndustrialCounter(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Get current counter from channel 0
		long counter = ic.GetCounter(BrickletIndustrialCounter.CHANNEL_0);
		Console.WriteLine("Counter (Channel 0): " + counter);

		// Get current signal data from channel 0
		int dutyCycle; long period, frequency; bool value;
		ic.GetSignalData(BrickletIndustrialCounter.CHANNEL_0, out dutyCycle, out period,
		                 out frequency, out value);

		Console.WriteLine("Duty Cycle (Channel 0): " + dutyCycle/100.0 + " %");
		Console.WriteLine("Period (Channel 0): " + period + " ns");
		Console.WriteLine("Frequency (Channel 0): " + frequency/1000.0 + " Hz");
		Console.WriteLine("Value (Channel 0): " + value);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
