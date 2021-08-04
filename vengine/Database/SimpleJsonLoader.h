#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Network/FunctionSerializer.h>
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

struct SimpleJsonVariant {
	vstd::variant<int64,
				  double,
				  vstd::string,
				  JsonObjID<IJsonDict>,
				  JsonObjID<IJsonArray>>
		value;
	SimpleJsonVariant(JsonVariant const& v) {
		auto func = [&](auto&& v) {
			value = v;
		};
		v.visit(
			func,
			func,
			func,
			func,
			func);
	}
	SimpleJsonVariant(JsonVariant&& v) {
		auto func = [&](auto&& v) {
			value = std::move(v);
		};
		v.visit(
			func,
			func,
			func,
			func,
			func);
	}
	operator JsonVariant() const {
		switch (value.GetType()) {
			case 0:
				return *reinterpret_cast<int64 const*>(value.GetPlaceHolder());
			case 1:
				return *reinterpret_cast<double const*>(value.GetPlaceHolder());
			case 2:
				return *reinterpret_cast<vstd::string const*>(value.GetPlaceHolder());
			case 3:
				return *reinterpret_cast<JsonObjID<IJsonDict> const*>(value.GetPlaceHolder());
			case 4:
				return *reinterpret_cast<JsonObjID<IJsonArray> const*>(value.GetPlaceHolder());
		}
		return JsonVariant();
	}
};

template<typename T>
void PushDataToVector(T&& v, vstd::vector<uint8_t>& serData) {
	using TT = std::remove_cvref_t<T>;
	vstd::SerDe<TT>::Set(v, serData);
}

class SimpleJsonLoader {
public:
	static bool Check(SimpleBinaryJson* db, SimpleJsonVariant const& var);
	static JsonVariant DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db);
	static void Serialize(SimpleBinaryJson* db, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data);
};
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	using TT = std::remove_cvref_t<T>;
	return vstd::SerDe<TT>::Get(arr);
}

}// namespace toolhub::db