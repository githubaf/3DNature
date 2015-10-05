
/***************************************************************************
* SPROMEPS.H
*
* (C) Copyright 1989-2002 Rainbow Technologies, Inc. All rights reserved.
*
* Description - SuperPro Multiple Entry Points Header file.
*
* Purpose     - This module provides a method for performing SuperPro API
*               commands so you do not have to deal with command packets.
*               It provides a function for each API command.             
*
* Revision 1.1 - Added new error codes and API's for 6.1
* Revision 1.2 - Aded new API's and typedef's for 6.2.0
*
****************************************************************************/
#ifndef _SPROMEPS_H
#define _SPROMEPS_H

                                       /* for dword alignment roll-up      */
#define SPRO_APIPACKET_ALIGNMENT_VALUE (sizeof(unsigned long))
#define SPRO_APIPACKET_SIZE            (1024+SPRO_APIPACKET_ALIGNMENT_VALUE)
#define SPRO_MAX_QUERY_SIZE            56              /* in bytes         */

/**  SuperPro API error codes.  **/
#define SP_SUCCESS                      0
#define SP_INVALID_FUNCTION_CODE        1
#define SP_INVALID_PACKET               2
#define SP_UNIT_NOT_FOUND               3
#define SP_ACCESS_DENIED                4
#define SP_INVALID_MEMORY_ADDRESS       5
#define SP_INVALID_ACCESS_CODE          6
#define SP_PORT_IS_BUSY                 7
#define SP_WRITE_NOT_READY              8
#define SP_NO_PORT_FOUND                9
#define SP_ALREADY_ZERO                 10
#define SP_DRIVER_OPEN_ERROR            11
#define SP_DRIVER_NOT_INSTALLED         12
#define SP_IO_COMMUNICATIONS_ERROR      13
#define SP_PACKET_TOO_SMALL             15
#define SP_INVALID_PARAMETER            16
#define SP_MEM_ACCESS_ERROR             17
#define SP_VERSION_NOT_SUPPORTED        18
#define SP_OS_NOT_SUPPORTED             19
#define SP_QUERY_TOO_LONG               20
#define SP_INVALID_COMMAND              21
#define SP_MEM_ALIGNMENT_ERROR          29
#define SP_DRIVER_IS_BUSY               30
#define SP_PORT_ALLOCATION_FAILURE      31
#define SP_PORT_RELEASE_FAILURE         32
#define SP_ACQUIRE_PORT_TIMEOUT         39
#define SP_SIGNAL_NOT_SUPPORTED         42
#define SP_UNKNOWN_MACHINE              44
#define SP_SYS_API_ERROR                45
#define SP_UNIT_IS_BUSY                 46
#define SP_INVALID_PORT_TYPE            47
#define SP_INVALID_MACH_TYPE            48
#define SP_INVALID_IRQ_MASK             49
#define SP_INVALID_CONT_METHOD          50
#define SP_INVALID_PORT_FLAGS           51
#define SP_INVALID_LOG_PORT_CFG         52
#define SP_INVALID_OS_TYPE              53
#define SP_INVALID_LOG_PORT_NUM         54
#define SP_INVALID_ROUTER_FLGS          56
#define SP_INIT_NOT_CALLED              57
#define SP_DRVR_TYPE_NOT_SUPPORTED      58
#define SP_FAIL_ON_DRIVER_COMM          59

#define SP_SERVER_PROBABLY_NOT_UP       60
#define SP_UNKNOWN_HOST                 61
#define SP_SENDTO_FAILED                62
#define SP_SOCKET_CREATION_FAILED       63
#define SP_NORESOURCES                  64
#define SP_BROADCAST_NOT_SUPPORTED      65
#define SP_BAD_SERVER_MESSAGE           66
#define SP_NO_SERVER_RUNNING            67
#define SP_NO_NETWORK                   68
#define SP_NO_SERVER_RESPONSE           69
#define SP_NO_LICENSE_AVAILABLE         70
#define SP_INVALID_LICENSE              71
#define SP_INVALID_OPERATION            72
#define SP_BUFFER_TOO_SMALL             73
#define SP_INTERNAL_ERROR               74
#define SP_PACKET_ALREADY_INITIALIZED   75
#define SP_PROTOCOL_NOT_INSTALLED       76


/* Define possible driver types (Not used refer to Driver Types). */
#define SP_DRVR_WIN31             4
#define SP_DRVR_WINNT             5

