#pragma vengine_package vengine_network

#include <Network/ObjectRegister.h>
#include <Network/IRegistObject.h>
namespace toolhub::net {
ObjectRegister::ObjectRegister() {
	incrementalID = 0;
}
ObjectRegister::~ObjectRegister() {
}
IRegistObject* ObjectRegister::CreateObjLocally(
	Runnable<IRegistObject*()> const& creater,
	bool msgIsFromServer) {
	auto CombineData = [&](uint64 id) {
		return (id << 1) | (msgIsFromServer ? 1 : 0);
	};
	auto ptr = CreateObj(creater);
	auto id = ++incrementalID;
	ptr->id = CombineData(id);
	std::lock_guard lck(mtx);
	allObjects.ForceEmplace(ptr->id, ptr);
	return ptr;
}
IRegistObject* ObjectRegister::CreateObj(Runnable<IRegistObject*()> const& creater) {
	auto c = creater();
	c->AddDisposeFunc([this](IRegistObject* ptr) {
		DisposeObj(ptr->GetLocalID());
	});
	return c;
}
IRegistObject* ObjectRegister::CreateObjByRemote(
	Runnable<IRegistObject*()> const& creater,
	uint64 remoteID) {
	auto ptr = CreateObj(creater);
	std::lock_guard lck(mtx);
	ptr->id = remoteID;
	allObjects.ForceEmplace(remoteID, ptr);
	return ptr;
}
void ObjectRegister::DisposeObj(
	uint64 id) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(id);
	if (ite) {
		ite.Value().ptr = nullptr;
		allObjects.Remove(ite);
	}
}

IRegistObject* ObjectRegister::GetObject(uint64 id) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(id);
	return ite ? ite.Value().get() : nullptr;
}
}// namespace toolhub::net