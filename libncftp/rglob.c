/* rglob.c
 *
 * Copyright (c) 2001 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* We need to use this because using NLST gives us more stuff than
 * we want back sometimes.  For example, say we have:
 *
 * /a		(directory)
 * /a/b		(directory)
 * /a/b/b1
 * /a/b/b2
 * /a/b/b3
 * /a/c		(directory)
 * /a/c/c1
 * /a/c/c2
 * /a/c/c3
 * /a/file
 *
 * If you did an "echo /a/<star>" in a normal unix shell, you would expect
 * to get back /a/b /a/c /a/file.  But NLST gives back:
 *
 * /a/b/b1
 * /a/b/b2
 * /a/b/b3
 * /a/c/c1
 * /a/c/c2
 * /a/c/c3
 * /a/file
 *
 * So we use the following routine to convert into the format I expect.
 */

void
RemoteGlobCollapse(const char *pattern, LineListPtr fileList)
{
	LinePtr lp, nextLine;
	string patPrefix;
	string cur, prev;
	char *endp, *cp, *dp;
	const char *pp, *cp2;
	int wasGlobChar;
	size_t plen;

	/* Copy all characters before the first glob-char. */
	dp = patPrefix;
	endp = dp + sizeof(patPrefix) - 1;
	wasGlobChar = 0;
	for (cp2 = pattern; dp < endp; ) {
		for (pp=kGlobChars; *pp != '\0'; pp++) {
			if (*pp == *cp2) {
				wasGlobChar = 1;
				break;
			}
		}
		if (wasGlobChar)
			break;
		*dp++ = *cp2++;
	}
	*dp = '\0';
	plen = (size_t) (dp - patPrefix);

	*prev = '\0';
	for (lp=fileList->first; lp != NULL; lp = nextLine) {
		nextLine = lp->next;
		if (strncmp(lp->line, patPrefix, plen) == 0) {
			(void) STRNCPY(cur, lp->line + plen);
			cp = strchr(cur, '/');
			if (cp == NULL)
				cp = strchr(cur, '\\');
			if (cp != NULL)
				*cp = '\0';
			if ((*prev != '\0') && (STREQ(cur, prev))) {
				nextLine = RemoveLine(fileList, lp);
			} else {
				(void) STRNCPY(prev, cur);
				/* We are playing with a dynamically
				 * allocated string, but since the
				 * following expression is guaranteed
				 * to be the same or shorter, we won't
				 * overwrite the bounds.
				 */
				(void) sprintf(lp->line, "%s%s", patPrefix, cur);
			}
		}
	}
}	/* RemoteGlobCollapse */




int
FTPRemoteGlob(FTPCIPtr cip, LineListPtr fileList, const char *pattern, int doGlob)
{
	char *cp;
	const char *lsflags;
	LinePtr lp;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (fileList == NULL)
		return (kErrBadParameter);
	InitLineList(fileList);

	if ((pattern == NULL) || (pattern[0] == '\0'))
		return (kErrBadParameter);

	/* Note that we do attempt to use glob characters even if the remote
	 * host isn't UNIX.  Most non-UNIX remote FTP servers look for UNIX
	 * style wildcards.
	 */
	if ((doGlob == 1) && (GLOBCHARSINSTR(pattern))) {
		/* Use NLST, which lists files one per line. */
		lsflags = "";
		
		/* Optimize for "NLST *" case which is same as "NLST". */
		if (strcmp(pattern, "*") == 0) {
			pattern = "";
			lsflags = "-a";
		} else if (strcmp(pattern, "**") == 0) {
			/* Hack; Lets you try "NLST -a" if you're daring. */
			/* Need to use "NLST -a" whenever possible,
			 * because wu-ftpd doesn't do NLST right, IMHO.
			 * (It doesn't include directories in the NLST
			 *  if you do "NLST /the/dir" without -a.)
			 */
			pattern = "";
			lsflags = "-a";
		}

		if ((result = FTPListToMemory2(cip, pattern, fileList, lsflags, 0, (int *) 0)) < 0) {
			if (*lsflags == '\0')
				return (result);
			/* Try again, without "-a" */
			lsflags = "";
			if ((result = FTPListToMemory2(cip, pattern, fileList, lsflags, 0, (int *) 0)) < 0) {
				return (result);
			}
		}
		if (fileList->first == NULL) {
			cip->errNo = kErrGlobNoMatch;
			return (kErrGlobNoMatch);
		}
		if (fileList->first == fileList->last) {
#define glberr(a) (ISTRNEQ(cp, a, strlen(a)))
			/* If we have only one item in the list, see if it really was
			 * an error message we would recognize.
			 */
			cp = strchr(fileList->first->line, ':');
			if (cp != NULL) {
				if (glberr(": No such file or directory")) {
					(void) RemoveLine(fileList, fileList->first);
					cip->errNo = kErrGlobFailed;
					return (kErrGlobFailed);
				} else if (glberr(": No match")) {
					cip->errNo = kErrGlobNoMatch;
					return (kErrGlobNoMatch);
				}
			}
		}
		RemoteGlobCollapse(pattern, fileList);
		for (lp=fileList->first; lp != NULL; lp = lp->next)
			PrintF(cip, "  Rglob [%s]\n", lp->line);
	} else {
		/* Or, if there were no globbing characters in 'pattern', then the
		 * pattern is really just a filename.  So for this case the
		 * file list is really just a single file.
		 */
		fileList->first = fileList->last = NULL;
		(void) AddLine(fileList, pattern);
	}
	return (kNoErr);
}	/* FTPRemoteGlob */
