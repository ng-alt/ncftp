/* rglobr.c
 *
 * Copyright (c) 1996-2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if 0
static void
print1(FileInfoListPtr list)
{
	FileInfoPtr fip;
	int i;

	for (i = 1, fip = list->first; fip != NULL; fip = fip->next, i++)
		printf("%d: %s\n", i, fip->relname == NULL ? "NULL" : fip->relname);
}



static void
print2(FileInfoListPtr list)
{
	FileInfoPtr fip;
	int i, n;

	n = list->nFileInfos;
	for (i=0; i<n; i++) {
		fip = list->vec[i];
		printf("%d: %s\n", i + 1, fip->relname == NULL ? "NULL" : fip->relname);
	}
}




static void
SortRecursiveFileList(FileInfoListPtr files)
{
	VectorizeFileInfoList(files);
	SortFileInfoList(files, 'b', '?');
	UnvectorizeFileInfoList(files);
}	/* SortRecursiveFileList */
#endif




int
FTPRemoteRecursiveFileList1(FTPCIPtr cip, char *const rdir, FileInfoListPtr files)
{
	LineList dirContents;
	FileInfoList fil;
	char cwd[512];
	int result;

	if ((result = FTPGetCWD(cip, cwd, sizeof(cwd))) < 0)
		return (result);

	InitFileInfoList(files);

	if (rdir == NULL)
		return (-1);

	if (FTPChdir(cip, rdir) < 0) {
		/* Probably not a directory.
		 * Just add it as a plain file
		 * to the list.
		 */
		(void) ConcatFileToFileInfoList(files, rdir);
		return (kNoErr);
	}

	/* Paths collected must be relative. */
	if ((result = FTPListToMemory2(cip, "", &dirContents, "-lRa", 1, (int *) 0)) < 0) {
		if ((result = FTPChdir(cip, cwd)) < 0) {
			return (result);
		}
	}

	(void) UnLslR(&fil, &dirContents, cip->serverType);
	DisposeLineListContents(&dirContents);
	/* Could sort it to breadth-first here. */
	/* (void) SortRecursiveFileList(&fil); */
	(void) ComputeRNames(&fil, rdir, 1, 1);
	(void) ConcatFileInfoList(files, &fil);
	DisposeFileInfoListContents(&fil);

	if ((result = FTPChdir(cip, cwd)) < 0) {
		return (result);
	}
	return (kNoErr);
}	/* FTPRemoteRecursiveFileList1 */




int
FTPRemoteRecursiveFileList(FTPCIPtr cip, LineListPtr fileList, FileInfoListPtr files)
{
	LinePtr filePtr, nextFilePtr;
	LineList dirContents;
	FileInfoList fil;
	char cwd[512];
	int result;
	char *rdir;

	if ((result = FTPGetCWD(cip, cwd, sizeof(cwd))) < 0)
		return (result);

	InitFileInfoList(files);

	for (filePtr = fileList->first;
		filePtr != NULL;
		filePtr = nextFilePtr)
	{
		nextFilePtr = filePtr->next;

		rdir = filePtr->line;
		if (rdir == NULL)
			continue;

		if (FTPChdir(cip, rdir) < 0) {
			/* Probably not a directory.
			 * Just add it as a plain file
			 * to the list.
			 */
			(void) ConcatFileToFileInfoList(files, rdir);
			continue;
		}

		/* Paths collected must be relative. */
		if ((result = FTPListToMemory2(cip, "", &dirContents, "-lRa", 1, (int *) 0)) < 0) {
			goto goback;
		}

		(void) UnLslR(&fil, &dirContents, cip->serverType);
		DisposeLineListContents(&dirContents);
		(void) ComputeRNames(&fil, rdir, 1, 1);
		(void) ConcatFileInfoList(files, &fil);
		DisposeFileInfoListContents(&fil);

goback:
		if ((result = FTPChdir(cip, cwd)) < 0) {
			return (result);
		}
	}	
	return (kNoErr);
}	/* FTPRemoteRecursiveFileList */
