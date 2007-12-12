/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#include "config.h"

#ifdef  __cplusplus
extern "C" {
#endif

int     nmea_atoi(const char *str, int str_sz, int radix);
double  nmea_atof(const char *str, int str_sz);
int     nmea_calc_crc(const char *buff, int buff_sz);

int     nmea_scanf(
        struct _nmeaDB *db,
        const char *src_buff, int buff_sz,
        const char *format, ...
        );
int     nmea_printf(
        struct _nmeaDB *db,
        char *dst_buff, int buff_sz,
        const char *format, ...
        );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TOK_H__ */
