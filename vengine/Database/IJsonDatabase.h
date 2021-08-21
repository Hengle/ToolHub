#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Utility/VGuid.h>
#include <Common/linq.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;
class IJsonDatabase : public vstd::IDisposable {
protected:
	~IJsonDatabase() = default;

public:
	virtual vstd::vector<uint8_t> Serialize() = 0;
	virtual void Read(
		std::span<uint8_t> data) = 0;
	virtual IJsonDict* GetRootNode() = 0;
	virtual vstd::unique_ptr<IJsonDict> CreateDict() = 0;
	virtual vstd::unique_ptr<IJsonArray> CreateArray() = 0;
};

}// namespace toolhub::db