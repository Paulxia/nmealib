/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: packet.h 16 2007-12-12 16:57:43Z xtimor $
 *
 */

/*! \file */

#include "nmea/db.h"
#include "nmea/packet.h"

extern nmeaPACKET_HANDLER _nmea_packet_GPGGA;

nmeaPACKET_HANDLER * _nmea_packet_handler_root = 0;

nmeaPACKET_HANDLER * nmea_packet_root(void)
{
    if(!_nmea_packet_handler_root)
    {
        nmea_packet_add(&_nmea_packet_GPGGA);
    }

    return _nmea_packet_handler_root;
}

int nmea_packet_add(nmeaPACKET_HANDLER *packet)
{
    if(!packet)
        return -1;

    packet->next = _nmea_packet_handler_root;
    _nmea_packet_handler_root = packet;

    return 0;
}
