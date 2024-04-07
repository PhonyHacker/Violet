#include "vlpch.h"
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Violet {

	namespace Utils {
		// ��ȡ�ļ��Ķ���������
		char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			// ʧ��ʱ���ؿ�ָ��
			if (!stream)
			{
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			// �ļ�Ϊ��ʱ���ؿ�ָ��
			if (size == 0)
			{
				return nullptr;
			}

			char* buffer = new char[size];		// ����һ��������
			stream.read((char*)buffer, size);	// ��ȡ�ļ����ݵ�������
			stream.close();

			*outSize = size;	// ������������Ĵ�С
			return buffer;		// ���ػ�����ָ��	
		}

		// ����C#����
		MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);	// ��ȡ�ļ�����

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			// �����ݼ�ӳ��
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				// ��ӡ������Ϣ
				const char* errorMessage = mono_image_strerror(status);
				VL_CORE_TRACE(errorMessage);
				return nullptr;
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);	// �ر�ӳ��

			delete[] fileData;		// �ͷ��ļ�����

			return assembly;		// ���س���ָ��
		}

		// ��ӡ�����е�������Ϣ
		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);												// ��ȡ����ӳ��
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);	// ��ȡ���Ͷ����
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);									// ��ȡ��������

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);				// ����������Ϣ

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);	// ��ȡ���������ռ�
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);			// ��ȡ�������� 

				VL_CORE_TRACE("---{}.{}", nameSpace, name);		// ��ӡ������Ϣ
			}
		}
	}

	// �洢�ű���������ݽṹ
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;	// ����
		MonoDomain* AppDomain = nullptr;	// Ӧ����

		MonoAssembly* CoreAssembly = nullptr;	// ���ĳ���
		MonoImage* CoreAssemblyImage = nullptr;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_MonoData = nullptr;	// �ű������ȫ������

	void ScriptEngine::Init()
	{
		s_MonoData = new ScriptEngineData();	// �����ű��������ݽṹ

		InitMono();	// ��ʼ��Mono����ʱ
	
		LoadAssembly("Resources/Scripts/Violet-ScriptCore.dll");
		LoadAssemblyClasses(s_MonoData->CoreAssembly);

		// ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		// Retrieve and instantiate class
		s_MonoData->EntityClass = ScriptClass("Violet", "Entity");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();	// �ر�Mono����ʱ
		delete s_MonoData;	// �ͷŽű��������ݽṹ
	}

	
	// ��ʼ��Mono����ʱ
	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");	// ���ó���·��

		MonoDomain* rootDomain = mono_jit_init("VioletJITRuntime");	// ��ʼ������
		VL_CORE_ASSERT(rootDomain);

		// �洢����ָ��
		s_MonoData->RootDomain = rootDomain;
	}

	// �ر�Mono����ʱ
	void ScriptEngine::ShutdownMono()
	{
		// NOTE(Yan): mono is a little confusing to shutdown, so maybe come back to this

		// �ر�MonoӦ����͸���
		s_MonoData->AppDomain = nullptr;
		s_MonoData->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_MonoData->AppDomain = mono_domain_create_appdomain("VioletScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

		// Move this maybe
		s_MonoData->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_MonoData->CoreAssemblyImage = mono_assembly_get_image(s_MonoData->CoreAssembly);
		Utils::PrintAssemblyTypes(s_MonoData->CoreAssembly);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_MonoData->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_MonoData->EntityClasses.find(fullClassName) != s_MonoData->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();		// �õ����ʵ������
		if (ScriptEngine::EntityClassExists(sc.ClassName)) {			// ����Ľű������Ƿ���ȷ
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_MonoData->EntityClasses[sc.ClassName], entity);// ʵ��������󣬲��洢OnCreate��OnUpdate���������ø���Entity�Ĺ��캯��������ʵ���UUID
			s_MonoData->EntityInstances[entity.GetUUID()] = instance;	// ���нű�map�洢��ЩScriptInstance(�����)
			instance->InvokeOnCreate();									// ����C#��OnCreate����
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		VL_CORE_ASSERT(s_MonoData->EntityInstances.find(entityUUID) != s_MonoData->EntityInstances.end());

		Ref<ScriptInstance> instance = s_MonoData->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_MonoData->SceneContext;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_MonoData->SceneContext = nullptr;

		s_MonoData->EntityInstances.clear();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_MonoData->EntityClasses;
	}

	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_MonoData->EntityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(image, "Violet", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

			if (monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (isEntity)
				s_MonoData->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_MonoData->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_MonoData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_MonoData->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_MonoData->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call Entity constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}
}