///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: log.cpp 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#include "log.h"
#include "config.h"
#include "sentence.h"
#include "prefix.h"
#include "mutex.h"
#include "context.h"

#include <sstream>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cstdio>

#if defined(BOX_WIN) && (_MSC_VER >= 1400)
# pragma warning(disable: 4996) // declared deprecated
#endif

namespace box
{

///////////////////////////////////////////////////////////
// NullLog
///////////////////////////////////////////////////////////

class NullLog : public Log
{
public:
    NullLog() {}
    virtual void _dolog(const Sentence &) {}
};

///////////////////////////////////////////////////////////
// LogSeed
///////////////////////////////////////////////////////////

struct LogSeed
{
    typedef std::vector<Log *> chain_vec_t;
    chain_vec_t chain_vec;
    CombinePrefix prefix;
    int allow_levels;
    Log *parent;
    Mutex mutex;
    static Log *toplog;
    static NullLog nulllog;

    LogSeed()
        : allow_levels(Log::LevelAll)
        , parent(0)
    {}
    ~LogSeed()
    {
        LockIt lock(&mutex);

        prefix.clear();

        chain_vec_t::iterator it = chain_vec.begin();
        for(; it != chain_vec.end(); ++it)
        {
            (*it)->_seed->parent = 0;
            delete (*it);
        }
    }
};

NullLog LogSeed::nulllog;
Log * LogSeed::toplog = &LogSeed::nulllog;

///////////////////////////////////////////////////////////
// Log
///////////////////////////////////////////////////////////

Log::Log(bool global)
{
    _seed = new LogSeed;
    if(global)
        specific()->addChain(this);        
}

//=========================================================
Log::~Log()
{
    if(_seed->parent)
        _seed->parent->delChain(this);
    delete _seed;
}

//=========================================================
Log * Log::specific()
{
    return LogSeed::toplog;
}

//=========================================================
void Log::specific(Log *i)
{
    if(i)
        LogSeed::toplog = i;
}

//=========================================================
void Log::level(int lvl)
{
    LockIt lock(&_seed->mutex);
    _seed->allow_levels = lvl;
}

//=========================================================
int Log::level() const
{
    LockIt lock(&_seed->mutex);
    return _seed->allow_levels;
}

//=========================================================
void Log::suppress(LogLevel lvl)
{
    LockIt lock(&_seed->mutex);
    if(lvl <= LevelCritical)
        lvl = LevelError;
    _seed->allow_levels = ((int)lvl) - 1;
}

//=========================================================
void Log::log(LogLevel level, const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    logv(level, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::log(LogLevel level, const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    logv(level, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::log(LogLevel level, const std::string &text)
{
    Sentence sen(
        Time(), level,
        std::string(),
        text
        );
    log(sen);
}

//=========================================================
void Log::log(const Sentence &data)
{
    if(allow(data.level))
    {
        Sentence sen(data);

        if(!(_seed->parent))
            sen.tm = Time::now();

        if(!sen.prefix_mark())
        {
            std::ostringstream ss;
            _seed->prefix.log(sen.tm, sen.level, ss);
            sen.prefix = ss.str() + sen.prefix;
            sen.prefix_mark(true);
        }

        _seed->mutex.lock();
        this->_dolog(sen);
        _seed->mutex.unlock();

        LogSeed::chain_vec_t::iterator it = _seed->chain_vec.begin();
        for(; it != _seed->chain_vec.end(); ++it)
            (*it)->log(sen);
    }
}

//=========================================================
void Log::log(LogLevel level, const std::wstring &text)
{
    log(level, Sentence::wctomb(text));
}

//=========================================================
void Log::logv(LogLevel level, const char *text, va_list *vl)
{
    char buff[BOX_SPRINTF_BUFF];
    int sz = BOX_POSIX(vsnprintf)(&buff[0], BOX_SPRINTF_BUFF - 1, text, *vl);
    if(sz > 0)
        log(level, std::string(&buff[0], sz));
}

//=========================================================
void Log::logv(LogLevel level, const wchar_t *text, va_list *vl)
{
    wchar_t buff[BOX_SPRINTF_BUFF];
    int sz = BOX_POSIX(vsnwprintf)(&buff[0], BOX_SPRINTF_BUFF - 1, text, *vl);
    if(sz > 0)
        log(level, std::wstring(&buff[0], sz));
}

//=========================================================
void Log::addChain(Log *ptr)
{
    LockIt lock(&_seed->mutex);
    LogSeed::chain_vec_t::const_iterator it = std::find(
        _seed->chain_vec.begin(), _seed->chain_vec.end(), ptr
        );
    if(it == _seed->chain_vec.end())
    {
        _seed->chain_vec.push_back(ptr);
        ptr->_seed->parent = this;
    }
    else
        delete ptr;
}

//=========================================================
void Log::delChain(Log *ptr)
{
    LockIt lock(&_seed->mutex);
    LogSeed::chain_vec_t::iterator it = std::find(
        _seed->chain_vec.begin(), _seed->chain_vec.end(), ptr
        );
    if(it != _seed->chain_vec.end())
    {
        _seed->chain_vec.erase(it);
        ptr->_seed->parent = 0;
    }
}

//=========================================================
void Log::addPrefix(BasePrefix *p)
{
    LockIt lock(&_seed->mutex);
    _seed->prefix.addChain(p);
}

//=========================================================
void Log::clearPrefix()
{
    LockIt lock(&_seed->mutex);
    _seed->prefix.clear();
}

//=========================================================
void Log::critical(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelCritical, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::critical(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelCritical, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::error(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelError, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::error(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelError, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::warning(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelWarning, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::warning(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelWarning, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::info(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelInfo, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::info(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelInfo, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::status(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelStatus, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::status(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelStatus, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::trace(const char *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelTrace, text, &arg_list);
    va_end(arg_list);
}

//=========================================================
void Log::trace(const wchar_t *text, ...)
{
    va_list arg_list;
    va_start(arg_list, text);
    Log::specific()->logv(Log::LevelTrace, text, &arg_list);
    va_end(arg_list);
}

} // namespace box

#if defined(BOX_WIN) && (_MSC_VER >= 1400)
# pragma warning(default: 4996)
#endif
