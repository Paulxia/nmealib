/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: data.h 10 2007-11-15 14:50:15Z xtimor $
 *
 */

/*! \file */

#ifndef __NMEA_VARIANT_H__
#define __NMEA_VARIANT_H__

#ifdef  __cplusplus
extern "C" {
#endif

enum _nmeaVARIANT_TYPE
{
    NMEA_NULL,
    NMEA_CHAR,
    NMEA_SHORT,
    NMEA_INT,
    NMEA_LONG,
    NMEA_FLOAT,
    NMEA_DOUBLE,
    NMEA_DATA
};

typedef struct _nmeaVARIANT
{
    int type;
    int size;
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

int nmea_variant_copy(nmeaVARIANT *src, nmeaVARIANT *dst);
int nmea_variant_convert(nmeaVARIANT *src, nmeaVARIANT *dst);

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_VARIANT_H__ */
