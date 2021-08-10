#pragma vengine_package vengine_network

#include <Network/IRegistObject.h>
namespace toolhub {
IRegistObject::~IRegistObject() {
	for (auto&& i : disposeFuncs) {
		i(this);
	}
}
IRegistObject::IRegistObject(uint64 id, bool createdLocally)
	: id(id), createdLocally(createdLocally) {
}
void IRegistObject::AddDisposeFunc(
	Runnable<void(IRegistObject*)>&& func) {
	disposeFuncs.emplace_back(std::move(func));
}
}// namespace toolhub