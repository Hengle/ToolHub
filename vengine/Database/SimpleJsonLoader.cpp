#pragma vengine_package vengine_database

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(IJsonDatabase* parent, SimpleJsonVariant const& var) {
	bool res = false;
	auto setTrue = [&](auto&&) {
		res = true;
	};
	var.value.visit(
		setTrue,
		setTrue,
		setTrue,
		[&](auto&& obj) {
			auto db = parent->GetDatabase(obj.dbIndex);
			if (db && (db->GetJsonObject(obj.instanceID) != nullptr))
				res = true;
		},
		[&](auto&& obj) {
			auto db = parent->GetDatabase(obj.dbIndex);
			if (db && (db->GetJsonArray(obj.instanceID) != nullptr))
				res = true;
		},
		setTrue,
		setTrue);
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
				return SimpleJsonVariant(ite);
			}
			return SimpleJsonVariant();
		}
		case ValueType::Array: {
			JsonObjIDBase obj = PopValue<JsonObjIDBase>(arr);
			auto db = parent->GetDatabase(obj.dbIndex);
			auto ite = db->GetJsonArray(obj.instanceID);
			if (ite) {
				return SimpleJsonVariant(ite);
			}
			return SimpleJsonVariant();
		}
		case ValueType::ValueDict: {
			//TODO: Deser
		}
		case ValueType::ValueArray: {
			//TODO: Deser
		}
		default:
			return SimpleJsonVariant();
	}
}
void SimpleJsonLoader::Serialize(IJsonDatabase* parent, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data) {
	data.push_back(v.value.GetType());
	auto func = [&]<typename TT>(TT&& f) {
		PushDataToVector(f, data);
	};
	auto addJsonObj = [&](auto&& d, auto&& isInvalidFunc) {
		auto otherDB = parent->GetDatabase(d.dbIndex);
		if (otherDB && !isInvalidFunc(otherDB, d.instanceID)) {
			func.operator()<JsonObjIDBase const&>(d);
		}
	};
	Runnable<void(JsonVariant const&)> serJsonVariant;
	auto serValueDict = [&](IJsonValueDict* d) {
		size_t sizeOffset = data.size();
		PushDataToVector<uint64>(0, data);
		auto ite = d->GetIterator();
		uint64 size = 0;
		LINQ_LOOP(i, *ite) {
			size++;
			PushDataToVector(i->key, data);
			serJsonVariant(i->value);
		}
		*(uint64*)(data.data() + sizeOffset) = size;
	};

	auto serValueArray = [&](IJsonValueArray* d) {
		size_t sizeOffset = data.size();
		PushDataToVector<uint64>(0, data);
		auto ite = d->GetIterator();
		uint64 size = 0;
		LINQ_LOOP(i, *ite) {
			size++;
			serJsonVariant(*i);
		}
		*(uint64*)(data.data() + sizeOffset) = size;
	};

	serJsonVariant = [&](JsonVariant const& v) {
		v.visit(
			func,
			func,
			func,
			[&](IJsonRefDict* d) {
				addJsonObj(JsonObjID<IJsonRefDict>(d), [](auto&&, auto&&) { return false; });
			},
			[&](IJsonRefArray* d) {
				addJsonObj(JsonObjID<IJsonRefArray>(d), [](auto&&, auto&&) { return false; });
			},
			serValueDict,
			serValueArray);
	};

	v.value.visit(
		func,
		func,
		func,
		[&](auto&& d) {
			addJsonObj(d, [&](IJsonSubDatabase* otherDB, uint64 instanceID) {
				return otherDB->GetJsonObject(instanceID) == nullptr;
			});
		},
		[&](auto&& d) {
			addJsonObj(d, [&](IJsonSubDatabase* otherDB, uint64 instanceID) {
				return otherDB->GetJsonArray(instanceID) == nullptr;
			});
		},
		[&](vstd::unique_ptr<IJsonValueDict> const& d) {
			serValueDict(d.get());
		},
		[&](vstd::unique_ptr<IJsonValueArray> const& d) {
			serValueArray(d.get());
		});
}

IJsonRefDict* SimpleJsonLoader::GetDictFromID(IJsonDatabase* db, JsonObjID<IJsonRefDict> const& id) {
	auto subDB = db->GetDatabase(id.dbIndex);
	return subDB->GetJsonObject(id.instanceID);
}
IJsonRefArray* SimpleJsonLoader::GetArrayFromID(IJsonDatabase* db, JsonObjID<IJsonRefArray> const& id) {
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
		func,
		[&](auto&& d) {
			value = CopyValueDict(d);
		},
		[&](auto&& d) {
			value = CopyValueArray(d);
		});
}

JsonVariant SimpleJsonVariant::GetVariant(IJsonDatabase* db) const {
	auto func = [&](auto&& v) -> JsonVariant {
		return v;
	};
	return value.visit(
		func,
		func,
		func,
		[db](JsonObjID<IJsonRefDict> const& dict) -> JsonVariant {
			return SimpleJsonLoader::GetDictFromID(db, dict);
		},
		[db](JsonObjID<IJsonRefArray> const& arr) -> JsonVariant {
			return SimpleJsonLoader::GetArrayFromID(db, arr);
		},
		[&](vstd::unique_ptr<IJsonValueDict> const& v) -> JsonVariant {
			return v.get();
		},
		[&](vstd::unique_ptr<IJsonValueArray> const& v) -> JsonVariant {
			return v.get();
		});
}
vstd::unique_ptr<IJsonValueDict> toolhub::db::SimpleJsonVariant::CopyValueDict(IJsonValueDict* valueDict) {
	//TODO
	return vstd::unique_ptr<IJsonValueDict>();
}
vstd::unique_ptr<IJsonValueArray> toolhub::db::SimpleJsonVariant::CopyValueArray(IJsonValueArray* valueDict) {
	//TODO
	return vstd::unique_ptr<IJsonValueArray>();
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
		func,
		[&](auto&& d) {
			value = CopyValueDict(d);
		},
		[&](auto&& d) {
			value = CopyValueArray(d);
		});
}
}// namespace toolhub::db