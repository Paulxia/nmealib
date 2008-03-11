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

#include "nmea/variable.h"

#include "os.h"

#ifdef _MSC_VER
#   pragma warning(disable:4100)
#endif

#define NMEA_CONVERT_PROC_DEF(t1,t2) \
    _nmea_variable_##t1##_2_##t2

#define NMEA_CONVERT_PROC(t1,t2) \
    int _nmea_variable_##t1##_2_##t2##( \
    void *dst, nmea_idx dst_type, nmea_idx dst_sz, \
    const void *src, nmea_idx src_type, nmea_idx src_sz) { \
    *((t2 *)dst) = (t2)*((t1 *)src); \
    return sizeof(t2); }

#define NMEA_CONVERT_PROC_DATA(t) \
    int _nmea_variable_##t##_2_data( \
    void *dst, nmea_idx dst_type, nmea_idx dst_sz, \
    const void *src, nmea_idx src_type, nmea_idx src_sz) { \
    os_memcpy(dst, src, src_sz); \
    return sizeof(src_sz); }

#define NMEA_CONVERT_PROC_FRAC(t1,t2) \
    int _nmea_variable_##t1##_2_##t2##( \
    void *dst, nmea_idx dst_type, nmea_idx dst_sz, \
    const void *src, nmea_idx src_type, nmea_idx src_sz) { \
    *((t2 *)dst) = (t2)(*((t1 *)src) + .5); \
    return sizeof(t2); }


typedef int (*_nmea_variable_convertor)(
    void *dst, nmea_idx dst_type, nmea_idx dst_sz,
    const void *src, nmea_idx src_type, nmea_idx src_sz
    );

#ifdef NMEA_CONFIG_CHECK_CONVERT
const nmea_idx _nmea_variable_size[NMEA_TYPE_LAST] = {
    0,
    sizeof(char),
    sizeof(short),
    sizeof(int),
    sizeof(long),
    sizeof(float),
    sizeof(double),
    0,
    0
};
const nmea_idx _nmea_variable_min_str_sz[NMEA_TYPE_LAST] = {
    0,
    sizeof("-127"),
    sizeof("–32768"),
    sizeof("–2147483648"),
    sizeof("–2147483648"),
    100,
    200,
    0,
    0
};
#endif

int _nmea_variable_none_2_none(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                               const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return 0;
}

/**********************************************************
 * char convention
 **********************************************************/

NMEA_CONVERT_PROC(char, char)
NMEA_CONVERT_PROC(char, short)
NMEA_CONVERT_PROC(char, int)
NMEA_CONVERT_PROC(char, long)
NMEA_CONVERT_PROC(char, float)
NMEA_CONVERT_PROC(char, double)
NMEA_CONVERT_PROC_DATA(char)

int _nmea_variable_char_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                 const void *src, nmea_idx src_type, nmea_idx src_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(char) + 1)
        return -1;
#endif

    ((char *)dst)[0] = ((char *)src)[0];
    ((char *)dst)[1] = '\x0';

    return sizeof(char);
}

/**********************************************************
* short convention
**********************************************************/

NMEA_CONVERT_PROC(short, char)
NMEA_CONVERT_PROC(short, short)
NMEA_CONVERT_PROC(short, int)
NMEA_CONVERT_PROC(short, long)
NMEA_CONVERT_PROC(short, float)
NMEA_CONVERT_PROC(short, double)
NMEA_CONVERT_PROC_DATA(short)

int _nmea_variable_short_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                  const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return os_snprintf(dst, dst_sz - 1, "%d", (int)*((short *)src));
}

/**********************************************************
 * int convention
 **********************************************************/

NMEA_CONVERT_PROC(int, char)
NMEA_CONVERT_PROC(int, short)
NMEA_CONVERT_PROC(int, int)
NMEA_CONVERT_PROC(int, long)
NMEA_CONVERT_PROC(int, float)
NMEA_CONVERT_PROC(int, double)
NMEA_CONVERT_PROC_DATA(int)

int _nmea_variable_int_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return os_snprintf(dst, dst_sz - 1, "%d", *((int *)src));
}

/**********************************************************
 * long convention
 **********************************************************/

NMEA_CONVERT_PROC(long, char)
NMEA_CONVERT_PROC(long, short)
NMEA_CONVERT_PROC(long, int)
NMEA_CONVERT_PROC(long, long)
NMEA_CONVERT_PROC(long, float)
NMEA_CONVERT_PROC(long, double)
NMEA_CONVERT_PROC_DATA(long)

int _nmea_variable_long_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                 const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return os_snprintf(dst, dst_sz - 1, "%d", *((int *)src));
}

/**********************************************************
* float convention
**********************************************************/

NMEA_CONVERT_PROC_FRAC(float, char)
NMEA_CONVERT_PROC_FRAC(float, short)
NMEA_CONVERT_PROC_FRAC(float, int)
NMEA_CONVERT_PROC_FRAC(float, long)
NMEA_CONVERT_PROC(float, float)
NMEA_CONVERT_PROC(float, double)
NMEA_CONVERT_PROC_DATA(float)

int _nmea_variable_float_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                  const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return os_snprintf(dst, dst_sz - 1, "%f", (double)*((float *)src));
}

/**********************************************************
 * double convention
 **********************************************************/

NMEA_CONVERT_PROC_FRAC(double, char)
NMEA_CONVERT_PROC_FRAC(double, short)
NMEA_CONVERT_PROC_FRAC(double, int)
NMEA_CONVERT_PROC_FRAC(double, long)
NMEA_CONVERT_PROC(double, float)
NMEA_CONVERT_PROC(double, double)
NMEA_CONVERT_PROC_DATA(double)

