#pragma once
#include <Common/Common.h>
namespace toolhub::db {
class IJsonDict;
class IJsonArray;

class IJsonDataBase {
public:
	virtual IJsonDict* GetRootObject() = 0;
	virtual IJsonDict* CreateJsonObject() = 0;
	virtual IJsonArray* CreateJsonArray() = 0;
	virtual void Dispose(IJsonDict* jsonObj) = 0;
	virtual void Dispose(IJsonArray* jsonArr) = 0;
	virtual void Save() = 0;

	virtual ~IJsonDataBase() {}
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};
}// namespace toolhub::db