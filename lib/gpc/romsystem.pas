unit romsystem;

interface

uses
  irq;

procedure power_off;
procedure power_init;

procedure reset; asmname 'reset';
procedure rom_reset;

implementation

procedure power_off;
begin
  asm ("push r6
        bset #7,@0xc4:8  ; software standby mode
        jsr @ power_off  ; ROM call
        pop r6");
end;

procedure power_init;
begin
  asm ("push r6
        jsr @ power_init ; ROM call
        bclr #7,@0xc4:8  ; disable software standby
        pop r6");
end;


procedure rom_reset;
begin
  disable_irqs;
  reset;
end;

end.
