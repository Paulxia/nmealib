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
