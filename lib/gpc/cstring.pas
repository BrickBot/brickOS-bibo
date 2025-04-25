unit cstring;

interface

uses
  types;

function memcpy(dest : pointer; const src : pointer; size : size_t) : pointer; asmname 'memcpy';
function memset(s : pointer; c : int; n : size_t) : pointer; asmname 'memset';
function strcpy(dest : pchar; const src : pchar) : pchar; asmname 'strcpy';
function strlen(const s : pchar) : int; asmname 'strlen';
function strcmp(const s1 : pchar; const s2 : pchar) : int; asmname 'strcmp';

implementation

end.