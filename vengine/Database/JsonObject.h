#pragma once
#include <Common/linq.h>
#include <Utility/VGuid.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;
class IJsonDataBase;
using JsonVariant = vstd::variant<int64,
								  double,
								  vstd::string,
								  IJsonDict*,
								  IJsonArray*>;

struct JsonKeyPair {
	vstd::string_view key;
	JsonVariant value;
};

class IJsonObject {
public:
	virtual ~IJsonObject() = default;
	virtual size_t Length() = 0;
	virtual bool IsDirty() = 0;
	virtual vstd::vector<uint8_t> GetSerData() = 0;
	virtual void DeSer(std::span<uint8_t> data) = 0;
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};

class IJsonDict : public IJsonObject {
protected:
	virtual ~IJsonDict() {}

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
	virtual ~IJsonArray(){};

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