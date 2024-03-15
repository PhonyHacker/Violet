#pragma once

#include "Violet/Core/Base.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
namespace Violet {
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define VL_CORE_TRACE(...) ::Violet::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VL_CORE_INFO(...) ::Violet::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VL_CORE_WARN(...) ::Violet::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VL_CORE_ERROR(...) ::Violet::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VL_CORE_FATAL(...) ::Violet::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define VL_TRACE(...) ::Violet::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VL_INFO(...) ::Violet::Log::GetClientLogger()->info(__VA_ARGS__)
#define VL_WARN(...) ::Violet::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VL_ERROR(...) ::Violet::Log::GetClientLogger()->error(__VA_ARGS__)
#define VL_FATAL(...) ::Violet::Log::GetClientLogger()->fatal(__VA_ARGS__)
#define VL_CRITICAL(...) ::Violet::Log::GetClientLogger()->critical(__VA_ARGS__)
