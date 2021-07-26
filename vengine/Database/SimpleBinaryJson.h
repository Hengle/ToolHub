#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
namespace toolhub::db {

class SimpleBinaryJson final : public IJsonDataBase {

public:
	using ObjMap = HashMap<uint64, std::pair<SimpleJsonObject*, uint8_t>>;

	void DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& data);
	bool DisposeProperty(ObjMap::Index data, SimpleJsonObject* obj);
	void Dispose(uint64 instanceID);
	void MarkDirty(SimpleJsonObject*);
	void MarkDelete(SimpleJsonObject*);

	ObjMap jsonObjs;
	HashMap<SimpleJsonObject*, bool> updateMap;
	Pool<SimpleJsonArray> arrPool;
	Pool<SimpleJsonDict> dictPool;
	SimpleJsonDict rootObj;
	uint64 instanceCount = 0;
	SimpleBinaryJson();
	IJsonDict* GetRootObject() override;
	IJsonDict* CreateJsonObject() override;
	IJsonArray* CreateJsonArray() override;
	bool Dispose(IJsonDict* jsonObj) override;
	bool Dispose(IJsonArray* jsonArr) override;
	vstd::vector<uint8_t> Sync() override;

	struct SerializeHeader {
		uint64 instanceCount;
	};
	SerializeHeader GetHeader() const {
		return {instanceCount};
	}

	vstd::vector<uint8_t> Serialize() override;

	void Read(std::span<uint8_t> data) override;
	KILL_COPY_CONSTRUCT(SimpleBinaryJson)
	KILL_MOVE_CONSTRUCT(SimpleBinaryJson)
};
}// namespace toolhub::db