/* Create SP types */
#ifdef __cplusplus
#define SP_EXPORT extern "C"
#else
#define SP_EXPORT extern
#endif
#define SP_LOCAL static

typedef int ENUM_SERVER_FLAG;


#ifndef _RBDRVR_INC

#if (defined(_OS2_) || defined(__OS2__) || defined(OS2_INCLUDED) || defined(_OS2_H))
#ifndef _OS2_
#define _OS2_
#endif
#endif

#if (defined(_QNX_) || defined(__QNX__))
#ifndef _QNX_
#define _QNX_ 1
#endif
#endif

#if ((defined(_NW_) || defined(CLIB_V311)) && !defined(_OS2_))
#ifndef _NW_
#define _NW_ 1
#endif
#endif

#if ((defined(_WIN32_) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(_NW_) && !defined(_QNX_))
#ifndef _WIN32_
#define _WIN32_ 1
#endif
#endif

#if ((defined(_WIN3_) || defined(__WINDOWS_H) || defined(_WINDOWS_) || \
      defined(_WIN_)  || defined(__WINDOWS__) || defined(_WINDOWS)  || \
      defined(_WINDLL)) && !defined(_OS2_) && !defined(_WIN32_))
#ifndef _WIN_
#define _WIN_ 1
#endif
#endif

#if (!defined(_OS2_) && !defined(_WIN32_) && !defined(_WIN_) && !defined(_NW_) && !defined(_QNX_))
#ifndef _DOS_
#define _DOS_ 1
#endif
#endif

#endif
/****************************************************************************
* SETTINGS FOR GNU C
****************************************************************************/
#if (defined(__GNUC__) || defined(_GNUC_) || defined(__EMX__))
#if defined(_OS2_)
#define _RB_SYSCALL  _System
#define _RB_STDCALL
#define _RB_FASTCALL
#define _RB_PASCAL   _Pascal
#define _RB_CDECL    _Cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      EXPENTRY
#define _RB_PTR      *
#endif
#endif
/***************************************************************************/

/****************************************************************************
* SETTINGS FOR IBM C SET/2
#if ((defined(__IBMC__) || defined(__IBMCPP__)) && !defined(_GNUC_))
#if defined(_OS2_)
#define _RB_SYSCALL  _system
#define _RB_STDCALL
#define _RB_FASTCALL _fastcall
#define _RB_PASCAL   _pascal
#define _RB_CDECL    _cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      EXPENTRY
#define _RB_PTR      *
#endif
#endif
****************************************************************************/

/***************************************************************************/

//*****************************************************************************/
//* SETTINGS FOR BORLAND C
#if defined(__BORLANDC__)
#if defined(_WIN32_)
#define _RB_STDCALL  __stdcall
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      WINAPI
#define _RB_PTR      *
#elif (defined(_OS2_))
#define _RB_STDCALL
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL   __pascal
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_PTR     *
#define _RB_API     EXPENTRY
#elif (defined(OS2_INCLUDED))
#define _RB_LOADDS  _loadds
#define _RB_STDCALL
#define _RB_FASTCALL
#define _RB_PASCAL  pascal
#define _RB_CDECL   _cdecl
#define _RB_FAR     far
#define _RB_NEAR    near
#define _RB_HUGE    huge
#define _RB_PTR     _RB_FAR *
#define _RB_API     _RB_FAR _RB_PASCAL _RB_LOADDS
#elif (defined(_WIN_) || defined(_WIN3_))
#define _RB_LOADDS _loadds
#define _RB_STDCALL
#define _RB_FASTCALL
#define _RB_PASCAL  pascal
#define _RB_CDECL   _cdecl
#define _RB_FAR     far
#define _RB_NEAR    near
#define _RB_HUGE    huge
#define _RB_PTR     _RB_FAR *
#define _RB_API     WINAPI _RB_LOADDS
#elif defined(_QNX_)
#define _RB_LOADDS _loadds
#define _RB_STDCALL
#define _RB_FASTCALL
#define _RB_PASCAL  pascal
#define _RB_CDECL   _cdecl
#define _RB_FAR     far
#define _RB_NEAR    near
#define _RB_HUGE    huge
#define _RB_PTR     _RB_FAR *
#define _RB_API     _RB_FAR _RB_CDECL
#elif defined(_DOS_)
#define _RB_STDCALL
#define _RB_FASTCALL
#define _RB_PASCAL  pascal
#define _RB_CDECL   _cdecl
#define _RB_FAR     far
#define _RB_NEAR    near
#define _RB_HUGE    huge
#if ( defined(_DOS4GW_) && defined(_WATC_) )
#define _RB_PTR     *
#endif
#if defined(_BPASF_)
#define _RB_PTR     _RB_FAR *
#define _RB_API     _RB_FAR _RB_PASCAL
#elif defined(_BPASN_)
#define _RB_PTR     _RB_FAR *
#define _RB_API     _RB_NEAR _RB_PASCAL
#else
#if (defined(__SMALL__) || defined(__MEDIUM__) || defined(__LARGE__) || defined(__COMPACT__))
#define _RB_PTR     _RB_FAR *
#elif defined(__HUGE__)
#define _RB_PTR     _RB_HUGE *
#elif defined(__TINY__)
#define _RB_PTR     *
#endif
#define _RB_API     _RB_FAR _RB_CDECL
#endif
#endif
#endif
/****************************************************************************/

