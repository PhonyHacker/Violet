#include "vlpch.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Violet {

	// 存储脚本引擎的数据结构
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;	// 根域
		MonoDomain* AppDomain = nullptr;	// 应用域

		MonoAssembly* CoreAssembly = nullptr;	// 核心程序集
	};

	static ScriptEngineData* s_MonoData = nullptr;	// 脚本引擎的全局数据

	void ScriptEngine::Init()
	{
		s_MonoData = new ScriptEngineData();	// 创建脚本引擎数据结构

		InitMono();	// 初始化Mono运行时
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();	// 关闭Mono运行时
		delete s_MonoData;	// 释放脚本引擎数据结构
	}

	// 读取文件的二进制数据
	char* ReadBytes(const std::string& filepath, uint32_t* outSize)
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
	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
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

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
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

	// 初始化Mono运行时
	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");	// 设置程序集路径

		MonoDomain* rootDomain = mono_jit_init("VioletJITRuntime");	// 初始化根域
		VL_CORE_ASSERT(rootDomain);

		// 存储根域指针
		s_MonoData->RootDomain = rootDomain;

		// 创建应用域
		s_MonoData->AppDomain = mono_domain_create_appdomain("VioletScriptRuntime", nullptr);
		mono_domain_set(s_MonoData->AppDomain, true);

		// 加载核心程序表，加载c#项目导出的dll
		s_MonoData->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/Violet-ScriptCore.dll");
		PrintAssemblyTypes(s_MonoData->CoreAssembly);

		MonoImage* assemblyImage = mono_assembly_get_image(s_MonoData->CoreAssembly);
		MonoClass* monoClass = mono_class_from_name(assemblyImage, "Violet", "Main");


		// 1. 创建对象（使用构造函数）
		MonoObject* instance = mono_object_new(s_MonoData->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		
		// 2. 调用函数
		MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
		mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);

		// 3. 调用有参函数
		MonoMethod* printIntFunc = mono_class_get_method_from_name(monoClass, "PrintInt", 1);

		int value = 5;
		void* param = &value;

		mono_runtime_invoke(printIntFunc, instance, &param, nullptr);
	
		// 4. 调用多个参数函数
		MonoMethod* printIntsFunc = mono_class_get_method_from_name(monoClass, "PrintInts", 2);
		int value2 = 508;
		void* params[2] =
		{
			&value,
			&value2
		};
		mono_runtime_invoke(printIntsFunc, instance, params, nullptr);

		// 5.调用带有字符串的有参函数
		MonoString* monoString = mono_string_new(s_MonoData->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
		void* stringParam = monoString;
		mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);

		// VL_CORE_ASSERT(false);
	}

	// 关闭Mono运行时
	void ScriptEngine::ShutdownMono()
	{
		// NOTE(Yan): mono is a little confusing to shutdown, so maybe come back to this

		// 关闭Mono应用域和根域
		// mono_domain_unload(s_Data->AppDomain);
		s_MonoData->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_MonoData->RootDomain = nullptr;
	}

}