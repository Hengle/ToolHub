#pragma vengine_package vengine_database

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/Pool.h>
#include <Database/SimpleBinaryJson.h>
#include <boost/asio.hpp>
#include <DatabaseInclude.h>
namespace toolhub::db {

class SimpleBinaryJson;
void SimpleBinaryJson::DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& v) {
	if (v.second == ARRAY_TYPE) {
		arrPool.Delete(static_cast<SimpleJsonArray*>(v.first));
	} else {
		dictPool.Delete(static_cast<SimpleJsonDict*>(v.first));
	}
}

void SimpleBinaryJson::Dispose(ObjMap::Index ite) {
	if (ite) {
		DisposeProperty(ite.Value());
		jsonObjs.Remove(ite);
	}
}
void SimpleBinaryJson::Dispose(ObjMap::Index ite, IDatabaseEvtVisitor* evtVisitor) {
	if (ite) {
		ite.Value().first->BeforeRemove(evtVisitor);
		DisposeProperty(ite.Value());
		jsonObjs.Remove(ite);
	}
}

void SimpleBinaryJson::MarkDirty(SimpleJsonObject* dict) {
	if (dict->dirtyID >= updateVec.size()) {
		dict->dirtyID = updateVec.size();
		updateVec.emplace_back(dict);
	} else {
		updateVec[dict->dirtyID] = dict;
	}
}

void SimpleBinaryJson::MarkDelete(SimpleJsonObject* dict) {
	if (dict->dirtyID != std::numeric_limits<uint64>::max()) {
		updateVec[dict->dirtyID] = dict->InstanceID();
	} else {
		dict->dirtyID = updateVec.size();
		updateVec.emplace_back(dict->InstanceID());
	}
}

SimpleBinaryJson::SimpleBinaryJson(uint64 index, IJsonDatabase* parent)
	: arrPool(256),
	  dictPool(256),
	  parent(parent),
	  index(index),
	  rootObj(0, this) {
}

IJsonDict* SimpleBinaryJson::GetRootObject() {
	return &rootObj;
}
IJsonDict* SimpleBinaryJson::CreateJsonObject() {
	auto id = ++instanceCount;
	auto v = dictPool.New(id, this);
	MarkDirty(v);
	v->dbIndexer = jsonObjs.Emplace(id, v, DICT_TYPE);
	return v;
}
IJsonArray* SimpleBinaryJson::CreateJsonArray() {
	auto id = ++instanceCount;
	auto v = arrPool.New(id, this);
	MarkDirty(v);
	v->dbIndexer = jsonObjs.Emplace(id, v, ARRAY_TYPE);
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
void SimpleBinaryJson::Dispose(IJsonDict* jsonObj) {
	auto dict = static_cast<SimpleJsonDict*>(jsonObj);
	MarkDelete(dict);
	jsonObjs.Remove(dict->dbIndexer);
	dictPool.Delete(dict);
}
void SimpleBinaryJson::Dispose(IJsonArray* jsonArr) {
	auto arr = static_cast<SimpleJsonArray*>(jsonArr);
	MarkDelete(arr);
	jsonObjs.Remove(arr->dbIndexer);
	arrPool.Delete(arr);
}

void SimpleBinaryJson::Dispose() {
	delete this;
}

vstd::vector<uint8_t> SimpleBinaryJson::IncreSerialize() {
	vstd::vector<uint8_t> serData;
	serData.reserve(16384);
	auto Push = [&]<typename T>(T&& v) {
		PushDataToVector<T>(std::forward<T>(v), serData);
	};
	Push.operator()<uint8_t>(253);
	Push(GetHeader());
	vstd::vector<SimpleJsonObject*> addCmds;
	addCmds.reserve(updateVec.size());
	vstd::vector<uint64> deleteCmds;
	deleteCmds.reserve(updateVec.size());
	for (auto&& i : updateVec) {
		i.visit(
			[&](auto o) {
				addCmds.push_back(o);
			},
			[&](auto o) {
				deleteCmds.push_back(o);
			});
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
		Push.operator()<uint64&>(i);
	}
	Push(std::numeric_limits<uint64>::max());
	Push.operator()<uint8_t>(0);
	updateVec.clear();
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
	rootObj.dirtyID = std::numeric_limits<uint64>::max();
	for (auto&& i : jsonObjs) {
		i.second.first->M_GetSerData(serData);
		i.second.first->dirtyID = std::numeric_limits<uint64>::max();
	}
	Push(std::numeric_limits<uint8_t>::max());
	updateVec.clear();
	return serData;
}

void SimpleBinaryJson::Ser_CreateObj(
	uint64 instanceID,
	std::span<uint8_t> sp,
	uint8_t targetType,
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs) {
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
		ptr->dbIndexer = ite;
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
		ptr->dbIndexer = ite;
		vecs.emplace_back(ptr, sp);
	}
}

