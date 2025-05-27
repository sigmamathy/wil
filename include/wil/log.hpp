#pragma once

#include <string>

namespace wil {

void LogInfo(std::string const& msg);

void LogWarn(std::string const& msg);

void LogErr(std::string const& msg);

void LogFatal(std::string const& msg);

void LogVulkan(int severity, const std::string &msg);

}
