/* open.c
 *
 * Copyright (c) 1996-2001 Mike Gleason, NCEMRSoft.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
FTPDeallocateHost(const FTPCIPtr cip)
{
	/* Requires the cip->bufSize field set,
	 * and the cip->buf set if the
	 * buffer is allocated.
	 */
	if (cip->buf != NULL) {
		(void) memset(cip->buf, 0, cip->bufSize);
		if (cip->doAllocBuf != 0) {
			free(cip->buf);
			cip->buf = NULL;
		}
	}

	if (cip->startingWorkingDirectory != NULL) {
		free(cip->startingWorkingDirectory);
		cip->startingWorkingDirectory = NULL;
	}

#if USE_SIO
	DisposeSReadlineInfo(&cip->ctrlSrl);
#endif
	DisposeLineListContents(&cip->lastFTPCmdResultLL);
}	/* FTPDeallocateHost */




int
FTPAllocateHost(const FTPCIPtr cip)
{
	char *buf;

	/* Requires the cip->bufSize field set,
	 * and the cip->buf cleared if the
	 * buffer is not allocated.
	 */
	if (cip->buf == NULL) {
		if (cip->doAllocBuf == 0) {
			/* User must supply buffer! */
			cip->errNo = kErrBadParameter;
			return (kErrBadParameter);
		} else {
			buf = (char *) calloc((size_t) 1, cip->bufSize);
			if (buf == NULL) {
				Error(cip, kDontPerror, "Malloc failed.\n");
				cip->errNo = kErrMallocFailed;
				return (kErrMallocFailed);
			}
			cip->buf = buf;
		}
	} else {
		(void) memset(cip->buf, 0, cip->bufSize);
	}
	return (kNoErr);
}	/* FTPAllocateHost */




void
FTPInitializeAnonPassword(const FTPLIPtr lip)
{
	if (lip == NULL)
		return;
	if (strcmp(lip->magic, kLibraryMagic))
		return;

	if (lip->defaultAnonPassword[0] == '\0')
		(void) STRNCPY(lip->defaultAnonPassword, "NcFTP@");
}	/* FTPInitializeAnonPassword */




