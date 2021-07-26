#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/JsonObject.h>
namespace toolhub::db {
class SimpleBinaryJson;
static constexpr uint8_t DICT_TYPE = 0;
static constexpr uint8_t ARRAY_TYPE = 1;
enum class ValueType : uint8_t {
	Int,
	Float,
	String,
	Dict,
	Array
};

ValueType type;
template<typename T>
void PushDataToVector(T&& v, vstd::vector<uint8_t>& serData) {
	using TT = std::remove_cvref_t<T>;
	auto lastLen = serData.size();
	if constexpr (std::is_same_v<TT, vstd::string>) {
		size_t sz = sizeof(uint64) + v.size();
		serData.resize(lastLen + sz);
		*reinterpret_cast<uint64*>(serData.data() + lastLen) = v.size();
		memcpy(serData.data() + lastLen + sizeof(uint64), v.data(), v.size());
	} else {
		constexpr size_t sz = sizeof(TT);
		serData.resize(lastLen + sz);
		*reinterpret_cast<TT*>(serData.data() + lastLen) = v;
	}
}

class SimpleJsonLoader {
public:
	static JsonVariant DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db);
	static void Serialize(JsonVariant const& v, vstd::vector<uint8_t>& data);
};
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	T* ptr = reinterpret_cast<T*>(arr.data());
	arr = std::span<uint8_t>(arr.data() + sizeof(T), arr.size() - sizeof(T));
	return T(std::move(*ptr));
}

}// namespace toolhub::db