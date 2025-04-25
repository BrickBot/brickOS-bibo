unit semaphore;

interface

{$I config.inc}

{$IFDEF CONF_SEMAPHORES}
uses
  types;

type
  sem_t = unsigned_char;
  psem_t = ^sem_t;

const
  EAGAIN = $FFFF;

function sem_init(sem : psem_t; pshared : int; value : unsigned_int) : int;
function sem_wait(sem : psem_t) : int; asmname 'sem_wait';
function sem_trywait(sem : psem_t) : int; asmname 'sem_trywait';
function sem_post(sem : psem_t) : int; asmname 'sem_post';
function sem_getvalue(sem : psem_t; sval : pint) : int;
function sem_destroy(sem : psem_t) : int;
{$ENDIF}

implementation

{$IFDEF CONF_SEMAPHORES}
function sem_init(sem : psem_t; pshared : int; value : unsigned_int) : int;
begin
  sem^ := sem_t(value);
  Result := 0;
end;

function sem_getvalue(sem : psem_t; sval : pint) : int;
begin
  sval^ := sem^;
  Result := 0;
end;

function sem_destroy(sem : psem_t) : int;
begin
  Result := 0;
end;
{$ENDIF}

end.