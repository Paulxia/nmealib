///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: logger.cpp 33 2007-04-10 12:41:28Z xtimor $
//
///////////////////////////////////////////////////////////

#include <sstream>

#include "logger.h"
#include "prefix.h"
#include "context.h"
#include "sentence.h"
#include "config.h"

#ifdef BOX_WIN
#   include <windows.h>
#else
#   include <fstream>
#endif

namespace box
{

///////////////////////////////////////////////////////////
// Logger
///////////////////////////////////////////////////////////

Logger::Logger(
    const std::string &name, const Params &p,
    bool global
    )
    : Log(global)
{
    _applyDefaultParams(name, p);
}

void Logger::_applyDefaultParams(
    const std::string &name, const Params &params
    )
{
    // level
    {
        std::string key = name + ".level";
        if(params.hasKey(key))
        {
            int allow = 0;
            bool supp = false;
            std::string val = Sentence::uppercase(params.getParam(key));

            if(!val.empty() && val[0] == '&')
            {
                supp = true;
                val.erase(0, 1);
            }

            if(val == "CRITICAL")
                allow = LevelCritical;
            else if(val == "ERROR")
                allow = LevelError;
            else if(val == "WARNING")
                allow = LevelWarning;
            else if(val == "STATUS")
                allow = LevelStatus;
            else if(val == "INFO")
                allow = LevelInfo;
            else if(val == "TRACE")
                allow = LevelTrace;
            else if(val == "ALL" || val == "*")
                allow = LevelAll;

            if(!supp)
                level(allow);
            else
                suppress((LogLevel)allow);
        }
    }

    // prefix
    {
        std::string key = name + ".prefix";
        if(params.hasKey(key))
        {
            clearPrefix();
            addPrefix(
                new MaskPrefix(params.getParam(key).c_str())
                );
        }
    }
}

///////////////////////////////////////////////////////////
// StreamLogger
///////////////////////////////////////////////////////////

StreamLogger::StreamLogger(std::ostream *stream, bool own, bool global)
: Logger("StreamLogger", Params(), global)
, _stream(stream)
, _own(own)
{
}

//=========================================================
StreamLogger::StreamLogger(const std::string &name, const Params &params, bool global)
: Logger(name, params, global)
, _stream(0)
, _own(false)
{
    std::string key = name + ".stream";
    if(params.hasKey(key))
    {
        std::string val = Sentence::uppercase(params.getParam(key));

        if(val == "CONSOLE")
            _stream = &std::cout;
        else if(val == "TTY")
            _stream = &std::cout;
        else if(val == "COUT")
            _stream = &std::cout;
        else if(val == "CLOG")
            _stream = &std::clog;
        else if(val == "CERR")
            _stream = &std::cerr;
    }
}

//=========================================================
StreamLogger::~StreamLogger()
{
    if(_own && _stream)
        delete _stream;
}

//=========================================================
void StreamLogger::_dolog(const Sentence &data)
{
    if(_stream)
    {
        if(!data.prefix.empty())
            *_stream << data.prefix << ' ';
        *_stream << data.message << std::endl;
    }
}

///////////////////////////////////////////////////////////
// FileLoggerSeed
///////////////////////////////////////////////////////////

struct FileLoggerSeed
{
    std::string file_name;
    bool always_open;

#ifdef BOX_WIN
    HANDLE peer;

    bool open()
    {
        if(file_name.empty())
            return false;
        peer = ::CreateFileW(
            Sentence::mbtowc(file_name).c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if(INVALID_HANDLE_VALUE == peer)
            peer = NULL;
        return NULL != peer;
    }
    bool isopen()
    {
        return (NULL != peer);
    }
    void close()
    {
        if(NULL != peer)
        {
            ::CloseHandle(peer);
            peer = NULL;
        }
    }
    bool lock()
    {
#ifdef LOCKFILE_EXCLUSIVE_LOCK
        OVERLAPPED overlapped;
        overlapped.Offset = 0;
        overlapped.OffsetHigh = 0;
        overlapped.hEvent = 0;

        return (TRUE == ::LockFileEx(
            peer, LOCKFILE_EXCLUSIVE_LOCK, 0, UINT_MAX, UINT_MAX, &overlapped
            ));
#else
        return true;
#endif
    }
    bool unlock()
    {
#ifdef LOCKFILE_EXCLUSIVE_LOCK
        OVERLAPPED overlapped;
        overlapped.Offset = 0;
        overlapped.OffsetHigh = 0;
        overlapped.hEvent = 0;

        return (TRUE == ::UnlockFileEx(
            peer, 0, UINT_MAX, UINT_MAX, &overlapped
            ));
#else
        return true;
#endif
    }
    bool write(const char *str, int size)
    {
        if(INVALID_SET_FILE_POINTER == SetFilePointer(peer, 0, NULL, FILE_END))
            return false;
        DWORD dwNumberOfBytesWritten;
        return (TRUE == WriteFile(
            peer, str, size, &dwNumberOfBytesWritten, NULL
            ));
    }

#else // BOX_WIN
    std::ofstream peer;

    bool open()
    {
        peer.open(file_name.c_str(), std::ios::app);
        return peer.is_open();
    }
    bool isopen()
    {
        return peer.is_open();
    }
    void close()
    {
        peer.close();
    }
    bool write(const char *str, int size)
    {
        peer.write(str, size);
        return !peer.bad();
    }
    bool lock()
    {
        return true;
    }
    bool unlock()
    {
        return true;
    }

#endif // BOX_WIN

    FileLoggerSeed(const std::string &fn, bool ao)
        : file_name(fn)
        , always_open(ao)
    {
#ifdef BOX_WIN
        peer = NULL;
#endif
    }

};

///////////////////////////////////////////////////////////
// FileLogger
///////////////////////////////////////////////////////////

FileLogger::FileLogger(const char *file_name, bool always_open, bool global)
: Logger("FileLogger", Params(), global)
{
    _seed = new FileLoggerSeed(file_name, always_open);
}

//=========================================================
FileLogger::FileLogger(const wchar_t *file_name, bool always_open, bool global)
: Logger("FileLogger", Params(), global)
{
    _seed = new FileLoggerSeed(Sentence::wctomb(file_name), always_open);
}

//=========================================================
FileLogger::FileLogger(const std::string &name, const Params &params, bool global)
: Logger(name, params, global)
{
    std::string file;
    bool open = false;

    // file
    {
        std::string key = name + ".file";
        if(params.hasKey(key))
            file = params.getParam(key);
    }
    // open
    {
        std::string key = name + ".always_open";
        if(params.hasKey(key))
        {
            std::string val = Sentence::uppercase(params.getParam(key));
            open = (val == "TRUE" || val == "1" || val == "T");
        }
    }

     _seed = new FileLoggerSeed(file, open);
}

//=========================================================
FileLogger::~FileLogger()
{
    delete _seed;
}

//=========================================================
void FileLogger::_dolog(const Sentence &data)
{
    if(!_seed)
        return;

    if(!_seed->isopen())
    {
        if(!_seed->open())
            return; // HELP!
    }

    if(_seed->lock())
    {
        std::ostringstream ss;
        ss << data.prefix << ' ' << data.message << std::endl;
        _seed->write(ss.str().c_str(), (int)ss.str().length());
        _seed->unlock();
    }

    if(!_seed->always_open)
        _seed->close();

}

} // namespace box
