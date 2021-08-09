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
	auto setTrack = [&](auto&& obj) {
		res == (obj.Get() != nullptr);
	};
	var.value.visit(
		setTrue,
		setTrue,
		setTrue,
		setTrack,
		setTrack,
		setNullCheck,
		setNullCheck);
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
		case ValueType::Dict: {
			uint64 dbIndex, instanceID;
			dbIndex = PopValue<uint64>(arr);
			instanceID = PopValue<uint64>(arr);
			auto db = parent->GetDatabase(dbIndex);
			auto ite = db->GetJsonObject(instanceID);
			if (ite) {
				return SimpleJsonVariant(ite->GetTrackFlag());
			}
			return SimpleJsonVariant();
		}
		case ValueType::Array: {
			uint64 dbIndex, instanceID;
			dbIndex = PopValue<uint64>(arr);
			instanceID = PopValue<uint64>(arr);
			auto db = parent->GetDatabase(dbIndex);
			auto ite = db->GetJsonArray(instanceID);
			if (ite) {
				return SimpleJsonVariant(ite->GetTrackFlag());
			}
			return SimpleJsonVariant();
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
	auto addJsonObj = [&](uint64 dbIndex, uint64 instanceID) {
		auto otherDB = parent->GetDatabase(dbIndex);
		if (otherDB) {
			func(dbIndex);
			func(instanceID);
		} else {
			data[dataOffset] = v.value.argSize;
		}
	};

	// Call SimpleVariant
	auto serObjTracker = [&](auto&& d) {
		auto ptr = d.Get();
		if (ptr) {
			addJsonObj(ptr->GetDatabase()->GetIndex(), ptr->GetInstanceID());
		} else {
			data[dataOffset] = v.value.argSize;
		}
	};
	auto serValue = [&](auto&& d) {
		d->M_GetSerData(data);
	};
	v.value.visit(
		func,		  //int64
		func,		  //double
		func,		  //string
		serObjTracker,//vstd::ObjectTrackFlag<IJsonRefDict>
		serObjTracker,// vstd::ObjectTrackFlag<IJsonRefArray>
		serValue,
		serValue);
}

IJsonRefDict* SimpleJsonLoader::GetDictFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonObject(instanceID);
}
IJsonRefArray* SimpleJsonLoader::GetArrayFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonArray(instanceID);
}
SimpleJsonVariant::SimpleJsonVariant(SimpleBinaryJson* db, JsonVariant const& v, SimpleJsonObject* obj) {
	auto func = [&](auto&& v) {
		value = v;
	};
	auto getSer = [&](auto&& d) {
		value = d->GetTrackFlag();
	};
	v.visit(
		func,
		func,
		func,
		getSer,
		getSer,
		[&](auto&& d) {
			value = db->dictValuePool.New(db, obj, d);
		},
		[&](auto&& d) {
			value = db->arrValuePool.New(db, obj, d);
		});
}

JsonVariant SimpleJsonVariant::GetVariant() const {
	auto func = [&](auto&& v) -> JsonVariant {
		return v;
	};
	auto getTracker = [&](auto const& flag) {
		return flag.Get();
	};
	return value.visit(
		func,	   //int64
		func,	   //double
		func,	   //string
		getTracker,//vstd::ObjectTrackFlag<IJsonRefDict>
		getTracker,// vstd::ObjectTrackFlag<IJsonRefArray>
		[&](vstd::unique_ptr<SimpleJsonValueDict> const& v) -> JsonVariant {
			return v.get();
		},
		[&](vstd::unique_ptr<SimpleJsonValueArray> const& v) -> JsonVariant {
			return v.get();
		});
}

}// namespace toolhub::db