#pragma vengine_package vengine_dll
#include <Common/VObject.h>
#include <mutex>

std::atomic<uint64_t> VObject::CurrentID = 0;
namespace vstd::detail {

#define PRINT_SIZE 256

Pool<LinkHeap, VEngine_AllocType::VEngine, true> allocPool(PRINT_SIZE, false);
spin_mutex globalMtx;
}// namespace vstd::detail
VObject::~VObject() noexcept {
	for (auto ite = disposeFuncs.begin(); ite != disposeFuncs.end(); ++ite) {
		(*ite)(this);
	}
}
VObject::VObject() {
	instanceID = ++CurrentID;
}
void VObject::AddEventBeforeDispose(Runnable<void(VObject*)>&& func) noexcept {
	disposeFuncs.emplace_back(std::move(func));
}
void PtrWeakLink::Dispose() noexcept {
	auto a = heapPtr;
	heapPtr = nullptr;
	if (a) {
		a->WeakDestructor();
	}
}
void LinkHeap::Destroy() {
	void* a;
	funcPtr_t<void(void*)> disposer;
	{
		std::unique_lock lck(mtx);
		a = ptr;
		disposer = this->disposer;
		this->refCount = 0;
		ptr = nullptr;
		auto looseRefCount = --this->looseRefCount;
		if (looseRefCount == 0) {
			lck.unlock();
			LinkHeap::ReturnHeap(this);
		}
	}
	if (a) {
		disposer(a);
	}
}
void LinkHeap::Destructor() {
	void* a;
	funcPtr_t<void(void*)> disposer;
	{
		std::unique_lock lck(mtx);
		auto refCount = --this->refCount;
		auto looseRefCount = --this->looseRefCount;
		if (refCount == 0) {
			a = ptr;
			ptr = nullptr;
			disposer = this->disposer;
		} else {
			a = nullptr;
			disposer = nullptr;
		}
		if (looseRefCount == 0) {
			lck.unlock();
			LinkHeap::ReturnHeap(this);
		}
	}
	if (a) {
		disposer(a);
	}
}

void LinkHeap::WeakDestructor() {
	std::unique_lock lck(mtx);
	if (--this->looseRefCount == 0) {
		lck.unlock();
		LinkHeap::ReturnHeap(this);
	}
}

void PtrLink::Destroy() noexcept {
	auto bb = heapPtr;
	heapPtr = nullptr;
	if (bb) {
		bb->Destroy();
	}
}
PtrLink::PtrLink(const PtrLinkBase& p) noexcept {
	offset = p.offset;
	auto func = [&]() {
		if (!p.heapPtr) return false;
		std::lock_guard lck(p.heapPtr->mtx);
		if (p.heapPtr->ptr) {
			++p.heapPtr->refCount;
			++p.heapPtr->looseRefCount;
			return true;
		}
		return false;
	};
	if (func()) {
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}
PtrLink::PtrLink(PtrLinkBase&& p) noexcept {
	offset = p.offset;
	heapPtr = p.heapPtr;
	p.heapPtr = nullptr;
}
void PtrLink::operator=(const PtrLinkBase& p) noexcept {
	if (&p == this) return;
	this->~PtrLink();
	new (this) PtrLink(p);
}
void PtrLink::operator=(PtrLinkBase&& p) noexcept {
	if (&p == this) return;
	this->~PtrLink();
	new (this) PtrLink(std::move(p));
}
void PtrWeakLink::Destroy() noexcept {
	auto bb = heapPtr;
	heapPtr = nullptr;
	if (bb) {
		bb->Destroy();
	}
}
void PtrLink::Dispose() noexcept {
	auto a = heapPtr;
	heapPtr = nullptr;
	if (a) {
		a->Destructor();
	}
}
LinkHeap* LinkHeap::GetHeap(void* obj, funcPtr_t<void(void*)> disp) noexcept {
	LinkHeap* ptr = vstd::detail::allocPool.New_Lock(vstd::detail::globalMtx);
	ptr->ptr = obj;
	ptr->disposer = disp;
	return ptr;
}
void LinkHeap::ReturnHeap(LinkHeap* value) noexcept {
	vstd::detail::allocPool.Delete_Lock(vstd::detail::globalMtx, value);
}
PtrWeakLink::PtrWeakLink(const PtrLinkBase& p) noexcept {
	offset = p.offset;
	auto func = [&]() {
		if (!p.heapPtr) return false;
		std::lock_guard lck(p.heapPtr->mtx);
		if (p.heapPtr->ptr) {
			++p.heapPtr->looseRefCount;
			return true;
		}
		return false;
	};
	if (func()) {
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}
PtrWeakLink::PtrWeakLink(PtrLinkBase&& p) noexcept {
	heapPtr = p.heapPtr;
	offset = p.offset;
	p.heapPtr = nullptr;
}
void PtrWeakLink::operator=(const PtrLinkBase& p) noexcept {
	if (&p == this) return;
	this->~PtrWeakLink();
	new (this) PtrWeakLink(p);
}
void PtrWeakLink::operator=(PtrLinkBase&& p) noexcept {
	if (&p == this) return;
	this->~PtrWeakLink();
	new (this) PtrWeakLink(std::move(p));
}

#undef PRINT_SIZE

void* VObjectClass::M_GetCastPtr(VObjectClass const* ths, VObject* ptr, Type t) {
	if (ths == nullptr)
		return nullptr;
	auto ite = ths->allowCastClass.Find(t);
	if (ite) {
		return ite.Value()(ptr);
	}
	return nullptr;
}

VObjectClass::VObjectClass() {
	baseLevel.store(nullptr, std::memory_order_release);
}
VObjectClass::VObjectClass(Type type, funcPtr_t<void*(VObject*)> func) {
	baseLevel.store(nullptr, std::memory_order_release);
	allowCastClass.Emplace(type, func);
}
VObjectClass::~VObjectClass() {
}

VObjectClass* VObjectClass::SetBase(VObjectClass const* base) {
	auto lastBase = this->baseLevel.exchange(base, std::memory_order_acq_rel);
	//Shall only executed once
	if (lastBase != base) {
		for (auto&& i : base->allowCastClass) {
			std::lock_guard lck(setBaseLock);
			allowCastClass.Emplace(i.first, i.second);
		}
	}
	return this;
}
