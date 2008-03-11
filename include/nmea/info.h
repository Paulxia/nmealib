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

#ifndef NMEA_INFO_H
#define NMEA_INFO_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "db.h"

#ifdef NMEA_CONFIG_USEINFO

#if defined(_MSC_VER)
#   pragma message("Warning! Structure nmeaINFO is depricated, please use nmeaDB")
#endif

/**
 * Summary GPS information from all parsed packets,
 * used also for generating NMEA stream (depricated, use nmea_get, nmea_set, nmea_char, nmea_short ...)
 * @see nmea_parse
 * @see nmea_get, nmea_set
 * @see nmea_char, nmea_short, nmea_int, nmea_long, nmea_float, nmea_double, nmea_string, nmea_satinfo
 */
typedef struct _nmeaINFO
{
    nmeaDB db;

    int     smask;      /**< Mask specifying types of packages from which data have been obtained */

    nmeaTIME utc;       /**< UTC of position */

    int     sig;        /**< GPS quality indicator (0 = Invalid; 1 = Fix; 2 = Differential, 3 = Sensitive) */
    int     fix;        /**< Operating mode, used for navigation (1 = Fix not available; 2 = 2D; 3 = 3D) */

    double  PDOP;       /**< Position Dilution Of Precision */
    double  HDOP;       /**< Horizontal Dilution Of Precision */
    double  VDOP;       /**< Vertical Dilution Of Precision */

    double  lat;        /**< Latitude in NDEG - +/-[degree][min].[sec/60] */
    double  lon;        /**< Longitude in NDEG - +/-[degree][min].[sec/60] */
    double  elv;        /**< Antenna altitude above/below mean sea level (geoid) in meters */
    double  speed;      /**< Speed over the ground in kilometers/hour */
    double  direction;  /**< Track angle in degrees True */
    double  declination; /**< Magnetic variation degrees (Easterly var. subtracts from true course) */

    nmeaSATINFO satinfo; /**< Satellites information */

} nmeaINFO;

void nmea_zero_INFO(nmeaINFO *info);
int nmea_get_info(nmeaDB *db, nmeaINFO *info);
int nmea_set_info(nmeaDB *db, nmeaINFO *info);

#endif /* NMEA_CONFIG_USEINFO */

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_INFO_H */
