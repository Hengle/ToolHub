#pragma vengine_package vengine_database

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/Pool.h>
#include <Database/SimpleBinaryJson.h>
#include <boost/asio.hpp>
#include <Database/DatabaseInclude.h>
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
		updateVec[dict->dirtyID] = dict->GetGUID();
	} else {
		dict->dirtyID = updateVec.size();
		updateVec.emplace_back(dict->GetGUID());
	}
}

SimpleBinaryJson::SimpleBinaryJson(vstd::Guid const& index, IJsonDatabase* parent)
	: arrPool(256),
	  dictPool(256),
	  dictValuePool(256),
	  arrValuePool(256),
	  parent(parent),
	  index(index),
	  rootGuid(vstd::Guid(true)) {
}

SimpleBinaryJson::~SimpleBinaryJson() {
	enabled = false;
}

IJsonRefDict* SimpleBinaryJson::GetRootObject() {
	auto ite = jsonObjs.Find(rootGuid);
	if (ite) return static_cast<SimpleJsonDict*>(ite.Value().first);
	auto ptr = dictPool.New(rootGuid, this);
	jsonObjs.Emplace(rootGuid, ptr, DICT_TYPE);
	return ptr;
}
IJsonRefDict* SimpleBinaryJson::CreateJsonObject() {
	vstd::Guid guid(true);
	auto v = dictPool.New(guid, this);
	MarkDirty(v);
	v->dbIndexer = jsonObjs.Emplace(guid, v, DICT_TYPE);
	return v;
}
IJsonRefArray* SimpleBinaryJson::CreateJsonArray() {
	vstd::Guid guid(true);
	auto v = arrPool.New(guid, this);
	MarkDirty(v);
	v->dbIndexer = jsonObjs.Emplace(guid, v, ARRAY_TYPE);
	return v;
}
IJsonRefDict* SimpleBinaryJson::GetJsonObject(vstd::Guid const& instanceID) {
	auto ite = jsonObjs.Find(instanceID);
	if (!ite)
		return nullptr;
	auto&& v = ite.Value();
	if (v.second != DICT_TYPE)
		return nullptr;
	return static_cast<SimpleJsonDict*>(v.first);
}
IJsonRefArray* SimpleBinaryJson::GetJsonArray(vstd::Guid const& instanceID) {
	auto ite = jsonObjs.Find(instanceID);
	if (!ite)
		return nullptr;
	auto&& v = ite.Value();
	if (v.second != ARRAY_TYPE)
		return nullptr;
	return static_cast<SimpleJsonArray*>(v.first);
}
void SimpleBinaryJson::Dispose(IJsonRefDict* jsonObj) {
	auto dict = static_cast<SimpleJsonDict*>(jsonObj);
	MarkDelete(dict);
	jsonObjs.Remove(dict->dbIndexer);
	dictPool.Delete(dict);
}
void SimpleBinaryJson::Dispose(IJsonRefArray* jsonArr) {
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
	Push(rootGuid);
	vstd::vector<SimpleJsonObject*> addCmds;
	addCmds.reserve(updateVec.size());
	vstd::vector<vstd::Guid> deleteCmds;
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
		Push.operator()<vstd::Guid&>(i);
	}
	Push(vstd::Guid(false));
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
	Push(rootGuid);
	for (auto&& i : jsonObjs) {
		i.second.first->M_GetSerData(serData);
		i.second.first->dirtyID = std::numeric_limits<uint64>::max();
	}
	Push(std::numeric_limits<uint8_t>::max());
	updateVec.clear();
	return serData;
}

