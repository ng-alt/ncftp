/* spool.c
 *
 * Copyright (c) 1992-2001 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"

#ifdef HAVE_LONG_FILE_NAMES

#include "spool.h"
#ifdef ncftp
#	include "trace.h"
#endif
#include "util.h"

int gUnprocessedJobs = 0;
int gJobs = 0;
int gHaveSpool = -1;

extern FTPLibraryInfo gLib;
extern char gOurDirectoryPath[], gOurInstallationPath[];
extern void CloseControlConnection(const FTPCIPtr);
extern int gSpoolSerial;

void
TruncBatchLog(void)
{
	char f[256];
	struct stat st;
	time_t t;
	int fd;

	if (gOurDirectoryPath[0] != '\0') { 
		time(&t);
		t -= 86400;
		(void) OurDirectoryPath(f, sizeof(f), kSpoolLog);
		if ((stat(f, &st) == 0) && (st.st_mtime < t)) {
			/* Truncate old log file.
			 * Do not remove it, since a process
			 * could still conceivably be going.
			 */
			fd = open(f, O_WRONLY|O_TRUNC, 00600);
			if (fd >= 0)
				close(fd);
		}
	}
}	/* TruncBatchLog */




int
HaveSpool(void)
{
#if defined(WIN32) || defined(_WINDOWS)
	char ncftpbatch[260];

	if (gHaveSpool < 0) {
		gHaveSpool = 0;
		if (gOurInstallationPath[0] != '\0') {
			OurInstallationPath(ncftpbatch, sizeof(ncftpbatch), "ncftpbatch.exe");
			gHaveSpool = (access(ncftpbatch, F_OK) == 0) ? 1 : 0;
		}
	}
#elif defined(BINDIR)
	char ncftpbatch[256];

	if (gHaveSpool < 0) {
		STRNCPY(ncftpbatch, BINDIR);
		STRNCAT(ncftpbatch, "/");
		STRNCAT(ncftpbatch, "ncftpbatch");
		gHaveSpool = (access(ncftpbatch, X_OK) == 0) ? 1 : 0;
	}
#else	/* BINDIR */
	if (gHaveSpool < 0) {
		if (geteuid() == 0) {
			gHaveSpool = (access("/usr/bin/ncftpbatch", X_OK) == 0) ? 1 : 0;
		} else {
			gHaveSpool = (system("ncftpbatch -X") == 0) ? 1 : 0;
		}
	}
#endif /* BINDIR */

	return (gHaveSpool);
}	/* HaveSpool */




int
CanSpool(void)
{
	char sdir[256];

	if (gOurDirectoryPath[0] == '\0') {
		return (-1);
	}
	if (MkSpoolDir(sdir, sizeof(sdir)) < 0)
		return (-1);
	return (0);
}	/* CanSpool */



#if defined(WIN32) || defined(_WINDOWS)
#else
static int
PWrite(int sfd, const char *const buf0, size_t size)
{
	int nleft;
	const char *buf = buf0;
	int nwrote;

	nleft = (int) size;
	for (;;) {
		nwrote = (int) write(sfd, buf, nleft);
		if (nwrote < 0) {
			if (errno != EINTR) {
				nwrote = (int) size - nleft;
				if (nwrote == 0)
					nwrote = -1;
				return (nwrote);
			} else {
				errno = 0;
				nwrote = 0;
				/* Try again. */
			}
		}
		nleft -= nwrote;
		if (nleft <= 0)
			break;
		buf += nwrote;
	}
	nwrote = (int) size - nleft;
	return (nwrote);
}	/* PWrite */
#endif



