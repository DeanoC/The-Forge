#pragma once
// replave TheForge tith al2o3 loging which is more libray friendly
#include "al2o3_platform/platform.h"

enum class LogLevel : uint8_t {
	eDEBUG = 0,
	eINFO = 1,
	eWARNING = 2,
	eERROR = 3
};

inline void TheForge_LogWrite(LogLevel level,
														char const *const file,
														int line,
														char const *const function,
														char const *const str,
														...) {
	va_list arglist;
	va_start(arglist, str);

	switch (level) {
	case LogLevel::eDEBUG: AL2O3_DebugMsg(file, line, function, str, arglist);
		break;
	case LogLevel::eINFO: AL2O3_InfoMsg(file, line, function, str, arglist);
		break;
	case LogLevel::eWARNING: AL2O3_WarningMsg(file, line, function, str, arglist);
		break;
	case LogLevel::eERROR: AL2O3_ErrorMsg(file, line, function, str, arglist);
		break;
	}
	va_end(arglist);

}

// Usage: LOGF(LogLevel::eINFO | LogLevel::eDEBUG, "Whatever string %s, this is an int %d", "This is a string", 1)
#define LOGF(log_level, ...) TheForge_LogWrite(log_level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
