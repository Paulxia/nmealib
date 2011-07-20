///////////////////////////////////////////////////////////
//
// Project: NmeaSDK
// Sub project: NMEA service
// URL: http://nmea.jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include <xtl/algorithm>

#include "box/section.h"
#include "box/context.h"

#include "context.h"
#include "session.h"
#include "generator.h"
#include "config.h"

namespace service
{

Context * Context::_specific = 0;

Context::Context()
: _isinit(false)
{
    specific(this);
}

void Context::specific(Context *instance)
{
    if(_specific)
        box::Log::error(_T("Try specify not null context, maybe you try create another context instance!"));
    else if(!instance)
        box::Log::error(_T("Try specify null context!"));
    else
        _specific = instance;
}

bool Context::init()
{
    bool res = false;

    if(_isinit)
        box::Log::error(_T("Try initialize context in second time!"));
    else
    {
        for(;;)
        {
            if(false == _initLogs())
                break;
            if(false == _initBuiltInGen())
                break;
            if(false == _loadPlugins())
                break;
            if(false == _restoreSessions())
                break;
            res = true;
            break;
        }
    }

    return res;
}

void Context::done()
{
}

bool Context::_initLogs(const xtl::tstring &log_cfg)
{
    if(!box::Context::init(NSD_LOGCFGFILE))
        return false;
    box::Section::default_bounds(false);
    box::Log::info(_T("Start NMEA Service..."));
    return true;
}

bool Context::_initBuiltInGen()
{
    box::Section log(_T("Context::_initBuiltInGen"));
    return true;
}

bool Context::_loadPlugins()
{
    box::Section log(_T("Context::_loadPlugins"));
    return true;
}

bool Context::_restoreSessions()
{
    box::Section log(_T("Context::_restoreSessions"));

    slist_t slist;
    enumPorts(slist);
    slist_t::iterator it = slist.begin();

    for(; it != slist.end(); ++it)
    {
    }
}

void Context::sessions(Context::sessions_t &slist) const
{
    xtl::MutexLocker locker(&_smutex);
    xtl::copy(_sessions.begin(), _sessions.end(), slist.begin());
}

struct FindSessionByName
{
    xtl::tstring name;
    inline FindSessionByName(const xtl::tstring &n)
        : name(n)
    {}
    inline bool operator ()(const Session *s)
    { return (s->name() == name); }
};

Session * Context::session(const xtl::tstring &name) const
{
    Session *res = 0;
    xtl::MutexLocker locker(&_smutex);
    sessions_t::iterator it = xtl::find_if(
        _sessions.begin(), _sessions.end(), FindSessionByName(name)
        );
    if(it != _sessions.end())
        res = *it;
    return res;
}

void Context::regGenerator(GeneratorAgent *agent)
{
    if(0 != generator(agent->name()))
    {
        box::Log::error(_T("Try add generator with exists name!"));
        delete agent;
    }
    else
    {
        xtl::MutexLocker locker(&_gmutex);
        _generators.push_back(agent);
    }
}

void Context::generators(Context::generators_t &glist) const
{
    xtl::MutexLocker locker(&_gmutex);
    xtl::copy(_generators.begin(), _generators.end(), glist.begin());
}

struct FindGeneratorAgentByName
{
    xtl::tstring name;
    inline FindGeneratorAgentByName(const xtl::tstring &n)
        : name(n)
    {}
    inline bool operator ()(const GeneratorAgent *s)
    { return (s->name() == name); }
};

GeneratorAgent * Context::generator(const xtl::tstring &name) const
{
    GeneratorAgent *res = 0;
    xtl::MutexLocker locker(&_gmutex);
    generators_t::iterator it = xtl::find_if(
        _generators.begin(), _generators.end(), FindGeneratorAgentByName(name)
        );
    if(it != _generators.end())
        res = *it;
    return res;
}

void Context::addSession(Session *i)
{
    xtl::MutexLocker locker(&_smutex);
    _sessions.push_back(i);
}

void Context::delSession(Session *i)
{
    xtl::MutexLocker locker(&_smutex);
    sessions_t::iterator it = xtl::find(
        _sessions.begin(), _sessions.end(), i
        );
    if(it != _sessions.end())
        _sessions.erase(it);
}

void Context::unregGenerator(GeneratorAgent *i)
{
    xtl::MutexLocker locker(&_gmutex);
    generators_t::iterator it = xtl::find(
        _generators.begin(), _generators.end(), i
        );
    if(it != _generators.end())
        _generators.erase(it);
}

} // namespace service
