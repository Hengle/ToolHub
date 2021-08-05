#pragma vengine_package vengine_unity

#include <Component.hpp>
#include <Common/DynamicDLL.h>

#include <Database/IJsonObject.h>
#include <Database/IJsonDatabase.h>
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
Component::Component(CSharpString& typeName, void*& parentDatabase) {
	// Value Type
	if (parentDatabase == nullptr) {
		data.update(data.IndexOf<ValueMap>, [](void* hashMap) {
			new (hashMap) ValueMap();
		});
	}
	// Ref Type
	else {
		auto subDB = reinterpret_cast<db::IJsonSubDatabase*>(parentDatabase);
		data = subDB->CreateJsonObject();
	}
}
Component::Component(vstd::unique_ptr<db::IJsonDict>&& ptr)
	: data(std::move(ptr)) {
}
Component::~Component() {
}
bool Component::GetBool(CSharpString& name) {
	return 0;
}
int64 Component::GetInt(CSharpString& name) {
	return 0;
}
double Component::GetFloat(CSharpString& name) {
	return 0;
}
CSharpString Component::GetString(CSharpString& name) {
	return CSharpString();
}
void* Component::GetComponent(CSharpString& name) {
	return nullptr;
}
BinaryArray Component::GetBoolArray(CSharpString& name) {
	return BinaryArray();
}
BinaryArray Component::GetIntArray(CSharpString& name) {
	return BinaryArray();
}
BinaryArray Component::GetFloatArray(CSharpString& name) {
	return BinaryArray();
}
BinaryArray Component::GetComponentArray(CSharpString& name) {
	return BinaryArray();
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
void Component::SetBoolArray(CSharpString& name, BinaryArray& value) {
}
void Component::Reset() {
	data.visit(
		[](auto&& v) {
			v.Clear();
		},
		[](auto&& v) {
			v = nullptr;
		});
}
void* Component::GetHandle() {
	return this;
}
bool Component::IsValueType() {
	return data.GetType() == data.IndexOf<ValueMap>;
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