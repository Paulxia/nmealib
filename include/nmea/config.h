/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

#ifndef NMEA_CONFIG_H
#define NMEA_CONFIG_H

/* Build config */

#define NMEA_CONFIG_USEINFO
#define NMEA_CONFIG_THREADSAFE
#define NMEA_CONFIG_MATH
//#define NMEA_CONFIG_CHECK_CONVERT

/* Utility defines */

#define NMEA_VERSION        ("1.0.0")
#define NMEA_VERSION_MAJOR  (1)
#define NMEA_VERSION_MINOR  (0)
#define NMEA_VERSION_PATCH  (0)

#define NMEA_CONVSTR_BUF    (255)
#define NMEA_RDBUFF_SZ      (1024)

/* Platform defines */

#if defined(WINCE) || defined(UNDER_CE)
#   define  NMEA_CE
#endif

#if defined(WIN32) || defined(NMEA_CE)
#   define  NMEA_WIN
#else
#   define  NMEA_UNI
#endif

#if defined(NMEA_WIN) && (_MSC_VER >= 1400)
# pragma warning(disable: 4996) /* declared deprecated */
#endif

#if defined(_MSC_VER)
# define NMEA_INLINE    __inline
#else
# define NMEA_INLINE    inline
#endif

#if !defined(NDEBUG) && !defined(NMEA_CE)
#   include <assert.h>
#   define NMEA_ASSERT(x)   assert(x)
#else
#   define NMEA_ASSERT(x)
#endif

#if !defined(NDEBUG) && !defined(NMEA_CONFIG_CHECK_CONVERT)
#   define NMEA_CONFIG_CHECK_CONVERT
#endif

#endif /* NMEA_CONFIG_H */
