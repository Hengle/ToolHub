#pragma vengine_package vengine_unity

#include <ObjectRegister.h>
namespace toolhub {
static vstd::optional<ObjectRegister> current_register = nullptr;
ObjectRegister* ObjectRegister::GetSingleton() {
	if (!current_register) current_register.New();
	return current_register;
}
void ObjectRegister::DisposeSingleton() {
	current_register.Delete();
}
ObjectRegister::ObjectRegister() {
	incrementalID = 0;
}
ObjectRegister::~ObjectRegister() {
}
void ObjectRegister::CreateObjLocally(
	Runnable<IRegistObject*()> const& creater) {
	auto ptr = CreateObj(creater);
	std::lock_guard lck(mtx);
	allObjects.ForceEmplace(CombineData(++incrementalID, true), ptr);
}
IRegistObject* ObjectRegister::CreateObj(Runnable<IRegistObject*()> const& creater) {
	auto c = creater();
	c->AddDisposeFunc([this](IRegistObject* ptr) {
		DisposeObj(ptr->GetLocalID(), ptr->IsCreatedLocally());
	});
	return nullptr;
}
void ObjectRegister::CreateObjByRemote(
	Runnable<IRegistObject*()> const& creater,
	uint64 remoteID) {
	auto ptr = CreateObj(creater);
	std::lock_guard lck(mtx);
	allObjects.ForceEmplace(CombineData(remoteID, false), ptr);
}
void ObjectRegister::DisposeObj(
	uint64 id,
	bool createLocally) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(CombineData(id, createLocally));
	if (ite) {
		ite.Value().ptr = nullptr;
		allObjects.Remove(ite);
	}
}

IRegistObject* ObjectRegister::GetObject(uint64 id, bool createLocally) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(CombineData(id, createLocally));
	return ite ? ite.Value().get() : nullptr;
}

}// namespace toolhub