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

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/Violet/vendor/VulkanSDK/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"


group "Dependencies"
	include "Violet/vendor/GLFW"
	include "Violet/vendor/Glad"
	include "Violet/vendor/imgui"
	include "Violet/vendor/yaml-cpp"

	project "Violet"
		location "Violet"
		kind "StaticLib"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

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
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
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
			"%{IncludeDir.stb_image}",
			"%{IncludeDir.entt}",
			"%{IncludeDir.yaml_cpp}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.VulkanSDK}"
		}
		links
		{
			"GLFW",
			"Glad",
			"ImGui",
			"yaml-cpp",
			"opengl32.lib"
		}

		filter "files:Violet/vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

		filter "system:windows"
			cppdialect "C++17"
			systemversion "latest"
			
			defines
			{
				"VL_PLATFORM_WINDOWS",
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

	project "Sandbox"
		location "Sandbox"
		kind "ConsoleApp"
		language "C++"
		cppdialect "C++17"
		staticruntime "off"

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

project "Violet-Editor"
	location "Violet-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

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

		defines{
			"VL_PLATFORM_WINDOWS"
		}
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