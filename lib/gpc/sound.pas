unit sound;

interface

uses
  types;

procedure sound_system(nr : unsigned);
function sound_playing : int;

implementation

procedure sound_system(nr : unsigned);
begin
  asm (
      "push %0\n"
      "mov.w #0x4003,r6\n"
      "jsr @sound_system\n"
      "adds #0x2,sp\n"
      : // output
      :"r"(nr)  // input
      :"r6", "cc", "memory" // clobbered
  );
end;

function sound_playing : int;
begin
  asm (
      "mov.w r7,r6\n"
      "push r6\n"
      "mov.w #0x700c,r6\n"
      "jsr @sound_playing\n"
      "adds #0x2,sp\n"
      "mov.w @r7,%0\n"
      : "=r"(result) // output
      :              // input
      :"r6", "cc", "memory" // clobbered
  );
end;

end.
