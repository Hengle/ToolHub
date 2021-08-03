#pragma vengine_package vengine_database

#include <Database/SimpleJsonArray.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

size_t SimpleJsonArray::Length() {

	return arrs.size();
}
JsonVariant SimpleJsonArray::Get(size_t index) {

	return arrs[index];
}
void SimpleJsonArray::LoadFromData(std::span<uint8_t> data) {
	if (!data.empty()) {
		uint64 arrSize = PopValue<uint64>(data);
		arrs.clear();
		arrs.reserve(arrSize);
		arrs.push_back_func(
			[&]() {
				return SimpleJsonLoader::DeSerialize(data, db);
			},
			arrSize);
	}
}
void SimpleJsonArray::Set(size_t index, JsonVariant value) {
	Update();
	arrs[index] = std::move(value);
}
void SimpleJsonArray::Remove(size_t index) {

	Update();
	arrs.erase(arrs.begin() + index);
}
IJsonDataBase* SimpleJsonArray::GetDatabase() { return db; }

void SimpleJsonArray::Add(JsonVariant value) {

	Update();
	arrs.emplace_back(std::move(value));
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> SimpleJsonArray::GetIterator() {

	return new vstd::linq::ConstIEnumerator(arrs);
}

vstd::optional<int64> SimpleJsonArray::GetInt(size_t index) {

	auto&& v = arrs[index];
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return vstd::optional<int64>();
}
vstd::optional<double> SimpleJsonArray::GetFloat(size_t index) {

	auto&& v = arrs[index];
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return vstd::optional<double>();
}
vstd::optional<vstd::string_view> SimpleJsonArray::GetString(size_t index) {

	auto&& v = arrs[index];
	if (v.GetType() == 2) {
		return *reinterpret_cast<vstd::string*>(v.GetPlaceHolder());
	}
	return vstd::optional<vstd::string_view>();
}
vstd::optional<IJsonDict*> SimpleJsonArray::GetDict(size_t index) {

	auto&& v = arrs[index];
	if (v.GetType() == 3) {
		auto id = *reinterpret_cast<uint64*>(v.GetPlaceHolder());
		auto ptr = db->GetJsonObject(id);
		if (ptr)
			return vstd::optional<IJsonDict*>(ptr);
	}
	return vstd::optional<IJsonDict*>();
}
vstd::optional<IJsonArray*> SimpleJsonArray::GetArray(size_t index) {

	auto&& v = arrs[index];
	if (v.GetType() == 4) {
		auto id = *reinterpret_cast<uint64*>(v.GetPlaceHolder());
		auto ptr = db->GetJsonArray(id);
		if (ptr)
			return vstd::optional<IJsonArray*>(ptr);
	}
	return vstd::optional<IJsonArray*>();
}
void SimpleJsonArray::M_GetSerData(vstd::vector<uint8_t>& data) {
	auto v = ARRAY_TYPE;
	PushDataToVector<uint8_t&>(v, data);
	PushDataToVector(instanceID, data);
	auto sizeOffset = data.size();
	data.resize(sizeOffset + sizeof(uint64));
	auto beginOffset = sizeOffset + sizeof(uint64);
	PushDataToVector(arrs.size(), data);
	for (auto&& v : arrs) {
		SimpleJsonLoader::Serialize(db, v, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}
SimpleJsonArray::SimpleJsonArray(uint64 instanceID, SimpleBinaryJson* db)
	: SimpleJsonObject(instanceID, db) {
}
SimpleJsonArray::~SimpleJsonArray() {
}
void SimpleJsonArray::AfterAdd(IDatabaseEvtVisitor* visitor) {
	visitor->AddArray(this);
}
void SimpleJsonArray::BeforeRemove(IDatabaseEvtVisitor* visitor) {
	visitor->RemoveArray(this);
}
void SimpleJsonArray::Dispose() {
	db->Dispose(this);
}
void SimpleJsonArray::Clean() {
	arrs.compact([&](JsonVariant const& v) {
		return SimpleJsonLoader::Check(db, v);
	});
}

}// namespace toolhub::db