int _nmea_variable_double_2_string(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                   const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    return os_snprintf(dst, dst_sz - 1, "%f", *((double *)src));
}

/**********************************************************
 * string convention
 **********************************************************/

int _nmea_variable_string_2_char(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                 const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    if(src_sz < sizeof(char))
        return 0;

    *(char *)dst = *(char *)src;

    return sizeof(char);
}

int _nmea_variable_string_2_short(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                  const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    *(short *)dst = (short)os_atoi((char *)src);
    return src_sz;
}

int _nmea_variable_string_2_int(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    *(int *)dst = (int)os_atoi((char *)src);
    return src_sz;
}

int _nmea_variable_string_2_long(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                 const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    *(long *)dst = (long)os_atoi((char *)src);
    return src_sz;
}

int _nmea_variable_string_2_float(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                  const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    *(float *)dst = (float)os_atof((char *)src);
    return src_sz;
}

int _nmea_variable_string_2_double(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                                   const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    *(double *)dst = (double)os_atof((char *)src);
    return src_sz;
}

/**********************************************************
 * data convention
 **********************************************************/

int _nmea_variable_data_2_data(void *dst, nmea_idx dst_type, nmea_idx dst_sz,
                               const void *src, nmea_idx src_type, nmea_idx src_sz)
{
    if(dst_sz < src_sz)
        return -1;

    os_memcpy(dst, src, src_sz);

    return src_sz;
}

const _nmea_variable_convertor _nmea_variable_convert[NMEA_TYPE_LAST][NMEA_TYPE_LAST] = {
    {   _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none,
        _nmea_variable_none_2_none
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(char,char),
        NMEA_CONVERT_PROC_DEF(char,short),
        NMEA_CONVERT_PROC_DEF(char,int),
        NMEA_CONVERT_PROC_DEF(char,long),
        NMEA_CONVERT_PROC_DEF(char,float),
        NMEA_CONVERT_PROC_DEF(char,double),
        _nmea_variable_char_2_string,
        NMEA_CONVERT_PROC_DEF(char,data)
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(short,char),
        NMEA_CONVERT_PROC_DEF(short,short),
        NMEA_CONVERT_PROC_DEF(short,int),
        NMEA_CONVERT_PROC_DEF(short,long),
        NMEA_CONVERT_PROC_DEF(short,float),
        NMEA_CONVERT_PROC_DEF(short,double),
        _nmea_variable_short_2_string,
        NMEA_CONVERT_PROC_DEF(short,data)
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(int,char),
        NMEA_CONVERT_PROC_DEF(int,short),
        NMEA_CONVERT_PROC_DEF(int,int),
        NMEA_CONVERT_PROC_DEF(int,long),
        NMEA_CONVERT_PROC_DEF(int,float),
        NMEA_CONVERT_PROC_DEF(int,double),
        _nmea_variable_int_2_string,
        NMEA_CONVERT_PROC_DEF(int,data)
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(long,char),
        NMEA_CONVERT_PROC_DEF(long,short),
        NMEA_CONVERT_PROC_DEF(long,int),
        NMEA_CONVERT_PROC_DEF(long,long),
        NMEA_CONVERT_PROC_DEF(long,float),
        NMEA_CONVERT_PROC_DEF(long,double),
        _nmea_variable_long_2_string,
        NMEA_CONVERT_PROC_DEF(long,data)
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(float,char),
        NMEA_CONVERT_PROC_DEF(float,short),
        NMEA_CONVERT_PROC_DEF(float,int),
        NMEA_CONVERT_PROC_DEF(float,long),
        NMEA_CONVERT_PROC_DEF(float,float),
        NMEA_CONVERT_PROC_DEF(float,double),
        _nmea_variable_float_2_string,
        NMEA_CONVERT_PROC_DEF(float,data)
        },
    {   _nmea_variable_none_2_none,
        NMEA_CONVERT_PROC_DEF(double,char),
        NMEA_CONVERT_PROC_DEF(double,short),
        NMEA_CONVERT_PROC_DEF(double,int),
        NMEA_CONVERT_PROC_DEF(double,long),
        NMEA_CONVERT_PROC_DEF(double,float),
        NMEA_CONVERT_PROC_DEF(double,double),
        _nmea_variable_double_2_string,
        NMEA_CONVERT_PROC_DEF(double,data)
        },
    {   _nmea_variable_none_2_none,
        _nmea_variable_string_2_char,
        _nmea_variable_string_2_short,
        _nmea_variable_string_2_int,
        _nmea_variable_string_2_long,
        _nmea_variable_string_2_float,
        _nmea_variable_string_2_double,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        },
    {   _nmea_variable_none_2_none,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data,
        _nmea_variable_data_2_data
        },
};

int nmea_variable_set(
    void *dst, nmea_idx dst_type, nmea_idx dst_sz,
    const void *src, nmea_idx src_type, nmea_idx src_sz
    )
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_type >= NMEA_TYPE_LAST || src_type >= NMEA_TYPE_LAST)
        return -1;
    if(_nmea_variable_size[dst_type] && dst_sz < _nmea_variable_size[dst_type])
        return -1;
    if(_nmea_variable_size[src_type] && src_sz < _nmea_variable_size[src_type])
        return -1;
#endif
    return (*_nmea_variable_convert[src_type][dst_type])(
        dst, dst_type, dst_sz, src, src_type, src_sz
        );
}
