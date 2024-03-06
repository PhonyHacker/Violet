#pragma once
#ifdef VL_PLATFORM_WINDOWS
	#ifdef VL_BUILD_DLL
		#define VIOLET_API __declspec(dllexport)
	#else
		#define VIOLET_API __declspec(dllimport)
	#endif
#else
	#error Violet only support Windows!
#endif

#ifdef VL_DEBUG
	#define VL_ENABLE_ASSERT
#endif

#ifdef VL_ENABLE_ASSERT
	#define VL_ASSERT(x, ...) { if(!(x)) {VL_ERROR("Assertion Failed: {0}"), __VA_ARGS__); __debugbreak();} }
	#define VL_CORE_ASSERT(x, ...) { if(!(x)) {VL_CORE_ERROR("Assert Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define VL_ASSERT(x, ...)
	#define VL_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define VL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)