#pragma vengine_package vengine_database
#include <Database/SimpleBinaryJson.h>
#include <Database/DatabaseInclude.h>
namespace toolhub::db {
SimpleBinaryJson::SimpleBinaryJson()
	: arrValuePool(256, false), dictValuePool(256, false) {
	root.New(this);
}
vstd::vector<uint8_t> SimpleBinaryJson::Serialize() {
	vstd::vector<uint8_t> data;
	root->M_GetSerData(data);
	return data;
}
void SimpleBinaryJson::Read(
	std::span<uint8_t const> data) {
	root->Reset();
	root->LoadFromSer(data);
}
vstd::string SimpleBinaryJson::Print() {
	return root->Print();
}
IJsonDict* SimpleBinaryJson::GetRootNode() {
	return root;
}
SimpleBinaryJson ::~SimpleBinaryJson() {
	root.Delete();
}
vstd::unique_ptr<IJsonDict> SimpleBinaryJson::CreateDict() {
	return dictValuePool.New(this);
}
vstd::unique_ptr<IJsonArray> SimpleBinaryJson::CreateArray() {
	return arrValuePool.New(this);
}
SimpleJsonValueDict* SimpleBinaryJson::CreateDict_Nake() {
	return dictValuePool.New(this);
}
SimpleJsonValueArray* SimpleBinaryJson::CreateArray_Nake() {
	return arrValuePool.New(this);
}
IJsonDatabase* Database_Impl::CreateDatabase() const {
	return new SimpleBinaryJson();
}
static vstd::optional<Database_Impl> database_Impl;
VENGINE_UNITY_EXTERN toolhub::db::Database const* Database_GetFactory() {
	database_Impl.New();
	return database_Impl;
}
}// namespace toolhub::db