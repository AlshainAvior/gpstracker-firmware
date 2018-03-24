#pragma once
#include "Config.h"

//#define DISABLE_LOGGING 1
#include <ArduinoLog.h>

#define LOG_SERIAL_SPEED 115200
#if _DEBUG
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#else
#define LOG_LEVEL LOG_LEVEL_NOTICE
#endif

#define LOG(level, f) Log.level(F("[" LOGGER_NAME "::" f "]\n"))
#define LOG_MSG(level, f, msg) Log.level(F("[" LOGGER_NAME "::" f "] " msg "\n"))
#define LOG_FORMAT(level, f, msg, ...) Log.level(F("[" LOGGER_NAME "::" f "] " msg "\n"), __VA_ARGS__)

#if _DEBUG
#define VERBOSE(f) LOG(verbose, f)
#define VERBOSE_MSG(f, msg) LOG_MSG(verbose, f, msg)
#define VERBOSE_FORMAT(f, msg, ...) LOG_FORMAT(verbose, f, msg, __VA_ARGS__)
#else
#define VERBOSE(f)
#define VERBOSE_MSG(f, msg)
#define VERBOSE_FORMAT(f, msg, ...)
#endif

#define NOTICE(f) LOG(notice, f)
#define NOTICE_MSG(f, msg) LOG_MSG(notice, f, msg)
#define NOTICE_FORMAT(f, msg, ...) LOG_FORMAT(notice, f, msg, __VA_ARGS__)


namespace logging {
	void setup();
}