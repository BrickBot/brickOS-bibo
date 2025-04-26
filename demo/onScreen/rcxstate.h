/*****************************************************************************/
/** RCXState *****************************************************************/
/**                                                                         **/

typedef struct RCXState {
  struct RCXState* (*view)(RCX* rcx);
  struct RCXState* (*onOff)(RCX* rcx);
  struct RCXState* (*prgm)(RCX* rcx);
  struct RCXState* (*run)(RCX* rcx);
  struct RCXState* (*tick)(RCX* rcx);
} RCXState;

RCXState RCXOffState, RCXOnState, RCXGoState, RCXBrowseState,
       RCXCmdState, RCXDelayState, RCXOutputState, RCXGotoState;

void fnDisplayCommand()
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  if (MyRCX.M_currentMacro == NULL)
    return;
  if (n >= MyRCX.M_currentMacro->nCmds || n < 0)
    n = MyRCX.M_currentMacro->nCurrentCmd = 0;
  if (MyRCX.M_currentMacro->nCmds > n)
  {
//    lcd_clear();
    cputc_hex_4((n>>4)&0x0f);
    cputc_hex_3(n&0x0f);
dlcd_show(LCD_3_DOT);
    MyRCX.M_currentMacro->AC_cmds[n]->display(MyRCX.M_currentMacro->AC_cmds[n]);
  }
}

void fnIncCommand()
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  if (++n >= MyRCX.M_currentMacro->nCmds)
    n=0;
  MyRCX.M_currentMacro->nCurrentCmd = n;
}

RCXState* fnRun()
{
  if (tRun == NULL) {
    tRun = execi(&taskRun, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
//    lcd_clear();
    //cputs("GO   ");
	cputc_native_user(CHAR_G, CHAR_O, 0, 0);  // GO
//    lcd_refresh();
    msleep(500);
//    lcd_clear();
//    lcd_refresh();
    return &RCXGoState;
  }
  else {
    kill(tRun);
    tRun = NULL;
    motor_a_dir(off);
    motor_b_dir(off);
    motor_c_dir(off);
//    lcd_clear();
    //cputs("OFF  ");
	cputc_native_user(CHAR_O, CHAR_F, CHAR_F, 0);  // OFF
//    lcd_refresh();
    msleep(500);
//    lcd_clear();
//    lcd_refresh();
    return &RCXBrowseState;
  }
}

RCXState* RCXOffState_View(RCX* rcx) { return &RCXOffState; }
RCXState* RCXOffState_OnOff(RCX* rcx) { return &RCXOffState; }
RCXState* RCXOffState_Prgm(RCX* rcx) { return &RCXOffState; }
RCXState* RCXOffState_Run(RCX* rcx) { return &RCXOffState; }
RCXState* RCXOffState_Tick(RCX* rcx) { return &RCXOffState; }

RCXState* RCXOnState_View(RCX* rcx) { return &RCXOnState; }
RCXState* RCXOnState_OnOff(RCX* rcx) { return &RCXOnState; }
RCXState* RCXOnState_Prgm(RCX* rcx) { return &RCXOnState; }
RCXState* RCXOnState_Run(RCX* rcx) { return &RCXOnState; }
RCXState* RCXOnState_Tick(RCX* rcx) { return &RCXOnState; }

RCXState* RCXGoState_View(RCX* rcx) { return &RCXGoState; }
RCXState* RCXGoState_OnOff(RCX* rcx) { rcx->turnoff(); return &RCXOffState; }
RCXState* RCXGoState_Prgm(RCX* rcx)
{
  return &RCXBrowseState;
}
RCXState* RCXGoState_Run(RCX* rcx) { return fnRun(); }
RCXState* RCXGoState_Tick(RCX* rcx) { return &RCXGoState; }

RCXState* RCXBrowseState_Prgm(RCX* rcx)
{
  fnIncCommand();
  return &RCXBrowseState;
}
RCXState* RCXBrowseState_OnOff(RCX* rcx) { rcx->turnoff(); return &RCXOffState;}
RCXState* RCXBrowseState_View(RCX* rcx) { return &RCXCmdState; }
RCXState* RCXBrowseState_Run(RCX* rcx) { return fnRun(); }
RCXState* RCXBrowseState_Tick(RCX* rcx)
{
  fnDisplayCommand();
  RCXCursor.set(3);
  RCXCursor.toggle();
  return &RCXBrowseState;
}

static AbstractCommand* oldCmd=NULL;

void fnDestroyOldCmd()
{
  if (oldCmd != NULL)
  {
    free(oldCmd);
    oldCmd = NULL;
  }
}

void fnRecoverOldCmd()
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  if (oldCmd != NULL)
  {
    AbstractCommand* tmp = MyRCX.M_currentMacro->AC_cmds[n];
    MyRCX.M_currentMacro->AC_cmds[n] = oldCmd;
    free(tmp);
    oldCmd = NULL;
  }
}

