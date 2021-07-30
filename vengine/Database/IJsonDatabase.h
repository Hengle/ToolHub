#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;
class IDatabaseEvtVisitor {
public:
	virtual void AddDict(IJsonDict* newDict) = 0;
	virtual void RemoveDict(IJsonDict* removedDict) = 0;
	virtual void AddArray(IJsonArray* newDict) = 0;
	virtual void RemoveArray(IJsonArray* newDict) = 0;
};
class IJsonDataBase {
public:
	virtual IJsonDict* GetRootObject() = 0;
	virtual IJsonDict* CreateJsonObject() = 0;
	virtual IJsonArray* CreateJsonArray() = 0;
	virtual IJsonDict* GetJsonObject(uint64 id) = 0;
	virtual IJsonArray* GetJsonArray(uint64 id) = 0;
	virtual vstd::vector<uint8_t> Serialize() = 0;
	virtual vstd::vector<uint8_t> IncreSerialize() = 0;
	virtual void Read(std::span<uint8_t> data) = 0;
	virtual void Read(
		std::span<uint8_t> data,
		IDatabaseEvtVisitor* evtVisitor) = 0;

	virtual ~IJsonDataBase() {}
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};


}// namespace toolhub::db