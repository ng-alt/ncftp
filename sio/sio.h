/* sio.h */

#ifndef _sio_h_
#define _sio_h_ 1

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

typedef struct SelectSet {
	fd_set fds;
	struct timeval timeout;
	int maxfd;
	int numfds;
} SelectSet, *SelectSetPtr;

/* For SReadline */
#ifndef _SReadlineInfo_
#define _SReadlineInfo_ 1
typedef struct SReadlineInfo {
	char *buf;		/* Pointer to beginning of buffer. */
	char *bufPtr;		/* Pointer to current position in buffer. */
	char *bufLim;		/* Pointer to end of buffer. */
	size_t bufSize;		/* Current size of buffer block. */
	size_t bufSizeMax;	/* Maximum size available for buffer. */
	int malloc;		/* If non-zero, malloc() was used for buf. */
	int fd;			/* File descriptor to use for I/O. */
	int timeoutLen;		/* Timeout to use, in seconds. */
	int requireEOLN;	/* When buffer is full, continue reading and discarding until \n? */
} SReadlineInfo;
#endif

#ifndef forever
#	define forever for ( ; ; )
#endif

/* Private decl; only for use when compiling sio code. */
#ifdef HAVE_SIGSETJMP
#	define SSetjmp(a) sigsetjmp(a, 1)
#	define SLongjmp(a,b) siglongjmp(a, b)
#	define Sjmp_buf sigjmp_buf
#else
#	define SSetjmp(a) setjmp(a)
#	define SLongjmp(a,b) longjmp(a, b)
#	define Sjmp_buf jmp_buf
#endif

/* Parameter to SBind */
#define kReUseAddrYes 1
#define kReUseAddrNo 0

/* Parameter to SRead/SWrite */
#define kFullBufferNotRequired				00000
#define kFullBufferRequired				00001
#define kFullBufferRequiredExceptLast			00002
#define	kNoFirstSelect					00010

/* Parameter to AddrToAddrStr */
#define kUseDNSYes 1
#define kUseDNSNo 0

#define kTimeoutErr (-2)
#define kBrokenPipeErr (-3)

#define kAddrStrToAddrMiscErr (-4)
#define kAddrStrToAddrBadHost (-5)

#define kSNewFailed (-6)
#define kSBindFailed (-7)
#define kSListenFailed (-8)

#define kSrlBufSize 2048

#define kNoTimeLimit 0

/* Return value from GetOurHostName */
#define kGethostnameFailed (-1)
#define kDomainnameUnknown (-2)
#define kFullyQualifiedHostNameTooLongForBuffer (-3)
#define kHostnameHardCoded 1
#define kGethostnameFullyQualified 2
#define kGethostbynameFullyQualified 3
#define kGethostbynameHostAliasFullyQualified 4
#define kGethostbyaddrFullyQualified 5
#define kGethostbyaddrHostAliasFullyQualified 6
#define kDomainnameHardCoded 7
#define kResInitDomainnameFound 8
#define kEtcResolvConfDomainFound 9
#define kEtcResolvConfSearchFound 10


#if 1 /* %config2% -- set by configure script -- do not modify */
#	ifndef NO_SIGNALS
#		define NO_SIGNALS 1
#	endif
#else
	/* #undef NO_SIGNALS */
#endif

/* Don't change the following line -- it is modified by the Configure script. */
#define SAccept SAcceptS

#ifndef SAccept
#	if defined(NO_SIGNALS) || defined(WIN32) || defined(_WINDOWS)
#		define SAccept SAcceptS
#	else
#		define SAccept SAcceptA
#	endif
#endif

#if !defined(ETIMEDOUT) && defined(WSAETIMEDOUT)
#	define ETIMEDOUT WSAETIMEDOUT
#endif

#if !defined(EADDRNOTAVAIL) && defined(WSAEADDRNOTAVAIL)
#	define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#endif

#if !defined(EWOULDBLOCK) && defined(WSAEWOULDBLOCK)
#	define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#if !defined(EINPROGRESS) && defined(WSAEINPROGRESS)
#	define EINPROGRESS WSAEINPROGRESS
#endif

#if !defined(WIN32) && !defined(_WINDOWS) && !defined(closesocket)
#	define closesocket close
#endif

#if !defined(WIN32) && !defined(_WINDOWS) && !defined(ioctlsocket)
#	define ioctlsocket ioctl
#endif

#if defined(WIN32) || defined(_WINDOWS)
#	define SETERRNO errno = WSAGetLastError();
#	define SETWSATIMEOUTERR WSASetLastError(WSAETIMEDOUT);
#else
#	define SETERRNO
#	define SETWSATIMEOUTERR
#endif

/*
 * Definitions for IP type of service (ip_tos)
 */
#ifndef IPTOS_LOWDELAY
#	define IPTOS_LOWDELAY		0x10
#	define IPTOS_THROUGHPUT		0x08
#	define IPTOS_RELIABILITY	0x04
#endif
#if !defined(IPTOS_LOWCOST) && !defined(IPTOS_MINCOST)
#	define IPTOS_LOWCOST		0x02
#	define IPTOS_MINCOST		IPTOS_LOWCOST
#elif !defined(IPTOS_LOWCOST) && defined(IPTOS_MINCOST)
#	define IPTOS_LOWCOST		IPTOS_MINCOST
#elif defined(IPTOS_LOWCOST) && !defined(IPTOS_MINCOST)
#	define IPTOS_MINCOST		IPTOS_LOWCOST
#endif

