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

#include "os.h"

typedef struct _nmeaBIND
{
    nmea_idx type;
    nmea_idx size;
    void *var;

} nmeaBIND;

int nmea_db_init(nmeaDB *db)
{
    int vars_space = sizeof(nmeaBIND) * NMEA_VALUE_LAST;

    os_memset(db, 0, sizeof(nmeaDB));

    if(0 == (db->vars = (nmeaBIND *)os_malloc(vars_space)))
        return -1;

    os_memset(db->vars, 0, vars_space);

    db->nvar = NMEA_VALUE_LAST;

    return 0;
}

int nmea_db_done(nmeaDB *db)
{
    if(db->vars)
        os_free(db->vars);

    os_memset(db, 0, sizeof(nmeaDB));

    return 0;
}

int nmea_db_hasvar(nmeaDB *db, nmea_idx index)
{
#ifdef NMEA_CONFIG_CHECK_RANGE
    if(index >= db->nvar)
        return -1;
#endif
    return NMEA_NONE == db->vars[index].type || !(db->vars[index].var);
}

int nmea_db_bind(nmeaDB *db, nmea_idx index, void *var, nmea_idx var_type, nmea_idx var_sz)
{
    nmeaBIND *b = &(db->vars[index]);

#ifdef NMEA_CONFIG_CHECK_RANGE
    if(index >= db->nvar)
        return -1;
#endif

    b->type = var_type;
    b->size = var_sz;
    b->var = var;

    return 0;
}

int nmea_db_unbind(nmeaDB *db, nmea_idx index)
{
    nmeaBIND *b = &(db->vars[index]);

#ifdef NMEA_CONFIG_CHECK_RANGE
    if(index >= db->nvar)
        return -1;
#endif

    b->type = NMEA_NONE;
    b->size = 0;
    b->var = 0;

    return 0;
}

int nmea_db_set(nmeaDB *db, nmea_idx index, const void *var, nmea_idx var_type, int var_sz)
{
    nmeaBIND *b = &(db->vars[index]);

#ifdef NMEA_CONFIG_CHECK_RANGE
    if(index >= db->nvar)
        return -1;
#endif

    return nmea_variable_set(b->var, b->type, b->size, var, var_type, var_sz);
}

int nmea_db_get(nmeaDB *db, nmea_idx index, void *var, nmea_idx var_type, int var_sz)
{
    nmeaBIND *b = &(db->vars[index]);

#ifdef NMEA_CONFIG_CHECK_RANGE
    if(index >= db->nvar)
        return -1;
#endif

    return nmea_variable_set(var, var_type, var_sz, b->var, b->type, b->size);
}

char nmea_db_char(nmeaDB *db, nmea_idx index)
{
    char value = 0;
    nmea_db_get(db, index, &value, NMEA_CHAR, sizeof(char));
    return value;
}

short nmea_db_short(nmeaDB *db, nmea_idx index)
{
    short value = 0;
    nmea_db_get(db, index, &value, NMEA_SHORT, sizeof(short));
    return value;
}

int nmea_db_int(nmeaDB *db, nmea_idx index)
{
    int value = 0;
    nmea_db_get(db, index, &value, NMEA_INT, sizeof(int));
    return value;
}

long nmea_db_long(nmeaDB *db, nmea_idx index)
{
    long value = 0;
    nmea_db_get(db, index, &value, NMEA_LONG, sizeof(long));
    return value;
}

float nmea_db_float(nmeaDB *db, nmea_idx index)
{
    float value = 0;
    nmea_db_get(db, index, &value, NMEA_FLOAT, sizeof(float));
    return value;
}

double nmea_db_double(nmeaDB *db, nmea_idx index)
{
    double value = 0;
    nmea_db_get(db, index, &value, NMEA_DOUBLE, sizeof(double));
    return value;
}

int nmea_db_set_char(nmeaDB *db, nmea_idx index, char value)
{
    return nmea_db_set(db, index, &value, NMEA_CHAR, sizeof(char));
}

int nmea_db_set_short(nmeaDB *db, nmea_idx index, short value)
{
    return nmea_db_set(db, index, &value, NMEA_SHORT, sizeof(short));
}

int nmea_db_set_int(nmeaDB *db, nmea_idx index, int value)
{
    return nmea_db_set(db, index, &value, NMEA_INT, sizeof(int));
}

int nmea_db_set_long(nmeaDB *db, nmea_idx index, long value)
{
    return nmea_db_set(db, index, &value, NMEA_LONG, sizeof(long));
}

int nmea_db_set_float(nmeaDB *db, nmea_idx index, float value)
{
    return nmea_db_set(db, index, &value, NMEA_FLOAT, sizeof(float));
}

int nmea_db_set_double(nmeaDB *db, nmea_idx index, double value)
{
    return nmea_db_set(db, index, &value, NMEA_DOUBLE, sizeof(double));
}
