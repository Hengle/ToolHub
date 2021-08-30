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
	if (!key.valid()) return ReadJsonVariant();
	auto ite = vars.Find(key);
	if (ite)
		return ite.Value().GetVariant();
	return ReadJsonVariant();
}
WriteJsonVariant SimpleJsonValueDict::GetAndSet(Key const& key, WriteJsonVariant&& newValue) {
	if (!key.valid()) return WriteJsonVariant();
	auto ite = vars.Find(key);
	if (ite) {
		auto result = std::move(ite.Value().value);
		if (newValue.valid()) {
			ite.Value().value = std::move(newValue);
		} else {
			vars.Remove(ite);
		}
		return result;
	} else {
		if (newValue.valid()) {
			vars.ForceEmplace(key, std::move(newValue));
		}
		return WriteJsonVariant();
	}
}
WriteJsonVariant SimpleJsonValueDict::GetAndRemove(Key const& key) {
	if (!key.valid()) return WriteJsonVariant();
	auto ite = vars.Find(key);
	if (ite) {
		auto result = std::move(ite.Value().value);
		vars.Remove(ite);
		return result;
	} else
		return WriteJsonVariant();
}
void SimpleJsonValueDict::Set(Key const& key, WriteJsonVariant&& value) {
	if (key.valid() && value.valid())
		vars.ForceEmplace(key, std::move(value));
}
void SimpleJsonValueDict::Remove(Key const& key) {
	if (key.valid())
		vars.TRemove(key);
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

void SimpleJsonValueDict::LoadFromSer(std::span<uint8_t const>& sp) {
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

void SimpleJsonValueArray::LoadFromSer(std::span<uint8_t const>& sp) {
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
		if (value.valid())
			arr[index].Set(std::move(value));
		else
			arr[index].value.dispose();
	}
}

void SimpleJsonValueArray::Remove(size_t index) {
	if (index < arr.size()) {
		arr.erase(arr.begin() + index);
	}
}

void SimpleJsonValueArray::Add(WriteJsonVariant&& value) {
	if (value.valid())
		arr.emplace_back(std::move(value));
}

WriteJsonVariant SimpleJsonValueArray::GetAndSet(size_t index, WriteJsonVariant&& newValue) {
	if (index >= arr.size())
		return WriteJsonVariant();
	WriteJsonVariant result = std::move(arr[index].value);
	if (newValue.valid())
		arr[index] = std::move(newValue);
	else
		arr[index].value.dispose();
	return result;
}
WriteJsonVariant SimpleJsonValueArray::GetAndRemove(size_t index) {
	if (index >= arr.size())
		return WriteJsonVariant();
	WriteJsonVariant result = std::move(arr[index].value);
	arr.erase(arr.begin() + index);
	return result;
}

vstd::unique_ptr<vstd::linq::Iterator<const ReadJsonVariant>> SimpleJsonValueArray::GetIterator() {
	return vstd::linq::ConstIEnumerator(arr)
		.make_transformer(
			[](auto&& var) -> ReadJsonVariant const {
				return var.GetVariant();
			})
		.MoveNew();
}
static void PrintSimpleJsonVariant(SimpleJsonVariant const& v, vstd::string& str, size_t layer, size_t valueLayer, bool emptySpaceBeforeOb) {
	auto func = [&](auto&& v) {
		str.push_back_all(' ', valueLayer);
		vstd::to_string(v, str);
	};
	v.value.visit(
		func,
		func,
		[&](vstd::string const& s) {
			str.push_back_all(' ', valueLayer);
			str << '\"'
				<< s
				<< '\"';
		},
		[&](vstd::unique_ptr<IJsonDict> const& ptr) {
			if (emptySpaceBeforeOb)
				str << '\n';
			static_cast<SimpleJsonValueDict*>(ptr.get())->M_Print(str, layer);
		},
		[&](vstd::unique_ptr<IJsonArray> const& ptr) {
			if (emptySpaceBeforeOb)
				str << '\n';
			static_cast<SimpleJsonValueArray*>(ptr.get())->M_Print(str, layer);
		},
		[&](vstd::Guid const& guid) {
			str.push_back_all(' ', valueLayer);
			size_t offst = str.size();
			str.resize(offst + 32);
			guid.ToString(str.data() + offst, false);
		});
}
static void PrintKeyVariant(SimpleJsonKey const& v, vstd::string& str) {
	auto func = [&](auto&& v) {
		vstd::to_string(v, str);
	};
	v.value.visit(
		func,
		[&](vstd::string const& s) {
			str << '\"'
				<< s
				<< '\"';
		},
		[&](vstd::Guid const& guid) {
			size_t offst = str.size();
			str.resize(offst + 32);
			guid.ToString(str.data() + offst, false);
		});
}
void SimpleJsonValueDict::M_Print(vstd::string& str, size_t space) {
	str.push_back_all(' ', space);
	str << "{\n"_sv;
	space += 2;
	auto disp = vstd::create_disposer([&]() {
		space -= 2;
		str.push_back_all(' ', space);
		str << '}';
	});
	size_t varsSize = vars.size() - 1;
	size_t index = 0;
	for (auto&& i : vars) {
		str.push_back_all(' ', space);
		PrintKeyVariant(i.first, str);
		str << " : "_sv;
		PrintSimpleJsonVariant(i.second, str, space, 0, true);
		if (index == varsSize) {
			str << '\n';
		} else {
			str << ",\n"_sv;
		}
		index++;
	}
}
void SimpleJsonValueArray::M_Print(vstd::string& str, size_t space) {
	str.push_back_all(' ', space);
	str << "[\n"_sv;
	space += 2;
	auto disp = vstd::create_disposer([&]() {
		space -= 2;
		str.push_back_all(' ', space);
		str << ']';
	});
	size_t arrSize = arr.size() - 1;
	size_t index = 0;
	for (auto&& i : arr) {
		PrintSimpleJsonVariant(i, str, space, space, false);
		if (index == arrSize) {
			str << '\n';
		} else {
			str << ",\n"_sv;
		}
		index++;
	}
}
}// namespace toolhub::db