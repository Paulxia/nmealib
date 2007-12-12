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

#include "nmea/db.h"

#include "nmea/hash.h"
#include "os.h"

#include <stdarg.h>

int nmea_db_init(nmeaDB *db)
{
    os_memset(db, 0, sizeof(nmeaDB));

    db->hash = (nmeaHASH *)os_malloc(sizeof(nmeaHASH));

    if(!db->hash)
        goto fail;

    db->rdbuff_sz = NMEA_RDBUFF_SZ;
    db->rdbuff = os_malloc(NMEA_RDBUFF_SZ);

    if(!db->rdbuff)
        goto fail;

    if(nmea_hash_init(db->hash, NMEA_VALUE_LAST) < 0)
        goto fail;

#ifdef NMEA_CONFIG_THREADSAFE
    if(0 == (db->lock = os_mutex_init()))
        goto fail;
#endif

    return 0;

fail:
    nmea_db_done(db);
    return -1;
}

int nmea_db_done(nmeaDB *db)
{
    if(db->hash)
    {
        if(db->hash->values)
            nmea_hash_done(db->hash);
        os_free(db->hash);
    }

    if(db->rdbuff)
        os_free(db->rdbuff);

#ifdef NMEA_CONFIG_THREADSAFE
    if(db->lock)
        os_mutex_done(db->lock);
#endif

    os_memset(db, 0, sizeof(nmeaDB));

    return 0;
}

int nmea_db_clear(nmeaDB *db)
{
    return nmea_hash_clear(db->hash);
}

int nmea_db_copy(nmeaDB *src, nmeaDB *dst)
{
    return nmea_hash_copy(src->hash, dst->hash);
}

#ifdef NMEA_CONFIG_THREADSAFE
int nmea_db_lock(nmeaDB *db)
{
    return os_mutex_lock(db->lock);
}

int nmea_db_unlock(nmeaDB *db)
{
    return os_mutex_unlock(db->lock);
}
#endif

int nmea_db_get(nmeaDB *db, int index, nmeaVARIANT *var)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return -1;

    return nmea_variant_copy(tmp, var);
}

int nmea_db_set(nmeaDB *db, int index, nmeaVARIANT *var)
{
    return nmea_hash_set(db->hash, (unsigned int)index, var);
}

char nmea_db_char(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    char value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return -1;
    if(nmea_variant_get(tmp, NMEA_CHAR, &value, sizeof(char)) < 0)
        return -1;

    return value;
}

short nmea_db_short(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    short value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return -1;
    if(nmea_variant_get(tmp, NMEA_SHORT, &value, sizeof(short)) < 0)
        return -1;

    return value;
}

int nmea_db_int(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    int value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return -1;
    if(nmea_variant_get(tmp, NMEA_INT, &value, sizeof(int)) < 0)
        return -1;

    return value;
}

long nmea_db_long(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    long value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return -1;
    if(nmea_variant_get(tmp, NMEA_LONG, &value, sizeof(long)) < 0)
        return -1;

    return value;
}

float nmea_db_float(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    float value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return 0;
    if(nmea_variant_get(tmp, NMEA_FLOAT, &value, sizeof(float)) < 0)
        return 0;

    return value;
}

double nmea_db_double(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;
    double value;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return 0;
    if(nmea_variant_get(tmp, NMEA_DOUBLE, &value, sizeof(double)) < 0)
        return 0;

    return value;
}

char * nmea_db_string(nmeaDB *db, int index)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, (unsigned int)index, &tmp) < 0)
        return 0;
    if(nmea_variant_get(tmp, NMEA_STRING, db->rdbuff, db->rdbuff_sz) < 0)
        return 0;

    return (char *)db->rdbuff;
}

nmeaSATINFO * nmea_db_satinfo(nmeaDB *db)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, NMEA_SAT_INFO, &tmp) < 0)
        return 0;
    if(nmea_variant_get(tmp, NMEA_DATA, db->rdbuff, db->rdbuff_sz) < 0)
        return 0;

    return (nmeaSATINFO *)db->rdbuff;
}

int nmea_db_set_char(nmeaDB *db, int index, char value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_CHAR, &value, sizeof(char)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_short(nmeaDB *db, int index, short value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_SHORT, &value, sizeof(short)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_int(nmeaDB *db, int index, int value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_INT, &value, sizeof(int)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_long(nmeaDB *db, int index, long value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_LONG, &value, sizeof(long)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_float(nmeaDB *db, int index, float value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_FLOAT, &value, sizeof(float)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_double(nmeaDB *db, int index, double value)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_DOUBLE, &value, sizeof(double)) < 0)
        return -1;

    return 0;
}

int nmea_db_set_string(nmeaDB *db, int index, char *value)
{
    return nmea_db_set_stringn(db, index, value, (int)os_strlen(value));
}

int nmea_db_set_stringn(nmeaDB *db, int index, char *value, int str_sz)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, index, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_STRING, value, str_sz) < 0)
        return -1;

    return 0;
}

int nmea_db_set_satinfo(nmeaDB *db, nmeaSATINFO *info)
{
    nmeaVARIANT *tmp;

    if(nmea_hash_get(db->hash, NMEA_SAT_INFO, &tmp) < 0)
        return -1;
    if(nmea_variant_set(tmp, NMEA_DATA, info, sizeof(nmeaSATINFO)) < 0)
        return -1;

    return 0;
}
