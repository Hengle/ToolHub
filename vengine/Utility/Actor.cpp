#pragma vengine_package vengine_dll
#include <Utility/Actor.h>
ActorPointer::~ActorPointer() {
	disposer(ptr);
}
void* Actor::GetComponent(Type t) const {
	std::lock_guard lck(mtx);
	auto ite = hash.Find(t);
	if (ite) return ite.Value().ptr;
	return nullptr;
}
void ActorPointer::operator=(ActorPointer const& p) {
	disposer(ptr);
	ptr = p.ptr;
	disposer = p.disposer;
}
void Actor::RemoveComponent(Type t) {
	std::lock_guard lck(mtx);
	hash.Remove(t);
}
void Actor::SetComponent(Type t, void* ptr, void (*disposer)(void*)) {
	std::lock_guard lck(mtx);
	auto&& ite = hash.Emplace(t).Value();
	ite.ptr = ptr;
	ite.disposer = disposer;
}
Actor::Actor() {}
Actor::Actor(uint32_t initComponentCapacity) : hash(initComponentCapacity) {}
Actor::~Actor() {}

void* ActorSingleThread::GetComponent(Type t) const {
	auto ite = hash.Find(t);
	if (ite) return ite.Value().ptr;
	return nullptr;
}
void ActorSingleThread::RemoveComponent(Type t) {
	hash.Remove(t);
}
void ActorSingleThread::SetComponent(Type t, void* ptr, void (*disposer)(void*)) {
	auto&& ite = hash.Emplace(t).Value();
	ite.ptr = ptr;
	ite.disposer = disposer;
}
ActorSingleThread::ActorSingleThread() {}
ActorSingleThread::ActorSingleThread(uint32_t initComponentCapacity) : hash(initComponentCapacity) {}
ActorSingleThread::~ActorSingleThread() {}