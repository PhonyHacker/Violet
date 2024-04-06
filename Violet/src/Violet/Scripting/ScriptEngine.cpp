#include "vlpch.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Violet {

	// �洢�ű���������ݽṹ
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;	// ����
		MonoDomain* AppDomain = nullptr;	// Ӧ����

		MonoAssembly* CoreAssembly = nullptr;	// ���ĳ���
	};

	static ScriptEngineData* s_MonoData = nullptr;	// �ű������ȫ������

	void ScriptEngine::Init()
	{
		s_MonoData = new ScriptEngineData();	// �����ű��������ݽṹ

		InitMono();	// ��ʼ��Mono����ʱ
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();	// �ر�Mono����ʱ
		delete s_MonoData;	// �ͷŽű��������ݽṹ
	}

	// ��ȡ�ļ��Ķ���������
	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
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
	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
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

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
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

	// ��ʼ��Mono����ʱ
	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");	// ���ó���·��

		MonoDomain* rootDomain = mono_jit_init("VioletJITRuntime");	// ��ʼ������
		VL_CORE_ASSERT(rootDomain);

		// �洢����ָ��
		s_MonoData->RootDomain = rootDomain;

		// ����Ӧ����
		s_MonoData->AppDomain = mono_domain_create_appdomain("VioletScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

		// ���غ��ĳ��������c#��Ŀ������dll
		s_MonoData->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/Violet-ScriptCore.dll");
		PrintAssemblyTypes(s_MonoData->CoreAssembly);

		MonoImage* assemblyImage = mono_assembly_get_image(s_MonoData->CoreAssembly);
		MonoClass* monoClass = mono_class_from_name(assemblyImage, "Violet", "Main");


		// 1. ��������ʹ�ù��캯����
		MonoObject* instance = mono_object_new(s_MonoData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		
		// 2. ���ú���
		MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
		mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);

		// 3. �����вκ���
		MonoMethod* printIntFunc = mono_class_get_method_from_name(monoClass, "PrintInt", 1);

		int value = 5;
		void* param = &value;

		mono_runtime_invoke(printIntFunc, instance, &param, nullptr);
	
		// 4. ���ö����������
		MonoMethod* printIntsFunc = mono_class_get_method_from_name(monoClass, "PrintInts", 2);
		int value2 = 508;
		void* params[2] =
		{
			&value,
			&value2
		};
		mono_runtime_invoke(printIntsFunc, instance, params, nullptr);

		// 5.���ô����ַ������вκ���
		MonoString* monoString = mono_string_new(s_MonoData->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
		void* stringParam = monoString;
		mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);

		// VL_CORE_ASSERT(false);
	}

	// �ر�Mono����ʱ
	void ScriptEngine::ShutdownMono()
	{
		// NOTE(Yan): mono is a little confusing to shutdown, so maybe come back to this

		// �ر�MonoӦ����͸���
		// mono_domain_unload(s_Data->AppDomain);
		s_MonoData->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_MonoData->RootDomain = nullptr;
	}

}