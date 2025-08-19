#pragma once

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/bundled/printf.h"

class log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return m_Logger; }

private:
	static std::shared_ptr<spdlog::logger> m_Logger;
};

#define LOG_TRACE(...)		::log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::log::GetLogger()->error(__VA_ARGS__)
#define LOG_FATAL(...)		::log::GetLogger()->fatal(__VA_ARGS__)