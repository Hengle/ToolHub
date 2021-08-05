#pragma once
#include <Component.h>
namespace toolhub {
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
VENGINE_UNITY_EXTERN void toolhub_Component_SetBoolArray(Component* AFD920F282E74FF8, CSharpString name, BinaryArray value) {
	AFD920F282E74FF8->SetBoolArray(name, value);
}
VENGINE_UNITY_EXTERN void toolhub_Component_Reset(Component* AFD920F282E74FF8) {
	AFD920F282E74FF8->Reset();
}
VENGINE_UNITY_EXTERN void toolhub_Component_GetHandle(void*& F32D2BF6260A4FBD, Component* AFD920F282E74FF8) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->GetHandle();
}
VENGINE_UNITY_EXTERN void toolhub_Component_IsValueType(bool& F32D2BF6260A4FBD, Component* AFD920F282E74FF8) {
	F32D2BF6260A4FBD = AFD920F282E74FF8->IsValueType();
}
VENGINE_UNITY_EXTERN void Dispose_toolhub_Component(Component* v) {
	delete v;
}
VENGINE_UNITY_EXTERN Component* Create_toolhub_Component0(CSharpString typeName, void* parentDatabase) {
	return new Component(typeName, parentDatabase);
}
}// namespace toolhub
