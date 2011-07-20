///////////////////////////////////////////////////////////
//
// Project: NmeaSDK
// Sub project: NMEA service
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __NMEASDK_SERVICE_SESSION_H__
#define __NMEASDK_SERVICE_SESSION_H__

#include <xtl/thread>

#include "params.h"

namespace service
{
    class Generator;

    ///////////////////////////////////////////////////////
    // Session
    ///////////////////////////////////////////////////////

    class Session
    {
    public:
        Session();
        ~Session();

        const xtl::tstring & name() const;

        bool create();
        bool attach(const xtl::tstring & port);
        void done();
        bool isAttached();

        bool gset(
            const xtl::tstring &name,
            const Params &params
            );
        xtl::tstring gname() const;
        Params gparams() const;
        void greset();

    private:
        xtl::tstring _name;
        Generator *_generator;
        xtl::thread _thread;

    private:
        Session(const Session &) {}
        Session & operator = (const Session &) { return *this; }

    };

} // namespace service

#endif // __NMEASDK_SERVICE_SESSION_H__
