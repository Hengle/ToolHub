#pragma vengine_package vengine_network
#include <Common/AllocateOverrider.h>

#include <Network/ObjectRegister.h>
#include <Network/IRegistObject.h>
namespace toolhub::net {
ObjectRegister::ObjectRegister() {
}
ObjectRegister::~ObjectRegister() {
}
IRegistObject* ObjectRegister::CreateObjLocally(
	Runnable<IRegistObject*()> const& creater) {

	vstd::Guid newGuid(true);
	auto ptr = CreateObj(creater, newGuid);
	std::lock_guard lck(mtx);
	allObjects.ForceEmplace(newGuid, ptr);
	return ptr;
}
IRegistObject* ObjectRegister::CreateObj(Runnable<IRegistObject*()> const& creater, vstd::Guid const& newGuid) {
	auto c = creater();
	c->guid = newGuid;
	c->AddDisposeFunc([this](IRegistObject* ptr) {
		DisposeObj(ptr->GetGUID());
	});
	return c;
}
IRegistObject* ObjectRegister::CreateObjByRemote(
	Runnable<IRegistObject*()> const& creater,
	vstd::Guid const& remoteID) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(remoteID);
	if (ite) return ite.Value().get();
	auto ptr = CreateObj(creater, remoteID);
	allObjects.ForceEmplace(remoteID, ptr);
	return ptr;
}
void ObjectRegister::DisposeObj(
	vstd::Guid const& id) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(id);
	if (ite) {
		ite.Value().ptr = nullptr;
		allObjects.Remove(ite);
	}
}

IRegistObject* ObjectRegister::GetObject(vstd::Guid const& id) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(id);
	return ite ? ite.Value().get() : nullptr;
}
static vstd::optional<ObjectRegister> objregister_singleton;
ObjectRegister* ObjectRegister::GetSingleton() {
	if (!objregister_singleton) objregister_singleton.New();
	return objregister_singleton;
}
void ObjectRegister::DisposeSingleton() {
	objregister_singleton.Delete();
}
}// namespace toolhub::net