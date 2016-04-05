#include "syshdrs.h"

#ifndef NO_SIGNALS
extern volatile Sjmp_buf gNetTimeoutJmp;
extern volatile Sjmp_buf gPipeJmp;
#endif

int
SConnect(int sfd, const struct sockaddr_in *const addr, int tlen)
{
#ifndef NO_SIGNALS
	int result;
	vsio_sigproc_t sigalrm;

	if (SSetjmp(gNetTimeoutJmp) != 0) {
		alarm(0);
		(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
		errno = ETIMEDOUT;
		return (kTimeoutErr);
	}

	sigalrm = (vsio_sigproc_t) SSignal(SIGALRM, SIOHandler);
	alarm((unsigned int) tlen);

	errno = 0;
	do {
		result = connect(sfd, (struct sockaddr *) addr,
			(int) sizeof(struct sockaddr_in));
	} while ((result < 0) && (errno == EINTR));

	alarm(0);
	(void) SSignal(SIGALRM, (sio_sigproc_t) sigalrm);
	return (result);
#else	/* NO_SIGNALS */
	unsigned long opt;
	fd_set ss, xx;
	struct timeval tv;
	int result;
#if defined(WIN32) || defined(_WINDOWS)
	int wsaErrno;
	int soerr, soerrsize;
#endif;

	errno = 0;
	if (tlen <= 0) {
		do {
			result = connect(sfd, (struct sockaddr *) addr,
				(int) sizeof(struct sockaddr_in));
			SETERRNO
		} while ((result < 0) && (errno == EINTR));
		return (result);
	}

#ifdef FIONBIO
	opt = 1;
	if (ioctlsocket(sfd, FIONBIO, &opt) != 0) {
		SETERRNO
		return (-1);
	}
#else
	if (fcntl(sfd, F_GETFL, &opt) < 0) {
		SETERRNO
		return (-1);
	} else if (fcntl(sfd, F_SETFL, opt | O_NONBLOCK) < 0) {
		SETERRNO
		return (-1);
	}
#endif

	errno = 0;
	result = connect(sfd, (struct sockaddr *) addr,
			(int) sizeof(struct sockaddr_in));
	if (result == 0) 
		return 0;	/* Already?!? */

	if ((result < 0) 
#if defined(WIN32) || defined(_WINDOWS)
		&& ((wsaErrno = WSAGetLastError()) != WSAEWOULDBLOCK)
		&& (wsaErrno != WSAEINPROGRESS)
#else
		&& (errno != EWOULDBLOCK) && (errno != EINPROGRESS)
#endif
		) {
		SETERRNO
		shutdown(sfd, 2);
		return (-1);
	}

	forever {
#if defined(WIN32) || defined(_WINDOWS)
		WSASetLastError(0);
#endif
		FD_ZERO(&ss);
		FD_SET(sfd, &ss);
		xx = ss;
		tv.tv_sec = tlen;
		tv.tv_usec = 0;
		result = select(sfd + 1, NULL, SELECT_TYPE_ARG234 &ss, SELECT_TYPE_ARG234 &xx, SELECT_TYPE_ARG5 &tv);
		if (result == 1) {
			/* ready */
			break;
		} else if (result == 0) {
			/* timeout */		
			errno = ETIMEDOUT;
			SETWSATIMEOUTERR
			/* Don't bother turning off FIONBIO */
			return (kTimeoutErr);
		} else if (errno != EINTR) {
			/* Don't bother turning off FIONBIO */
			SETERRNO
			return (-1);
		}
	}

	/* Supposedly once the select() returns with a writable
	 * descriptor, it is connected and we don't need to
	 * recall connect().  When select() returns an exception,
	 * the connection failed -- we can get the connect error
	 * doing a write on the socket which will err out.
	 */

	if (FD_ISSET(sfd, &xx)) {
#if defined(WIN32) || defined(_WINDOWS)
		errno = 0;
		soerr = 0;
		soerrsize = sizeof(soerr);
		result = getsockopt(sfd, SOL_SOCKET, SO_ERROR, (char *) &soerr, &soerrsize);
		if ((result >= 0) && (soerr != 0)) {
			errno = soerr;
		} else {
			errno = 0;
			(void) send(sfd, "\0", 1, 0);
			SETERRNO
		}
#else
		errno = 0;
		(void) send(sfd, "\0", 1, 0);
#endif
		result = errno;
		shutdown(sfd, 2);
		errno = result;
		return (-1);
	}

#ifdef FIONBIO
	opt = 0;
	if (ioctlsocket(sfd, FIONBIO, &opt) != 0) {
		SETERRNO
		shutdown(sfd, 2);
		return (-1);
	}
#else
	if (fcntl(sfd, F_SETFL, opt) < 0) {
		SETERRNO
		shutdown(sfd, 2);
		return (-1);
	}
#endif

	return (0);
#endif	/* NO_SIGNALS */
}	/* SConnect */
