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

#ifndef NMEA_VARIANT_H
#define NMEA_VARIANT_H

#include "config.h"

#ifdef _MSC_VER
#   pragma warning(disable:4201)
#endif

#ifdef  __cplusplus
extern "C" {
#endif

enum _nmeaVARIANT_TYPE
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

typedef struct _nmeaVARIANT
{
    unsigned int type;
    unsigned int size;

    union
    {
        char vchar;
        short vshort;
        int vint;
        long vlong;
        float vfloat;
        double vdouble;
        void *vdata;

    };

} nmeaVARIANT;

void    nmea_variant_zero(nmeaVARIANT *var);
void    nmea_variant_clear(nmeaVARIANT *var);
int     nmea_variant_copy(nmeaVARIANT *src, nmeaVARIANT *dst);
int     nmea_variant_set(nmeaVARIANT *dst, unsigned int vtype, void *src, unsigned int src_sz);
int     nmea_variant_get(nmeaVARIANT *src, unsigned int vtype, void *dst, unsigned int dst_sz);

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_VARIANT_H */
