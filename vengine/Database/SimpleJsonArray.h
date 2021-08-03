#pragma once
#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleJsonObject.h>
namespace toolhub::db {

class SimpleJsonArray final : public SimpleJsonObject, public IJsonArray {
public:
	vstd::vector<JsonVariant> arrs;
	size_t Length() override;
	JsonVariant Get(size_t index) override;
	void LoadFromData(std::span<uint8_t> data) override;
	void Set(size_t index, JsonVariant value) override;
	void Remove(size_t index) override;
	void Add(JsonVariant value) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> GetIterator() override;
	vstd::optional<int64> GetInt(size_t index) override;
	vstd::optional<double> GetFloat(size_t index) override;
	vstd::optional<vstd::string_view> GetString(size_t index) override;
	vstd::optional<IJsonDict*> GetDict(size_t index) override;
	vstd::optional<IJsonArray*> GetArray(size_t index) override;
	uint64 GetInstanceID() override { return instanceID; }
	IJsonDataBase* GetDatabase() override;
	void M_GetSerData(vstd::vector<uint8_t>& data) override;
	void Clean() override;
	vstd::vector<uint8_t> GetSerData() override {
		vstd::vector<uint8_t> v;
		M_GetSerData(v);
		return v;
	}
	SimpleJsonArray(uint64 instanceID, SimpleBinaryJson* db);
	~SimpleJsonArray();
	void Dispose() override;
	void AfterAdd(IDatabaseEvtVisitor* visitor) override;
	void BeforeRemove(IDatabaseEvtVisitor* visitor) override;
};
}// namespace toolhub::db