void SimpleBinaryJson::Read(std::span<uint8_t> sp) {
	//size_t pp = reinterpret_cast<size_t>(sp.data());

	auto serType = PopValue<uint8_t>(sp);
	std::span<uint8_t> rootChunk;
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>> vecs;

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
							while (Ser_PopValue(sp, rootChunk, vecs)) {}
							break;
						case 128:
							while (true) {
								uint64 instanceID = PopValue<uint64>(sp);
								if (instanceID == std::numeric_limits<uint64>::max()) break;
								Dispose(jsonObjs.Find(instanceID));
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

			while (Ser_PopValue(sp, rootChunk, vecs)) {}
		} break;
	}
	if (!rootChunk.empty()) {
		rootObj.LoadFromData(rootChunk);
	}
	for (auto&& i : vecs) {
		i.first->LoadFromData(i.second);
	}
}

bool SimpleBinaryJson::Ser_PopValue(
	std::span<uint8_t>& sp,
	std::span<uint8_t>& rootChunk,
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs) {
	auto CreateObj = [&](uint64 instanceID, std::span<uint8_t> sp, uint8_t targetType) {
		Ser_CreateObj(instanceID, sp, targetType, vecs);
	};
	auto CreateRoot = [&](std::span<uint8_t> sp) {
		rootChunk = sp;
		return &rootObj;
	};
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
}

void SimpleBinaryJson::Read(
	std::span<uint8_t> sp,
	IDatabaseEvtVisitor* evtVisitor) {

	auto serType = PopValue<uint8_t>(sp);
	std::span<uint8_t> rootChunk;
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>> vecs;

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
							while (Ser_PopValue(sp, rootChunk, vecs)) {}
							break;
						case 128:
							while (true) {
								uint64 instanceID = PopValue<uint64>(sp);
								if (instanceID == std::numeric_limits<uint64>::max()) break;
								Dispose(jsonObjs.Find(instanceID), evtVisitor);
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

			while (Ser_PopValue(sp, rootChunk, vecs)) {}
		} break;
	}
	if (!rootChunk.empty()) {
		rootObj.LoadFromData(rootChunk);
		rootObj.AfterAdd(evtVisitor);
	}
	for (auto&& i : vecs) {
		i.first->LoadFromData(i.second);
		i.first->AfterAdd(evtVisitor);
	}
}

class SimpleDatabaseParent : public IJsonDatabase, public vstd::IOperatorNewBase {
public:
	vstd::vector<IJsonSubDatabase*> subDatabases;
	~SimpleDatabaseParent() {
		for (auto i : subDatabases) {
			i->Dispose();
		}
	}
	void Dispose() override {
		delete this;
	}
	IJsonSubDatabase* CreateDatabase(std::span<uint8_t> command) override {
		auto js = new SimpleBinaryJson(subDatabases.size(), this);
		subDatabases.push_back(js);
		return js;
	}
	IJsonSubDatabase* CreateOrGetDatabase(uint64 targetIndex, std::span<uint8_t> command) override {
		auto lastSize = subDatabases.size();
		subDatabases.resize(targetIndex + 1);
		if (subDatabases.size() > lastSize) {
			memset(
				subDatabases.data() + lastSize,
				0,
				(subDatabases.size() - lastSize) * sizeof(IJsonSubDatabase*));
		}
		auto&& v = subDatabases[targetIndex];
		if (!v) {
			v = new SimpleBinaryJson(targetIndex, this);
		}
		return v;
	}
	void DisposeDatabase(uint64 index) override {
		if (index < subDatabases.size()) {
			auto&& v = subDatabases[index];
			v->Dispose();
			v = nullptr;
		}
	}
	IJsonSubDatabase* GetDatabase(uint64 index) override {
		if (index < subDatabases.size()) {
			return subDatabases[index];
		}
		return nullptr;
	}
};
IJsonDatabase* Database_Impl::CreateDatabase() const {
	return new SimpleDatabaseParent();
}
}// namespace toolhub::db