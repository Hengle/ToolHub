#pragma once
#include <Common/linq.h>
#include <Utility/VGuid.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;
using JsonVariant = vstd::variant<int64,
								  double,
								  bool,
								  vstd::string,
								  IJsonDict*,
								  IJsonArray*>;
struct JsonKeyPair {
	vstd::string_view key;
	JsonVariant value;
};

class IJsonDict {
protected:
	virtual ~IJsonDict() {}

public:
	virtual JsonVariant Get(vstd::string_view key) = 0;
	virtual void Set(vstd::string key, JsonVariant value) = 0;
	virtual void Remove(vstd::string const& key) = 0;
	virtual vstd::linq::Iterator<JsonKeyPair>* GetIterator() = 0;
};

class IJsonArray {
protected:
	virtual ~IJsonArray(){};

public:
	virtual size_t Length() = 0;
	virtual JsonVariant Get(size_t index) = 0;
	virtual void Set(size_t index, JsonVariant value) = 0;
	virtual void Remove(size_t index) = 0;
	virtual void Add(JsonVariant value) = 0;
	virtual vstd::linq::Iterator<JsonVariant>* GetIterator() = 0;
};
}// namespace toolhub::db