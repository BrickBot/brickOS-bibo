unit irq;

interface

var
  reset_vector : pointer; asmname 'reset_vector'; external;
  nmi_vector : pointer; asmname 'nmi_vector'; external;
  irq0_vector : pointer; asmname 'irq0_vector'; external;
  irq1_vector : pointer; asmname 'irq1_vector'; external;
  irq2_vector : pointer; asmname 'irq2_vector'; external;
  icia_vector : pointer; asmname 'icia_vector'; external;
  icib_vector : pointer; asmname 'icib_vector'; external;
  icic_vector : pointer; asmname 'icic_vector'; external;
  icid_vector : pointer; asmname 'icid_vector'; external;
  ocia_vector : pointer; asmname 'ocia_vector'; external;
  ocib_vector : pointer; asmname 'ocib_vector'; external;
  fovi_vector : pointer; asmname 'fovi_vector'; external;
  cmi0a_vector : pointer; asmname 'cmi0a_vector'; external;
  cmi0b_vector : pointer; asmname 'cmi0b_vector'; external;
  ovi0_vector : pointer; asmname 'ovi0_vector'; external;
  cmi1a_vector : pointer; asmname 'cmi1a_vector'; external;
  cmi1b_vector : pointer; asmname 'cmi1b_vector'; external;
  ovi1_vector : pointer; asmname 'ovi1_vector'; external;
  eri_vector : pointer; asmname 'eri_vector'; external;
  rxi_vector : pointer; asmname 'rxi_vector'; external;
  txi_vector : pointer; asmname 'txi_vector'; external;
  tei_vector : pointer; asmname 'tei_vector'; external;
  ad_vector : pointer; asmname 'ad_vector'; external;
  wovf_vector : pointer; asmname 'wovf_vector'; external;
  rom_reset_vector : pointer; asmname 'rom_reset_vector'; external;

procedure rom_ocia_handler; asmname 'rom_ocia_handler';
procedure rom_ocia_return; asmname 'rom_ocia_return';
procedure rom_dummy_handler; asmname 'rom_dummy_handler';

procedure disable_irqs;
procedure enable_irqs;

implementation

procedure disable_irqs;
begin
  asm ("\torc  #0x80,ccr\n":::"cc");
end;

procedure enable_irqs;
begin
  asm ("\tandc #0x7f,ccr\n":::"cc");
end;

end.
