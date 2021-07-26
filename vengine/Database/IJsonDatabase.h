#pragma once
#include <Common/Common.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;

class IJsonDataBase {
public:
	virtual IJsonDict* GetRootObject() = 0;
	virtual IJsonDict* CreateJsonObject() = 0;
	virtual IJsonArray* CreateJsonArray() = 0;
	virtual bool Dispose(IJsonDict* jsonObj) = 0;
	virtual bool Dispose(IJsonArray* jsonArr) = 0;
	virtual vstd::vector<uint8_t> Serialize() = 0;
	virtual vstd::vector<uint8_t> Sync() = 0;
	virtual void Read(std::span<uint8_t> data) = 0;

	virtual ~IJsonDataBase() {}
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};
IJsonDataBase* CreateSimpleJsonDB();
}// namespace toolhub::db