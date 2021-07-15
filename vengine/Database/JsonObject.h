#pragma once
#include <Common/linq.h>
namespace toolhub::db {
class JsonObject;
class JsonArray;
class VENGINE_DLL_COMPUTE JsonVariant {
public:
	enum class ValueType : uint8_t {
		None = 0,
		Int = 1,
		Float = 2,
		Bool = 3,
		String = 4,
		JsonObject = 5,
		JsonArray = 6
	};

private:
	union {
		int64 intValue;
		double floatValue;
		bool boolValue;
		vstd::string_view strValue;
		JsonObject* jsonObj;
		JsonArray* jsonArr;
	};
	ValueType type;

public:
	ValueType GetType() const { return type; }
	JsonVariant();
	JsonVariant(int64 intValue);
	JsonVariant(double floatValue);
	JsonVariant(bool boolValue);
	JsonVariant(vstd::string_view strValue);
	JsonVariant(JsonObject* jsonObj);
	JsonVariant(JsonArray* jsonArr);

	vstd::optional<int64> GetInt() const;
	vstd::optional<double> GetFloat() const;
	vstd::optional<bool> GetBool() const;
	vstd::optional<vstd::string_view> GetString() const;
	vstd::optional<JsonObject*> GetObject() const;
	vstd::optional<JsonArray*> GetArray() const;
};
struct JsonKeyPair {
	vstd::string_view key;
	JsonVariant value;
};
class JsonObject {
protected:
	virtual ~JsonObject() {}

public:
	virtual JsonVariant Get(vstd::string_view key) = 0;
	virtual void Set(vstd::string_view key, JsonVariant const& value) = 0;
	virtual vstd::linq::Iterator<JsonKeyPair>* GetIterator() = 0;
};

class JsonArray : public vstd::linq::Iterator<JsonVariant> {
protected:
	virtual ~JsonArray(){};

public:
	virtual size_t Length() = 0;
	virtual JsonVariant Get(size_t index) = 0;
	virtual void Set(size_t index, JsonVariant const& value) = 0;
	virtual void Remove(size_t index) = 0;
	virtual void Add(JsonVariant const& value) = 0;
	virtual vstd::linq::Iterator<JsonVariant>* GetIterator() = 0;
};
}// namespace toolhub::db