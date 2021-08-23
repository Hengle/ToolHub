#pragma once
#include <Common/linq.h>
#include <Utility/VGuid.h>
#include <Database/IJsonDatabase.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;

using ReadJsonVariant = vstd::variant<int64,
									  double,
									  vstd::string_view,
									  IJsonDict*,
									  IJsonArray*,
									  vstd::Guid>;
using WriteJsonVariant = vstd::variant<int64,
									   double,
									   vstd::string,
									   vstd::unique_ptr<IJsonDict>,
									   vstd::unique_ptr<IJsonArray>,
									   vstd::Guid>;
using Key = vstd::variant<int64,
						  vstd::string_view,
						  vstd::Guid>;
struct JsonKeyPair {
	Key key;
	ReadJsonVariant value;
	JsonKeyPair(
		Key&& key,
		ReadJsonVariant&& value) : key(std::move(key)), value(std::move(value)) {}
};

class IJsonObject : protected vstd::IDisposable {

protected:
	~IJsonObject() = default;

public:
	virtual size_t Length() = 0;
	virtual vstd::vector<uint8_t> GetSerData() = 0;
	virtual void Reset() = 0;
	virtual bool IsEmpty() = 0;
};

class IJsonDict : public IJsonObject {
	friend class vstd::unique_ptr<IJsonDict>;

protected:
	~IJsonDict() {}

public:
	virtual ReadJsonVariant Get(Key const& key) = 0;
	virtual void Set(Key const& key, WriteJsonVariant&& value) = 0;
	virtual void Remove(Key const& key) = 0;
	virtual WriteJsonVariant GetAndSet(Key const& key, WriteJsonVariant&& newValue) = 0;
	virtual WriteJsonVariant GetAndRemove(Key const& key) = 0;

	virtual vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() = 0;
};

class IJsonArray : public IJsonObject {
	friend class vstd::unique_ptr<IJsonArray>;

protected:
	~IJsonArray(){};

public:
	virtual ReadJsonVariant Get(size_t index) = 0;
	virtual void Set(size_t index, WriteJsonVariant&& value) = 0;
	virtual void Remove(size_t index) = 0;
	virtual void Add(WriteJsonVariant&& value) = 0;
	virtual WriteJsonVariant GetAndSet(size_t index, WriteJsonVariant&& newValue) = 0;
	virtual WriteJsonVariant GetAndRemove(size_t) = 0;
	virtual vstd::unique_ptr<vstd::linq::Iterator<const ReadJsonVariant>> GetIterator() = 0;
};

}// namespace toolhub::db