#pragma once
// 供外部程序引用

#include "Violet/Core/Application.h"
#include "Violet/Core/Layer.h"
				
#include "Violet/Core/Log.h"
				
#include "Violet/Core/Timestep.h"
#include "Violet/Core/Input.h"
#include "Violet/Core/KeyCode.h"
#include "Violet/Core/MouseButtonCode.h"
#include "Violet/OrthographicCameraController.h"
#include "Violet/ImGui/ImGuiLayer.h"

#include "Violet/Scene/Scene.h"
#include "Violet/Scene/Entity.h"
#include "Violet/Scene/Components.h"
#include "Violet/Scene/ScriptableEntity.h"

// --渲染相关--------------------
#include "Violet/Renderer/Renderer.h"
#include "Violet/Renderer/Renderer2D.h"
#include "Violet/Renderer/RenderCommand.h"

#include "Violet/Renderer/Buffer.h"
#include "Violet/Renderer/Shader.h"
#include "Violet/Renderer/Framebuffer.h"
#include "Violet/Renderer/Texture.h"
#include <Violet/Renderer/SubTexture2D.h>
#include "Violet/Renderer/VertexArray.h"

#include "Violet/Renderer/OrthographicCamera.h"
