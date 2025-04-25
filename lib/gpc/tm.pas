unit tm;

interface

uses
  types;

const
  PRIO_LOWEST  = 1;
  PRIO_NORMAL  = 10;
  PRIO_HIGHEST = 20;

const
  T_DEAD     = 0;
  T_ZOMBIE   = 1;
  T_WAITING  = 2;
  T_SLEEPING = 3;
  T_RUNNING  = 4;

  T_KERNEL   = 1 shl 0;
  T_USER     = 1 shl 1;
  T_IDLE     = 1 shl 2;


  DEFAULT_STACK_SIZE = 512;

type
  Ppchain_t = ^pchain_t;

  Ptdata_t = ^tdata_t;

  tdata_t = record
    sp_save : Psize_t;

    tstate : tstate_t;
    tflags : tflags_t;
    priority : Ppchain_t;

    next : Ptdata_t;
    prev : Ptdata_t;
    parent : Ptdata_t;

    stack_base : Psize_t;

    wakeup : wakeupPtr; // a function pointer
    wakeup_data : wakeup_t;
  end;

  pchain_t = record
    priority : priority_t;
    next : Ppchain_t;
    prev : Ppchain_t;
    ctid : Ptdata_t;
  end;

const
  SP_RETURN_OFFSET = 10;
  IDLE_STACK_SIZE  = 128;

var
  td_single : tdata_t; asmname 'td_single'; external;
  ctid : Ptdata_t; asmname 'ctid'; external;
  nb_tasks : unsigned_int; asmname 'nb_tasks'; external;
  nb_system_tasks : unsigned_int; asmname 'nb_system_tasks'; external;
  tm_timeslice : unsigned_char; asmname 'tm_timeslice'; external;

procedure tm_init; asmname 'tm_init';
procedure tm_start; asmname 'tm_start';
procedure tm_switcher; asmname 'tm_switcher';
function tm_scheduler(old_sp : Psize_t) : Psize_t; asmname 'tm_scheduler';
function tm_idle_task(i : integer; c : PPChar) : integer; asmname 'tm_idle_task';
function shutdown_requested : boolean;

implementation

function shutdown_requested : boolean;
begin
{$IFDEF CONF_TM}
  result := (ctid^.tflags and T_SHUTDOWN) <> 0;
{$ELSE}
  result := false;
{$ENDIF}
end;

end.