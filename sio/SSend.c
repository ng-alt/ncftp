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
SSend(int sfd, char *buf0, size_t size, int fl, int tlen)
{
	char *volatile buf = buf0;
	send_return_t nwrote;
	volatile send_size_t nleft;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nwrote = (send_return_t) size - (send_return_t) nleft;
		if (nwrote > 0)
			return ((int) nwrote);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	if (SSetjmp(gPipeJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nwrote = (send_return_t) size - (send_return_t) nleft;
		if (nwrote > 0)
			return ((int) nwrote);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);

	nleft = (send_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done > now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nwrote = (send_return_t) size - (send_return_t) nleft;
			if (nwrote == 0) {
				nwrote = kTimeoutErr;
				errno = ETIMEDOUT;
			}
			goto done;
		}
		(void) alarm(tleft);
		nwrote = send(sfd, buf, nleft, fl);
		(void) alarm(0);
		if (nwrote < 0) {
			if (errno != EINTR) {
				nwrote = (send_return_t) size - (send_return_t) nleft;
				if (nwrote == 0)
					nwrote = (send_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nwrote = 0;
				/* Try again. */
			}
		}
		nleft -= (send_size_t) nwrote;
		if (nleft == 0)
			break;
		buf += nwrote;
		time(&now);
	}
	nwrote = (send_return_t) size - (send_return_t) nleft;

done:
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nwrote);
}	/* SSend */

#else

int
SSend(int sfd, char *buf0, size_t size, int fl, int tlen)
{
	char *buf = buf0;
	send_return_t nwrote;
	send_size_t nleft;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;

	nleft = (send_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done > now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nwrote = (send_return_t) size - (send_return_t) nleft;
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
				nwrote = (send_return_t) size - (send_return_t) nleft;
				if (nwrote > 0)
					return ((int) nwrote);
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			} else if (errno != EINTR) {
				return (-1);
			}
		}

		nwrote = send(sfd, buf, nleft, fl);

		if (nwrote < 0) {
			if (errno != EINTR) {
				nwrote = (send_return_t) size - (send_return_t) nleft;
				if (nwrote == 0)
					nwrote = (send_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nwrote = 0;
				/* Try again. */
			}
		}
		nleft -= (send_size_t) nwrote;
		if (nleft == 0)
			break;
		buf += nwrote;
		time(&now);
	}
	nwrote = (send_return_t) size - (send_return_t) nleft;

done:
	return ((int) nwrote);
}	/* SSend */

#endif
