#include <Common/Common.h>
#include <IJsonObject.h>
namespace toolhub::db {
class SimpleJsonValueDict : public IJsonValueDict {
	JsonVariant Get(vstd::string_view key) override;
	void Set(vstd::string key, JsonVariant value) override;
	void Remove(vstd::string const& key) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() override;
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void Clean() override;
	void Reset() override;
};

class SimpleJsonValueArray : public IJsonValueArray {
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void Clean() override;
	void Reset() override;
	JsonVariant Get(size_t index) override;
	void Set(size_t index, JsonVariant value) override;
	void Remove(size_t index) override;
	void Add(JsonVariant value) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonVariant>> GetIterator() override;
};
}// namespace toolhub::db