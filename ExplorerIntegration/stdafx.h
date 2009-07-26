// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atltypes.h>
#include <atlstr.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#include <atlapp.h>
#pragma warning(pop)
#include <atlgdi.h>

#pragma warning(push)
#pragma warning(disable: 4189 4267)
#include "atlgdix.h"
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4702)
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using namespace std;
#pragma warning(pop)

using namespace ATL;

#pragma warning(push)
#pragma warning(disable: 4244 4267 4511 4512 701 4702)
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
using namespace boost;
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4510 4610)
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
using namespace boost::multi_index;
#pragma warning(pop)

#include "../shared/Structures.h"

#include "../Console/Helpers.h"
#include "../Console/ImageHandler.h"
#include "../Console/SettingsHandler.h"
#include "TabSettings.h"
#include "Synchronization.h"

//////////////////////////////////////////////////////////////////////////
// Global variables
extern syncCriticalSection					g_oSync;			///> object to synchronize access to global data
extern shared_ptr<SettingsHandler>			g_settingsHandler;
extern vector< shared_ptr<CTabSettings> >	g_vTabs;


//////////////////////////////////////////////////////////////////////////
// global methods
void LoadConsoleSettings();


//////////////////////////////////////////////////////////////////////////////
// Global hotkeys

#define IDC_GLOBAL_ACTIVATE	0xB000


//////////////////////////////////////////////////////////////////////////////
// trace function and TRACE macro

#ifdef _DEBUG

void Trace(const wchar_t* pszFormat, ...);

#define TRACE		::Trace

#else

#define TRACE		__noop

#endif // _DEBUG

//////////////////////////////////////////////////////////////////////////////


// uncomment this define to store debug information in "c:\*.log" files
//#define DEBUG_TO_LOG_FILES
