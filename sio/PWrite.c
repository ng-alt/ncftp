#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if !defined(NO_SIGNALS) && defined(SIGPIPE)
extern Sjmp_buf gPipeJmp;
#endif
int
PWrite(int sfd, const char *const buf0, size_t size)
{
	write_return_t nwrote;
#if defined(NO_SIGNALS) || !defined(SIGPIPE)
	write_size_t nleft;
	const char *buf = buf0;
#else
	volatile write_size_t nleft;
	const char *volatile buf = buf0;
	vsio_sigproc_t sigpipe;

	if (SSetjmp(gPipeJmp) != 0) {
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nwrote = (write_return_t) size - (write_return_t) nleft;
		if (nwrote > 0)
			return ((int) nwrote);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);
#endif

	nleft = (write_size_t) size;
	forever {
		nwrote = write(sfd, buf, nleft);
		if (nwrote < 0) {
			if (errno != EINTR) {
				nwrote = (write_return_t) size - (write_return_t) nleft;
				if (nwrote == 0)
					nwrote = (write_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nwrote = 0;
				/* Try again. */
			}
		}
		nleft -= (write_size_t) nwrote;
		if (nleft == 0)
			break;
		buf += nwrote;
	}
	nwrote = (write_return_t) size - (write_return_t) nleft;

done:
#if !defined(NO_SIGNALS) && defined(SIGPIPE)
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
#endif

	return ((int) nwrote);
}	/* PWrite */
