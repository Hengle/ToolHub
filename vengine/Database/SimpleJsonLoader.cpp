#pragma vengine_package vengine_compute

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
SimpleJsonLoader::SimpleJsonLoader()
	: loaded(false) {
}
void SimpleJsonLoader::Reset() {
	loaded = false;
}
JsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db) {
	ValueType type = PopValue<ValueType>(arr);
	auto ReadDict = [&](auto&& map) {
		uint64 instanceID = PopValue<uint64>(arr);
		auto ite = map.Find(instanceID);
		if (ite) return JsonVariant(ite.Value());
		return JsonVariant();
	};
	switch (type) {
		case ValueType::Int: {
			int64 v = PopValue<int64>(arr);
			return JsonVariant(v);
		}
		case ValueType::Float: {
			double d = PopValue<double>(arr);
			return JsonVariant(d);
		}

		case ValueType::String: {
			uint64 strSize = PopValue<uint64>(arr);
			auto strv = vstd::string_view((char const*)arr.data(), strSize);
			arr = std::span<uint8_t>(arr.data() + strSize, arr.size() - strSize);
			return JsonVariant(strv);
		}
		case ValueType::Dict: {
			return ReadDict(db->dictObj.map);
		}
		case ValueType::Array: {
			return ReadDict(db->arrObj.map);
		}
		default:
			return JsonVariant();
	}
}
void SimpleJsonLoader::Serialize(JsonVariant const& v, vstd::vector<uint8_t>& data) {
	data.push_back(v.GetType());
	auto func = [&]<typename TT>(TT&& f) {
		using T = std::remove_cvref_t<TT>;
		auto lastLen = data.size();
		data.resize(lastLen + sizeof(T));
		*reinterpret_cast<T*>(data.data() + lastLen) = std::forward<TT>(f);
	};
	auto getInstance = [&](auto&& f) {
		func(f->instanceID);
	};
	v.visit(
		func,
		func,
		[&](vstd::string const& str) {
			func(str.size());
			auto lastLen = data.size();
			data.resize(lastLen + str.size());
			memcpy(data.data() + lastLen, str.data(), str.size());
		},
		[&](IJsonDict* d) {
			getInstance(static_cast<SimpleJsonDict*>(d));
		},
		[&](IJsonArray* d) {
			getInstance(static_cast<SimpleJsonArray*>(d));
		});
}

}// namespace toolhub::db