/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: data.h 10 2007-11-15 14:50:15Z xtimor $
 *
 */

/*! \file */

#ifndef __NMEA_PACKET_H__
#define __NMEA_PACKET_H__

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _nmeaPACKET_HANDLER
{
    struct _nmeaPACKET_HANDLER *next;
    int packet_id;
    int (*check)(const char *header, int header_sz);
    int (*scanf)(const char *nmea, int nmea_sz);
    int (*printf)(char *buff, int buff_sz);

} nmeaPACKET_HANDLER;

nmeaPACKET_HANDLER * nmea_packet_root(void);
int nmea_packet_add(nmeaPACKET_HANDLER *packet);

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_PACKET_H__ */
