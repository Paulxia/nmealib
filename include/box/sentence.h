///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_SENTENCE_H__
#define __BOX_SENTENCE_H__

#include <string>

#include "log.h"
#include "time.h"

namespace box
{

///////////////////////////////////////////////////////////
// Sentence
///////////////////////////////////////////////////////////

struct Sentence
{
    enum Flags
    {
        FlagPrefixMark  = 0x01
    };

    Time tm;
    int flags;
    Log::LogLevel level;
    std::string prefix;
    std::string message;

    inline Sentence()
    {}
    inline Sentence(
        const Time &t,
        Log::LogLevel l,
        const std::string &p,
        const std::string &m
        )
        : tm(t)
        , flags(0)
        , level(l)
        , prefix(p)
        , message(m)
    {}
    inline Sentence(const Sentence &src)
        : tm(src.tm)
        , flags(src.flags)
        , level(src.level)
        , prefix(src.prefix)
        , message(src.message)
    {}

    inline void addPrefix(const std::string &text)
    { prefix = text + prefix; }
    inline void addMessage(const std::string &text)
    { message = text + message; }

    inline void prefix_mark(bool state)
    {
        if(state)
            flags |= FlagPrefixMark;
        else
            flags &= ~FlagPrefixMark;
    }
    inline bool prefix_mark() const
    {
        return (0 != (flags & FlagPrefixMark));
    }

    inline Sentence & operator = (const Sentence &src)
    {
        tm = src.tm;
        flags = src.flags;
        level = src.level;
        prefix = src.prefix;
        message = src.message;
    }

    static std::string wctomb(const std::wstring &wc_str);
    static std::wstring mbtowc(const std::string &c_str);
    static std::string uppercase(const std::string &str);
};

} // namespace box

#endif // __BOX_SENTENCE_H__
