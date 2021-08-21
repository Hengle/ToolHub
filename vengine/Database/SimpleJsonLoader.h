#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Network/FunctionSerializer.h>
namespace toolhub::db {
class SimpleBinaryJson;
class SimpleJsonValueArray;
class SimpleJsonValueDict;
static constexpr uint8_t DICT_TYPE = 0;
static constexpr uint8_t ARRAY_TYPE = 1;
enum class ValueType : uint8_t {
	Int,
	Float,
	String,
	ValueDict,
	ValueArray,
	GUID
};

struct SimpleJsonVariant {
	WriteJsonVariant value;
	template<typename... Args>
	SimpleJsonVariant(Args&&... args)
		: value(std::forward<Args>(args)...) {
	}
	SimpleJsonVariant(SimpleJsonVariant const& v);
	SimpleJsonVariant(SimpleJsonVariant& v) : SimpleJsonVariant((SimpleJsonVariant const&)v) {}
	SimpleJsonVariant(SimpleJsonVariant&& v) : value(std::move(v.value)) {}
	SimpleJsonVariant(SimpleJsonVariant const&& v) = delete;

	SimpleJsonVariant(ReadJsonVariant const& v);
	SimpleJsonVariant( ReadJsonVariant& v) : SimpleJsonVariant((ReadJsonVariant const&)v) {}
	SimpleJsonVariant( ReadJsonVariant&& v) : SimpleJsonVariant((ReadJsonVariant const&)v) {}


	ReadJsonVariant GetVariant() const;
	template<typename... Args>
	void Set(Args&&... args) {
		this->~SimpleJsonVariant();
		new (this) SimpleJsonVariant(std::forward<Args>(args)...);
	}
	template<typename A>
	SimpleJsonVariant& operator=(A&& a) {
		this->~SimpleJsonVariant();
		new (this) SimpleJsonVariant(std::forward<A>(a));
		return *this;
	}
};

template<typename T>
void PushDataToVector(T&& v, vstd::vector<uint8_t>& serData) {
	using TT = std::remove_cvref_t<T>;
	vstd::SerDe<TT>::Set(v, serData);
}

class SimpleJsonLoader {
public:
	static bool Check(IJsonDatabase* db, SimpleJsonVariant const& var);
	static SimpleJsonVariant DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db);
	static void Serialize(SimpleJsonVariant const& v, vstd::vector<uint8_t>& data);
};
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	using TT = std::remove_cvref_t<T>;
	return vstd::SerDe<TT>::Get(arr);
}

}// namespace toolhub::db