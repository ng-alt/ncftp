/* c_opennologin.c
 *
 * Copyright (c) 2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
FTPOpenHostNoLogin(const FTPCIPtr cip)
{
	int result;
	time_t t0, t1;
	int elapsed;
	int dials;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->host[0] == '\0') {
		cip->errNo = kErrBadParameter;
		return (kErrBadParameter);
	}

	for (	result = kErrConnectMiscErr, dials = 0;
		cip->maxDials < 0 || dials < cip->maxDials;
		dials++)
	{

		/* Allocate (or if the host was closed, reallocate)
		 * the transfer data buffer.
		 */
		result = FTPAllocateHost(cip);
		if (result < 0)
			return (result);

		if (dials > 0)
			PrintF(cip, "Retry Number: %d\n", dials);
		if (cip->redialStatusProc != 0)
			(*cip->redialStatusProc)(cip, kRedialStatusDialing, dials);
		(void) time(&t0);
		result = OpenControlConnection(cip, cip->host, cip->port);
		(void) time(&t1);
		if (result == kNoErr) {
			/* We were hooked up successfully. */
			PrintF(cip, "Connected to %s.\n", cip->host);

			/* Not logging in... */
			if (result == kNoErr)
				break;
		} else if ((result != kErrConnectRetryableErr) && (result != kErrConnectRefused) && (result != kErrRemoteHostClosedConnection)) {
			/* Irrecoverable error, so don't bother redialing.
			 * The error message should have already been printed
			 * from OpenControlConnection().
			 */
			PrintF(cip, "Cannot recover from miscellaneous open error %d.\n", result);
			return result;
		}

		/* Retryable error, wait and then redial. */
		if (cip->redialDelay > 0) {
			/* But don't sleep if this is the last loop. */
			if ((cip->maxDials < 0) || (dials < (cip->maxDials - 1))) {
				elapsed = (int) (t1 - t0);
				if (elapsed < cip->redialDelay) {
					PrintF(cip, "Sleeping %u seconds.\n",
						(unsigned) cip->redialDelay - elapsed);
					if (cip->redialStatusProc != 0)
						(*cip->redialStatusProc)(cip, kRedialStatusSleeping, cip->redialDelay - elapsed);
					(void) sleep((unsigned) cip->redialDelay - elapsed);
				}
			}
		}
	}
	return (result);
}	/* FTPOpenHostNoLogin */
