#include "vlpch.h"
#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include "Violet/Core/Application.h"
#include "Violet/Core/Timer.h"
#include "Violet/Project/Project.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

#include "FileWatch.h"

namespace Violet {
	std::string ScriptEngine::s_ProjectName = "";

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
		// 读取文件的二进制数据
		char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			// 失败时返回空指针
			if (!stream)
			{
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			// 文件为空时返回空指针
			if (size == 0)
			{
				return nullptr;
			}

			char* buffer = new char[size];		// 创建一个缓冲区
			stream.read((char*)buffer, size);	// 读取文件数据到缓冲区
			stream.close();

			*outSize = size;	// 更新输出参数的大小
			return buffer;		// 返回缓冲区指针	
		}

		// 加在C#程序集
		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);	// 读取文件数据

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			// 打开数据集映像
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				// 打印错误消息
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
			mono_image_close(image);	// 关闭映像

			delete[] fileData;		// 释放文件数据

			return assembly;		// 返回程序集指针
		}

		// 打印程序集中的类型信息
		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);												// 获取程序集映像
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);	// 获取类型定义表
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);									// 获取类型数量

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);				// 解码类型信息

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);	// 获取类型命名空间
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);			// 获取类型名称 

				VL_CORE_TRACE("---{}.{}", nameSpace, name);		// 打印类型信息
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

	// 存储脚本引擎的数据结构
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;	// 根域
		MonoDomain* AppDomain = nullptr;	// 应用域

		MonoAssembly* CoreAssembly = nullptr;	// 核心程序集
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

	static ScriptEngineData* s_MonoData = nullptr;	// 脚本引擎的全局数据
	static bool s_IsMonoInited = false;

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
		s_MonoData = new ScriptEngineData();	// 创建脚本引擎数据结构

		if (s_IsMonoInited)
		{
			ShutdownMonoApp();
		}

		InitMono();	// 初始化Mono运行时
		s_IsMonoInited = true;

		ScriptGlue::RegisterFunctions();

		bool status = LoadAssembly("Resources/Scripts/Violet-ScriptCore.dll");
		if (!status)
		{
			VL_CORE_ERROR("[ScriptEngine] Could not load Violet-ScriptCore assembly.");
			return;
		}

		auto scriptModulePath = Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		status = LoadAppAssembly(scriptModulePath);
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
		ShutdownMono();	// 关闭Mono运行时
		delete s_MonoData;	// 释放脚本引擎数据结构
	}

	// 初始化Mono运行时
	void ScriptEngine::InitMono()
	{
		if (s_IsMonoInited)
			return;

		mono_set_assemblies_path("mono/lib");	// 设置程序集路径

		if (s_MonoData->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}
		
		MonoDomain* rootDomain = mono_jit_init("VioletJITRuntime");	// 初始化根域
		VL_CORE_ASSERT(rootDomain);

		// 存储根域指针
		s_MonoData->RootDomain = rootDomain;

		if (s_MonoData->EnableDebugging)
			mono_debug_domain_create(s_MonoData->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMonoApp()
	{
		// 关闭Mono应用域
		if (s_MonoData->AppDomain != nullptr)
		{
			mono_domain_unload(s_MonoData->AppDomain);
			delete s_MonoData->AppDomain; 
			s_MonoData->AppDomain = nullptr;
		}
	}

	// 关闭Mono运行时
	void ScriptEngine::ShutdownMono()
	{
		if (s_MonoData == nullptr)
			return;

		mono_domain_set(mono_get_root_domain(), false);

		// 关闭Mono应用域和根域
		if(s_MonoData->AppDomain != nullptr)
		{
			mono_domain_unload(s_MonoData->AppDomain);
			s_MonoData->AppDomain = nullptr;
		}
		if (s_MonoData->RootDomain != nullptr)
		{
			mono_jit_cleanup(s_MonoData->RootDomain);
			s_MonoData->RootDomain = nullptr;
		}

	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		s_MonoData->AppDomain = mono_domain_create_appdomain("VioletScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

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
		const auto& sc = entity.GetComponent<ScriptComponent>();		// 得到这个实体的组件
		if (ScriptEngine::EntityClassExists(sc.ClassName))				// 组件的脚本名称是否正确
		{			
			UUID entityID = entity.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_MonoData->EntityClasses[sc.ClassName], entity);// 实例化类对象，并存储OnCreate、OnUpdate函数，调用父类Entity的构造函数，传入实体的UUID
			s_MonoData->EntityInstances[entityID] = instance;	// 运行脚本map存储这些ScriptInstance(类对象)
			
			// Copy field values
			if (s_MonoData->EntityScriptFields.find(entityID) != s_MonoData->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_MonoData->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
					instance->SetFieldValueInternal(name, fieldInstance.Buffer);
			}
			
			instance->InvokeOnCreate();									// 调用C#的OnCreate函数
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
		// 清除已加载的实体类信息
		s_MonoData->EntityClasses.clear();

		// 获取类型定义表信息
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_MonoData->AppAssemblyImage, MONO_TABLE_TYPEDEF);

		// 获取类型数量
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_MonoData->CoreAssemblyImage, "Violet", "Entity");

		// 遍历类型定义表
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			// 获取命名空间和类名
			const char* nameSpace = mono_metadata_string_heap(s_MonoData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_MonoData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			// 获取类信息
			MonoClass* monoClass = mono_class_from_name(s_MonoData->AppAssemblyImage, nameSpace, className);

			// 检查类是否是实体类的子类
			if (monoClass == entityClass)
				continue;

			if (!mono_class_is_subclass_of(monoClass, entityClass, false))
				continue;

			// 创建 ScriptClass 对象并存储在 EntityClasses 字典中
			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_MonoData->EntityClasses[fullName] = scriptClass;

			// 遍历类的字段信息
			int fieldCount = mono_class_num_fields(monoClass);
#ifdef VL_DEBUG
			VL_CORE_WARN("{} has {} fields:", className, fieldCount);
#endif
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);	// 访问级别
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					// 获取字段类型并存储在 ScriptClass 对象中
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
#ifdef VL_DEBUG
					VL_CORE_TRACE("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));
#endif // VL_DEBUG
					scriptClass->Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_MonoData->CoreAssemblyImage;
	}

	MonoString* ScriptEngine::CreateString(const char* string)
	{
		return mono_string_new(s_MonoData->AppDomain, string);
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
			//MonoObject* exception = nullptr;
			//mono_runtime_invoke(m_OnUpdateMethod, m_Instance, &param, &exception);
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
