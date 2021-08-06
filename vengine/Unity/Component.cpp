#pragma vengine_package vengine_unity

#include <Component.hpp>
#include <Common/DynamicDLL.h>

#include <Database/IJsonObject.h>
#include <Database/IJsonDatabase.h>
#include <Network/FunctionSerializer.h>
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
	return data.visit(
		[&](ValueMap& v) -> bool {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return false;
			auto&& value = ite.Value();
			if (value.GetType() != value.IndexOf<bool>) return false;
			return value.get<value.IndexOf<bool>>();
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->GetInt(name.ToSV());
			return value ? *value : false;
		});
}
int64 Component::GetInt(CSharpString& name) {
	auto retZero = [](auto&&) -> int64 { return 0; };
	auto retValue = [](auto&& v) -> int64 {
		return v;
	};

	return data.visit(
		[&](ValueMap& v) -> int64 {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return 0;

			return ite.Value().visit(
				retValue,
				retValue,
				retZero,
				retZero,
				retZero,
				retZero);
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->Get(name.ToSV());
			return value.visit(
				retValue,
				retValue,
				retZero,
				retZero,
				retZero);
		});
}
double Component::GetFloat(CSharpString& name) {
	auto retZero = [](auto&&) -> double { return 0; };
	auto retValue = [](auto&& v) -> double {
		return v;
	};
	return data.visit(
		[&](ValueMap& v) -> double {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return 0;

			return ite.Value().visit(
				retValue,
				retValue,
				retZero,
				retZero,
				retZero,
				retZero);
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->Get(name.ToSV());
			return value.visit(
				retValue,
				retValue,
				retZero,
				retZero,
				retZero);
		});
}
CSharpString Component::GetString(CSharpString& name) {
	auto retZero = [](auto&&) -> CSharpString { return CSharpString(); };
	return data.visit(
		[&](ValueMap& v) -> CSharpString {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return CSharpString();
			if (ite.Value().IsTypeOf<vstd::string>()) {
				return CSharpString(ite.Value().get<ValueStructType::IndexOf<vstd::string>>());
			}
			return CSharpString();
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->Get(name.ToSV());
			if (value.IsTypeOf<vstd::string_view>()) {
				return CSharpString(value.get<db::JsonVariant::IndexOf<vstd::string_view>>());
			}
			return CSharpString();
		});
}
void* Component::GetComponent(CSharpString& name) {
	return data.visit(
		[&](ValueMap& v) -> void* {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return nullptr;
			auto&& value = ite.Value();
			if (value.IsTypeOf<vstd::unique_ptr<Component>>()) {
				return value.get<value.IndexOf<vstd::unique_ptr<Component>>>().get();
			}
			return nullptr;
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->Get(name.ToSV());
			if (value.IsTypeOf<db::IJsonDict*>()) {
				return value.get<db::JsonVariant::IndexOf<db::IJsonDict*>>();
			}
		});
}
BinaryArray Component::GetArray(CSharpString& name, BinaryType tarType, size_t stride) {
	auto retZero = [](auto&&) { return BinaryArray(); };
	auto getSP = [&](std::span<uint8_t> sp) {
		BinaryType type = vstd::SerDe<BinaryType>::Get(sp);
		if (type != tarType) return BinaryArray();
		return BinaryArray(
			sp.data(), sp.size() / stride, stride);
	};
	return data.visit(
		[&](ValueMap& v) -> BinaryArray {
			auto str = name.ToString();
			auto ite = v.Find(str);
			if (!ite) return BinaryArray();
			return ite.Value().visit(
				retZero,
				retZero,
				retZero,
				retZero,
				retZero,
				getSP,
				retZero);
		},
		[&](vstd::unique_ptr<db::IJsonDict>& v) {
			auto value = v->Get(name.ToSV());
			return value.visit(
				retZero,
				retZero,
				retZero,
				retZero,
				retZero,
				getSP);
		});
}
BinaryArray Component::GetBoolArray(CSharpString& name) {
	return GetArray(name, BinaryType::Bool, sizeof(bool));
}
BinaryArray Component::GetIntArray(CSharpString& name) {
	return GetArray(name, BinaryType::Int, sizeof(int64));
}
BinaryArray Component::GetFloatArray(CSharpString& name) {
	return GetArray(name, BinaryType::Float, sizeof(double));
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
vstd::string CSharpString::ToString() const {
	vstd::string str;
	str.resize(size);
	auto ptr = str.data();
	for (auto i : vstd::ptr_range(ptr, ptr + size)) {
		*ptr = i;
	}
	return str;
}
vstd::string_view CSharpString::ToSV() const {
	return vstd::string_view(ptr, size);
}
}// namespace toolhub