#pragma once
#include <Common/Common.h>
// Entry:
// toolhub::db::Database const* Database_GetFactory()
namespace toolhub::db {
class IJsonDatabase;
class Database {
public:
	virtual IJsonDatabase* CreateDatabase() const = 0;
};
#ifdef VENGINE_DATABASE_PROJECT
class Database_Impl final : public Database {
public:
	IJsonDatabase* CreateDatabase() const override;
};
#endif
}// namespace toolhub::db