int
FTPLoginHost(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result = kErrLoginFailed;
	int anonLogin;
	int sentpass = 0;
	int fwloggedin;
	int firstTime;
	char cwd[512];

	if (cip == NULL)
		return (kErrBadParameter);
	if ((cip->firewallType < kFirewallNotInUse) || (cip->firewallType > kFirewallLastType))
		return (kErrBadParameter);

	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	anonLogin = 0;
	if (cip->user[0] == '\0')
		(void) STRNCPY(cip->user, "anonymous");
	if ((strcmp(cip->user, "anonymous") == 0) || (strcmp(cip->user, "ftp") == 0)) {
		anonLogin = 1;
		/* Try to get the email address if you didn't specify
		 * a password when the user is anonymous.
		 */
		if (cip->pass[0] == '\0') {
			FTPInitializeAnonPassword(cip->lip);
			(void) STRNCPY(cip->pass, cip->lip->defaultAnonPassword);
		}
	}

	rp = InitResponse();
	if (rp == NULL) {
		result = kErrMallocFailed;
		cip->errNo = kErrMallocFailed;
		goto done2;
	}

	for (firstTime = 1, fwloggedin = 0; ; ) {
		/* Here's a mini finite-automaton for the login process.
		 *
		 * Originally, the FTP protocol was designed to be entirely
		 * implementable from a FA.  It could be done, but I don't think
		 * it's something an interactive process could be the most
		 * effective with.
		 */

		if (firstTime != 0) {
			rp->code = 220;
			firstTime = 0;
		} else if (result < 0) {
			goto done;
		}

		switch (rp->code) {
			case 220:	/* Welcome, ready for new user. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0)) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->user);
				} else if (cip->firewallType == kFirewallUserAtSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->user, cip->host);
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s@%s", cip->user, cip->firewallUser, cip->host);
				} else if (cip->firewallType == kFirewallUserAtSiteFwuPassFwp) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s %s", cip->user, cip->host, cip->firewallUser);
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->firewallUser, cip->host);
				} else if (cip->firewallType > kFirewallNotInUse) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->firewallUser);
				} else {
					goto unknown;
				}
				break;

			case 230:	/* 230 User logged in, proceed. */
			case 231:	/* User name accepted. */
			case 202:	/* Command not implemented, superfluous at this site. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0))
					goto okay;

				/* Now logged in to the firewall. */
				fwloggedin++;

				if (cip->firewallType == kFirewallLoginThenUserAtSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->user, cip->host);
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					goto okay;
				} else if (cip->firewallType == kFirewallOpenSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "OPEN %s", cip->host);
				} else if (cip->firewallType == kFirewallSiteSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "SITE %s", cip->host);
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->user);
				} else /* kFirewallUserAtSite */ {
					goto okay;
				}
				break;

			case 421:	/* 421 Service not available, closing control connection. */
				result = kErrHostDisconnectedDuringLogin;
				goto done;
				
			case 331:	/* 331 User name okay, need password. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0)) {
					if ((cip->pass[0] == '\0') && (cip->passphraseProc != kNoFTPGetPassphraseProc))
						(*cip->passphraseProc)(cip, &rp->msg, cip->pass, sizeof(cip->pass));
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
				} else if (cip->firewallType == kFirewallUserAtSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s@%s", cip->pass, cip->firewallPass);
				} else if (cip->firewallType == kFirewallUserAtSiteFwuPassFwp) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->firewallPass);
				} else if (cip->firewallType > kFirewallNotInUse) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->firewallPass);
				} else {
					goto unknown;
				}
				sentpass++;
				break;

			case 332:	/* 332 Need account for login. */
			case 532: 	/* 532 Need account for storing files. */
				ReInitResponse(cip, rp);
				result = RCmd(cip, rp, "ACCT %s", cip->acct);
				break;

			case 530:	/* Not logged in. */
				result = (sentpass != 0) ? kErrBadRemoteUserOrPassword : kErrBadRemoteUser;
				goto done;

			case 501:	/* Syntax error in parameters or arguments. */
			case 503:	/* Bad sequence of commands. */
			case 550:	/* Can't set guest privileges. */
				goto done;
				
			default:
			unknown:
				if (rp->msg.first == NULL) {
					Error(cip, kDontPerror, "Lost connection during login.\n");
				} else {
					Error(cip, kDontPerror, "Unexpected response: %s\n",
						rp->msg.first->line
					);
				}
				goto done;
		}
	}

okay:
	/* Do the application's connect message callback, if present. */
	if (cip->onLoginMsgProc != 0)
		(*cip->onLoginMsgProc)(cip, rp);
	DoneWithResponse(cip, rp);
	result = 0;
	cip->loggedIn = 1;

	/* Make a note of what our root directory is.
	 * This is often different from "/" when not
	 * logged in anonymously.
	 */
	if (cip->startingWorkingDirectory != NULL) {
		free(cip->startingWorkingDirectory);
		cip->startingWorkingDirectory = NULL;
	}
	if ((cip->doNotGetStartingWorkingDirectory == 0) &&
		(FTPGetCWD(cip, cwd, sizeof(cwd)) == kNoErr))
	{
		cip->startingWorkingDirectory = StrDup(cwd);
	}

	/* When a new site is opened, ASCII mode is assumed (by protocol). */
	cip->curTransferType = 'A';
	PrintF(cip, "Logged in to %s as %s.\n", cip->host, cip->user);

	/* Don't leave cleartext password in memory. */
	if ((anonLogin == 0) && (cip->leavePass == 0))
		(void) memset(cip->pass, '*', strlen(cip->pass));

	if (result < 0)
		cip->errNo = result;
	return result;

done:
	DoneWithResponse(cip, rp);

done2:
	/* Don't leave cleartext password in memory. */
	if ((anonLogin == 0) && (cip->leavePass == 0))
		(void) memset(cip->pass, '*', strlen(cip->pass));
	if (result < 0)
		cip->errNo = result;
	return result;
}	/* FTPLoginHost */




