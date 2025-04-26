/*****************************************************************************/
/** AbstractCommand, etc. ****************************************************/
/**                                                                         **/

typedef enum {eCmdNOP=0, eCmdDelay=1, eCmdOutput=2, eCmdGoto=3, eCmdInsert=4,
  eCmdDelete=5, eCmdAppend=6} CommandType;

typedef struct AbstractCommand {
  CommandType type;
  void (*execute)(struct AbstractCommand* ac);
  void (*display)(struct AbstractCommand* ac);
} AbstractCommand;

typedef struct GotoCommand {
  CommandType type;
  void (*execute)(struct GotoCommand* ac);
  void (*display)(struct GotoCommand* ac);
  void (*incgoto)(struct GotoCommand* ac);
  void (*incgoto16)(struct GotoCommand* ac);
  int iGoto;
} GotoCommand;

void GotoCommand_execute(GotoCommand* cmd)
{
}

void GotoCommand_incgoto(GotoCommand* cmd)
{
  if (++(cmd->iGoto) >= 256)
    cmd->iGoto = cmd->iGoto-256;
}

void GotoCommand_incgoto16(GotoCommand* cmd)
{
  cmd->iGoto = cmd->iGoto + 16;
  if (cmd->iGoto >= 256)
    cmd->iGoto = cmd->iGoto-256;
}

void GotoCommand_display(GotoCommand* cmd)
{
  cputc_hex_2((cmd->iGoto >> 4) & 0x0f);
  cputc_hex_1(cmd->iGoto & 0x0f);
  cputc_native_0(CHAR_g);
}

void GotoCommand_constructor(GotoCommand* cmd, int iGoto)
{
  cmd->type = eCmdGoto;
  cmd->execute = GotoCommand_execute;
  cmd->display = GotoCommand_display;
  cmd->incgoto = GotoCommand_incgoto;
  cmd->incgoto16 = GotoCommand_incgoto16;
  cmd->iGoto = iGoto;
}

typedef struct NOPCommand {
  CommandType type;
  void (*execute)(struct NOPCommand* ac);
  void (*display)(struct NOPCommand* ac);
  char cOp;
} NOPCommand;

void NOPCommand_execute(NOPCommand* cmd)
{
}

void NOPCommand_display(NOPCommand* cmd)
{
  cputc_native_2(CHAR_SPACE);
  cputc_native_1(CHAR_SPACE);
//  cputc_native_0(ascii_display_codes[(int)(cmd->cOp)]);
// TODO: Verify that hex_display_codes is a suitable replacement.
//   NOP command does not appear to be selectable, so this might be a non-issue
  cputc_hex_0(cmd->cOp);
//  lcd_refresh();
}

void NOPCommand_constructor(NOPCommand* cmd, char cOp)
{
  cmd->type = eCmdNOP;
  cmd->execute = NOPCommand_execute;
  cmd->display = NOPCommand_display;
  cmd->cOp = cOp;
}

typedef struct DelayCommand {
  CommandType type;
  void (*execute)(struct DelayCommand* ac);
  void (*display)(struct DelayCommand* ac);
  void (*displaytoggle)(struct DelayCommand* ac);
  void (*incdelay)(struct DelayCommand* ac);
  void (*incdelay16)(struct DelayCommand* ac);
  int dsec;
} DelayCommand;

void DelayCommand_execute(DelayCommand* cmd)
{
  cmd->display(cmd);
  msleep(cmd->dsec * 100);
}

void HideLCDMotors()
{
  dlcd_hide(LCD_A_RIGHT);
  dlcd_hide(LCD_A_LEFT);
  dlcd_hide(LCD_A_SELECT);
  dlcd_hide(LCD_B_RIGHT);
  dlcd_hide(LCD_B_LEFT);
  dlcd_hide(LCD_B_SELECT);
  dlcd_hide(LCD_C_RIGHT);
  dlcd_hide(LCD_C_LEFT);
  dlcd_hide(LCD_C_SELECT);
}

void DelayCommand_display_primitive(DelayCommand* cmd, int bToggle)
{
  static int bSwitch = 0;
  bSwitch = bToggle?!bSwitch:0;

  HideLCDMotors();
  cputc_native_2(bSwitch ? CHAR_SPACE : hex_display_codes[(cmd->dsec>>4)&0x0F] );
  cputc_native_1(bSwitch ? CHAR_SPACE : hex_display_codes[cmd->dsec&0x0F] );
  cputc_native_0(CHAR_d);
//  lcd_refresh();
}

void DelayCommand_display(DelayCommand* cmd)
{
  DelayCommand_display_primitive(cmd, 0);
}

void DelayCommand_displaytoggle(DelayCommand* cmd)
{
  DelayCommand_display_primitive(cmd, 1);
}

void DelayCommand_incdelay(DelayCommand* cmd)
{
  if (++(cmd->dsec) >= 256)
    cmd->dsec = cmd->dsec-256;
}

void DelayCommand_incdelay16(DelayCommand* cmd)
{
  cmd->dsec = cmd->dsec + 16;
  if (cmd->dsec >= 256)
    cmd->dsec = cmd->dsec-256;
}

void DelayCommand_constructor(DelayCommand* cmd, int dsec)
{
  cmd->type = eCmdDelay;
  cmd->execute = DelayCommand_execute;
  cmd->display = DelayCommand_display;
  cmd->displaytoggle = DelayCommand_displaytoggle;
  cmd->incdelay = DelayCommand_incdelay;
  cmd->incdelay16 = DelayCommand_incdelay16;
  cmd->dsec = dsec;
}

