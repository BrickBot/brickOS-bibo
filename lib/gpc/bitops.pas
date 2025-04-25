unit bitops;

interface

uses
  types;

procedure bit_set(b : punsigned_char; const bit : byte);
procedure bit_clear(b : punsigned_char; const bit : byte);
procedure bit_load(mask : punsigned_char; const bit : byte);
procedure bit_iload(mask : punsigned_char; const bit : byte);
procedure bit_store(b : punsigned_char; const bit : byte);

implementation

procedure bit_set(b : punsigned_char; const bit : byte);
begin
  case bit of
     0 : asm ( "bset #0,@%0\n" : : "r" (b));
     1 : asm ( "bset #1,@%0\n" : : "r" (b));
     2 : asm ( "bset #2,@%0\n" : : "r" (b));
     3 : asm ( "bset #3,@%0\n" : : "r" (b));
     4 : asm ( "bset #4,@%0\n" : : "r" (b));
     5 : asm ( "bset #5,@%0\n" : : "r" (b));
     6 : asm ( "bset #6,@%0\n" : : "r" (b));
     7 : asm ( "bset #7,@%0\n" : : "r" (b));
  end;
end;

procedure bit_clear(b : punsigned_char; const bit : byte);
begin
  case bit of
     0 : asm ( "bclr #0,@%0\n" : : "r" (b));
     1 : asm ( "bclr #1,@%0\n" : : "r" (b));
     2 : asm ( "bclr #2,@%0\n" : : "r" (b));
     3 : asm ( "bclr #3,@%0\n" : : "r" (b));
     4 : asm ( "bclr #4,@%0\n" : : "r" (b));
     5 : asm ( "bclr #5,@%0\n" : : "r" (b));
     6 : asm ( "bclr #6,@%0\n" : : "r" (b));
     7 : asm ( "bclr #7,@%0\n" : : "r" (b));
  end;
end;

procedure bit_load(mask : punsigned_char; const bit : byte);
begin
  case bit of
     0 : asm ( "bld #0,%0l\n" : : "r" (mask));
     1 : asm ( "bld #1,%0l\n" : : "r" (mask));
     2 : asm ( "bld #2,%0l\n" : : "r" (mask));
     3 : asm ( "bld #3,%0l\n" : : "r" (mask));
     4 : asm ( "bld #4,%0l\n" : : "r" (mask));
     5 : asm ( "bld #5,%0l\n" : : "r" (mask));
     6 : asm ( "bld #6,%0l\n" : : "r" (mask));
     7 : asm ( "bld #7,%0l\n" : : "r" (mask));
  end;
end;

procedure bit_iload(mask : punsigned_char; const bit : byte);
begin
  case bit of
     0 : asm ( "bild #0,%0l\n" : : "r" (mask));
     1 : asm ( "bild #1,%0l\n" : : "r" (mask));
     2 : asm ( "bild #2,%0l\n" : : "r" (mask));
     3 : asm ( "bild #3,%0l\n" : : "r" (mask));
     4 : asm ( "bild #4,%0l\n" : : "r" (mask));
     5 : asm ( "bild #5,%0l\n" : : "r" (mask));
     6 : asm ( "bild #6,%0l\n" : : "r" (mask));
     7 : asm ( "bild #7,%0l\n" : : "r" (mask));
  end;
end;

procedure bit_store(b : punsigned_char; const bit : byte);
begin
  case bit of
     0 : asm ( "bst #0,@%0\n" : : "r" (b));
     1 : asm ( "bst #1,@%0\n" : : "r" (b));
     2 : asm ( "bst #2,@%0\n" : : "r" (b));
     3 : asm ( "bst #3,@%0\n" : : "r" (b));
     4 : asm ( "bst #4,@%0\n" : : "r" (b));
     5 : asm ( "bst #5,@%0\n" : : "r" (b));
     6 : asm ( "bst #6,@%0\n" : : "r" (b));
     7 : asm ( "bst #7,@%0\n" : : "r" (b));
  end;
end;

end.
