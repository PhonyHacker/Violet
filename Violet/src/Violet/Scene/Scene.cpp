#include "vlpch.h"

#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "ScriptableEntity.h"

#include "Violet/Scripting/ScriptEngine.h"
#include "Violet/Renderer/Renderer2D.h"
#include "Violet/Physics/Physics2D.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_contact.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

#include <glm/glm.hpp>

namespace Violet {
	Scene::Scene()
	{
		m_Systems.Push(new ScriptSystem(this));
		m_Systems.Push(new PhysicsSystem(this));
		m_Systems.Push(new RenderSystem(this));
	}

	Scene::~Scene()
	{}

	// 复制源场景中特定类型的组件到目标场景中
	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	// 复制场景
	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		// 创建新的场景实例
		Ref<Scene> newScene = CreateRef<Scene>();

		// 复制视口大小
		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// 在新场景中创建实体，并构建UUID到entt ID的映射表
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// 复制组件到新场景中（不包括ID和Tag）
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		auto i = name.size();
		if (i > 20)
			tag.Tag = "Entity";
		else
			tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		int tag = SystemTag::Type::Script | SystemTag::Physics;
		m_Systems.Init(tag);
	}

	void Scene::OnRuntimeStop()
	{
		int tag = SystemTag::Physics;
		m_Systems.Detach(tag);

	}

	void Scene::OnSimulationStart()
	{
		int tag = SystemTag::Physics;
		m_Systems.Init(tag);
	}

	void Scene::OnSimulationStop()
	{
		int tag = SystemTag::Physics;
		m_Systems.Detach(tag);

	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		RunningRender = true;
		int tag = SystemTag::Render;

		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			tag |= SystemTag::Script | SystemTag::Physics;
		}

		m_Systems.Update(tag, ts);

	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera* camera)
	{
		RunningRender = false;
		UserCamera = camera;

		int tag = SystemTag::Render;

		if (!m_IsPaused || m_StepFrames-- > 0)
		{
			tag |= SystemTag::Physics;
		}
		m_Systems.Update(tag, ts);
	}
	void Scene::OnUpdateEditor(Timestep ts, EditorCamera* camera)
	{
		RunningRender = false;
		UserCamera = camera;

		int tag = SystemTag::Render;
		m_Systems.Update(tag, ts);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	void Scene::Step(int frames)
	{
		m_StepFrames = frames;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (m_EntityMap.find(uuid) != m_EntityMap.end())
			return { m_EntityMap.at(uuid), this };
		return {};
	}

	//class BoxData
	//{
	//public:
	//	int Tag;
	//};
	//class MyContactListener : public b2ContactListener {
	//	void BeginContact(b2Contact* contact) {
	//		// 获取碰撞的两个物体
	//		b2Fixture* fixtureA = contact->GetFixtureA();
	//		b2Fixture* fixtureB = contact->GetFixtureB();
	//		// 获取碰撞的两个物体)
	//		b2Body* bodyA = fixtureA->GetBody();
	//		b2Body* bodyB = fixtureB->GetBody();
	//		//BoxData* dataA = reinterpret_cast<BoxData*>(fixtureA->GetUserData().pointer);
	//		//BoxData* dataB = reinterpret_cast<BoxData*>(fixtureB->GetUserData().pointer);
	//		std::cout << bodyA->GetUserData().pointer << std::endl;
	//		std::cout << bodyB->GetUserData().pointer << std::endl;
	//		if (bodyA->GetUserData().pointer != 0 || bodyB->GetUserData().pointer != 0)
	//		{
	//			VL_CORE_INFO("Player contact");
	//		}

	//		// 输出碰撞的两个物体的用户数据
	//		//bodyA->GetUserData();
	//		//std::string userDataA = static_cast<std::string>(bodyA->GetUserData());
	//		//std::string userDataB = static_cast<std::string>(bodyB->GetUserData());
	//		VL_CORE_INFO("Begin contac");

	//	}

	//	void EndContact(b2Contact* contact) {
	//		// 碰撞结束时的处理逻辑
	//	}
	//};

	//static MyContactListener* contactListener;

	void PhysicsSystem::Init()
	{
		VL_CORE_INFO("Physics");

		m_PhysicsWorld = new b2World({ 0.0f, -9.8f });

		//contactListener = new MyContactListener();
		//m_PhysicsWorld->SetContactListener(contactListener);

		auto view = m_Scene->Reg().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			auto& string = entity.GetName();

			//BoxData* data = new BoxData();

			b2BodyDef bodyDef;

			//if (string == "Player")
			//{
			//	//data->Tag = 1;
			//	bodyDef.userData.pointer = 1;
			//	//bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(data);
			//	VL_CORE_INFO("find player");
			//}

			bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.Type);
			bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
			bodyDef.angle = transform.Rotation.z;
			bodyDef.gravityScale = rb2d.GravityScale;

			b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
			body->SetFixedRotation(rb2d.FixedRotation);
			rb2d.RuntimeBody = body;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				b2PolygonShape boxShape;
				// boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}


			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void PhysicsSystem::Update(Timestep ts)
	{
		const int32_t velocityIterations = 6;
		const int32_t positionIterations = 2;
		m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = m_Scene->Reg().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
		}
	}
	void PhysicsSystem::Detach()
	{
		while (m_PhysicsWorld->GetBodyList()) {
			b2Body* body = m_PhysicsWorld->GetBodyList();
			m_PhysicsWorld->DestroyBody(body);
		}
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void ScriptSystem::Init()
	{
		VL_CORE_INFO("Script");
		ScriptEngine::OnRuntimeStart(m_Scene);
		// Instantiate all script entities

		auto view = m_Scene->Reg().view<ScriptComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene};
			ScriptEngine::OnCreateEntity(entity);
		}
	}
	void ScriptSystem::Update(Timestep ts)
	{
		// C# Entity OnUpdate
		auto view = m_Scene->Reg().view<ScriptComponent>();
		for (auto e : view)
		{
			Entity entity = { e, m_Scene };
			ScriptEngine::OnUpdateEntity(entity, ts);
		}
	}

	void ScriptSystem::Detach()
	{

	}
	void RenderSystem::Init()
	{
		VL_CORE_INFO("Render");

	}
	void RenderSystem::Update(Timestep ts)
	{
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		if (m_Scene->RunningRender)
		{
			auto view = m_Scene->Reg().view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);
				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}
		else
		{
			mainCamera = m_Scene->UserCamera;
			cameraTransform = m_Scene->UserCamera->GetViewMatrix();
		}


		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			// Draw sprites
			{
				auto group = m_Scene->Reg().group<TransformComponent>(entt::get<SpriteRendererComponent>);

				for (auto entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}

			// Draw circles
			{
				auto view = m_Scene->Reg().view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}

			// Draw text
			{
				auto view = m_Scene->Reg().view<TransformComponent, TextComponent>();
				for (auto entity : view)
				{
					auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);

					Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);
				}
			}

		}
		Renderer2D::EndScene();
	}
	void RenderSystem::Detach()
	{

	}
}
