/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id$
 *
 */

#ifndef NMEA_PARSE_H
#define NMEA_PARSE_H

#include "db.h"
#include "packet.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _nmeaPARSER
{
    nmeaDB *db;
    nmeaPACKET_HANDLER *root;
    unsigned char *buffer;
    int buff_size;
    int buff_use;

} nmeaPARSER;

int     nmea_parser_init(nmeaPARSER *parser, nmeaDB *db, int parse_mask);
void    nmea_parser_clear(nmeaPARSER *parser);
void    nmea_parser_done(nmeaPARSER *parser);

int     nmea_parse(
        nmeaPARSER *parser,
        const char *buff, int buff_sz
        );

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_PARSE_H */
