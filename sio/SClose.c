#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef NO_SIGNALS
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

int
SClose(int sfd, int tlen)
{
#ifndef NO_SIGNALS
	vsio_sigproc_t sigalrm, sigpipe;

	if (sfd < 0) {
		errno = EBADF;
		return (-1);
	}

	if (tlen < 1) {
		/* Don't time it, shut it down now. */
		if (SetSocketLinger(sfd, 0, 0) == 0) {
			/* Linger disabled, so close()
			 * should not block.
			 */
			return (closesocket(sfd));
		} else {
			/* This will result in a fd leak,
			 * but it's either that or hang forever.
			 */
			(void) shutdown(sfd, 2);
			return (closesocket(sfd));
		}
	}

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		if (SetSocketLinger(sfd, 0, 0) == 0) {
			/* Linger disabled, so close()
			 * should not block.
			 */
			return closesocket(sfd);
		} else {
			/* This will result in a fd leak,
			 * but it's either that or hang forever.
			 */
			(void) shutdown(sfd, 2);
			return (closesocket(sfd));
		}
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIG_IGN);

	alarm((alarm_time_t) tlen);
	for (;;) {
		if (closesocket(sfd) == 0) {
			errno = 0;
			break;
		}
		if (errno != EINTR)
			break;
	} 
	alarm(0);
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);

	if ((errno != 0) && (errno != EBADF)) {
		if (SetSocketLinger(sfd, 0, 0) == 0) {
			/* Linger disabled, so close()
			 * should not block.
			 */
			(void) closesocket(sfd);
		} else {
			/* This will result in a fd leak,
			 * but it's either that or hang forever.
			 */
			(void) shutdown(sfd, 2);
			(void) closesocket(sfd);
		}
	}
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((errno == 0) ? 0 : (-1));
#else
	struct timeval tv;
	int result;
	time_t done, now;
	fd_set ss, ss2;

	if (sfd < 0) {
		errno = EBADF;
		return (-1);
	}

	if (tlen < 1) {
		/* Don't time it, shut it down now. */
		if (SetSocketLinger(sfd, 0, 0) == 0) {
			/* Linger disabled, so close()
			 * should not block.
			 */
			return (closesocket(sfd));
		} else {
			/* This will result in a fd leak,
			 * but it's either that or hang forever.
			 */
			(void) shutdown(sfd, 2);
			return (closesocket(sfd));
		}
	}

	/* Wait until the socket is ready for writing (usually easy). */
	time(&now);
	done = now + tlen;

	forever {
		tlen = done - now;
		if (tlen <= 0) {
			/* timeout */
			if (SetSocketLinger(sfd, 0, 0) == 0) {
				/* Linger disabled, so close()
				 * should not block.
				 */
				(void) closesocket(sfd);
			} else {
				/* This will result in a fd leak,
				 * but it's either that or hang forever.
				 */
				(void) shutdown(sfd, 2);
				(void) closesocket(sfd);
			}
			errno = ETIMEDOUT;
			return (kTimeoutErr);
		}

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
		memcpy(&ss2, &ss, sizeof(ss2));
		tv.tv_sec = (tv_sec_t) tlen;
		tv.tv_usec = 0;
		result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &ss2, SELECT_TYPE_ARG5 &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */
			if (SetSocketLinger(sfd, 0, 0) == 0) {
				/* Linger disabled, so close()
				 * should not block.
				 */
				(void) closesocket(sfd);
			} else {
				/* This will result in a fd leak,
				 * but it's either that or hang forever.
				 */
				(void) shutdown(sfd, 2);
				(void) closesocket(sfd);
			}
			errno = ETIMEDOUT;
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			/* Error, done. This end may have been shutdown. */
			break;
		}
		time(&now);
	}

	/* Wait until the socket is ready for reading. */
	forever {
		tlen = done - now;
		if (tlen <= 0) {
			/* timeout */
			if (SetSocketLinger(sfd, 0, 0) == 0) {
				/* Linger disabled, so close()
				 * should not block.
				 */
				(void) closesocket(sfd);
			} else {
				/* This will result in a fd leak,
				 * but it's either that or hang forever.
				 */
				(void) shutdown(sfd, 2);
				(void) closesocket(sfd);
			}
			errno = ETIMEDOUT;
			return (kTimeoutErr);
		}

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
		memcpy(&ss2, &ss, sizeof(ss2));
		result = select(sfd + 1, SELECT_TYPE_ARG234 &ss, NULL, SELECT_TYPE_ARG234 &ss2, SELECT_TYPE_ARG5 &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */
			if (SetSocketLinger(sfd, 0, 0) == 0) {
				/* Linger disabled, so close()
				 * should not block.
				 */
				(void) closesocket(sfd);
			} else {
				(void) shutdown(sfd, 2);
				(void) closesocket(sfd);
			}
			errno = ETIMEDOUT;
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			/* Error, done. This end may have been shutdown. */
			break;
		}
		time(&now);
	}

	/* If we get here, close() won't block. */
	return closesocket(sfd);
#endif
}	/* SClose */
