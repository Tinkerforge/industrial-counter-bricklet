program ExampleCallback;

{$ifdef MSWINDOWS}{$apptype CONSOLE}{$endif}
{$ifdef FPC}{$mode OBJFPC}{$H+}{$endif}

uses
  SysUtils, IPConnection, BrickletIndustrialCounter;

type
  TExample = class
  private
    ipcon: TIPConnection;
    ic: TBrickletIndustrialCounter;
  public
    procedure AllCounterCB(sender: TBrickletIndustrialCounter;
                           const counter: TArray0To3OfInt64);
    procedure Execute;
  end;

const
  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; { Change XYZ to the UID of your Industrial Counter Bricklet }

var
  e: TExample;

{ Callback procedure for all counter callback }
procedure TExample.AllCounterCB(sender: TBrickletIndustrialCounter;
                                const counter: TArray0To3OfInt64);
begin
  WriteLn(Format('Counter (Channel 0): %d', [counter[0]]));
  WriteLn(Format('Counter (Channel 1): %d', [counter[1]]));
  WriteLn(Format('Counter (Channel 2): %d', [counter[2]]));
  WriteLn(Format('Counter (Channel 3): %d', [counter[3]]));
  WriteLn('');
end;

procedure TExample.Execute;
begin
  { Create IP connection }
  ipcon := TIPConnection.Create;

  { Create device object }
  ic := TBrickletIndustrialCounter.Create(UID, ipcon);

  { Connect to brickd }
  ipcon.Connect(HOST, PORT);
  { Don't use device before ipcon is connected }

  { Register all counter callback to procedure AllCounterCB }
  ic.OnAllCounter := {$ifdef FPC}@{$endif}AllCounterCB;

  { Set period for all counter callback to 1s (1000ms) }
  ic.SetAllCounterCallbackConfiguration(1000, true);

  WriteLn('Press key to exit');
  ReadLn;
  ipcon.Destroy; { Calls ipcon.Disconnect internally }
end;

begin
  e := TExample.Create;
  e.Execute;
  e.Destroy;
end.
