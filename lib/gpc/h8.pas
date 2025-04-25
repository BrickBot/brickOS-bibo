unit h8;

interface

uses
  types;

var
  // 16 bit timer registers
  T_IER  : unsigned_char; asmname 'T_IER'; external;
  T_CSR  : unsigned_char; asmname 'T_CSR'; external;

  T_CNT  : unsigned; asmname 'T_CNT'; external;
  T_OCRA : unsigned; asmname 'T_OCRA'; external;
  T_OCRB : unsigned; asmname 'T_OCRB'; external;

  T_CR   : unsigned_char; asmname 'T_CR'; external;
  T_OCR  : unsigned_char; asmname 'T_OCR'; external;

  T_ICRA : unsigned; asmname 'T_ICRA'; external;
  T_ICRB : unsigned; asmname 'T_ICRB'; external;
  T_ICRC : unsigned; asmname 'T_ICRC'; external;
  T_ICRD : unsigned; asmname 'T_ICRD'; external;

const
  // TIER bitmasks
  TIER_ENABLE_ICA = $80;
  TIER_ENABLE_ICB = $40;
  TIER_ENABLE_ICC = $20;
  TIER_ENABLE_ICD = $10;
  TIER_ENABLE_OCA = $08;
  TIER_ENABLE_OCB = $04;
  TIER_ENABLE_OF  = $02;
  TIER_RESERVED   = $01;

  TCSR_ICA        = $80;
  TCSR_ICB        = $40;
  TCSR_ICC        = $20;
  TCSR_ICD        = $10;
  TCSR_OCA        = $08;
  TCSR_OCB        = $04;
  TCSR_OF         = $02;
  TCSR_RESET_ON_A	= $01;

  TCR_A_RISING    = $80;
  TCR_B_RISING    = $40;
  TCR_C_RISING    = $20;
  TCR_D_RISING    = $10;
  TCR_BUFFER_A    = $08;
  TCR_BUFFER_B    = $04;
  TCR_CLOCK_2     = $00;
  TCR_CLOCK_8     = $01;
  TCR_CLOCK_32    = $02;
  TCR_CLOCK_EXT   = $03;

  TOCR_OCRA         = $00;
  TOCR_OCRB         = $10;
  TOCR_ENABLE_A     = $08;
  TOCR_ENABLE_B     = $04;
  TOCR_HIGH_LEVEL_A = $02;
  TOCR_HIGH_LEVEL_B = $01;

var
  // 8 bit timer registers
  STCR    : unsigned_char; asmname 'STCR'; external;
  T0_CR   : unsigned_char; asmname 'T0_CR'; external;
  T0_CSR  : unsigned_char; asmname 'T0_CSR'; external;
  T0_CORA : unsigned_char; asmname 'T0_CORA'; external;
  T0_CORB : unsigned_char; asmname 'T0_CORB'; external;
  T0_CNT  : unsigned_char; asmname 'T0_CNT'; external;
  T1_CR   : unsigned_char; asmname 'T1_CR'; external;
  T1_CSR  : unsigned_char; asmname 'T1_CSR'; external;
  T1_CORA : unsigned_char; asmname 'T1_CORA'; external;
  T1_CORB : unsigned_char; asmname 'T1_CORB'; external;
  T1_CNT  : unsigned_char; asmname 'T1_CNT'; external;

const
// CR flag bitmasks
  CR_ENABLE_IRQA     = $40;
  CR_ENABLE_IRQB     = $80;
  CR_ENABLE_IRQO     = $20;

  CR_CLEAR_NEVER     = $00;
  CR_CLEAR_ON_A      = $08;
  CR_CLEAR_ON_B      = $10;
  CR_CLEAR_ON_EXTERN = $18;

// CSR bitmasks
  CSR_MATCH_A  = $40;
  CSR_MATCH_B  = $80;
  CSR_OVERFLOW = $20;

  CSR_IGNORE_B    = $00;
  CSR_0_ON_B      = $04;
  CSR_1_ON_B      = $08;
  CSR_TOGGLE_ON_B = $0c;

  CSR_IGNORE_A    = $00;
  CSR_0_ON_A      = $01;
  CSR_1_ON_A      = $02;
  CSR_TOGGLE_ON_A = $03;

var
  // Serial port
  S_RDR : unsigned_char; asmname 'S_RDR'; external;
  S_TDR : unsigned_char; asmname 'S_TDR'; external;
  S_MR  : unsigned_char; asmname 'S_MR'; external;
  S_CR  : unsigned_char; asmname 'S_CR'; external;
  S_SR  : unsigned_char; asmname 'S_SR'; external;
  S_BRR : unsigned_char; asmname 'S_BRR'; external;
  S_TCR : unsigned_char; asmname 'S_TCR'; external;

