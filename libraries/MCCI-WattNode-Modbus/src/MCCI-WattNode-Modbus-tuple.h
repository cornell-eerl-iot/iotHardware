/* MCCI-WattNode-Modbus-tuple.h	Sun Apr 22 2018 15:33:48 tmm */

/*

Module:  MCCI-WattNode-Modbus-tuple.h

Function:
	Wrapper for <tuple> when working with Arduino code.

Version:
	V0.1.0	Sun Apr 22 2018 15:33:48 tmm	Edit level 1

Copyright notice:
	This file copyright (C) 2018 by

		MCCI Corporation
		3520 Krums Corners Road
		Ithaca, NY  14850

	See accompanying LICENSE file.
 
Author:
	Terry Moore, MCCI Corporation	April 2018

Revision history:
   0.5.0  Sun Apr 22 2018 15:33:48  tmm
	Module created.

*/

#ifndef _MCCI_WATTNODE_MODBUS_TUPLE_H_		/* prevent multiple includes */
#define _MCCI_WATTNODE_MODBUS_TUPLE_H_

#pragma once

// this is ugly. Arduino.h defines macros min() and max().... which clobber
// functional. So we do some dark magic.
#if defined(Arduino_h)
# define MCCI_WATTNODE_MODBUS_TUPLE_min_defined 1
# define MCCI_WATTNODE_MODBUS_TUPLE_max_defined 1
#else
# if defined(min)
#  define MCCI_WATTNODE_MODBUS_TUPLE_min_defined 1
# else
#  define MCCI_WATTNODE_MODBUS_TUPLE_min_defined 0
# endif
# if defined(max)
#  define MCCI_WATTNODE_MODBUS_TUPLE_max_defined 1
# else
#  define MCCI_WATTNODE_MODBUS_TUPLE_max_defined 0
# endif
#endif

#if MCCI_WATTNODE_MODBUS_TUPLE_min_defined
# undef min
#endif
#if MCCI_WATTNODE_MODBUS_TUPLE_max_defined
# undef max
#endif

#include <tuple>

/*
|| Now, we reinstate the Arduino.h values... I said it was ugly.
|| In an ideal world we'd be able to save and restore the below. Bear
|| in mind that there may be other words that Arduino.h steps on,
|| breaking C++ header files: it defines a number of lower-case macros with
|| commonly-used words. Since there are no namespaces for macros, it's
|| an unpleasant situation.
*/
#if MCCI_WATTNODE_MODBUS_TUPLE_min_defined
# define min(a,b) ((a)<(b)?(a):(b))
#endif

#if MCCI_WATTNODE_MODBUS_TUPLE_max_defined
# define max(a,b) ((a)>(b)?(a):(b))
#endif


/**** end of MCCI-WattNode-Modbus-tuple.h ****/
#endif /* _MCCI_WATTNODE_MODBUS_TUPLE_H_ */
