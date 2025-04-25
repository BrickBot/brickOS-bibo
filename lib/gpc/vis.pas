unit vis;

interface

{$I config.inc}

{$IFDEF CONF_VIS}
procedure vis_handler; asmname 'vis_handler';
{$ENDIF}

implementation

end.