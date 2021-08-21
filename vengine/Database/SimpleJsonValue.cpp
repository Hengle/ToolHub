#pragma vengine_package vengine_database

#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
#include <Common/Common.h>
namespace toolhub::db {
SimpleJsonValueDict::SimpleJsonValueDict(SimpleBinaryJson* db) {
	this->db = db;
}
SimpleJsonValueDict::~SimpleJsonValueDict() {
}
SimpleJsonValueDict::SimpleJsonValueDict(
	SimpleBinaryJson* db,
	IJsonDict* src) {
	if (src->IsEmpty()) return;
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
			[&](vstd::unique_ptr<IJsonDict> const& v) {
				vars.Emplace(i.first, db->dictValuePool.New(db, static_cast<SimpleJsonValueDict*>(v.get())));
			},
			[&](vstd::unique_ptr<IJsonArray> const& v) {
				vars.Emplace(i.first, db->arrValuePool.New(db, static_cast<SimpleJsonValueArray*>(v.get())));
			},
			copyDefault);
	}
}
ReadJsonVariant SimpleJsonValueDict::Get(Key const& key) {
	auto ite = vars.Find(key);
	if (ite)
		return ite.Value().GetVariant();
	return ReadJsonVariant();
}
WriteJsonVariant SimpleJsonValueDict::GetAndSet(Key const& key, WriteJsonVariant&& newValue) {
	auto ite = vars.Find(key);
	if (ite) {
		auto result = std::move(ite.Value().value);
		ite.Value().value = std::move(newValue);
		return result;
	} else {
		vars.ForceEmplace(key, std::move(newValue));
		return WriteJsonVariant();
	}
}
WriteJsonVariant SimpleJsonValueDict::GetAndRemove(Key const& key) {
	auto ite = vars.Find(key);
	if (ite) {
		auto result = std::move(ite.Value().value);
		vars.Remove(ite);
		return result;
	} else
		return WriteJsonVariant();
}
void SimpleJsonValueDict::Set(Key const& key, WriteJsonVariant&& value) {
	vars.ForceEmplace(key, std::move(value));
}
void SimpleJsonValueDict::Remove(Key const& key) {
	vars.Remove(SimpleJsonKey(key));
}
vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> SimpleJsonValueDict::GetIterator() {
	return vstd::linq::ConstIEnumerator(vars)
		.make_transformer(
			[](auto&& value) -> const JsonKeyPair {
				return JsonKeyPair(value.first.GetKey(), value.second.GetVariant());
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
		PushDataToVector(kv.first.value, data);
		SimpleJsonLoader::Serialize(kv.second, data);
	}
}

void SimpleJsonValueDict::LoadFromSer(std::span<uint8_t>& sp) {
	auto sz = PopValue<uint64>(sp);
	vars.reserve(sz);
	for (auto i : vstd::range(sz)) {
		auto key = PopValue<SimpleJsonKey::ValueType>(sp);
		vars.Emplace(std::move(key), SimpleJsonLoader::DeSerialize(sp, db));
	}
}

void SimpleJsonValueDict::Reset() {
	vars.Clear();
}

void SimpleJsonValueDict::Dispose() {
	db->dictValuePool.Delete(this);
}
void SimpleJsonValueArray::Dispose() {
	db->arrValuePool.Delete(this);
}

SimpleJsonValueArray::SimpleJsonValueArray(
	SimpleBinaryJson* db) {
	this->db = db;
}
SimpleJsonValueArray::~SimpleJsonValueArray() {
}

SimpleJsonValueArray::SimpleJsonValueArray(
	SimpleBinaryJson* db,
	IJsonArray* src) {
	this->db = db;
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
			[&](vstd::unique_ptr<IJsonDict> const& v) {
				arr.emplace_back(db->dictValuePool.New(db, static_cast<SimpleJsonValueDict*>(v.get())));
			},
			[&](vstd::unique_ptr<IJsonArray> const& v) {
				arr.emplace_back(db->arrValuePool.New(db, static_cast<SimpleJsonValueArray*>(v.get())));
			},
			copyDefault);
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
		SimpleJsonLoader::Serialize(v, data);
	}
}

void SimpleJsonValueArray::LoadFromSer(std::span<uint8_t>& sp) {
	auto sz = PopValue<uint64>(sp);
	arr.reserve(sz);
	for (auto i : vstd::range(sz)) {
		arr.emplace_back(SimpleJsonLoader::DeSerialize(sp, db));
	}
}

void SimpleJsonValueArray::Reset() {
	arr.clear();
}

ReadJsonVariant SimpleJsonValueArray::Get(size_t index) {
	if (index >= arr.size())
		return ReadJsonVariant();
	return arr[index].GetVariant();
}

void SimpleJsonValueArray::Set(size_t index, WriteJsonVariant&& value) {
	if (index < arr.size()) {
		arr[index].Set(std::move(value));
	}
}

void SimpleJsonValueArray::Remove(size_t index) {
	if (index < arr.size()) {
		arr.erase(arr.begin() + index);
	}
}

void SimpleJsonValueArray::Add(WriteJsonVariant&& value) {

	arr.emplace_back(std::move(value));
}

vstd::unique_ptr<vstd::linq::Iterator<const ReadJsonVariant>> SimpleJsonValueArray::GetIterator() {
	return vstd::linq::ConstIEnumerator(arr)
		.make_transformer(
			[](auto&& var) -> ReadJsonVariant const {
				return var.GetVariant();
			})
		.MoveNew();
}
}// namespace toolhub::db