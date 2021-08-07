#pragma vengine_package vengine_database

#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
bool SimpleJsonLoader::Check(IJsonDatabase* parent, SimpleJsonVariant const& var) {
	bool res = false;
	auto setTrue = [&](auto&&) {
		res = true;
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
		auto serObj = [&](auto&& d) {
			addJsonObj(d->GetDatabase()->GetIndex(), d->GetInstanceID());
		};
		v.visit(
			func,		   //int64
			func,		   //double
			func,		   //string_view
			serObj,		   //IJsonRefDict*
			serObj,		   //IJsonRefArray*
			serValueDict,  //IJsonValueDict*
			serValueArray);//IJsonValueArray*
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
	v.value.visit(
		func,		  //int64
		func,		  //double
		func,		  //string
		serObjTracker,//vstd::ObjectTrackFlag<IJsonRefDict>
		serObjTracker,// vstd::ObjectTrackFlag<IJsonRefArray>
		[&](vstd::unique_ptr<IJsonValueDict> const& d) {
			serValueDict(d.get());
		},
		[&](vstd::unique_ptr<IJsonValueArray> const& d) {
			serValueArray(d.get());
		});
}

IJsonRefDict* SimpleJsonLoader::GetDictFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonObject(instanceID);
}
IJsonRefArray* SimpleJsonLoader::GetArrayFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID) {
	auto subDB = db->GetDatabase(dbIndex);
	return subDB->GetJsonArray(instanceID);
}
SimpleJsonVariant::SimpleJsonVariant(JsonVariant const& v) {
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
	auto getTracker = [&](auto const& flag) {
		return flag.Get();
	};
	return value.visit(
		func,	   //int64
		func,	   //double
		func,	   //string
		getTracker,//vstd::ObjectTrackFlag<IJsonRefDict>
		getTracker,// vstd::ObjectTrackFlag<IJsonRefArray>
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
			value = CopyValueDict(d);
		},
		[&](auto&& d) {
			value = CopyValueArray(d);
		});
}
}// namespace toolhub::db