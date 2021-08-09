#pragma vengine_package vengine_database

#include <Database/SimpleJsonDict.h>
#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
namespace toolhub::db {

//Dict Deserialize

JsonVariant SimpleJsonDict::Get(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (ite) return ite.Value().GetVariant();
	return JsonVariant();
}
void SimpleJsonDict::Set(vstd::string key, JsonVariant value) {
	Update();
	vars.ForceEmplace(std::move(key), db, value, static_cast<SimpleJsonObject*>(this));
}
void SimpleJsonDict::LoadFromData(std::span<uint8_t> data) {
	if (!data.empty()) {
		uint64 arrSize = PopValue<uint64>(data);
		vars.Clear();
		vars.reserve(arrSize);
		auto GetNextKeyValue = [&]() {
			auto str = PopValue<vstd::string>(data);
			return std::pair<vstd::string, SimpleJsonVariant>(std::move(str), SimpleJsonLoader::DeSerialize(data, db, this));
		};
		for (auto i : vstd::range(arrSize)) {
			auto kv = GetNextKeyValue();
			vars.Emplace(std::move(kv.first), std::move(kv.second));
		}
	}
}
void SimpleJsonDict::Remove(vstd::string const& key) {
	Update();
	vars.Remove(key);
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> SimpleJsonDict::GetIterator() {

	return vstd::linq::ConstIEnumerator(vars)
		.make_transformer(
			[this](auto&& kv) -> const JsonKeyPair {
				return JsonKeyPair{kv.first, kv.second.GetVariant()};
			})
		.MoveNew();
}

size_t SimpleJsonDict::Length() {
	return vars.size();
}

void SimpleJsonDict::M_GetSerData(vstd::vector<uint8_t>& data) {
	auto v = DICT_TYPE;
	PushDataToVector<uint8_t&>(v, data);
	PushDataToVector(instanceID, data);
	auto sizeOffset = data.size();
	data.resize(sizeOffset + sizeof(uint64));
	auto beginOffset = sizeOffset + sizeof(uint64);
	PushDataToVector(vars.size(), data);
	for (auto&& kv : vars) {
		PushDataToVector(kv.first, data);
		SimpleJsonLoader::Serialize(db->GetParent(), kv.second, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}
void SimpleJsonDict::Clean() {
	SimpleJsonLoader::Clean(db->GetParent(), vars);
}
void SimpleJsonDict::Reset() {
	vars.Clear();
}
SimpleJsonDict::SimpleJsonDict(uint64 instanceID, SimpleBinaryJson* db)
	: SimpleJsonObject(instanceID, db) {
	//TODO: deser data
}
IJsonSubDatabase* SimpleJsonDict::GetDatabase() { return db; }
SimpleJsonDict::~SimpleJsonDict() {
}
IJsonValueDict* SimpleJsonDict::AddOrGetDict(vstd::string key) {
	auto ite = vars.Find(key);
	if (ite) {
		auto ptr = ite.Value().value.try_get<vstd::unique_ptr<SimpleJsonValueDict>>();
		if (ptr) return ptr->get();
	}
	Update();
	auto result = db->dictValuePool.New(db, this);
	ite = vars.ForceEmplace(std::move(key), result);
	return result;
}
IJsonValueArray* SimpleJsonDict::AddOrGetArray(vstd::string key) {
	auto ite = vars.Find(key);
	if (ite) {
		auto ptr = ite.Value().value.try_get<vstd::unique_ptr<SimpleJsonValueArray>>();
		if (ptr) return ptr->get();
	}
	Update();
	auto result = db->arrValuePool.New(db, this);
	ite = vars.ForceEmplace(std::move(key), result);
	return result;
}
void SimpleJsonDict::Dispose() {
	db->Dispose(this);
}
void SimpleJsonDict::AfterAdd(IDatabaseEvtVisitor* visitor) {
	visitor->AddDict(this);
}
void SimpleJsonDict::BeforeRemove(IDatabaseEvtVisitor* visitor) {
	visitor->RemoveDict(this);
}
}// namespace toolhub::db