/*****************************************************************************/
/** Macro ********************************************************************/
/**                                                                         **/

typedef struct Macro {
  AbstractCommand** AC_cmds;
  int nCmds;
  int nCurrentCmd;
  void (*add)(struct Macro*, AbstractCommand* ac);
  void (*delete_currentcmd)(struct Macro*);
  void (*erase)(struct Macro*);
} Macro;

void Macro_fn_add(Macro* macro, AbstractCommand* ac)
{
  if (macro->AC_cmds == NULL || macro->nCmds == 0)
  {
    macro->AC_cmds = (AbstractCommand**)malloc(sizeof(AbstractCommand*)*256);
    macro->nCmds = 0;
  }
  if (macro->nCmds < 256)
  {
    macro->AC_cmds[macro->nCmds] = ac;
    ++(macro->nCmds);
  }
}

void Macro_fn_delete_currentcmd(Macro* macro)
{
  int i;

  if (macro->nCmds == 0)
    return;

  for (i=macro->nCurrentCmd+1; i<macro->nCmds; ++i)
  {
    macro->AC_cmds[i-1] = macro->AC_cmds[i];
  }

  --macro->nCmds;

  if (macro->nCmds == 0)
  {
    free(macro->AC_cmds);
    macro->AC_cmds = NULL;
  }
}

void Macro_fn_erase(Macro* macro)
{
  if (macro->nCmds > 0)
  {
    int i;
    for (i=0; i < macro->nCmds; ++i)
      free(macro->AC_cmds[i]);
    free(macro->AC_cmds);
  }
  macro->AC_cmds = NULL;
  macro->nCurrentCmd = 0;
}

void Macro_constructor(Macro* macro)
{
  macro->add = Macro_fn_add;
  macro->erase = Macro_fn_erase;
  macro->delete_currentcmd = Macro_fn_delete_currentcmd;
  macro->nCmds = 0;
  macro->erase(macro);
}

/*****************************************************************************/
