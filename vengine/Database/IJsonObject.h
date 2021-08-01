#pragma once
#include <Common/linq.h>
#include <Utility/VGuid.h>
#include <Database/IJsonDatabase.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;
template<typename T>
struct JsonObjID {
	uint64 instanceID;
	JsonObjID(T* ptr) {
		instanceID = ptr->GetInstanceID();
	}
	operator uint64() const {
		return instanceID;
	}
};

using JsonVariant = vstd::variant<int64,
								  double,
								  vstd::string,
								  JsonObjID<IJsonDict>,
								  JsonObjID<IJsonArray>>;

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
	virtual uint64 GetInstanceID() = 0;
	virtual void Clean() = 0;
	virtual IJsonDataBase* GetDatabase() = 0;
};

class IJsonDict : public IJsonObject {
protected:
	~IJsonDict() {}

public:
	virtual JsonVariant Get(vstd::string_view key) = 0;
	virtual void Set(vstd::string key, JsonVariant value) = 0;
	virtual void Remove(vstd::string const& key) = 0;
	virtual vstd::unique_ptr<vstd::linq::Iterator<JsonKeyPair>> GetIterator() = 0;

	virtual vstd::optional<int64> GetInt(vstd::string_view key) = 0;
	virtual vstd::optional<double> GetFloat(vstd::string_view key) = 0;
	virtual vstd::optional<vstd::string_view> GetString(vstd::string_view key) = 0;
	virtual vstd::optional<IJsonDict*> GetDict(vstd::string_view key) = 0;
	virtual vstd::optional<IJsonArray*> GetArray(vstd::string_view key) = 0;
};

class IJsonArray : public IJsonObject {
protected:
	~IJsonArray(){};

public:
	virtual JsonVariant Get(size_t index) = 0;
	virtual void Set(size_t index, JsonVariant value) = 0;
	virtual void Remove(size_t index) = 0;
	virtual void Add(JsonVariant value) = 0;
	virtual vstd::unique_ptr<vstd::linq::Iterator<JsonVariant>> GetIterator() = 0;

	virtual vstd::optional<int64> GetInt(size_t index) = 0;
	virtual vstd::optional<double> GetFloat(size_t index) = 0;
	virtual vstd::optional<vstd::string_view> GetString(size_t index) = 0;
	virtual vstd::optional<IJsonDict*> GetDict(size_t index) = 0;
	virtual vstd::optional<IJsonArray*> GetArray(size_t index) = 0;
};
}// namespace toolhub::db