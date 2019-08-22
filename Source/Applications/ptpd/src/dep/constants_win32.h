#ifndef CONSTANTS_WIN32_H
#define CONSTANTS_WIN32_H

#include "../ptpd.h"

#define SOCKETS_WSA2


# if defined(PCAP)
#  define TIMESTAMPING_PCAP
# else
#  define TIMESTAMPING_GET_TIME
# endif

#include <limits.h>

/* minimal windows version */
#define _WIN32_WINNT_WS03   0x0502
#define WINVER    _WIN32_WINNT_WS03

#define NOGDI

#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <io.h>

#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#if !defined(PATH_MAX)
#define PATH_MAX MAX_PATH
#endif

/* Microsoft Visual C */
#if defined(_MSC_VER)
 #ifdef  _WIN64
  typedef __int64    ssize_t;
 #else
  typedef _W64 int   ssize_t;
 #endif

 // not ideal define
 //#define snprintf sprintf_s
 //#define strncpy(d, s, n) strcpy_s(d, n, s)

 #define STDOUT_FILENO 1
 #define STDERR_FILENO 2
 #include "freegetopt/getopt.h"

#else
 #include <getopt.h>
#endif

// have to be implemented
#define openlog(a,b,c)
#define fdatasync(a)
#define vsyslog(a,b,c) vfprintf(stdout, b, c)
#define setlinebuf(a)

#define gethostbyname2(a, b) gethostbyname(a)


#if defined(TIMESTAMPING_PCAP)
#define WPCAP
#define HAVE_U_INT8_T
#define HAVE_U_INT16_T
#define HAVE_U_INT32_T
#define HAVE_U_INT64_T
#include <pcap.h>
#endif

#define CLOCK_INTERRUPTS_PER_SECOND 64


/* UNIX compatibility */
#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7

#define LOG_USER    0

/* findIface*/
#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#undef TRUE
#undef FALSE

#define IFACE_NAME_LENGTH         64
#define NET_ADDRESS_LENGTH        16

#define IFCONF_LENGTH             10

#define MAXHOSTNAMELEN            256

# if BYTE_ORDER == LITTLE_ENDIAN
#   define PTPD_LSBF
# elif BYTE_ORDER == BIG_ENDIAN
#   define PTPD_MSBF
# endif

extern HANDLE TimerSelectInterrupt;

#endif
