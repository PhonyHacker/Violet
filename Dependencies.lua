
-- Violet Dependencies

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/Violet/vendor/stb_image"
IncludeDir["yaml_cpp"] = "%{wks.location}/Violet/vendor/yaml-cpp/include"
IncludeDir["Box2D"] = "%{wks.location}/Violet/vendor/Box2D/include"
IncludeDir["filewatch"] = "%{wks.location}/Violet/vendor/filewatch"
IncludeDir["GLFW"] = "%{wks.location}/Violet/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Violet/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Violet/vendor/ImGui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Violet/vendor/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/Violet/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/Violet/vendor/entt/include"
IncludeDir["mono"] = "%{wks.location}/Violet/vendor/mono/include"
IncludeDir["msdfgen"] = "%{wks.location}/Violet/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/Violet/vendor/msdf-atlas-gen/msdf-atlas-gen"

LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/Violet/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"