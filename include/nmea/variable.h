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

#ifndef NMEA_VARIABLE_H
#define NMEA_VARIABLE_H

#include "config.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef unsigned int nmea_idx;

enum _nmeaVALUE_TYPE
{
    NMEA_NONE = 0,
    NMEA_CHAR,
    NMEA_SHORT,
    NMEA_INT,
    NMEA_LONG,
    NMEA_FLOAT,
    NMEA_DOUBLE,
    NMEA_STRING,
    NMEA_DATA,
    NMEA_TYPE_LAST
};

int nmea_variable_set(
    void *dst, nmea_idx dst_type, nmea_idx dst_sz,
    const void *src, nmea_idx src_type, nmea_idx src_sz
    );

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_VARIABLE_H */
