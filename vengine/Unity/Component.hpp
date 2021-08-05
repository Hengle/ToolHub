#pragma once
#include <Unity/UnityInclude.h>
namespace toolhub {
namespace db {
class IJsonDict;
class IJsonDatabase;
}
class Component final : public vstd::IOperatorNewBase {
private:
	using ComponentVariant = vstd::variant<
		int64,
		double,
		vstd::string,
		Component*>;
	struct ValueType {
		HashMap<vstd::string, ComponentVariant> keyValues;
	};
	struct RefType {
		vstd::unique_ptr<db::IJsonDict> db;
	};
	struct AssetType {
		vstd::string guid;
	};

	vstd::variant<
		AssetType,
		RefType,
		ValueType
		>
		data;

public:

	Component(CSharpString& typeName, bool& isValue);
	Component(CSharpString& typeName, CSharpString& guid);
	Component(void*& handle);
	~Component();
	bool GetBool(CSharpString& name);
	int64 GetInt(CSharpString& name);
	double GetFloat(CSharpString& name);
	CSharpString GetString(CSharpString& name);
	void* GetComponent(CSharpString& name);
	BinaryArray GetBoolArray(CSharpString& name);
	BinaryArray GetIntArray(CSharpString& name);
	BinaryArray GetFloatArray(CSharpString& name);
	BinaryArray GetComponentArray(CSharpString& name);
	void SetInt(CSharpString& name, int64& value);
	void SetBool(CSharpString& name, bool& value);
	void SetFloat(CSharpString& name, double& value);
	void SetString(CSharpString& name, CSharpString& value);
	void SetComponent(CSharpString& name, void*& comp);
	void SetIntArray(CSharpString& name, BinaryArray& value);
	void SetFloatArray(CSharpString& name, BinaryArray& value);
	void SetComponentArray(CSharpString& name, BinaryArray& value);
	void Reset();
	void* GetHandle();
	ComponentType GetCompType();
};
VENGINE_UNITY_EXTERN void toolhub_Component_GetBool(bool& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetBool(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetInt(int64& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetInt(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetFloat(double& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetFloat(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetString(CSharpString& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetString(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetComponent(void*& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetComponent(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetBoolArray(BinaryArray& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetBoolArray(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetIntArray(BinaryArray& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetIntArray(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetFloatArray(BinaryArray& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetFloatArray(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetComponentArray(BinaryArray& F32D2BF6260A4FBD, Component* AFD920F282E74FF8, CSharpString name) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetComponentArray(name);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetInt(Component* AFD920F282E74FF8, CSharpString name, int64 value) {
	AFD920F282E74FF8->SetInt(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetBool(Component* AFD920F282E74FF8, CSharpString name, bool value) {
	AFD920F282E74FF8->SetBool(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetFloat(Component* AFD920F282E74FF8, CSharpString name, double value) {
	AFD920F282E74FF8->SetFloat(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetString(Component* AFD920F282E74FF8, CSharpString name, CSharpString value) {
	AFD920F282E74FF8->SetString(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetComponent(Component* AFD920F282E74FF8, CSharpString name, void* comp) {
	AFD920F282E74FF8->SetComponent(name, comp);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetIntArray(Component* AFD920F282E74FF8, CSharpString name, BinaryArray value) {
	AFD920F282E74FF8->SetIntArray(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetFloatArray(Component* AFD920F282E74FF8, CSharpString name, BinaryArray value) {
	AFD920F282E74FF8->SetFloatArray(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_SetComponentArray(Component* AFD920F282E74FF8, CSharpString name, BinaryArray value) {
	AFD920F282E74FF8->SetComponentArray(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_Reset(Component* AFD920F282E74FF8) {
	AFD920F282E74FF8->Reset();
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetHandle(void*& F32D2BF6260A4FBD, Component* AFD920F282E74FF8) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetHandle();
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetCompType(ComponentType& F32D2BF6260A4FBD, Component* AFD920F282E74FF8) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetCompType();
}
VENGINE_UNITY_EXTERN void Dispose_toolhub_Component(Component* v) {
	delete v;
}
VENGINE_UNITY_EXTERN Component* Create_toolhub_Component0(CSharpString typeName, bool isValue) {
	return new Component(typeName, isValue);
}
VENGINE_UNITY_EXTERN Component* Create_toolhub_Component1(CSharpString typeName, CSharpString guid) {
	return new Component(typeName, guid);
}
VENGINE_UNITY_EXTERN Component* Create_toolhub_Component2(void* handle) {
	return new Component(handle);
}
}// namespace toolhub
