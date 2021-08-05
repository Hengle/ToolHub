#pragma vengine_package vengine_database

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(IJsonDatabase* parent, SimpleJsonVariant const& var) {
	bool res = false;
	var.value.visit(
		[&](auto&& integer) {
			res = true;
		},
		[&](auto&& flt) {
			res = true;
		},
		[&](auto&& str) {
			res = true;
		},
		[&](auto&& obj) {
			auto db = parent->GetDatabase(obj.dbIndex);
			if (db && (db->GetJsonObject(obj.instanceID) != nullptr))
				res = true;
		},
		[&](auto&& obj) {
			auto db = parent->GetDatabase(obj.dbIndex);
			if (db && (db->GetJsonArray(obj.instanceID) != nullptr))
				res = true;
		});
	return res;
}
JsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, IJsonDatabase* parent) {
	ValueType type = PopValue<ValueType>(arr);
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
			return JsonVariant(PopValue<vstd::string>(arr));
		}
		case ValueType::Dict: {
			;

			JsonObjIDBase obj = PopValue<JsonObjIDBase>(arr);
			auto db = parent->GetDatabase(obj.dbIndex);
			auto ite = db->GetJsonObject(obj.instanceID);
			if (ite) {
				return JsonVariant(static_cast<SimpleJsonDict*>(ite));
			}
			return JsonVariant();
		}
		case ValueType::Array: {
			JsonObjIDBase obj = PopValue<JsonObjIDBase>(arr);
			auto db = parent->GetDatabase(obj.dbIndex);
			auto ite = db->GetJsonArray(obj.instanceID);
			if (ite) {
				return JsonVariant(static_cast<SimpleJsonArray*>(ite));
			}
			return JsonVariant();
		}
		default:
			return JsonVariant();
	}
}
void SimpleJsonLoader::Serialize(IJsonDatabase* parent, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data) {
	auto func = [&]<typename TT>(TT&& f) {
		data.push_back(v.value.GetType());
		PushDataToVector(f, data);
	};
	auto addJsonObj = [&](auto&& d, auto&& isInvalidFunc) {
		auto otherDB = parent->GetDatabase(d.dbIndex);
		if (!otherDB || isInvalidFunc(otherDB, d.instanceID)) {
			data.push_back(v.value.argSize);
		} else {
			func.operator()<JsonObjIDBase const&>(d);
		}
	};
	v.value.visit(
		func,
		func,
		[&](vstd::string const& str) {
			PushDataToVector(str, data);
		},
		[&](auto&& d) {
			addJsonObj(d, [&](IJsonSubDatabase* otherDB, uint64 instanceID) {
				return otherDB->GetJsonObject(instanceID) == nullptr;
			});
		},
		[&](auto&& d) {
			addJsonObj(d, [&](IJsonSubDatabase* otherDB, uint64 instanceID) {
				return otherDB->GetJsonArray(instanceID) == nullptr;
			});
		});
}

}// namespace toolhub::db