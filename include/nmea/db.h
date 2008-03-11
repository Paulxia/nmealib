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

#ifndef NMEA_DB_H
#define NMEA_DB_H

#include "variable.h"

#define NMEA_SIG_BAD        (0)
#define NMEA_SIG_LOW        (1)
#define NMEA_SIG_MID        (2)
#define NMEA_SIG_HIGH       (3)

#define NMEA_FIX_BAD        (1)
#define NMEA_FIX_2D         (2)
#define NMEA_FIX_3D         (3)

#define NMEA_MAXSAT         (12)
#define NMEA_SATINPACK      (4)
#define NMEA_NSATPACKS      (NMEA_MAXSAT / NMEA_SATINPACK)

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * All variables in NMEA database
 * @see nmeaDB
 */
enum _nmeaVALUE
{
    NMEA_PACKET_MASK,
    NMEA_DATE_DAY,
    NMEA_DATE_MONTH,
    NMEA_DATE_YEAR,
    NMEA_TIME_HOUR,
    NMEA_TIME_MIN,
    NMEA_TIME_SEC,
    NMEA_LAT,
    NMEA_LON,
    NMEA_FIX_QUALITY,
    NMEA_FIX_TYPE,
    NMEA_PDOP,
    NMEA_HDOP,
    NMEA_VDOP,
    NMEA_ELEVATION,
    NMEA_SPEED,
    NMEA_DIRECTION,
    NMEA_DECLINATION,
    NMEA_SAT_INUSE,
    NMEA_SAT_INVIEW,
    NMEA_SAT_MAX,
    NMEA_SATELLITE,
    NMEA_VALUE_LAST
};

/**
 * Information about satellite
 * @see nmeaSATINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATELLITE
{
    int     id;         /**< Satellite PRN number */
    int     in_use;     /**< Used in position fix */
    int     elv;        /**< Elevation in degrees, 90 maximum */
    int     azimuth;    /**< Azimuth, degrees from true north, 000 to 359 */
    int     sig;        /**< Signal, 00-99 dB */

} nmeaSATELLITE;

typedef struct _nmeaDB
{
    struct _nmeaBIND *vars;
    nmea_idx nvar;

} nmeaDB;

int     nmea_db_init(nmeaDB *db);
int     nmea_db_done(nmeaDB *db);

int     nmea_db_hasvar(nmeaDB *db, nmea_idx index);
int     nmea_db_bind(nmeaDB *db, nmea_idx index, void *var, nmea_idx var_type, nmea_idx var_sz);
int     nmea_db_unbind(nmeaDB *db, nmea_idx index);

int     nmea_db_set(nmeaDB *db, nmea_idx index, const void *var, nmea_idx var_type, int var_sz);
int     nmea_db_get(nmeaDB *db, nmea_idx index, void *var, nmea_idx var_type, int var_sz);

char    nmea_db_char(nmeaDB *db, nmea_idx index);
short   nmea_db_short(nmeaDB *db, nmea_idx index);
int     nmea_db_int(nmeaDB *db, nmea_idx index);
long    nmea_db_long(nmeaDB *db, nmea_idx index);
float   nmea_db_float(nmeaDB *db, nmea_idx index);
double  nmea_db_double(nmeaDB *db, nmea_idx index);

int     nmea_db_set_char(nmeaDB *db, nmea_idx index, char value);
int     nmea_db_set_short(nmeaDB *db, nmea_idx index, short value);
int     nmea_db_set_int(nmeaDB *db, nmea_idx index, int value);
int     nmea_db_set_long(nmeaDB *db, nmea_idx index, long value);
int     nmea_db_set_float(nmeaDB *db, nmea_idx index, float value);
int     nmea_db_set_double(nmeaDB *db, nmea_idx index, double value);

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_DB_H */
