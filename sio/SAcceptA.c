#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

int
SAcceptA(int sfd, struct sockaddr_in *const addr, int tlen)
{
	int result;
#ifndef NO_SIGNALS
	vsio_sigproc_t sigalrm, sigpipe;
#endif
	sockaddr_size_t size;

	if (tlen < 0) {
		errno = 0;
		for (;;) {
			size = (sockaddr_size_t) sizeof(struct sockaddr_in);
			result = accept(sfd, (struct sockaddr *) addr, &size);
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
	alarm((alarm_time_t) tlen);

	errno = 0;
	do {
		size = (sockaddr_size_t) sizeof(struct sockaddr_in);
		result = accept(sfd, (struct sockaddr *) addr, &size);
	} while ((result < 0) && (errno == EINTR));

	alarm(0);
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
	return (result);
#else
	return (-1);
#endif
}	/* SAcceptA */
