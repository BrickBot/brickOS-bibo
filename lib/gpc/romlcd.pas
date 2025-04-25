unit romlcd;

interface

uses
  types;

type
  lcd_segment = integer;
  lcd_number_style = integer;
  lcd_comma_style = integer;

const
  // lcd_segment constants (enum in c)
  ls_man_stand = $3006;
  ls_man_run   = $3007;
  ls_s1_select = $3008;
  ls_s1_active = $3009;
  ls_s2_select = $300A;
  ls_s2_active = $300B;
  ls_s3_select = $300C;
  ls_s3_active = $300D;
  ls_a_select  = $300E;
  ls_a_left    = $300F;
  ls_a_right   = $3010;
  ls_b_select  = $3011;
  ls_b_left    = $3012;
  ls_b_right   = $3013;
  ls_c_select  = $3014;
  ls_c_left    = $3015;
  ls_c_right   = $3016;
  ls_unknown_1 = $3017;		//!< seemingly without effect. cycle reset?
  ls_circle    = $3018;		//!< 0..3 quarters: add one. 4 quarters: reset
  ls_dot       = $3019;		//!< 0..4 dots: add a dot. 5 dots: reset
      	      	      	        // dots are added on the right.
  ls_dot_inv   = $301A;		//!< 0 dots: show 5. 1..4 dots: subtract one
      	      	      	        // dots are removed on the left
  ls_battery_x = $301B;
  ls_ir_half   = $301C; 	//! the IR display values are mutually exclusive.
  ls_ir_full   = $301D;   	//! the IR display values are mutually exclusive.
  ls_everything= $3020;

  // lcd_number_style constants (enum in c)
  lns_digit     = $3017;
  lns_sign      = $3001;
  lns_unsign    = $301F;

  // lcd_comma_style constants (enum in c)
  lcs_digit_comma = $0000;
  lcs_e0          = $3002;
  lcs_e_1         = $3003;
  lcs_e_2         = $3004;
  lcs_e_3         = $3005;

inline procedure lcd_int(i : integer);
inline procedure lcd_unsigned(u : unsigned);
inline procedure lcd_clock(t : unsigned);
inline procedure lcd_digit(d : integer);

procedure lcd_number(i : integer; n : lcd_number_style; c : lcd_comma_style); asmname 'lcd_number';

procedure lcd_show(seg : lcd_segment);
procedure lcd_hide(seg : lcd_segment);
procedure lcd_clear;


implementation

inline procedure lcd_int(i : integer);
begin
  lcd_number(i, lns_sign, lcs_e0);
end;

inline procedure lcd_unsigned(u : unsigned);
begin
  lcd_number(u, lns_unsign, lcs_e_2);
end;

inline procedure lcd_clock(t : unsigned);
begin
  lcd_number(t, lns_unsign, lcs_e_2);
end;

inline procedure lcd_digit(d : integer);
begin
  lcd_number(d, lns_digit, lcs_digit_comma);
end;

procedure lcd_show(seg : lcd_segment);
begin
  asm(
    "push r6\n"
    "mov.w %0,r6\n"
    "jsr @lcd_show\n"
    "pop r6\n"
    :	// outputs
    :"r"(seg)	// inputs
  );
end;

procedure lcd_hide(seg : lcd_segment);
begin
  asm(
    "push r6\n"
    "mov.w %0,r6\n"
    "jsr @lcd_hide\n"
    "pop r6\n"
    :	// outputs
    :"r"(seg)	// inputs
  );
end;

procedure lcd_clear;
begin
  asm(
    "push r6\n"
    "jsr @lcd_clear\n"
    "pop r6\n"
  );
end;

end.