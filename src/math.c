///////////////////////////////////////////////////////////
//
// NMEA library
// URL: http://nmea.sourceforge.net
// Author: Tim (xtimor@gmail.com)
// Licence: http://www.gnu.org/licenses/lgpl.html
// $Id$
//
///////////////////////////////////////////////////////////

/*! \file math.h */

#include <math.h>
#include <float.h>

#include "nmea/math.h"

/**
 * \fn nmea_degree2radian
 * \brief Convert degree to radian
 */

/**
 * \fn nmea_radian2degree
 * \brief Convert radian to degree
 */

/**
 * \brief Convert NDEG (NMEA degree) to fractional degree
 */
double nmea_ndeg2degree(double val)
{
    double deg = ((int)(val / 100));
    val = deg + (val - deg * 100) / 60;
    return val;
}

/**
 * \brief Convert fractional degree to NDEG (NMEA degree)
 */
double nmea_degree2ndeg(double val)
{
    double int_part;
    double fra_part;
    fra_part = modf(val, &int_part);
    val = int_part * 100 + fra_part * 60;
    return val;
}

/**
 * \fn nmea_ndeg2radian
 * \brief Convert NDEG (NMEA degree) to radian
 */

/**
 * \fn nmea_radian2ndeg
 * \brief Convert radian to NDEG (NMEA degree)
 */

/**
 * \brief Calculate PDOP (Position Dilution Of Precision) factor
 */
double nmea_calc_pdop(double hdop, double vdop)
{
    return sqrt(pow(hdop, 2) + pow(vdop, 2));
}

/**
 * \brief Calculate distance between two points
 * \return Distance in meters
 */
double nmea_distance(
        const nmeaPOS *from_pos,    ///< From position in radians
        const nmeaPOS *to_pos       ///< To position in radians
        )
{
    double dist = ((double)NMEA_EARTHRADIUS_M) * acos(
        sin(to_pos->lat) * sin(from_pos->lat) +
        cos(to_pos->lat) * cos(from_pos->lat) * cos(to_pos->lon - from_pos->lon)
        );
    return dist;
}

/**
 * \brief Horizontal move of point position
 */
int nmea_move_horz(
    const nmeaPOS *start_pos,   ///< Start position in radians
    nmeaPOS *end_pos,           ///< Result position in radians
    double azimuth,             ///< Azimuth (degree) [0, 359]
    double distance             ///< Distance (km)
    )
{
    nmeaPOS p1 = *start_pos;
    int RetVal = 1;

    distance /= NMEA_EARTHRADIUS_KM; // Angular distance covered on earth's surface
    azimuth = nmea_degree2radian(azimuth);

    end_pos->lat = asin(
        sin(p1.lat) * cos(distance) + cos(p1.lat) * sin(distance) * cos(azimuth));
    end_pos->lon = p1.lon + atan2(
        sin(azimuth) * sin(distance) * cos(p1.lat), cos(distance) - sin(p1.lat) * sin(end_pos->lat));

    if(_isnan(end_pos->lat) || _isnan(end_pos->lon))
    {
        end_pos->lat = 0; end_pos->lon = 0;
        RetVal = 0;
    }

    return RetVal;
}

/**
 * \brief Convert position from INFO to radians position
 */
void nmea_info2pos(const nmeaINFO *info, nmeaPOS *pos)
{
    pos->lat = nmea_ndeg2radian(info->lat);
    pos->lon = nmea_ndeg2radian(info->lon);
}

/**
 * \brief Convert radians position to INFOs position
 */
void nmea_pos2info(const nmeaPOS *pos, nmeaINFO *info)
{
    info->lat = nmea_radian2ndeg(pos->lat);
    info->lon = nmea_radian2ndeg(pos->lon);
}
