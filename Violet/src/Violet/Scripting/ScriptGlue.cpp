#include "vlpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "Violet/Core/UUID.h"
#include "Violet/Core/KeyCode.h"
#include "Violet/Core/Input.h"

#include "Violet/Physics/Physics2D.h"

#include "Violet/Scene/Scene.h"
#include "Violet/Scene/Entity.h"
#include "Violet/Scene/Components.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"
namespace Violet {
	namespace Utils {

		std::string MonoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);
			return str;
		}

	}

	// static std::unordered_map<MonoType*, Component> s_EntityHasComponentFuncs;
	static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define VL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Violet.InternalCalls::" #Name, Name)

	// 定义静态C++函数，用于在C++中实现具体逻辑

#pragma region Debug Log
	
	// 将MonoString转换为UTF-8字符串并打印输出
	static void NativeLog(MonoString* string, int parameter)
	{
		std::string str = Utils::MonoStringToString(string);
		std::cout << str << ", " << parameter << std::endl;
	}

	// 打印输入的glm::vec3向量并归一化
	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		//VL_CORE_WARN("Value: {0}", *parameter);
		std::cout << parameter->x << "," << parameter->y << "," << parameter->z << std::endl;
		*outResult = glm::normalize(*parameter);
	}

	// 计算输入的glm::vec3向量与自身的点积并返回结果
	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		//VL_CORE_WARN("Value: {0}", *parameter);
		std::cout << parameter->x << "," << parameter->y << "," << parameter->z << std::endl;
		return glm::dot(*parameter, *parameter);
	}
#pragma endregion

#pragma region Script
	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}
#pragma endregion 

#pragma region Entity

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VL_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static bool Entity_AddComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		VL_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		if (s_EntityHasComponentFuncs.at(managedType)(entity))
			return false;

		s_EntityAddComponentFuncs.at(managedType)(entity);
		return true;
	}

	static uint64_t Entity_CreateEntity(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->CreateEntity(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}

	static bool Entity_DeleteEntity(UUID entityID)
	{
		VL_CORE_TRACE(entityID);
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		if (!entity)
			return false;

		scene->DestroyEntity(entity);
		return true;
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;

		return entity.GetUUID();
	}
#pragma endregion

#pragma region Transform
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void TransformComponent_GetRotation(UUID entityID, glm::vec3* outRotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Rotation = *rotation;
	}

	static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Scale = *scale;
	}
#pragma endregion

#pragma region Rigibody
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}
#pragma endregion

#pragma region Input
	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static bool Input_MouseButtonPressed(MouseButtonCode button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	static void Input_GetMousePosition(glm::vec2* outPosition)
	{
		*outPosition = Input::GetMousePosition();
	}

	static void Input_GetMouseImGuiPosition(glm::vec2* outPosition)
	{
		*outPosition = Input::GetMouseImGuiPosition();
	}

	static uint64_t Input_GetMouseHoever()
	{
		Entity entity = Input::GetMouseHoevered();

		if (!entity)
			return 0;

		return entity.GetUUID();
	}
#pragma endregion

#pragma region Camera
	static bool CameraComponent_GetIsPrimary(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		return entity.GetComponent<CameraComponent>().Primary;
	}
	
	static void CameraComponent_SetIsPrimary(UUID entityID, bool flag)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);

		entity.GetComponent<CameraComponent>().Primary = flag;
	}
#pragma endregion

#pragma region Text
	static MonoString* TextComponent_GetText(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return ScriptEngine::CreateString(tc.TextString.c_str());
	}

	static void TextComponent_SetText(UUID entityID, MonoString* textString)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.TextString = Utils::MonoStringToString(textString);
	}

	static void TextComponent_GetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		*color = tc.Color;
	}

	static void TextComponent_SetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Color = *color;
	}

	static float TextComponent_GetKerning(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.Kerning;
	}

	static void TextComponent_SetKerning(UUID entityID, float kerning)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Kerning = kerning;
	}

	static float TextComponent_GetLineSpacing(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.LineSpacing;
	}

	static void TextComponent_SetLineSpacing(UUID entityID, float lineSpacing)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.LineSpacing = lineSpacing;
	}
#pragma endregion

