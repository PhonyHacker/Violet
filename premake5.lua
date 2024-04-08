include "Dependencies.lua"
workspace "Violet"
	architecture "x86_64"
	startproject "Violet-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}	

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

group "Dependencies"
	include "Violet/vendor/Box2D"
	include "Violet/vendor/GLFW"
	include "Violet/vendor/Glad"
	include "Violet/vendor/imgui"
	include "Violet/vendor/yaml-cpp"
group ""

group "Core"
	project "Violet"
		location "Violet"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

		
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
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
		}

		defines
		{
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE"
		}
		includedirs
		{
			"%{prj.name}/src",
			"%{prj.name}/vendor/spdlog/include",
			"%{IncludeDir.Box2D}",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.Glad}",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.glm}",
			"%{IncludeDir.stb_image}",
			"%{IncludeDir.entt}",
			"%{IncludeDir.mono}",
			"%{IncludeDir.yaml_cpp}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.VulkanSDK}"
		}
		links
		{
			"Box2D",
			"GLFW",
			"Glad",
			"ImGui",
			"yaml-cpp",
			"opengl32.lib",
			"%{Library.mono}",
		}

		filter "files:Violet/vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

		filter "system:windows"
			systemversion "latest"
			
			defines
			{
			}
			
			links
			{
				"%{Library.WinSock}",
				"%{Library.WinMM}",
				"%{Library.WinVersion}",
				"%{Library.BCrypt}",
			}
		filter "configurations:Debug"
			defines "VL_DEBUG"
			runtime "Debug"
			symbols "On"

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}"
			}

		filter "configurations:Release"
			defines "VL_RELEASE"
			runtime "Release"
			optimize "On"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}"
			}

		filter "configurations:Dist"
			defines "VL_DIST"
			runtime "Release"
			optimize "On"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}"
			}
------------------------------
	project "Violet-ScriptCore"
		location "Violet-ScriptCore"
		kind "SharedLib"
		language "C#"
		dotnetframework "4.7.2"

		targetdir ("%../Violet-Editor/Resources/Scripts")
		objdir ("%../Violet-Editor/Resources/Scripts/Intermediates")

		files 
		{
			"Source/**.cs",
			"Properties/**.cs"
		}

		filter "configurations:Debug"
			optimize "Off"
			symbols "Default"

		filter "configurations:Release"
			optimize "On"
			symbols "Default"

		filter "configurations:Dist"
			optimize "Full"
			symbols "Off"
group ""

group "Tools"
	project "Violet-Editor"
		location "Violet-Editor"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{wks.location}/Violet/vendor/spdlog/include",
			"%{wks.location}/Violet/src",
			"%{wks.location}/Violet/vendor",
			"%{IncludeDir.glm}",
			"%{IncludeDir.entt}",
			"%{IncludeDir.ImGuizmo}"
		}

		links
		{
			"Violet"
		}

		filter "system:windows"
			systemversion "latest"

		filter "configurations:Debug"
			defines "VL_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "VL_RELEASE"
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines "VL_DIST"
			runtime "Release"
			optimize "on"

group ""

group "Misc"
	project "Sandbox"
		location "Sandbox"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

		targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
		objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

		files
		{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}

		includedirs
		{
			"%{wks.location}/Violet/vendor/spdlog/include",
			"%{wks.location}/Violet/src",
			"%{wks.location}/Violet/vendor",
			"%{IncludeDir.glm}",
			"%{IncludeDir.entt}"
		}

		links
		{
			"Violet"
		}

	filter "system:windows"
			systemversion "latest"

			defines{
				"VL_PLATFORM_WINDOWS"
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
group ""