void fnIncCmdType()
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  AbstractCommand* cmd = MyRCX.M_currentMacro->AC_cmds[n];
  int nextType;
  switch(cmd->type)
  {
    case eCmdDelay:
      nextType=eCmdOutput;
      break;
    case eCmdOutput:
      nextType=eCmdGoto;
      break;
    case eCmdGoto:
      nextType=eCmdDelay;
      break;
    default:
      nextType=cmd->type;
  }
  if (oldCmd != NULL && nextType == oldCmd->type)
    fnRecoverOldCmd();
  else if (nextType != cmd->type)
  {
    if (cmd->type != eCmdNOP && oldCmd == NULL)
      oldCmd = cmd;
    else if (cmd->type == eCmdNOP)
      free(cmd);
    switch(nextType)
    {
      case eCmdDelay:
        cmd = (AbstractCommand*)malloc(sizeof(DelayCommand));
        DelayCommand_constructor((DelayCommand*)cmd, 0);
        break;
      case eCmdOutput:
        cmd = (AbstractCommand*)malloc(sizeof(OutputCommand));
        OutputCommand_constructor((OutputCommand*)cmd, 7, 8, 7);
        break;
      case eCmdGoto:
        cmd = (AbstractCommand*)malloc(sizeof(GotoCommand));
        GotoCommand_constructor((GotoCommand*)cmd, n+1);
        break;
      default:
        cmd = (AbstractCommand*)malloc(sizeof(NOPCommand));
        NOPCommand_constructor((NOPCommand*)cmd, nextType);
        break;
    }
    MyRCX.M_currentMacro->AC_cmds[n] = cmd;
  }
}

RCXState* RCXCmdState_View(RCX* rcx)
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  AbstractCommand* cmd = MyRCX.M_currentMacro->AC_cmds[n];
  fnDestroyOldCmd();
  if (cmd->type == eCmdDelay)
    return &RCXDelayState;
  else if (cmd->type == eCmdOutput)
    return &RCXOutputState;
  else if (cmd->type == eCmdGoto)
    return &RCXGotoState;
  else
    return &RCXCmdState;
}
RCXState* RCXCmdState_OnOff(RCX* rcx)
{
  fnDestroyOldCmd();
  rcx->turnoff(); return &RCXOffState;
}
RCXState* RCXCmdState_Prgm(RCX* rcx)
{
  fnIncCmdType();
  return &RCXCmdState;
}
RCXState* RCXCmdState_Run(RCX* rcx)
{
  fnDestroyOldCmd();
  return fnRun();
}
RCXState* RCXCmdState_Tick(RCX* rcx)
{
  fnDisplayCommand();
  RCXCursor.set(0);
  RCXCursor.toggle();
  return &RCXCmdState;
}

RCXState* RCXDelayState_View(RCX* rcx)
{
  if (RCXCursor.get() == 2)
    return &RCXBrowseState;
  else {
    RCXCursor.set(2);
    return &RCXDelayState;
  }
}
RCXState* RCXDelayState_OnOff(RCX* rcx) { rcx->turnoff(); return &RCXOffState; }
RCXState* RCXDelayState_Prgm(RCX* rcx)
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  DelayCommand* cmd = (DelayCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
  if (RCXCursor.get() == 1)
    cmd->incdelay(cmd);
  else if (RCXCursor.get() == 2)
    cmd->incdelay16(cmd);
  return &RCXDelayState;
}
RCXState* RCXDelayState_Run(RCX* rcx) { return fnRun(); }
RCXState* RCXDelayState_Tick(RCX* rcx)
{
  if (RCXCursor.get() == 0)
    RCXCursor.set(1);
  fnDisplayCommand();
  RCXCursor.toggle();
  return &RCXDelayState;
}

