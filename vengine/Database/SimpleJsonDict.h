#pragma once
#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleJsonObject.h>
namespace toolhub::db {
class SimpleJsonDict final : public SimpleJsonObject, public IJsonRefDict {
public:
	HashMap<vstd::string, SimpleJsonVariant> vars;
	//Dict Deserialize
	JsonVariant Get(vstd::string_view key) override;
	void Set(vstd::string key, JsonVariant value) override;
	void LoadFromData(std::span<uint8_t> data) override;

	void Remove(vstd::string const& key) override;
	void MarkDirty() override { Update(); }
	vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() override;
	uint64 GetInstanceID() override { return instanceID; }
	size_t Length() override;
	void M_GetSerData(vstd::vector<uint8_t>& data) override;
	IJsonSubDatabase* GetDatabase() override;
	vstd::vector<uint8_t> GetSerData() override {
		vstd::vector<uint8_t> v;
		M_GetSerData(v);
		return v;
	}
	void Clean() override;
	void Reset() override;
	SimpleJsonDict(uint64 instanceID, SimpleBinaryJson* db);
	~SimpleJsonDict();
	void Dispose() override;
	void AfterAdd(IDatabaseEvtVisitor* visitor) override;
	void BeforeRemove(IDatabaseEvtVisitor* visitor) override;
};
}// namespace toolhub::db