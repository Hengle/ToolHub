#pragma once
#include <Common/Common.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
class SimpleJsonLoader;
class SimpleJsonObject {
public:
	SimpleJsonLoader loader;
	uint64 instanceID = 0;
	SimpleBinaryJson* db = nullptr;
	uint64 version = 0;
	void Update();
	virtual ~SimpleJsonObject();
};
}// namespace toolhub::db