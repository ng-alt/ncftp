/* u_getpw.c
 *
 * Copyright (c) 2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if defined(WIN32) || defined(_WINDOWS)
#else

#ifdef BSDOS
int getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
#endif

int
GetPwUid(struct passwd *pwp, const uid_t uid, char *const pwbuf, size_t pwbufsize)
{
	struct passwd *p;

#if defined(HAVE_GETPWUID_R) && (defined(SOLARIS) && !defined(_POSIX_PTHREAD_SEMANTICS))
	memset(pwbuf, 0, pwbufsize);
	p = getpwuid_r(uid, pwp, pwbuf, pwbufsize);
	if (p != NULL)
		return (0);
#elif defined(HAVE__POSIX_GETPWUID_R)
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((_posix_getpwuid_r(uid, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#elif defined(HAVE_GETPWUID_R)
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((getpwuid_r(uid, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#else
	p = getpwuid(uid);
	if (p != NULL) {
		memcpy(pwp, p, sizeof(struct passwd));
		return (0);
	} else {
		memset(pwbuf, 0, pwbufsize);
		memset(pwp, 0, sizeof(struct passwd));
	}
#endif
	return (-1);
}	/* GetPwUid */




int
GetPwNam(struct passwd *pwp, const char *const nam, char *const pwbuf, size_t pwbufsize)
{
	struct passwd *p;

#if defined(HAVE_GETPWNAM_R) && (defined(SOLARIS) && !defined(_POSIX_PTHREAD_SEMANTICS))
	memset(pwbuf, 0, pwbufsize);
	p = getpwnam_r(nam, pwp, pwbuf, pwbufsize);
	if (p != NULL)
		return (0);
#elif defined(HAVE__POSIX_GETPWNAM_R)
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((_posix_getpwnam_r(nam, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#elif defined(HAVE_GETPWNAM_R)
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((getpwnam_r(nam, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#else
	p = getpwnam(nam);
	if (p != NULL) {
		memcpy(pwp, p, sizeof(struct passwd));
		return (0);
	} else {
		memset(pwbuf, 0, pwbufsize);
		memset(pwp, 0, sizeof(struct passwd));
	}
#endif
	return (-1);
}	/* GetPwNam */



/* This looks up the user's password entry, trying to look by the username.
 * We have a couple of extra hacks in place to increase the probability
 * that we can get the username.
 */
int
GetMyPwEnt(struct passwd *pwp, char *const pwbuf, size_t pwbufsize)
{
	char *cp;
	int rc;

#ifdef HAVE_GETLOGIN_R
	char logname[128];
	memset(logname, 0, sizeof(logname));
	(void) getlogin_r(logname, sizeof(logname) - 1);
	cp = (logname[0] == '\0') ? NULL : logname;
#else
	cp = getlogin();
#endif

	if (cp == NULL) {
		cp = (char *) getenv("LOGNAME");
		if (cp == NULL)
			cp = (char *) getenv("USER");
	}
	rc = -1;
	if ((cp != NULL) && (cp[0] != '\0'))
		rc = GetPwNam(pwp, cp, pwbuf, pwbufsize);
	if (rc != 0)
		rc = GetPwUid(pwp, getuid(), pwbuf, pwbufsize);
	return (rc);
}	/* GetMyPwEnt */

#endif	/* UNIX */
