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

#include "nmea/variant.h"

#include "os.h"

#ifdef _MSC_VER
#   pragma warning(disable:4100)
#endif

#define NMEA_CONVERT_PROC_DEF(t1,t2) \
    _nmea_variant_##t1##_2_##t2

#ifdef NMEA_CONFIG_CHECK_CONVERT
#   define NMEA_CONVERT_PROC(t1,t2) \
    int _nmea_variant_##t1##_2_##t2##(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    if(dst_sz < sizeof(t2)) return -1; \
    *((t2 *)dst) = (t2)src->v##t1; \
    return sizeof(t2); }
#else
#   define NMEA_CONVERT_PROC(t1,t2) \
    int _nmea_variant_##t1##_2_##t2##(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    *((t2 *)dst) = (t2)src->v##t1; \
    return sizeof(t2); }
#endif

#ifdef NMEA_CONFIG_CHECK_CONVERT
#   define NMEA_CONVERT_PROC_DATA(t) \
    int _nmea_variant_##t##_2_data(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    if(dst_sz < sizeof(t)) return -1; \
    os_memcpy(dst, &src->vchar, sizeof(t)); \
    return sizeof(t); }
#else
#   define NMEA_CONVERT_PROC_DATA(t) \
    int _nmea_variant_##t##_2_data(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    os_memcpy(dst, &src->vchar, sizeof(t)); \
    return sizeof(t); }
#endif

#ifdef NMEA_CONFIG_CHECK_CONVERT
#   define NMEA_CONVERT_PROC_FRAC(t1,t2) \
    int _nmea_variant_##t1##_2_##t2##(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    if(dst_sz < sizeof(t2)) return -1; \
    *((t2 *)dst) = (t2)(src->v##t1 + .5); \
    return sizeof(t2); }
#else
#   define NMEA_CONVERT_PROC_FRAC(t1,t2) \
    int _nmea_variant_##t1##_2_##t2##(nmeaVARIANT *src, void *dst, unsigned int dst_sz) { \
    *((t2 *)dst) = (t2)(src->v##t1 + .5); \
    return sizeof(t2); }
#endif


typedef int (*_nmea_variant_convertor)(nmeaVARIANT *, void *, unsigned int);

const unsigned int _nmea_variant_union_sz = sizeof(nmeaVARIANT) - (sizeof(unsigned int) * 2);

#ifdef NMEA_CONFIG_CHECK_CONVERT
const unsigned int _nmea_variant_size[NMEA_TYPE_LAST] = {
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
const unsigned int _nmea_variant_min_str_sz[NMEA_TYPE_LAST] = {
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

void nmea_variant_zero(nmeaVARIANT *var)
{
    os_memset(var, 0, sizeof(nmeaVARIANT));
}

void nmea_variant_clear(nmeaVARIANT *var)
{
    if(var->type >= NMEA_STRING)
        os_free(var->vdata);
    var->type = NMEA_NONE;
}

int nmea_variant_copy(nmeaVARIANT *src, nmeaVARIANT *dst)
{
    if(dst->type >= NMEA_STRING)
        os_free(dst->vdata);

    if(src->type < NMEA_STRING)
        *dst = *src;
    else
    {
        dst->type = src->type;
        dst->size = src->size;

        dst->vdata = os_malloc(src->size);

        if(!dst->vdata)
        {
            dst->type = NMEA_NONE;
            return -1;
        }

        os_memcpy(dst->vdata, src->vdata, src->size);
    }

    return 0;
}

int nmea_variant_set(nmeaVARIANT *dst, unsigned int vtype, void *src, unsigned int src_sz)
{
    if(dst->type >= NMEA_STRING)
        os_free(dst->vdata);

    dst->type = vtype;

#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(vtype >= NMEA_TYPE_LAST || (_nmea_variant_size[vtype] && src_sz != _nmea_variant_size[vtype]))
        return -1;
    else
#endif
    if(vtype < NMEA_STRING)
        os_memcpy(&dst->vchar, src, src_sz);
    else
    {
        dst->vdata = os_malloc(src_sz);

        if(!dst->vdata)
        {
            dst->type = NMEA_NONE;
            return -1;
        }

        os_memcpy(dst->vdata, src, src_sz);

        dst->size = src_sz;
    }

    return 0;
}

int _nmea_variant_none_2_none(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    os_memset(&dst, 0, dst_sz);
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

int _nmea_variant_char_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(char) + 1)
        return -1;
#endif

    ((char *)dst)[0] = src->vchar;
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

int _nmea_variant_short_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    return os_snprintf(dst, dst_sz, "%d", (int)src->vshort);
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

int _nmea_variant_int_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    return os_snprintf(dst, dst_sz, "%d", (int)src->vint);
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

int _nmea_variant_long_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    return os_snprintf(dst, dst_sz, "%d", (int)src->vlong);
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

int _nmea_variant_float_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    return os_snprintf(dst, dst_sz, "%f", (double)src->vfloat);
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

int _nmea_variant_double_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    return os_snprintf(dst, dst_sz, "%f", src->vdouble);
}

/**********************************************************
 * string convention
 **********************************************************/

int _nmea_variant_string_2_char(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(char))
        return -1;
#endif

    if(src->size < sizeof(char))
        return 0;

    *(char *)dst = *(char *)src->vdata;

    return sizeof(char);
}

int _nmea_variant_string_2_short(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(short))
        return -1;
#endif

    *(short *)dst = (short)os_atoi((char *)src->vdata);

    return src->size;
}

int _nmea_variant_string_2_int(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(int))
        return -1;
