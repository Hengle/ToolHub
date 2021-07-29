#pragma vengine_package vengine_database

#include <DatabaseInclude.h>
#include <Common/DynamicLink.h>
namespace toolhub::db {
static Database_Impl ddbb;
}

toolhub::db::Database const* Database_GetFactory() {
	return &toolhub::db::ddbb;
}
VENGINE_LINK_FUNC(Database_GetFactory);
