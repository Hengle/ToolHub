#pragma vengine_package vengine_database

#include <Database/SimpleJsonObject.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
SimpleJsonObject::SimpleJsonObject(
	vstd::Guid const& guid,
	SimpleBinaryJson* db)
	: selfGuid(guid),
	  db(db) {
}
void SimpleJsonObject::Update() {
	db->MarkDirty(this);
}
}// namespace toolhub::db