void SimpleBinaryJson::Ser_CreateObj(
	vstd::Guid const& instanceID,
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

bool SimpleBinaryJson::Ser_PopValue(
	std::span<uint8_t>& sp,
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs) {

	auto CreateObj = [&](vstd::Guid const& instanceID, std::span<uint8_t> sp, uint8_t targetType) {
		Ser_CreateObj(instanceID, sp, targetType, vecs);
	};

	uint8_t type = PopValue<uint8_t>(sp);
	if (type == std::numeric_limits<uint8_t>::max())
		return false;
	vstd::Guid instanceID = PopValue<vstd::Guid>(sp);
	uint64 spanSize = PopValue<uint64>(sp);

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

	sp = std::span<uint8_t>(sp.data() + spanSize, sp.size() - spanSize);
	return true;
}

void SimpleBinaryJson::Read(
	std::span<uint8_t> sp,
	IDatabaseEvtVisitor* evtVisitor) {

	auto oo = sp.data();
	auto serType = PopValue<uint8_t>(sp);
	rootGuid = PopValue<vstd::Guid>(sp);
	vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>> vecs;

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
							while (Ser_PopValue(sp, vecs)) {}
							break;
						case 128:
							while (true) {
								vstd::Guid const& instanceID = PopValue<vstd::Guid const&>(sp);
								if (!instanceID) break;
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

			while (Ser_PopValue(sp, vecs)) {
			}
		} break;
	}
	for (auto&& i : vecs) {
		i.first->LoadFromData(i.second);
		i.first->AfterAdd(evtVisitor);
	}
}
ThreadTaskHandle SimpleBinaryJson::CollectGarbage(ThreadPool* tPool) {
	return tPool->GetTask([this, tPool]() {
		HashMap<vstd::Guid, bool> collectedGuid;
		Runnable<void(IJsonRefArray*)> iteArr;
		Runnable<void(IJsonRefDict*)> iteDict;
		Runnable<void(IJsonValueDict*)> iteValueDict;
		Runnable<void(IJsonValueArray*)> iteValueArr;

		auto processVariant = [&](JsonVariant const& var) {
			var.visit(
				[](auto&&) {},
				[](auto&&) {},
				[](auto&&) {},
				iteDict,
				iteArr,
				iteValueDict,
				iteValueArr,
				[&](auto&& guid) { collectedGuid.Emplace(guid, true); });
		};
		iteArr = [&](IJsonRefArray* arr) {
			if (!arr) return;
			if (typeid(*arr) == typeid(SimpleJsonArray)) {
				SimpleJsonArray* ptr = static_cast<SimpleJsonArray*>(arr);
				for (auto&& i : ptr->arrs) {
					processVariant(i.GetVariant());
				}
			} else {
				auto ite = arr->GetIterator();
				LINQ_LOOP(i, *ite) {
					processVariant(*i);
				}
			}
		};
		iteDict = [&](IJsonRefDict* dict) {
			if (!dict) return;
			if (typeid(*dict) == typeid(SimpleJsonDict)) {
				SimpleJsonDict* ptr = static_cast<SimpleJsonDict*>(dict);
				for (auto&& i : ptr->vars) {
					processVariant(i.second.GetVariant());
				}
			} else {
				auto ite = dict->GetIterator();
				LINQ_LOOP(i, *ite) {
					processVariant(i->value);
				}
			}
		};
		iteValueDict = [&](IJsonValueDict* dict) {
			if (typeid(*dict) == typeid(SimpleJsonValueDict)) {
				SimpleJsonValueDict* ptr = static_cast<SimpleJsonValueDict*>(dict);
				for (auto&& i : ptr->vars) {
					processVariant(i.second.GetVariant());
				}
			} else {
				auto ite = dict->GetIterator();
				LINQ_LOOP(i, *ite) {
					processVariant(i->value);
				}
			}
		};
		iteValueArr = [&](IJsonValueArray* arr) {
			if (typeid(*arr) == typeid(SimpleJsonValueArray)) {
				SimpleJsonValueArray* ptr = static_cast<SimpleJsonValueArray*>(arr);
				for (auto&& i : ptr->arr) {
					processVariant(i.GetVariant());
				}
			} else {
				auto ite = arr->GetIterator();
				LINQ_LOOP(i, *ite) {
					processVariant(*i);
				}
			}
		};
		auto cleanHandle = tPool->GetBeginEndTask(
			[&](size_t beg, size_t ed) {
				auto b = jsonObjs.begin() + beg;
				auto e = jsonObjs.begin() + ed;
				for (auto ite = b; ite != e; ++ite) {
					if ((*ite).second.second == DICT_TYPE) {
						auto ptr = static_cast<SimpleJsonDict*>((*ite).second.first);
						ptr->Clean();
						iteDict(ptr);

					} else {
						auto ptr = static_cast<SimpleJsonArray*>((*ite).second.first);
						ptr->Clean();
						iteArr(ptr);
					}
				}
			},
			jsonObjs.size());
		auto collectHandle = tPool->GetTask([&]() {
			vstd::vector<ObjMap::NodePair const*> removeList;
			for (auto&& i : jsonObjs) {
				if (!collectedGuid.Find(i.first))
					removeList.push_back(&i);
			}
			for (auto&& i : removeList) {
				jsonObjs.Remove(i);
			}
		});
		collectHandle.AddDepend(cleanHandle);
		collectHandle.Complete();
	});
}

class SimpleDatabaseParent : public IJsonDatabase, public vstd::IOperatorNewBase {
public:
	HashMap<vstd::Guid, vstd::unique_ptr<IJsonSubDatabase>> subDatabases;
	~SimpleDatabaseParent() {
	}
	void Dispose() override {
		delete this;
	}
	IJsonSubDatabase* CreateDatabase(std::span<uint8_t> command) override {
		vstd::Guid guid(true);
		auto js = new SimpleBinaryJson(guid, this);
		subDatabases.Emplace(guid, js);
		return js;
	}
	IJsonSubDatabase* CreateOrGetDatabase(vstd::Guid const& targetIndex, std::span<uint8_t> command) override {
		auto lastSize = subDatabases.size();
		auto ite = subDatabases.Find(targetIndex);
		if (ite) return ite.Value().get();
		subDatabases.Emplace(targetIndex, new SimpleBinaryJson(targetIndex, this));
	}
	void DisposeDatabase(vstd::Guid const& index) override {
		auto ite = subDatabases.Find(index);
		if (ite)
			subDatabases.Remove(ite);
	}
	IJsonSubDatabase* GetDatabase(vstd::Guid const& index) override {
		auto ite = subDatabases.Find(index);
		if (ite)
			return ite.Value().get();
		return nullptr;
	}
};
IJsonDatabase* Database_Impl::CreateDatabase() const {
	return new SimpleDatabaseParent();
}
}// namespace toolhub::db