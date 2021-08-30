#pragma once
#include <Common/Common.h>
#include <Database/IJsonObject.h>
#include <Database/SimpleJsonLoader.h>
namespace toolhub::db {
struct SimpleJsonKey {
	using ValueType = vstd::variant<int64,
									vstd::string,
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
					case ValueType::IndexOf<vstd::string>:
						new (ptr) vstd::string(v.force_get<vstd::string_view>());
						break;
					case ValueType::IndexOf<vstd::Guid>:
						new (ptr) vstd::Guid(v.force_get<vstd::Guid>());
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
			case ValueType::IndexOf<vstd::Guid>:
				return Key(value.force_get<vstd::Guid>());
			case ValueType::IndexOf<vstd::string>:
				return Key(value.force_get<vstd::string>());
			default:
				return Key();
		}
	}
	bool operator==(SimpleJsonKey const& key) const {
		if (key.value.GetType() != value.GetType()) return false;
		switch (value.GetType()) {
			case ValueType::IndexOf<int64>:
				return value.force_get<int64>() == key.value.force_get<int64>();
			case ValueType::IndexOf<vstd::Guid>:
				return value.force_get<vstd::Guid>() == key.value.force_get<vstd::Guid>();
			case ValueType::IndexOf<vstd::string>:
				return value.force_get<vstd::string>() == key.value.force_get<vstd::string>();
		}
		return true;
	}

	bool EqualToKey(Key const& key) const {
		if (key.GetType() != value.GetType()) return false;
		switch (value.GetType()) {
			case ValueType::IndexOf<int64>:
				return value.force_get<int64>() == key.force_get<int64>();
			case ValueType::IndexOf<vstd::Guid>:
				return value.force_get<vstd::Guid>() == key.force_get<vstd::Guid>();
			case ValueType::IndexOf<vstd::string>:
				return value.force_get<vstd::string>() == key.force_get<vstd::string_view>();
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

			case ValueType::IndexOf<vstd::Guid>:
				return getHash(*reinterpret_cast<vstd::Guid const*>(value.GetPlaceHolder()));

			case ValueType::IndexOf<vstd::string>:
				return getHash(*reinterpret_cast<vstd::string const*>(value.GetPlaceHolder()));
		}
	}

	bool operator!=(SimpleJsonKey const& key) const {
		return !operator==(key);
	}
};
struct SimpleJsonKeyHash {
	template<typename T>
	size_t operator()(T const& key) const {
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, SimpleJsonKey>)
			return key.GetHashCode();
		else {
			auto getHash = [](auto&& v) {
				vstd::hash<std::remove_cvref_t<decltype(v)>> h;
				return h(v);
			};
			switch (key.GetType()) {
				case Key::IndexOf<int64>:
					return getHash(*reinterpret_cast<int64 const*>(key.GetPlaceHolder()));

				case Key::IndexOf<vstd::Guid>:
					return getHash(*reinterpret_cast<vstd::Guid const*>(key.GetPlaceHolder()));

				case Key::IndexOf<vstd::string_view>:
					return getHash(*reinterpret_cast<vstd::string const*>(key.GetPlaceHolder()));
			}
		}
	}
};
struct SimpleJsonKeyEqual {
	template<typename T>
	bool operator()(SimpleJsonKey const& key, T const& t) const {
		if constexpr (std::is_same_v<std::remove_cvref_t<T>, Key>) {
			return key.EqualToKey(t);
		} else {
			return key == t;
		}
	}
};
using KVMap = HashMap<SimpleJsonKey, SimpleJsonVariant, SimpleJsonKeyHash, SimpleJsonKeyEqual>;
class SimpleJsonValue {
protected:
	SimpleBinaryJson* db;
};

class SimpleJsonValueDict final : public IJsonDict, public SimpleJsonValue {

public:
	void Dispose() override;
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
	void LoadFromSer(std::span<uint8_t const>& arr);
	void Reset() override;
	bool IsEmpty() override { return vars.size() == 0; }
	WriteJsonVariant GetAndSet(Key const& key, WriteJsonVariant&& newValue) override;
	WriteJsonVariant GetAndRemove(Key const& key) override;
	void M_Print(vstd::string& str, size_t space);
	vstd::string Print() override {
		vstd::string str;
		M_Print(str, 0);
		return str;
	}
};

class SimpleJsonValueArray final : public IJsonArray, public SimpleJsonValue {

public:
	void Dispose() override;
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
	void LoadFromSer(std::span<uint8_t const>& arr);
	void Reset() override;

	ReadJsonVariant Get(size_t index) override;
	void Set(size_t index, WriteJsonVariant&& value) override;
	void Remove(size_t index) override;
	void Add(WriteJsonVariant&& value) override;
	vstd::unique_ptr<vstd::linq::Iterator<const ReadJsonVariant>> GetIterator() override;
	bool IsEmpty() override { return arr.size() == 0; }
	WriteJsonVariant GetAndSet(size_t index, WriteJsonVariant&& newValue) override;
	WriteJsonVariant GetAndRemove(size_t) override;
	void M_Print(vstd::string& str, size_t space);
	vstd::string Print() override {
		vstd::string str;
		M_Print(str, 0);
		return str;
	}
};
}// namespace toolhub::db