const
// register bitmasks
//
  SMR_SYNC        = $80;
  SMR_ASYNC       = $00;
  SMR_7BIT        = $40;
  SMR_8BIT        = $00;
  SMR_P_NONE      = $00;
  SMR_P_EVEN      = $20;
  SMR_P_ODD       = $30;
  SMR_1STOP       = $00;
  SMR_2STOP       = $08;
  SMR_MP          = $04;
  SMR_CLOCK       = $00;
  SMR_CLOCK_4     = $01;
  SMR_CLOCK_16    = $02;
  SMR_CLOCK_64    = $03;
  SCR_TX_IRQ      = $80;
  SCR_RX_IRQ      = $40;
  SCR_TRANSMIT    = $20;
  SCR_RECEIVE     = $10;
  SCR_MP_IRQ      = $08;
  SCR_TE_IRQ      = $04;
  SCR_INT_CLOCK   = $00;
  SCR_EXT_CLOCK   = $02;
  SCR_CLOCK_OUT   = $01;
  SSR_TRANS_EMPTY = $80;
  SSR_RECV_FULL   = $40;
  SSR_OVERRUN_ERR = $20;
  SSR_FRAMING_ERR = $10;
  SSR_PARITY_ERR  = $08;
  SSR_ERRORS      = $38;
  SSR_TRANS_END   = $04;
  SSR_MP          = $02;
  SSR_MP_TRANSFER = $01;

// values for the bit rate register BRR
// assuming CMR_CLOCK selected on 16 MHz processor
// error <= 0.16%
  B2400  = 207;
  B4800  = 103;
  B9600  = 51;
  B19200 = 25;
  B38400 = 12;

var
  // A/D converter
  AD_A_H : unsigned_char; asmname 'AD_A_H'; external;
  AD_A_L : unsigned_char; asmname 'AD_A_L'; external;
  AD_B_H : unsigned_char; asmname 'AD_B_H'; external;
  AD_B_L : unsigned_char; asmname 'AD_B_L'; external;
  AD_C_H : unsigned_char; asmname 'AD_C_H'; external;
  AD_C_L : unsigned_char; asmname 'AD_C_L'; external;
  AD_D_H : unsigned_char; asmname 'AD_D_H'; external;
  AD_D_L : unsigned_char; asmname 'AD_D_L'; external;
  AD_A   : unsigned; asmname 'AD_A'; external;
  AD_B   : unsigned; asmname 'AD_B'; external;
  AD_C   : unsigned; asmname 'AD_C'; external;
  AD_D   : unsigned; asmname 'AD_D'; external;
  AD_CSR : unsigned_char; asmname 'AD_CSR'; external;
  AD_CR  : unsigned_char; asmname 'AD_CR'; external;

const
// A/D CSR bitmasks
  ADCSR_END        = $80;
  ADCSR_ENABLE_IRQ = $40;
  ADCSR_START      = $20;
  ADCSR_SCAN       = $10;
  ADCSR_TIME_266   = $00;
  ADCSR_TIME_134   = $08;

  ADCSR_GROUP_0    = $00;
  ADCSR_GROUP_1    = $04;

  ADCSR_AN_0       = $00;
  ADCSR_AN_1       = $01;
  ADCSR_AN_2       = $02;
  ADCSR_AN_3       = $03;

// A/D CR bitmasks
  ADCR_EXTERN      = $80;

var
  // system control register
  // FIXME: incomplete
  SYSCR : unsigned_char; asmname 'SYSCR'; external;

const
  SYSCR_SOFTWARE_STANDBY = $80;

var
  // I/O ports
  PORT1_PCR : unsigned_char; asmname 'PORT1_PCR'; external;
  PORT2_PCR : unsigned_char; asmname 'PORT2_PCR'; external;
  PORT3_PCR : unsigned_char; asmname 'PORT3_PCR'; external;
  PORT1_DDR : unsigned_char; asmname 'PORT1_DDR'; external;
  PORT1     : unsigned_char; asmname 'PORT1'; external;
  PORT2_DDR : unsigned_char; asmname 'PORT2_DDR'; external;
  PORT2     : unsigned_char; asmname 'PORT2'; external;
  PORT3_DDR : unsigned_char; asmname 'PORT3_DDR'; external;
  PORT3     : unsigned_char; asmname 'PORT3'; external;
  PORT4_DDR : unsigned_char; asmname 'PORT4_DDR'; external;
  PORT4     : unsigned_char; asmname 'PORT4'; external;
  PORT5_DDR : unsigned_char; asmname 'PORT5_DDR'; external;
  PORT5     : unsigned_char; asmname 'PORT5'; external;
  PORT6_DDR : unsigned_char; asmname 'PORT6_DDR'; external;
  PORT6     : unsigned_char; asmname 'PORT6'; external;
  PORT7     : unsigned_char; asmname 'PORT7'; external;

// watch dog timer control register
var
  WDT_CSR : unsigned_int; asmname 'WDT_CSR'; external;

const
  WDT_CSR_PASSWORD        = $A500;
  WDT_CSR_ENABLE          = $0020;
  WDT_CSR_MODE_WATCHDOG   = $0040;
  WDT_CSR_MODE_OVERFLOW   = $0000;
  WDT_CSR_WATCHDOG_NMI    = $0000;
  WDT_CSR_WATCHDOG_RES    = $0008;
  WDT_CSR_CLOCK_2         = $0000;
  WDT_CSR_CLOCK_32        = $0001;
  WDT_CSR_CLOCK_64        = $0002;
  WDT_CSR_CLOCK_128       = $0003;
  WDT_CSR_CLOCK_256       = $0004;
  WDT_CSR_CLOCK_512       = $0005;
  WDT_CSR_CLOCK_2048      = $0006;
  WDT_CSR_CLOCK_4096      = $0007;

// watch dog timer counter register
var
  WDT_CNT : unsigned_char; asmname 'WDT_CNT'; external;

const
  WDT_CNT_PASSWORD        = $5A00;
  WDT_CNT_CLEAR           = $0000;
  WDT_CNT_MSEC_64         = $0006;


implementation

end.