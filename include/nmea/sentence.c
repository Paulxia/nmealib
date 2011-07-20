///////////////////////////////////////////////////////////
//
// NMEA library
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: sentence.c 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include <memory.h>

#include "sentence.h"

void nmea_zero_GPGGA(nmeaGPGGA *pack)
{
    memset(pack, 0, sizeof(nmeaGPGGA));
    nmea_time_now(&pack->utc_time);
    pack->ns = 'N';
    pack->ew = 'E';
    pack->elv_units = 'M';
    pack->diff_units = 'M';
}

void nmea_zero_GPGSA(nmeaGPGSA *pack)
{
    memset(pack, 0, sizeof(nmeaGPGSA));
    pack->fix_mode = 'A';
    pack->fix_type = NMEA_FIX_BAD;
}

void nmea_zero_GPGSV(nmeaGPGSV *pack)
{
    memset(pack, 0, sizeof(nmeaGPGSV));
}

void nmea_zero_GPRMC(nmeaGPRMC *pack)
{
    memset(pack, 0, sizeof(nmeaGPRMC));
    nmea_time_now(&pack->utc);
    pack->status = 'V';
    pack->ns = 'N';
    pack->ew = 'E';
    pack->declin_ew = 'E';
}

void nmea_zero_GPVTG(nmeaGPVTG *pack)
{
    memset(pack, 0, sizeof(nmeaGPVTG));
    pack->dir_t = 'T';
    pack->dec_m = 'M';
    pack->spn_n = 'N';
    pack->spk_k = 'K';
}
