#pragma once

#include <memory>
#include "Violet/Core/PlatformDetection.h"

#ifdef VL_DEBUG
#define VL_ENABLE_ASSERT
#endif

#ifdef VL_ENABLE_ASSERT
#define VL_ASSERT(x, ...) { if(!(x)) {VL_ERROR("Assertion Failed: {0}"), __VA_ARGS__); __debugbreak();} }
#define VL_ASSERT(x) { if(!(x)) {VL_ERROR("Assertion Failed")); __debugbreak();} }

#define VL_CORE_ASSERT(x, ...) { if(!(x)) {VL_CORE_ERROR("Assert Failed: {0}", __VA_ARGS__); __debugbreak();}}
#define VL_CORE_ASSERT(x) { if(!(x)) {VL_CORE_ERROR("Assert Failed"); __debugbreak();}}
#else
#define VL_ASSERT(x, ...)
#define VL_CORE_ASSERT(x, ...)
#define VL_ASSERT(x)
#define VL_CORE_ASSERT(x)
#endif

#define BIT(x) (1 << x)

#define VL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Violet {
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) 
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "Violet/Core/Log.h"
// #include "Violet/Core/Assert.h"