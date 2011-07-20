///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_CONFIG_H__
#define __BOX_CONFIG_H__

#define BOX_VERSION         ("0.1.2b")
#define BOX_VERSION_MAJOR   (0)
#define BOX_VERSION_MINOR   (1)
#define BOX_VERSION_PATCH   (2)

#define BOX_SPRINTF_BUFF    (2048)

#if defined(WINCE) || defined(UNDER_CE)
#   define  BOX_CE
#endif

#if defined(WIN32) || defined(BOX_CE)
#   define  BOX_WIN
#else
#   define  BOX_UNI
#endif

#if defined(_MSC_VER)
#   define BOX_POSIX(x) _##x
#else
#   define BOX_POSIX(x) x
#endif

#if !defined(NDEBUG) && !defined(BOX_CE)
#   include <assert.h>
#   define BOX_ASSERT(x)   assert(x)
#else
#   define BOX_ASSERT(x)
#endif

#endif // __BOX_CONFIG_H__
