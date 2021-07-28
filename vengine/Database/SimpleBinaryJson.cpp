#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/Pool.h>
#include <Database/SimpleBinaryJson.h>
#include <boost/asio.hpp>
namespace toolhub::db {

class SimpleBinaryJson;
void SimpleBinaryJson::DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& v) {
	if (v.second == ARRAY_TYPE) {
		arrPool.Delete(static_cast<SimpleJsonArray*>(v.first));
	} else {
		dictPool.Delete(static_cast<SimpleJsonDict*>(v.first));
	}
}
bool SimpleBinaryJson::DisposeProperty(ObjMap::Index data, SimpleJsonObject* obj) {
	if (!data || data.Value().first != obj) {
		return false;
	}
	DisposeProperty(data.Value());
	jsonObjs.Remove(data);
	return true;
}

void SimpleBinaryJson::Dispose(uint64 instanceID) {
	auto ite = jsonObjs.Find(instanceID);
	if (ite) {
		DisposeProperty(ite.Value());
		jsonObjs.Remove(ite);
	}
}

void SimpleBinaryJson::MarkDirty(SimpleJsonObject* instanceID) {
	updateMap.ForceEmplace(instanceID, true);
}

void SimpleBinaryJson::MarkDelete(SimpleJsonObject* instanceID) {
	updateMap.ForceEmplace(instanceID, false);
}

SimpleBinaryJson::SimpleBinaryJson()
	: arrPool(256),
	  dictPool(256),
	  rootObj(0, this) {
}
IJsonDict* SimpleBinaryJson::GetRootObject() {
	return &rootObj;
}
IJsonDict* SimpleBinaryJson::CreateJsonObject() {
	auto id = ++instanceCount;
	auto v = dictPool.New(id, this);
	MarkDirty(v);
	jsonObjs.Emplace(id, v, DICT_TYPE);
	return v;
}
IJsonArray* SimpleBinaryJson::CreateJsonArray() {
	auto id = ++instanceCount;
	auto v = arrPool.New(id, this);
	MarkDirty(v);
	jsonObjs.Emplace(id, v, ARRAY_TYPE);
	return v;
}
IJsonDict* SimpleBinaryJson::GetJsonObject(uint64 instanceID) {
	auto ite = jsonObjs.Find(instanceID);
	if (!ite)
		return nullptr;
	auto&& v = ite.Value();
	if (v.second != DICT_TYPE)
		return nullptr;
	return static_cast<SimpleJsonDict*>(v.first);
}
IJsonArray* SimpleBinaryJson::GetJsonArray(uint64 instanceID) {
	auto ite = jsonObjs.Find(instanceID);
	if (!ite)
		return nullptr;
	auto&& v = ite.Value();
	if (v.second != ARRAY_TYPE)
		return nullptr;
	return static_cast<SimpleJsonArray*>(v.first);
}
bool SimpleBinaryJson::Dispose(IJsonDict* jsonObj) {
	auto dict = static_cast<SimpleJsonDict*>(jsonObj);
	MarkDelete(dict);
	auto ite = jsonObjs.Find(dict->GetInstanceID());
	return DisposeProperty(ite, dict);
}
bool SimpleBinaryJson::Dispose(IJsonArray* jsonArr) {
	auto arr = static_cast<SimpleJsonArray*>(jsonArr);
	MarkDelete(arr);
	auto ite = jsonObjs.Find(arr->GetInstanceID());
	return DisposeProperty(ite, arr);
}

vstd::vector<uint8_t> SimpleBinaryJson::Sync() {
	vstd::vector<uint8_t> serData;
	serData.reserve(16384);
	auto Push = [&]<typename T>(T&& v) {
		PushDataToVector<T>(std::forward<T>(v), serData);
	};
	Push.operator()<uint8_t>(253);
	Push(GetHeader());
	vstd::vector<SimpleJsonObject*> addCmds;
	addCmds.reserve(updateMap.size());
	vstd::vector<SimpleJsonObject*> deleteCmds;
	deleteCmds.reserve(updateMap.size());
	for (auto&& i : updateMap) {
		if (i.second) {
			addCmds.push_back(i.first);
		} else
			deleteCmds.push_back(i.first);
		i.first->Reset();
	}
	// Create
	Push.operator()<uint8_t>(127);
	for (auto&& i : addCmds) {
		i->M_GetSerData(serData);
	}
	Push(std::numeric_limits<uint8_t>::max());
	// Delete
	Push.operator()<uint8_t>(128);
	for (auto&& i : deleteCmds) {
		auto vv = i->InstanceID();
		Push.operator()<uint64&>(vv);
	}
	Push(std::numeric_limits<uint64>::max());
	Push.operator()<uint8_t>(0);
	updateMap.Clear();
	return serData;
}