static void
FTPExamineMlstFeatures(const FTPCIPtr cip, const char *features)
{
	char buf[256], *feat;
	int flags;
	char *ctext;

	flags = 0;
	STRNCPY(buf, features);
	ctext = NULL;
	feat = strtokc(buf, ";*", &ctext);
	while (feat != NULL) {
		if (ISTRNEQ(feat, "OS.", 3))
			feat += 3;
		if (ISTREQ(feat, "type")) {
			flags |= kMlsOptType;
		} else if (ISTREQ(feat, "size")) {
			flags |= kMlsOptSize;
		} else if (ISTREQ(feat, "modify")) {
			flags |= kMlsOptModify;
		} else if (ISTREQ(feat, "UNIX.mode")) {
			flags |= kMlsOptUNIXmode;
		} else if (ISTREQ(feat, "UNIX.owner")) {
			flags |= kMlsOptUNIXowner;
		} else if (ISTREQ(feat, "UNIX.group")) {
			flags |= kMlsOptUNIXgroup;
		} else if (ISTREQ(feat, "perm")) {
			flags |= kMlsOptPerm;
		} else if (ISTREQ(feat, "UNIX.uid")) {
			flags |= kMlsOptUNIXuid;
		} else if (ISTREQ(feat, "UNIX.gid")) {
			flags |= kMlsOptUNIXgid;
		} else if (ISTREQ(feat, "UNIX.gid")) {
			flags |= kMlsOptUnique;
		}
		feat = strtokc(NULL, ";*", &ctext);
	}

	cip->mlsFeatures = flags;
}	/* FTPExamineMlstFeatures */




int
FTPQueryFeatures(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result;
	LinePtr lp;
	char *cp, *p;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->serverType == kServerTypeNetWareFTP) {
		/* NetWare 5.00 server freaks out when
		 * you give it a command it doesn't
		 * recognize, so cheat here and return.
		 */
		cip->hasPASV = kCommandAvailable;
		cip->hasSIZE = kCommandNotAvailable;
		cip->hasMDTM = kCommandNotAvailable;
		cip->hasREST = kCommandNotAvailable;
		cip->NLSTfileParamWorks = kCommandAvailable;
		cip->hasUTIME = kCommandNotAvailable;
		cip->hasCLNT = kCommandNotAvailable;
		cip->hasMLST = kCommandNotAvailable;
		cip->hasMLSD = kCommandNotAvailable;
		return (kNoErr);
	}

	rp = InitResponse();
	if (rp == NULL) {
		cip->errNo = kErrMallocFailed;
		result = cip->errNo;
	} else {
		rp->printMode = (kResponseNoPrint|kResponseNoSave);
		result = RCmd(cip, rp, "FEAT");
		if (result < kNoErr) {
			DoneWithResponse(cip, rp);
			return (result);
		} else if (result != 2) {
			/* We cheat here and pre-populate some
			 * fields when the server is wu-ftpd.
			 * This server is very common and we
			 * know it has always had these.
			 */
			 if (cip->serverType == kServerTypeWuFTPd) {
				cip->hasPASV = kCommandAvailable;
				cip->hasSIZE = kCommandAvailable;
				cip->hasMDTM = kCommandAvailable;
				cip->hasREST = kCommandAvailable;
				cip->NLSTfileParamWorks = kCommandAvailable;
			} else if (cip->serverType == kServerTypeNcFTPd) {
				cip->hasPASV = kCommandAvailable;
				cip->hasSIZE = kCommandAvailable;
				cip->hasMDTM = kCommandAvailable;
				cip->hasREST = kCommandAvailable;
				cip->NLSTfileParamWorks = kCommandAvailable;
			}

			/* Newer commands are only shown in FEAT,
			 * so we don't have to do the "try it,
			 * then save that it didn't work" thing.
			 */
			cip->hasMLST = kCommandNotAvailable;
			cip->hasMLSD = kCommandNotAvailable;
		} else {
			cip->hasFEAT = kCommandAvailable;

			for (lp = rp->msg.first; lp != NULL; lp = lp->next) {
				/* If first character was not a space it is
				 * either:
				 *
				 * (a) The header line in the response;
				 * (b) The trailer line in the response;
				 * (c) A protocol violation.
				 */
				cp = lp->line;
				if (*cp++ != ' ')
					continue;
				if (ISTRNCMP(cp, "PASV", 4) == 0) {
					cip->hasPASV = kCommandAvailable;
				} else if (ISTRNCMP(cp, "SIZE", 4) == 0) {
					cip->hasSIZE = kCommandAvailable;
				} else if (ISTRNCMP(cp, "MDTM", 4) == 0) {
					cip->hasMDTM = kCommandAvailable;
				} else if (ISTRNCMP(cp, "REST", 4) == 0) {
					cip->hasREST = kCommandAvailable;
				} else if (ISTRNCMP(cp, "UTIME", 5) == 0) {
					cip->hasUTIME = kCommandAvailable;
				} else if (ISTRNCMP(cp, "MLST", 4) == 0) {
					cip->hasMLST = kCommandAvailable;
					cip->hasMLSD = kCommandAvailable;
					FTPExamineMlstFeatures(cip, cp + 5);
				} else if (ISTRNCMP(cp, "CLNT", 4) == 0) {
					cip->hasCLNT = kCommandAvailable;
				} else if (ISTRNCMP(cp, "Compliance Level: ", 18) == 0) {
					/* Probably only NcFTPd will ever implement this.
					 * But we use it internally to differentiate
					 * between different NcFTPd implementations of
					 * IETF extensions.
					 */
					cip->ietfCompatLevel = atoi(cp + 18);
				}
			}
		}

		ReInitResponse(cip, rp);
		result = RCmd(cip, rp, "HELP SITE");
		if (result == 2) {
			for (lp = rp->msg.first; lp != NULL; lp = lp->next) {
				cp = lp->line;
				if (strstr(cp, "RETRBUFSIZE") != NULL)
					cip->hasRETRBUFSIZE = kCommandAvailable;
				if (strstr(cp, "RBUFSZ") != NULL)
					cip->hasRBUFSZ = kCommandAvailable;
				/* See if RBUFSIZ matches (but not STORBUFSIZE) */
				if (
					((p = strstr(cp, "RBUFSIZ")) != NULL) &&
					(
					 	(p == cp) ||
						((p > cp) && (!isupper((int) p[-1])))
					)
				)
					cip->hasRBUFSIZ = kCommandAvailable;
				if (strstr(cp, "STORBUFSIZE") != NULL)
					cip->hasSTORBUFSIZE = kCommandAvailable;
				if (strstr(cp, "SBUFSIZ") != NULL)
					cip->hasSBUFSIZ = kCommandAvailable;
				if (strstr(cp, "SBUFSZ") != NULL)
					cip->hasSBUFSZ = kCommandAvailable;
				if (strstr(cp, "BUFSIZE") != NULL)
					cip->hasBUFSIZE = kCommandAvailable;
			}
		}
		DoneWithResponse(cip, rp);
	}

	return (kNoErr);
}	/* FTPQueryFeatures */



