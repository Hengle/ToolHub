#pragma once
#include <Common/Common.h>
#include <Database/SimpleJsonLoader.h>
#include <Utility/VGuid.h>
namespace toolhub::db {

class SimpleJsonLoader;
class SimpleJsonObject {
protected:
	vstd::Guid selfGuid;
	SimpleBinaryJson* db = nullptr;
	SimpleJsonObject(
		vstd::Guid const& guid,
		SimpleBinaryJson* db);
	~SimpleJsonObject() {}

public:
	HashMap<vstd::Guid, std::pair<SimpleJsonObject*, uint8_t>>::Index dbIndexer;
	uint64 dirtyID = std::numeric_limits<uint64>::max();
	SimpleBinaryJson* GetDB() const { return db; }
	vstd::Guid const& GetGUID() const { return selfGuid; }
	void Update();
	virtual void M_GetSerData(vstd::vector<uint8_t>& data) = 0;
	virtual void LoadFromData(std::span<uint8_t> data) = 0;
	virtual void AfterAdd(IDatabaseEvtVisitor* visitor) = 0;
	virtual void BeforeRemove(IDatabaseEvtVisitor* visitor) = 0;
};
}// namespace toolhub::db