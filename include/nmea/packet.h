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

#ifndef NMEA_PACKET_H
#define NMEA_PACKET_H

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

typedef struct _nmeaPACKET_HANDLER
{
    struct _nmeaPACKET_HANDLER *next;
    int packet_id;
    int (*check)(const char *header, int header_sz);
    int (*scanf)(struct _nmeaDB *db, const char *nmea, int nmea_sz);
    int (*printf)(struct _nmeaDB *db, char *buff, int buff_sz);

} nmeaPACKET_HANDLER;

nmeaPACKET_HANDLER * nmea_packet_root(void);
int nmea_packet_add(nmeaPACKET_HANDLER *packet);

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_PACKET_H */
