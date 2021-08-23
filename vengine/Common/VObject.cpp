#pragma vengine_package vengine_dll
#include <Common/VObject.h>
#include <mutex>
std::atomic<uint64_t> VObject::CurrentID = 0;
ArrayList<LinkHeap*, VEngine_AllocType::Default> LinkHeap::heapPtrs;
spin_mutex LinkHeap::mtx;
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
	if (a && (--a->looseRefCount <= 0)) {
		LinkHeap::ReturnHeap(a);
	}
}
void PtrLink::Destroy() noexcept {
	auto bb = heapPtr;
	heapPtr = nullptr;
	if (bb) {
		auto a = bb->ptr.exchange(nullptr, std::memory_order_acq_rel);
		auto func = bb->disposer;
		bb->disposer = nullptr;
		if (a) {
			func(a);
		}
		auto looseRefCount = --bb->looseRefCount;
		if (looseRefCount <= 0)
			LinkHeap::ReturnHeap(bb);
	}
}
PtrLink::PtrLink(const PtrLink& p) noexcept : offset(p.offset) {
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->refCount;
		++p.heapPtr->looseRefCount;
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}
PtrLink::PtrLink(PtrLink&& p) noexcept
	: offset(p.offset),
	  heapPtr(p.heapPtr) {
	p.heapPtr = nullptr;
}
void PtrLink::operator=(const PtrLink& p) noexcept {
	if (&p == this) return;
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->refCount;
		++p.heapPtr->looseRefCount;
		Dispose();
		heapPtr = p.heapPtr;
	} else {
		Dispose();
	}
	offset = p.offset;
}
void PtrLink::operator=(PtrLink&& p) noexcept {
	if (&p == this) return;
	Dispose();
	heapPtr = p.heapPtr;
	offset = p.offset;
	p.heapPtr = nullptr;
}
void PtrWeakLink::Destroy() noexcept {
	auto bb = heapPtr;
	heapPtr = nullptr;
	if (bb) {
		auto a = bb->ptr.exchange(nullptr, std::memory_order_acq_rel);
		auto func = bb->disposer;
		bb->disposer = nullptr;
		if (a) {
			func(a);
		}
		auto looseRefCount = --bb->looseRefCount;
		if (looseRefCount <= 0)
			LinkHeap::ReturnHeap(bb);
	}
}
void PtrLink::Dispose() noexcept {
	auto a = heapPtr;
	heapPtr = nullptr;
	if (a) {
		auto refCount = --a->refCount;
		auto looseRefCount = --a->looseRefCount;
		if (refCount <= 0) {
			auto bb = a->ptr.exchange(nullptr, std::memory_order_acq_rel);
			auto func = a->disposer;
			a->disposer = nullptr;
			if (bb) {
				func(bb);
			}
		}
		if (looseRefCount <= 0)
			LinkHeap::ReturnHeap(a);
	}
}
#define PRINT_SIZE 512
void LinkHeap::Resize() noexcept {
	if (heapPtrs.empty()) {
		LinkHeap* ptrs = (LinkHeap*)malloc(sizeof(LinkHeap) * PRINT_SIZE);
		heapPtrs.resize(PRINT_SIZE);
		for (uint32_t i = 0; i < PRINT_SIZE; ++i) {
			heapPtrs[i] = ptrs + i;
		}
	}
}
LinkHeap* LinkHeap::GetHeap(void* obj, void (*disp)(void*)) noexcept {
	LinkHeap* ptr = nullptr;
	{
		std::lock_guard<decltype(mtx)> lck(mtx);
		Resize();
		ptr = heapPtrs.erase_last();
	}
	ptr->ptr = obj;
	ptr->disposer = disp;
	ptr->refCount = 1;
	ptr->looseRefCount = 1;
	return ptr;
}
void LinkHeap::ReturnHeap(LinkHeap* value) noexcept {
	std::lock_guard<decltype(mtx)> lck(mtx);
	heapPtrs.push_back(value);
}
PtrWeakLink::PtrWeakLink(const PtrLink& p) noexcept : offset(p.offset) {
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->looseRefCount;
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}
PtrWeakLink::PtrWeakLink(const PtrWeakLink& p) noexcept : offset(p.offset) {
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->looseRefCount;
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}
PtrWeakLink::PtrWeakLink(PtrWeakLink&& p) noexcept
	: heapPtr(p.heapPtr),
	  offset(p.offset) {
	p.heapPtr = nullptr;
}
void PtrWeakLink::operator=(const PtrLink& p) noexcept {
	if (p.heapPtr && p.heapPtr->ptr) {
		Dispose();
		++p.heapPtr->looseRefCount;
		heapPtr = p.heapPtr;
	} else {
		Dispose();
	}
	offset = p.offset;
}
void PtrWeakLink::operator=(const PtrWeakLink& p) noexcept {
	if (&p == this) return;
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->looseRefCount;
		Dispose();
		heapPtr = p.heapPtr;
	} else {
		Dispose();
	}
	offset = p.offset;
}
void PtrWeakLink::operator=(PtrWeakLink&& p) noexcept {
	if (&p == this) return;
	Dispose();
	heapPtr = p.heapPtr;
	offset = p.offset;
	p.heapPtr = nullptr;
}
PtrLink::PtrLink(const PtrWeakLink& p) noexcept
	: offset(p.offset) {
	if (p.heapPtr && p.heapPtr->ptr) {
		++p.heapPtr->refCount;
		++p.heapPtr->looseRefCount;
		heapPtr = p.heapPtr;
	} else {
		heapPtr = nullptr;
	}
}

PtrLink::PtrLink(PtrWeakLink&& p) noexcept
	: heapPtr(p.heapPtr),
	  offset(p.offset) {
	if (heapPtr) {
		++heapPtr->refCount;
	}
	p.heapPtr = nullptr;
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
