#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

int
UAcceptA(int sfd, struct sockaddr_un *const addr, int *ualen, int tlen)
{
	int result;
#ifndef NO_SIGNALS
	vsio_sigproc_t sigalrm, sigpipe;
#endif
	sockaddr_size_t ualen2;

	if (tlen < 0) {
		errno = 0;
		for (;;) {
			ualen2 = (sockaddr_size_t) sizeof(struct sockaddr_un);
			result = accept(sfd, (struct sockaddr *) addr, &ualen2);
			*ualen = (int) ualen2;
			if ((result >= 0) || (errno != EINTR))
				return (result);
		}
	}

#ifndef NO_SIGNALS
	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIG_IGN);
	alarm((unsigned int) tlen);

	errno = 0;
	do {
		ualen2 = (sockaddr_size_t) sizeof(struct sockaddr_un);
		result = accept(sfd, (struct sockaddr *) addr, &ualen2);
		*ualen = (int) ualen2;
	} while ((result < 0) && (errno == EINTR));

	alarm(0);
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
	return (result);
#else
	return (-1);
#endif
}	/* UAcceptA */