void
Jobs(void)
{
#if defined(WIN32) || defined(_WINDOWS)
	assert(0); // Not supported
#else
	char *argv[8];
	pid_t pid;
#ifdef BINDIR
	char ncftpbatch[256];

	STRNCPY(ncftpbatch, BINDIR);
	STRNCAT(ncftpbatch, "/");
	STRNCAT(ncftpbatch, "ncftpbatch");
#endif	/* BINDIR */

	pid = fork();
	if (pid < 0) {
		perror("fork");
	} else if (pid == 0) {
		argv[0] = (char *) "ncftpbatch";
		argv[1] = (char *) "-l";
		argv[2] = NULL;

#ifdef BINDIR
		(void) execv(ncftpbatch, argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in installed as %s?\n", argv[0], ncftpbatch);
#else	/* BINDIR */
		(void) execvp(argv[0], argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in your $PATH?\n", argv[0]);
#endif	/* BINDIR */
		perror(argv[0]);
		exit(1);
	} else {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif
	}
#endif
}	/* Jobs */




void
RunBatchWithCore(const FTPCIPtr cip)
{
#if defined(WIN32) || defined(_WINDOWS)
	RunBatch();
#else	/* UNIX */
	int pfd[2];
	char pfdstr[32];
	char *argv[8];
	pid_t pid = 0;
#ifdef BINDIR
	char ncftpbatch[256];

	STRNCPY(ncftpbatch, BINDIR);
	STRNCAT(ncftpbatch, "/");
	STRNCAT(ncftpbatch, "ncftpbatch");
#endif	/* BINDIR */

	if (pipe(pfd) < 0) {
		perror("pipe");
	}

	(void) sprintf(pfdstr, "%d", pfd[0]);
	pid = fork();
	if (pid < 0) {
		(void) close(pfd[0]);
		(void) close(pfd[1]);
		perror("fork");
	} else if (pid == 0) {
		(void) close(pfd[1]);	/* Child closes write end. */
		argv[0] = (char *) "ncftpbatch";
#ifdef DEBUG_NCFTPBATCH
		argv[1] = (char *) "-D";
		argv[2] = (char *) "-Z";
		argv[3] = (char *) "15";
		argv[4] = (char *) "-|";
		argv[5] = pfdstr;
		argv[6] = NULL;
#else
		argv[1] = (char *) "-d";
		argv[2] = (char *) "-|";
		argv[3] = pfdstr;
		argv[4] = NULL;
#endif

#ifdef BINDIR
		(void) execv(ncftpbatch, argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in installed as %s?\n", argv[0], ncftpbatch);
#else	/* BINDIR */
		(void) execvp(argv[0], argv);
		(void) fprintf(stderr, "Could not run %s.  Is it in your $PATH?\n", argv[0]);
#endif	/* BINDIR */
		perror(argv[0]);
		exit(1);
	}
	(void) close(pfd[0]);	/* Parent closes read end. */
	(void) PWrite(pfd[1], (const char *) cip->lip, sizeof(FTPLibraryInfo));
	(void) PWrite(pfd[1], (const char *) cip, sizeof(FTPConnectionInfo));
	(void) close(pfd[1]);	/* Parent closes read end. */

	/* Close it now, or else this process would send
	 * the server a QUIT message.  This will cause it
	 * to think it already has.
	 */
	CloseControlConnection(cip);

	if (pid > 1) {
#ifdef HAVE_WAITPID
		(void) waitpid(pid, NULL, 0);
#else
		(void) wait(NULL);
#endif	/* HAVE_WAITPID */
	}
#endif	/* UNIX */
}	/* RunBatchWithCore */



void
RunBatchIfNeeded(const FTPCIPtr cip)
{
	if (gUnprocessedJobs > 0) {
#ifdef ncftp
		Trace(0, "Running ncftp_batch for %d job%s.\n", gUnprocessedJobs, gUnprocessedJobs > 0 ? "s" : "");
		gUnprocessedJobs = 0;
		RunBatchWithCore(cip);
#else
		gUnprocessedJobs = 0;
		RunBatch();
#endif
	}
}	/* RunBatchIfNeeded */

#endif	/* HAVE_LONG_FILE_NAMES */
