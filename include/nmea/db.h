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

#include "config.h"

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

#define NMEA_DEF_LAT        (5001.2621)
#define NMEA_DEF_LON        (3613.0595)

#ifdef  __cplusplus
extern "C" {
#endif

enum _nmeaPACKET_TYPE
{
    NMEA_PT_GPGGA   = 0x0001,
    NMEA_PT_GPGSA   = 0x0002,
    NMEA_PT_GPGSV   = 0x0004,
    NMEA_PT_GPRMC   = 0x0008,
    NMEA_PT_GPVTG   = 0x0010,
    NMEA_PT_ALL     = 0xFFFF
};

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
    NMEA_SAT_INFO,
    NMEA_VALUE_LAST
};

/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS
{
    double lat;         /**< Latitude */
    double lon;         /**< Longitude */

} nmeaPOS;

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

/**
 * Information about all satellites in view
 * @see nmeaINFO
 * @see nmeaGPGSV
 */
typedef struct _nmeaSATINFO
{
    int     inuse;      /**< Number of satellites in use (not those in view) */
    int     inview;     /**< Total number of satellites in view */
    nmeaSATELLITE sat[NMEA_MAXSAT]; /**< Satellites information */

} nmeaSATINFO;

typedef struct _nmeaDB
{
    struct _nmeaHASH *hash;
    void *rdbuff;
    unsigned int rdbuff_sz;
#ifdef NMEA_CONFIG_THREADSAFE
    void *lock;
#endif

} nmeaDB;

int     nmea_db_init(nmeaDB *db);
int     nmea_db_done(nmeaDB *db);
int     nmea_db_clear(nmeaDB *db);
int     nmea_db_copy(nmeaDB *src, nmeaDB *dst);

#ifdef NMEA_CONFIG_THREADSAFE
int     nmea_db_lock(nmeaDB *db);
int     nmea_db_unlock(nmeaDB *db);
#else
#   define nmea_db_lock(x)
#   define nmea_db_unlock(x)
#endif

struct _nmeaVARIANT;

int     nmea_db_get(nmeaDB *db, int index, struct _nmeaVARIANT *);
int     nmea_db_set(nmeaDB *db, int index, struct _nmeaVARIANT *);

char    nmea_db_char(nmeaDB *db, int index);
short   nmea_db_short(nmeaDB *db, int index);
int     nmea_db_int(nmeaDB *db, int index);
long    nmea_db_long(nmeaDB *db, int index);
float   nmea_db_float(nmeaDB *db, int index);
double  nmea_db_double(nmeaDB *db, int index);

char *  nmea_db_string(nmeaDB *db, int index);
nmeaSATINFO * nmea_db_satinfo(nmeaDB *db);

int     nmea_db_set_char(nmeaDB *db, int index, char value);
int     nmea_db_set_short(nmeaDB *db, int index, short value);
int     nmea_db_set_int(nmeaDB *db, int index, int value);
int     nmea_db_set_long(nmeaDB *db, int index, long value);
int     nmea_db_set_float(nmeaDB *db, int index, float value);
int     nmea_db_set_double(nmeaDB *db, int index, double value);
int     nmea_db_set_string(nmeaDB *db, int index, char *value);
int     nmea_db_set_stringn(nmeaDB *db, int index, char *value, int str_sz);
int     nmea_db_set_satinfo(nmeaDB *db, nmeaSATINFO *info);

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_DB_H */
