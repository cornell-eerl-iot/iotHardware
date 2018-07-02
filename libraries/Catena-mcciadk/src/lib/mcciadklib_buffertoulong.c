/* mcciadklib_buffertoulong.c	Wed Mar 22 2017 23:55:01 tmm */

/*

Module:  mcciadklib_buffertoulong.c

Function:
	Home for McciAdkLib_BufferToUlong()

Version:
	V0.1.1	Wed Mar 22 2017 23:55:01 tmm	Edit level 3

Copyright notice:
	This file copyright (C) 2013, 2017 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	An unpublished work.  All rights reserved.

	This file is proprietary information, and may not be disclosed or
	copied without the prior permission of MCCI Corporation

Author:
	ChaeHee Won, MCCI Corporation	October 2013

Revision history:
   0.1.1  Wed Mar 22 2017 23:55:01  tmm
	Adapt for MCCI ADK.

*/

#include "mcciadk_baselib.h"

#include <limits.h>

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

Name:	McciAdkLib_BufferToUlong

Function:
	Converts text in buffer to unsigned long integer.

Definition:
	size_t McciAdkLib_BufferToUlong(
		const char *s,
		size_t n,
		unsigned base,
		unsigned long *pulnum OUT,
		bool *pfOverflow OUT
		);

Description:
	This function converts text from the buffer at (*s, length n) to
	an |unsigned long int|, using |base| as a guide to the conversion.

	|base| must be one of the following values:

	0	indicating that the number is to be converted following
		C compiler rules.

	2..36	indicating that the number is to be converted using the
		specified fixed base.

	McciAdkLib_BufferToUlong() first skips any leading whitespace from
	the input string, where whitespace is defined as any character in the
	range 0x01..0x20, inclusive.

	Then it consumes an optional '-', and then converts the following
	digits according to base.  If base is zero, then a leading '0x' or
	'0X' indicates that the number is hexadecimal; otherwise a leading
	'0' indicates that the number is octal; otherwise the number is
	assumed to be decimal.

	Digits are consumed from the string until a non-digit (in the current
	base) is seen, or until a the entire string has been consumed.

	The resulting value is stored into *pulnum.  As a convenience, pulnum
	may safely be specified as NULL.

	Some important properties of this routine are:

	1.	If no digits are seen, 0 is returned.

	2.	*pulnum is always guaranteed to be set to a value determined
		by this function, even if errors are detected.  While not 
		always an improvement, this makes *pulnum a functional result 
		of McciAdkLib_BufferToUlong().

	3.	If base == 0, then the number is scanned adaptively, using
		the usual rules for the C compiler:  leading zeroes cause the
		number to be interpreted in octal, leading "0x" or "0X"
		cause number to be interpreted in hex; otherwise number is
		interpreted in decimal.

	4.	If base == 16, there must be NO leading "0x" or "0X".

	5.	If base == 0, and the input buffer (after deleting leading
		blanks and consuming the optional minus sign) is "0x" or 
		"0X" -- i.e., is the hex prefix ONLY --,
		then this routine will convert the "0" and will return a
		byte count indicating that the 'X' was not consumed.

	6.	If the number is too large, but still consists of valid
		digits, then all the digits will be consumed, but ULONG_MAX
		will be returned, and *pfOverflow will be set true.

Returns:
	Number of characters actually consumed from buffer, assuming that
	no error was detected.  If no digits were seen, we'll always return
	0 (even if there was leading whitespace that we skipped over).

Notes:
	Digits must be in the latin alphabet [A-Za-z] set.
	The input string must be UTF-8, latin1, ANSI, or a similar character
	set that maps to ANSI.

	There is no overflow checking. 

	It is odd to support negation for an unsigned number input.

Examples:
	const char t1[] = " 31.5";
	const char t2[] = " -31.5";
	const char t3[] = " -0x";
	const char t4[] = " -0x0";
	const char t5[] = "-80000001";
	unsigned long result;
	size_t nc;

	nc = McciAdkLib_BufferToUlong(t1, sizeof(t1), 0, &result, NULL);
	// nc is now 3, and result is now 31

	nc = McciAdkLib_BufferToUlong(t2, sizeof(t2), 2, &result, NULL);
	// nc is now 0, and result is 0: 3 is not a valid base-2 digit

	nc = McciAdkLib_BufferToUlong(t2, sizeof(t2), 4, &result, NULL);
	// nc is now 4, and result is -13

	nc = McciAdkLib_BufferToUlong(t3, sizeof(t3), 0, &result, NULL);
	// nc is now 3, and result is 0

	nc = McciAdkLib_BufferToUlong(t4, sizeof(t4), 0, &result, NULL);
	// nc is now 5, and result is 0

	nc = McciAdkLib_BufferToUlong(t5, sizeof(t5), 16, &result, NULL);
	// nc is now 8, and result is 0x80000000

*/

size_t
McciAdkLib_BufferToUlong(
	const char *s,
	size_t n,
	unsigned base,
	unsigned long *pulnum,
	bool *pfOverflow
	)
	{
	size_t l;
	bool neg;
	bool digseen;
	unsigned c;
	unsigned long ulnum;
	unsigned long maxval;
	unsigned long maxval_digit;
	bool fOverflow;

	/* initialize, do idiot checks */
	l = n;

	/* set post-condition */
	fOverflow = false;

	if (pfOverflow)
		*pfOverflow = fOverflow;

	if (s == NULL || base == 1 || /* base < 0 || */ base > 36)
		{
		if (pfOverflow)
			*pfOverflow = false;
		if (pulnum)
			*pulnum = 0;

		return 0;
		}

	/* skip leading whitespace */
	/*NOSTRICT*/
	while (l > 0 && ((c = *s & 0xFF), McciAdkLib_CharIsWhite(c)))
		++s, --l;

	/* check for leading minus */
	if (l > 1 && s[0] == '-')
		{
		neg = true;
		++s, --l;
		}
	else
		neg = false;

	if (base == 0)
		{
		if (l <= 1 || s[0] != '0')
			base = 10;
		else if (l >= 3 && (s[1] == 'x' || s[1] == 'X'))
			{
			base = 16;
			s += 2;
			l -= 2;
			}
		else
			base = 8;
		}

	/* now, scan the number */
	maxval = ULONG_MAX / base;
	maxval_digit = ULONG_MAX - (maxval * base);

	ulnum = 0;
	for (digseen = false; l > 0; --l, ++s, digseen = true)
		{
		unsigned d;
		c = *s & 0xFF;
		c = McciAdkLib_CharToLower(c);

		if (McciAdkLib_CharIsDigit(c))
			d = c - '0';
		else if (McciAdkLib_CharIsLower(c))
			d = c - 'a' + 10;
		else
			break;

		if (d < base)
			{
			if (ulnum > maxval ||
			    (ulnum == maxval && d > maxval_digit))
				{
				fOverflow = true;
				ulnum = ULONG_MAX;
				}
			else
				ulnum = ulnum * base + d;
			}
		else
			break;
		}

	/* return result, negating if necessary */
	if (pulnum)
		{
		if (fOverflow)
			*pulnum = ULONG_MAX;
		else if (neg)
			*pulnum = 0 - ulnum;
		else
			*pulnum = ulnum;
		}

	if (pfOverflow)
		*pfOverflow = fOverflow;

	/* return number of characters */
	if (digseen)
		return n - l;
	else
		return 0;
	}

/**** end of mcciadklib_buffertoulong.c ****/
