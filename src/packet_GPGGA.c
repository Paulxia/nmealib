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
#include "os.h"
#include "tok.h"

const char _nmea_gpgga_head[] = "GPGGA";
const int _nmea_gpgga_head_sz = sizeof(_nmea_gpgga_head) - 1;

int nmea_gpgga_check(const char *header, int header_sz)
{
    if(header_sz != _nmea_gpgga_head_sz)
        return 0;
    return 0 == os_memcmp(header, _nmea_gpgga_head, header_sz);
}

int nmea_gpgga_scanf(struct _nmeaDB *db, const char *nmea, int nmea_sz)
{
}

int nmea_gpgga_printf(struct _nmeaDB *db, char *buff, int buff_sz)
{
}

nmeaPACKET_HANDLER _nmea_packet_GPGGA = {
    0, NMEA_PT_GPGGA,
    nmea_gpgga_check,
    nmea_gpgga_scanf,
    nmea_gpgga_printf
};
