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

#ifndef NMEA_HASH_H
#define NMEA_HASH_H

#include "variant.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _nmeaHASH
{
    nmeaVARIANT *values;
    int count;

} nmeaHASH;

int nmea_hash_init(nmeaHASH *hash);
int nmea_hash_clear(nmeaHASH *hash);
int nmea_hash_copy(nmeaHASH *src, nmeaHASH *dst);
int nmea_hash_get(nmeaHASH *hash, int index, nmeaVARIANT *value);
int nmea_hash_set(nmeaHASH *hash, int index, nmeaVARIANT *value);

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_HASH_H */