/***************************************************************************/

/****************************************************************************
* SETTINGS FOR MICROSOFT C
****************************************************************************/
#if  defined(_MSC_VER)
#if  defined(_WIN32_)
#define _RB_STDCALL  __stdcall
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#ifdef _DRVDRV_
#define _RB_API  _SP_API
#else
#define _RB_API  WINAPI
#endif
#define _RB_PTR      *
#elif defined(_OS2_)
#define _RB_LOADDS   _loadds
#define _SP_API
#define _RB_FASTCALL _fastcall
#define _RB_PASCAL   _pascal
#define _RB_CDECL    _cdecl
#define _RB_FAR      _far
#define _RB_NEAR     _near
#define _RB_HUGE     _huge
#define _RB_API      _RB_FAR _RB_PASCAL
#define _RB_PTR      _RB_FAR *
#elif (defined(_WIN_) || defined(_WIN3_))
#define _RB_LOADDS   _loadds
#define _SP_API
#define _RB_FASTCALL _fastcall
#define _RB_PASCAL   _pascal
#define _RB_CDECL    _cdecl
#define _RB_FAR      _far
#define _RB_NEAR     _near
#define _RB_HUGE     _huge
#define _RB_API      WINAPI _RB_LOADDS
#define _RB_PTR      _RB_FAR *
#elif (_MSC_VER <= 7)
#define _RB_LOADDS   _loadds
#define _SP_API
#define _RB_FASTCALL _fastcall
#define _RB_PASCAL   _pascal
#define _RB_CDECL    _cdecl
#define _RB_FAR      _far
#define _RB_NEAR     _near
#define _RB_HUGE     _huge
#define _RB_API      _RB_FAR _RB_PASCAL
#define _RB_PTR      _RB_FAR *
#else
#define _SP_API  __stdcall
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      _RB_CDECL
#define _RB_PTR      *
#endif
#endif
/***************************************************************************/

/****************************************************************************
* SETTINGS FOR WATCOM C */
#if defined(__WATCOMC__)
#if defined(_WIN32_)
#define _RB_STDCALL  __stdcall
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      WINAPI
#define _RB_PTR      *
#elif defined(_OS2_)
#define _RB_STDCALL
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL   __pascal
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      EXPENTRY
#define _RB_PTR      _RB_FAR *
#elif defined(_NW_)
#define _RB_STDCALL
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL   __pascal
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      _RB_CDECL
#define _RB_PTR      *
#elif defined(_QNX_)
#define _RB_STDCALL
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL   __pascal
#define _RB_CDECL    __cdecl
#if defined(__FLAT__)
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_PTR      *
#define _RB_API      _RB_CDECL
#else
#define _RB_FAR      __far
#define _RB_NEAR     __near
#define _RB_HUGE     __huge
#define _RB_PTR      _RB_FAR *
#define _RB_API      _RB_FAR _RB_CDECL
#endif
#else
#define _RB_STDCALL
#define _RB_FASTCALL __fastcall
#define _RB_PASCAL   __pascal
#define _RB_CDECL    __cdecl
#define _RB_FAR
#define _RB_NEAR
#define _RB_HUGE
#define _RB_API      _RB_CDECL
#define _RB_PTR      *
#endif
#endif
/****************************************************************************/


/***************************************************************************/

#define SP_LOADDS   _RB_LOADDS
#define SP_SYSCALL  _RB_SYSCALL
#define SP_STDCALL  _SP_API
#define SP_FASTCALL _RB_FASTCALL
#define SP_PASCAL   _RB_PASCAL
#define SP_CDECL    _RB_CDECL
#define SP_FAR      _RB_FAR
#define SP_NEAR     _RB_NEAR
#define SP_HUGE     _RB_HUGE
#if ( defined(_DOS4GW_) && defined(__BORLANDC__) && defined(_WATC_) )
#define SP_API      __cdecl
#else
#define SP_API      _RB_API
#endif
#define SP_PTR      _RB_PTR
#define SP_IN
#define SP_OUT
#define SP_IO