int
FTPCloseHost(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	/* Data connection shouldn't be open normally. */
	if (cip->dataSocket != kClosedFileDescriptor)
		FTPAbortDataTransfer(cip);

	result = kNoErr;
	if (cip->connected != 0) {
		rp = InitResponse();
		if (rp == NULL) {
			cip->errNo = kErrMallocFailed;
			result = cip->errNo;
		} else {
			rp->eofOkay = 1;	/* We are expecting EOF after this cmd. */
			cip->eofOkay = 1;
			(void) RCmd(cip, rp, "QUIT");
			DoneWithResponse(cip, rp);
		}
	}
	
	CloseControlConnection(cip);

	/* Dispose dynamic data structures, so you won't leak
	 * if you OpenHost with this again.
	 */
	FTPDeallocateHost(cip);
	return (result);
}	/* FTPCloseHost */




int
FTPOpenHost(const FTPCIPtr cip)
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

			result = FTPLoginHost(cip);
			if (result == kNoErr) {
				(void) FTPQueryFeatures(cip);
				break;
			}

			/* Close and try again. */
			(void) FTPCloseHost(cip);

			/* Originally we also stopped retyring if
			 * we got kErrBadRemoteUser and non-anonymous,
			 * but some FTP servers apparently do their
			 * max user check after the username is sent.
			 */
			if (result == kErrBadRemoteUserOrPassword /* || (result == kErrBadRemoteUser) */) {
				if (strcmp(cip->user, "anonymous") != 0) {
					/* Non-anonymous login was denied, and
					 * retrying is not going to help.
					 */
					break;
				}
			}
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
}	/* FTPOpenHost */




