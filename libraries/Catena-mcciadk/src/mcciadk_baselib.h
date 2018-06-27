/*

Module:  mcciadk_baselib.h

Function:
	The basic ADK library.

Copyright notice:
        See accompanying LICENSE file.

Author:
	Terry Moore, MCCI Corporation	October 2016

*/

#ifndef _MCCIADK_BASELIB_H_		/* prevent multiple includes */
#define _MCCIADK_BASELIB_H_

#ifndef _MCCIADK_ENV_H_
# include <mcciadk_env.h>
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

MCCIADK_BEGIN_DECLS

size_t
McciAdkLib_BufferToUlong(
	const char *s,
	size_t n,
	unsigned base,
	unsigned long *pulnum,
	bool *pfOverflow
	);

static inline size_t
McciAdkLib_BufferToUint32(
	const char *s,
	size_t n,
	unsigned base,
	uint32_t *pulnum,
	bool *pfOverflow
	)
	{
	// this will throw a compile error if uint32_t != ulong
	return McciAdkLib_BufferToUlong(s, n, base, pulnum, pfOverflow);
	}

// check whether c is a decimal digit
static inline bool
McciAdkLib_CharIsDigit(
	char c
	)
	{
	return ('0' <= c && c <= '9');
	}

// check whether c is lower case 'a'..'z'
static inline bool
McciAdkLib_CharIsLower(
	char c
	)
	{
	return ('a' <= c && c <= 'z');
	}

// check whether c is ANSI printable
static inline bool
McciAdkLib_CharIsPrint(
	char c
	)
	{
	return (0x20 <= c && c <= 0x7e);
	}

// check whether c is upper case 'A'..'Z'
static inline bool
McciAdkLib_CharIsUpper(
	char c
	)
	{
	return ('A' <= c && c <= 'Z');
	}

// check whether c is ' ' or a control character in 0..0x1f
static inline bool
McciAdkLib_CharIsWhite(
	char c
	)
	{
	return ((c & 0xFF) <= 0x20);
	}

// if c is an upper case letter, return the lower-case equivalent;
// otherwise return c unchanged.
static inline char
McciAdkLib_CharToLower(
	char c
	)
	{
	if (McciAdkLib_CharIsUpper(c))
		return c - 'A' + 'a';
	else
		return c;
	}

size_t
McciAdkLib_SafeCopyString(
	char *pBuffer,
	size_t nBuffer,
	size_t iBuffer,
	const char *pString
	);

// compare strings, case-insensitive
int
McciAdkLib_StringCompareCaseInsensitive(
        const char *pLeft,
        const char *pRight
        );

size_t
McciAdkLib_FormatDumpLine(
	char *pLine,
	size_t nLine,
        size_t iLine,
	uint32_t address,
	const uint8_t *pBuffer,
	size_t nBuffer
	);

const char *
McciAdkLib_MultiSzIndex(
	const char * pmultiszStrings,
	unsigned uIndex
	);

size_t
McciAdkLib_Snprintf(
	char *pOutbuf,
	size_t nOutbuf,
	size_t iOutbuf,
	const char *pFmt,
	...
	);

size_t
McciAdkLib_Vsnprintf(
	char *pOutbuf,
	size_t nOutbuf,
	size_t iOutbuf,
	const char *pFmt,
	va_list ap
	);

MCCIADK_END_DECLS

/**** end of mcciadk_baselib.h ****/
#endif /* _MCCIADK_BASELIB_H_ */
