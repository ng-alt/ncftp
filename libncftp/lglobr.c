/* lglobr.c
 *
 * Copyright (c) 2001 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Used by FTPLocalRecursiveFileList */
typedef struct LRFLState {
	int relativePathStartOffset;
	FileInfoListPtr filp;
} LRFLState;

static int
FTPLocalRecursiveFileListFtwProc(const FtwInfoPtr ftwip)
{
	FileInfo fi;
	mode_t m;
	LRFLState *lrflstate;

	if ((ftwip->curPath[0] == '\0') || (strcmp(ftwip->curPath, ".") == 0))
		return (0);

	lrflstate = (LRFLState *) ftwip->userdata;
	fi.relname = StrDup(ftwip->curPath + lrflstate->relativePathStartOffset);
	fi.rname = NULL;
	fi.lname = StrDup(ftwip->curPath);
	fi.mdtm = ftwip->curStat.st_mtime;
	fi.rlinkto = NULL;
	fi.plug = NULL;

	m = ftwip->curStat.st_mode;
	if (S_ISREG(m) != 0) {
		/* file */
		fi.type = '-';
		fi.size = (longest_int) ftwip->curStat.st_size;
		(void) AddFileInfo(lrflstate->filp, &fi);
	} else if (S_ISDIR(m)) {
		/* directory */
		fi.type = 'd';
		fi.size = (longest_int) 0;
		(void) AddFileInfo(lrflstate->filp, &fi);
#if defined(S_ISLNK) && defined(HAVE_READLINK)
	} else if (S_ISLNK(m)) {
		/* symbolic link */
		fi.type = 'l';
		fi.size = (longest_int) 0;
		fi.rlinkto = calloc(256, 1);
		if (fi.rlinkto != NULL) {
			if (readlink(ftwip->curPath, fi.rlinkto, 255) < 0) {
				free(fi.rlinkto);
				fi.rlinkto = NULL;
			} else {
				(void) AddFileInfo(lrflstate->filp, &fi);
			}
		}
		if (fi.rlinkto == NULL) {
			free(fi.relname);
			free(fi.lname);
		}
#endif	/* S_ISLNK */
	} else {
		/* Unknown type, skip it */
		free(fi.relname);
		free(fi.lname);
	}

	return (0);
}	/* FTPLocalRecursiveFileListFtwProc */




int
FTPLocalRecursiveFileList2(FTPCIPtr cip, LineListPtr fileList, FileInfoListPtr files, int erelative)
{
	LinePtr filePtr, nextFilePtr;
	char *cp;
	FtwInfo ftwi;
	LRFLState lrflstate;
	struct Stat st;
	FileInfo fi;

	FtwInit(&ftwi);
	InitFileInfoList(files);
	lrflstate.filp = files;

	for (filePtr = fileList->first;
		filePtr != NULL;
		filePtr = nextFilePtr)
	{
		nextFilePtr = filePtr->next;

		cp = NULL;
		if (erelative != 0) {
			/* Don't skip anything if requested */
			lrflstate.relativePathStartOffset = 0;
			cp = filePtr->line;
		} if (strcmp(filePtr->line, ".") == 0) {
			/* skip . and then a / character */
			lrflstate.relativePathStartOffset = 2;
		} else if (strcmp(filePtr->line, "") == 0) {
			/* skip a / character */
			lrflstate.relativePathStartOffset = 1;
		} else if ((cp = StrRFindLocalPathDelim(filePtr->line)) == NULL) {
			/* Don't skip anything if this is a directory */
			lrflstate.relativePathStartOffset = 0;
			cp = filePtr->line;
		} else {
			/* Skip everything except the basename directory */
			cp++;
			lrflstate.relativePathStartOffset = (int) (cp - filePtr->line);
		}

		/* Use Stat rather than Lstat so top level path can be a link. */
		if (Stat(filePtr->line[0] == '\0' ? "." : filePtr->line, &st) < 0) {
			Error(cip, kDoPerror, "could not stat %s.\n", filePtr->line[0] == '\0' ? "." : filePtr->line);
			continue;
		}

		if (S_ISDIR(st.st_mode) == 0) {
			fi.relname = StrDup(cp);
			fi.rname = NULL;
			fi.lname = StrDup(filePtr->line);
			fi.mdtm = st.st_mtime;
			fi.size = (longest_int) st.st_size;
			fi.rlinkto = NULL;
			fi.plug = NULL;
			fi.type = '-';
			(void) AddFileInfo(files, &fi);
			continue;			/* wasn't a directory */
		}

		/* Paths collected must be relative. */
		ftwi.userdata = &lrflstate;
		(void) Ftw(&ftwi, filePtr->line, FTPLocalRecursiveFileListFtwProc);
	}

	FtwDispose(&ftwi);
	return (kNoErr);
}	/* FTPLocalRecursiveFileList2 */




int
FTPLocalRecursiveFileList(FTPCIPtr cip, LineListPtr fileList, FileInfoListPtr files)
{
	return (FTPLocalRecursiveFileList2(cip, fileList, files, 0));
}	/* FTPLocalRecursiveFileList */