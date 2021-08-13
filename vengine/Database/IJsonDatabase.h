#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Utility/VGuid.h>
#include <JobSystem/ThreadPool.h>
namespace toolhub::db {
class IJsonRefDict;
class IJsonRefArray;
class IJsonValueDict;
class IJsonValueArray;

class IDatabaseEvtVisitor {
public:
	virtual void AddDict(IJsonRefDict* newDict) = 0;
	virtual void RemoveDict(IJsonRefDict* removedDict) = 0;
	virtual void AddArray(IJsonRefArray* newDict) = 0;
	virtual void RemoveArray(IJsonRefArray* newDict) = 0;
};
class IJsonSubDatabase;
class IJsonDatabase : public vstd::IDisposable {
protected:
	~IJsonDatabase() = default;

public:
	virtual IJsonSubDatabase* CreateDatabase(std::span<uint8_t> command) = 0;
	virtual IJsonSubDatabase* CreateOrGetDatabase(vstd::Guid const& targetIndex, std::span<uint8_t> command) = 0;
	virtual void DisposeDatabase(vstd::Guid const& index) = 0;
	virtual IJsonSubDatabase* GetDatabase(vstd::Guid const& index) = 0;
};
class IJsonSubDatabase : public vstd::IDisposable {
protected:
	~IJsonSubDatabase() = default;

public:
	virtual vstd::Guid GetGUID() = 0;
	virtual IJsonRefDict* GetRootObject() = 0;
	virtual IJsonRefDict* CreateJsonObject() = 0;
	virtual IJsonRefArray* CreateJsonArray() = 0;
	virtual IJsonRefDict* GetJsonObject(vstd::Guid const& id) = 0;
	virtual IJsonRefArray* GetJsonArray(vstd::Guid const& id) = 0;
	virtual vstd::vector<uint8_t> Serialize() = 0;
	virtual vstd::vector<uint8_t> IncreSerialize() = 0;
	virtual IJsonDatabase* GetParent() = 0;
	virtual ThreadTaskHandle CollectGarbage(ThreadPool* tPool) = 0;
	virtual void Read(
		std::span<uint8_t> data,
		IDatabaseEvtVisitor* evtVisitor) = 0;
};

}// namespace toolhub::db