/* mcciadklib_stringcomparecaseinsensitive.c	Thu Dec 07 2017 00:16:05 tmm */

/*

Module:  mcciadklib_stringcomparecaseinsensitive.c

Function:
	McciAdkLib_StringCompareCaseInsensitive()

Version:
	V0.1.2	Thu Dec 07 2017 00:16:05 tmm	Edit level 1

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
   0.1.2  Thu Dec 07 2017 00:16:05  tmm
	Module created.

*/

#include "mcciadk_baselib.h"

/*

Name:	McciAdkLib_StringCompareCaseInsensitive()

Function:
	Case-insensitive compare (for ASCII, in any case)

Definition:
	int McciAdkLib_StringCompareCaseInsensitive(
		const char *pLeft,
                const char *pRight
		);

Description:
	Compare pLeft to pRight. If strings are equal (ignoring ASCII upper/
        lower case differences), return 0. Otherwise return -1 if left is
        lexicographically less than right, 1 if left is greater than right.
        NULL pointers sort less than any non-NULL pointer. Letters are folded
        to lower case for the purposes of sorting.

Returns:
	The result of the compare.

*/

int
McciAdkLib_StringCompareCaseInsensitive(
        const char *pLeft,
        const char *pRight
        )
        {
        if (pLeft == NULL)
                return pRight ? 0 : -1;

        if (pRight == NULL)
                return 1;

        for (;; ++pLeft, ++pRight)
                {
                int cLeft, cRight;

                cLeft = *pLeft;
                cRight = *pRight;

                if (cLeft == cRight)
                        {
                        if (cLeft != 0)
                                continue;
                        /* strings are equal */
                        return 0;
                        }

                cLeft = McciAdkLib_CharToLower(cLeft & 0xFF);
                cRight = McciAdkLib_CharToLower(cRight & 0xFF);

                if (cLeft == cRight)
                        continue;

                else return (cLeft < cRight) ? -1 : 1;
                }
        }
