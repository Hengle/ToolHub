#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/JsonObject.h>
namespace toolhub::db {
class SimpleBinaryJson;
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
	using TT = std::remove_cvref_t<decltype(v)>;
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
	std::atomic_bool loaded;
	std::mutex mtx;

public:
	std::span<uint8_t> dataChunk;
	//TODO
	SimpleJsonLoader();
	void Reset();
	template<typename T>
	void Load(T&& t) {
		if (loaded.load(std::memory_order_acquire)) return;
		std::lock_guard lck(mtx);
		if (loaded.load(std::memory_order_acquire)) return;
		if (!dataChunk.empty())
			t.Load(dataChunk);
		loaded.store(true, std::memory_order_release);
	}
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