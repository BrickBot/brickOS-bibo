unit unistd;

interface

uses
  types;

{$I config.inc}

type
  code_start_t = ^function(int, PPChar) : int;

function execi(code_start : code_start_t; argc : int; argv : PPChar;
  priority : priority_t; stack_size : size_t) : tid_t; asmname 'execi';
procedure bexit(code : int); asmname 'exit';
procedure yield; asmname 'yield';
{$IFDEF CONF_TM}
function wait_event(wakeup : wakeupPtr; data : wakeup_t) : wakeup_t; asmname 'wait_event';
function sleep(sec : unsigned_int) : unsigned_int; asmname 'sleep';
function msleep(msec : unsigned_int) : unsigned_int; asmname 'msleep';
{$ELSE}
function wait_event(wakeup : wakeupPtr; data : wakeup_t) : wakeup_t;
function sleep(sec : unsigned_int) : unsigned_int;
function msleep(msec : unsigned_int) : unsigned_int;
{$ENDIF}
procedure shutdown_task(tid : tid_t); asmname 'shutdown_task';
procedure shutdown_tasks(flags : tflags_t); asmname 'shutdown_tasks';
procedure kill(tid : tid_t); asmname 'kill';
procedure killall(p : priority_t); asmname 'killall';

implementation

{$IFDEF CONF_TM}
{$ELSE}
function wait_event(wakeup : wakeupPtr; data : wakeup_t) : wakeup_t;
begin
  result := wakeup^(data);
  while result = 0 do
  begin
    result := wakeup^(data);
  end;
end;

function sleep(sec : unsigned_int) : unsigned_int;
begin
  delay(1000*msec);
end;

function msleep(msec : unsigned_int) : unsigned_int;
begin
  delay(msec);
end;
{$ENDIF}

end.