typedef struct OutputCommand {
  CommandType type;
  void (*execute)(struct OutputCommand* ac);
  void (*display)(struct OutputCommand* ac);
  void (*displaytoggle)(struct OutputCommand* ac, int ncol);
  void (*incpower)(struct OutputCommand* ac);
  void (*incport)(struct OutputCommand* ac);
  void (*incfwd)(struct OutputCommand* ac, int incPort);
  int iPort;
  int nPower; /* -1 = off, 0 = free, 1..8 on */
  int iFwd;
} OutputCommand;

void OutputCommand_execute(OutputCommand* cmd)
{
  static int ncol=0;
  cmd->display(cmd);
  if (ncol > 3)
    ncol=0;
  if (cmd->iPort & 4)
  {
      if (cmd->nPower > 0 && cmd->nPower <=8)
        motor_c_speed(cmd->nPower);
      if (cmd->nPower < 0)
        motor_c_dir(brake);
      else if (cmd->nPower == 0)
        motor_c_dir(off);
      else if (cmd->iFwd & 4)
        motor_c_dir(fwd);
      else
        motor_c_dir(rev);
  }
  if (cmd->iPort & 2)
  {
      if (cmd->nPower > 0 && cmd->nPower <=8)
        motor_b_speed(cmd->nPower);
      if (cmd->nPower < 0)
        motor_b_dir(brake);
      else if (cmd->nPower == 0)
        motor_b_dir(off);
      else if (cmd->iFwd & 2)
        motor_b_dir(fwd);
      else
        motor_b_dir(rev);
  }
  if (cmd->iPort & 1)
  {
      if (cmd->nPower > 0 && cmd->nPower <=8)
        motor_a_speed(cmd->nPower);
      if (cmd->nPower < 0)
        motor_a_dir(brake);
      else if (cmd->nPower == 0)
        motor_a_dir(off);
      else if (cmd->iFwd & 1)
        motor_a_dir(fwd);
      else
        motor_a_dir(rev);
  }
  // msleep(250);
}

void OutputCommand_display_primitive(OutputCommand* cmd, int bToggle, int ncol)
{
  static int bSwitch = 0;
  bSwitch = bToggle?!bSwitch:0;

  HideLCDMotors();
  if (cmd->iPort == 1)
    cputc_native_2(CHAR_A);
  else if (cmd->iPort == 2)
    cputc_native_2(CHAR_B);
  else if (cmd->iPort == 4)
    cputc_native_2(CHAR_C);
  else
    cputc_native_2(CHAR_SPACE);
  cputc_native_1((bSwitch&&(ncol==1)) ? CHAR_UNDERSCORE : ((cmd->nPower<0) ? CHAR_DASH : hex_display_codes[cmd->nPower]));
  cputc_native_0((bSwitch&&(ncol==0)) ? CHAR_SPACE : CHAR_o);
  if (cmd->iPort & 1)
  {
      if (cmd->iFwd & 1)
         dlcd_show(LCD_A_RIGHT);
      else
         dlcd_show(LCD_A_LEFT);
      if (bSwitch&&(ncol==2))
      	dlcd_show(LCD_A_SELECT);
      if (bSwitch&&(ncol==3))
      {
      	dlcd_show(LCD_A_LEFT);
      	dlcd_show(LCD_A_RIGHT);
      }
  }
  if (cmd->iPort & 2)
  {
      if (cmd->iFwd & 2)
         dlcd_show(LCD_B_RIGHT);
      else
         dlcd_show(LCD_B_LEFT);
      if (bSwitch&&(ncol==2))
      	dlcd_show(LCD_B_SELECT);
      if (bSwitch&&(ncol==3))
      {
      	dlcd_show(LCD_B_LEFT);
      	dlcd_show(LCD_B_RIGHT);
      }
  }
  if (cmd->iPort & 4)
  {
      if (cmd->iFwd & 4)
         dlcd_show(LCD_C_RIGHT);
      else
         dlcd_show(LCD_C_LEFT);
      if (bSwitch&&(ncol==2))
      	dlcd_show(LCD_C_SELECT);
      if (bSwitch&&(ncol==3))
      {
      	dlcd_show(LCD_C_LEFT);
      	dlcd_show(LCD_C_RIGHT);
      }
  }
//  lcd_refresh();
}

void OutputCommand_display(OutputCommand* cmd)
{
  OutputCommand_display_primitive(cmd, 0, 0);
}

void OutputCommand_displaytoggle(OutputCommand* cmd, int ncol)
{
  OutputCommand_display_primitive(cmd, 1, ncol);
}

void OutputCommand_incport(OutputCommand* cmd)
{
  if (++(cmd->iPort) > 7)
    cmd->iPort = 1;
}

void OutputCommand_incpower(OutputCommand* cmd)
{
  if (++(cmd->nPower) > 8)
    cmd->nPower = -1;
}

void OutputCommand_incfwd(OutputCommand* cmd, int incPort)
{
  switch(incPort & 3)
  {
    case 1:
      cmd->iFwd = cmd->iFwd ^ 1;
      break;
    case 2:
      cmd->iFwd = cmd->iFwd ^ 2;
      break;
    case 3:
      cmd->iFwd = cmd->iFwd ^ 4;
      break;
  }
}

void OutputCommand_constructor(OutputCommand* cmd, int iPort, int nPower,
  int iFwd)
{
  cmd->type = eCmdOutput;
  cmd->execute = OutputCommand_execute;
  cmd->display = OutputCommand_display;
  cmd->displaytoggle = OutputCommand_displaytoggle;
  cmd->incport = OutputCommand_incport;
  cmd->incpower = OutputCommand_incpower;
  cmd->incfwd = OutputCommand_incfwd;
  cmd->iPort = iPort;
  cmd->nPower = nPower;
  cmd->iFwd = iFwd;
}

/**                                                                         **/
/** AbstractCommand, etc. ****************************************************/
/*****************************************************************************/

