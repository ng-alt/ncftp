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
SSendto(int sfd, const char *const buf, size_t size, int fl, const struct sockaddr_in *const toAddr, int tlen)
{
	send_return_t nwrote;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

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
	tleft = (done > now) ? ((alarm_time_t) (done - now)) : 0;
	forever {
		(void) alarm(tleft);
		nwrote = sendto(sfd, buf, (send_size_t) size, fl,
			(const struct sockaddr *) toAddr,
			(sockaddr_size_t) sizeof(struct sockaddr_in));
		(void) alarm(0);
		if (nwrote >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
		errno = 0;
		time(&now);
		tleft = (done > now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nwrote = kTimeoutErr;
			errno = ETIMEDOUT;
			break;
		}
	}

	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nwrote);
}	/* SSendto */

#else

int
SSendto(int sfd, const char *const buf, size_t size, int fl, const struct sockaddr_in *const toAddr, int tlen)
{
	send_return_t nwrote;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result;

	time(&now);
	done = now + tlen;
	nwrote = 0;
	forever {
		forever {
			if (now >= done) {
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
				return (kTimeoutErr);
			}
			tleft = (done > now) ? ((int) (done - now)) : 0;
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
			result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG5 &tv);
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
			time(&now);
		}

		nwrote = sendto(sfd, buf, (send_size_t) size, fl,
			(const struct sockaddr *) toAddr,
			(sockaddr_size_t) sizeof(struct sockaddr_in));

		if (nwrote >= 0)
			break;
		if (errno != EINTR)
			break;		/* Fatal error. */
	}

	return ((int) nwrote);
}	/* SSendto */

#endif





int
Sendto(int sfd, const char *const buf, size_t size, const struct sockaddr_in *const toAddr)
{
	int result;

	do {
		result = (int) sendto(sfd, buf, (send_size_t) size, 0,
				(const struct sockaddr *) toAddr,
				(sockaddr_size_t) sizeof(struct sockaddr_in));
	} while ((result < 0) && (errno == EINTR));
	return (result);
}	/* Sendto */
