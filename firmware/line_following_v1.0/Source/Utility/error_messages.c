/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static char* g_static_error_messages [eErrorCode_Last + 1] = {
    [eErrorCode_OSOK] = "Operation successful (0)\n",
    [eErrorCode_PERM] = "Operation not permitted (1)\n",
    [eErrorCode_NOENT] = "No such file or directory (2)\n",
    [eErrorCode_SRCH] = "No such process (3)\n",
    [eErrorCode_INTR] = "Interrupted system call (4)\n",
    [eErrorCode_IO] = "I/O error (5)\n",
    [eErrorCode_NXIO] = "No such device or address (6)\n",
    [eErrorCode_2BIG] = "Argument list too long (7)\n",
    [eErrorCode_NOEXEC] = "Exec format error (8)\n",
    [eErrorCode_BADF] = "Bad file number (9)\n",
    [eErrorCode_CHILD] = "No child processes (10)\n",
    [eErrorCode_AGAIN] = "Try again (11)\n",
    [eErrorCode_NOMEM] = "Out of memory (12)\n",
    [eErrorCode_ACCES] = "Permission denied (13)\n",
    [eErrorCode_FAULT] = "Bad address (14)\n",
    [eErrorCode_NOTBLK] = "Block device required (15)\n",
    [eErrorCode_BUSY] = "Device or resource busy (16)\n",
    [eErrorCode_EXIST] = "File exists (17)\n",
    [eErrorCode_XDEV] = "Cross-device link (18)\n",
    [eErrorCode_NODEV] = "No such device (19)\n",
    [eErrorCode_NOTDIR] = "Not a directory (20)\n",
    [eErrorCode_ISDIR] = "Is a directory (21)\n",
    [eErrorCode_INVAL] = "Invalid argument (22)\n",
    [eErrorCode_NFILE] = "File table overflow (23)\n",
    [eErrorCode_MFILE] = "Too many open files (24)\n",
    [eErrorCode_NOTTY] = "Not a typewriter (25)\n",
    [eErrorCode_TXTBSY] = "Text file busy (26)\n",
    [eErrorCode_FBIG] = "File too large (27)\n",
    [eErrorCode_NOSPC] = "No space left on device (28)\n",
    [eErrorCode_SPIPE] = "Illegal seek (29)\n",
    [eErrorCode_ROFS] = "Read-only file system (30)\n",
    [eErrorCode_MLINK] = "Too many links (31)\n",
    [eErrorCode_PIPE] = "Broken pipe (32)\n",
    [eErrorCode_DOM] = "Math argument out of domain of func (33)\n",
    [eErrorCode_RANGE] = "Math result not representable (34)\n",
    [eErrorCode_DEADLK] = "Resource deadlock would occur (35)\n",
    [eErrorCode_NAMETOOLONG] = "File name too long (36)\n",
    [eErrorCode_NOLCK] = "No record locks available (37)\n",
    [eErrorCode_NOSYS] = "Function not implemented (38)\n",
    [eErrorCode_NOTEMPTY] = "Directory not empty (39)\n",
    [eErrorCode_LOOP] = "Too many symbolic links encountered (40)\n",
    [eErrorCode_NOMSG] = "No message of desired type (42)\n",
    [eErrorCode_IDRM] = "Identifier removed (43)\n",
    [eErrorCode_CHRNG] = "Channel number out of range (44)\n",
    [eErrorCode_L2NSYNC] = "Level 2 not synchronized (45)\n",
    [eErrorCode_L3HLT] = "Level 3 halted (46)\n",
    [eErrorCode_L3RST] = "Level 3 reset (47)\n",
    [eErrorCode_LNRNG] = "Link number out of range (48)\n",
    [eErrorCode_UNATCH] = "Protocol driver not attached (49)\n",
    [eErrorCode_NOCSI] = "No CSI structure available (50)\n",
    [eErrorCode_L2HLT] = "Level 2 halted (51)\n",
    [eErrorCode_BADE] = "Invalid exchange (52)\n",
    [eErrorCode_BADR] = "Invalid request descriptor (53)\n",
    [eErrorCode_XFULL] = "Exchange full (54)\n",
    [eErrorCode_NOANO] = "No anode (55)\n",
    [eErrorCode_BADRQC] = "Invalid request code (56)\n",
    [eErrorCode_BADSLT] = "Invalid slot (57)\n",
    [eErrorCode_BFONT] = "Bad font file format (59)\n",
    [eErrorCode_NOSTR] = "Device not a stream (60)\n",
    [eErrorCode_NODATA] = "No data available (61)\n",
    [eErrorCode_TIME] = "Timer expired (62)\n",
    [eErrorCode_NOSR] = "Out of streams resources (63)\n",
    [eErrorCode_NONET] = "Machine is not on the network (64)\n",
    [eErrorCode_NOPKG] = "Package not installed (65)\n",
    [eErrorCode_REMOTE] = "Object is remote (66)\n",
    [eErrorCode_NOLINK] = "Link has been severed (67)\n",
    [eErrorCode_ADV] = "Advertise error (68)\n",
    [eErrorCode_SRMNT] = "Srmount error (69)\n",
    [eErrorCode_COMM] = "Communication error on send (70)\n",
    [eErrorCode_PROTO] = "Protocol error (71)\n",
    [eErrorCode_MULTIHOP] = "Multihop attempted (72)\n",
    [eErrorCode_DOTDOT] = "RFS specific error (73)\n",
    [eErrorCode_BADMSG] = "Not a data message (74)\n",
    [eErrorCode_OVERFLOW] = "Value too large for defined data type (75)\n",
    [eErrorCode_NOTUNIQ] = "Name not unique on network (76)\n",
    [eErrorCode_BADFD] = "File descriptor in bad state (77)\n",
    [eErrorCode_REMCHG] = "Remote address changed (78)\n",
    [eErrorCode_LIBACC] = "Cannot access a needed shared library (79)\n",
    [eErrorCode_LIBBAD] = "Accessing a corrupted shared library (80)\n",
    [eErrorCode_LIBSCN] = ".lib section in a.out corrupted (81)\n",
    [eErrorCode_LIBMAX] = "Attempting to link in too many shared libraries (82)\n",
    [eErrorCode_LIBEXEC] = "Cannot exec a shared library directly (83)\n",
    [eErrorCode_ILSEQ] = "Illegal byte sequence (84)\n",
    [eErrorCode_RESTART] = "Interrupted system call should be restarted (85)\n",
    [eErrorCode_STRPIPE] = "Streams pipe error (86)\n",
    [eErrorCode_USERS] = "Too many users (87)\n",
    [eErrorCode_NOTSOCK] = "Socket operation on non-socket (88)\n",
    [eErrorCode_DESTADDRREQ] = "Destination address required (89)\n",
    [eErrorCode_MSGSIZE] = "Message too long (90)\n",
    [eErrorCode_PROTOTYPE] = "Protocol wrong type for socket (91)\n",
    [eErrorCode_NOPROTOOPT] = "Protocol not available (92)\n",
    [eErrorCode_PROTONOSUPPORT] = "Protocol not supported (93)\n",
    [eErrorCode_SOCKTNOSUPPORT] = "Socket type not supported (94)\n",
    [eErrorCode_OPNOTSUPP] = "Operation not supported on transport endpoint (95)\n",
    [eErrorCode_PFNOSUPPORT] = "Protocol family not supported (96)\n",
    [eErrorCode_AFNOSUPPORT] = "Address family not supported by protocol (97)\n",
    [eErrorCode_ADDRINUSE] = "Address already in use (98)\n",
    [eErrorCode_ADDRNOTAVAIL] = "Cannot assign requested address (99)\n",
    [eErrorCode_NETDOWN] = "Network is down (100)\n",
    [eErrorCode_NETUNREACH] = "Network is unreachable (101)\n",
    [eErrorCode_NETRESET] = "Network dropped connection because of reset (102)\n",
    [eErrorCode_CONNABORTED] = "Software caused connection abort (103)\n",
    [eErrorCode_CONNRESET] = "Connection reset by peer (104)\n",
    [eErrorCode_NOBUFS] = "No buffer space available (105)\n",
    [eErrorCode_ISCONN] = "Transport endpoint is already connected (106)\n",
    [eErrorCode_NOTCONN] = "Transport endpoint is not connected (107)\n",
    [eErrorCode_SHUTDOWN] = "Cannot send after transport endpoint shutdown (108)\n",
    [eErrorCode_TOOMANYREFS] = "Too many references: cannot splice (109)\n",
    [eErrorCode_TIMEDOUT] = "Connection timed out (110)\n",
    [eErrorCode_CONNREFUSED] = "Connection refused (111)\n",
    [eErrorCode_HOSTDOWN] = "Host is down (112)\n",
    [eErrorCode_HOSTUNREACH] = "No route to host (113)\n",
    [eErrorCode_ALREADY] = "Operation already in progress (114)\n",
    [eErrorCode_INPROGRESS] = "Operation now in progress (115)\n",
    [eErrorCode_STALE] = "Stale NFS file handle (116)\n",
    [eErrorCode_UCLEAN] = "Structure needs cleaning (117)\n",
    [eErrorCode_NOTNAM] = "Not a XENIX named type file (118)\n",
    [eErrorCode_NAVAIL] = "No XENIX semaphores available (119)\n",
    [eErrorCode_ISNAM] = "Is a named type file (120)\n",
    [eErrorCode_REMOTEIO] = "Remote I/O error (121)\n",
    [eErrorCode_DQUOT] = "Quota exceeded (122)\n",
    [eErrorCode_NOMEDIUM] = "No medium found (123)\n",
    [eErrorCode_MEDIUMTYPE] = "Wrong medium type (124)\n",
    [eErrorCode_CANCELED] = "Operation Canceled (125)\n",
    [eErrorCode_NOKEY] = "Required key not available (126)\n",
    [eErrorCode_KEYEXPIRED] = "Key has expired (127)\n",
    [eErrorCode_KEYREVOKED] = "Key has been revoked (128)\n",
    [eErrorCode_KEYREJECTED] = "Key was rejected by service (129)\n",
    [eErrorCode_OWNERDEAD] = "Owner died (130)\n",
    [eErrorCode_NOTRECOVERABLE] = "State not recoverable (131)\n",
    [eErrorCode_Last] = "Unknown error code\n"
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

const char* Error_Message_To_String(eErrorCode_t error_code) {
    if ((error_code < eErrorCode_First) || (error_code >= eErrorCode_Last)) {
        return g_static_error_messages[eErrorCode_Last];
    }
    
    return g_static_error_messages[error_code];
}
