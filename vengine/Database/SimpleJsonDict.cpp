#pragma vengine_package vengine_compute

#include <Database/SimpleJsonDict.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

void SimpleJsonDict::ExecuteLoad() {
	loader.Load(*this);
}
//Dict Deserialize
void SimpleJsonDict::Load(std::span<uint8_t> sp) {
	uint64 arrSize = PopValue<uint64>(sp);
	vars.Clear();
	vars.reserve(arrSize);
	auto GetNextKeyValue = [&]() {
		uint64 strSize = PopValue<uint64>(sp);
		auto strv = vstd::string_view((char const*)sp.data(), strSize);
		sp = std::span<uint8_t>(sp.data() + strSize, sp.size() - strSize);
		return std::pair<vstd::string, JsonVariant>(strv, SimpleJsonLoader::DeSerialize(sp, db));
	};
	for (auto i : vstd::range(arrSize)) {
		auto kv = GetNextKeyValue();
		vars.Emplace(std::move(kv.first), std::move(kv.second));
	}
}
JsonVariant SimpleJsonDict::Get(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (ite) return ite.Value();
	return JsonVariant();
}
void SimpleJsonDict::Set(vstd::string key, JsonVariant value) {
	isDirty = true;
	vars.ForceEmplace(std::move(key), std::move(value));
}
void SimpleJsonDict::Remove(vstd::string const& key) {
	ExecuteLoad();
	isDirty = true;
	vars.Remove(key);
}
vstd::unique_ptr<vstd::linq::Iterator<JsonKeyPair>> SimpleJsonDict::GetIterator() {
	ExecuteLoad();
	return vstd::linq::IEnumerator(vars)
		.make_transformer(
			[](auto&& kv) {
				return JsonKeyPair{kv.first, kv.second};
			})
		.MoveNew();
}

vstd::optional<int64> SimpleJsonDict::GetInt(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (!ite) return nullptr;
	auto&& v = ite.Value();
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return nullptr;
};
vstd::optional<double> SimpleJsonDict::GetFloat(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (!ite) return nullptr;
	auto&& v = ite.Value();
	switch (v.GetType()) {
		case 0:
			return *reinterpret_cast<int64*>(v.GetPlaceHolder());
		case 1:
			return *reinterpret_cast<double*>(v.GetPlaceHolder());
		case 2:
			return *reinterpret_cast<bool*>(v.GetPlaceHolder()) ? 1 : 0;
	}
	return nullptr;
};
vstd::optional<vstd::string_view> SimpleJsonDict::GetString(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (!ite) return nullptr;
	auto&& v = ite.Value();
	if (v.GetType() == 2) {
		return *reinterpret_cast<vstd::string*>(v.GetPlaceHolder());
	}
	return nullptr;
};
vstd::optional<IJsonDict*> SimpleJsonDict::GetDict(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (!ite) return nullptr;
	auto&& v = ite.Value();
	if (v.GetType() == 3) {
		return *reinterpret_cast<IJsonDict**>(v.GetPlaceHolder());
	}
	return nullptr;
};
vstd::optional<IJsonArray*> SimpleJsonDict::GetArray(vstd::string_view key) {
	ExecuteLoad();
	auto ite = vars.Find(key);
	if (!ite) return nullptr;
	auto&& v = ite.Value();
	if (v.GetType() == 4) {
		return *reinterpret_cast<IJsonArray**>(v.GetPlaceHolder());
	}
	return nullptr;
};

size_t SimpleJsonDict::Length() {
	ExecuteLoad();
	return vars.size();
}

void SimpleJsonDict::M_GetSerData(vstd::vector<uint8_t>& data) {
	ExecuteLoad();
	PushDataToVector(instanceID, data);
	auto sizeOffset = data.size();
	data.resize(sizeOffset + sizeof(uint64));
	auto beginOffset = sizeOffset + sizeof(uint64);
	PushDataToVector(vars.size(), data);
	for (auto&& kv : vars) {
		PushDataToVector(kv.first, data);
		SimpleJsonLoader::Serialize(kv.second, data);
	}
	auto endOffset = data.size();
	*reinterpret_cast<uint64*>(data.data() + sizeOffset) = endOffset - beginOffset;
}

void SimpleJsonDict::DeSer(std::span<uint8_t> data) {
	loader.Reset();
	loader.dataChunk = std::span<uint8_t>(data.data() + sizeof(uint64) * 2, data.size() - sizeof(uint64) * 2);
	ExecuteLoad();
	isDirty = true;
}

}// namespace toolhub::db