#endif

    *(int *)dst = (int)os_atoi((char *)src->vdata);

    return src->size;
}

int _nmea_variant_string_2_long(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(long))
        return -1;
#endif

    *(long *)dst = (long)os_atoi((char *)src->vdata);

    return src->size;
}

int _nmea_variant_string_2_float(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(float))
        return -1;
#endif

    *(float *)dst = (float)os_atof((char *)src->vdata);

    return src->size;
}

int _nmea_variant_string_2_double(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(dst_sz < sizeof(double))
        return -1;
#endif

    *(double *)dst = (double)os_atof((char *)src->vdata);

    return src->size;
}

int _nmea_variant_string_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    unsigned int tsz;

    dst_sz -= 1;
    tsz = (src->size > dst_sz)?dst_sz:src->size;

    os_memcpy(dst, src->vdata, tsz);
    ((char *)dst)[tsz] = '\x0';

    return tsz;
}

int _nmea_variant_string_2_data(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    int tsz = (src->size > dst_sz)?dst_sz:src->size;
    os_memcpy(dst, src->vdata, tsz);
    return tsz;
}

/**********************************************************
 * data convention
 **********************************************************/

int _nmea_variant_data_2_any(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    unsigned int src_sz = (src->size > dst_sz)?dst_sz:src->size;

    os_memcpy(dst, src->vdata, src_sz);

    return src_sz;
}

int _nmea_variant_data_2_string(nmeaVARIANT *src, void *dst, unsigned int dst_sz)
{
    unsigned int src_sz;

    dst_sz -= 1;
    src_sz = (src->size > dst_sz)?dst_sz:src->size;

    os_memcpy(dst, src->vdata, src_sz);
    ((char *)dst)[src_sz] = '\x0';

    return src_sz;
}

const _nmea_variant_convertor _nmea_variant_convert[NMEA_TYPE_LAST][NMEA_TYPE_LAST] = {
    {   _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none,
        _nmea_variant_none_2_none
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(char,char),
        NMEA_CONVERT_PROC_DEF(char,short),
        NMEA_CONVERT_PROC_DEF(char,int),
        NMEA_CONVERT_PROC_DEF(char,long),
        NMEA_CONVERT_PROC_DEF(char,float),
        NMEA_CONVERT_PROC_DEF(char,double),
        _nmea_variant_char_2_string,
        NMEA_CONVERT_PROC_DEF(char,data)
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(short,char),
        NMEA_CONVERT_PROC_DEF(short,short),
        NMEA_CONVERT_PROC_DEF(short,int),
        NMEA_CONVERT_PROC_DEF(short,long),
        NMEA_CONVERT_PROC_DEF(short,float),
        NMEA_CONVERT_PROC_DEF(short,double),
        _nmea_variant_short_2_string,
        NMEA_CONVERT_PROC_DEF(short,data)
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(int,char),
        NMEA_CONVERT_PROC_DEF(int,short),
        NMEA_CONVERT_PROC_DEF(int,int),
        NMEA_CONVERT_PROC_DEF(int,long),
        NMEA_CONVERT_PROC_DEF(int,float),
        NMEA_CONVERT_PROC_DEF(int,double),
        _nmea_variant_int_2_string,
        NMEA_CONVERT_PROC_DEF(int,data)
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(long,char),
        NMEA_CONVERT_PROC_DEF(long,short),
        NMEA_CONVERT_PROC_DEF(long,int),
        NMEA_CONVERT_PROC_DEF(long,long),
        NMEA_CONVERT_PROC_DEF(long,float),
        NMEA_CONVERT_PROC_DEF(long,double),
        _nmea_variant_long_2_string,
        NMEA_CONVERT_PROC_DEF(long,data)
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(float,char),
        NMEA_CONVERT_PROC_DEF(float,short),
        NMEA_CONVERT_PROC_DEF(float,int),
        NMEA_CONVERT_PROC_DEF(float,long),
        NMEA_CONVERT_PROC_DEF(float,float),
        NMEA_CONVERT_PROC_DEF(float,double),
        _nmea_variant_float_2_string,
        NMEA_CONVERT_PROC_DEF(float,data)
        },
    {   _nmea_variant_none_2_none,
        NMEA_CONVERT_PROC_DEF(double,char),
        NMEA_CONVERT_PROC_DEF(double,short),
        NMEA_CONVERT_PROC_DEF(double,int),
        NMEA_CONVERT_PROC_DEF(double,long),
        NMEA_CONVERT_PROC_DEF(double,float),
        NMEA_CONVERT_PROC_DEF(double,double),
        _nmea_variant_double_2_string,
        NMEA_CONVERT_PROC_DEF(double,data)
        },
    {   _nmea_variant_none_2_none,
        _nmea_variant_string_2_char,
        _nmea_variant_string_2_short,
        _nmea_variant_string_2_int,
        _nmea_variant_string_2_long,
        _nmea_variant_string_2_float,
        _nmea_variant_string_2_double,
        _nmea_variant_string_2_string,
        _nmea_variant_string_2_data,
        },
    {   _nmea_variant_none_2_none,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_any,
        _nmea_variant_data_2_string,
        _nmea_variant_data_2_any
        },
};

int nmea_variant_get(nmeaVARIANT *src, unsigned int vtype, void *dst, unsigned int dst_sz)
{
#ifdef NMEA_CONFIG_CHECK_CONVERT
    if(vtype >= NMEA_TYPE_LAST)
        return -1;
#endif
    return (*_nmea_variant_convert[src->type][vtype])(src, dst, dst_sz);
}
