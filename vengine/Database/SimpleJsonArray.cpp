#pragma vengine_package vengine_database

#include <Database/SimpleJsonArray.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

size_t SimpleJsonArray::Length() {

	return arrs.size();
}
JsonVariant SimpleJsonArray::Get(size_t index) {
	return arrs[index].GetVariant(db->GetParent());
}
void SimpleJsonArray::LoadFromData(std::span<uint8_t> data) {
	if (!data.empty()) {
		uint64 arrSize = PopValue<uint64>(data);
		arrs.clear();
		arrs.reserve(arrSize);
		arrs.push_back_func(
			[&]() {
				return SimpleJsonLoader::DeSerialize(data, db->GetParent());
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
IJsonSubDatabase* SimpleJsonArray::GetDatabase() { return db; }

void SimpleJsonArray::Add(JsonVariant value) {

	Update();
	arrs.emplace_back(std::move(value));
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> SimpleJsonArray::GetIterator() {

	return vstd::linq::ConstIEnumerator(arrs)
		.make_transformer([this](auto&& func) -> JsonVariant const {
			return func.GetVariant(db->GetParent());
		})
		.MoveNew();
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
		SimpleJsonLoader::Serialize(db->GetParent(), v, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}
void SimpleJsonArray::Reset() {
	Update();
	arrs.clear();
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
	Update();
	arrs.compact([&](SimpleJsonVariant const& v) {
		return SimpleJsonLoader::Check(db->GetParent(), v);
	});
}

}// namespace toolhub::db