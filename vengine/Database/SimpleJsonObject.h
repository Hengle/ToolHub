#pragma once
#include <Common/Common.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
class SimpleJsonLoader;
class SimpleJsonObject {
protected:
	uint64 instanceID = 0;
	SimpleBinaryJson* db = nullptr;
	bool isDirty = false;
	SimpleJsonObject(
		uint64 instanceID,
		SimpleBinaryJson* db);
	~SimpleJsonObject() {}

public:
	SimpleBinaryJson* GetDB() const { return db; }
	uint64 InstanceID() const { return instanceID; }
	void Update();
	void Reset();
	virtual void M_GetSerData(vstd::vector<uint8_t>& data) = 0;
	virtual void LoadFromData(std::span<uint8_t> data) = 0;
};
}// namespace toolhub::db