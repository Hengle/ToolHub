#pragma vengine_package vengine_compute

#include <Database/SimpleJsonArray.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

void SimpleJsonArray::ExecuteLoad() {
	loader.Load(*this);
}
//Array DeSerialize
void SimpleJsonArray::Load(std::span<uint8_t> sp) {
	uint64 arrSize = PopValue<uint64>(sp);
	arrs.clear();
	arrs.reserve(arrSize);
	arrs.push_back_func(
		[&](size_t i) {
			return SimpleJsonLoader::DeSerialize(sp, db);
		},
		arrSize);
}
size_t SimpleJsonArray::Length() {
	ExecuteLoad();
	return arrs.size();
}
JsonVariant SimpleJsonArray::Get(size_t index) {
	ExecuteLoad();
	return arrs[index];
}
void SimpleJsonArray::Set(size_t index, JsonVariant value) {
	ExecuteLoad();
	isDirty = true;
	arrs[index] = std::move(value);
}
void SimpleJsonArray::Remove(size_t index) {
	ExecuteLoad();
	isDirty = true;
	arrs.erase(arrs.begin() + index);
}
void SimpleJsonArray::Add(JsonVariant value) {
	ExecuteLoad();
	isDirty = true;
	arrs.emplace_back(std::move(value));
}
vstd::unique_ptr<vstd::linq::Iterator<JsonVariant>> SimpleJsonArray::GetIterator() {
	ExecuteLoad();
	return new vstd::linq::IEnumerator(arrs);
}

vstd::optional<int64> SimpleJsonArray::GetInt(size_t index) {
	ExecuteLoad();
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
	ExecuteLoad();
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
	ExecuteLoad();
	auto&& v = arrs[index];
	if (v.GetType() == 2) {
		return *reinterpret_cast<vstd::string*>(v.GetPlaceHolder());
	}
	return nullptr;
}
vstd::optional<IJsonDict*> SimpleJsonArray::GetDict(size_t index) {
	ExecuteLoad();
	auto&& v = arrs[index];
	if (v.GetType() == 3) {
		return *reinterpret_cast<IJsonDict**>(v.GetPlaceHolder());
	}
	return nullptr;
}
vstd::optional<IJsonArray*> SimpleJsonArray::GetArray(size_t index) {
	ExecuteLoad();
	auto&& v = arrs[index];
	if (v.GetType() == 4) {
		return *reinterpret_cast<IJsonArray**>(v.GetPlaceHolder());
	}
	return nullptr;
}
void SimpleJsonArray::M_GetSerData(vstd::vector<uint8_t>& data) {
	ExecuteLoad();
	PushDataToVector(instanceID, data);
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
void SimpleJsonArray::DeSer(std::span<uint8_t> data) {
	auto lastID = instanceID;
	loader.Reset();
	loader.dataChunk = std::span<uint8_t>(data.data() + sizeof(uint64) * 2, data.size() - sizeof(uint64) * 2);
	ExecuteLoad();
	isDirty = true;
	instanceID = lastID;
}
}// namespace toolhub::db