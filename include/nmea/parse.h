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

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _nmeaPARSER
{
    void *top_node;
    void *end_node;
    unsigned char *buffer;
    int buff_size;
    int buff_use;

} nmeaPARSER;

int     nmea_parser_init(nmeaPARSER *parser, int parse_mask);
void    nmea_parser_clear(nmeaPARSER *parser);
void    nmea_parser_destroy(nmeaPARSER *parser);

int     nmea_parse(
        nmeaPARSER *parser,
        const char *buff, int buff_sz,
        nmeaDB *db
        );

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_PARSE_H */
