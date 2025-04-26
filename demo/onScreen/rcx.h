/*****************************************************************************/
/** RCX  *********************************************************************/
/**                                                                         **/

typedef struct RCX {
  Macro** M_macros;
  int nMacros;
  Macro* M_currentMacro;
  int nCurrentMacro;
  void (*turnoff)();
  void (*run)(struct RCX* rcx);
  void (*stop)(struct RCX* rcx);
  struct RCXState* myState;
} RCX;

void RCXexecutor(RCX* rcx)
{
  if (rcx->M_currentMacro != NULL)
  {
    rcx->M_currentMacro->nCurrentCmd = 0;

    while (!(shutdown_requested()) && (rcx->M_currentMacro->nCurrentCmd < rcx->M_currentMacro->nCmds))
    {
      AbstractCommand* ac =
        rcx->M_currentMacro->AC_cmds[rcx->M_currentMacro->nCurrentCmd];
      if (ac->type == eCmdGoto)
      {
        GotoCommand* cmd = (GotoCommand*)ac;
        if (cmd->iGoto < rcx->M_currentMacro->nCmds)
          rcx->M_currentMacro->nCurrentCmd = cmd->iGoto;
      }
      else
      {
        ac->execute(ac);
        ++(rcx->M_currentMacro->nCurrentCmd);
      }
    }

    motor_a_dir(off);
    motor_b_dir(off);
    motor_c_dir(off);
  }
}

int taskRun();
//int taskButtonRun();
//int taskButtonOnOff();
int taskButtonPrgm();
int taskButtonView();
int taskShutdownRequested();

static void taskTimer(void*);
timer_t tTaskTimer = {0, taskTimer };

tid_t tRun = NULL;
//tid_t tTimer = NULL;
tid_t tButtonView = NULL;
//tid_t tButtonRun = NULL;
//tid_t tButtonOnOff = NULL;
tid_t tButtonPrgm = NULL;
tid_t tShutdownRequested = NULL;

wakeup_t button_release_wakeup(wakeup_t data);

void RCXrun(RCX* rcx)
{
  add_timer(100, &tTaskTimer);

//  taskTimer(NULL);

//  tTimer = execi(&taskTimer, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
  tButtonView = execi(&taskButtonView, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
//  tButtonRun = execi(&taskButtonRun, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
//  tButtonOnOff = execi(&taskButtonOnOff, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
  tButtonPrgm = execi(&taskButtonPrgm, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
  tShutdownRequested = execi(&taskShutdownRequested, 0, NULL, PRIO_LOWEST, DEFAULT_STACK_SIZE);
}

void RCXstop(RCX* rcx)
{
//  lcd_show( 0x3018 /* lcd_segment.circle */ );
  remove_timer(&tTaskTimer);

//  if (tTimer != NULL)
//  {
//    kill(tTimer);
//    tTimer = NULL;
//  }
  if (tRun != NULL)
  {
    shutdown_task(tRun);
    tRun = NULL;
  }
  if (tButtonView != NULL)
  {
    shutdown_task(tButtonView);
    tButtonView = NULL;
  }
  if (tButtonPrgm != NULL)
  {
    shutdown_task(tButtonPrgm);
    tButtonPrgm = NULL;
  }
//  if (tButtonRun != NULL)
//  {
//    kill(tButtonRun);
//    tButtonRun = NULL;
//  }
//  if (tButtonOnOff != NULL)
//  {
//    kill(tButtonOnOff);
//    tButtonOnOff = NULL;
//  }

  if (tShutdownRequested != NULL)
  {
    shutdown_task(tShutdownRequested);
    tShutdownRequested = NULL;
  }

  lcd_clear();
}

void RCXturnoff()
{
  extern RCX MyRCX;
  MyRCX.stop(&MyRCX);
}

void RCX_constructor(RCX* rcx)
{
  rcx->M_macros = (Macro**)malloc(sizeof(Macro*));
  rcx->nMacros = 1;
  rcx->nCurrentMacro = 1;
  rcx->M_currentMacro = (Macro*)malloc(sizeof(Macro));
  Macro_constructor(rcx->M_currentMacro);
  rcx->M_macros[0] = rcx->M_currentMacro;
  rcx->turnoff = RCXturnoff;
  rcx->run = RCXrun;
  rcx->stop = RCXstop;
  rcx->myState = NULL;
}

/**                                                                         **/
/** RCX **********************************************************************/
/*****************************************************************************/

