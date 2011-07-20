///////////////////////////////////////////////////////////
//
// Project: BOX
// URL: http://jugum.org
// Author: Tim (xtimor@jugum.org)
// $Id: config.h 20 2007-04-04 08:05:03Z xtimor $
//
///////////////////////////////////////////////////////////

#include "sentence.h"
#include "config.h"

namespace box
{

//=========================================================
std::string Sentence::wctomb(const std::wstring &wc_str)
{
    char buff[BOX_SPRINTF_BUFF];
    int size = BOX_POSIX(snprintf)(&buff[0], BOX_SPRINTF_BUFF - 1, "%S", wc_str.c_str());
    return std::string(&buff[0], size);
}

//=========================================================
std::wstring Sentence::mbtowc(const std::string &c_str)
{
    wchar_t buff[BOX_SPRINTF_BUFF];
    int size = BOX_POSIX(snwprintf)(&buff[0], BOX_SPRINTF_BUFF - 1, L"%S", c_str.c_str());
    return std::wstring(&buff[0], size);
}

//=========================================================
std::string Sentence::uppercase(const std::string &str)
{
    std::string resv(str);
    for(std::string::size_type i = 0; i < str.length(); ++i)
        resv[i] = (char)::toupper(resv[i]);
    return resv;
}

} // namespace box
