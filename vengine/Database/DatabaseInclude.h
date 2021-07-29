#pragma once
#include <Common/Common.h>
namespace toolhub::db {
class IJsonDataBase;
class Database {
public:
	virtual IJsonDataBase* CreateSimpleJsonDB() const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT
class Database_Impl final : public Database {
public:
	IJsonDataBase* CreateSimpleJsonDB() const override;
};
#endif
}// namespace toolhub::db