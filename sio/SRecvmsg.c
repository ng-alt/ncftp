#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

#ifndef HAVE_RECVMSG
int
SRecvmsg(int UNUSED(sfd), void *const UNUSED(msg), int UNUSED(fl), int UNUSED(tlen))
{
	LIBSIO_USE_VAR(sfd);
	LIBSIO_USE_VAR(msg);
	LIBSIO_USE_VAR(fl);
	LIBSIO_USE_VAR(tlen);
#	ifdef ENOSYS
	errno = ENOSYS;
#	endif
	return (-1);
}

#elif !defined(NO_SIGNALS)

int
SRecvmsg(int sfd, void *const msg, int fl, int tlen)
{
	recv_return_t nread;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

	if (tlen < 0) {
		errno = 0;
		for (;;) {
			nread = recvmsg(sfd, (struct msghdr *) msg, fl);
			if ((nread >= 0) || (errno != EINTR))
				return ((int) nread);
		}
	}

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	if (SSetjmp(gPipeJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);

	time(&now);
	done = now + tlen;
	tleft = (done < now) ? ((alarm_time_t) (done - now)) : 0;
	forever {
		(void) alarm(tleft);
		nread = recvmsg(sfd, (struct msghdr *) msg, fl);
		(void) alarm(0);
		if (nread >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (done < now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nread = kTimeoutErr;
			errno = ETIMEDOUT;
			break;
		}
	}

	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nread);
}	/* SRecvmsg */

#elif defined(HAVE_RECVMSG)

int
SRecvmsg(int sfd, void *const msg, int fl, int tlen)
{
	recv_return_t nread;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;

	if (tlen < 0) {
		errno = 0;
		for (;;) {
			nread = recvmsg(sfd, (struct msghdr *) msg, fl);
			if ((nread >= 0) || (errno != EINTR))
				return ((int) nread);
		}
	}

	time(&now);
	done = now + tlen;
	tleft = (int) (done - now);
	forever {
				
		for (;;) {
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
			tv.tv_sec = (tv_sec_t) tleft;
			tv.tv_usec = 0;
			result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, SELECT_TYPE_ARG5 &tv);
			if (result == 1) {
				/* ready */
				break;
			} else if (result == 0) {
				/* timeout */
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			} else if (errno != EINTR) {
				return (-1);
			}
		}

		nread = recvmsg(sfd, (struct msghdr *) msg, fl);

		if (nread >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (int) (done - now);
		if (tleft < 1) {
			nread = kTimeoutErr;
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			break;
		}
	}

	return ((int) nread);
}	/* SRecvmsg */

#endif
