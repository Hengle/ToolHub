#pragma vengine_package vengine_compute

#include <Database/JsonObject.h>
namespace toolhub::db {
JsonVariant::JsonVariant()
	: type(ValueType::None) {
}
JsonVariant::JsonVariant(int64 intValue)
	: type(ValueType::Int),
	  intValue(intValue) {}
JsonVariant::JsonVariant(double floatValue)
	: type(ValueType::Float),
	  floatValue(floatValue) {
}
JsonVariant::JsonVariant(bool boolValue)
	: type(ValueType::Bool),
	  boolValue(boolValue) {
}
JsonVariant::JsonVariant(vstd::string_view strValue)
	: type(ValueType::String),
	  strValue(strValue) {
}
JsonVariant::JsonVariant(JsonObject* jsonObj)
	: type(ValueType::JsonObject),
	  jsonObj(jsonObj) {
}
JsonVariant::JsonVariant(JsonArray* jsonArr)
	: type(ValueType::JsonArray),
	  jsonArr(jsonArr) {
}
vstd::optional<int64> JsonVariant::GetInt() const {
	switch (type) {
		case ValueType::Int:
			return intValue;
		case ValueType::Float:
			return floatValue;
		case ValueType::Bool:
			return boolValue ? 1ll : 0ll;
	}
	return vstd::optional<int64>();
}
vstd::optional<double> JsonVariant::GetFloat() const {
	switch (type) {
		case ValueType::Int:
			return intValue;
		case ValueType::Float:
			return floatValue;
		case ValueType::Bool:
			return boolValue ? 1ll : 0ll;
	}
	return vstd::optional<double>();
}
vstd::optional<bool> JsonVariant::GetBool() const {
	switch (type) {
		case ValueType::Int:
			return intValue != 0 ? true : false;
		case ValueType::Float:
			return floatValue != 0 ? true : false;
		case ValueType::Bool:
			return boolValue;
	}
	return vstd::optional<bool>();
}
vstd::optional<vstd::string_view> JsonVariant::GetString() const {
	if (type == ValueType::String)
		return strValue;
	return vstd::optional<vstd::string_view>();
}
vstd::optional<JsonObject*> JsonVariant::GetObject() const {
	if (type == ValueType::JsonObject)
		return jsonObj;
	return vstd::optional<JsonObject*>();
}
vstd::optional<JsonArray*> JsonVariant::GetArray() const {
	if (type == ValueType::JsonArray)
		return jsonArr;
	return vstd::optional<JsonArray*>();
}
}// namespace toolhub::db
