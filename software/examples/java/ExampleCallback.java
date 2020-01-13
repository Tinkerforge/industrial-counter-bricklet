import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletIndustrialCounter;

public class ExampleCallback {
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

		// Add all counter listener
		ic.addAllCounterListener(new BrickletIndustrialCounter.AllCounterListener() {
			public void allCounter(long[] counter) {
				System.out.println("Counter (Channel 0): " + counter[0]);
				System.out.println("Counter (Channel 1): " + counter[1]);
				System.out.println("Counter (Channel 2): " + counter[2]);
				System.out.println("Counter (Channel 3): " + counter[3]);
				System.out.println("");
			}
		});

		// Set period for all counter callback to 1s (1000ms)
		ic.setAllCounterCallbackConfiguration(1000, true);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
