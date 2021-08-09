#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Network/FunctionSerializer.h>
#include <Utility/ObjectTracker.h>
namespace toolhub::db {
class SimpleBinaryJson;
class SimpleJsonObject;
class SimpleJsonValueArray;
class SimpleJsonValueDict;
static constexpr uint8_t DICT_TYPE = 0;
static constexpr uint8_t ARRAY_TYPE = 1;
enum class ValueType : uint8_t {
	Int,
	Float,
	String,
	Dict,
	Array,
	ValueDict,
	ValueArray
};

struct SimpleJsonVariant {
	vstd::variant<int64,
				  double,
				  vstd::string,
				  vstd::ObjectTrackFlag<IJsonRefDict>,
				  vstd::ObjectTrackFlag<IJsonRefArray>,
				  vstd::unique_ptr<SimpleJsonValueDict>,
				  vstd::unique_ptr<SimpleJsonValueArray>>
		value;
	template<typename... Args>
	SimpleJsonVariant(Args&&... args)
		: value(std::forward<Args>(args)...) {
	}
	SimpleJsonVariant(SimpleJsonVariant const& v) = delete;
	SimpleJsonVariant(SimpleJsonVariant& v) = delete;
	SimpleJsonVariant(SimpleJsonVariant&& v) : value(std::move(v.value)) {}
	SimpleJsonVariant(SimpleJsonVariant const&& v) = delete;

	SimpleJsonVariant(SimpleBinaryJson* db, JsonVariant const& v, SimpleJsonObject* obj);
	SimpleJsonVariant(SimpleBinaryJson* db, JsonVariant& v, SimpleJsonObject* obj) : SimpleJsonVariant(db, (JsonVariant const&)v, obj) {}
	JsonVariant GetVariant() const;
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
	static IJsonRefDict* GetDictFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID);
	static IJsonRefArray* GetArrayFromID(IJsonDatabase* db, uint64 dbIndex, uint64 instanceID);
	static bool Check(IJsonDatabase* db, SimpleJsonVariant const& var);
	static void Clean(IJsonDatabase* db, HashMap<vstd::string, SimpleJsonVariant>& var);
	static void Clean(IJsonDatabase* db, vstd::vector<SimpleJsonVariant>& var);
	static SimpleJsonVariant DeSerialize(std::span<uint8_t>& arr, SimpleBinaryJson* db, SimpleJsonObject* obj);
	static void Serialize(IJsonDatabase* db, SimpleJsonVariant const& v, vstd::vector<uint8_t>& data);
};
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	using TT = std::remove_cvref_t<T>;
	return vstd::SerDe<TT>::Get(arr);
}

}// namespace toolhub::db