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
	};

	static ScriptEngineData* s_MonoData = nullptr;	// �ű������ȫ������

	void ScriptEngine::Init()
	{
		s_MonoData = new ScriptEngineData();	// �����ű��������ݽṹ

		InitMono();	// ��ʼ��Mono����ʱ
	
		LoadAssembly("Resources/Scripts/Violet-ScriptCore.dll");

		ScriptGlue::RegisterFunctions();

		s_MonoData->EntityClass = ScriptClass("Violet", "Entity");

		MonoObject* instance = s_MonoData->EntityClass.Instantiate();

		MonoMethod* printMessageFunc = s_MonoData->EntityClass.GetMethod("PrintMessage", 0);
		s_MonoData->EntityClass.InvokeMethod(instance, printMessageFunc);

		MonoMethod* printIntFunc = s_MonoData->EntityClass.GetMethod("PrintInt", 1);

		int value = 5;
		void* param = &value;

		s_MonoData->EntityClass.InvokeMethod(instance, printIntFunc, &param);

		MonoMethod* printIntsFunc = s_MonoData->EntityClass.GetMethod("PrintInts", 2);
		int value2 = 508;
		void* params[2] =
		{
			&value,
			&value2
		};
		s_MonoData->EntityClass.InvokeMethod(instance, printIntsFunc, params);

		MonoString* monoString = mono_string_new(s_MonoData->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = s_MonoData->EntityClass.GetMethod("PrintCustomMessage", 1);
		void* stringParam = monoString;
		s_MonoData->EntityClass.InvokeMethod(instance, printCustomMessageFunc, &stringParam);

		// VL_CORE_ASSERT(false);
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
		s_MonoData->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

		// Move this maybe
		s_MonoData->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_MonoData->CoreAssemblyImage = mono_assembly_get_image(s_MonoData->CoreAssembly);
		// Utils::PrintAssemblyTypes(s_MonoData->CoreAssembly);
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
}