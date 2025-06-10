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

void LogMessage_(LogSeverity severity, const std::string &str)
{
	switch (severity)
	{
		case LOG_SEVERITY_INFO:
			std::cout << "[INFO " << GetCurrentTime_() << "] " << str << '\n';
			break;

		case LOG_SEVERITY_WARN:
			std::cout << "[\033[33mWARN\033[0m " << GetCurrentTime_() << "] \033[33m" << str << "\033[0m\n";
			break;

		case LOG_SEVERITY_ERROR:
			std::cout << "[\033[31mERRO\033[0m " << GetCurrentTime_() << "] \033[31m" << str << "\033[0m\n";
			break;

		case LOG_SEVERITY_FATAL:
			std::cout << "[\033[35mFATL\033[0m " << GetCurrentTime_() << "] \033[35m" << str << "\033[0m\n";
			break;
	}
}

void LogVulkan(int severity, const std::string &msg)
{
	std::cout << "[VULK " << GetCurrentTime_() << "] ";
	if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		std::cout << "\033[31m" << msg << "\033[0m\n";
	} else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cout << "\033[33m" << msg << "\033[0m\n";
	} else {
		std::cout << msg << '\n';
	}
}

}

