///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: prefix.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_PREFIX_H__
#define __BOX_PREFIX_H__

#include <vector>
#include <iostream>
#include <string>

#include "log.h"

namespace box
{

class Time;

///////////////////////////////////////////////////////////
// BasePrefix
///////////////////////////////////////////////////////////

class BasePrefix
{
public:
    virtual ~BasePrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss) = 0;

protected:
    BasePrefix() {}
    BasePrefix(const BasePrefix &) {}
    BasePrefix & operator = (const BasePrefix &) { return *this; }

};

///////////////////////////////////////////////////////////
// CombinePrefix
///////////////////////////////////////////////////////////

class CombinePrefix : public BasePrefix
{
public:
    CombinePrefix();
    virtual ~CombinePrefix();

    void addChain(BasePrefix *);
    void clear();

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);

private:
    typedef std::vector<BasePrefix *> chain_vec_t;
    chain_vec_t _chain_vec;

};

///////////////////////////////////////////////////////////
// StaticPrefix
///////////////////////////////////////////////////////////

class StaticPrefix : public BasePrefix
{
public:
    StaticPrefix(const std::string &text);

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);

private:
    std::string _text;

};

///////////////////////////////////////////////////////////
// LevelInfoPrefix
///////////////////////////////////////////////////////////

class LevelInfoPrefix : public BasePrefix
{
public:
    LevelInfoPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeDayPrefix
///////////////////////////////////////////////////////////

class TimeDayPrefix : public BasePrefix
{
public:
    TimeDayPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeMonPrefix
///////////////////////////////////////////////////////////

class TimeMonPrefix : public BasePrefix
{
public:
    TimeMonPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeYearPrefix
///////////////////////////////////////////////////////////

class TimeYearPrefix : public BasePrefix
{
public:
    TimeYearPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeHourPrefix
///////////////////////////////////////////////////////////

class TimeHourPrefix : public BasePrefix
{
public:
    TimeHourPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeMinPrefix
///////////////////////////////////////////////////////////

class TimeMinPrefix : public BasePrefix
{
public:
    TimeMinPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeSecPrefix
///////////////////////////////////////////////////////////

class TimeSecPrefix : public BasePrefix
{
public:
    TimeSecPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// TimeMSecPrefix
///////////////////////////////////////////////////////////

class TimeMSecPrefix : public BasePrefix
{
public:
    TimeMSecPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// ThreadIdPrefix
///////////////////////////////////////////////////////////

class ThreadIdPrefix : public BasePrefix
{
public:
    ThreadIdPrefix() {}

    virtual void log(
        const Time &tm,
        Log::LogLevel level,
        std::ostream &ss);
};

///////////////////////////////////////////////////////////
// MaskPrefix
///////////////////////////////////////////////////////////

class MaskPrefix : public CombinePrefix
{
public:
    MaskPrefix(const char *mask);
    MaskPrefix(const wchar_t *mask);
    void build(const char *mask);
    void build(const wchar_t *mask);
};

} // namespace box

#endif // __BOX_PREFIX_H__
