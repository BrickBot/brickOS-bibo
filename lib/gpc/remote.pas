unit remote;

interface

{$I config.inc}

{$IFDEF CONF_LR_HANDLER}

uses
  types;

const
  LR_TIMEOUT = 100;
  LRKEY_M1   = $1;
  LRKEY_M2   = $2;
  LRKEY_M3   = $4;
  LRKEY_A1   = $8;
  LRKEY_B1   = $10;
  LRKEY_C1   = $20;
  LRKEY_A2   = $40;
  LRKEY_B2   = $80;
  LRKEY_C2   = $100;
  LRKEY_P1   = $200;
  LRKEY_P2   = $400;
  LRKEY_P3   = $800;
  LRKEY_P4   = $1000;
  LRKEY_P5   = $2000;
  LRKEY_STOP = $4000;
  LRKEY_BEEP = $8000;

type
  EventType = (LREVT_NONE, LREVT_KEYON, LREVT_KEYOFF);
// the remote key handler type
  lr_handler_t = ^function(unsigned_int; unsigned_int) : int;
// typedef int ( * lr_handler_t ) (unsigned int, unsigned int);

// remote handler.
var
  lr_handler : lr_handler_t; asmname 'lr_handler'; external;

procedure lr_startup; asmname 'lr_startup';
procedure lr_init; asmname 'lr_init';
procedure lr_shutdown; asmname 'lr_shutdown';

procedure lr_set_handler(handler : lr_handler_t);

// dummy remote event handler
var
  LR_DUMMY_HANDLER : lr_handler_t = nil;

{$ENDIF}

implementation

{$IFDEF CONF_LR_HANDLER}

procedure lr_set_handler(handler : lr_handler_t);
begin
  lr_handler := handler;
end;

{$ENDIF}

end.