#pragma once
#include <Common/Common.h>
namespace toolhub::db {
class JsonObject;
class JsonArray;

class IJsonDataBase {
public:
	virtual JsonObject* GetRootObject() = 0;
	virtual void Dispose(JsonObject* jsonObj) = 0;
	virtual void Dispose(JsonArray* jsonArr) = 0;
	virtual void Save() = 0;

	virtual ~IJsonDataBase() {}
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};
}// namespace toolhub::db