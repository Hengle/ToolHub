#pragma vengine_package vengine_database

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(IJsonDatabase* parent, SimpleJsonVariant const& var) {
	bool res = false;
	auto setTrue = [&](auto&&) {
		res = true;
	};
	auto setNullCheck = [&](auto&& v) {
		res = (v.get() != nullptr);
	};
	var.value.visit(
		setTrue,
		setTrue,
		setTrue,
		setNullCheck,
		setNullCheck,
		setTrue);
	return res;
}
void SimpleJsonLoader::Clean(IJsonDatabase* db, HashMap<vstd::string, SimpleJsonVariant>& vars) {
	vstd::vector<std::remove_reference_t<decltype(vars)>::NodePair const*> removeIndices;
	for (auto&& i : vars) {
		if (!SimpleJsonLoader::Check(db, i.second)) {
			removeIndices.push_back(&i);
		}
	}
	for (auto&& i : removeIndices) {
		vars.Remove(*i);
	}
}
void SimpleJsonLoader::Clean(IJsonDatabase* db, vstd::vector<SimpleJsonVariant>& arrs) {
	arrs.compact([&](SimpleJsonVariant const& v) {
		return SimpleJsonLoader::Check(db, v);
	});
}
SimpleJsonVariant SimpleJsonLoader::DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* localDB, SimpleJsonObject* obj) {
	auto parent = localDB->GetParent();
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
		case ValueType::ValueDict: {
			auto ptr = localDB->dictValuePool.New(localDB, obj);
			ptr->LoadFromSer(arr);
			return SimpleJsonVariant(ptr);
		}
		case ValueType::ValueArray: {
			auto ptr = localDB->arrValuePool.New(localDB, obj);
			ptr->LoadFromSer(arr);
			return SimpleJsonVariant(ptr);
		}
		case ValueType::GUID: {
			return SimpleJsonVariant(PopValue<vstd::Guid>(arr));
		}
		default:
			return SimpleJsonVariant();
	}
}
void SimpleJsonLoader::Serialize(IJsonDatabase* parent, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data) {
	size_t dataOffset = data.size();
	data.push_back(v.value.GetType());
	auto func = [&]<typename TT>(TT&& f) {
		PushDataToVector(f, data);
	};
	auto addJsonObj = [&](vstd::Guid const& dbIndex, vstd::Guid const& instanceID) {
		auto otherDB = parent->GetDatabase(dbIndex);
		if (otherDB) {
			func(dbIndex);
			func(instanceID);
		} else {
			data[dataOffset] = v.value.argSize;
		}
	};

	auto serValue = [&](auto&& d) {
		d->M_GetSerData(data);
	};
	v.value.visit(
		func,//int64
		func,//double
		func,//string
		serValue,
		serValue,
		func);
}
void SimpleJsonLoader::RemoveAllGuid(vstd::Guid const& guid, SimpleBinaryJson* db) {
	auto ite = db->jsonObjs.Find(guid);
	if (!ite) return;
	auto&& obj = ite.Value();
	if (obj.second == DICT_TYPE) {
		auto ptr = static_cast<SimpleJsonDict*>(obj.first);
		for (auto&& i : ptr->vars) {
			RemoveAllGuid(i.second, db);
		}
		db->Dispose(ptr);

	} else {
		auto ptr = static_cast<SimpleJsonArray*>(obj.first);
		for (auto&& i : ptr->arrs) {
			RemoveAllGuid(i, db);
		}
		db->Dispose(ptr);
	}
}

void SimpleJsonLoader::RemoveAllGuid(SimpleJsonVariant const& v, SimpleBinaryJson* db) {
	auto doNothing = [](auto&&) {};
	v.value.visit(
		doNothing,
		doNothing,
		doNothing,
		[&](vstd::unique_ptr<SimpleJsonValueDict> const& ptr) {
			for (auto&& i : ptr->vars) {
				RemoveAllGuid(i.second, db);
			}
		},
		[&](vstd::unique_ptr<SimpleJsonValueArray> const& ptr) {
			for (auto&& i : ptr->arr) {
				RemoveAllGuid(i, db);
			}
		},
		[&](vstd::Guid const& guid) {
			RemoveAllGuid(guid, db);
		});
}

IJsonRefDict* SimpleJsonLoader::GetDictFromID(IJsonDatabase* db, vstd::Guid const& dbIndex, vstd::Guid const& instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonObject(instanceID);
}
IJsonRefArray* SimpleJsonLoader::GetArrayFromID(IJsonDatabase* db, vstd::Guid const& dbIndex, vstd::Guid const& instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonArray(instanceID);
}
SimpleJsonVariant::SimpleJsonVariant(SimpleBinaryJson* db, JsonVariant const& v, SimpleJsonObject* obj) {
	auto func = [&](auto&& v) {
		value = v;
	};
	v.visit(
		func,
		func,
		func,
		[&](auto&& d) {
			value = db->dictValuePool.New(db, obj, d);
		},
		[&](auto&& d) {
			value = db->arrValuePool.New(db, obj, d);
		},
		func);
}

JsonVariant SimpleJsonVariant::GetVariant() const {
	auto func = [&](auto&& v) -> JsonVariant {
		return v;
	};
	return value.visit(
		func,//int64
		func,//double
		func,//string
		[&](vstd::unique_ptr<SimpleJsonValueDict> const& v) -> JsonVariant {
			return v.get();
		},
		[&](vstd::unique_ptr<SimpleJsonValueArray> const& v) -> JsonVariant {
			return v.get();
		},
		func);
}

}// namespace toolhub::db