#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

#ifndef NO_SIGNALS

int
SWrite(int sfd, const char *const buf0, size_t size, int tlen, int swopts)
{
	const char *volatile buf = buf0;
	write_return_t nwrote;
	volatile write_size_t nleft;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nwrote = (write_return_t) size - (write_return_t) nleft;
		if (nwrote > 0)
			return ((int) nwrote);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	if (SSetjmp(gPipeJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nwrote = (write_return_t) size - (write_return_t) nleft;
		if (nwrote > 0)
			return ((int) nwrote);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);

	nleft = (write_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done > now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nwrote = (write_return_t) size - (write_return_t) nleft;
			if (nwrote == 0) {
				nwrote = kTimeoutErr;
				errno = ETIMEDOUT;
			}
			goto done;
		}
		(void) alarm(tleft);
		nwrote = write(sfd, buf, nleft);
		(void) alarm(0);
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
		time(&now);
	}
	nwrote = (write_return_t) size - (write_return_t) nleft;

done:
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nwrote);
}	/* SWrite */

#else

int
SWrite(int sfd, const char *const buf0, size_t size, int tlen, int swopts)
{
	const char *buf = buf0;
	write_return_t nwrote;
	write_size_t nleft;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result, firstSelect;

	nleft = (write_size_t) size;
	time(&now);
	done = now + tlen;
	firstSelect = 1;

	forever {
		tleft = (done > now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nwrote = (write_return_t) size - (write_return_t) nleft;
			if (nwrote == 0) {
				nwrote = kTimeoutErr;
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
			}
			goto done;
		}


		/* Unfortunately this doesn't help when the
		 * send buffer fills during the time we're
		 * writing to it, so you could still be
		 * blocked after breaking this loop and starting
		 * the write.
		 */
		if (!firstSelect || ((swopts & kNoFirstSelect) == 0)) {
			forever {
				errno = 0;
				MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
				MY_FD_SET(sfd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
				tv.tv_sec = (tv_sec_t) tlen;
				tv.tv_usec = 0;
				result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG5 &tv);
				if (result == 1) {
					/* ready */
					break;
				} else if (result == 0) {
					/* timeout */		
					nwrote = (write_return_t) size - (write_return_t) nleft;
					if (nwrote > 0)
						return ((int) nwrote);
					errno = ETIMEDOUT;
					SETWSATIMEOUTERR
						return (kTimeoutErr);
				} else if (errno != EINTR) {
					return (-1);
				}
			}
			firstSelect = 0;
		}
		
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
		nwrote = send(sfd, buf, (send_size_t) nleft, 0);
#else
		nwrote = write(sfd, buf, nleft);
#endif

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
		time(&now);
	}
	nwrote = (write_return_t) size - (write_return_t) nleft;

done:
	return ((int) nwrote);
}	/* SWrite */

#endif
