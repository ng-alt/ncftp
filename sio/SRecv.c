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
SRecv(int sfd, char *const buf0, size_t size, int fl, int tlen, int retry)
{
	recv_return_t nread;
	volatile recv_size_t nleft;
	char *volatile buf = buf0;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nread = (recv_return_t) size - (recv_return_t) nleft;
		if ((nread > 0) && (retry == kFullBufferNotRequired))
			return ((int) nread);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	if (SSetjmp(gPipeJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nread = (recv_return_t) size - (recv_return_t) nleft;
		if ((nread > 0) && (retry == kFullBufferNotRequired))
			return ((int) nread);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);
	errno = 0;

	nleft = (recv_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done > now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nread = (recv_return_t) size - (recv_return_t) nleft;
			if ((nread == 0) || (retry == kFullBufferRequired)) {
				nread = kTimeoutErr;
				errno = ETIMEDOUT;
			}
			goto done;
		}
		(void) alarm(tleft);
		nread = recv(sfd, (char *) buf, nleft, fl);
		(void) alarm(0);
		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				if (retry == kFullBufferRequiredExceptLast)
					nread = (recv_return_t) size - (recv_return_t) nleft;
				goto done;
			} else if (errno != EINTR) {
				nread = (recv_return_t) size - (recv_return_t) nleft;
				if (nread == 0)
					nread = (recv_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nread = 0;
				/* Try again. */
			}
		}
		nleft -= (recv_size_t) nread;
		if ((nleft == 0) || ((retry == 0) && (nleft != (recv_size_t) size)))
			break;
		buf += nread;
		time(&now);
	}
	nread = (recv_return_t) size - (recv_return_t) nleft;

done:
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nread);
}	/* SRecv */

#else

int
SRecv(int sfd, char *const buf0, size_t size, int fl, int tlen, int retry)
{
	recv_return_t nread;
	recv_size_t nleft;
	char *buf = buf0;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;

	errno = 0;

	nleft = (recv_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done > now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nread = (recv_return_t) size - (recv_return_t) nleft;
			if ((nread == 0) || (retry == kFullBufferRequired)) {
				nread = kTimeoutErr;
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
			}
			goto done;
		}

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
			result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, SELECT_TYPE_ARG5 &tv);
			if (result == 1) {
				/* ready */
				break;
			} else if (result == 0) {
				/* timeout */		
				nread = (recv_return_t) size - (recv_return_t) nleft;
				if ((nread > 0) && (retry == kFullBufferNotRequired))
					return ((int) nread);
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			} else if (errno != EINTR) {
				return (-1);
			}
		}

		nread = recv(sfd, (char *) buf, nleft, fl);

		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				if (retry == kFullBufferRequiredExceptLast)
					nread = (recv_return_t) size - (recv_return_t) nleft;
				goto done;
			} else if (errno != EINTR) {
				nread = (recv_return_t) size - (recv_return_t) nleft;
				if (nread == 0)
					nread = (recv_return_t) -1;
				goto done;
			} else {
				errno = 0;
				nread = 0;
				/* Try again. */
			}
		}
		nleft -= (recv_size_t) nread;
		if ((nleft == 0) || ((retry == 0) && (nleft != (recv_size_t) size)))
			break;
		buf += nread;
		time(&now);
	}
	nread = (recv_return_t) size - (recv_return_t) nleft;

done:
	return ((int) nread);
}	/* SRecv */

#endif

