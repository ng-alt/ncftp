#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

static const char UNUSED(gSioVersion[]) = "@(#) sio 6.1.2 ** Copyright 1992-2002 Mike Gleason. All rights reserved.";

#ifdef NO_SIGNALS
static const char UNUSED(gNoSignalsMarker[]) = "@(#) sio - NO_SIGNALS";
#else
extern Sjmp_buf gNetTimeoutJmp;
extern Sjmp_buf gPipeJmp;
#endif

/* Read up to "size" bytes on sfd before "tlen" seconds.
 *
 * If "retry" is on, after a successful read of less than "size"
 * bytes, it will attempt to read more, upto "size."
 *
 * If the timer elapses and one or more bytes were read, it returns
 * that number, otherwise a timeout error is returned.
 *
 * Although "retry" would seem to indicate you may want to always
 * read "size" bytes or else it is an error, even with that on you
 * may get back a value < size.  Set "retry" to 0 when you want to
 * return as soon as there is a chunk of data whose size is <= "size".
 */

#ifndef NO_SIGNALS

int
SRead(int sfd, char *const buf0, size_t size, int tlen, int retry)
{
	read_return_t nread;
	volatile read_size_t nleft;
	char *volatile buf = buf0;
	alarm_time_t tleft;
	vsio_sigproc_t sigalrm, sigpipe;
	time_t done, now;

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nread = (read_return_t) size - (read_return_t) nleft;
		if ((nread > 0) && ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0))
			return ((int) nread);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	if (SSetjmp(gPipeJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);
		nread = (read_return_t) size - (read_return_t) nleft;
		if ((nread > 0) && ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0))
			return ((int) nread);
		errno = EPIPE;
		return (kBrokenPipeErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	sigpipe = (vsio_sigproc_t) SSignal(SIGPIPE, SIOHandler);
	errno = 0;

	nleft = (read_size_t) size;
	time(&now);
	done = now + tlen;
	forever {
		tleft = (done < now) ? ((alarm_time_t) (done - now)) : 0;
		if (tleft < 1) {
			nread = (read_return_t) size - (read_return_t) nleft;
			if ((nread == 0) || ((retry & (kFullBufferRequired)) != 0)) {
				nread = kTimeoutErr;
				errno = ETIMEDOUT;
			}
			goto done;
		}
		(void) alarm(tleft);
		nread = read(sfd, (char *) buf, nleft);
		(void) alarm(0);
		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				if (retry == ((retry & (kFullBufferRequiredExceptLast)) != 0))
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

				/* Ignore this line... */
				LIBSIO_USE_VAR(gSioVersion);
			}
		}
		nleft -= (read_size_t) nread;
		if ((nleft == 0) || (((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0) && (nleft != (read_size_t) size)))
			break;
		buf += nread;
		time(&now);
	}
	nread = (read_return_t) size - (read_return_t) nleft;

done:
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	(void) SSignal(SIGPIPE, (sio_sigproc_t) sigpipe);

	return ((int) nread);
}	/* SRead */

#else

int
SRead(int sfd, char *const buf0, size_t size, int tlen, int retry)
{
	read_return_t nread;
	read_size_t nleft;
	char *buf = buf0;
	int tleft;
	time_t done, now;
	fd_set ss;
	struct timeval tv;
	int result, firstRead;

	errno = 0;

	nleft = (read_size_t) size;
	time(&now);
	done = now + tlen;
	firstRead = 1;

	forever {
		tleft = (int) (done - now);
		if (tleft < 1) {
			nread = (read_return_t) size - (read_return_t) nleft;
			if ((nread == 0) || ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) != 0)) {
				nread = kTimeoutErr;
				errno = ETIMEDOUT;
				SETWSATIMEOUTERR
			}
			goto done;
		}
		
		if (!firstRead || ((retry & kNoFirstSelect) == 0)) {
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
					nread = (read_return_t) size - (read_return_t) nleft;
					if ((nread > 0) && ((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0))
						return ((int) nread);
					errno = ETIMEDOUT;
					SETWSATIMEOUTERR
						return (kTimeoutErr);
				} else if (errno != EINTR) {
					return (-1);
				}
			}
			firstRead = 0;
		}

#if defined(WIN32) || defined(_WINDOWS)
		nread = recv(sfd, (char *) buf, (recv_size_t) nleft, 0);
#else
		nread = read(sfd, (char *) buf, nleft);
#endif

		if (nread <= 0) {
			if (nread == 0) {
				/* EOF */
				if (retry == ((retry & (kFullBufferRequiredExceptLast)) != 0))
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

				/* Ignore these two lines */
				LIBSIO_USE_VAR(gSioVersion);
				LIBSIO_USE_VAR(gNoSignalsMarker);
			}
		}
		nleft -= (read_size_t) nread;
		if ((nleft == 0) || (((retry & (kFullBufferRequired|kFullBufferRequiredExceptLast)) == 0) && (nleft != (read_size_t) size)))
			break;
		buf += nread;
		time(&now);
	}
	nread = (read_return_t) size - (read_return_t) nleft;

done:
	return ((int) nread);
}	/* SRead */

#endif
