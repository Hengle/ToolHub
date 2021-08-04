#pragma vengine_package vengine_unity

#include <Component.hpp>
#include <Common/DynamicDLL.h>

StackObject<DynamicDLL> database_dll;
VENGINE_UNITY_EXTERN void DllImport_Init(char const* dllPath) {
	char const* mallocPath = "mimalloc.dll";
	auto inputSize = strlen(mallocPath);
	auto dllPathSize = strlen(dllPath);
	char* mimallocPathStack = reinterpret_cast<char*>(alloca(inputSize + dllPathSize + 1));
	memcpy(mimallocPathStack, dllPath, dllPathSize);
	memcpy(mimallocPathStack + dllPathSize, mallocPath, inputSize);
	mimallocPathStack[dllPathSize + inputSize] = 0;
	vengine_init_malloc_path(mimallocPathStack);
	database_dll.New((vstd::string(dllPath) + "VEngine_Database.dll").c_str());
}
namespace toolhub {
Component::Component(CSharpString& typeName, bool& isValue) {
}
Component::Component(CSharpString& typeName, CSharpString& guid) {
}
Component::Component(void*& handle) {
}
bool Component::GetBool(CSharpString& name) {
}
int64 Component::GetInt(CSharpString& name) {
}
double Component::GetFloat(CSharpString& name) {
}
CSharpString Component::GetString(CSharpString& name) {
}
void* Component::GetComponent(CSharpString& name) {
}
BinaryArray Component::GetBoolArray(CSharpString& name) {
}
BinaryArray Component::GetIntArray(CSharpString& name) {
}
BinaryArray Component::GetFloatArray(CSharpString& name) {
}
BinaryArray Component::GetComponentArray(CSharpString& name) {
}
void Component::SetInt(CSharpString& name, int64& value) {
}
void Component::SetBool(CSharpString& name, bool& value) {
}
void Component::SetFloat(CSharpString& name, double& value) {
}
void Component::SetString(CSharpString& name, CSharpString& value) {
}
void Component::SetComponent(CSharpString& name, void*& comp) {
}
void Component::SetIntArray(CSharpString& name, BinaryArray& value) {
}
void Component::SetFloatArray(CSharpString& name, BinaryArray& value) {
}
void Component::SetComponentArray(CSharpString& name, BinaryArray& value) {
}
void Component::Reset() {
}
void* Component::GetHandle() {
}
ComponentType Component::GetCompType() {
}
vstd::string CSharpString::ToString() {
	vstd::string str;
	str.resize(size);
	auto ptr = str.data();
	for (auto i : vstd::ptr_range(ptr, ptr + size)) {
		*ptr = i;
	}
	return str;
}
}// namespace toolhub