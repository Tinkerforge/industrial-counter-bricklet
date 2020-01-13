use std::{error::Error, io, thread};
use tinkerforge::{industrial_counter_bricklet::*, ip_connection::IpConnection};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Industrial Counter Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let ic = IndustrialCounterBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    let all_counter_receiver = ic.get_all_counter_callback_receiver();

    // Spawn thread to handle received callback messages.
    // This thread ends when the `ic` object
    // is dropped, so there is no need for manual cleanup.
    thread::spawn(move || {
        for all_counter in all_counter_receiver {
            println!("Counter (Channel 0): {}", all_counter[0]);
            println!("Counter (Channel 1): {}", all_counter[1]);
            println!("Counter (Channel 2): {}", all_counter[2]);
            println!("Counter (Channel 3): {}", all_counter[3]);
            println!();
        }
    });

    // Set period for all counter callback to 1s (1000ms).
    ic.set_all_counter_callback_configuration(1000, true);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
