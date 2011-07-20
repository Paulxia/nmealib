///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: log.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_LOG_H__
#define __BOX_LOG_H__

#include <string>
#include <cstdarg>

#ifdef NDEBUG
#   define BOX_DEBUG_TRACE(x)
#else
#   define BOX_DEBUG_TRACE(x) Log::trace x
#endif

namespace box
{

class BasePrefix;
struct Sentence;

///////////////////////////////////////////////////////////
// Log
///////////////////////////////////////////////////////////

class Log
{
public:
    enum LogLevel
    {
        LevelCritical   = 0x0001,
        LevelError      = 0x0002,
        LevelWarning    = 0x0004,
        LevelStatus     = 0x0008,
        LevelInfo       = 0x0010,
        LevelTrace      = 0x0020,
        LevelUser1      = 0x0100,
        LevelUser2      = 0x0200,
        LevelUser3      = 0x0400,
        LevelUser4      = 0x0800,
        LevelAll        = 0xFFFF
    };

    virtual ~Log();

    static Log * specific();
    static void specific(Log *);

    void level(int lvl);
    int level() const;
    void suppress(LogLevel lvl);

    inline bool allow(LogLevel lvl) const
    { return (0 != (level() & lvl)); }

    void log(const Sentence &data);
    void log(LogLevel level, const char *text, ...);
    void log(LogLevel level, const wchar_t *text, ...);
    void log(LogLevel level, const std::string &text);
    void log(LogLevel level, const std::wstring &text);
    void logv(LogLevel level, const char *text, va_list *vl);
    void logv(LogLevel level, const wchar_t *text, va_list *vl);

    void addChain(Log *ptr);
    void delChain(Log *ptr);

    void addPrefix(BasePrefix *p);
    void clearPrefix();

    static void critical(const char *text, ...);
    static void critical(const wchar_t *text, ...);
    static void error(const char *text, ...);
    static void error(const wchar_t *text, ...);
    static void warning(const char *text, ...);
    static void warning(const wchar_t *text, ...);
    static void info(const char *text, ...);
    static void info(const wchar_t *text, ...);
    static void status(const char *text, ...);
    static void status(const wchar_t *text, ...);
    static void trace(const char *text, ...);
    static void trace(const wchar_t *text, ...);

protected:
    Log(bool global = true);
    virtual void _dolog(const Sentence &data) = 0;

private:
    friend class Context;
    friend struct LogSeed;
    struct LogSeed *_seed;
    int _level;

};

} // namespace box

#endif // __BOX_LOG_H__
