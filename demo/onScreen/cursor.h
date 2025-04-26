typedef struct
{
  int col;
  void (*toggle)();
  void (*set)(int);
  int (*get)();
  void (*inc)();
  void (*dec)();
  void (*show)();
  void (*hide)();
} Cursor;

extern Cursor RCXCursor;
extern RCX MyRCX;

int Cursor_get()
{
  return RCXCursor.col;
}

void Cursor_toggle()
{
  static int toggle=0;
  toggle=!toggle;
  switch(Cursor_get())
  {
    case 0:
      if (toggle)
        dlcd_show(LCD_0_BOT);
      else
        dlcd_hide(LCD_0_BOT);
      break;
    case 1:
      if (toggle)
        dlcd_show(LCD_1_BOT);
      else
        dlcd_hide(LCD_1_BOT);
      break;
    case 2:
      if (toggle)
        dlcd_show(LCD_2_BOT);
      else
        dlcd_hide(LCD_2_BOT);
      break;
    case 3:
      if (toggle)
        dlcd_show(LCD_3_BOT);
      else
        dlcd_hide(LCD_3_BOT);
      break;
    case 4:
      if (toggle)
        dlcd_show(LCD_4_BOT);
      else
        dlcd_hide(LCD_4_BOT);
      break;
    case 5:
      if (toggle)
        dlcd_show(LCD_A_SELECT);
      else
        dlcd_hide(LCD_A_SELECT);
      break;
    case 6:
      if (toggle)
        dlcd_show(LCD_B_SELECT);
      else
        dlcd_hide(LCD_B_SELECT);
      break;
    case 7:
      if (toggle)
        dlcd_show(LCD_C_SELECT);
      else
        dlcd_hide(LCD_C_SELECT);
      break;
    case 8:
      if (toggle)
        dlcd_show(LCD_S1_SELECT);
      else
        dlcd_hide(LCD_S1_SELECT);
      break;
    case 9:
      if (toggle)
        dlcd_show(LCD_S2_SELECT);
      else
        dlcd_hide(LCD_S2_SELECT);
      break;
    case 10:
      if (toggle)
        dlcd_show(LCD_S3_SELECT);
      else
        dlcd_hide(LCD_S3_SELECT);
      break;
  }
//  lcd_refresh();
}

void Cursor_set(int n)
{
  RCXCursor.col = n;
}

void Cursor_inc()
{
  int i=Cursor_get();
  if (++i > 10)
    i = 1;
  Cursor_set(i);
}

void Cursor_dec()
{
  int i=Cursor_get();
  if (--i < 0)
    i = 10;
  Cursor_set(i);
}

void Cursor_hide()
{
  int i=Cursor_get();
  if (i>=0)
    Cursor_set(-i-1);
}

void Cursor_show()
{
  int i=Cursor_get();
  if (i<0)
    Cursor_set(-i-1);
}

Cursor RCXCursor={-1,&Cursor_toggle,&Cursor_set,&Cursor_get,
 &Cursor_inc, &Cursor_dec, &Cursor_show, &Cursor_hide};