#if !defined(_RBTYPES_INC)

typedef                void  RB_VOID;
typedef unsigned       char  RB_BOOLEAN;
typedef unsigned       char  RB_BYTE;
typedef unsigned short int   RB_WORD;
typedef unsigned long  int   RB_DWORD;
typedef long int             RB_LONG;

typedef RB_VOID    SP_PTR RBP_VOID;
typedef RB_BYTE    SP_PTR RBP_BYTE;
typedef RB_BOOLEAN SP_PTR RBP_BOOLEAN;
typedef RB_WORD    SP_PTR RBP_WORD;
typedef RB_DWORD   SP_PTR RBP_DWORD;

#endif


typedef RB_DWORD RB_SPRO_APIPACKET[SPRO_APIPACKET_SIZE/sizeof(RB_DWORD)];
typedef RB_WORD  SP_STATUS;
typedef RBP_VOID RBP_SPRO_APIPACKET;

/* define a unit info structure */
typedef struct _RB_UNIT_INFO {
RB_WORD devID;
RB_WORD serialNum;
RB_WORD port;
RB_BYTE reserved[18];
} RB_UNIT_INFO;
typedef RB_UNIT_INFO SP_PTR RBP_UNIT_INFO;

/* provided for packward compatibility for OS/2 spromeps.h */
#undef RNBO_SPRO_API
#define RNBO_SPRO_APIPACKET     RB_SPRO_APIPACKET
#define RNBO_SPRO_APIPACKET_PTR RBP_SPRO_APIPACKET
#define RNBO_SPRO_API           SP_API
#define RNBO_USHORT_PTR         RBP_WORD
#define RNBO_UCHAR_PTR          RBP_BYTE
#define RNBO_QUERY_PTR          RBP_BYTE
#define RNBO_ULONG_PTR          RBP_DWORD

/* machine types */
#define RB_MIN_MACH_TYPE          0
#define RB_AUTODETECT_MACHINE     0          /* Autodetect machine type    */
#define RB_IBM_MACHINE            1          /* defines IBM type hw        */
#define RB_NEC_MACHINE            2          /* defines NEC PC-9800 hw     */
#define RB_FMR_MACHINE            3          /* defines Fujitsu FMR hw     */
#define RB_MAX_MACH_TYPE          3

/* OS types */
#define RB_MIN_OS_TYPE            0
#define RB_AUTODETECT_OS_TYPE     0          /* Autodetect OS type         */
#define RB_OS_DOS                 1          /* DOS operating system       */
#define RB_OS_RSRV1               2          /* reserved                   */
#define RB_OS_RSRV2               3          /* reserved                   */
#define RB_OS_WIN3x               4          /* Windows 3.x operating env  */
#define RB_OS_WINNT               5          /* Windows NT operating system*/
#define RB_OS_OS2                 6          /* OS/2 operating system      */
#define RB_OS_WIN95               7          /* Windows 95 operating system*/
#define RB_OS_WIN32s              8          /* Windows WIN32s env         */
#define RB_OS_NW                  9          /* Netware operating system   */
#define RB_OS_QNX                 10
#define RB_OS_LINUX	          12         /* Linux operating system     */
#define RB_MAX_OS_TYPE            9

/* Driver types */
#define RB_DOSRM_LOCAL_DRVR       1          /* DOS Real Mode local driver */
#define RB_WIN3x_LOCAL_DRVR       2          /* Windows 3.x local driver   */
#define RB_WIN32s_LOCAL_DRVR      3          /* Win32s local driver        */
#define RB_WIN3x_SYS_DRVR         4          /* Windows 3.x system driver  */
#define RB_WINNT_SYS_DRVR         5          /* Windows NT system driver   */
#define RB_OS2_SYS_DRVR           6          /* OS/2 system driver         */
#define RB_WIN95_SYS_DRVR         7          /* Windows 95 system driver   */
#define RB_NW_LOCAL_DRVR          8          /* Netware local driver       */
#define RB_QNX_LOCAL_DRVR         9          /* QNX local driver           */

