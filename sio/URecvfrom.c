#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS

extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;

int
URecvfrom(int sfd, char *const buf, size_t size, int fl, struct sockaddr_un *const fromAddr, int *ualen, int tlen)
{
	recv_return_t nread;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;
	sockaddr_size_t ualen2;

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
		if (tleft < 1) {
			nread = kTimeoutErr;
			errno = ETIMEDOUT;
			break;
		}
		ualen2 = sizeof(struct sockaddr_un);
		(void) alarm(tleft);
		nread = recvfrom(sfd, buf, (recv_size_t) size, fl,
			(struct sockaddr *) fromAddr, &ualen2);
		(void) alarm(0);
		*ualen = (int) ualen2;
		if (nread >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (done < now) ? ((alarm_time_t) (done - now)) : 0;
	}

	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nread);
}	/* URecvfrom */

#else

int
URecvfrom(int sfd, char *const buf, size_t size, int fl, struct sockaddr_un *const fromAddr, int *ualen, int tlen)
{
	recv_return_t nread;
	int tleft;
	fd_set ss;
	struct timeval tv;
	int result;
	time_t done, now;
	sockaddr_size_t alen;

	time(&now);
	done = now + tlen;
	tleft = (done < now) ? ((int) (done - now)) : 0;
	nread = 0;
	forever {
		alen = (sockaddr_size_t) sizeof(struct sockaddr_un);
				
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

		nread = recvfrom(sfd, buf, (recv_size_t) size, fl,
			(struct sockaddr *) fromAddr, &alen);
		*ualen = (int) alen;

		if (nread >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (done < now) ? ((int) (done - now)) : 0;
		if (tleft < 1) {
			nread = kTimeoutErr;
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			break;
		}
	}

	return ((int) nread);
}	/* URecvfrom */

#endif
