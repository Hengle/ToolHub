#pragma once
#include <Database/SimpleJsonLoader.h>
#include <Database/SimpleJsonObject.h>
namespace toolhub::db {
class SimpleJsonDict : public IJsonDict {
public:
	HashMap<vstd::string, JsonVariant> vars;
	SimpleJsonObject jsonObj;
	//Dict Deserialize
	void Load(std::span<uint8_t> sp);
	JsonVariant Get(vstd::string_view key) override;
	void Set(vstd::string key, JsonVariant value) override;
	void Remove(vstd::string const& key) override;
	vstd::unique_ptr<vstd::linq::Iterator<JsonKeyPair>> GetIterator() override;
	vstd::optional<int64> GetInt(vstd::string_view key) override;
	vstd::optional<double> GetFloat(vstd::string_view key) override;
	vstd::optional<vstd::string_view> GetString(vstd::string_view key) override;
	vstd::optional<IJsonDict*> GetDict(vstd::string_view key) override;
	vstd::optional<IJsonArray*> GetArray(vstd::string_view key) override;
	size_t Length() override;
	void M_GetSerData(vstd::vector<uint8_t>& data);
	vstd::vector<uint8_t> GetSerData() override {
		vstd::vector<uint8_t> v;
		M_GetSerData(v);
		return v;
	}
};
}// namespace toolhub::db