/* Router Flags */
#define RB_ROUTER_USE_LOCAL_DRVR  0x0001     /* use linked in driver       */
#define RB_ROUTER_USE_SYS_DRVR    0x0002     /* use system driver          */
#define RB_ROUTER_AUTODETECT_DRVR (RB_ROUTER_USE_LOCAL_DRVR | \
                                   RB_ROUTER_USE_SYS_DRVR)      
#define RB_MAX_ROUTER_FLAGS       (RB_ROUTER_USE_LOCAL_DRVR | \
                                   RB_ROUTER_USE_SYS_DRVR)

/* Port Params flags */
#define RB_FIRST_LOG_PORT         0          /* first logical port         */
#define RB_LAST_LOG_PORT          3          /* last logical port          */
#define RB_VALIDATE_PORT          0x00000001 /* I/O validate port exsitence*/
#define RB_CONT_HNDLR_INSTALLED   0x00000002 /* OUT    system contention   */
#define RB_USER_DEFINED_PORT      0x00000004 /* OUT    user defined port   */
#define RB_FLAGS_DRVR_DEFINED     0x80000000 /* I/O driver defined flags   */
#define RB_RSRV_PORT_FLAGS        0x7FFFFFF8 /* reserved                   */
#define RB_DEFAULT_PORT_FLAGS     (RB_VALIDATE_PORT | RB_FLAGS_DRVR_DEFINED)
#define RB_USE_AUTOTIMING         0

/* Port types */
#define RB_MIN_PORT_TYPE          0
#define RB_AUTODETECT_PORT_TYPE   0          /* IN   autodetect port type  */
#define RB_NEC_PORT_TYPE          1          /* I/O NEC-PC9800 series port */
#define RB_FMR_PORT_TYPE          2          /* I/O Fujitus FMR series port*/
#define RB_PS2_PORT_TYPE          3          /* I/O IBM/AT/PS2 series port */
#define RB_PS2_DMA_PORT_TYPE      4          /* I/O IBM PS2 DMA series port*/
#define RB_MAX_PORT_TYPE          4

/* Contention Methods (bit mask) */
#define RB_CONT_METH_SYS_ALLOC    0x00000001 /* I/O System port allocation */     
#define RB_CONT_METH_NT_RIRQL     0x00000002 /* OUT    NT Raise IRQ level  */    
#define RB_CONT_METH_SYS_INT      0x00000004 /* I/O Disable System Ints    */ 
#define RB_CONT_METH_MASK_INT     0x00000008 /* I/O Mask ints at PIC       */     
#define RB_CONT_METH_WIN_CS       0x00000010 /* I/O Windows Critical Sect  */   
#define RB_CONT_METH_POLL_HW      0x00000020 /* I/O H/W polling of port    */
#define RB_CONT_METH_RBW          0x00000040 /* I/O Read Before Write      */
#define RB_CONT_METH_DRVR_DEFINED 0x80000000 /* Contention defined by drvr.*/

/* Interrupts to mask (bit mask) */
#define RB_IRQ_MASK_LPT1      0x0001         /* mask LPT1  interrupt       */
#define RB_IRQ_MASK_LPT2      0x0002         /* mask LPT2  interrupt       */
#define RB_IRQ_MASK_TIMER     0x0004         /* mask TIMER interrupt       */
#define RB_IRQ_MAX_MASK       (RB_IRQ_MASK_LPT1 | \
                               RB_IRQ_MASK_LPT2 | \
                               RB_IRQ_MASK_TIMER)
#define RB_IRQ_MASK_DEF       (RB_IRQ_MASK_LPT1 | \
                               RB_IRQ_MASK_TIMER)

/* Define default retry counts and intervals */
#define RB_PORT_CONT_RETRY_CNT_DEF 100       /* 100 retries for port cont  */
#define RB_PORT_CONT_RETRY_INT_DEF 300       /* 300 ms retry interval      */
#define RB_DEV_RETRY_CNT_DEF       100       /* 100 retries for device     */

/* Define the cmd field values for RB_SPRO_LIB_PARAMS  */
#define RB_SET_LIB_PARAMS_CMD      0x0001    /* Set library parameters     */
#define RB_GET_LIB_PARAMS_CMD      0x0002    /* Get library parameters     */

/* define the func field values for RB_SPRO_LIB_PARAMS */
#define RB_MACHINE_TYPE_FUNC       0x0001    /* Set/Get Machine type       */
#define RB_DELAY_FUNC              0x0002    /* Set/Get Delay value        */
#define RB_MASK_INTS_FUNC          0x0003    /* Set/Get Mask interrupts    */
#define RB_ROUTER_FLAGS_FUNC       0x0004    /* Set/Get Router flags       */
#define RB_OS_PARAMS_FUNC          0x0005    /* Set/Get O/S parameters     */
#define RB_PORT_PARAMS_FUNC        0x0006    /* Set/Get Port Parameters    */

