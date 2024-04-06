#include "vlpch.h"
#include "ScriptGlue.h"

#include "mono/metadata/object.h"

namespace Violet {
#define VL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Violet.InternalCalls::" #Name, Name)

	// ���徲̬C++������������C++��ʵ�־����߼�

	// ��MonoStringת��ΪUTF-8�ַ�������ӡ���
	static void NativeLog(MonoString* string, int parameter)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << parameter << std::endl;
	}

	// ��ӡ�����glm::vec3��������һ��
	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		//VL_CORE_WARN("Value: {0}", *parameter);
		std::cout << parameter->x << "," << parameter->y << "," << parameter->z << std::endl;
		*outResult = glm::normalize(*parameter);
	}

	// ���������glm::vec3����������ĵ�������ؽ��
	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		//VL_CORE_WARN("Value: {0}", *parameter);
		std::cout << parameter->x << "," << parameter->y << "," << parameter->z << std::endl;
		return glm::dot(*parameter, *parameter);
	}

	// ע�ắ������C++����ע��ΪC#�е��ڲ�����
	void ScriptGlue::RegisterFunctions()
	{
		VL_ADD_INTERNAL_CALL(NativeLog);
		VL_ADD_INTERNAL_CALL(NativeLog_Vector);
		VL_ADD_INTERNAL_CALL(NativeLog_VectorDot);
	}

}