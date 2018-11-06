use std::{error::Error, io};

use tinkerforge::{industrial_counter_bricklet::*, ipconnection::IpConnection};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Industrial Counter Bricklet

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let industrial_counter_bricklet = IndustrialCounterBricklet::new(UID, &ipcon); // Create device object

    ipcon.connect(HOST, PORT).recv()??; // Connect to brickd
                                        // Don't use device before ipcon is connected

    // Get current counter from channel 0
    let counter = industrial_counter_bricklet.get_counter(INDUSTRIAL_COUNTER_BRICKLET_CHANNEL_0).recv()?;
    println!("Counter (Channel 0): {}", counter);

    // Get current signal data from channel 0
    let get_signal_data_result = industrial_counter_bricklet.get_signal_data(INDUSTRIAL_COUNTER_BRICKLET_CHANNEL_0).recv()?;

    println!("Duty Cycle (Channel 0): {}{}", get_signal_data_result.duty_cycle as f32 / 100.0, " %");
    println!("Period (Channel 0): {}{}", get_signal_data_result.period, " ns");
    println!("Frequency (Channel 0): {}{}", get_signal_data_result.frequency as f32 / 1000.0, " Hz");
    println!("Value (Channel 0): {}", get_signal_data_result.value);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