typedef struct _RB_SP_OS_PARAMS {
SP_IO  RB_WORD osType;                       /* type of Operating System   */
SP_OUT RB_WORD osVer;                        /* version of Operating System*/
} RB_SP_OS_PARAMS;
typedef RB_SP_OS_PARAMS SP_PTR RBP_SP_OS_PARAMS;

typedef struct _RB_SP_PORT_PARAMS {
SP_IO  RB_WORD  logPortNum;                  /* logical port number         */
SP_IO  RB_WORD  sysPortNum;                  /* system  port number         */
SP_IO  RB_WORD  portType;                    /* port type                   */
SP_IO  RB_WORD  phyAddr;                     /* physcial address            */
SP_OUT RB_WORD  mappedAddr;                  /* map address                 */
SP_IO  RB_WORD  deviceRetryCnt;              /* device retry count          */
SP_IO  RB_WORD  contentionRetryCnt;          /* port contention retry count */
SP_IO  RB_WORD  padding1;
SP_IO  RB_DWORD contentionMethod;            /* port contention method      */
SP_IO  RB_DWORD contentionRetryInterval;     /* port contention retry int   */
SP_IO  RB_DWORD flags1;                      /* port flags                  */
} RB_SP_PORT_PARAMS;
typedef RB_SP_PORT_PARAMS SP_PTR RBP_SP_PORT_PARAMS;

typedef union  _RB_SP_CFG_PARAMS {
SP_IO RB_WORD        machineType;      /* machine type: IBM, NEC, or FMR   */
SP_IO RB_WORD        delay;            /* number of loops for 2us delay    */
SP_IO RB_WORD        maskInterrupts;   /* interrupts to mask               */
SP_IO RB_WORD        routerFlags;      /* request routing flags            */
SP_IO RB_SP_OS_PARAMS   osParams;      /* OS parameters                    */
SP_IO RB_SP_PORT_PARAMS portParams;    /* port parameters                  */
} RB_SP_CFG_PARAMS;
typedef RB_SP_CFG_PARAMS SP_PTR RBP_SP_CFG_PARAMS;

typedef struct _RB_SPRO_LIB_PARAMS {
SP_IN RB_WORD   cmd;                   /* command - set/get parameters     */
SP_IN RB_WORD   func;                  /* function to set/get              */
SP_IO RB_SP_CFG_PARAMS params;
} RB_SPRO_LIB_PARAMS;
typedef RB_SPRO_LIB_PARAMS SP_PTR RBP_SPRO_LIB_PARAMS;

/*
 * License update interval during which a client should contact the 
 * server to let it know "I am alive".
  */
#define LIC_UPDATE_INT  120 /* 2*60 = 2 min.*/
/* Making the LIcense Update Time programmable*/
#define MAX_HEARTBEAT	2592000	/* 30*24*60*60 seconds */
#define MIN_HEARTBEAT	60	/* 60 seconds */
#define INFINITE_HEARTBEAT	0xFFFFFFFF


/* Maximum number of devices */
#define MAX_NUM_DEV   10

/* Maximum host name length */
#define MAX_NAME_LEN  64

/* Maximum host address length */
#define MAX_ADDR_LEN  32

/* Enum server falgs */
#define NSPRO_RET_ON_FIRST            1
#define NSPRO_GET_ALL_SERVERS         2
#define NSPRO_RET_ON_FIRST_AVAILABLE  4

/* Set Protocol falgs */
typedef RB_WORD PROTOCOL_FLAG;
#define NSPRO_TCP_PROTOCOL         1
#define NSPRO_IPX_PROTOCOL         2
#define NSPRO_NETBEUI_PROTOCOL     4
#define NSPRO_SAP_PROTOCOL         8

/* Monitoring information */
typedef struct tag_nsproKeyMonitorInfo {
   RB_WORD      devId;      /* developer id of the key */
   RB_WORD      hardLimit;  /* hardlimit of the key */
   RB_WORD      inUse;      /* Number of licenses in use for the key */
   RB_WORD      numTimeOut; /* Number of timeouts recorded for the key */
   RB_WORD      highestUse; /* Highest number of licenses issued from 
                               the key throughout the life of server
                             */
} NSPRO_KEY_MONITOR_INFO;

