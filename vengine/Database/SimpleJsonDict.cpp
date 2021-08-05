#pragma vengine_package vengine_database

#include <Database/SimpleJsonDict.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

//Dict Deserialize

JsonVariant SimpleJsonDict::Get(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (ite) return ite.Value().GetVariant(db->GetParent());
	return JsonVariant();
}
void SimpleJsonDict::Set(vstd::string key, JsonVariant value) {
	Update();
	vars.ForceEmplace(std::move(key), std::move(value));
}
void SimpleJsonDict::LoadFromData(std::span<uint8_t> data) {
	if (!data.empty()) {
		uint64 arrSize = PopValue<uint64>(data);
		vars.Clear();
		vars.reserve(arrSize);
		auto GetNextKeyValue = [&]() {
			auto str = PopValue<vstd::string>(data);

			return std::pair<vstd::string, SimpleJsonVariant>(std::move(str), SimpleJsonLoader::DeSerialize(data, db->GetParent()));
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
				return JsonKeyPair{kv.first, kv.second.GetVariant(db->GetParent())};
			})
		.MoveNew();
}

vstd::optional<int64> SimpleJsonDict::GetInt(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (!ite) return vstd::optional<int64>();
	auto&& v = ite.Value().value;
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return vstd::optional<int64>();
};
vstd::optional<double> SimpleJsonDict::GetFloat(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (!ite) return vstd::optional<double>();
	auto&& v = ite.Value().value;
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return vstd::optional<double>();
};
vstd::optional<vstd::string_view> SimpleJsonDict::GetString(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (!ite) return vstd::optional<vstd::string_view>();
	auto&& v = ite.Value().value;
	if (v.GetType() == 2) {
		return *reinterpret_cast<vstd::string*>(v.GetPlaceHolder());
	}
	return vstd::optional<vstd::string_view>();
};
vstd::optional<IJsonDict*> SimpleJsonDict::GetDict(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (!ite) return vstd::optional<IJsonDict*>();
	auto&& v = ite.Value().value;
	if (v.GetType() == 3) {
		auto&& id = *reinterpret_cast<JsonObjID<IJsonDict>*>(v.GetPlaceHolder());
		auto localDB = db->GetParent()->GetDatabase(id.dbIndex);
		auto ptr = localDB->GetJsonObject(id.instanceID);
		if (ptr)
			return vstd::optional<IJsonDict*>(ptr);
	}
	return vstd::optional<IJsonDict*>();
};
vstd::optional<IJsonArray*> SimpleJsonDict::GetArray(vstd::string_view key) {

	auto ite = vars.Find(key);
	if (!ite) return vstd::optional<IJsonArray*>();
	auto&& v = ite.Value().value;
	if (v.GetType() == 4) {
		auto&& id = *reinterpret_cast<JsonObjID<IJsonArray>*>(v.GetPlaceHolder());
		auto localDB = db->GetParent()->GetDatabase(id.dbIndex);
		auto ptr = localDB->GetJsonArray(id.instanceID);
		if (ptr)
			return vstd::optional<IJsonArray*>(ptr);
	}
	return vstd::optional<IJsonArray*>();
};

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
	vstd::vector<vstd::string const*> removeIndices;
	for (auto&& i : vars) {
		if (!SimpleJsonLoader::Check(db->GetParent(), i.second)) {
			removeIndices.push_back(&i.first);
		}
	}
	for (auto&& i : removeIndices) {
		vars.Remove(*i);
	}
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