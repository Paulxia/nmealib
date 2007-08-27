/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

#ifndef __NMEA_MATH_H__
#define __NMEA_MATH_H__

#include "info.h"

#define NMEA_PI             (3.141592653589793)             /**< PI value */
#define NMEA_PI180          (NMEA_PI / 180)                 /**< PI division by 180 */
#define NMEA_EARTHRADIUS_KM (6378)                          /**< Earth's mean radius in km */
#define NMEA_EARTHRADIUS_M  (NMEA_EARTHRADIUS_KM * 1000)    /**< Earth's mean radius in m */
#define NMEA_DOP_FACTOR     (5)                             /**< Factor for translating DOP to meters */

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * degree VS radian
 */

NMEA_INLINE double nmea_degree2radian(double val)
{ return (val * NMEA_PI180); }
NMEA_INLINE double nmea_radian2degree(double val)
{ return (val / NMEA_PI180); }

/*
 * NDEG (NMEA degree)
 */

double nmea_ndeg2degree(double val);
double nmea_degree2ndeg(double val);

NMEA_INLINE double nmea_ndeg2radian(double val)
{ return nmea_degree2radian(nmea_ndeg2degree(val)); }
NMEA_INLINE double nmea_radian2ndeg(double val)
{ return nmea_degree2ndeg(nmea_radian2degree(val)); }

/*
 * DOP
 */

double nmea_calc_pdop(double hdop, double vdop);

NMEA_INLINE double nmea_dop2meters(double dop)
{ return (dop * NMEA_DOP_FACTOR); }
NMEA_INLINE double nmea_meters2dop(double meters)
{ return (meters / NMEA_DOP_FACTOR); }

/*
 * positions work
 */

void nmea_info2pos(const nmeaINFO *info, nmeaPOS *pos);
void nmea_pos2info(const nmeaPOS *pos, nmeaINFO *info);

double  nmea_distance(
        const nmeaPOS *from_pos,
        const nmeaPOS *to_pos
        );

int     nmea_move_horz(
        const nmeaPOS *start_pos,
        nmeaPOS *end_pos,
        double azimuth,
        double distance
        );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_MATH_H__ */