#pragma region SpriteRendererComponent
	static void SpriteRendererComponent_GetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<SpriteRendererComponent>());

		auto& sc = entity.GetComponent<SpriteRendererComponent>();
		*color = sc.Color;
	}

	static void SpriteRendererComponent_SetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<SpriteRendererComponent>());

		auto& sc = entity.GetComponent<SpriteRendererComponent>();
		sc.Color = *color;
	}

	static void SpriteRendererComponent_SetTexture2D(UUID entityID, MonoString* path)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<SpriteRendererComponent>());

		auto& sc = entity.GetComponent<SpriteRendererComponent>();
		
		std::filesystem::path resource = std::filesystem::current_path() / Utils::MonoStringToString(path);
		//VL_TRACE(resource.string());
		sc.Texture = Texture2D::Create(resource.string());

	}

	static float SpriteRendererComponent_GetTilingFactor(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<SpriteRendererComponent>());

		auto& sc = entity.GetComponent<SpriteRendererComponent>();
		return sc.TilingFactor;
	}

	static void SpriteRendererComponent_SetTilingFactor(UUID entityID, float TilingFactor)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		VL_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		VL_CORE_ASSERT(entity);
		VL_CORE_ASSERT(entity.HasComponent<SpriteRendererComponent>());

		auto& sc = entity.GetComponent<SpriteRendererComponent>();
		sc.TilingFactor = TilingFactor;
	}
#pragma endregion


	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
			{
				// 获取每个组件的类型名称
				std::string_view typeName = typeid(Component).name();
				// 查找最后一个冒号的位置以提取实际的结构名称
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				// 构造对应C#类的完整名称
				std::string managedTypename = fmt::format("Violet.{}", structName);

				// 从核心程序集中检索托管类型的MonoType*
				MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
				if (!managedType)
				{
#ifdef  VL_DEBUG
					VL_CORE_ERROR("无法找到组件类型 {}", managedTypename);

#endif
					return;
				}
				else
				{
#ifdef VL_DEBUG
					VL_CORE_TRACE("注册组件类型 {}", managedTypename);

#endif
				}
				// 将每个组件类型与一个Lambda函数关联
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
				s_EntityAddComponentFuncs[managedType] = [](Entity entity) { entity.AddComponent<Component>(); };
			}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	// 注册函数，将C++函数注册为C#中的内部调用
	void ScriptGlue::RegisterFunctions()
	{
		// Debug
		VL_ADD_INTERNAL_CALL(NativeLog);
		VL_ADD_INTERNAL_CALL(NativeLog_Vector);
		VL_ADD_INTERNAL_CALL(NativeLog_VectorDot);
		VL_ADD_INTERNAL_CALL(GetScriptInstance);

		// Entity
		VL_ADD_INTERNAL_CALL(Entity_HasComponent);
		VL_ADD_INTERNAL_CALL(Entity_AddComponent);
		VL_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		VL_ADD_INTERNAL_CALL(Entity_CreateEntity);
		VL_ADD_INTERNAL_CALL(Entity_DeleteEntity);

		// Transform
		VL_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		VL_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		VL_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		VL_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		VL_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		VL_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		// Rigidbody
		VL_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		VL_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		VL_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		VL_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		VL_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);

		// Camera
		VL_ADD_INTERNAL_CALL(CameraComponent_GetIsPrimary);
		VL_ADD_INTERNAL_CALL(CameraComponent_SetIsPrimary);

		// Text
		VL_ADD_INTERNAL_CALL(TextComponent_GetText);
		VL_ADD_INTERNAL_CALL(TextComponent_SetText);
		VL_ADD_INTERNAL_CALL(TextComponent_GetColor);
		VL_ADD_INTERNAL_CALL(TextComponent_SetColor);
		VL_ADD_INTERNAL_CALL(TextComponent_GetKerning);
		VL_ADD_INTERNAL_CALL(TextComponent_SetKerning);
		VL_ADD_INTERNAL_CALL(TextComponent_GetLineSpacing);
		VL_ADD_INTERNAL_CALL(TextComponent_SetLineSpacing);

		// Input
		VL_ADD_INTERNAL_CALL(Input_IsKeyDown);
		VL_ADD_INTERNAL_CALL(Input_MouseButtonPressed);
		VL_ADD_INTERNAL_CALL(Input_GetMousePosition);
		VL_ADD_INTERNAL_CALL(Input_GetMouseImGuiPosition);
		VL_ADD_INTERNAL_CALL(Input_GetMouseHoever);
		
		// SpriteRenderer
		VL_ADD_INTERNAL_CALL(SpriteRendererComponent_GetColor);
		VL_ADD_INTERNAL_CALL(SpriteRendererComponent_SetColor);
		VL_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture2D);
		VL_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTilingFactor);
		VL_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTilingFactor);


	}

}