#ifndef INADDR_NONE
#	define INADDR_NONE		(0xffffffff)	/* <netinet/in.h> should have it. */
#endif
#ifndef INADDR_ANY
#	define INADDR_ANY		(0x00000000)
#endif


typedef void (*sio_sigproc_t)(int);
typedef volatile sio_sigproc_t vsio_sigproc_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int gLibSio_Uses_Me_To_Quiet_Variable_Unused_Warnings;

#if (defined(__APPLE_CC__)) && (__APPLE_CC__ < 10000)
#	define LIBSIO_USE_VAR(a) gLibSio_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#elif (defined(__GNUC__)) && (__GNUC__ >= 3)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((__unused__))
#	endif
#	define LIBSIO_USE_VAR(a)
#elif (defined(__GNUC__)) && (__GNUC__ == 2)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((unused))
#	endif
#	define LIBSIO_USE_VAR(a)
#else
#	define LIBSIO_USE_VAR(a) gLibSio_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#endif

/* DNSUtil.c */
int GetHostByName(struct hostent *const hp, const char *const name, char *const hpbuf, size_t hpbufsize);
int GetHostByAddr(struct hostent *const hp, void *addr, int asize, int atype, char *const hpbuf, size_t hpbufsize);
int GetHostEntry(struct hostent *const hp, const char *const host, struct in_addr *const ip_address, char *const hpbuf, size_t hpbufsize);
int GetOurHostName(char *const host, const size_t siz);

/* PRead.c */
int PRead(int, char *const, size_t, int);

/* PWrite.c */
int PWrite(int, const char *const, size_t);

/* SAcceptA.c */
int SAcceptA(int, struct sockaddr_in *const, int);

/* SAcceptS.c */
int SAcceptS(int, struct sockaddr_in *const, int);

/* SBind.c */
int SBind(int, const int, const int, const int);
int SListen(int, int);

/* SClose.c */
int SClose(int, int);

/* SConnect.c */
int SConnect(int, const struct sockaddr_in *const, int);

/* SConnectByName.c */
int SConnectByName(int, const char *const, const int);

/* SNew.c */
int SNewStreamClient(void);
int SNewDatagramClient(void);
int SNewStreamServer(const int, const int, const int, int);
int SNewDatagramServer(const int, const int, const int);

/* SRead.c */
int SRead(int, char *const, size_t, int, int);

/* SReadline.c */
void FlushSReadlineInfo(SReadlineInfo *);
int InitSReadlineInfo(SReadlineInfo *, int, char *, size_t, int, int);
void DisposeSReadlineInfo(SReadlineInfo *);
int SReadline(SReadlineInfo *, char *const, size_t);

/* SRecv.c */
int SRecv(int, char *const, size_t, int, int, int);

/* SRecvfrom.c */
int SRecvfrom(int, char *const, size_t, int, struct sockaddr_in *const, int);

/* SRecvmsg.c */
int SRecvmsg(int, void *const, int, int);

/* SSelect.c */
void SelectSetInit(SelectSetPtr const, const double);
void SelectSetAdd(SelectSetPtr const, const int);
void SelectSetRemove(SelectSetPtr const, const int);
int SelectW(SelectSetPtr, SelectSetPtr);
int SelectR(SelectSetPtr, SelectSetPtr);

/* SSend.c */
int SSend(int, char *, size_t, int, int);

/* SSendto.c */
int SSendto(int, const char *const, size_t, int, const struct sockaddr_in *const, int);
int Sendto(int, const char *const, size_t, const struct sockaddr_in *const);

/* SSendtoByName.c */
int SSendtoByName(int, const char *const, size_t, int, const char *const, int);
int SendtoByName(int, const char *const, size_t, const char *const);

/* SWait.c */
int SWaitUntilReadyForReading(const int sfd, const int tlen);
int SWaitUntilReadyForWriting(const int sfd, const int tlen);

/* SWrite.c */
int SWrite(int, const char *const, size_t, int, int);

/* SocketUtil.c */
int GetSocketBufSize(const int, size_t *const, size_t *const);
int SetSocketBufSize(const int, const size_t, const size_t);
int GetSocketNagleAlgorithm(const int);
int SetSocketNagleAlgorithm(const int, const int);
int GetSocketLinger(const int, int *const);
int SetSocketLinger(const int, const int, const int);
int GetSocketKeepAlive(const int fd);
int SetSocketKeepAlive(const int fd, const int onoff);
int GetSocketTypeOfService(const int fd);
int SetSocketTypeOfService(const int fd, const int tosType);
int GetSocketInlineOutOfBandData(const int fd);
int SetSocketInlineOutOfBandData(const int fd, const int onoff);

/* StrAddr.c */
unsigned int ServiceNameToPortNumber(const char *const s, const int proto);
int ServicePortNumberToName(unsigned short port, char *const dst, const size_t dsize, const int proto);
void InetNtoA(char *dst, struct in_addr *ia, size_t siz);
int AddrStrToAddr(const char *const, struct sockaddr_in *const, const int);
char *AddrToAddrStr(char *const dst, size_t dsize, struct sockaddr_in * const saddrp, int dns, const char *fmt);

/* SError.c */
const char *SError(int e);

/* main.c */
void SIOHandler(int);
void (*SSignal(int signum, void (*handler)(int)))(int);

#ifdef __cplusplus
}
#endif

#endif	/* _sio_h_ */
