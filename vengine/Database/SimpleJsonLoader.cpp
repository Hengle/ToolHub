#pragma vengine_package vengine_compute

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(SimpleBinaryJson* db, JsonVariant const& var) {
	bool res;
	auto check = [&](uint64 dict) {
		res = db->jsonObjs.Find(dict);
	};
	var.visit(
		[&](auto&& integer) {
			res = true;
		},
		[&](auto&& flt) {
			res = true;
		},
		[&](auto&& str) {
			res = true;
		},
		check,
		check);
	return res;
}
JsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db) {
	ValueType type = PopValue<ValueType>(arr);
	auto ReadDict = [&](uint8_t typ, auto&& func) -> JsonVariant {
		uint64 instanceID = PopValue<uint64>(arr);
		auto ite = db->jsonObjs.Find(instanceID);
		if (ite && ite.Value().second == typ) {
			return func(ite.Value().first);
		}
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
			return ReadDict(DICT_TYPE, [](SimpleJsonObject* obj) {
				return JsonVariant(static_cast<SimpleJsonDict*>(obj));
			});
		}
		case ValueType::Array: {
			return ReadDict(ARRAY_TYPE, [](SimpleJsonObject* obj) {
				return JsonVariant(static_cast<SimpleJsonArray*>(obj));
			});
		}
		default:
			return JsonVariant();
	}
}
void SimpleJsonLoader::Serialize(SimpleBinaryJson* db, JsonVariant const& v, vstd::vector<uint8_t>& data) {
	auto func = [&]<typename TT>(TT&& f) {
		data.push_back(v.GetType());
		using T = std::remove_cvref_t<TT>;
		auto lastLen = data.size();
		data.resize(lastLen + sizeof(T));
		*reinterpret_cast<T*>(data.data() + lastLen) = std::forward<TT>(f);
	};
	auto checkFunc = [&](uint64 d) {
		if (!db->jsonObjs.Find(d)) {
			data.push_back(v.argSize);
		} else {
			func(d);
		}
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
		checkFunc,
		checkFunc);
}

}// namespace toolhub::db