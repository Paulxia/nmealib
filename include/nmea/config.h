///////////////////////////////////////////////////////////
//
// NMEA library
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __NMEA_CONFIG_H__
#define __NMEA_CONFIG_H__

#define NMEA_VERSION        ("0.4.0")
#define NMEA_VERSION_MAJOR  (0)
#define NMEA_VERSION_MINOR  (4)
#define NMEA_VERSION_PATCH  (0)

#define NMEA_CONVSTR_BUF    (255)

#if defined(WINCE) || defined(UNDER_CE)
#   define  NMEA_CE
#endif

#if defined(WIN32) || defined(NMEA_CE)
#   define  NMEA_WIN
#else
#   define  NMEA_UNI
#endif

#if defined(NMEA_WIN) && (_MSC_VER >= 1400)
# pragma warning(disable: 4996) // declared deprecated
#endif

#if defined(_MSC_VER)
# define NMEA_POSIX(x)  _##x
# define NMEA_INLINE    __inline
#else
# define NMEA_POSIX(x)  x
# define NMEA_INLINE    inline
#endif

#if !defined(NDEBUG) && !defined(NMEA_CE)
#   include <assert.h>
#   define NMEA_ASSERT(x)   assert(x)
#else
#   define NMEA_ASSERT(x)
#endif


#endif // __NMEA_CONFIG_H__
