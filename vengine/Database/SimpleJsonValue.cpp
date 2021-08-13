#pragma vengine_package vengine_database

#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
#include <Common/Common.h>
namespace toolhub::db {
SimpleJsonValueDict::SimpleJsonValueDict(
	SimpleBinaryJson* db,
	SimpleJsonObject* parent)
	: SimpleJsonValueBase(db, parent) {
}
SimpleJsonValueDict::~SimpleJsonValueDict() {
}
void SimpleJsonValueBase::Update() {
	parent->Update();
}
SimpleJsonValueDict::SimpleJsonValueDict(
	SimpleBinaryJson* db,
	SimpleJsonObject* parent,
	IJsonValueDict* src)
	: SimpleJsonValueBase(db, parent) {
	if (typeid(*src) == typeid(SimpleJsonValueDict)) {
		SimpleJsonValueDict* srcDict = static_cast<SimpleJsonValueDict*>(src);
		vars.reserve(srcDict->vars.size());
		for (auto&& i : srcDict->vars) {
			auto copyDefault = [&](auto&& v) {
				vars.Emplace(i.first, v);
			};
			i.second.value.visit(
				copyDefault,
				copyDefault,
				copyDefault,
				copyDefault,
				copyDefault,
				[&](vstd::unique_ptr<SimpleJsonValueDict> const& v) {
					vars.Emplace(i.first, db->dictValuePool.New(db, parent, v.get()));
				},
				[&](vstd::unique_ptr<SimpleJsonValueArray> const& v) {
					vars.Emplace(i.first, db->arrValuePool.New(db, parent, v.get()));
				},
				copyDefault);
		}
	} else {
		auto iterator = src->GetIterator();
		LINQ_LOOP(i, *iterator) {
			vars.Emplace(i->key, db, i->value, parent);
		}
	}
}
JsonVariant SimpleJsonValueDict::Get(vstd::string_view key) {
	auto ite = vars.Find(key);
	if (ite)
		return ite.Value().GetVariant();
	return JsonVariant();
}
void SimpleJsonValueDict::Set(vstd::string key, JsonVariant value) {
	Update();
	vars.ForceEmplace(std::move(key), db, value, parent);
}
void SimpleJsonValueDict::Remove(vstd::string const& key) {
	Update();
	vars.Remove(key);
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> SimpleJsonValueDict::GetIterator() {
	return vstd::linq::ConstIEnumerator(vars)
		.make_transformer(
			[](auto&& value) -> const JsonKeyPair {
				return JsonKeyPair{value.first, value.second.GetVariant()};
			})
		.MoveNew();
}
size_t SimpleJsonValueDict::Length() {
	return vars.size();
}
vstd::vector<uint8_t> SimpleJsonValueDict::GetSerData() {
	vstd::vector<uint8_t> result;
	M_GetSerData(result);
	return result;
}
void SimpleJsonValueDict::M_GetSerData(vstd::vector<uint8_t>& data) {
	PushDataToVector<uint64>(vars.size(), data);
	for (auto&& kv : vars) {
		PushDataToVector(kv.first, data);
		SimpleJsonLoader::Serialize(db->GetParent(), kv.second, data);
	}
}

void SimpleJsonValueDict::LoadFromSer(std::span<uint8_t>& sp) {
	auto sz = PopValue<uint64>(sp);
	vars.reserve(sz);
	for (auto i : vstd::range(sz)) {
		auto key = PopValue<vstd::string>(sp);
		vars.Emplace(std::move(key), SimpleJsonLoader::DeSerialize(sp, db, parent));
	}
}

void SimpleJsonValueDict::Clean() {
	SimpleJsonLoader::Clean(db->GetParent(), vars);
}
void SimpleJsonValueDict::Reset() {
	Update();
	vars.Clear();
}

void SimpleJsonValueDict::Dispose() {
	if (db->Enabled())
		db->dictValuePool.Delete(this);
}
IJsonValueDict* SimpleJsonValueDict::AddOrGetDict(vstd::string key) {
	auto ite = vars.Find(key);
	if (ite) {
		auto ptr = ite.Value().value.try_get<vstd::unique_ptr<SimpleJsonValueDict>>();
		if (ptr) return ptr->get();
	}
	Update();
	auto result = db->dictValuePool.New(db, parent);
	ite = vars.ForceEmplace(std::move(key), result);
	return result;
}
IJsonValueArray* SimpleJsonValueDict::AddOrGetArray(vstd::string key) {
	auto ite = vars.Find(key);
	if (ite) {
		auto ptr = ite.Value().value.try_get<vstd::unique_ptr<SimpleJsonValueArray>>();
		if (ptr) return ptr->get();
	}
	Update();
	auto result = db->arrValuePool.New(db, parent);
	ite = vars.ForceEmplace(std::move(key), result);
	return result;
}

void SimpleJsonValueArray::Dispose() {
	if (db->Enabled())
		db->arrValuePool.Delete(this);
}

IJsonValueDict* SimpleJsonValueArray::AddDict() {
	Update();
	auto r = db->dictValuePool.New(db, parent);
	arr.emplace_back(r);
	return r;
}

IJsonValueArray* SimpleJsonValueArray::AddArray() {
	Update();
	auto r = db->arrValuePool.New(db, parent);
	arr.emplace_back(r);
	return r;
}

SimpleJsonValueArray::SimpleJsonValueArray(
	SimpleBinaryJson* db,
	SimpleJsonObject* parent)
	: SimpleJsonValueBase(db, parent) {
}
SimpleJsonValueArray::~SimpleJsonValueArray() {
}

SimpleJsonValueArray::SimpleJsonValueArray(
	SimpleBinaryJson* db,
	SimpleJsonObject* parent, IJsonValueArray* src)
	: SimpleJsonValueBase(db, parent) {
	if (typeid(*src) == typeid(SimpleJsonValueArray)) {
		SimpleJsonValueArray* srcArr = static_cast<SimpleJsonValueArray*>(src);
		arr.reserve(srcArr->arr.size());
		auto copyDefault = [&](auto&& v) {
			arr.emplace_back(v);
		};
		for (auto&& i : srcArr->arr) {
			i.value.visit(
				copyDefault,
				copyDefault,
				copyDefault,
				copyDefault,
				copyDefault,
				[&](vstd::unique_ptr<SimpleJsonValueDict> const& v) {
					arr.emplace_back(db->dictValuePool.New(db, parent, v.get()));
				},
				[&](vstd::unique_ptr<SimpleJsonValueArray> const& v) {
					arr.emplace_back(db->arrValuePool.New(db, parent, v.get()));
				},
				copyDefault);
		}
	} else {
		auto ite = src->GetIterator();
		LINQ_LOOP(i, *ite) {
			arr.emplace_back(db, *i, parent);
		}
	}
}

size_t SimpleJsonValueArray::Length() {
	return arr.size();
}

vstd::vector<uint8_t> SimpleJsonValueArray::GetSerData() {
	vstd::vector<uint8_t> result;
	M_GetSerData(result);
	return result;
}
void SimpleJsonValueArray::M_GetSerData(vstd::vector<uint8_t>& data) {
	PushDataToVector<uint64>(arr.size(), data);
	for (auto&& v : arr) {
		SimpleJsonLoader::Serialize(db->GetParent(), v, data);
	}
}

void SimpleJsonValueArray::LoadFromSer(std::span<uint8_t>& sp) {
	auto sz = PopValue<uint64>(sp);
	arr.reserve(sz);
	for (auto i : vstd::range(sz)) {
		arr.emplace_back(SimpleJsonLoader::DeSerialize(sp, db, parent));
	}
}

void SimpleJsonValueArray::Clean() {
	SimpleJsonLoader::Clean(db->GetParent(), arr);
}

void SimpleJsonValueArray::Reset() {
	Update();
	arr.clear();
}

JsonVariant SimpleJsonValueArray::Get(size_t index) {
	if (index >= arr.size())
		return JsonVariant();
	return arr[index].GetVariant();
}

void SimpleJsonValueArray::Set(size_t index, JsonVariant value) {
	if (index < arr.size()) {
		Update();
		arr[index].Set(db, value, parent);
	}
}

void SimpleJsonValueArray::Remove(size_t index) {
	if (index < arr.size()) {
		Update();
		arr.erase(arr.begin() + index);
	}
}

void SimpleJsonValueArray::Add(JsonVariant value) {
	Update();
	arr.emplace_back(db, value, parent);
}

vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> SimpleJsonValueArray::GetIterator() {
	return vstd::linq::ConstIEnumerator(arr)
		.make_transformer(
			[](auto&& var) -> JsonVariant const {
				return var.GetVariant();
			})
		.MoveNew();
}
}// namespace toolhub::db