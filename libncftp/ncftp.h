/* ncftp.h
 *
 * Copyright (c) 1996-2001 Mike Gleason, NCEMRSoft.
 * All rights reserved.
 *
 */

#ifndef _ncftp_h_
#define _ncftp_h_ 1

#ifdef __cplusplus
extern "C"
{
#endif	/* __cplusplus */

#define kLibraryVersion "@(#) LibNcFTP 3.1.1 (December 23, 2001)"

#if defined(WIN32) || defined(_WINDOWS)
#	pragma once
#	pragma warning(disable : 4127)	// warning C4127: conditional expression is constant
#	pragma warning(disable : 4100)	// warning C4100: 'lpReserved' : unreferenced formal parameter
#	pragma warning(disable : 4514)	// warning C4514: unreferenced inline function has been removed
#	pragma warning(disable : 4115)	// warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#	pragma warning(disable : 4201)	// warning C4201: nonstandard extension used : nameless struct/union
#	pragma warning(disable : 4214)	// warning C4214: nonstandard extension used : bit field types other than int
#	pragma warning(disable : 4115)	// warning C4115: 'IRpcStubBuffer' : named type definition in parentheses
#	pragma warning(disable : 4711)	// warning C4711: function selected for automatic inline expansion
#	ifndef WINVER
#		define WINVER 0x0400
#	endif
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0400
#	endif
#	ifndef _INC_WINDOWS
#		include <windows.h>	/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	endif
#	ifndef _INC_TYPES
#		include <sys/types.h>
#	endif
#	ifndef _INC_STAT
#		include <sys/stat.h>
#	endif
#	ifndef _INC_IO
#		include <io.h>
#	endif
#	ifndef _INC_CONIO
#		include <conio.h>
#	endif
#	ifndef _INC_DIRECT
#		include <direct.h>
#	endif
#	ifndef _INC_STDIO
#		include <stdio.h>
#	endif
#	ifndef _INC_STRING
#		include <string.h>
#	endif
#	ifndef _INC_STDLIB
#		include <stdlib.h>
#	endif
#	ifndef _INC_STDDEF
#		include <stddef.h>
#	endif
#	ifndef _INC_STDARG
#		include <stdarg.h>
#	endif
#	ifndef _INC_CTYPE
#		include <ctype.h>
#	endif
#	ifndef _INC_TIME
#		include <time.h>
#	endif
#	ifndef _INC_ERRNO
#		include <errno.h>
#	endif
#	ifndef _INC_SIGNAL
#		include <signal.h>
#	endif
#	ifndef _INC_SETJMP
#		include <setjmp.h>
#	endif
#	define longest_int LONGLONG
#	define longest_int LONGLONG
#	define longest_uint ULONGLONG
#	ifndef HAVE_LONG_LONG
#		define HAVE_LONG_LONG 1
#	endif
#	ifndef SCANF_LONG_LONG
#		define SCANF_LONG_LONG "%I64d"
#	endif
#	ifndef PRINTF_LONG_LONG
#		define PRINTF_LONG_LONG "%I64d"
#	endif
#	ifndef PRINTF_LONG_LONG_I64D
#		define PRINTF_LONG_LONG_I64D 1
#	endif
#	ifndef SCANF_LONG_LONG_I64D
#		define SCANF_LONG_LONG_I64D 1
#	endif
#	ifndef USE_SIO
#		define USE_SIO 1
#	endif
#	ifndef NO_SIGNALS
#		define NO_SIGNALS 1
#	endif
#	ifndef main_void_return_t
#		define main_void_return_t void
#	endif
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	ifndef mode_t
#		define mode_t int
#	endif
#	ifndef S_ISREG
#		define S_ISREG(m)      (((m) & _S_IFMT) == _S_IFREG)
#		define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
#		define S_ISLNK(m)      (0)
#	endif
#	ifndef open
#		define open _open
#		define write _write
#		define read _read
#		define close _close
#		define lseek _lseek
#		define stat _stat
#		define lstat _stat
#		define fstat _fstat
#		define dup _dup
#		define utime _utime
#		define utimbuf _utimbuf
#	endif
#	ifndef unlink
#		define unlink remove
#	endif
#	ifndef HAVE_LONG_FILE_NAMES
#		define HAVE_LONG_FILE_NAMES 1
#	endif
#	ifndef HAVE_MKTIME
#		define HAVE_MKTIME 1
#	endif
#	ifndef HAVE_MEMMOVE
#		define HAVE_MEMMOVE 1
#	endif
#	ifndef HAVE_STRSTR
#		define HAVE_STRSTR 1
#	endif
#	ifndef HAVE_VSNPRINTF
#		define HAVE_VSNPRINTF 1
#	endif
#	ifndef HAVE_SNPRINTF
#		define HAVE_SNPRINTF 1
#	endif
#	ifndef vsnprintf
#		define vsnprintf _vsnprintf
#	endif
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#else
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/time.h>
#	include <sys/wait.h>
#	include <stdio.h>
#	include <string.h>
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
#	include <stdlib.h>
#	include <stddef.h>
#	include <stdarg.h>
#	include <ctype.h>
#	include <time.h>
#	include <errno.h>
#	include <signal.h>
#	include <setjmp.h>
#	if !defined(__ultrix) || !defined(XTI)
#		include <sys/socket.h>
#	endif
#	include <netinet/in.h>
#	include <fcntl.h>
#	include <netdb.h>
#	ifndef USE_SIO
#		define USE_SIO 1
#	endif
#	ifndef NO_SIGNALS
#		define NO_SIGNALS 1
#	endif
#endif

#include "ncftp_errno.h"

/* This is used to verify validty of the data passed in.
 * It also specifies the minimum version that is binary-compatibile with
 * this version.  (So this may not necessarily be kLibraryVersion.)
 */
#define kLibraryMagic "LibNcFTP 3.1.0"

#ifndef longest_int
#define longest_int long long
#define longest_uint unsigned long long
#endif

#ifndef forever
#	define forever for ( ; ; )
#endif

typedef void (*FTPSigProc)(int);

typedef struct Line *LinePtr;
typedef struct Line {
	LinePtr prev, next;
	char *line;
} Line;

typedef struct LineList {
	LinePtr first, last;
	int nLines;
} LineList, *LineListPtr;

typedef struct Response {
	LineList msg;
	int codeType;
	int code;
	int printMode;
	int eofOkay;
	int hadEof;
} Response, *ResponsePtr;

#if USE_SIO && !defined(_SReadlineInfo_)
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

typedef struct FTPLibraryInfo {
	char magic[16];				/* Don't modify this field. */
	int init;				/* Don't modify this field. */
	int socksInit;				/* Don't modify this field. */
	unsigned int defaultPort;		/* Don't modify this field. */
	char defaultAnonPassword[80];		/* You may set this after init. */
} FTPLibraryInfo, *FTPLIPtr;

typedef struct FTPConnectionInfo *FTPCIPtr;
typedef void (*FTPProgressMeterProc)(const FTPCIPtr, int);
typedef void (*FTPLogProc)(const FTPCIPtr, char *);
typedef void (*FTPConnectMessageProc)(const FTPCIPtr, ResponsePtr);
typedef void (*FTPLoginMessageProc)(const FTPCIPtr, ResponsePtr);
typedef void (*FTPRedialStatusProc)(const FTPCIPtr, int, int);
typedef void (*FTPPrintResponseProc)(const FTPCIPtr, ResponsePtr);
typedef void (*FTPGetPassphraseProc)(const FTPCIPtr, LineListPtr pwPrompt, char *pass, size_t dsize);

typedef struct FTPConnectionInfo {
	char magic[16];				/* Don't modify this field. */
	
	char host[64];				/* REQUIRED input parameter. */
	char user[64];				/* OPTIONAL input parameter. */
	char pass[64];				/* OPTIONAL input parameter. */
	char acct[64];				/* OPTIONAL input parameter. */
	unsigned int port;			/* OPTIONAL input parameter. */

	int errNo;				/* You may modify this if you want. */
	char lastFTPCmdResultStr[128];		/* You may modify this if you want. */
	LineList lastFTPCmdResultLL;		/* Use, but do not modify. */
	int lastFTPCmdResultNum;		/* You may modify this if you want. */

	FILE *debugLog;				/* OPTIONAL input parameter. */
	FTPLogProc debugLogProc;		/* OPTIONAL input parameter. */

	unsigned int xferTimeout;		/* OPTIONAL input parameter. */
	unsigned int connTimeout;		/* OPTIONAL input parameter. */
	unsigned int ctrlTimeout;		/* OPTIONAL input parameter. */
	unsigned int abortTimeout;		/* OPTIONAL input parameter. */

	int maxDials;				/* OPTIONAL input parameter. */
	int redialDelay;			/* OPTIONAL input parameter. */

	int dataPortMode;			/* OPTIONAL input parameter. */

	int firewallType;			/* OPTIONAL input parameter. */
	char firewallHost[64];			/* OPTIONAL input parameter. */
	char firewallUser[64];			/* OPTIONAL input parameter. */
	char firewallPass[64];			/* OPTIONAL input parameter. */
	unsigned int firewallPort;		/* OPTIONAL input parameter. */

	size_t ctrlSocketRBufSize;		/* OPTIONAL input parameter. */
	size_t ctrlSocketSBufSize;		/* OPTIONAL input parameter. */
	size_t dataSocketRBufSize;		/* OPTIONAL input parameter. */
	size_t dataSocketSBufSize;		/* OPTIONAL input parameter. */

	const char *asciiFilenameExtensions;	/* OPTIONAL input parameter. */

	unsigned short ephemLo;			/* OPTIONAL input parameter. */
	unsigned short ephemHi;			/* OPTIONAL input parameter. */

	FTPConnectMessageProc onConnectMsgProc; /* OPTIONAL input parameter. */
	FTPRedialStatusProc redialStatusProc;	/* OPTIONAL input parameter. */
	FTPPrintResponseProc printResponseProc; /* OPTIONAL input parameter. */
	FTPLoginMessageProc onLoginMsgProc;	/* OPTIONAL input parameter. */

	FTPGetPassphraseProc passphraseProc;	/* OPTIONAL input parameter. */

	/* Used during transfers; valid during progress meter updates. */
	FTPProgressMeterProc progress;		/* OPTIONAL input parameter. */
	longest_int bytesTransferred;		/* Do not modify this field. */
	int useProgressMeter;			/* Used internally. */
	struct timeval t0;			/* Do not modify this field. */
	double sec;				/* Do not modify this field. */
	double secLeft;				/* Do not modify this field. */
	double kBytesPerSec;			/* Do not modify this field. */
	double percentCompleted;		/* Do not modify this field. */
	longest_int expectedSize;		/* Do not modify this field. */
	time_t mdtm;				/* Do not modify this field. */
	time_t nextProgressUpdate;		/* Do not modify this field. */
	const char *rname;			/* Do not modify this field. */
	const char *lname;			/* Do not modify this field. */
	int stalled;				/* Do not modify this field. */
	int dataTimedOut;			/* Do not modify this field. */
	int cancelXfer;				/* You may modify this. */

	char actualHost[64];			/* Do not modify this field. */
	char ip[32];				/* Do not modify this field. */

	char *startingWorkingDirectory;		/* Use, but do not modify. */

	int connected;				/* Do not modify this field. */
	int loggedIn;				/* Do not modify this field. */
	int curTransferType;			/* Do not modify this field. */
	longest_int startPoint;			/* Do not modify this field. */
	FILE *errLog;				/* DEPRECATED input parameter. */
	FTPLogProc errLogProc;			/* DEPRECATED input parameter. */
	FTPLIPtr lip;				/* Do not modify this field. */

	int hasPASV;				/* Do not modify this field. */
	int hasSIZE;				/* Do not modify this field. */
	int hasMDTM;				/* Do not modify this field. */
	int hasREST;				/* Do not modify this field. */
	int hasNLST_d;				/* Do not modify this field. */
	int hasUTIME;				/* Do not modify this field. */
	int hasFEAT;				/* Do not modify this field. */
	int hasMLSD;				/* Do not modify this field. */
	int hasMLST;				/* Do not modify this field. */
	int usedMLS;				/* Do not modify this field. */
	int hasCLNT;				/* Do not modify this field. */
	int hasRETRBUFSIZE;			/* Do not modify this field. */
	int hasRBUFSIZ;				/* Do not modify this field. */
	int hasRBUFSZ;				/* Do not modify this field. */
	int hasSTORBUFSIZE;			/* Do not modify this field. */
	int hasSBUFSIZ;				/* Do not modify this field. */
	int hasSBUFSZ;				/* Do not modify this field. */
	int hasBUFSIZE;				/* Do not modify this field. */
	int mlsFeatures;			/* Do not modify this field. */
	int STATfileParamWorks;			/* Do not modify this field. */
	int NLSTfileParamWorks;			/* Do not modify this field. */

	struct sockaddr_in servCtlAddr;		/* Do not modify this field. */
	struct sockaddr_in servDataAddr;	/* Do not modify this field. */
	struct sockaddr_in ourCtlAddr;		/* Do not modify this field. */
	struct sockaddr_in ourDataAddr;		/* Do not modify this field. */
	int netMode;				/* Do not use or modify. */
	char *buf;				/* Do not modify this field. */
	size_t bufSize;				/* Do not modify this field. */
	int doAllocBuf;				/* Do not modify this field. */
	FILE *cin;				/* Do not use or modify. */
	FILE *cout;				/* Do not use or modify. */
	int ctrlSocketR;			/* You may use but not modify/close. */
	int ctrlSocketW;			/* You may use but not modify/close. */
	int dataSocket;				/* You may use but not modify/close. */
	int leavePass;				/* You may modify this. */
	int eofOkay;				/* Do not use or modify. */
	int require20;				/* You may modify this. */
	int usingTAR;				/* Use, but do not modify. */
	int serverType;				/* Do not use or modify. */
	int ietfCompatLevel;			/* Do not use or modify. */
	int numDownloads;			/* Do not use or modify. */
	int numUploads;				/* Do not use or modify. */
	int numListings;			/* Do not use or modify. */
	int doNotGetStartingWorkingDirectory;	/* You may modify this. */
#if USE_SIO
	char srlBuf[768];
	SReadlineInfo ctrlSrl;			/* Do not use or modify. */
#endif
	int iUser;				/* Scratch integer field you can use. */
	void *pUser;				/* Scratch pointer field you can use. */
	longest_int llUser;			/* Scratch long long field you can use. */
	int reserved[32];			/* Do not use or modify. */
} FTPConnectionInfo;

typedef struct FileInfo *FileInfoPtr, **FileInfoVec;
typedef struct FileInfo {
	FileInfoPtr prev, next;
	char *relname;
	char *rname;
	char *rlinkto;
	char *lname;
	char *plug;	/* permissions, links, user, group */
	int type;
	time_t mdtm;
	longest_int size;
	size_t relnameLen;
	int mode;	/* Only set by UnMlsD() */
} FileInfo;

typedef struct FileInfoList {
	FileInfoPtr first, last;
	FileInfoVec vec;
	size_t maxFileLen;
	size_t maxPlugLen;
	int nFileInfos;
	int sortKey;
	int sortOrder;
} FileInfoList, *FileInfoListPtr;

/* Used with UnMlsT() */
typedef struct MLstItem{
	char fname[512];
	char linkto[512];
	int ftype;
	longest_int fsize;
	time_t ftime;
	int mode;		/* "UNIX.mode" fact */
	int uid;		/* "UNIX.uid" fact */
	int gid;		/* "UNIX.gid" fact */
	char perm[16];		/* "perm" fact */
	char owner[16];		/* "UNIX.owner" fact */
	char group[16];		/* "UNIX.group" fact */
} MLstItem, *MLstItemPtr;

/* Messages we pass to the current progress meter function. */
#define kPrInitMsg			1
#define kPrUpdateMsg			2
#define kPrEndMsg			3

/* Parameter for OpenDataConnection() */
#define kSendPortMode			0
#define kPassiveMode			1
#define kFallBackToSendPortMode		2

/* Parameter for AcceptDataConnection() */
#define kAcceptForWriting		00100
#define kAcceptForReading		00101
#define kNetWriting			kAcceptForWriting
#define kNetReading			kAcceptForReading

/* Value for printMode field of Response structure.
 * Generally, don't worry about this.
 */
#define kResponseNoPrint 00001
#define kResponseNoSave  00002
#define kResponseNoProc  00002

#define kDefaultFTPPort			21

#define kDefaultFTPBufSize		32768

#ifdef USE_SIO
/* This version of the library can handle timeouts without
 * a user-installed signal handler.
 */
#define kDefaultXferTimeout		600
#define kDefaultConnTimeout		30
#define kDefaultCtrlTimeout		135
#define kDefaultAbortTimeout	10
#else
/* The library doesn't use timeouts by default because it would
 * break apps that don't have a SIGALRM handler.
 */
#define kDefaultXferTimeout		(0)	/* No timeout. */
#define kDefaultConnTimeout		(0)	/* No timeout. */
#define kDefaultCtrlTimeout		(0)	/* No timeout. */
#define kDefaultAbortTimeout		10
#endif


/* Suggested timeout values, in seconds, if you use timeouts. */
#define kSuggestedDefaultXferTimeout	(0)	/* No timeout on data blocks. */
#define kSuggestedDefaultConnTimeout	30
#define kSuggestedDefaultCtrlTimeout	135	/* 2*MSL, + slop */ 
#define kSuggestedAbortTimeout		10

#define kDefaultMaxDials		3
#define kDefaultRedialDelay		20	/* seconds */

#define kDefaultDataPortMode		kSendPortMode

#define kRedialStatusDialing		0
#define kRedialStatusSleeping		1

#ifndef INADDR_NONE
#	define INADDR_NONE		(0xffffffff)	/* <netinet/in.h> should have it. */
#endif
#ifndef INADDR_ANY
#	define INADDR_ANY		(0x00000000)
#endif

#define kTypeAscii			'A'
#define kTypeBinary			'I'
#define kTypeEbcdic			'E'

#define kGlobChars 			"[*?"
#define GLOBCHARSINSTR(a)		(strpbrk(a, kGlobChars) != NULL)

#define kGlobYes			1
#define kGlobNo				0
#define kRecursiveYes			1
#define kRecursiveNo			0
#define kAppendYes			1
#define kAppendNo			0
#define kResumeYes			1
#define kResumeNo			0
#define kDeleteYes			1
#define kDeleteNo			0
#define kTarYes				1
#define kTarNo				0

#define UNIMPLEMENTED_CMD(a)		((a == 500) || (a == 502) || (a == 504))

/* Possible values returned by GetDateAndTime. */
#define kSizeUnknown			((longest_int) (-1))
#define kModTimeUnknown			((time_t) (-1))

#define kCommandAvailabilityUnknown	(-1)
#define kCommandAvailable		1
#define kCommandNotAvailable		0

/* Values returned by FTPDecodeURL. */
#define kNotURL				(-1)
#define kMalformedURL			(-2)

/* Values for the firewall/proxy open. */
#define kFirewallNotInUse			0
#define kFirewallUserAtSite			1
#define kFirewallLoginThenUserAtSite		2
#define kFirewallSiteSite			3
#define kFirewallOpenSite			4
#define kFirewallUserAtUserPassAtPass		5
#define kFirewallFwuAtSiteFwpUserPass		6
#define kFirewallUserAtSiteFwuPassFwp		7
#define kFirewallLastType			kFirewallUserAtSiteFwuPassFwp

/* For MLSD, MLST, and STAT. */
#define kPreferredMlsOpts	(kMlsOptType | kMlsOptSize | kMlsOptModify | kMlsOptUNIXmode | kMlsOptUNIXowner | kMlsOptUNIXgroup | kMlsOptUNIXuid | kMlsOptUNIXgid | kMlsOptPerm)

#define kMlsOptType		00001
#define kMlsOptSize		00002
#define kMlsOptModify		00004
#define kMlsOptUNIXmode		00010
#define kMlsOptUNIXowner	00020
#define kMlsOptUNIXgroup	00040
#define kMlsOptPerm		00100
#define kMlsOptUNIXuid		00200
#define kMlsOptUNIXgid		00400
#define kMlsOptUnique		01000

/* For FTPChdir3(). */
#define kChdirOnly		00000
#define kChdirAndMkdir		00001
#define kChdirAndGetCWD		00002
#define kChdirOneSubdirAtATime	00004
#define kChdirFullPath		00010

/* Return codes for custom ConfirmResumeDownloadProcs. */
#define kConfirmResumeProcNotUsed 0
#define kConfirmResumeProcSaidSkip 1
#define kConfirmResumeProcSaidResume 2
#define kConfirmResumeProcSaidOverwrite 3
#define kConfirmResumeProcSaidAppend 4
#define kConfirmResumeProcSaidBestGuess 5
#define kConfirmResumeProcSaidCancel 6

typedef int (*FTPConfirmResumeDownloadProc)(
	const char **localpath,
	longest_int localsize,
	time_t localmtime,
	const char *remotepath,
	longest_int remotesize,
	time_t remotetime,
	longest_int *startPoint
);

typedef int (*FTPConfirmResumeUploadProc)(
	const char *localpath,
	longest_int localsize,
	time_t localmtime,
	const char **remotepath,
	longest_int remotesize,
	time_t remotetime,
	longest_int *startPoint
);

#define kNoFTPConfirmResumeDownloadProc	((FTPConfirmResumeDownloadProc) 0)
#define kNoFTPConfirmResumeUploadProc	((FTPConfirmResumeUploadProc) 0)
#define kNoFTPGetPassphraseProc		((FTPGetPassphraseProc) 0)
#define kNoFTPLogProcProc		((FTPLogProc) 0)
#define kNoFTPConnectMessageProc	((FTPConnectMessageProc) 0)
#define kNoFTPLoginMessageProc		((FTPLoginMessageProc) 0)
#define kNoFTPRedialStatusProc		((FTPRedialStatusProc) 0)

/* Types of FTP server software.
 *
 * We try to recognize a few of these, for information
 * only, and occasional uses to determine some additional
 * or broken functionality.
 */
#define kServerTypeUnknown		0
#define kServerTypeWuFTPd		1
#define kServerTypeNcFTPd		2
#define kServerTypeProFTPD		3
#define kServerTypeMicrosoftFTP		4
#define kServerTypeWarFTPd		5
#define kServerTypeServ_U		6
#define kServerTypeWFTPD		7
#define kServerTypeVFTPD		8
#define kServerTypeFTP_Max		9
#define kServerTypeRoxen		10
#define kServerTypeNetWareFTP		11
#define kServerTypeWS_FTP		12

#if !defined(WIN32) && !defined(_WINDOWS) && !defined(closesocket)
#	define closesocket close
#endif

#if !defined(WIN32) && !defined(_WINDOWS) && !defined(ioctlsocket)
#	define ioctlsocket ioctl
#endif

#ifndef STDIN_FILENO
#	define STDIN_FILENO 0
#	define STDOUT_FILENO 1
#	define STDERR_FILENO 2
#endif

#if defined(WIN32) || defined(_WINDOWS)
#	define LOCAL_PATH_DELIM '\\'
#	define LOCAL_PATH_DELIM_STR "\\"
#	define LOCAL_PATH_ALTDELIM '/'
#	define IsLocalPathDelim(c) ((c == LOCAL_PATH_DELIM) || (c == LOCAL_PATH_ALTDELIM))
#	define UNC_PATH_PREFIX "\\\\"
#	define IsUNCPrefixed(s) (IsLocalPathDelim(s[0]) && IsLocalPathDelim(s[1]))
#else
#	define LOCAL_PATH_DELIM '/'
#	define LOCAL_PATH_DELIM_STR "/"
#	define StrFindLocalPathDelim(a) strchr(a, LOCAL_PATH_DELIM)
#	define StrRFindLocalPathDelim(a) strrchr(a, LOCAL_PATH_DELIM)
#	define StrRemoveTrailingLocalPathDelim StrRemoveTrailingSlashes
#	define IsLocalPathDelim(c) (c == LOCAL_PATH_DELIM)
#	define TVFSPathToLocalPath(s)
#	define LocalPathToTVFSPath(s)
#endif

#if defined(WIN32) || defined(_WINDOWS)
typedef struct dirent {
	char d_name[MAX_PATH];
} dirent;

typedef struct DIR {
	HANDLE searchHandle;
	char *dirpath;
	WIN32_FIND_DATA ffd;
	dirent dent;
} DIR;

/* Identical in declaration to struct _stati64.
 * Unfortunately, we don't get _stati64 unless we're on an IA-64
 */
struct WinStat64 {
	_dev_t st_dev;
	_ino_t st_ino;
	unsigned short st_mode;
	short st_nlink;
	short st_uid;
	short st_gid;
	_dev_t st_rdev;
	__int64 st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
};

int WinFStat64(const int h0, struct WinStat64 *const stp);
int WinStat64(const char *const path, struct WinStat64 *const stp);

#	ifndef Stat
#		define Stat WinStat64
#		define Lstat WinStat64
#		define Fstat WinFStat64
#	endif
#endif	/* WIN32 */

/* The following block may be changed by configure script */
#ifndef Stat
#define Stat stat64
#define Lstat lstat64
#define Fstat fstat64
#endif

#define kFtwNoAutoGrowButContinue (-1)
#define kFtwNoAutoGrowAndFail 0
#define kFtwAutoGrow 1
#define kFtwMagic 0xF234567F

typedef struct FtwInfo *FtwInfoPtr;

typedef int (*FtwProc)(const FtwInfoPtr ftwip);

typedef struct FtwInfo {
	unsigned int init;
	FtwProc proc;
	char *curPath;
	size_t curPathLen;
	size_t curPathAllocSize;
	size_t startPathLen;
	char *curFile;
	size_t curFileLen;
	int curType;
	struct Stat curStat;
	int noAutoMallocAndFree;
	int dirSeparator;
	char rootDir[4];
	int autoGrow;
	size_t depth;
	size_t maxDepth;
	size_t numDirs;
	size_t numFiles;
	size_t numLinks;
	const char *rlinkto;	/* only valid during FTPFtw() */
	int reserved;
	void *cip;		/* only valid during FTPFtw() */
	void *userdata;
} FtwInfo;

typedef struct GetoptInfo {
	unsigned int init;	/* When properly initialized, set to 0xF123456F */
	int err;	/* if error message should be printed */
	int ind;	/* index into parent argv vector */
	int opt;	/* character checked for validity */
	char *arg;	/* argument associated with option */
	char *place;	/* saved position in an arg */
} GetoptInfo, *GetoptInfoPtr;

extern const char gLibNcFTPVersion[];
extern int gLibNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings;

#if (defined(__APPLE_CC__)) && (__APPLE_CC__ < 10000)
#	define LIBNCFTP_USE_VAR(a) gLibNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#elif (defined(__GNUC__)) && (__GNUC__ >= 3)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((__unused__))
#	endif
#	define LIBNCFTP_USE_VAR(a)
#elif (defined(__GNUC__)) && (__GNUC__ == 2)
#	ifndef UNUSED
#		define UNUSED(a) a __attribute__ ((unused))
#	endif
#	define LIBNCFTP_USE_VAR(a)
#else
#	define LIBNCFTP_USE_VAR(a) gLibNcFTP_Uses_Me_To_Quiet_Variable_Unused_Warnings = (a == 0)
#	ifndef UNUSED
#		define UNUSED(a) a
#	endif
#endif

/* Public routines */
void FTPAbortDataTransfer(const FTPCIPtr cip);
int FTPChdir(const FTPCIPtr cip, const char *const cdCwd);
int FTPChdirAndGetCWD(const FTPCIPtr cip, const char *const cdCwd, char *const newCwd, const size_t newCwdSize);
int FTPChdir3(FTPCIPtr cip, const char *const cdCwd, char *const newCwd, const size_t newCwdSize, int flags);
int FTPChdirList(FTPCIPtr cip, LineListPtr const cdlist, char *const newCwd, const size_t newCwdSize, int flags);
int FTPChmod(const FTPCIPtr cip, const char *const pattern, const char *const mode, const int doGlob);
int FTPCloseHost(const FTPCIPtr cip);
int FTPCmd(const FTPCIPtr cip, const char *const cmdspec, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;
int FTPDecodeURL(const FTPCIPtr cip, char *const url, LineListPtr cdlist, char *const fn, const size_t fnsize, int *const xtype, int *const wantnlst);
int FTPDelete(const FTPCIPtr cip, const char *const pattern, const int recurse, const int doGlob);
int FTPFileExists(const FTPCIPtr cip, const char *const file);
int FTPFileModificationTime(const FTPCIPtr cip, const char *const file, time_t *const mdtm);
int FTPFileSize(const FTPCIPtr cip, const char *const file, longest_int *const size, const int type);
int FTPFileSizeAndModificationTime(const FTPCIPtr cip, const char *const file, longest_int *const size, const int type, time_t *const mdtm);
int FTPFileType(const FTPCIPtr cip, const char *const file, int *const ftype);
int FTPFtw(const FTPCIPtr cip, const FtwInfoPtr ftwip, const char *const path, FtwProc proc);
int FTPGetCWD(const FTPCIPtr cip, char *const newCwd, const size_t newCwdSize);
int FTPGetFiles3(const FTPCIPtr cip, const char *pattern1, const char *const dstdir1, const int recurse, int doGlob, const int xtype, const int resumeflag, int appendflag, const int deleteflag, const int tarflag, const FTPConfirmResumeDownloadProc resumeProc, int UNUSED(reserved));
int FTPGetOneFile3(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int resumeflag, const int appendflag, const int deleteflag, const FTPConfirmResumeDownloadProc resumeProc, int UNUSED(reserved));
int FTPInitConnectionInfo(const FTPLIPtr lip, const FTPCIPtr cip, size_t bufsize);
int FTPInitLibrary(const FTPLIPtr lip);
int FTPIsDir(const FTPCIPtr cip, const char *const dir);
int FTPIsRegularFile(const FTPCIPtr cip, const char *const file);
int FTPList(const FTPCIPtr cip, const int outfd, const int longMode, const char *const lsflag);
int FTPListToMemory(const FTPCIPtr cip, const char *const pattern, const LineListPtr llines, const char *const lsflags);
int FTPListToMemory2(const FTPCIPtr cip, const char *const pattern, const LineListPtr llines, const char *const lsflags, const int blanklines, int *const tryMLSD);
int FTPLocalGlob(FTPCIPtr cip, LineListPtr fileList, const char *pattern, int doGlob);
int FTPLoginHost(const FTPCIPtr cip);
int FTPMkdir(const FTPCIPtr cip, const char *const newDir, const int recurse);
int FTPOpenHost(const FTPCIPtr cip);
int FTPOpenHostNoLogin(const FTPCIPtr cip);
void FTPPerror(const FTPCIPtr cip, const int err, const int eerr, const char *const s1, const char *const s2);
int FTPPutFiles3(const FTPCIPtr cip, const char *const pattern, const char *const dstdir1, const int recurse, const int doGlob, const int xtype, int appendflag, const char *const tmppfx, const char *const tmpsfx, const int resumeflag, const int deleteflag, const FTPConfirmResumeUploadProc resumeProc, int UNUSED(reserved));
int FTPPutOneFile3(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int appendflag, const char *const tmppfx, const char *const tmpsfx, const int resumeflag, const int deleteflag, const FTPConfirmResumeUploadProc resumeProc, int UNUSED(reserved));
int FTPRemoteGlob(FTPCIPtr cip, LineListPtr fileList, const char *pattern, int doGlob);
int FTPRename(const FTPCIPtr cip, const char *const oldname, const char *const newname);
int FTPRmdir(const FTPCIPtr cip, const char *const pattern, const int recurse, const int doGlob);
void FTPShutdownHost(const FTPCIPtr cip);
const char *FTPStrError(int e);
char *FTPStrError2(const FTPCIPtr cip, int e, char *const dst, const size_t dstsize, int eerr);
int FTPSymlink(const FTPCIPtr cip, const char *const lfrom, const char *const lto);
int FTPUmask(const FTPCIPtr cip, const char *const umsk);
int FTPUtime(const FTPCIPtr cip, const char *const file, time_t actime, time_t modtime, time_t crtime);

/* LineList routines */
int CopyLineList(LineListPtr, LineListPtr);
void DisposeLineListContents(LineListPtr);
void InitLineList(LineListPtr);
LinePtr RemoveLine(LineListPtr, LinePtr);
LinePtr AddLine(LineListPtr, const char *);

/* Ftw routines */
void FtwInit(FtwInfo *const ftwip);
void FtwDispose(FtwInfo *const ftwip);
int Ftw(FtwInfo *const ftwip, const char *const path, FtwProc proc);
void FtwSetBuf(FtwInfo *const ftwip, char *const buf, const size_t bufsize, int autogrow);
#if defined(WIN32) || defined(_WINDOWS)
DIR *opendir(const char *const path);
struct dirent *readdir(DIR *dir);
void closedir(DIR *dir);
#endif	/* WIN32 */

/* Getopt routines */
void GetoptReset(GetoptInfo *const opt);
int Getopt(GetoptInfo *const opt, int nargc, char **const nargv, const char *const ostr);

/* Other routines that might be useful. */
void InitWinsock(void);
void DisposeWinsock(void);
char *StrDup(const char *);
char *FGets(char *, size_t, FILE *);
void GetHomeDir(char *, size_t);
void GetUsrName(char *, size_t);
void Scramble(unsigned char *dst, size_t dsize, unsigned char *src, char *key);
struct tm *Gmtime(time_t t, struct tm *const tp);
struct tm *Localtime(time_t t, struct tm *const tp);
time_t GetUTCOffset(const int mon, const int mday);
time_t UnMDTMDate(char *);
int MkDirs(const char *const, int mode1);
char *GetPass(const char *const prompt, char *const pwbuf, const size_t pwbufsize);
int FilenameExtensionIndicatesASCII(const char *const pathName, const char *const extnList);
void StrRemoveTrailingSlashes(char *dst);
#if defined(WIN32) || defined(_WINDOWS)
char *StrFindLocalPathDelim(const char *src);
char *StrRFindLocalPathDelim(const char *src);
void StrRemoveTrailingLocalPathDelim(char *dst);
void TVFSPathToLocalPath(char *dst);
void LocalPathToTVFSPath(char *dst);
int gettimeofday(struct timeval *const tp, void *junk);
void WinSleep(unsigned int seconds);
#endif

#ifdef HAVE_SIGACTION
void (*NcSignal(int signum, void (*handler)(int)))(int);
#elif !defined(NcSignal)
#	define NcSignal signal
#endif

/* Obselete routines. */
int FTPGetOneFile(const FTPCIPtr cip, const char *const file, const char *const dstfile);
int FTPGetOneFile2(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int resumeflag, const int appendflag);
int FTPGetFiles(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob);
int FTPGetFiles2(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob, const int xtype, const int resumeflag, const int appendflag);
int FTPGetOneFileAscii(const FTPCIPtr cip, const char *const file, const char *const dstfile);
int FTPGetFilesAscii(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob);
int FTPPutOneFile(const FTPCIPtr cip, const char *const file, const char *const dstfile);
int FTPPutOneFile2(const FTPCIPtr cip, const char *const file, const char *const dstfile, const int xtype, const int fdtouse, const int appendflag, const char *const tmppfx, const char *const tmpsfx);
int FTPPutFiles(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob);
int FTPPutFiles2(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob, const int xtype, const int appendflag, const char *const tmppfx, const char *const tmpsfx);
int FTPPutOneFileAscii(const FTPCIPtr cip, const char *const file, const char *const dstfile);
int FTPPutFilesAscii(const FTPCIPtr cip, const char *const pattern, const char *const dstdir, const int recurse, const int doGlob);
int FTPPutFileFromMemory(const FTPCIPtr cip, const char *volatile dstfile, const char *volatile src, const size_t srcLen, const int appendflag);

/* Everything else below are private routines, or stuff for testing */
int FTPInitConnectionInfo2(const FTPLIPtr lip, const FTPCIPtr cip, char *const buf, size_t bufSize);
char *FTPGetLocalCWD(char *buf, size_t size);
int FTPQueryFeatures(const FTPCIPtr);
int FTPMListOneFile(const FTPCIPtr cip, const char *const file, const MLstItemPtr mlip);
void FTPInitializeAnonPassword(const FTPLIPtr);
void FTPInitIOTimer(const FTPCIPtr);
int FTPStartDataCmd(const FTPCIPtr, int, int, longest_int, const char *,...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 5, 6)))
#endif
;
void FTPStartIOTimer(const FTPCIPtr);
void FTPStopIOTimer(const FTPCIPtr);
void FTPUpdateIOTimer(const FTPCIPtr);
int FTPSetTransferType(const FTPCIPtr, int);
int FTPEndDataCmd(const FTPCIPtr, int);
int FTPRemoteHelp(const FTPCIPtr, const char *const, const LineListPtr);
int FTPCmdNoResponse(const FTPCIPtr, const char *const cmdspec,...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;
int WaitResponse(const FTPCIPtr, unsigned int);

/* We suggest using the Ftw() routines rather than these two. */
int FTPLocalRecursiveFileList(FTPCIPtr, LineListPtr, FileInfoListPtr);
int FTPLocalRecursiveFileList2(FTPCIPtr cip, LineListPtr fileList, FileInfoListPtr files, int erelative);

int FTPMkdir2(const FTPCIPtr cip, const char *const newDir, const int recurse, const char *const curDir);

/* FTPFtw() is recommended since it is more reliable (read: thorough),
 * but these two work faster.
 */
int FTPRemoteRecursiveFileList(FTPCIPtr, LineListPtr, FileInfoListPtr);
int FTPRemoteRecursiveFileList1(FTPCIPtr, char *const, FileInfoListPtr);

int FTPRebuildConnectionInfo(const FTPLIPtr lip, const FTPCIPtr cip);

/* FTPFileExists() calls these as needed. */
int FTPFileExistsStat(const FTPCIPtr cip, const char *const file);
int FTPFileExistsNlst(const FTPCIPtr cip, const char *const file);
int FTPFileExists2(const FTPCIPtr cip, const char *const file, const int tryMDTM, const int trySIZE, const int tryMLST, const int trySTAT, const int tryNLST);

int BufferGets(char *, size_t, int, char *, char **, char **, size_t);
void DisposeFileInfoListContents(FileInfoListPtr);
void InitFileInfoList(FileInfoListPtr);
void InitFileInfo(FileInfoPtr);
FileInfoPtr RemoveFileInfo(FileInfoListPtr, FileInfoPtr);
FileInfoPtr AddFileInfo(FileInfoListPtr, FileInfoPtr);
void SortFileInfoList(FileInfoListPtr, int, int);
void VectorizeFileInfoList(FileInfoListPtr);
void UnvectorizeFileInfoList(FileInfoListPtr);
int ComputeRNames(FileInfoListPtr, const char *, int, int);
int ComputeLNames(FileInfoListPtr, const char *, const char *, int);
int ConcatFileInfoList(FileInfoListPtr, FileInfoListPtr);
int ConcatFileToFileInfoList(FileInfoListPtr, char *);
int LineListToFileInfoList(LineListPtr, FileInfoListPtr);
int LineToFileInfoList(LinePtr, FileInfoListPtr);
void URLCopyToken(char *, size_t, const char *, size_t);
int UnMlsT(const char *const, const MLstItemPtr);
int UnMlsD(FileInfoListPtr, LineListPtr);
int UnLslR(FileInfoListPtr, LineListPtr, int);
void TraceResponse(const FTPCIPtr, ResponsePtr);
void PrintResponse(const FTPCIPtr, LineListPtr);
void DoneWithResponse(const FTPCIPtr, ResponsePtr);
ResponsePtr InitResponse(void);
void ReInitResponse(const FTPCIPtr, ResponsePtr);
int GetTelnetString(const FTPCIPtr, char *, size_t, FILE *, FILE *);
int GetResponse(const FTPCIPtr, ResponsePtr);
int RCmd(const FTPCIPtr, ResponsePtr, const char *, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 3, 4)))
#endif
;

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* _ncftp_h_ */
