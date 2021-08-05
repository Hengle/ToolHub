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
SimpleJsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, IJsonDatabase* parent) {
	ValueType type = PopValue<ValueType>(arr);
	switch (type) {
		case ValueType::Int: {
			int64 v = PopValue<int64>(arr);
			return SimpleJsonVariant(v);
		}
		case ValueType::Float: {
			double d = PopValue<double>(arr);
			return SimpleJsonVariant(d);
		}

		case ValueType::String: {
			return SimpleJsonVariant(PopValue<vstd::string>(arr));
		}
		case ValueType::Dict: {
			JsonObjIDBase obj = PopValue<JsonObjIDBase>(arr);
			auto db = parent->GetDatabase(obj.dbIndex);
			auto ite = db->GetJsonObject(obj.instanceID);
			if (ite) {
				return SimpleJsonVariant(static_cast<SimpleJsonDict*>(ite));
			}
			return SimpleJsonVariant();
		}
		case ValueType::Array: {
			JsonObjIDBase obj = PopValue<JsonObjIDBase>(arr);
			auto db = parent->GetDatabase(obj.dbIndex);
			auto ite = db->GetJsonArray(obj.instanceID);
			if (ite) {
				return SimpleJsonVariant(static_cast<SimpleJsonArray*>(ite));
			}
			return SimpleJsonVariant();
		}
		default:
			return SimpleJsonVariant();
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

IJsonDict* SimpleJsonLoader::GetDictFromID(IJsonDatabase* db, JsonObjID<IJsonDict> const& id) {
	auto subDB = db->GetDatabase(id.dbIndex);
	return subDB->GetJsonObject(id.instanceID);
}
IJsonArray* SimpleJsonLoader::GetArrayFromID(IJsonDatabase* db, JsonObjID<IJsonArray> const& id) {
	auto subDB = db->GetDatabase(id.dbIndex);
	return subDB->GetJsonArray(id.instanceID);
}
SimpleJsonVariant::SimpleJsonVariant(JsonVariant const& v) {
	auto func = [&](auto&& v) {
		value = v;
	};
	v.visit(
		func,
		func,
		func,
		func,
		func);
}

JsonVariant SimpleJsonVariant::GetVariant(IJsonDatabase* db) const {
	auto func = [&](auto&& v) -> JsonVariant {
		return v;
	};
	return value.visit(
		func,
		func,
		func,
		[db](JsonObjID<IJsonDict> const& dict) -> JsonVariant {
			return SimpleJsonLoader::GetDictFromID(db, dict);
		},
		[db](JsonObjID<IJsonArray> const& arr) -> JsonVariant {
			return SimpleJsonLoader::GetArrayFromID(db, arr);
		});
}
SimpleJsonVariant::SimpleJsonVariant(JsonVariant&& v) {
	auto func = [&](auto&& v) {
		value = std::move(v);
	};
	v.visit(
		func,
		func,
		func,
		func,
		func);
}
}// namespace toolhub::db