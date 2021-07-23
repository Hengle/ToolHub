#pragma vengine_package vengine_compute

#include <Database/SimpleJsonObject.h>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {
void SimpleJsonObject::Update() {
	version = db->version;
}
SimpleJsonObject::~SimpleJsonObject() {
}
}// namespace toolhub::db