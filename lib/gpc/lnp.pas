unit lnp;

interface

uses
  types, h8, time, unistd;

{$I config.inc}

const
  LNP_HOSTMASK = $f0;
  LNP_PORTMASK = $00ff and not LNP_HOSTMASK;

{$UNDEF PROTOCOL_OR_MESSAGE}

{$IFDEF CONF_RCX_PROTOCOL}
{$DEFINE PROTOCOL_OR_MESSAGE}
{$ENDIF}

{$IFDEF CONF_RCX_MESSAGE}
{$DEFINE PROTOCOL_OR_MESSAGE}
{$ENDIF}

{$IFDEF PROTOCOL_OR_MESSAGE}
const
  LNP_RCX_HEADER_LENGTH    = 3-1;
  LNP_RCX_REMOTE_OP_LENGTH = 5-3;
  LNP_RCX_MSG_OP_LENGTH    = 5-3;

{$ENDIF}

type
  lnp_integrity_state_t = (
    LNPwaitHeader,
    LNPwaitLength,
    LNPwaitData,
    LNPwaitCRC,
{$IFDEF PROTOCOL_OR_MESSAGE}
    //! states when waiting for rcx protocol message
    LNPwaitRMH1,  // note that first byte is consumed by LNPwaitHeader
    LNPwaitRMH2,  // inverted header
    LNPwaitRMH3,  // actually, RCX opcode dispatch
    LNPwaitRMH4,  // remote opcode inverted
{$ENDIF}
{$IFDEF CONF_RCX_PROTOCOL}
    //! states when waiting for remote buttons args
    LNPwaitRB0, // high-byte
    LNPwaitRB0I,
    LNPwaitRB1, // low-byte
    LNPwaitRB1I,
    LNPwaitRC,  // RCX checksum
    LNPwaitRCI,
{$ENDIF}
{$IFDEF CONF_RCX_MESSAGE}
    //! states when waiting for rcx message opcode
    LNPwaitMH3, // RCX message OP
    LNPwaitMH4,
    LNPwaitMN,  // message number
    LNPwaitMNC,
    LNPwaitMC,  // RCX checksum
    LNPwaitMCC,
{$ENDIF}
    LNP_error
  );

var
  lnp_timeout_counter : unsigned_short; asmname 'lnp_timeout_counter'; external;
  lnp_timeout : unsigned_short; asmname 'lnp_timeout'; external;
  lnp_integrity_state : lnp_integrity_state_t; asmname 'lnp_integrity_state'; external;

function lnp_checksum_copy(dest : punsigned_char; const data : punsigned_char;
  length : unsigned) : unsigned_char; asmname 'lnp_checksum_copy';
procedure lnp_integrity_byte(b : unsigned_char); asmname 'lnp_integrity_byte';
procedure lnp_integrity_reset; asmname 'lnp_integrity_reset';

function lnp_integrity_active : boolean;
procedure lnp_timeout_reset; asmname 'lnp_timeout_reset';
procedure lnp_timeout_set(timeout : unsigned_short);
procedure lnp_init; asmname 'lnp_init';

type
  lnp_integrity_handler_t = ^procedure(const punsigned_char; unsigned_char);
  lnp_addressing_handler_t = ^procedure(const punsigned_char; unsigned_char; unsigned_char);

var
  LNP_DUMMY_INTEGRITY : lnp_integrity_handler_t = nil;
  LNP_DUMMY_ADDRESSING : lnp_addressing_handler_t = nil;

{$IFDEF CONF_RCX_PROTOCOL}
type
  lnp_remote_handler_t = ^procedure(unsigned_int);

var
  LNP_DUMMY_REMOTE : lnp_remote_handler_t = nil;
{$ENDIF}

var
  lnp_integrity_handler : lnp_integrity_handler_t; asmname 'lnp_integrity_handler'; external;
  lnp_addressing_handler : array[0..LNP_PORTMASK] of lnp_addressing_handler_t; asmname 'lnp_addressing_handler'; external;
  lnp_hostaddr : unsigned_char; asmname 'lnp_hostaddr'; external;

{$IFDEF CONF_RCX_PROTOCOL}
var
  lnp_remote_handler : lnp_remote_handler_t; asmname 'lnp_remote_handler'; external;
{$ENDIF}

