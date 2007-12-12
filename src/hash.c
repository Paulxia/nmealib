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

#include "nmea/hash.h"

#include "os.h"

int nmea_hash_init(nmeaHASH *hash, unsigned int size)
{
    unsigned int i;

    os_memset(hash, 0, sizeof(nmeaHASH));

    if(size < 1)
        return -1;

    hash->values = (nmeaVARIANT *)os_malloc(size * sizeof(nmeaVARIANT));

    if(!hash->values)
        return -1;

    hash->size = size;

    for(i = 0; i < size; ++i)
        nmea_variant_zero(&hash->values[i]);

    return 0;
}

int nmea_hash_clear(nmeaHASH *hash)
{
    unsigned int i;

    for(i = 0; i < hash->size; ++i)
        nmea_variant_clear(&hash->values[i]);

    return hash->size;
}

int nmea_hash_copy(nmeaHASH *src, nmeaHASH *dst)
{
    unsigned int i;
    nmeaHASH tmp;

    nmea_hash_clear(dst);

    if(src->size > dst->size)
    {
        tmp = *dst;

        if(nmea_hash_init(dst, src->size) < 0)
        {
            *dst = tmp;
            return -1;
        }
    }

    for(i = 0; i < src->size; ++i)
        nmea_variant_copy(&src->values[i], &dst->values[i]);

    return 0;
}

int nmea_hash_done(nmeaHASH *hash)
{
    unsigned int i;

    for(i = 0; i < hash->size; ++i)
        nmea_variant_clear(&hash->values[i]);

    os_free(hash->values);

    os_memset(hash, 0, sizeof(nmeaHASH));

    return 0;
}

int nmea_hash_get(nmeaHASH *hash, unsigned int index, nmeaVARIANT **value)
{
    if(index >= hash->size)
        return -1;

    *value = &hash->values[index];

    return 0;
}

int nmea_hash_set(nmeaHASH *hash, unsigned int index, nmeaVARIANT *value)
{
    if(index >= hash->size)
        return -1;

    nmea_variant_copy(value, &hash->values[index]);

    return 0;
}
