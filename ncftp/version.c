#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma no_pch
#	pragma hdrstop
#endif

/******************************************************************************/

const char gVersion[] = "@(#) NcFTP 3.1.2/975 Jan 28 2002, 11:50 PM";

/******************************************************************************/

#ifdef O_S
const char gOS[] = O_S;
#elif defined(WIN32) || defined(_WINDOWS)
const char gOS[] = "Windows";
#else
const char gOS[] = "UNIX";
#endif

/******************************************************************************/

const char gCopyright[] = "@(#) \
Copyright (c) 1992-2002 by Mike Gleason.\n\
All rights reserved.\n\
";

/******************************************************************************/