int
FTPInitConnectionInfo2(const FTPLIPtr lip, const FTPCIPtr cip, char *const buf, size_t bufSize)
{
	size_t siz;

	if ((lip == NULL) || (cip == NULL) || (bufSize == 0))
		return (kErrBadParameter);

	siz = sizeof(FTPConnectionInfo);
	(void) memset(cip, 0, siz);

	if (strcmp(lip->magic, kLibraryMagic))
		return (kErrBadMagic);

	cip->bufSize = bufSize;
	if (buf == NULL) {
		/* We're responsible for allocating the buffer. */
		cip->buf = NULL;	/* denote that it needs to be allocated. */
		cip->doAllocBuf = 1;
	} else {
		/* The application is passing us a buffer to use. */
		cip->buf = buf;
		cip->doAllocBuf = 0;
	}
	cip->port = lip->defaultPort;
	cip->firewallPort = lip->defaultPort;
	cip->maxDials = kDefaultMaxDials;
	cip->redialDelay = kDefaultRedialDelay;
	cip->xferTimeout = kDefaultXferTimeout;
	cip->connTimeout = kDefaultConnTimeout;
	cip->ctrlTimeout = kDefaultCtrlTimeout;
	cip->abortTimeout = kDefaultAbortTimeout;
	cip->ctrlSocketR = kClosedFileDescriptor;
	cip->ctrlSocketW = kClosedFileDescriptor;
	cip->dataPortMode = kFallBackToSendPortMode;
	cip->dataSocket = kClosedFileDescriptor;
	cip->lip = lip;
	cip->hasPASV = kCommandAvailabilityUnknown;
	cip->hasSIZE = kCommandAvailabilityUnknown;
	cip->hasMDTM = kCommandAvailabilityUnknown;
	cip->hasREST = kCommandAvailabilityUnknown;
	cip->hasNLST_d = kCommandAvailabilityUnknown;
	cip->hasUTIME = kCommandAvailabilityUnknown;
	cip->hasFEAT = kCommandAvailabilityUnknown;
	cip->hasMLSD = kCommandAvailabilityUnknown;
	cip->hasMLST = kCommandAvailabilityUnknown;
	cip->hasCLNT = kCommandAvailabilityUnknown;
	cip->hasRETRBUFSIZE = kCommandAvailabilityUnknown;
	cip->hasRBUFSIZ = kCommandAvailabilityUnknown;
	cip->hasRBUFSZ = kCommandAvailabilityUnknown;
	cip->hasSTORBUFSIZE = kCommandAvailabilityUnknown;
	cip->hasSBUFSIZ = kCommandAvailabilityUnknown;
	cip->hasSBUFSZ = kCommandAvailabilityUnknown;
	cip->STATfileParamWorks = kCommandAvailabilityUnknown;
	cip->NLSTfileParamWorks = kCommandAvailabilityUnknown;
	cip->firewallType = kFirewallNotInUse;
	cip->startingWorkingDirectory = NULL;
	(void) STRNCPY(cip->magic, kLibraryMagic);
	(void) STRNCPY(cip->user, "anonymous");
	return (kNoErr);
}	/* FTPInitConnectionInfo2 */




int
FTPInitConnectionInfo(const FTPLIPtr lip, const FTPCIPtr cip, size_t bufSize)
{
	return (FTPInitConnectionInfo2(lip, cip, NULL, bufSize));
}	/* FTPInitConnectionInfo */




int
FTPInitLibrary(const FTPLIPtr lip)
{
	if (lip == NULL)
		return (kErrBadParameter);

	(void) memset(lip, 0, sizeof(FTPLibraryInfo));

	lip->defaultPort = ServiceNameToPortNumber("ftp", 't');
	if (lip->defaultPort == 0)
		lip->defaultPort = (unsigned int) kDefaultFTPPort;

	lip->init = 1;
	(void) STRNCPY(lip->magic, kLibraryMagic);

	/* We'll initialize the defaultAnonPassword field
	 * later when we try the first anon ftp connection.
	 */

#ifdef HAVE_LIBSOCKS
	SOCKSinit("libncftp");
	lip->socksInit = 1;
#endif
	return (kNoErr);
}	/* FTPInitLibrary */

/* Open.c */
