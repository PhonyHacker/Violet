workspace "Violet"
	architecture "x86_64"

	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	IncludeDir = {}
	IncludeDir["GLFW"] = "Violet/vendor/GLFW/include"
	IncludeDir["Glad"] = "Violet/vendor/Glad/include"
	IncludeDir["ImGui"] = "Violet/vendor/imgui"
	IncludeDir["glm"] = "Violet/vendor/glm"
	IncludeDir["stb_image"] = "Violet/vendor/stb_image"


	include "Violet/vendor/GLFW"
	include "Violet/vendor/Glad"
	include "Violet/vendor/imgui"

	project "Violet"
		location "Violet"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

		targetdir ("bin/" .. outputdir .. "/%{prj.name}")
		objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
		
		pchheader "vlpch.h"
		pchsource "Violet/src/vlpch.cpp"

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/vendor/stb_image/**.h",
			"%{prj.name}/vendor/stb_image/**.cpp",
			"%{prj.name}/vendor/glm/glm/**.hpp",
			"%{prj.name}/vendor/glm/glm/**.inl",
		}

		defines
		{
			"_CRT_SECURE_NO_WARINGS"
		}
		includedirs
		{
			"%{prj.name}/src",
			"%{prj.name}/vendor/spdlog/include",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.Glad}",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.stb_image}"
		}
		links
		{
			"GLFW",
			"Glad",
			"ImGui",
			"opengl32.lib"
		}

		filter "system:windows"
			cppdialect "C++17"
			systemversion "latest"
			
			defines
			{
				"VL_BUILD_DLL",
				"GLFW_INCLUDE_NONE"
			}

			postbuildcommands
			{
				("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
			}

		filter "configurations:Debug"
			defines "VL_DEBUG"
			runtime "Debug"
			symbols "On"
		filter "configurations:Release"
			defines "VL_RELEASE"
			runtime "Release"
			optimize "On"
		filter "configurations:Dist"
			defines "VL_DIST"
			runtime "Release"
			optimize "On"

	project "Sandbox"
		location "Sandbox"
		kind "ConsoleAPP"
		language "C++"
		cppdialect "C++17"
		staticruntime "on"

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
			"Violet/src",
			"Violet/vendor",
			"%{IncludeDir.glm}"
		}

		links
		{
			"Violet"
		}

		filter "system:windows"
			cppdialect "C++17"
			systemversion "latest"
			
		filter "configurations:Debug"
			defines "VL_DEBUG"
			runtime "Debug"
			symbols "On"
		filter "configurations:Release"
			defines "VL_RELEASE"
			runtime "Release"
			optimize "On"
		filter "configurations:Dist"
			defines "VL_DIST"
			runtime "Release"
			optimize "On"