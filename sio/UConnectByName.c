#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
UConnectByName(int sfd, const char * const addrStr, const int tlen)
{
	int result;
	struct sockaddr_un remoteAddr;
	int ualen;

	ualen = MakeSockAddrUn(&remoteAddr, addrStr);
	result = UConnect(sfd, &remoteAddr, ualen, tlen);
	return (result);
}	/* UConnectByName */
