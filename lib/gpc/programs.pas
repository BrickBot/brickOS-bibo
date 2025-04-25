unit programs;

interface

uses
  types;

const
  PROG_MAX = 8;

var
  cprog : unsigned; asmname 'cprog'; external;

procedure program_stop(flag : int); asmname 'program_stop';
function program_valid(nr : unsigned) : int; asmname 'program_valid';
procedure program_init; asmname 'program_init';
procedure program_shutdown; asmname 'program_shutdown';

implementation

end.