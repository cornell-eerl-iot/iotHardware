/*

Module:  mcciadklib_safecopystring.c

Function:
	Home for McciAdkLib_SafeCopyString()

Copyright notice:
	See accompanying LICENSE file

Author:
	ChaeHee Won, MCCI Corporation	November 2013

*/

#include "mcciadk_baselib.h"

/****************************************************************************\
|
|	Manifest constants & typedefs.
|
\****************************************************************************/


/****************************************************************************\
|
|	Read-only data.
|
\****************************************************************************/


/****************************************************************************\
|
|	Variables.
|
\****************************************************************************/



/*

Name:	McciAdkLib_SafeCopyString()

Function:
	String copy rotuine that is reasonably safe to use.

Definition:
	size_t
	McciAdkLib_SafeCopyString(
		char *pBuffer,
		size_t nBuffer,
		size_t iBuffer,
		const char *pString
		);

Description:
	This routine copyies memory from the input string to the given
	offset in the buffer, and appends a '\0', taking into account
	the size of the buffer.

	pBuffer is a buffer that has nBuffer bytes allocated to it.
	pString points to a '\0'-terminated string (ANSI, UTF-8, etc --
	encoding is not critical as long as '\0' always designates the
	end of the string.

	Bytes from pString are copied to pBuffer + iBuffer.  In no case
	will data be written outside the range of bytes pBuffer[0..nBuffer).

	The resulting string at pBuffer+iBuffer is guaranteed to be '\0'-
	terminated.  Therefore, the maximum string size that can be handled
	without truncation is (nBuffer - iBuffer - 1) bytes long.

	We can consider boundary conditions without loss of generality by
	considering only the case where iBuffer == 0.

	If pBuffer == NULL, pString == NULL or nBuffer == 0, then the result
	is always 0.

	if nBuffer == 1, then the result is also always 0, but pBuffer[0]
	will be set to '\0'.

	If nBuffer > strlen(pString), then the entire string will be copied
	to pBuffer, and a trailing '\0' is provided.

	If nBuffer == strlen(pString), then all but the last byte is copied,
	a trailing '\0' is provided, and the result is (nBuffer - 1), or
	equivalently strlen(pString)-1.

Returns:
	Number of bytes of pString placed into the buffer.
	The result + iBuffer will always be less than nBuffer (in order
	to guarantee a trailing '\0'), unless nBuffer is zero.

Notes:
	If (iBuffer + the result) >= nBuffer, then you should assume
	that one or more bytes of the string was truncated.  If nBuffer>0,
	and iBuffer + the result == nBuffer-1, then the string may have
	been truncated.

        This implementation favors simplicity and correctness over raw
        speed.

*/

size_t
McciAdkLib_SafeCopyString(
	char *pBuffer,
	size_t nBuffer,
	size_t iBuffer,
	const char *pString
	)
	{
	char *p;

	if (pBuffer == NULL || nBuffer == 0)
		return 0;

	if (iBuffer >= nBuffer-1)
		{
		pBuffer[nBuffer-1] = '\0';
		return 0;
		}

	pBuffer += iBuffer;
	nBuffer -= iBuffer;

	for (p = pBuffer; nBuffer > 1; --nBuffer)
		{
		const char c = *pString++;
		if (c == 0)
			break;
		*p++ = c;
		}

	*p = '\0';
	return p - pBuffer;
	}

/**** end of mcciadklib_safecopystring.c ****/
