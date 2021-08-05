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
class IJsonSubDatabase;
class IJsonDatabase : public vstd::IDisposable {
protected:
	~IJsonDatabase() = default;

public:
	virtual IJsonSubDatabase* CreateDatabase(std::span<uint8_t> command) = 0;
	virtual IJsonSubDatabase* CreateOrGetDatabase(uint64 targetIndex, std::span<uint8_t> command) = 0;
	virtual void DisposeDatabase(uint64 index) = 0;
	virtual IJsonSubDatabase* GetDatabase(uint64 index) = 0;
};
class IJsonSubDatabase : public vstd::IDisposable {
protected:
	~IJsonSubDatabase() = default;

public:
	virtual uint64 GetIndex() = 0;
	virtual IJsonDict* GetRootObject() = 0;
	virtual IJsonDict* CreateJsonObject() = 0;
	virtual IJsonArray* CreateJsonArray() = 0;
	virtual IJsonDict* GetJsonObject(uint64 id) = 0;
	virtual IJsonArray* GetJsonArray(uint64 id) = 0;
	virtual vstd::vector<uint8_t> Serialize() = 0;
	virtual vstd::vector<uint8_t> IncreSerialize() = 0;
	virtual void Read(std::span<uint8_t> data) = 0;
	virtual IJsonDatabase* GetParent() = 0;
	virtual void Read(
		std::span<uint8_t> data,
		IDatabaseEvtVisitor* evtVisitor) = 0;
};

}// namespace toolhub::db