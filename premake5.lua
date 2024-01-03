workspace "Violet"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Include directories relative to root folder (solution directory)
	IncludeDir = {}
	IncludeDir["GLFW"] = "Violet/vendor/GLFW/include"

	include "Violet/vendor/GLFW"

	project "Violet"
		location "Violet"
		kind "SharedLib"
		language "C++"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		pchheader "vlpch.h"
		pchsource "Violet/src/vlpch.cpp"

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{prj.name}/src",
			"%{prj.name}/vendor/spdlog/include",
			"%{IncludeDir.GLFW}"
		}
		links
		{
			"GLFW",
			"opengl32.lib"
		}

		filter "system:windows"
			cppdialect "C++17"
			staticruntime "On"
			systemversion "latest"
			
			defines
			{
				"VL_PLATFORM_WINDOWS",
				"VL_BUILD_DLL"
			}

			postbuildcommands
			{
				("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
			}

		filter "configurations:Debug"
			defines "VL_DEBUG"
			symbols "On"
		filter "configurations:Release"
			defines "VL_RELEASE"
			optimize "On"
		filter "configurations:Dist"
			defines "VL_DIST"
			symbols "On"

	project "Sandbox"
		location "Sandbox"
		kind "ConsoleAPP"
		language "C++"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"Violet/vendor/spdlog/include",
			"Violet/src"
		}

		links
		{
			"Violet"
		}

		filter "system:windows"
			cppdialect "C++17"
			staticruntime "On"
			systemversion "latest"
			
			defines
			{
				"VL_PLATFORM_WINDOWS"
			}

		filter "configurations:Debug"
			defines "VL_DEBUG"
			symbols "On"
		filter "configurations:Release"
			defines "VL_RELEASE"
			optimize "On"
		filter "configurations:Dist"
			defines "VL_DIST"
			symbols "On"