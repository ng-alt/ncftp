/* u_getcwd.c
 *
 * Copyright (c) 2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Use getcwd/getwd to get the full path of the current local
 * working directory.
 */
char *
FTPGetLocalCWD(char *buf, size_t size)
{
#ifdef HAVE_GETCWD
	memset(buf, 0, size);
	if ((getcwd(buf, size - 1) == NULL) || (buf[size - 1] != '\0') || (buf[size - 2] != '\0')) {
		memset(buf, 0, size);
		return NULL;	
	}
	return (buf);
#elif defined(HAVE_GETWD) && !defined(_REENTRANT)
	static char *cwdBuf = NULL;
	char *dp;
	
	/* Due to the way getwd is usually implemented, it's
	 * important to have a buffer large enough to hold the
	 * whole thing.  getwd usually starts at the end of the
	 * buffer, and works backwards, returning you a pointer
	 * to the beginning of it when it finishes.
	 */
	if (size < MAXPATHLEN) {
		/* Buffer not big enough, so use a temporary one,
		 * and then copy the first 'size' bytes of the
		 * temporary buffer to your 'buf.'
		 */
		if (cwdBuf == NULL) {
			cwdBuf = (char *) malloc((size_t) MAXPATHLEN);
			if (cwdBuf == NULL) {
				return NULL;
			}
		}
		dp = cwdBuf;
	} else {
		/* Buffer is big enough already. */
		dp = buf;
	}
	*dp = '\0';
	if (getwd(dp) == NULL) {
		/* getwd() should write the reason why in the buffer then,
		 * according to the man pages.
		 */
		(void) Strncpy(buf, ".", size);
		return (NULL);
	}
	return (Strncpy(buf, dp, size));

#elif (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	memset(buf, 0, size);
	if ((GetCurrentDirectory((DWORD) size - 1, buf) < 1) || (buf[size - 1] != '\0') || (buf[size - 2] != '\0')) {
		memset(buf, 0, size);
		return NULL;	
	}
	return buf;
#else
	/* Not a solution, but does anybody not have either of
	 * getcwd or getwd?
	 */
	--Error--;
#endif
}   /* FTPGetLocalCWD */