RCXState* RCXGotoState_View(RCX* rcx){
  if (RCXCursor.get() == 2)
    return &RCXBrowseState;
  else {
    RCXCursor.set(2);
    return &RCXGotoState;
  }
}
RCXState* RCXGotoState_OnOff(RCX* rcx) { rcx->turnoff(); return &RCXOffState; }
RCXState* RCXGotoState_Prgm(RCX* rcx){
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  GotoCommand* cmd = (GotoCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
  if (RCXCursor.get() == 1)
    cmd->incgoto(cmd);
  else if (RCXCursor.get() == 2)
    cmd->incgoto16(cmd);
  return &RCXGotoState;
}
RCXState* RCXGotoState_Run(RCX* rcx) { return fnRun(); }
RCXState* RCXGotoState_Tick(RCX* rcx)
{
  if (RCXCursor.get() == 0)
    RCXCursor.set(1);
  fnDisplayCommand();
  RCXCursor.toggle();
  return &RCXGotoState;
}

RCXState* RCXOutputState_View(RCX* rcx)
{
  switch(RCXCursor.get())
  {
    case 1:
      RCXCursor.set(2);
      return &RCXOutputState;
    case 2:
		{
        int n = MyRCX.M_currentMacro->nCurrentCmd;
        OutputCommand* cmd = (OutputCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
        if (cmd->iPort & 1)
          RCXCursor.set(5);
        else if (cmd->iPort & 2)
          RCXCursor.set(6);
        else if (cmd->iPort & 4)
          RCXCursor.set(7);
        else
          return &RCXBrowseState;
      }
      return &RCXOutputState;
    case 5:
		{
        int n = MyRCX.M_currentMacro->nCurrentCmd;
        OutputCommand* cmd = (OutputCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
        if (cmd->iPort & 2)
          RCXCursor.set(6);
        else if (cmd->iPort & 4)
          RCXCursor.set(7);
        else
          return &RCXBrowseState;
      }
      return &RCXOutputState;
    case 6:
		{
        int n = MyRCX.M_currentMacro->nCurrentCmd;
        OutputCommand* cmd = (OutputCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
        if (cmd->iPort & 4)
          RCXCursor.set(7);
        else
          return &RCXBrowseState;
      }
      return &RCXOutputState;
    default:
      return &RCXBrowseState;
  }
}
RCXState* RCXOutputState_OnOff(RCX* rcx) { rcx->turnoff(); return &RCXOffState;}
RCXState* RCXOutputState_Prgm(RCX* rcx)
{
  int n = MyRCX.M_currentMacro->nCurrentCmd;
  OutputCommand* cmd = (OutputCommand*)(MyRCX.M_currentMacro->AC_cmds[n]);
  switch(RCXCursor.get())
  {
    case 1: cmd->incpower(cmd); break;
    case 2: cmd->incport(cmd); break;
    default: cmd->incfwd(cmd, RCXCursor.get()-4); break;
  }
  return &RCXOutputState;
}
RCXState* RCXOutputState_Run(RCX* rcx) { return fnRun(); }
RCXState* RCXOutputState_Tick(RCX* rcx)
{
  if (RCXCursor.get() == 0)
    RCXCursor.set(1);
  fnDisplayCommand();
  RCXCursor.toggle();
  return &RCXOutputState;
}

void RCXStateConstructors()
{
  RCXOffState.view = RCXOffState_View;
  RCXOffState.onOff = RCXOffState_OnOff;
  RCXOffState.prgm = RCXOffState_Prgm;
  RCXOffState.run = RCXOffState_Run;
  RCXOffState.tick = RCXOffState_Tick;

  RCXOnState.view = RCXOnState_View;
  RCXOnState.onOff = RCXOnState_OnOff;
  RCXOnState.prgm = RCXOnState_Prgm;
  RCXOnState.run = RCXOnState_Run;
  RCXOnState.tick = RCXOnState_Tick;

  RCXGoState.view = RCXGoState_View;
  RCXGoState.onOff = RCXGoState_OnOff;
  RCXGoState.prgm = RCXGoState_Prgm;
  RCXGoState.run = RCXGoState_Run;
  RCXGoState.tick = RCXGoState_Tick;

  RCXBrowseState.view = RCXBrowseState_View;
  RCXBrowseState.onOff = RCXBrowseState_OnOff;
  RCXBrowseState.prgm = RCXBrowseState_Prgm;
  RCXBrowseState.run = RCXBrowseState_Run;
  RCXBrowseState.tick = RCXBrowseState_Tick;

  RCXCmdState.view = RCXCmdState_View;
  RCXCmdState.onOff = RCXCmdState_OnOff;
  RCXCmdState.prgm = RCXCmdState_Prgm;
  RCXCmdState.run = RCXCmdState_Run;
  RCXCmdState.tick = RCXCmdState_Tick;

  RCXDelayState.view = RCXDelayState_View;
  RCXDelayState.onOff = RCXDelayState_OnOff;
  RCXDelayState.prgm = RCXDelayState_Prgm;
  RCXDelayState.run = RCXDelayState_Run;
  RCXDelayState.tick = RCXDelayState_Tick;

  RCXOutputState.view = RCXOutputState_View;
  RCXOutputState.onOff = RCXOutputState_OnOff;
  RCXOutputState.prgm = RCXOutputState_Prgm;
  RCXOutputState.run = RCXOutputState_Run;
  RCXOutputState.tick = RCXOutputState_Tick;

  RCXGotoState.view = RCXGotoState_View;
  RCXGotoState.onOff = RCXGotoState_OnOff;
  RCXGotoState.prgm = RCXGotoState_Prgm;
  RCXGotoState.run = RCXGotoState_Run;
  RCXGotoState.tick = RCXGotoState_Tick;
}

/**                                                                         **/
/** RCXState *****************************************************************/
/*****************************************************************************/
