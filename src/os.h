/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

/*! \file */

#ifndef NMEA_OS_H
#define NMEA_OS_H

#include "nmea/config.h"

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define os_malloc   malloc
#define os_free     free
#define os_memcpy   memcpy
#define os_memset   memset
#define os_memcmp   memcmp
#define os_strchr   strchr

#define os_atoi     atoi
#define os_atof     atof
#define os_strlen   strlen

#ifndef _MSC_VER
#   define os_snprintf  snprintf
#else
#   define os_snprintf  _snprintf
#endif

#endif /* NMEA_OS_H */
