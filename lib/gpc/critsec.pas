unit critsec;

interface

{$I config.inc}

{$IFDEF CONF_TM}
uses
  types, tm;

{$IFDEF CONF_CRITICAL_SECTIONS}

type
  critsec = record
    count : unsigned_char;
    task : Ptdata_t;
  end;
  critsec_t = critsec;
  Pcritsec_t = ^critsec_t;

function wait_critical_section(data : wakeup_t) : wakeup_t; asmname 'wait_critical_section';
procedure initialize_critical_section(cs : Pcritsec_t);
function enter_critical_section(cs : Pcritsec_t) : int; asmname 'enter_critical_section';
function leave_critical_section(cs : Pcritsec_t) : int;
procedure destroy_critical_section(cs : Pcritsec_t);
{$ENDIF}

var
  kernel_critsec_count : unsigned_char; asmname 'kernel_critsec_count'; external;

function locked_increment(counter : punsigned_char) : int; asmname 'locked_increment';
function locked_decrement(counter : punsigned_char) : int; asmname 'locked_decrement';
{$ENDIF}

implementation

{$IFDEF CONF_TM}
{$IFDEF CONF_CRITICAL_SECTIONS}
procedure initialize_critical_section(cs : Pcritsec_t);
begin
  cs^.count := 0;
end;

function leave_critical_section(cs : Pcritsec_t) : int;
begin
  result := locked_decrement(@(cs^.count));
end;

procedure destroy_critical_section(cs : Pcritsec_t);
begin
  // do nothing
end;

{$ENDIF}
{$ENDIF}
end.