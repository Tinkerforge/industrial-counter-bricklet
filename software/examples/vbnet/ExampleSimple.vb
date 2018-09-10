Imports System
Imports Tinkerforge

Module ExampleSimple
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Industrial Counter Bricklet

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim ic As New BrickletIndustrialCounter(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Get current counter from channel 0
        Dim counter As Long = ic.GetCounter(BrickletIndustrialCounter.CHANNEL_0)
        Console.WriteLine("Counter (Channel 0): " + counter.ToString())

        ' Get current signal data from channel 0
        Dim dutyCycle As Integer
        Dim period, frequency As Long
        Dim value As Boolean

        ic.GetSignalData(BrickletIndustrialCounter.CHANNEL_0, dutyCycle, period, _
                         frequency, value)

        Console.WriteLine("Duty Cycle (Channel 0): " + (dutyCycle/100.0).ToString() + " %")
        Console.WriteLine("Period (Channel 0): " + period.ToString() + " ns")
        Console.WriteLine("Frequency (Channel 0): " + (frequency/1000.0).ToString() + " Hz")
        Console.WriteLine("Value (Channel 0): " + value.ToString())

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
