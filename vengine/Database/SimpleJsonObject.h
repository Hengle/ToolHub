#pragma once
#include <Common/Common.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
class SimpleJsonLoader;
class SimpleJsonObject {
protected:
	uint64 instanceID = 0;
	SimpleBinaryJson* db = nullptr;
	SimpleJsonObject(
		uint64 instanceID,
		SimpleBinaryJson* db);
	~SimpleJsonObject() {}

public:
	uint64 dirtyID = std::numeric_limits<uint64>::max();
	SimpleBinaryJson* GetDB() const { return db; }
	uint64 InstanceID() const { return instanceID; }
	void Update();
	virtual void M_GetSerData(vstd::vector<uint8_t>& data) = 0;
	virtual void LoadFromData(std::span<uint8_t> data) = 0;
	virtual void AfterAdd(IDatabaseEvtVisitor* visitor) = 0;
	virtual void BeforeRemove(IDatabaseEvtVisitor* visitor) = 0;
};
}// namespace toolhub::db