/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: parse.h 16 2007-12-12 16:57:43Z xtimor $
 *
 */

#include "nmea/parse.h"
#include "nmea/db.h"
#include "nmea/packet.h"
#include "os.h"
#include "tok.h"

typedef struct _nmeaPREPARSE
{
    const char *head;
    int head_sz;
    const char *body;
    int body_sz;
    int crc;

} nmeaPREPARSE;

int _nmea_preparse(const char *buff, int buff_sz, nmeaPREPARSE *pp)
{
    static const int tail_sz = 3 /* *[CRC] */ + 2 /* \r\n */;

    const char *end_buff = buff + buff_sz;
    int nread = 0, crc = 0;
    int parse_head = 1;

    NMEA_ASSERT(buff && pp);

    if(*buff != '$')
        return 0;

    pp->head = buff += 1;

    for(;buff < end_buff; ++buff, ++nread)
    {
        if('$' == *buff)
        {
            buff = 0;
            break;
        }
        else if(',' == *buff && parse_head)
        {
            parse_head = 0;
            pp->head_sz = (int)(buff - pp->head);
            crc ^= (int)*buff;
        }
        else if('*' == *buff)
        {
            pp->body_sz = (int)(buff - (pp->head + pp->head_sz));

            if(buff + tail_sz <= end_buff && '\r' == buff[3] && '\n' == buff[4])
            {
                pp->crc = nmea_atoi(buff + 1, 2, 16);
                nread = buff_sz - (int)(end_buff - (buff + tail_sz));
                if(pp->crc != crc)
                {
                    pp->crc = -1;
                    buff = 0;
                }
            }

            break;
        }
        else
            crc ^= (int)*buff;
    }

    if(pp->crc < 0 && buff)
        nread = 0;

    return nread;
}

int nmea_parser_init(nmeaPARSER *parser, nmeaDB *db, int parse_mask)
{
    nmeaPACKET_HANDLER *gp = nmea_packet_root();
    nmeaPACKET_HANDLER *lp = 0;

    os_memset(parser, 0, sizeof(nmeaPARSER));

    while(gp)
    {
        if(0 != (gp->packet_id & parse_mask))
        {
            if(0 == lp)
                parser->root = gp;
            else
                lp->next = gp;

            lp = gp;
        }

        gp = gp->next;
    }

    if(0 == parser->root)
        return -1;

    if(0 == (parser->buffer = (unsigned char *)os_malloc(NMEA_PARSER_BUFSZ)))
        return -1;

    parser->db = db;
    parser->buff_size = NMEA_PARSER_BUFSZ;
    parser->buff_use = 0;

    return 0;
}

void nmea_parser_clear(nmeaPARSER *parser)
{
    os_memset(parser->buffer, 0, parser->buff_size);
    parser->buff_use = 0;
}

void nmea_parser_done(nmeaPARSER *parser)
{
    os_free(parser->buffer);
    os_memset(parser, 0, sizeof(nmeaPARSER));
}

int _nmea_real_parse(
        nmeaPARSER *parser,
        const char *buff, int buff_sz,
        nmeaDB *db
        )
{
    nmeaPREPARSE preparse;
    nmeaPACKET_HANDLER *hdl;
    int nparsed = 0, sen_sz;

    NMEA_ASSERT(parser && parser->buffer);

    /* add */
    if(parser->buff_use + buff_sz >= parser->buff_size)
        nmea_parser_clear(parser);

    os_memcpy(parser->buffer + parser->buff_use, buff, buff_sz);
    parser->buff_use += buff_sz;

    /* parse */
    for(;;)
    {
        sen_sz = _nmea_preparse(
            (const char *)parser->buffer + nparsed,
            (int)parser->buff_use - nparsed, &preparse);

        if(!sen_sz)
        {
            if(nparsed)
                memcpy(
                parser->buffer,
                parser->buffer + nparsed,
                parser->buff_use -= nparsed);
            break;
        }
        else if(preparse.crc >= 0)
        {
            hdl = parser->root;

            while(hdl)
            {
                if(hdl->check(preparse.head, preparse.head_sz))
                {
                    hdl->scanf(db, preparse.body, preparse.body_sz);
                    break;
                }
            }
        }

        nparsed += sen_sz;
    }

    return nparsed;
}

int nmea_parse(
        nmeaPARSER *parser,
        const char *buff, int buff_sz
        )
{
    int nparse, nparsed = 0;

    do
    {
        if(buff_sz > parser->buff_size)
            nparse = parser->buff_size;
        else
            nparse = buff_sz;

        nparsed += _nmea_real_parse(
            parser, buff, nparse, parser->db
            );

        buff_sz -= nparse;

    } while(buff_sz);

    return nparsed;
}
