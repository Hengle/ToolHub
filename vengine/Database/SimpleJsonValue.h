#pragma once
#include <Common/Common.h>
#include <Database/IJsonObject.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
class SimpleBinaryJson;
class SimpleJsonObject;
class SimpleJsonValueBase {
protected:
	SimpleBinaryJson* db;
	SimpleJsonObject* parent;
	SimpleJsonValueBase(
		SimpleBinaryJson* db,
		SimpleJsonObject* parent)
		: db(db), parent(parent) {}
};
class SimpleJsonValueDict final : public IJsonValueDict, public SimpleJsonValueBase {
public:
	HashMap<vstd::string, SimpleJsonVariant> vars;
	SimpleJsonValueDict(
		SimpleBinaryJson* db,
		SimpleJsonObject* parent);
	~SimpleJsonValueDict();
	SimpleJsonValueDict(
		SimpleBinaryJson* db,
		SimpleJsonObject* parent,
		IJsonValueDict* src);
	JsonVariant Get(vstd::string_view key) override;
	void Set(vstd::string key, JsonVariant value) override;
	void Remove(vstd::string const& key) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() override;
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void M_GetSerData(vstd::vector<uint8_t>& arr);
	void LoadFromSer(std::span<uint8_t>& arr);
	void Clean() override;
	void Reset() override;
	void Dispose() override;
	IJsonValueDict* AddOrGetDict(vstd::string key) override;
	IJsonValueArray* AddOrGetArray(vstd::string key) override;
};

class SimpleJsonValueArray final : public IJsonValueArray, public SimpleJsonValueBase {
public:
	vstd::vector<SimpleJsonVariant> arr;

	SimpleJsonValueArray(
		SimpleBinaryJson* db,
		SimpleJsonObject* parent);
	~SimpleJsonValueArray();
	SimpleJsonValueArray(
		SimpleBinaryJson* db,
		SimpleJsonObject* parent,
		IJsonValueArray* src);
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void M_GetSerData(vstd::vector<uint8_t>& result);
	void LoadFromSer(std::span<uint8_t>& arr);
	void Clean() override;
	void Reset() override;
	JsonVariant Get(size_t index) override;
	void Set(size_t index, JsonVariant value) override;
	void Remove(size_t index) override;
	void Add(JsonVariant value) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> GetIterator() override;
	void Dispose() override;
	IJsonValueDict* AddDict() override;
	IJsonValueArray* AddArray() override;
};
}// namespace toolhub::db