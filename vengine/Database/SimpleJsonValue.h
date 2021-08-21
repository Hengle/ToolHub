#pragma once
#include <Common/Common.h>
#include <Database/IJsonObject.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
struct SimpleJsonKey {
	using ValueType = vstd::variant<int64,
									double,
									vstd::string,
									vstd::vector<uint8_t>,
									vstd::Guid>;

	ValueType value;
	SimpleJsonKey(ValueType const& value)
		: value(value) {}
	SimpleJsonKey(ValueType&& value)
		: value(std::move(value)) {}
	SimpleJsonKey(Key const& v) {
		if (v.GetType() < ValueType::argSize) {
			value.update(v.GetType(), [&](void* ptr) {
				switch (v.GetType()) {
					case ValueType::IndexOf<int64>:
						new (ptr) int64(v.force_get<int64>());
						break;
					case ValueType::IndexOf<double>:
						new (ptr) double(v.force_get<double>());
						break;
					case ValueType::IndexOf<vstd::Guid>:
						new (ptr) vstd::Guid(v.force_get<vstd::Guid>());
						break;
					case ValueType::IndexOf<vstd::string>:
						new (ptr) vstd::string(v.force_get<vstd::string_view>());
						break;
					case ValueType::IndexOf<vstd::vector<uint8_t>>:
						new (ptr) vstd::vector<uint8_t>(v.force_get<std::span<uint8_t>>());
						break;
				}
			});
		}
	}
	SimpleJsonKey(Key&& v)
		: SimpleJsonKey(v) {}
	Key GetKey() const {
		switch (value.GetType()) {
			case ValueType::IndexOf<int64>:
				return Key(value.force_get<int64>());
			case ValueType::IndexOf<double>:
				return Key(value.force_get<double>());
			case ValueType::IndexOf<vstd::Guid>:
				return Key(value.force_get<vstd::Guid>());
			case ValueType::IndexOf<vstd::string>:
				return Key(value.force_get<vstd::string>());
			case ValueType::IndexOf<vstd::vector<uint8_t>>: {
				return Key(static_cast<std::span<uint8_t>>(value.force_get<vstd::vector<uint8_t>>()));
			}
			default:
				return Key();
		}
	}
	bool operator==(SimpleJsonKey const& key) const {
		if (key.value.GetType() != value.GetType()) return false;
		switch (value.GetType()) {
			case ValueType::IndexOf<int64>:
				return value.force_get<int64>() == key.value.force_get<int64>();
			case ValueType::IndexOf<double>:
				return value.force_get<double>() == key.value.force_get<double>();
			case ValueType::IndexOf<vstd::Guid>:
				return value.force_get<vstd::Guid>() == key.value.force_get<vstd::Guid>();
			case ValueType::IndexOf<vstd::string>:
				return value.force_get<vstd::string>() == key.value.force_get<vstd::string>();
			case ValueType::IndexOf<vstd::vector<uint8_t>>: {
				auto&& aVec = value.force_get<vstd::vector<uint8_t>>();
				auto&& bVec = key.value.force_get<vstd::vector<uint8_t>>();
				if (aVec.size() != bVec.size()) return false;
				return memcmp(aVec.data(), bVec.data(), aVec.size()) == 0;
			}
		}
		return true;
	}
	size_t GetHashCode() const {
		auto getHash = [](auto&& v) {
			vstd::hash<std::remove_cvref_t<decltype(v)>> h;
			return h(v);
		};
		switch (value.GetType()) {
			case ValueType::IndexOf<int64>:
				return getHash(*reinterpret_cast<int64 const*>(value.GetPlaceHolder()));

			case ValueType::IndexOf<double>:
				return getHash(*reinterpret_cast<int64 const*>(value.GetPlaceHolder()));

			case ValueType::IndexOf<vstd::Guid>:
				return getHash(*reinterpret_cast<vstd::Guid const*>(value.GetPlaceHolder()));

			case ValueType::IndexOf<vstd::string>:
				return getHash(*reinterpret_cast<vstd::string const*>(value.GetPlaceHolder()));

			case ValueType::IndexOf<vstd::vector<uint8_t>>: {
				auto aVec = *reinterpret_cast<vstd::vector<uint8_t> const*>(value.GetPlaceHolder());
				return Hash::CharArrayHash(reinterpret_cast<char const*>(aVec.data()), aVec.size());
			}
		}
	}
	bool operator!=(SimpleJsonKey const& key) const {
		return !operator==(key);
	}
};
struct SimpleJsonKeyHash {
	size_t operator()(SimpleJsonKey const& key) const {
		return key.GetHashCode();
	}
};
using KVMap = HashMap<SimpleJsonKey, SimpleJsonVariant, SimpleJsonKeyHash>;
class SimpleJsonValue {
protected:
	SimpleBinaryJson* db;
};

class SimpleJsonValueDict final : public IJsonDict, public SimpleJsonValue {
	void Dispose() override;

public:
	SimpleBinaryJson* GetDB() const { return db; }
	KVMap vars;
	SimpleJsonValueDict(SimpleBinaryJson* db);
	~SimpleJsonValueDict();
	SimpleJsonValueDict(
		SimpleBinaryJson* db,
		IJsonDict* src);
	ReadJsonVariant Get(Key const& key) override;
	void Set(Key const& key, WriteJsonVariant&& value) override;
	void Remove(Key const& key) override;
	vstd::unique_ptr<vstd::linq::Iterator<const JsonKeyPair>> GetIterator() override;
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void M_GetSerData(vstd::vector<uint8_t>& arr);
	void LoadFromSer(std::span<uint8_t>& arr);
	void Reset() override;
	bool IsEmpty() override { return vars.size() == 0; }
	WriteJsonVariant GetAndSet(Key const& key, WriteJsonVariant&& newValue) override;
	WriteJsonVariant GetAndRemove(Key const& key) override;
};

class SimpleJsonValueArray final : public IJsonArray, public SimpleJsonValue {
	void Dispose() override;

public:
	SimpleBinaryJson* GetDB() const { return db; }
	vstd::vector<SimpleJsonVariant> arr;
	SimpleJsonValueArray(SimpleBinaryJson* db);
	~SimpleJsonValueArray();
	SimpleJsonValueArray(
		SimpleBinaryJson* db,
		IJsonArray* src);
	size_t Length() override;
	vstd::vector<uint8_t> GetSerData() override;
	void M_GetSerData(vstd::vector<uint8_t>& result);
	void LoadFromSer(std::span<uint8_t>& arr);
	void Reset() override;

	ReadJsonVariant Get(size_t index) override;
	void Set(size_t index, WriteJsonVariant&& value) override;
	void Remove(size_t index) override;
	void Add(WriteJsonVariant&& value) override;
	vstd::unique_ptr<vstd::linq::Iterator<const ReadJsonVariant>> GetIterator() override;
	bool IsEmpty() override { return arr.size() == 0; }
};
}// namespace toolhub::db