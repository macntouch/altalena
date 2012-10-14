// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _WIN32_WINNT		//Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#pragma unmanaged
#include "IwUtils.h"
#include "Logger.h"
#include "Profiler.h"
#include "LightweightProcess.h"
#include "LocalProcessRegistrar.h"
#include "ActiveObject.h"
#include "Configuration.h"
#include "ConfigurationFactory.h"
#pragma managed


#using <mscorlib.dll>
using namespace System;

#include<vcclr.h>

//using namespace ivrworx;