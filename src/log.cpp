#include <wil/log.hpp>

#include <iostream>
#include <chrono>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace wil {

std::string GetCurrentTime_()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M");
    return ss.str();
}

static bool prev_newline = false;

void LogMessage_(LogSeverity severity, const std::string &str)
{
	switch (severity)
	{
		case LOG_SEVERITY_INFO:
			prev_newline = false;
			std::cout << "[INFO " << GetCurrentTime_() << "] " << str << '\n';
			break;

		case LOG_SEVERITY_WARN:
			if (!prev_newline) {
				std::cout << '\n';
				prev_newline = true;
			}
			std::cout << "\033[33m[WARN " << GetCurrentTime_() << "] " << str << "\033[0m\n\n";
			break;

		case LOG_SEVERITY_ERROR:
			if (!prev_newline) {
				std::cout << '\n';
				prev_newline = true;
			}
			std::cout << "\033[31m[ERRO " << GetCurrentTime_() << "] " << str << "\033[0m\n\n";
			break;

		case LOG_SEVERITY_FATAL:
			if (!prev_newline) {
				std::cout << '\n';
				prev_newline = true;
			}
			std::cout << "\033[35m[FATL " << GetCurrentTime_() << "] " << str << "\033[0m\n\n";
			break;
	}
}

void LogVulkan(int severity, const std::string &msg)
{
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		if (!prev_newline) {
			std::cout << '\n';
			prev_newline = true;
		}
		std::cout << "\033[31m[VULK " << GetCurrentTime_() << "] " << msg << "\033[0m\n\n";
	}
	else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		if (!prev_newline) {
			std::cout << '\n';
			prev_newline = true;
		}
		std::cout << "\033[33m[VULK " << GetCurrentTime_() << "] " << msg << "\033[0m\n\n";
	}
	else
	{
		prev_newline = false;
		std::cout << "[VULK " << GetCurrentTime_() << "] " << msg << '\n';
	}
}

}