/* SproMedic Information */
typedef struct tag_nsproMedicInfo {
   char                     serverName         [MAX_NAME_LEN];
   char                     serverIPAddress    [MAX_ADDR_LEN];  /* IP Address*/
   char                     serverIPXAddress   [MAX_ADDR_LEN];  /* IPX Address*/
   char                     version            [MAX_NAME_LEN];  /* Server Version*/    
   RB_WORD                  protocol;                           /* Server Protocol*/
   NSPRO_KEY_MONITOR_INFO   nsproKeyInfo       [MAX_NUM_DEV] ;  
} NSPRO_MEDIC_INFO;


typedef struct tag_nsproMonitorInfo {
   char          serverName[MAX_NAME_LEN];
   char          serverIPAddress[MAX_ADDR_LEN]; /* Server's IP address */
   char          serverIPXAddress[MAX_ADDR_LEN]; /* Server's IPX address */
   char          version[MAX_NAME_LEN];     /* version of the server */
//   RB_WORD    numKeys; 
                                         /* Number of keys attached to the server */
   RB_WORD    protocol;    /* Protocols supported by the server */
   NSPRO_KEY_MONITOR_INFO  sproKeyMonitorInfo;     
} NSPRO_MONITOR_INFO; 


/* Server information returned by RNBOsproEnumServer */
typedef struct {
   char       serverAddress[MAX_ADDR_LEN];
   RB_WORD    numLicAvail;
}  NSPRO_SERVER_INFO;


/*Communication Modes */
#define RNBO_STANDALONE         __TEXT("RNBO_STANDALONE")
#define RNBO_SPN_DRIVER         __TEXT("RNBO_SPN_DRIVER")
#define RNBO_SPN_LOCAL          __TEXT("RNBO_SPN_LOCAL")
#define RNBO_SPN_BROADCAST      __TEXT("RNBO_SPN_BROADCAST")
#define RNBO_SPN_ALL_MODES      __TEXT("RNBO_SPN_ALL_MODES")
#define RNBO_SPN_SERVER_MODES   __TEXT("RNBO_SPN_SERVER_MODES")



#ifndef _GLUE_DLL
/*Define the extern routines */
SP_EXPORT
SP_STATUS SP_API RNBOsproCfgLibParams( SP_IO RBP_SPRO_APIPACKET  thePacket,
                                       SP_IO RBP_SPRO_LIB_PARAMS params );
SP_EXPORT
SP_STATUS SP_API RNBOsproFormatPacket( SP_OUT RBP_SPRO_APIPACKET thePacket,
                                       SP_IN  RB_WORD            thePacketSize );
SP_EXPORT
SP_STATUS SP_API RNBOsproInitialize( SP_OUT RBP_SPRO_APIPACKET packet );

SP_EXPORT
SP_STATUS SP_API RNBOsproFindFirstUnit( SP_IN RBP_SPRO_APIPACKET packet,
                                        SP_IN RB_WORD            devleoperID );
SP_EXPORT
SP_STATUS SP_API RNBOsproFindNextUnit( SP_IN RBP_SPRO_APIPACKET packet );

SP_EXPORT
SP_STATUS SP_API RNBOsproRead( SP_IN  RBP_SPRO_APIPACKET packet,
                               SP_IN  RB_WORD            address,
                               SP_OUT RBP_WORD           data );
SP_EXPORT
SP_STATUS SP_API RNBOsproExtendedRead( SP_IN  RBP_SPRO_APIPACKET packet,
                                       SP_IN  RB_WORD            address,
                                       SP_OUT RBP_WORD           data,
                                       SP_OUT RBP_BYTE           accessCode );
SP_EXPORT
SP_STATUS SP_API RNBOsproWrite( SP_IN RBP_SPRO_APIPACKET packet,
                                SP_IN RB_WORD            writePassword,
                                SP_IN RB_WORD            address,
                                SP_IN RB_WORD            data,
                                SP_IN RB_BYTE            accessCode );
SP_EXPORT
SP_STATUS SP_API RNBOsproOverwrite( SP_IN RBP_SPRO_APIPACKET packet,
                                    SP_IN RB_WORD            writePassword,
                                    SP_IN RB_WORD            overwritePassword1,
                                    SP_IN RB_WORD            overwritePassword2,
                                    SP_IN RB_WORD            address,
                                    SP_IN RB_WORD            data,
                                    SP_IN RB_BYTE            accessCode );
