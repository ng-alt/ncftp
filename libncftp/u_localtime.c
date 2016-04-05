/* u_localtime.c
 *
 * Copyright (c) 2002 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

struct tm *
Localtime(time_t t, struct tm *const tp)
{
#if defined(HAVE_LOCALTIME_R)
	if (t == 0)
		time(&t);
	if (localtime_r(&t, tp) != NULL)
		return (tp);
#else
	struct tm *tmp;

	if (t == 0)
		time(&t);
	tmp = localtime(&t);
	if (tmp != NULL) {
		memcpy(tp, tmp, sizeof(struct tm));
		return (tp);
	}
#endif
	memset(tp, 0, sizeof(struct tm));
	return NULL;
}	/* Localtime */
