/* c_chdir3.c
 *
 * Copyright (c) 2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif
int
FTPChdir3(FTPCIPtr cip, const char *const cdCwd, char *const newCwd, const size_t newCwdSize, int flags)
{
	char *cp, *startcp;
	int result;
	int lastSubDir;
	int mkd, pwd;
	int chdir_err;
	int did_chdir, did_mkdir;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cdCwd == NULL) {
		result = kErrInvalidDirParam;
		cip->errNo = kErrInvalidDirParam;
		return result;
	}

	/* Retain backwards compatibility with earlier library versions. */
	if (flags == kChdirOnly)
		flags = kChdirFullPath;

	chdir_err = kNoErr;
	if ((flags & kChdirFullPath) != 0) {
		did_chdir = 0;
		did_mkdir = 0;
		if ((flags & kChdirAndGetCWD) != 0) {
			result = FTPChdirAndGetCWD(cip, cdCwd, newCwd, newCwdSize);
			if (result == kNoErr) {
				did_chdir = 1;
			} else {
				chdir_err = result;
			}
		} else {
			result = FTPChdir(cip, cdCwd);
			if (result == kNoErr) {
				did_chdir = 1;
			} else {
				chdir_err = result;
			}
		}
		if ((did_chdir == 0) && ((flags & kChdirAndMkdir) != 0)) {
			result = FTPMkdir(cip, cdCwd, kRecursiveYes);
			if (result == kNoErr) {
				did_mkdir = 1;
			}
		}
		/* If we just created the directory, chdir to it now. */
		if ((did_mkdir != 0) && ((flags & kChdirAndGetCWD) != 0)) {
			result = FTPChdirAndGetCWD(cip, cdCwd, newCwd, newCwdSize);
			if (result == kNoErr) {
				did_chdir = 1;
			} else {
				chdir_err = result;
			}
		} else if (did_mkdir != 0) {
			result = FTPChdir(cip, cdCwd);
			if (result == kNoErr) {
				did_chdir = 1;
			} else {
				chdir_err = result;
			}
		}
		if (did_chdir != 0)
			return (kNoErr);
	}

	if ((flags & kChdirOneSubdirAtATime) == 0) {
		/* Valid flags must have one or both of 
		 * kChdirOneSubdirAtATime and kChdirFullPath set.
		 * If we're here, subdir mode was not requested,
		 * and if chdir_err is 0 then kChdirFullPath was
		 * not set either.
		 */
		if (chdir_err != kNoErr)
			return (chdir_err);
		/* below: return (kErrBadParameter); */
	} else {
		cp = cip->buf;
		cp[cip->bufSize - 1] = '\0';
		(void) Strncpy(cip->buf, cdCwd, cip->bufSize);
		if (cp[cip->bufSize - 1] != '\0')
			return (kErrBadParameter);
		
		mkd = (flags & kChdirAndMkdir);
		pwd = (flags & kChdirAndGetCWD);

		if ((cdCwd[0] == '\0') || (strcmp(cdCwd, ".") == 0)) {
			result = 0;
			if (flags == kChdirAndGetCWD)
				result = FTPGetCWD(cip, newCwd, newCwdSize);
			return (result);
		}

		lastSubDir = 0;
		do {
			startcp = cp;
			cp = StrFindLocalPathDelim(cp);
			if (cp != NULL) {
				/* If this is the first slash in an absolute
				 * path, then startcp will be empty.  We will
				 * use this below to treat this as the root
				 * directory.
				 */
				*cp++ = '\0';
			} else {
				lastSubDir = 1;
			}
			if (strcmp(startcp, ".") == 0) {
				result = 0;
				if ((lastSubDir != 0) && (pwd != 0))
					result = FTPGetCWD(cip, newCwd, newCwdSize);
			} else if ((lastSubDir != 0) && (pwd != 0)) {
				result = FTPChdirAndGetCWD(cip, (*startcp != '\0') ? startcp : "/", newCwd, newCwdSize);
			} else {
				result = FTPChdir(cip, (*startcp != '\0') ? startcp : "/");
			}
			if (result < 0) {
				if ((mkd != 0) && (*startcp != '\0')) {
					if (FTPCmd(cip, "MKD %s", startcp) == 2) {
						result = FTPChdir(cip, startcp);
					} else {
						/* couldn't change nor create */
						cip->errNo = result;
					}
				} else {
					cip->errNo = result;
				}
			}
		} while ((!lastSubDir) && (result == 0));
	}

	if (flags == kChdirAndGetCWD) {
		return (FTPChdirAndGetCWD(cip, cdCwd, newCwd, newCwdSize));
	} else if (flags == kChdirAndMkdir) {
		result = FTPMkdir(cip, cdCwd, kRecursiveYes);
		if (result == kNoErr)
			result = FTPChdir(cip, cdCwd);
		return result;
	} else if (flags == (kChdirAndMkdir|kChdirAndGetCWD)) {
		result = FTPMkdir(cip, cdCwd, kRecursiveYes);
		if (result == kNoErr)
			result = FTPChdirAndGetCWD(cip, cdCwd, newCwd, newCwdSize);
		return result;
	} else if ((flags & kChdirOneSubdirAtATime) != 0) {
		cp = cip->buf;
		cp[cip->bufSize - 1] = '\0';
		(void) Strncpy(cip->buf, cdCwd, cip->bufSize);
		if (cp[cip->bufSize - 1] != '\0')
			return (kErrBadParameter);
		
		mkd = (flags & kChdirAndMkdir);
		pwd = (flags & kChdirAndGetCWD);

		if ((cdCwd[0] == '\0') || (strcmp(cdCwd, ".") == 0)) {
			result = 0;
			if (flags == kChdirAndGetCWD)
				result = FTPGetCWD(cip, newCwd, newCwdSize);
			return (result);
		}

		lastSubDir = 0;
		do {
			startcp = cp;
			cp = StrFindLocalPathDelim(cp);
			if (cp != NULL) {
				/* If this is the first slash in an absolute
				 * path, then startcp will be empty.  We will
				 * use this below to treat this as the root
				 * directory.
				 */
				*cp++ = '\0';
			} else {
				lastSubDir = 1;
			}
			if (strcmp(startcp, ".") == 0) {
				result = 0;
				if ((lastSubDir != 0) && (pwd != 0))
					result = FTPGetCWD(cip, newCwd, newCwdSize);
			} else if ((lastSubDir != 0) && (pwd != 0)) {
				result = FTPChdirAndGetCWD(cip, (*startcp != '\0') ? startcp : "/", newCwd, newCwdSize);
			} else {
				result = FTPChdir(cip, (*startcp != '\0') ? startcp : "/");
			}
			if (result < 0) {
				if ((mkd != 0) && (*startcp != '\0')) {
					if (FTPCmd(cip, "MKD %s", startcp) == 2) {
						result = FTPChdir(cip, startcp);
					} else {
						/* couldn't change nor create */
						cip->errNo = result;
					}
				} else {
					cip->errNo = result;
				}
			}
		} while ((!lastSubDir) && (result == 0));

		return (result);
	}

	return (kErrBadParameter);
}	/* FTPChdir3 */