SP_EXPORT
SP_STATUS SP_API RNBOsproDecrement( SP_IN RBP_SPRO_APIPACKET packet,
                                    SP_IN RB_WORD            writePassword,
                                    SP_IN RB_WORD            address );
SP_EXPORT
SP_STATUS SP_API RNBOsproActivate( SP_IN RBP_SPRO_APIPACKET packet,
                                   SP_IN RB_WORD            writePassword,
                                   SP_IN RB_WORD            activatePassword1,
                                   SP_IN RB_WORD            activatePassword2,
                                   SP_IN RB_WORD            address );
SP_EXPORT
SP_STATUS SP_API RNBOsproQuery( SP_IN  RBP_SPRO_APIPACKET packet,
                                SP_IN  RB_WORD            address,
                                SP_IN  RBP_VOID           queryData,
                                SP_OUT RBP_VOID           response,
                                SP_OUT RBP_DWORD          response32,
                                SP_IN  RB_WORD            length );
SP_EXPORT
RB_WORD SP_API RNBOsproGetFullStatus( SP_IN RBP_SPRO_APIPACKET thePacket );

SP_EXPORT
SP_STATUS SP_API RNBOsproGetVersion( SP_IN  RBP_SPRO_APIPACKET thePacket,
                                     SP_OUT RBP_BYTE           majVer,
                                     SP_OUT RBP_BYTE           minVer,
                                     SP_OUT RBP_BYTE           rev,
                                     SP_OUT RBP_BYTE           osDrvrType );


SP_EXPORT
SP_STATUS SP_API RNBOsproGetUnitInfo( SP_IO  RBP_SPRO_APIPACKET thePacket,
                                      SP_OUT RBP_UNIT_INFO      unitInfo );
SP_EXPORT
SP_STATUS SP_API RNBOsproSetUnitInfo( SP_IO RBP_SPRO_APIPACKET thePacket,
                                      SP_IN RBP_UNIT_INFO      unitInfo );

/* SRW - Added to support the networking capabilities of SuperPro 6.1 */
SP_EXPORT
SP_STATUS SP_API RNBOsproGetHardLimit( SP_IN  RBP_SPRO_APIPACKET thePacket,
                                       SP_OUT RBP_WORD HardLimit );
                                      
SP_EXPORT SP_STATUS SP_API  RNBOsproGetSubLicense (
	RBP_SPRO_APIPACKET	packet,     /* IN */
	RB_WORD			        address     /* IN - cellno of sublicense */
);

SP_EXPORT SP_STATUS SP_API  RNBOsproReleaseLicense (
	RBP_SPRO_APIPACKET	  packet,     /* IN */
	RB_WORD	              address,    /* IN - sublicense cell no */
	RBP_WORD	            numSubLic     /* IN OUT - number of sublicenses to release 
                                                  and no of sublic actually released*/
);


SP_EXPORT SP_STATUS SP_API RNBOsproSetContactServer (
	RBP_SPRO_APIPACKET	  packet,       /* IN */
	char				  *serverName   /* IN */
);

SP_EXPORT SP_STATUS SP_API RNBOsproGetContactServer (
	RBP_SPRO_APIPACKET  packet,         /* IN */
	char               *serverNameBuf,  /* OUT */ 
	RB_WORD             serverNameBufSz /* IN */
);

SP_EXPORT SP_STATUS SP_API  RNBOsproEnumServer (
		ENUM_SERVER_FLAG    enumFlag,       /* IN */
		RB_WORD             developerId,    /* IN */
		NSPRO_SERVER_INFO  *serverInfo,     /* OUT */
		RBP_WORD            numServerInfo   /* IN OUT */
	);


SP_EXPORT SP_STATUS SP_API  RNBOsproGetKeyInfo (
	RBP_SPRO_APIPACKET	packet,			        /* IN */
 	RB_WORD			    devId,					/* IN */
    RB_WORD             keyIndex,				/* IN */
 	NSPRO_MONITOR_INFO  *nsproMonitorInfo	    /* OUT*/
	);

SP_EXPORT SP_STATUS SP_API  RNBOsproSetProtocol (
	RBP_SPRO_APIPACKET	packet,			        /* IN */
    PROTOCOL_FLAG       protocol		        /* IN */
	);

SP_EXPORT SP_STATUS SP_API RNBOsproSetHeartBeat(
  RBP_SPRO_APIPACKET packet,          /* IN */
  RB_DWORD           heartBeatValue   /* IN */ 
);
                                  
#endif
#endif /* _SPROMEPS_H */
/* end of file */
