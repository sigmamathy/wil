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

#ifndef NDEBUG
#define WIL_ASSERT(...) if (!(__VA_ARGS__))\
		WIL_LOGFATAL("Assertion '{}' failed. (" __FILE__ ":{})", #__VA_ARGS__, __LINE__);
#else
#define WIL_ASSERT(...)
#endif

void LogVulkan(int severity, const std::string &msg);

template<class T, unsigned Dim> class Vector;

}

template<class T>
struct std::formatter<wil::Vector<T,2>> : std::formatter<std::string> {
	auto format(wil::Vector<T,2> v, format_context& ctx) const {
		return formatter<string>::format(
				std::format("[{}, {}]", v.x, v.y), ctx);
	}
};

template<class T>
struct std::formatter<wil::Vector<T,3>> : std::formatter<std::string> {
	auto format(wil::Vector<T,3> v, format_context& ctx) const {
		return formatter<string>::format(
				std::format("[{}, {}, {}]", v.x, v.y, v.z), ctx);
	}
};

template<class T>
struct std::formatter<wil::Vector<T,4>> : std::formatter<std::string> {
	auto format(wil::Vector<T,4> v, format_context& ctx) const {
		return formatter<string>::format(
				std::format("[{}, {}, {}, {}]", v.x, v.y, v.z, v.w), ctx);
	}
};