procedure lnp_integrity_set_handler(handler : lnp_integrity_handler_t);
procedure lnp_addressing_set_handler(port : unsigned_char; handler : lnp_addressing_handler_t);
procedure lnp_set_hostaddr(host : unsigned_char);

{$IFDEF CONF_RCX_PROTOCOL}
procedure lnp_remote_set_handler(handler : lnp_remote_handler_t);
{$ENDIF}

{$IFDEF CONF_RCX_MESSAGE}
var
  lnp_rcx_message : unsigned_char; asmname 'lnp_rcx_message'; external;

function send_msg(msg : unsigned_char) : int; asmname 'send_msg';
procedure clear_msg;
function msg_received(data : wakeup_t) : wakeup_t; asmname 'msg_received';
function get_msg : unsigned_char;
{$ENDIF}

function lnp_integrity_write(const data : punsigned_char;
  length : unsigned_char) : int; asmname 'lnp_integrity_write';
function lnp_addressing_write(const data : punsigned_char;
  length, dest, srcport : unsigned_char) : int; asmname 'lnp_addressing_write';

function lnp_logical_write(const buf : pointer; len : size_t) : int; asmname 'lnp_logical_write';
procedure lnp_logical_fflush; asmname 'lnp_logical_fflush';

const
{$IFDEF CONF_LNP_FAST}
  LNP_LOGICAL_BAUD_RATE = B4800;
  LNP_LOGICAL_PARITY    = SMR_P_NONE;
  LNP_BYTE_TIME         = 3 div TICK_IN_MS; // avoiding call to function MSecsToTicks;
{$ELSE}
  LNP_LOGICAL_BAUD_RATE = B2400;
  LNP_LOGICAL_PARITY    = SMR_P_ODD;
  LNP_BYTE_TIME         = 5 div TICK_IN_MS; // avoiding call to function MSecsToTicks;
{$ENDIF}
  LNP_BYTE_TIMEOUT      = 3 * LNP_BYTE_TIME;
  LNP_BYTE_SAFE         = 4 * LNP_BYTE_TIME;
  LNP_WAIT_TXOK         = 2 * LNP_BYTE_TIMEOUT;
  LNP_WAIT_COLL         = 4 * LNP_BYTE_TIMEOUT;
  TX_COLL               = -1;
  TX_IDLE               =  0;
  TX_ACTIVE             =  1;

var
  tx_state : signed_char; asmname 'tx_state'; external;

procedure lnp_logical_init; asmname 'lnp_logical_init';
procedure lnp_logical_shutdown; asmname 'lnp_logical_shutdown';
procedure txend_handler; asmname 'txend_handler';

procedure lnp_logical_range(IsFar : boolean);
function lnp_logical_range_is_far : boolean;

implementation

function lnp_integrity_active : boolean;
begin
  result := lnp_integrity_state <> LNPwaitHeader;
end;

procedure lnp_timeout_set(timeout : unsigned_short);
begin
  lnp_timeout         := timeout;
  lnp_timeout_counter := timeout;
end;

procedure lnp_integrity_set_handler(handler : lnp_integrity_handler_t);
begin
  lnp_integrity_handler := handler;
end;

procedure lnp_addressing_set_handler(port : unsigned_char; handler : lnp_addressing_handler_t);
begin
  if (not (port and LNP_HOSTMASK)) <> 0 then
    lnp_addressing_handler[port] := handler;
end;

procedure lnp_set_hostaddr(host : unsigned_char);
begin
  lnp_hostaddr := ((host shl 4) and LNP_HOSTMASK);
end;

{$IFDEF CONF_RCX_PROTOCOL}
procedure lnp_remote_set_handler(handler : lnp_remote_handler_t);
begin
  lnp_remote_handler := handler;
end;
{$ENDIF}

{$IFDEF CONF_RCX_MESSAGE}
procedure clear_msg;
begin
  lnp_rcx_message := 0;
end;

function get_msg : unsigned_char;
begin
  clear_msg;
  wait_event(@msg_received, 0);
  result := lnp_rcx_message;
end;

{$ENDIF}

procedure lnp_logical_range(IsFar : boolean);
begin
  if IsFar then
    PORT4 := (PORT4 and (not 1))
  else
    PORT4 := (PORT4 or 1);
end;

function lnp_logical_range_is_far : boolean;
begin
  result := (PORT4 and 1) <> 0;
end;


end.
