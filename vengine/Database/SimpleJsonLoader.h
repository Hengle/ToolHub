#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Network/FunctionSerializer.h>
namespace toolhub::db {
struct JsonObjIDBase {
	uint64 instanceID;
	uint64 dbIndex;
};
template<typename T>
struct JsonObjID : public JsonObjIDBase {

	JsonObjID(T* ptr)
		: JsonObjIDBase{
			ptr->GetInstanceID(),
			ptr->GetDatabase()->GetIndex()} {
	}
};
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
	template<typename... Args>
	SimpleJsonVariant(Args&&... args)
		: value(std::forward<Args>(args)...) {
	}
	SimpleJsonVariant(JsonVariant const& v);
	SimpleJsonVariant(JsonVariant const&& v)
		: SimpleJsonVariant(v) {}
	SimpleJsonVariant(JsonVariant& v)
		: SimpleJsonVariant((JsonVariant const&)v) {}
	SimpleJsonVariant(JsonVariant&& v);
	JsonVariant GetVariant(IJsonDatabase* db) const;
};

template<typename T>
void PushDataToVector(T&& v, vstd::vector<uint8_t>& serData) {
	using TT = std::remove_cvref_t<T>;
	vstd::SerDe<TT>::Set(v, serData);
}

class SimpleJsonLoader {
public:
	static IJsonDict* GetDictFromID(IJsonDatabase* db, JsonObjID<IJsonDict> const& id);
	static IJsonArray* GetArrayFromID(IJsonDatabase* db, JsonObjID<IJsonArray> const& id);
	static bool Check(IJsonDatabase* db, SimpleJsonVariant const& var);
	static SimpleJsonVariant DeSerialize(std::span<uint8_t>& arr, IJsonDatabase* db);
	static void Serialize(IJsonDatabase* db, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data);
}; 
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	using TT = std::remove_cvref_t<T>;
	return vstd::SerDe<TT>::Get(arr);
}

}// namespace toolhub::db