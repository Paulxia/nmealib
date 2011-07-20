///////////////////////////////////////////////////////////
//
// Project: XTL
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mkspec.h 27 2007-04-04 19:33:20Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __XTL_PREPARE_H
#define __XTL_PREPARE_H

#ifndef __XTL_BEGIN_NAMESPACE
#   define __XTL xtl
#   define __XTL_BEGIN_NAMESPACE namespace xtl {
#   define __XTL_END_NAMESPACE }
#endif

#ifndef __STD
#   define __STD std
#endif

#if (defined(_UNICODE) || defined(UNICODE))
#   define __XTL_USE_UNICODE
#endif

///////////////////////////////////////////////////////////
// Choose the target platform
///////////////////////////////////////////////////////////

#if defined(_MSC_VER) || (defined(__BORLANDC__) && defined(_Windows))
#   define __XTL_USE_WINAPI
#   define __XTL_USE_PRAGMAONE
#   if defined(_WIN32_WCE) || defined(_UNDER_CE) || defined(UNDER_CE)
#       define __XTL_HABITATOF_WINCE
#   else
#       define __XTL_HABITATOF_WINPC
#   endif
#elif (defined(__MINGW32__) && defined(__MSVCRT__))
#   define __XTL_USE_WINAPI
#   define __XTL_HABITATOF_WINPC
#else
#   define __XTL_HABITATOF_NIX
#endif

///////////////////////////////////////////////////////////
// Windows
///////////////////////////////////////////////////////////

#ifdef __XTL_USE_WINAPI

#if _MSC_VER < 1300
#   pragma warning(disable: 4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#   pragma warning(disable: 4275) // non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#   pragma warning(disable: 4514) // unreferenced inline/local function has been removed
#   pragma warning(disable: 4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning(disable: 4097) // typedef-name 'identifier1' used as synonym for class-name 'identifier2'
#   pragma warning(disable: 4706) // assignment within conditional expression
#   pragma warning(disable: 4786) // truncating debug info after 255 characters
#   pragma warning(disable: 4660) // template-class specialization 'identifier' is already instantiated
#   pragma warning(disable: 4355) // 'this' : used in base member initializer list
#   pragma warning(disable: 4231) // nonstandard extension used : 'extern' before template explicit instantiation
#   pragma warning(disable: 4710) // function not inlined
#endif

#if _MSC_VER >= 1400
#   pragma warning(disable: 4996) // declared deprecated
#   pragma warning(disable: 6011) // dereferencing NULL pointer
#   pragma warning(disable: 6387) // does not adhere to the specification for the function
#endif

__XTL_BEGIN_NAMESPACE

typedef __int8  int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned int uint;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

__XTL_END_NAMESPACE

#ifdef __XTL_USE_PRAGMAONE
#   pragma once 
#endif

#else /* __XTL_USE_WINAPI */

///////////////////////////////////////////////////////////
// Unix
///////////////////////////////////////////////////////////

typedef char int8;
typedef short int16;
typedef long int32;
typedef long long int64;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;

#endif /* __XTL_USE_WINAPI */

#endif /* __XTL_PREPARE_H */
