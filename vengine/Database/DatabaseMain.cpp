#pragma vengine_package vengine_database

#include <DatabaseInclude.h>
namespace toolhub::db {
static Database_Impl ddbb;
}

VENGINE_UNITY_EXTERN toolhub::db::Database const* Database_GetFactory() {
	return &toolhub::db::ddbb;
}
