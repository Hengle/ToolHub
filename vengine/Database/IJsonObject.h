#pragma once
#include <Common/linq.h>
#include <Utility/VGuid.h>
#include <Database/IJsonDatabase.h>
namespace toolhub::db {
class IJsonRefDict;
class IJsonRefArray;
class IJsonValueDict;
class IJsonValueArray;

using JsonVariant = vstd::variant<int64,
								  double,
								  vstd::string_view,
								  IJsonValueDict*,
								  IJsonValueArray*,
								  vstd::Guid>;

struct JsonKeyPair {
	vstd::string_view key;
	JsonVariant value;
};

class IJsonObject : public vstd::IDisposable {
protected:
	~IJsonObject() = default;

public:
	virtual size_t Length() = 0;
	virtual vstd::vector<uint8_t> GetSerData() = 0;
	virtual void Clean() = 0;
	virtual void Reset() = 0;
};
class IJsonRefType {
public:
	virtual vstd::Guid GetGUID() = 0;
	virtual IJsonSubDatabase* GetDatabase() = 0;
};

class IJsonDict {
protected:
	~IJsonDict() {}

public:
	virtual JsonVariant Get(vstd::string_view key) = 0;
	virtual void Set(vstd::string key, JsonVariant value) = 0;
	virtual IJsonValueDict* AddOrGetDict(vstd::string key) = 0;
	virtual IJsonValueArray* AddOrGetArray(vstd::string key) = 0;
	virtual void Remove(vstd::string const& key) = 0;
	virtual vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() = 0;
};

class IJsonArray {
protected:
	~IJsonArray(){};

public:
	virtual JsonVariant Get(size_t index) = 0;
	virtual void Set(size_t index, JsonVariant value) = 0;
	virtual void Remove(size_t index) = 0;
	virtual void Add(JsonVariant value) = 0;
	virtual IJsonValueDict* AddDict() = 0;
	virtual IJsonValueArray* AddArray() = 0;
	virtual vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> GetIterator() = 0;
};

class IJsonRefDict : public IJsonDict, public IJsonRefType, public IJsonObject {};
class IJsonRefArray : public IJsonArray, public IJsonRefType, public IJsonObject{};

class IJsonValueDict : public IJsonDict, public IJsonObject {};
class IJsonValueArray : public IJsonArray, public IJsonObject {};

}// namespace toolhub::db