vstd::vector<uint8_t> SimpleBinaryJson::Serialize() {
	vstd::vector<uint8_t> serData;
	serData.reserve(65536);
	auto Push = [&]<typename T>(T&& v) {
		PushDataToVector<T>(std::forward<T>(v), serData);
	};

	Push.operator()<uint8_t>(254);
	Push(GetHeader());
	/////////////// Root Obj
	rootObj.M_GetSerData(serData);
	rootObj.Reset();
	for (auto&& i : jsonObjs) {
		i.second.first->M_GetSerData(serData);
		i.second.first->Reset();
	}
	Push(std::numeric_limits<uint8_t>::max());
	updateMap.Clear();
	return serData;
}
void SimpleBinaryJson::Read(std::span<uint8_t> sp) {
	//size_t pp = reinterpret_cast<size_t>(sp.data());
	auto serType = PopValue<uint8_t>(sp);
	std::span<uint8_t> rootChunk;
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>> vecs;
	auto CreateObj = [&](uint64 instanceID, std::span<uint8_t> sp, uint8_t targetType) {
		auto ite = jsonObjs.Find(instanceID);
		if (ite) {
			auto type = ite.Value().second;
			auto&& ptr = ite.Value().first;
			if (type != targetType) {
				//dict
				if (type == DICT_TYPE) {
					dictPool.Delete(static_cast<SimpleJsonDict*>(ptr));
					ptr = arrPool.New(instanceID, this);
				}
				//array
				else {
					arrPool.Delete(static_cast<SimpleJsonArray*>(ptr));
					ptr = dictPool.New(instanceID, this);
				}
			}
			vecs.emplace_back(ptr, sp);
		} else {
			ite = jsonObjs.Emplace(instanceID, nullptr, targetType);
			auto&& ptr = ite.Value().first;
			if (targetType == DICT_TYPE) {
				ptr = dictPool.New(instanceID, this);
			}
			//array
			else {
				ptr = arrPool.New(instanceID, this);
			}
			vecs.emplace_back(ptr, sp);
		}
	};
	auto CreateRoot = [&](std::span<uint8_t> sp) {
		rootChunk = sp;
		return &rootObj;
	};
	auto PopObj = [&]() {
		uint8_t type = PopValue<uint8_t>(sp);
		if (type == std::numeric_limits<uint8_t>::max())
			return false;
		uint64 instanceID = PopValue<uint64>(sp);
		uint64 spanSize = PopValue<uint64>(sp);
		if (instanceID == 0) {
			if (type != DICT_TYPE)
				return false;
			CreateRoot(std::span<uint8_t>(sp.data(), spanSize));
		} else {

			switch (type) {
				//Array
				case ARRAY_TYPE:
					CreateObj(instanceID, std::span<uint8_t>(sp.data(), spanSize), ARRAY_TYPE);
					break;
				//Dict
				case DICT_TYPE:
					CreateObj(instanceID, std::span<uint8_t>(sp.data(), spanSize), DICT_TYPE);
					break;
			}
		}
		sp = std::span<uint8_t>(sp.data() + spanSize, sp.size() - spanSize);
		return true;
	};
	// Header
	SerializeHeader header = PopValue<SerializeHeader>(sp);
	instanceCount = header.instanceCount;

	switch (serType) {
		//Sync Object
		case 253:
			[&]() {
				while (true) {
					auto cmd = PopValue<uint8_t>(sp);
					switch (cmd) {
						case 0:
							return;
						case 127:
							while (PopObj()) {}
							break;
						case 128:
							while (true) {
								uint64 instanceID = PopValue<uint64>(sp);
								if (instanceID == std::numeric_limits<uint64>::max()) break;
								Dispose(instanceID);
							}
							break;
					}
				}
			}();
			break;

		//Rebuild all
		case 254: {
			for (auto&& i : jsonObjs) {
				DisposeProperty(i.second);
			}
			jsonObjs.Clear();

			while (PopObj()) {}
		} break;
	}
	if (!rootChunk.empty()) {
		rootObj.LoadFromData(rootChunk);
	}
	for (auto&& i : vecs) {
		i.first->LoadFromData(i.second);
	}
}

IJsonDataBase* CreateSimpleJsonDB() {
	return new SimpleBinaryJson();
}
}// namespace toolhub::db