#pragma once
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {

class SimpleJsonArray : public IJsonArray {
public:
	bool isDirty = false;
	vstd::vector<JsonVariant> arrs;
	SimpleJsonLoader loader;
	uint64 instanceID;
	SimpleBinaryJson* db;
	void ExecuteLoad();
	//Array DeSerialize
	void Load(std::span<uint8_t> sp);
	size_t Length() override;
	JsonVariant Get(size_t index) override;
	void Set(size_t index, JsonVariant value) override;
	void Remove(size_t index) override;
	void Add(JsonVariant value) override;
	vstd::unique_ptr<vstd::linq::Iterator<JsonVariant>> GetIterator() override;
	vstd::optional<int64> GetInt(size_t index) override;
	vstd::optional<double> GetFloat(size_t index) override;
	vstd::optional<vstd::string_view> GetString(size_t index) override;
	vstd::optional<IJsonDict*> GetDict(size_t index) override;
	vstd::optional<IJsonArray*> GetArray(size_t index) override;
	bool IsDirty() override { return isDirty; }

	void M_GetSerData(vstd::vector<uint8_t>& data);
	vstd::vector<uint8_t> GetSerData() override {
		vstd::vector<uint8_t> v;
		M_GetSerData(v);
		return v;
	}
	void DeSer(std::span<uint8_t> data) override;
};
}// namespace toolhub::db