// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change this value to use different versions
#define WINVER 0x0420

#define _WIN32_WCE_AYGSHELL 1
#define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA

#define DEV_NAME        (_T("VirtualGPSJet"))
#define DEF_PATHRESERVE (1024)

#include <atlbase.h>
#include <atlapp.h>
#include <atlimage.h>
#include <atltime.h>

#include <Windows.h>

using namespace WTL;

extern CAppModule _Module;

#include <shlwapi.h>
#include <atlresCE.h>
#include <atlmisc.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <aygshell.h>

#pragma comment(lib, "aygshell.lib")

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace std
{
    typedef std::basic_string<TCHAR> tstring;
    typedef std::basic_stringstream<TCHAR> tsstring;
}

#include "resource.h"
