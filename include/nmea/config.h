/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: config.h 17 2008-03-11 11:56:11Z xtimor $
 *
 */

#ifndef __NMEA_CONFIG_H__
#define __NMEA_CONFIG_H__

#define NMEA_CONVSTR_BUF    (256)
#define NMEA_TIMEPARSE_BUF  (256)

#define NMEA_POSIX(x)  x
#define NMEA_INLINE    inline

#if !defined(NDEBUG)
#include <assert.h>
#define NMEA_ASSERT(x)   assert(x)
#else
#define NMEA_ASSERT(x)
#endif

#endif /* __NMEA_CONFIG_H__ */
