#pragma once
// replave TheForge tith al2o3 logging which is more library friendly
#include "al2o3_platform/platform.h"
#include <stdarg.h> // for va_xxx functions

enum class LogLevel : uint8_t {
	eDEBUG = 0,
	eINFO = 1,
	eWARNING = 2,
	eERROR = 3
};

// Usage: LOGF(LogLevel::eINFO | LogLevel::eDEBUG, "Whatever string %s, this is an int %d", "This is a string", 1)
#define LOGF(log_level, ...)  do {\
switch (log_level) { \
case LogLevel::eDEBUG: AL2O3_DebugMsg(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); break; \
case LogLevel::eINFO: AL2O3_InfoMsg(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); break; \
case LogLevel::eWARNING: AL2O3_WarningMsg(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); break; \
case LogLevel::eERROR: AL2O3_ErrorMsg(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); break; \
} } while(false)

#define _FailedAssert(file, line, msg) AL2O3_FailedAssert(file, line, __FUNCTION__, msg)