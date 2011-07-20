///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: logger.h 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __BOX_LOGGER_H__
#define __BOX_LOGGER_H__

#include "log.h"

namespace box
{

class Params;

///////////////////////////////////////////////////////////
// Logger
///////////////////////////////////////////////////////////

class Logger : public Log
{
protected:
    Logger(
        const std::string &name, const Params &p,
        bool global = true
        );

private:
    void _applyDefaultParams(
        const std::string &name, const Params &p
        );
};

///////////////////////////////////////////////////////////
// LoggerAgent
///////////////////////////////////////////////////////////

class LoggerAgent
{
public:
    virtual Logger * create(
        const std::string &name, const Params &
        ) = 0;
};

///////////////////////////////////////////////////////////
// TLogAgent
///////////////////////////////////////////////////////////

template <class _LoggerTy>
class TLoggerAgent : public LoggerAgent
{
public:
    virtual Logger * create(
        const std::string &name, const Params &p
        )
    {
        return new _LoggerTy(name, p);
    }
};

///////////////////////////////////////////////////////////
// StreamLogger
///////////////////////////////////////////////////////////

class StreamLogger : public Logger
{
public:
    StreamLogger(
        std::ostream *stream,
        bool own = false,
        bool global = true
        );
    StreamLogger(
        const std::string &name,
        const Params &params,
        bool global = true
        );
    virtual ~StreamLogger();

protected:
    virtual void _dolog(const Sentence &data);

private:
    std::ostream *_stream;
    bool _own;

};

///////////////////////////////////////////////////////////
// FileLogger
///////////////////////////////////////////////////////////

class FileLogger : public Logger
{
public:
    FileLogger(
        const char *file_name,
        bool always_open = false,
        bool global = true
        );
    FileLogger(
        const wchar_t *file_name,
        bool always_open = false,
        bool global = true
        );
    FileLogger(
        const std::string &name,
        const Params &params,
        bool global = true
        );
    virtual ~FileLogger();

protected:
    virtual void _dolog(const Sentence &data);

private:
    struct FileLoggerSeed *_seed;

};

} // namespace box

#endif // __BOX_LOGGER_H__
