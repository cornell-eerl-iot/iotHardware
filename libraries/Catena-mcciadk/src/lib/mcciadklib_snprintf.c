/* mcciadklib_snprintf.c	Thu Mar 23 2017 01:13:11 tmm */

/*

Module:  mcciadklib_snprintf.c

Function:
	McciAdkLib_Snprintf()

Version:
	V0.1.1	Thu Mar 23 2017 01:13:11 tmm	Edit level 1

Copyright notice:
	This file copyright (C) 2017 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	An unpublished work.  All rights reserved.
	
	This file is proprietary information, and may not be disclosed or
	copied without the prior permission of MCCI Corporation.
 
Author:
	Terry Moore, MCCI Corporation	March 2017

Revision history:
   0.1.1  Thu Mar 23 2017 01:13:11  tmm
	Module created.

*/

#include "mcciadk_baselib.h"

#include <stdio.h>

/****************************************************************************\
|
|		Manifest constants & typedefs.
|
|	This is strictly for private types and constants which will not 
|	be exported.
|
\****************************************************************************/



/****************************************************************************\
|
|	Read-only data.
|
|	If program is to be ROM-able, these must all be tagged read-only 
|	using the ROM storage class; they may be global.
|
\****************************************************************************/



/****************************************************************************\
|
|	VARIABLES:
|
|	If program is to be ROM-able, these must be initialized
|	using the BSS keyword.  (This allows for compilers that require
|	every variable to have an initializer.)  Note that only those 
|	variables owned by this module should be declared here, using the BSS
|	keyword; this allows for linkers that dislike multiple declarations
|	of objects.
|
\****************************************************************************/


/*

Name:	McciAdkLib_Snprintf()

Index:	Name:	McciAdkLib_Vsnprintf()

Function:
	Safer, more flexible form of snprintf().

Definition:
	size_t McciAdkLib_Snprintf(
		char *pOutbuf,
		size_t nOutbuf,
		size_t iOutbuf,
		const char *pFmt,
		...
		);

	size_t McciAdkLib_Vsnprintf(
		char *pOutbuf,
		size_t nOutbuf,
		size_t iOutbuf,
		const char *pFmt,
		va_list ap
		);

Description:
	These functions call snprintf() so that the resulting string is
	formatted starting at offset iOutbuf in the buffer at pOutbuf, 
	ensuring that writes never go outsize pOutbuf + [0..nOutbuf-1],
	and ensuring that the resulting string is nul-terminated. (This
	means that the actual length can never be more than nOutbuf-1,
	as pOutbuf[nOutbuf - 1] will be forced to be the terminator if
	needed.)

Returns:
	The resulting index.

*/

size_t
McciAdkLib_Snprintf(
	char *pOutbuf,
	size_t nOutbuf,
	size_t iOutbuf,
	const char *pFmt,
	...
	)
	{
	va_list ap;
	size_t nResult;

	va_start(ap, pFmt);
	nResult = McciAdkLib_Vsnprintf(pOutbuf, nOutbuf, iOutbuf, pFmt, ap);
	va_end(ap);

	return nResult;
	}

size_t
McciAdkLib_Vsnprintf(
	char *pOutbuf,
	size_t nOutbuf,
	size_t iOutbuf,
	const char *pFmt,
	va_list ap
	)
	{
	size_t nSprintf;

	if (iOutbuf >= nOutbuf || pOutbuf == NULL)
		return nOutbuf;

	nSprintf = vsnprintf(
			pOutbuf + iOutbuf,
			nOutbuf - iOutbuf,
			pFmt,
			ap
			);

	if (nSprintf >= nOutbuf - iOutbuf)
		{
		pOutbuf[nOutbuf - 1] = '\0';
		nSprintf = nOutbuf - iOutbuf - 1;
		}
	
	return nSprintf;
	}
