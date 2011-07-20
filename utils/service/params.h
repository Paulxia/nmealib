///////////////////////////////////////////////////////////
//
// Project: NmeaSDK
// Sub project: NMEA service
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __NMEASDK_SERVICE_PARAMS_H__
#define __NMEASDK_SERVICE_PARAMS_H__

#include <map>

#include "xtl/string.h"

namespace service
{
    ///////////////////////////////////////////////////////
    // Params
    ///////////////////////////////////////////////////////

    class Params
    {
    public:
        enum ValueType
        {
            StringValue,
            IntValue,
            DoubleValue,
            EnumValue,
            FileNameValue
        };

        struct ParamData
        {
            xtl::tstring brief;
            xtl::tstring description;
            xtl::tstring value;
            Params::ValueType type;
            xtl::tstring enumstr;

            inline ParamData(
                xtl::tstring b = xtl::tstring(),
                xtl::tstring d = xtl::tstring(),
                xtl::tstring v = xtl::tstring(),
                Params::ValueType t = Params::StringValue,
                xtl::tstring e = xtl::tstring()
                )
                : brief(b)
                , description(d)
                , value(v)
                , type(t)
                , enumstr(e)
            {}
            inline ParamData(const ParamData &data)
                : brief(data.brief)
                , description(data.description)
                , value(data.value)
                , type(data.type)
                , enumstr(data.enumstr)
            {}
            inline ParamData & operator = (const ParamData &data)
            {
                brief = data.brief;
                description = data.description;
                value = data.value;
                type = data.type;
                enumstr = data.enumstr;
                return *this;
            }
            inline bool operator == (const ParamData &data)
            {
                return
                    brief == data.brief &&
                    description == data.description &&
                    value == data.value &&
                    type == data.type &&
                    enumstr == data.enumstr;
            }
            inline bool operator != (const ParamData &data)
            {
                return !operator == (data);
            }
        };

        typedef xtl::map<xtl::tstring, ParamData> params_t;
        const static ParamData null;

        Params();
        Params(const Params &) {}
        Params & operator = (const Params &) { return *this; }
        virtual ~Params();

        bool add(
            xtl::tstring key,
            xtl::tstring brief,
            xtl::tstring description,
            xtl::tstring value,
            ValueType type = StringValue,
            xtl::tstring enumstr = xtl::tstring()
            );
        bool add(
            xtl::tstring key,
            const ParamData &data
            );
        void del(
            const xtl::tstring &key
            );
        void clear();

        const ParamData & data(const xtl::tstring &key) const;
        inline const params_t & map() const { return _params; }

    private:
        params_t _params;

    };

} // namespace service

#endif // __NMEASDK_SERVICE_PARAMS_H__
