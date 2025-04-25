unit mm;

interface

uses
  types;

const
  MM_FREE = $0000;
  MM_RESERVED = $FFFF;
  MM_HEADER_SIZE = 2;
  MM_SPLIT_THRESH = MM_HEADER_SIZE + 8;

var
  mm_start : size_t; asmname 'mm_start'; external;
  mm_first_free : Psize_t; asmname 'mm_first_free'; external;

procedure mm_init; asmname 'mm_init';
function mm_free_mem : int; asmname 'mm_free_mem';

implementation

end.