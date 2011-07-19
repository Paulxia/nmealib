/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2008 Timur Sinitsyn
 * Copyright (c) 2011 Ferry Huberts
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nmea/info.h>

#include <nmea/sentence.h>

#include <string.h>

void nmea_zero_INFO(nmeaINFO *info)
{
    memset(info, 0, sizeof(nmeaINFO));
    nmea_time_now(&info->utc);
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;
}

/**
 * Determine whether a given nmeaINFO structure has a certain field.
 *
 * nmeaINFO dependencies:
 <pre>
 field/sentence GPGGA   GPGSA   GPGSV   GPRMC   GPVTG
 smask:         x       x       x       x       x
 utc:           x                       x
 sig:           x                       x
 fix:                   x               x
 PDOP:                  x
 HDOP:          x       x
 VDOP:                  x
 lat:           x                       x
 lon:           x                       x
 elv:           x
 speed:                                 x       x
 direction:                             x       x
 declination:                                   x
 satinfo:               x       x
 </pre>
 *
 * @param smask
 * the smask of a nmeaINFO structure
 * @param fieldName
 * the field name
 *
 * @return
 * - true when the nmeaINFO structure has the field
 * - false otherwise
 */
bool nmea_INFO_has_field(int smask, nmeaINFO_FIELD fieldName) {
	switch (fieldName) {
		case SMASK:
			return true;

		case UTC:
		case SIG:
		case LAT:
		case LON:
			return ((smask & (GPGGA | GPRMC)) != 0);

		case FIX:
			return ((smask & (GPGSA | GPRMC)) != 0);

		case PDOP:
		case VDOP:
			return ((smask & GPGSA) != 0);

		case HDOP:
			return ((smask & (GPGGA | GPGSA)) != 0);

		case ELV:
			return ((smask & GPGGA) != 0);

		case SPEED:
		case DIRECTION:
			return ((smask & (GPRMC | GPVTG)) != 0);

		case DECLINATION:
			return ((smask & GPVTG) != 0);

		case SATINFO:
			return ((smask & (GPGSA | GPGSV)) != 0);

		default:
			return false;
	}
}
