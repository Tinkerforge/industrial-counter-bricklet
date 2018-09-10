import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletIndustrialCounter;
import com.tinkerforge.BrickletIndustrialCounter.SignalData;

public class ExampleSimple {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your Industrial Counter Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletIndustrialCounter ic =
		  new BrickletIndustrialCounter(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Get current counter from channel 0
		long counter = ic.getCounter(BrickletIndustrialCounter.CHANNEL_0); // Can throw com.tinkerforge.TimeoutException
		System.out.println("Counter (Channel 0): " + counter);

		// Get current signal data from channel 0
		SignalData signalData = ic.getSignalData(BrickletIndustrialCounter.CHANNEL_0); // Can throw com.tinkerforge.TimeoutException

		System.out.println("Duty Cycle (Channel 0): " + signalData.dutyCycle/100.0 + " %");
		System.out.println("Period (Channel 0): " + signalData.period + " ns");
		System.out.println("Frequency (Channel 0): " + signalData.frequency/1000.0 + " Hz");
		System.out.println("Value (Channel 0): " + signalData.value);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
