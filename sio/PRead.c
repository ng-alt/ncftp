#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if !defined(NO_SIGNALS) && defined(SIGPIPE)
extern Sjmp_buf gPipeJmp;
#endif

/* Read up to "size" bytes on sfd.
 *
 * If "retry" is on, after a successful read of less than "size"
 * bytes, it will attempt to read more, upto "size."
 *
 * Although "retry" would seem to indicate you may want to always
 * read "size" bytes or else it is an error, even with that on you
 * may get back a value < size.  Set "retry" to 0 when you want to
 * return as soon as there is a chunk of data whose size is <= "size".
 */

int
PRead(int sfd, char *const buf0, size_t size, int retry)
{
	read_return_t nread;
#if defined(NO_SIGNALS) || !defined(SIGPIPE)
	read_size_t nleft;
	char *buf = buf0;
#else
	char *volatile buf = buf0;
	volatile read_size_t nleft;
	vsio_sigproc_t sigpipe;

	if (SSetjmp(gPipeJmp) != 0) {
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nread = (read_return_t) size - (read_return_t) nleft;
		if (nread > 0)
			return ((int) nread);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);
#endif
	errno = 0;

	nleft = (read_size_t) size;
	forever {
		nread = read(sfd, buf, nleft);
		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				nread = (read_return_t) size - (read_return_t) nleft;
				goto done;
			} else if (errno != EINTR) {
				nread = (read_return_t) size - (read_return_t) nleft;
				if (nread == 0)
					nread = (read_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nread = 0;
				/* Try again. */
			}
		}
		nleft -= (read_size_t) nread;
		if ((nleft == 0) || (retry == 0))
			break;
		buf += nread;
	}
	nread = (read_return_t) size - (read_return_t) nleft;

done:
#if !defined(NO_SIGNALS) && defined(SIGPIPE)
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
#endif
	return ((int) nread);
}	/* PRead */
