Imports System
Imports Tinkerforge

Module ExampleCallback
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Industrial Counter Bricklet

    ' Callback subroutine for all counter callback
    Sub AllCounterCB(ByVal sender As BrickletIndustrialCounter, ByVal counter As Long())
        Console.WriteLine("Counter (Channel 0): " + counter(0).ToString())
        Console.WriteLine("Counter (Channel 1): " + counter(1).ToString())
        Console.WriteLine("Counter (Channel 2): " + counter(2).ToString())
        Console.WriteLine("Counter (Channel 3): " + counter(3).ToString())
        Console.WriteLine("")
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim ic As New BrickletIndustrialCounter(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register all counter callback to subroutine AllCounterCB
        AddHandler ic.AllCounterCallback, AddressOf AllCounterCB

        ' Set period for all counter callback to 1s (1000ms)
        ic.SetAllCounterCallbackConfiguration(1000, True)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
