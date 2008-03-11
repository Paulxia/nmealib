#include <nmea/db.h>
#include <nmea/parse.h>
#include <string.h>

int main()
{
    const char *buff[] = {
        "$GPGGA,111609.14,5001.27,N,3613.06,E,3,08,0.0,10.2,M,0.0,M,0.0,0000*70\r\n",
        "$GPGSV,2,1,08,01,05,005,80,02,05,050,80,03,05,095,80,04,05,140,80*7f\r\n",
        "$GPGSV,2,2,08,05,05,185,80,06,05,230,80,07,05,275,80,08,05,320,80*71\r\n",
        "$GPGSA,A,3,01,02,03,04,05,06,07,08,00,00,00,00,0.0,0.0,0.0*3a\r\n",
        "$GPRMC,111609.14,A,5001.27,N,3613.06,E,11.2,0.0,261206,0.0,E*50\r\n",
        "$GPVTG,217.5,T,208.8,M,000.00,N,000.01,K*4C\r\n"
    };

    nmeaDB db;
    nmeaPARSER parser;

    nmeaINFO info;

    nmea_info_init(&info);

    nmea_parser_init(&parser, &info, NMEA_PT_ALL);
    nmea_parse(&parser, buff[0], (int)strlen(buff[0]));

    if(!nmea_db_hasvar(&info, NMEA_LAT))
        return -1;

    nmeaINFO gpsinfo1;

    nmea_parse(nmeastream, &gpsinfo1);

    nmeaINFO wpsinfo;
    wpsinfo.lat = 12312.123;
    ...

    nmea_zero(&final_info);

    copy_location(&final_info, &gps_info);
    copy_bearing(&final_info, &wps_info);

    

    nmeaDB nofixDB;



    if (!gpsinfo1.fix && !wpsinfo)
        nmea_generate(&nofixDB);
    else if(gpsinfo1.fix)
        nmea_generate(&gpsinfo1);



    final_info = wps_info;

    nmea_db_copy(&final_info, &gps_info, NMEA_SPEED | NMEA_DIRECTION);

    nmea_db_copy(&final_info, &gps_info, NMEA_SPEED);
    nmea_db_set(&final_info, &gps_info, NMEA_DIRECTION);

    nmeaINFO gps_info;
    nmea_parse(gpsnmea, &gps_info);

    if (gps_info.mask & NMEA_SPEED)

    info2.speed = gps_info.speed;
    info2.direction = gps_info.direction;

    NMEA_COPY_MASK(info2, info1, NMEA_SPEED NMEA_DIRECTION);

    NMEA_SPEED NMEA_DIRECTION

    /*
    short b;

    nmea_db_init(&db);

    nmea_db_set_double(&db, NMEA_LAT, 65.5);

    b = nmea_db_short(&db, NMEA_LAT);

    nmea_db_done(&db);
    */

    /*

    int it;
    nmeaINFO info;
    nmeaPARSER parser;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    for(it = 0; it < 6; ++it)
        nmea_parse(&parser, buff[it], (int)strlen(buff[it]), &info);

    nmea_parser_destroy(&parser);
    */

    return 0;
}
