#pragma vengine_package vengine_compute

#include <Database/SimpleJsonObject.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
SimpleJsonObject::SimpleJsonObject(
	uint64 instanceID,
	SimpleBinaryJson* db)
	: instanceID(instanceID),
	  db(db) {
}
void SimpleJsonObject::Update() {
	db->MarkDirty(this);
}
}// namespace toolhub::db