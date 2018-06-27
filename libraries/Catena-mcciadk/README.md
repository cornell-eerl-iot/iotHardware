# Catena-mcciadk

This repository contains the MCCI&reg; ADK, a version of the MCCI XDK adapted for use on Catena&reg;-like Arduinos by [MCCI Corporation](http://www.mcci.com).

[![GitHub release](https://img.shields.io/github/release/mcci-catena/Catena-mcciadk.svg)](https://github.com/mcci-catena/arduino-lorawan/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/mcci-catena/Catena-mcciadk/latest.svg)](https://github.com/mcci-catena/Catena-mcciadk/compare/v0.1.3...master)
[![Build Status](https://travis-ci.com/mcci-catena/Catena-mcciadk.svg?branch=master)](https://travis-ci.com/mcci-catena/Catena-mcciadk)

**Contents**
<!-- TOC depthFrom:2 updateOnSave:true -->

- [Introduction](#introduction)
- [`mcciadk_env.h`](#mcciadk_envh)
	- [Compile-time text manipulation](#compile-time-text-manipulation)
	- [Value Computation](#value-computation)
	- [Static Assert](#static-assert)
	- [C++/C wrappers](#cc-wrappers)
	- [Parameter and Variable Decoration](#parameter-and-variable-decoration)
- [`mcciadk_guid.h`](#mcciadk_guidh)
- [`mcciadk_baselib.h`](#mcciadk_baselibh)
- [Release History](#release-history)
- [Bugs or Issues](#bugs-or-issues)
- [License and Credits](#license-and-credits)
	- [MIT License](#mit-license)
	- [Contributors](#contributors)
	- [Trademark Acknowledgements](#trademark-acknowledgements)
	- [Support Open Source Hardware](#support-open-source-hardware)

<!-- /TOC -->
## Introduction

MCCI uses its XDK (cross-platform development kit) for writing portable code for use across development environments, compilers and operating systems. Although we don't think it's really appropriate for Arduino work, we find some of its idioms extremely useful, along with some of the functions.

By convention, MCCI makes specific versions of development environments for different targets. We have, for example, a "MCCI WDK" library for use in with the Microsoft Windows Driver Development Kit; we have an "LDK" library for use with the Linux kernel; and we have a "PDK" library for use in portable Posix application development. All share basic features with the XDK. So it made sense to us to create an "Arduino Development Kit" ("ADK") that makes it easy to bring in other MCCI code to Arduino environments.

Some of these macros represent ways of dealing with portabiltiy that actually are not applicable in the Arduino environment. However, we use them anyway.

- MCCI has lots of code that use these constructs, and it's easier (and more reliable) to port in the code using the macros.
- If we use these macros, we can quickly port code that we develop in the Arduino environment to our other xDKs.
- We're used to this style, and so we can focus on the tasks at hand.

There are two parts to the ADK:

- header files and compile-time helpers
- library routines

We'll discuss them in turn.

## `mcciadk_env.h`

This header file sets up a common compile-time environment.

### Compile-time text manipulation

- `MCCIADK_STRING(sym)` returns `sym` as a string, without macro expansion.
- `MCCIADK_STRINGVAL(sym)` returns `sym` as a string, after macro expansion.
- `MCCIADK_CONCAT(a, b)` returns `ab` as a single token, without macro expansion.
- `MCCIADK_CONCAT3(a, b, c)`, returns `abc` as a single token, without macro expansion.
- `MCCIADK_CONCATVAL3(a, b, c)`, returns `abc` as a single token, *after* macro expansion.

### Value Computation

- `MCCIADK_CONTAINER_OF(innerPointer, OuterType, innerFieldName)`: given a pointer to a field in a larger structure, the type of the larger structure, and the name of the inner field, `MCCIADK_CONTAINER_OF()` returns a pointer of type `OuterType` pointing to the larger structure.

- `MCCIADK_MEMTAG()`: generates a consistent memory tag (with controlled byte order) from a four-byte sequence. Often used to tag data structures to assist with port-mortem crash analysis.

- `MCCIADK_LENOF(array)`: returns the number of elements in the array (as opposed to `sizeof(array)`, which returns the number of bytes in the array).

### Static Assert

- `MCCIADK_C_ASSERT(constExpr)`: this is the equivalent of C++- 2017's `static_assert(constExpr)` built-in. It generates a compile-time error if `constExpr` evaluates to zero. The compile error is, unfortunately, not terribly informative (as it will refer to the declaration of an array with negative size).

Use this whereever a declaration is valid.

### C++/C wrappers

We don't like the repeated construct:

```c++
#ifdef __cplusplus
extern "C" {
#endif
```

and it's close. We think it adds typing and is just kind of ugly. And we strive to minimize `#if` in code files (as opposed to header files.) So instead, we have macros:

- `MCCIADK_BEGIN_DECLS` expands to the `extern "C" {` wrapper if compiling in C++ mode, or to nothing otherwise.

- `MCCIADK_END_DECLS` expands to ```}``` if compiling in C++ mode, or to nothing otherwise.

### Parameter and Variable Decoration

We like to compile with high levels of warnings, but this means we'll get errors and warnings about unreferenced parameters. Sometimes these warnings are bogus, because the parameters are part of a contract rather than being something important to the caller.

We thus have macros to tell the compiler to back off, we know what we're doing.  The specific macros are:

- `MCCIADK_UNREFERENCED_PARAMETER(name)` states that parameter `name` is intentionally not referenced by the routine.

- `MCCIADK_API_PARAMETER(name)` states that parameter `name` is supplied as part of an API definition. The routine doesn't need it.  This is subtly different than `MCCIADK_UNREFERENCED_PARAMETER()`; it's a stronger claim. A parameter might be unreferenced because code has evolved and we've not got around to fixing it. An API parameter is unreferenced in this particular routine, but that doesn't mean that it's not needed elsewhere.

- `MCCIADK_UNREFERENCED_VARIABLE(v)` states that variable `v` is intentionally unreferenced (or tells the compiler that, even though it doesn't seem to be referenced, it really is). This might be used in stubs, or in code that is being tested.

## `mcciadk_guid.h`

This header file defines useful macros for GUID manipulation.  (GUIDs or UUIDs are 128-bit values that are often used in abstract API construction.)

## `mcciadk_baselib.h`

This header file provides a number of portable APIs for use by ADK clients.

- `McciAdkLib_BufferToUlong()` converts a string buffer to an unsigned long. It has well-defined error handling and overflow properties.

- `McciAdkLib_BufferToUint32()` converts a string buffer to a `uint32_t`. It has well-defined error handling and overflow properties.

- `McciAdkLib_CharIsLower()`, `McciAdkLib_CharIsPrint()`, `McciAdkLib_CharIsUpper()`, `McciAdkLib_CharIsWhite()`, and `McciAdkLib_CharToLower()` duplicate some of the functions of `<ctype.h>`. They're justified because they avoid Unicode and other portability distractions in embedded systems with small memory.

- `McciAdkLib_StringCompareCaseInsensitive()` compares two ASCII strings without considering case, and also without internationalization considerations. 

- `McciAdkLib_MultiSzIndex()` is used for handling arrays of string values. Rather than using an array of `char*` pointers, you simply write the values into a string constant separated by "\0" values. For example, for three values, you might write:

   ```c++
   char my_list[] = 
   	"one" "\0"
   	"two" "\0"
   	"three" "\0"
   	;
   ```

   Then, `McciAdkLib_MultiSzIndex(my_list, 0)` will return a pointer to `"one"`, `McciAdkLib_MultiSzIndex(my_list, 1)` will return `"two"`, and `McciAdkLib_MultiSzIndex(my_list, 2)` will return `"three"`. Other indices will return `nullptr`.

- `McciAdkLib_FormatDumpLine()` prepares one line of a classic "memory dump" string in a buffer, with pointer, hex values, and equivalent printable values.

- `McciAdkLib_Snprintf()` and `McciAdkLib_Vsnprintf()` are like `snprintf()` and `vsnprintf()` from `<stdio.h>`, but:

   1. You don't need `<stdio.h>` in scope to use them.
   2. They behave differently on buffer overflow. They always return the actual length of the string written to buffer, without the trailing `\0', not the length that might have been written.
   3. They allow you to easily index into the output buffer.

## Release History

- v0.1.3 adds documentation, continuous integration with Travis CI, and fixes some compile warnings.

## Bugs or Issues

If you find a bug you can submit an issue here on github:

https://github.com/mcci-catena/Cantea-mcciadk/issues


Before posting a new issue, please check if the same problem has been already reported by someone else to avoid duplicates.

## License and Credits

### MIT License

Please refer to the license file accompanying this repository.

### Contributors

Terry Moore and ChaeHee Won of MCCI were the pricipal contributors to the code in the ADK (and the XDK on which it's based).

### Trademark Acknowledgements

MCCI and MCCI Catena are registered trademarks of MCCI Corporation. All other trademarks are the properties of their respective owners.

### Support Open Source Hardware 

MCCI invests time and resources providing this open source code, please support MCCI and open-source hardware by purchasing products from MCCI, Adafruit and other open-source hardware/software vendors!

For information about MCCI's products, please visit [store.mcci.com](https://store.mcci.com/).
