#pragma vengine_package vengine_network

#include <Network/ObjectRegister.h>
namespace toolhub::net {
ObjectRegister::ObjectRegister() {
	incrementalID = 0;
}
ObjectRegister::~ObjectRegister() {
}
IRegistObject* ObjectRegister::CreateObjLocally(
	Runnable<IRegistObject*()> const& creater) {
	auto ptr = CreateObj(creater);
	std::lock_guard lck(mtx);
	auto id = ++incrementalID;
	ptr->id = id;
	ptr->createdLocally = true;
	allObjects.ForceEmplace(CombineData(id, true), ptr);
	return ptr;
}
IRegistObject* ObjectRegister::CreateObj(Runnable<IRegistObject*()> const& creater) {
	auto c = creater();
	c->AddDisposeFunc([this](IRegistObject* ptr) {
		DisposeObj(ptr->GetLocalID(), ptr->IsCreatedLocally());
	});
	return c;
}
IRegistObject* ObjectRegister::CreateObjByRemote(
	Runnable<IRegistObject*()> const& creater,
	uint64 remoteID) {
	auto ptr = CreateObj(creater);
	std::lock_guard lck(mtx);
	ptr->id = remoteID;
	ptr->createdLocally = false;
	allObjects.ForceEmplace(CombineData(remoteID, false), ptr);
	return ptr;
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
IRegistObject* ObjectRegister::GetObject(uint64 id) {
	std::lock_guard lck(mtx);
	auto ite = allObjects.Find(id);
	return ite ? ite.Value().get() : nullptr;
}
}// namespace toolhub::net