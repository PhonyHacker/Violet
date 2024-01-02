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

#define BIT(x) (1 << x)