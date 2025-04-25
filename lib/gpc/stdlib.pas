unit stdlib;

interface

uses
  types;

function calloc(nmemb : size_t; size : size_t) : pointer; asmname 'calloc';
function malloc(size : size_t) : pointer; asmname 'malloc';
procedure free(ptr : pointer); asmname 'free';

function random : long_int; asmname 'random';
procedure srandom(seed : unsigned_int); asmname 'srandom';

implementation

end.