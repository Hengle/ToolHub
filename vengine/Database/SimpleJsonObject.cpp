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
	if (isDirty) return;
	isDirty = true;
	db->MarkDirty(this);
}
void SimpleJsonObject::Reset() {
	isDirty = false;
}
}// namespace toolhub::db