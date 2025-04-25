#ifndef SOURCE_UTILITY_ERROR_MESSAGES_H_
#define SOURCE_UTILITY_ERROR_MESSAGES_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eErrorCode {
    eErrorCode_First = 0,
    eErrorCode_OSOK = eErrorCode_First,            // 0: Operation succesful
    eErrorCode_PERM,                               // 1: Operation not permitted
    eErrorCode_NOENT,                              // 2: No such file or directory
    eErrorCode_SRCH,                               // 3: No such process
    eErrorCode_INTR,                               // 4: Interrupted system call
    eErrorCode_IO,                                 // 5: I/O error
    eErrorCode_NXIO,                               // 6: No such device or address
    eErrorCode_2BIG,                               // 7: Argument list too long
    eErrorCode_NOEXEC,                             // 8: Exec format error
    eErrorCode_BADF,                               // 9: Bad file number
    eErrorCode_CHILD,                              // 10: No child processes
    eErrorCode_AGAIN,                              // 11: Try again
    eErrorCode_NOMEM,                              // 12: Out of memory
    eErrorCode_ACCES,                              // 13: Permission denied
    eErrorCode_FAULT,                              // 14: Bad address
    eErrorCode_NOTBLK,                             // 15: Block device required
    eErrorCode_BUSY,                               // 16: Device or resource busy
    eErrorCode_EXIST,                              // 17: File exists
    eErrorCode_XDEV,                               // 18: Cross-device link
    eErrorCode_NODEV,                              // 19: No such device
    eErrorCode_NOTDIR,                             // 20: Not a directory
    eErrorCode_ISDIR,                              // 21: Is a directory
    eErrorCode_INVAL,                              // 22: Invalid argument
    eErrorCode_NFILE,                              // 23: File table overflow
    eErrorCode_MFILE,                              // 24: Too many open files
    eErrorCode_NOTTY,                              // 25: Not a typewriter
    eErrorCode_TXTBSY,                             // 26: Text file busy
    eErrorCode_FBIG,                               // 27: File too large
    eErrorCode_NOSPC,                              // 28: No space left on device
    eErrorCode_SPIPE,                              // 29: Illegal seek
    eErrorCode_ROFS,                               // 30: Read-only file system
    eErrorCode_MLINK,                              // 31: Too many links
    eErrorCode_PIPE,                               // 32: Broken pipe
    eErrorCode_DOM,                                // 33: Math argument out of domain of func
    eErrorCode_RANGE,                              // 34: Math result not representable
    eErrorCode_DEADLK,                             // 35: Resource deadlock would occur
    eErrorCode_NAMETOOLONG,                        // 36: File name too long
    eErrorCode_NOLCK,                              // 37: No record locks available
    eErrorCode_NOSYS,                              // 38: Function not implemented
    eErrorCode_NOTEMPTY,                           // 39: Directory not empty
    eErrorCode_LOOP,                               // 40: Too many symbolic links encountered
    eErrorCode_NOMSG,                              // 42: No message of desired type
    eErrorCode_IDRM,                               // 43: Identifier removed
    eErrorCode_CHRNG,                              // 44: Channel number out of range
    eErrorCode_L2NSYNC,                            // 45: Level 2 not synchronized
    eErrorCode_L3HLT,                              // 46: Level 3 halted
    eErrorCode_L3RST,                              // 47: Level 3 reset
    eErrorCode_LNRNG,                              // 48: Link number out of range
    eErrorCode_UNATCH,                             // 49: Protocol driver not attached
    eErrorCode_NOCSI,                              // 50: No CSI structure available
    eErrorCode_L2HLT,                              // 51: Level 2 halted
    eErrorCode_BADE,                               // 52: Invalid exchange
    eErrorCode_BADR,                               // 53: Invalid request descriptor
    eErrorCode_XFULL,                              // 54: Exchange full
    eErrorCode_NOANO,                              // 55: No anode
    eErrorCode_BADRQC,                             // 56: Invalid request code
    eErrorCode_BADSLT,                             // 57: Invalid slot
    eErrorCode_BFONT,                              // 59: Bad font file format
    eErrorCode_NOSTR,                              // 60: Device not a stream
    eErrorCode_NODATA,                             // 61: No data available
    eErrorCode_TIME,                               // 62: Timer expired
    eErrorCode_NOSR,                               // 63: Out of streams resources
    eErrorCode_NONET,                              // 64: Machine is not on the network
    eErrorCode_NOPKG,                              // 65: Package not installed
    eErrorCode_REMOTE,                             // 66: Object is remote
    eErrorCode_NOLINK,                             // 67: Link has been severed
    eErrorCode_ADV,                                // 68: Advertise error
    eErrorCode_SRMNT,                              // 69: Srmount error
    eErrorCode_COMM,                               // 70: Communication error on send
    eErrorCode_PROTO,                              // 71: Protocol error
    eErrorCode_MULTIHOP,                           // 72: Multihop attempted
    eErrorCode_DOTDOT,                             // 73: RFS specific error
    eErrorCode_BADMSG,                             // 74: Not a data message
    eErrorCode_OVERFLOW,                           // 75: Value too large for defined data type
    eErrorCode_NOTUNIQ,                            // 76: Name not unique on network
    eErrorCode_BADFD,                              // 77: File descriptor in bad state
    eErrorCode_REMCHG,                             // 78: Remote address changed
    eErrorCode_LIBACC,                             // 79: Cannot access a needed shared library
    eErrorCode_LIBBAD,                             // 80: Accessing a corrupted shared library
    eErrorCode_LIBSCN,                             // 81: .lib section in a.out corrupted
    eErrorCode_LIBMAX,                             // 82: Attempting to link in too many shared libraries
    eErrorCode_LIBEXEC,                            // 83: Cannot exec a shared library directly
    eErrorCode_ILSEQ,                              // 84: Illegal byte sequence
    eErrorCode_RESTART,                            // 85: Interrupted system call should be restarted
    eErrorCode_STRPIPE,                            // 86: Streams pipe error
    eErrorCode_USERS,                              // 87: Too many users
    eErrorCode_NOTSOCK,                            // 88: Socket operation on non-socket
    eErrorCode_DESTADDRREQ,                        // 89: Destination address required
    eErrorCode_MSGSIZE,                            // 90: Message too long
    eErrorCode_PROTOTYPE,                          // 91: Protocol wrong type for socket
    eErrorCode_NOPROTOOPT,                         // 92: Protocol not available
    eErrorCode_PROTONOSUPPORT,                     // 93: Protocol not supported
    eErrorCode_SOCKTNOSUPPORT,                     // 94: Socket type not supported
    eErrorCode_OPNOTSUPP,                          // 95: Operation not supported on transport endpoint
    eErrorCode_PFNOSUPPORT,                        // 96: Protocol family not supported
    eErrorCode_AFNOSUPPORT,                        // 97: Address family not supported by protocol
    eErrorCode_ADDRINUSE,                          // 98: Address already in use
    eErrorCode_ADDRNOTAVAIL,                       // 99: Cannot assign requested address
    eErrorCode_NETDOWN,                            // 100: Network is down
    eErrorCode_NETUNREACH,                         // 101: Network is unreachable
    eErrorCode_NETRESET,                           // 102: Network dropped connection because of reset
    eErrorCode_CONNABORTED,                        // 103: Software caused connection abort
    eErrorCode_CONNRESET,                          // 104: Connection reset by peer
    eErrorCode_NOBUFS,                             // 105: No buffer space available
    eErrorCode_ISCONN,                             // 106: Transport endpoint is already connected
    eErrorCode_NOTCONN,                            // 107: Transport endpoint is not connected
    eErrorCode_SHUTDOWN,                           // 108: Cannot send after transport endpoint shutdown
    eErrorCode_TOOMANYREFS,                        // 109: Too many references: cannot splice
    eErrorCode_TIMEDOUT,                           // 110: Connection timed out
    eErrorCode_CONNREFUSED,                        // 111: Connection refused
    eErrorCode_HOSTDOWN,                           // 112: Host is down
    eErrorCode_HOSTUNREACH,                        // 113: No route to host
    eErrorCode_ALREADY,                            // 114: Operation already in progress
    eErrorCode_INPROGRESS,                         // 115: Operation now in progress
    eErrorCode_STALE,                              // 116: Stale NFS file handle
    eErrorCode_UCLEAN,                             // 117: Structure needs cleaning
    eErrorCode_NOTNAM,                             // 118: Not a XENIX named type file
    eErrorCode_NAVAIL,                             // 119: No XENIX semaphores available
    eErrorCode_ISNAM,                              // 120: Is a named type file
    eErrorCode_REMOTEIO,                           // 121: Remote I/O error
    eErrorCode_DQUOT,                              // 122: Quota exceeded
    eErrorCode_NOMEDIUM,                           // 123: No medium found
    eErrorCode_MEDIUMTYPE,                         // 124: Wrong medium type
    eErrorCode_CANCELED,                           // 125: Operation Canceled
    eErrorCode_NOKEY,                              // 126: Required key not available
    eErrorCode_KEYEXPIRED,                         // 127: Key has expired
    eErrorCode_KEYREVOKED,                         // 128: Key has been revoked
    eErrorCode_KEYREJECTED,                        // 129: Key was rejected by service
    eErrorCode_OWNERDEAD,                          // 130: Owner died
    eErrorCode_NOTRECOVERABLE,                     // 131: State not recoverable
    eErrorCode_Last                                 
} eErrorCode_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

const char* Error_Message_To_String(eErrorCode_t error_code);

#endif /* SOURCE_UTILITY_ERROR_MESSAGES_H_ */
