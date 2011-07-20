///////////////////////////////////////////////////////////
//
// Project: NmeaSDK
// Sub project: NMEA service
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __NMEASDK_SERVICE_CONTEXT_H__
#define __NMEASDK_SERVICE_CONTEXT_H__

#include <xtl/list>
#include <xtl/string>

namespace service
{
    class Session;
    class GeneratorAgent;

    ///////////////////////////////////////////////////////
    // Context
    ///////////////////////////////////////////////////////

    class Context
    {
    public:
        typedef xtl::list<Session *> sessions_t;
        typedef xtl::list<GeneratorAgent *> generators_t;
        typedef xtl::list<xtl::tstring> slist_t;

        static Context * specific();
        static void specific(Context *instance);

        Context();

        bool init(const xtl::tstring &log_cfg = _T("nmeasrv_log.cfg"));
        void done();

        void sessions(sessions_t &slist) const;
        Session * session(const xtl::tstring &name) const;

        void regGenerator(GeneratorAgent *agent);
        void generators(generators_t &glist) const;
        GeneratorAgent * generator(const xtl::tstring &name) const;

    private:
        Context(const Context &) {}
        Context & operator = (const Context &) { return *this; }
        ~Context();

        bool _initLogs(const xtl::tstring &log_cfg);
        bool _initBuiltInGen();
        bool _loadPlugins();
        bool _restoreSessions();

        friend class Session;
        void _addSession(Session *);
        void _delSession(Session *);

        friend class GeneratorAgent;
        void _unregGenerator(GeneratorAgent *);

        void _enumPorts(slist_t &plist);
        xtl::tstring _newPort();
        void _delPort(const xtl::tstring &name);

    private:
        mutable sessions_t _sessions;
        mutable generators_t _generators;
        mutable xtl::Mutex _smutex;
        mutable xtl::Mutex _gmutex;

        static Context * _specific;
    };

    inline Context * Context::specific() { return _specific; }

} // namespace service

#endif // __NMEASDK_SERVICE_CONTEXT_H__
