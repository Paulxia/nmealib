///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: prefix.cpp 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#include "prefix.h"
#include "log.h"
#include "time.h"
#include "sentence.h"
#include "config.h"

#include <algorithm>
#include <iomanip>
#include <ctype.h>

#ifdef BOX_WIN
#   include <windows.h>
#else
#   include <pthread.h>
#endif

namespace box
{

///////////////////////////////////////////////////////////
// CombinePrefix
///////////////////////////////////////////////////////////

CombinePrefix::CombinePrefix()
{
}

//=========================================================
CombinePrefix::~CombinePrefix()
{
    clear();
}

//=========================================================
void CombinePrefix::addChain(BasePrefix *ptr)
{
    chain_vec_t::const_iterator it = std::find(
        _chain_vec.begin(), _chain_vec.end(), ptr
        );
    if(it == _chain_vec.end())
        _chain_vec.push_back(ptr);
    else
        delete ptr;
}

//=========================================================
void CombinePrefix::clear()
{
    chain_vec_t::iterator it = _chain_vec.begin();
    for(; it != _chain_vec.end(); ++it)
        delete (*it);
    _chain_vec.clear();
}

//=========================================================
void CombinePrefix::log(const Time &tm, Log::LogLevel level, std::ostream &ss)
{
    chain_vec_t::iterator it = _chain_vec.begin();
    for(; it != _chain_vec.end(); ++it)
        (*it)->log(tm, level, ss);
}

///////////////////////////////////////////////////////////
// StaticPrefix
///////////////////////////////////////////////////////////

StaticPrefix::StaticPrefix(const std::string &text)
    : _text(text)
{
}

//=========================================================
void StaticPrefix::log(const Time &, Log::LogLevel, std::ostream &ss)
{
    ss << _text;
}

///////////////////////////////////////////////////////////
// LevelInfoPrefix
///////////////////////////////////////////////////////////

void LevelInfoPrefix::log(const Time &, Log::LogLevel level, std::ostream &ss)
{
    switch(level)
    {
    case Log::LevelCritical:
        ss << 'C'; break;
    case Log::LevelError:
        ss << 'E'; break;
    case Log::LevelWarning:
        ss << 'W'; break;
    case Log::LevelInfo:
        ss << 'i'; break;
    case Log::LevelStatus:
        ss << 's'; break;
    case Log::LevelTrace:
        ss << 't'; break;
    default:
        ss << '*'; break;
    };
}

///////////////////////////////////////////////////////////
// TimeDayPrefix
///////////////////////////////////////////////////////////

void TimeDayPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(2) << (int)tm.day();
}

///////////////////////////////////////////////////////////
// TimeMonPrefix
///////////////////////////////////////////////////////////

void TimeMonPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(2) << (int)tm.month();
}

///////////////////////////////////////////////////////////
// TimeYearPrefix
///////////////////////////////////////////////////////////

void TimeYearPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(4) << (int)tm.year();
}

///////////////////////////////////////////////////////////
// TimeHourPrefix
///////////////////////////////////////////////////////////

void TimeHourPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(2) << (int)tm.hour();
}

///////////////////////////////////////////////////////////
// TimeMinPrefix
///////////////////////////////////////////////////////////

void TimeMinPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(2) << (int)tm.min();
}

///////////////////////////////////////////////////////////
// TimeSecPrefix
///////////////////////////////////////////////////////////

void TimeSecPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(2) << (int)tm.sec();
}

///////////////////////////////////////////////////////////
// TimeMSecPrefix
///////////////////////////////////////////////////////////

void TimeMSecPrefix::log(const Time &tm, Log::LogLevel, std::ostream &ss)
{
    ss << std::setfill('0') << std::setw(3) << (int)tm.milliseconds();
}

///////////////////////////////////////////////////////////
// ThreadIdPrefix
///////////////////////////////////////////////////////////

void ThreadIdPrefix::log(const Time &, Log::LogLevel, std::ostream &ss)
{
    unsigned int tid = 0;

#ifdef BOX_WIN
    tid = (unsigned int)GetCurrentThreadId();
#else
#   tid = (unsigned int)pthread_self();
#endif

    ss
        << "0x" << std::uppercase << std::setfill('0') << std::setw(8)
        << std::hex << tid;
}

///////////////////////////////////////////////////////////
// MaskPrefix
///////////////////////////////////////////////////////////

MaskPrefix::MaskPrefix(const char *mask)
{
    build(mask);
}

//=========================================================
MaskPrefix::MaskPrefix(const wchar_t *mask)
{
    build(mask);
}

//=========================================================
void MaskPrefix::build(const char *mask)
{
    clear();

    std::string ptxt;

    while(*mask)
    {
        if(*mask == '%' && mask[1] != 0)
        {
            if(!ptxt.empty())
            {
                addChain(new StaticPrefix(ptxt));
                ptxt.clear();
            }

            switch(mask[1])
            {
            case 'D':
                addChain(new TimeDayPrefix);
                break;
            case 'M':
                addChain(new TimeMonPrefix);
                break;
            case 'Y':
                addChain(new TimeYearPrefix);
                break;
            case 'h':
                addChain(new TimeHourPrefix);
                break;
            case 'm':
                addChain(new TimeMinPrefix);
                break;
            case 's':
                addChain(new TimeSecPrefix);
                break;
            case 'x':
                addChain(new TimeMSecPrefix);
                break;
            case 'l':
                addChain(new LevelInfoPrefix);
                break;
            case 't':
                addChain(new ThreadIdPrefix);
                break;
            default:
                addChain(new StaticPrefix(std::string("%") + mask[1]));
                break;
            };
            ++mask;
        }
        else
            ptxt += *mask;

        ++mask;
    }

    if(!ptxt.empty())
        addChain(new StaticPrefix(ptxt));
}

//=========================================================
void MaskPrefix::build(const wchar_t *mask)
{
    build(Sentence::wctomb(mask).c_str());
}

} // namespace box
