#pragma once

#include <string>
#include <format>

namespace wil {

enum LogSeverity
{
	LOG_SEVERITY_INFO,
	LOG_SEVERITY_WARN,
	LOG_SEVERITY_ERROR,
	LOG_SEVERITY_FATAL,
};

void LogMessage_(LogSeverity severity, const std::string &msg);

template<class... Ts>
void LogFormatString(LogSeverity severity, std::format_string<Ts...> fmt, Ts&&... args) {
	LogMessage_(severity, std::format(fmt, std::forward<Ts>(args)...));
}

#define WIL_LOGINFO(...) LogFormatString(::wil::LOG_SEVERITY_INFO, __VA_ARGS__)
#define WIL_LOGWARN(...) LogFormatString(::wil::LOG_SEVERITY_WARN, __VA_ARGS__)
#define WIL_LOGERROR(...) LogFormatString(::wil::LOG_SEVERITY_ERROR, __VA_ARGS__)
#define WIL_LOGFATAL(...) LogFormatString(::wil::LOG_SEVERITY_FATAL, __VA_ARGS__)

void LogVulkan(int severity, const std::string &msg);

}
