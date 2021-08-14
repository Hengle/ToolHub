#pragma vengine_package vengine_database

#include <Database/SimpleJsonArray.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

size_t SimpleJsonArray::Length() {

	return arrs.size();
}
JsonVariant SimpleJsonArray::Get(size_t index) {
	return arrs[index].GetVariant();
}
void SimpleJsonArray::LoadFromData(std::span<uint8_t> data) {
	if (!data.empty()) {
		uint64 arrSize = PopValue<uint64>(data);
		arrs.clear();
		arrs.reserve(arrSize);
		arrs.push_back_func(
			[&]() {
				return SimpleJsonLoader::DeSerialize(data, db, this);
			},
			arrSize);
	}
}
void SimpleJsonArray::Set(size_t index, JsonVariant value) {
	if (!value.valid()) {
		VEngine_Log("Invalid Value\n");
		VENGINE_EXIT;
	}
	if (index >= arrs.size()) {
		VEngine_Log("Index Out Range!\n");
		VENGINE_EXIT;
	}

	arrs[index].Set(db, value, static_cast<SimpleJsonObject*>(this));
}
void SimpleJsonArray::Remove(size_t index) {
	if (index >= arrs.size()) {
		VEngine_Log("Index Out Range!\n");
		VENGINE_EXIT;
	}

	arrs.erase(arrs.begin() + index);
}
IJsonSubDatabase* SimpleJsonArray::GetDatabase() { return db; }

void SimpleJsonArray::Add(JsonVariant value) {
	if (!value.valid()) {
		VEngine_Log("Invalid Value\n");
		VENGINE_EXIT;
	}

	arrs.emplace_back(db, value, static_cast<SimpleJsonObject*>(this));
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> SimpleJsonArray::GetIterator() {

	return vstd::linq::ConstIEnumerator(arrs)
		.make_transformer([this](auto&& func) -> JsonVariant const {
			return func.GetVariant();
		})
		.MoveNew();
}
void SimpleJsonArray::M_GetSerData(vstd::vector<uint8_t>& data) {
	auto v = ARRAY_TYPE;
	PushDataToVector<uint8_t&>(v, data);
	PushDataToVector(selfGuid, data);
	auto sizeOffset = data.size();
	data.resize(sizeOffset + sizeof(uint64));
	auto beginOffset = sizeOffset + sizeof(uint64);
	PushDataToVector(arrs.size(), data);
	for (auto&& v : arrs) {
		SimpleJsonLoader::Serialize(db->GetParent(), v, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}
void SimpleJsonArray::Reset() {

	arrs.clear();
}
SimpleJsonArray::SimpleJsonArray(vstd::Guid const& instanceID, SimpleBinaryJson* db)
	: SimpleJsonObject(instanceID, db) {
}
SimpleJsonArray::~SimpleJsonArray() {
}

IJsonValueDict* SimpleJsonArray::AddDict() {

	auto r = db->dictValuePool.New(db, this);
	arrs.emplace_back(r);
	return r;
}

IJsonValueArray* SimpleJsonArray::AddArray() {

	auto r = db->arrValuePool.New(db, this);
	arrs.emplace_back(r);
	return r;
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
	SimpleJsonLoader::Clean(db->GetParent(), arrs);
}

}// namespace toolhub::db