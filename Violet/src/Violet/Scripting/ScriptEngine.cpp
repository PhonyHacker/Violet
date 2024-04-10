#include "vlpch.h"
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include "Violet/Core/Application.h"
#include "Violet/Core/Timer.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

#include "FileWatch.h"

namespace Violet {
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },

		{ "Violet.Vector2", ScriptFieldType::Vector2 },
		{ "Violet.Vector3", ScriptFieldType::Vector3 },
		{ "Violet.Vector4", ScriptFieldType::Vector4 },

		{ "Violet.Entity", ScriptFieldType::Entity },
	};

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
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
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

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					uint32_t pdbFileSize = 0;
					char* pdbFileData = ReadBytes(pdbPath, &pdbFileSize);
					mono_debug_open_image_from_memory(image, (const mono_byte*)pdbFileData, pdbFileSize);
					VL_CORE_INFO("Loaded PDB {}", pdbPath);
					delete[] pdbFileData;
				}
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

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				VL_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}

	}

	// �洢�ű���������ݽṹ
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;	// ����
		MonoDomain* AppDomain = nullptr;	// Ӧ����

		MonoAssembly* CoreAssembly = nullptr;	// ���ĳ���
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;
#ifdef VL_DEBUG
		bool EnableDebugging = true;
#else
		bool EnableDebugging = false;
#endif
		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_MonoData = nullptr;	// �ű������ȫ������

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_MonoData->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_MonoData->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([]()
				{
					s_MonoData->AppAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly();
				});
		}
	}

	void ScriptEngine::Init()
	{
		s_MonoData = new ScriptEngineData();	// �����ű��������ݽṹ

		InitMono();	// ��ʼ��Mono����ʱ
		ScriptGlue::RegisterFunctions();

		bool status = LoadAssembly("Resources/Scripts/Violet-ScriptCore.dll");
		if (!status)
		{
			VL_CORE_ERROR("[ScriptEngine] Could not load Violet-ScriptCore assembly.");
			return;
		}
		status = LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		if (!status)
		{
			VL_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_MonoData->EntityClass = ScriptClass("Violet", "Entity", true);
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

		if (s_MonoData->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("VioletJITRuntime");	// ��ʼ������
		VL_CORE_ASSERT(rootDomain);

		// �洢����ָ��
		s_MonoData->RootDomain = rootDomain;

		if (s_MonoData->EnableDebugging)
			mono_debug_domain_create(s_MonoData->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	// �ر�Mono����ʱ
	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		// �ر�MonoӦ����͸���
		mono_domain_unload(s_MonoData->AppDomain);
		s_MonoData->AppDomain = nullptr;

		mono_jit_cleanup(s_MonoData->RootDomain);
		s_MonoData->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_MonoData->AppDomain = mono_domain_create_appdomain("VioletScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

		// Move this maybe
		s_MonoData->CoreAssemblyFilepath = filepath;
		s_MonoData->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_MonoData->EnableDebugging);
		if (s_MonoData->CoreAssembly == nullptr)
			return false;
		s_MonoData->CoreAssemblyImage = mono_assembly_get_image(s_MonoData->CoreAssembly);
		// Utils::PrintAssemblyTypes(s_MonoData->CoreAssembly);

		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// Move this maybe
		s_MonoData->AppAssemblyFilepath = filepath;
		s_MonoData->AppAssembly = Utils::LoadMonoAssembly(filepath, s_MonoData->EnableDebugging);
		if (s_MonoData->CoreAssembly == nullptr)
			return false;

		auto assemb = s_MonoData->AppAssembly;
		s_MonoData->AppAssemblyImage = mono_assembly_get_image(s_MonoData->AppAssembly);
		auto assembi = s_MonoData->AppAssemblyImage;
		// Utils::PrintAssemblyTypes(s_Data->AppAssembly);

		s_MonoData->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		s_MonoData->AssemblyReloadPending = false;
		
		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_MonoData->AppDomain);

		LoadAssembly(s_MonoData->CoreAssemblyFilepath);
		LoadAppAssembly(s_MonoData->AppAssemblyFilepath);
		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_MonoData->EntityClass = ScriptClass("Violet", "Entity", true);
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
		if (ScriptEngine::EntityClassExists(sc.ClassName))				// ����Ľű������Ƿ���ȷ
		{			
			UUID entityID = entity.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_MonoData->EntityClasses[sc.ClassName], entity);// ʵ��������󣬲��洢OnCreate��OnUpdate���������ø���Entity�Ĺ��캯��������ʵ���UUID
			s_MonoData->EntityInstances[entityID] = instance;	// ���нű�map�洢��ЩScriptInstance(�����)
			
			// Copy field values
			if (s_MonoData->EntityScriptFields.find(entityID) != s_MonoData->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_MonoData->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}
			
			instance->InvokeOnCreate();									// ����C#��OnCreate����
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();

		if (s_MonoData->EntityInstances.find(entityUUID) != s_MonoData->EntityInstances.end())
		{
			Ref<ScriptInstance> instance = s_MonoData->EntityInstances[entityUUID];
			instance->InvokeOnUpdate((float)ts);
		}
		else
		{
			VL_CORE_ERROR("Could not find ScriptInstance for entity {}", entityUUID);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_MonoData->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_MonoData->EntityInstances.find(entityID);
		if (it == s_MonoData->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_MonoData->EntityClasses.find(name) == s_MonoData->EntityClasses.end())
			return nullptr;

		return s_MonoData->EntityClasses.at(name);
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

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		VL_CORE_ASSERT(entity);

		UUID entityID = entity.GetUUID();
		return s_MonoData->EntityScriptFields[entityID];
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		// ����Ѽ��ص�ʵ������Ϣ
		s_MonoData->EntityClasses.clear();

		// ��ȡ���Ͷ������Ϣ
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_MonoData->AppAssemblyImage, MONO_TABLE_TYPEDEF);

		// ��ȡ��������
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_MonoData->CoreAssemblyImage, "Violet", "Entity");

		// �������Ͷ����
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			// ��ȡ�����ռ������
			const char* nameSpace = mono_metadata_string_heap(s_MonoData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_MonoData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			// ��ȡ����Ϣ
			MonoClass* monoClass = mono_class_from_name(s_MonoData->AppAssemblyImage, nameSpace, className);

			// ������Ƿ���ʵ���������
			if (monoClass == entityClass)
				continue;

			if (!mono_class_is_subclass_of(monoClass, entityClass, false))
				continue;

			// ���� ScriptClass ���󲢴洢�� EntityClasses �ֵ���
			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_MonoData->EntityClasses[fullName] = scriptClass;

			// ��������ֶ���Ϣ
			int fieldCount = mono_class_num_fields(monoClass);
			VL_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);	// ���ʼ���
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					// ��ȡ�ֶ����Ͳ��洢�� ScriptClass ������
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					VL_CORE_TRACE("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}

		auto& entityClasses = s_MonoData->EntityClasses;
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

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		if(isCore)
			m_MonoClass = mono_class_from_name(s_MonoData->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
		else
			m_MonoClass = mono_class_from_name(s_MonoData->AppAssemblyImage, classNamespace.c_str(), className.c_str());
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
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		VL_CORE_ASSERT(s_MonoData->EntityInstances.find(uuid) != s_MonoData->EntityInstances.end());
		return s_MonoData->EntityInstances.at(uuid)->GetManagedObject();
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

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}
}