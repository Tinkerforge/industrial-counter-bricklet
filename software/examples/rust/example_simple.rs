use std::{error::Error, io};

use tinkerforge::{industrial_counter_bricklet::*, ip_connection::IpConnection};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Industrial Counter Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let ic = IndustrialCounterBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    // Get current counter from channel 0.
    let counter = ic
        .get_counter(INDUSTRIAL_COUNTER_BRICKLET_CHANNEL_0)
        .recv()?;
    println!("Counter (Channel 0): {}", counter);

    // Get current signal data from channel 0.
    let signal_data = ic
        .get_signal_data(INDUSTRIAL_COUNTER_BRICKLET_CHANNEL_0)
        .recv()?;

    println!(
        "Duty Cycle (Channel 0): {} %",
        signal_data.duty_cycle as f32 / 100.0
    );
    println!("Period (Channel 0): {} ns", signal_data.period);
    println!(
        "Frequency (Channel 0): {} Hz",
        signal_data.frequency as f32 / 1000.0
    );
    println!("Value (Channel 0): {}", signal_data.value);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
