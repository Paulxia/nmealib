///////////////////////////////////////////////////////////
//
// Project: XTL
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: mkspec.h 27 2007-04-04 19:33:20Z xtimor $
//
///////////////////////////////////////////////////////////

#ifndef __XTL_ROOT_H
#define __XTL_ROOT_H

#include "prepare.h"

#ifdef __XTL_USE_PRAGMAONE
#   pragma once 
#endif

#include <string>
#include <stdio.h>

#ifdef __XTL_USE_WINAPI
#   include <windows.h>
#else
#   include <errno.h>
#endif

__XTL_BEGIN_NAMESPACE

///////////////////////////////////////////////////////////
// tchar_t
///////////////////////////////////////////////////////////

#ifdef __XTL_USE_UNICODE
    typedef wchar_t tchar_t;
#   ifndef _T
#   define _T(x) (L##x)
#   endif
#else
    typedef char    tchar_t;
#   ifndef _T
#   define _T(x) (x)
#   endif
#endif

///////////////////////////////////////////////////////////
// root
///////////////////////////////////////////////////////////

class root
{
public:
    typedef long offset_type;
    typedef size_t size_type;
};

///////////////////////////////////////////////////////////
// noncopyable
///////////////////////////////////////////////////////////

class noncopyable : public root
{
public:
    inline noncopyable() {}
    ~noncopyable() {}

private:
    noncopyable(const noncopyable&) {}
    const noncopyable& operator= (const noncopyable&) { return *this; }

};

///////////////////////////////////////////////////////////
// string
///////////////////////////////////////////////////////////

using   std::string;
using   std::wstring;
using   std::basic_string;
typedef std::basic_string<tchar_t> tstring;

inline void tconv(string &dest, const wstring &src)
{
    dest.resize(src.size());
    ::wcstombs(&dest[0], src.c_str(), src.size());
}

inline void tconv(string &dest, const string &src)
{
    dest = src;
}

inline void tconv(wstring &dest, const string &src)
{
    dest.resize(src.size());
    ::mbstowcs(&dest[0], src.c_str(), src.size());
}

inline void tconv(wstring &dest, const wstring &src)
{
    dest = src;
}

///////////////////////////////////////////////////////////
// basic_exception
///////////////////////////////////////////////////////////

template<class _ChTy>
class basic_exception : public root
{
public:
    typedef basic_string<_ChTy> string_type;
    inline basic_exception(const string_type &what)
        : _what(what)
    {}
    const string_type & what() const { return _what; }
private:
    string_type _what;
};

typedef basic_exception<char>       exception;
typedef basic_exception<wchar_t>    wexception;
typedef basic_exception<tchar_t>    texception;

///////////////////////////////////////////////////////////
// basic_system_error
///////////////////////////////////////////////////////////

template<class _ChTy>
class basic_system_error : public basic_exception<_ChTy>
{
public:
    inline basic_system_error(
        const string_type &prefix = string_type())
        : basic_exception(prefix + get())
    {}
    static string_type get()
    {
        string_type msg;
#ifdef __XTL_USE_WINAPI
        LPVOID lpMsgBuf;
        FormatMessageW( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            0, // Default language
            (LPWSTR) &lpMsgBuf,
            0,
            NULL 
        );
        tconv(msg, wstring((LPCWSTR)lpMsgBuf));
        LocalFree(lpMsgBuf);
#else
        tconv(msg, string(strerror(errno));
#endif
        return msg;
    }
};

typedef basic_system_error<char>    system_error;
typedef basic_system_error<wchar_t> wsystem_error;
typedef basic_system_error<tchar_t> tsystem_error;

__XTL_END_NAMESPACE

#endif /* __XTL_ROOT_H */
