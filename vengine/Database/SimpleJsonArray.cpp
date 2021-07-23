#pragma vengine_package vengine_compute

#include <Database/SimpleJsonArray.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
//Array DeSerialize
void SimpleJsonArray::Load(std::span<uint8_t> sp) {
	uint64 arrSize = PopValue<uint64>(sp);
	arrs.clear();
	arrs.reserve(arrSize);
	arrs.push_back_func(
		[&](size_t i) {
			return SimpleJsonLoader::DeSerialize(sp, jsonObj.db);
		},
		arrSize);
}
size_t SimpleJsonArray::Length() {
	jsonObj.loader.Load(*this);
	return arrs.size();
}
JsonVariant SimpleJsonArray::Get(size_t index) {
	jsonObj.loader.Load(*this);
	return arrs[index];
}
void SimpleJsonArray::Set(size_t index, JsonVariant value) {
	jsonObj.loader.Load(*this);
	jsonObj.Update();
	arrs[index] = std::move(value);
}
void SimpleJsonArray::Remove(size_t index) {
	jsonObj.loader.Load(*this);
	jsonObj.Update();
	arrs.erase(arrs.begin() + index);
}

void SimpleJsonArray::Add(JsonVariant value) {
	jsonObj.loader.Load(*this);
	jsonObj.Update();
	arrs.emplace_back(std::move(value));
}
vstd::unique_ptr<vstd::linq::Iterator<JsonVariant>> SimpleJsonArray::GetIterator() {
	jsonObj.loader.Load(*this);
	return new vstd::linq::IEnumerator(arrs);
}

vstd::optional<int64> SimpleJsonArray::GetInt(size_t index) {
	jsonObj.loader.Load(*this);
	auto&& v = arrs[index];
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return nullptr;
}
vstd::optional<double> SimpleJsonArray::GetFloat(size_t index) {
	jsonObj.loader.Load(*this);
	auto&& v = arrs[index];
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return nullptr;
}
vstd::optional<vstd::string_view> SimpleJsonArray::GetString(size_t index) {
	jsonObj.loader.Load(*this);
	auto&& v = arrs[index];
	if (v.GetType() == 2) {
		return *reinterpret_cast<vstd::string*>(v.GetPlaceHolder());
	}
	return nullptr;
}
vstd::optional<IJsonDict*> SimpleJsonArray::GetDict(size_t index) {
	jsonObj.loader.Load(*this);
	auto&& v = arrs[index];
	if (v.GetType() == 3) {
		return *reinterpret_cast<IJsonDict**>(v.GetPlaceHolder());
	}
	return nullptr;
}
vstd::optional<IJsonArray*> SimpleJsonArray::GetArray(size_t index) {
	jsonObj.loader.Load(*this);
	auto&& v = arrs[index];
	if (v.GetType() == 4) {
		return *reinterpret_cast<IJsonArray**>(v.GetPlaceHolder());
	}
	return nullptr;
}
void SimpleJsonArray::M_GetSerData(vstd::vector<uint8_t>& data) {
	jsonObj.loader.Load(*this);
	PushDataToVector<uint8_t>(0, data);
	PushDataToVector(jsonObj.version, data);
	PushDataToVector(jsonObj.instanceID, data);
	auto sizeOffset = data.size();
	data.resize(sizeOffset + sizeof(uint64));
	auto beginOffset = sizeOffset + sizeof(uint64);
	PushDataToVector(arrs.size(), data);
	for (auto&& v : arrs) {
		SimpleJsonLoader::Serialize(v, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}
}// namespace toolhub::db