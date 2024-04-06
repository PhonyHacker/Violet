#include "vlpch.h"
#include "ScriptGlue.h"

#include "mono/metadata/object.h"

namespace Violet {
#define VL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Violet.InternalCalls::" #Name, Name)

	// 定义静态C++函数，用于在C++中实现具体逻辑

	// 将MonoString转换为UTF-8字符串并打印输出
	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
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

	// 注册函数，将C++函数注册为C#中的内部调用
	void ScriptGlue::RegisterFunctions()
	{
		VL_ADD_INTERNAL_CALL(NativeLog);
		VL_ADD_INTERNAL_CALL(NativeLog_Vector);
		VL_ADD_INTERNAL_CALL(NativeLog_VectorDot);
	}

}