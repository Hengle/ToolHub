#pragma once
#include <VEngineConfig.h>
#include <atomic>
#include <mutex>
#include <Common/Runnable.h>
#include <Common/MetaLib.h>
#include <assert.h>
#include <Common/vector.h>
#include <Common/Memory.h>
#include <Common/Log.h>
#include <Common/spin_mutex.h>
#include <Common/HashMap.h>

class PtrLink;
class VObject;
class VENGINE_DLL_COMMON VObjectClass {
protected:
	template<typename OT, typename T>
	void AddType() {
		allowCastClass.Emplace(typeid(T), [](VObject* vobj) -> void* {
			return static_cast<T*>(static_cast<OT*>(vobj));
		});
	}

	std::atomic<VObjectClass const*> baseLevel;

public:
	VObjectClass* operator->() {
		return this;
	}
	template<typename SelfType, typename... Types>
	void AddTypes() {
		allowCastClass.Reserve(sizeof...(Types) + 1);
		allowCastClass.Emplace(typeid(SelfType), [](VObject* vobj) -> void* {
			return static_cast<SelfType*>(vobj);
		});
		auto&& v = {(AddType<SelfType, Types>(), 0)...};
	}
	HashMap<Type, funcPtr_t<void*(VObject*)>, vstd::hash<Type>, std::equal_to<Type>, VEngine_AllocType::Default> allowCastClass;
	VObjectClass();
	VObjectClass(Type type, funcPtr_t<void*(VObject*)>);
	~VObjectClass();
	VObjectClass* SetBase(VObjectClass const* base);
	template<typename T>
	static T* GetCastPtr(VObjectClass const* cls, VObject* ptr);
};

//__VA_ARGS__
#define REGIST_VOBJ_CLASS(CLSNAME, ...)         \
	static VObjectClass* Get##CLSNAME##Cls_() { \
		static VObjectClass vobj;               \
		vobj.AddTypes<CLSNAME, __VA_ARGS__>();  \
		return &vobj;                           \
	}                                           \
	static VObjectClass* CLSNAME##Cls_ = Get##CLSNAME##Cls_();

#define REGIST_SINGLE_VOBJ_CLASS(CLSNAME)                                           \
	static VObjectClass CLSNAME##Cls_(typeid(CLSNAME), [](VObject* vobj) -> void* { \
		return static_cast<CLSNAME*>(vobj);                                         \
	})

#define SET_VOBJ_CLASS(CLSNAME) originClassDesc = CLSNAME##Cls_->SetBase(originClassDesc)

class VENGINE_DLL_COMMON VObject {
	friend class PtrLink;

private:
	vstd::vector<Runnable<void(VObject*)>> disposeFuncs;
	static std::atomic<size_t> CurrentID;
	size_t instanceID;

protected:
	VObjectClass const* originClassDesc = nullptr;
	VObject();
	template<typename T>
	T* _GetInterface(VObject* ptr) const {
		return VObjectClass::GetCastPtr<T>(originClassDesc, ptr);
	}

public:
	Type GetType() const noexcept {
		return typeid(*const_cast<VObject*>(this));
	}
	template<typename T>
	T* GetInterface() {
		return _GetInterface(this);
	}
	template<typename T>
	T const* GetInterface() const {
		return _GetInterface(const_cast<VObject*>(this));
	}
	template<typename T>
	size_t GetInterfaceOffset() const {
		return reinterpret_cast<size_t>(VObjectClass::GetCastPtr<T>(originClassDesc, reinterpret_cast<VObject*>(1))) - (size_t)1;
	}

	void AddEventBeforeDispose(Runnable<void(VObject*)>&& func) noexcept;
	size_t GetInstanceID() const noexcept { return instanceID; }
	virtual ~VObject() noexcept;
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
	KILL_COPY_CONSTRUCT(VObject)
	VObject(VObject&& v) = delete;
};
template<typename T>
T* VObjectClass::GetCastPtr(VObjectClass const* ths, VObject* ptr) {
	if (ths == nullptr)
		return nullptr;
	auto ite = ths->allowCastClass.Find(typeid(T));
	if (ite) {
		return reinterpret_cast<T*>(ite.Value()(ptr));
	}
	VObjectClass const* basePtr = ths->baseLevel.load(std::memory_order_relaxed);
	if (basePtr) {
		return GetCastPtr<T>(basePtr, ptr);
	}
}
class PtrLink;
class PtrWeakLink;
struct VENGINE_DLL_COMMON LinkHeap {
	friend class PtrLink;
	friend class PtrWeakLink;

private:
	static void Resize() noexcept;
	funcPtr_t<void(void*)> disposer;
	std::atomic<int32_t> refCount;
	std::atomic<int32_t> looseRefCount;
	static ArrayList<LinkHeap*, VEngine_AllocType::Default> heapPtrs;
	static spin_mutex mtx;
	static LinkHeap* GetHeap(void* obj, void (*disp)(void*)) noexcept;
	static void ReturnHeap(LinkHeap* value) noexcept;

public:
	void* ptr;
};

class VEngine;
class PtrWeakLink;
class VENGINE_DLL_COMMON PtrLink {
	friend class VEngine;
	friend class PtrWeakLink;

public:
	LinkHeap* heapPtr;
	size_t offset = 0;
	inline PtrLink() noexcept : heapPtr(nullptr) {
	}
	void Dispose() noexcept;
	template<typename T>
	PtrLink(T* obj, funcPtr_t<void(void*)> disposer) noexcept {
		heapPtr = LinkHeap::GetHeap(obj, disposer);
	}

	PtrLink(const PtrLink& p) noexcept;
	PtrLink(PtrLink&& p) noexcept;
	void operator=(const PtrLink& p) noexcept;
	void operator=(PtrLink&& p) noexcept;
	PtrLink(const PtrWeakLink& link) noexcept;
	PtrLink(PtrWeakLink&& link) noexcept;

	void Destroy() noexcept;
	~PtrLink() noexcept {
		Dispose();
	}
};
class VENGINE_DLL_COMMON PtrWeakLink {
public:
	LinkHeap* heapPtr;
	size_t offset = 0;
	PtrWeakLink() noexcept : heapPtr(nullptr) {
	}

	void Dispose() noexcept;
	PtrWeakLink(const PtrLink& p) noexcept;
	PtrWeakLink(const PtrWeakLink& p) noexcept;
	PtrWeakLink(PtrWeakLink&& p) noexcept;//TODO
	void operator=(const PtrLink& p) noexcept;
	void operator=(const PtrWeakLink& p) noexcept;
	void operator=(PtrWeakLink&& p) noexcept;//TODO
	void Destroy() noexcept;

	inline ~PtrWeakLink() noexcept {
		Dispose();
	}
};
namespace vstd {
template<typename T, typename F>
size_t GetOffset() {
	T* const ptr = reinterpret_cast<T*>(1);
	F* const fPtr = static_cast<F*>(ptr);
	return (size_t)fPtr - (size_t)1;
}
}// namespace vstd
template<typename T>
class ObjWeakPtr;
template<typename T>
class ObjectPtr;
class SharedWeakFlag;
class SharedFlag {
	friend class SharedWeakFlag;

private:
	PtrLink link;

public:
	SharedFlag(size_t value)
		: link(reinterpret_cast<char*>(value), [](void*) {}) {
	}
	SharedFlag() : link() {}
	SharedFlag(SharedFlag const& flag)
		: link(flag.link) {
	}
	SharedFlag(SharedFlag&& flag)
		: link(std::move(flag.link)) {
	}
	inline SharedFlag(SharedWeakFlag const& flag);
	inline SharedFlag(SharedWeakFlag&& flag);

	inline void Dispose() {
		link.Dispose();
	}
	inline operator size_t() const noexcept {
		if (link.heapPtr == nullptr) {
			return 0;
		}
		return reinterpret_cast<size_t>(link.heapPtr->ptr);
	}
	inline void operator=(const SharedWeakFlag& other) noexcept;
	inline void operator=(const SharedFlag& other) noexcept {
		link = other.link;
	}
	inline void operator=(SharedFlag&& other) noexcept {
		link = std::move(other.link);
	}
};

class SharedWeakFlag {
	friend class SharedFlag;

private:
	PtrWeakLink link;

public:
	SharedWeakFlag() : link() {}
	SharedWeakFlag(SharedFlag const& flag) : link(flag.link) {}
	SharedWeakFlag(SharedWeakFlag const& flag) : link(flag.link) {}
	SharedWeakFlag(SharedWeakFlag&& flag) : link(std::move(flag.link)) {}
	inline void operator=(const SharedWeakFlag& other) noexcept {
		link = other.link;
	}
	inline void operator=(SharedWeakFlag&& other) noexcept {
		link = std::move(other.link);
	}
	inline void operator=(const SharedFlag& other) noexcept {
		link = other.link;
	}
	inline operator size_t() const noexcept {
		if (link.heapPtr == nullptr) {
			return 0;
		}
		return reinterpret_cast<size_t>(link.heapPtr->ptr);
	}
};

void SharedFlag::operator=(const SharedWeakFlag& other) noexcept {
	link = other.link;
}

SharedFlag::SharedFlag(SharedWeakFlag const& flag)
	: link(flag.link) {
}
SharedFlag::SharedFlag(SharedWeakFlag&& flag)
	: link(std::move(flag.link)) {
}
template<typename T>
class ObjectPtr {
private:
	friend class ObjWeakPtr<T>;
	PtrLink link;
	inline ObjectPtr(T* ptr, funcPtr_t<void(void*)> disposer) noexcept : link(ptr, disposer) {
	}
	T* GetPtr() const noexcept {
		return reinterpret_cast<T*>(reinterpret_cast<size_t>(link.heapPtr->ptr) + link.offset);
	}

public:
	ObjectPtr(const PtrLink& link, size_t addOffset) noexcept : link(link) {
		this->link.offset += addOffset;
	}
	ObjectPtr(PtrLink&& link, size_t addOffset) noexcept : link(std::move(link)) {
		this->link.offset += addOffset;
	}
	inline ObjectPtr() noexcept : link() {}
	inline ObjectPtr(std::nullptr_t) noexcept : link() {
	}
	inline ObjectPtr(const ObjectPtr<T>& ptr) noexcept : link(ptr.link) {
	}
	inline ObjectPtr(ObjectPtr<T>&& ptr) noexcept : link(std::move(ptr.link)) {
	}
	inline ObjectPtr(const ObjWeakPtr<T>& ptr) noexcept;
	inline ObjectPtr(ObjWeakPtr<T>&& ptr) noexcept;
	static ObjectPtr<T> MakePtr(T* ptr) noexcept {
		return ObjectPtr<T>(ptr, [](void* ptr) -> void {
			delete (reinterpret_cast<T*>(ptr));
		});
	}
	static ObjectPtr<T> MakePtr(T* ptr, funcPtr_t<void(void*)> disposer) noexcept {
		return ObjectPtr<T>(ptr, disposer);
	}
	template<typename... Args>
	static ObjectPtr<T> NewObject(Args&&... args) {
		T* ptr = vengine_new<T>(std::forward<Args>(args)...);
		return ObjectPtr<T>(ptr, [](void* ptr) -> void {
			vengine_delete<T>(reinterpret_cast<T*>(ptr));
		});
	}
	static ObjectPtr<T> MakePtrNoMemoryFree(T* ptr) noexcept {
		return ObjectPtr<T>(ptr, [](void* ptr) -> void {
			if (std::is_trivially_destructible_v<T>)
				(reinterpret_cast<T*>(ptr))->~T();
		});
	}
	static ObjectPtr<T> MakePtr(ObjectPtr<T>) noexcept = delete;

	inline operator bool() const noexcept {
		return link.heapPtr != nullptr && link.heapPtr->ptr != nullptr;
	}
	inline bool operator!() const {
		return !operator bool();
	}

	inline operator T*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline void Destroy() noexcept {
		link.Destroy();
	}

	template<typename F>
	inline ObjectPtr<F> CastTo() const& noexcept {
		return ObjectPtr<F>(link, vstd::GetOffset<T, F>());
	}
	template<typename F>
	ObjectPtr<F> InterfaceCast() const& noexcept {
		static_assert(std::is_same_v<T, VObject> || std::is_base_of_v<VObject, T> || std::_Always_false<F>, "Only vobject class can use this!");
		return ObjectPtr<F>(link, GetPtr()->GetInterfaceOffset<F>());
	}
	template<typename F>
	ObjectPtr<F> InterfaceCast() && noexcept {
		static_assert(std::is_same_v<T, VObject> || std::is_base_of_v<VObject, T> || std::_Always_false<F>, "Only vobject class can use this!");
		return ObjectPtr<F>(std::move(link), GetPtr()->GetInterfaceOffset<F>());
	}
	template<typename F>
	inline ObjectPtr<F> Reinterpret_CastTo(size_t offset) const& noexcept {
		return ObjectPtr<F>(link, offset);
	}
	template<typename F>
	inline ObjectPtr<F> CastTo() && noexcept {
		return ObjectPtr<F>(std::move(link), vstd::GetOffset<T, F>());
	}
	template<typename F>
	inline ObjectPtr<F> Reinterpret_CastTo(size_t offset) && noexcept {
		return ObjectPtr<F>(std::move(link), offset);
	}
	inline void operator=(const ObjWeakPtr<T>& other) noexcept;
	inline void operator=(const ObjectPtr<T>& other) noexcept {
		link = other.link;
	}
	inline void operator=(ObjectPtr<T>&& other) noexcept {
		link = std::move(other.link);
	}
	inline void operator=(T* other) noexcept = delete;
	inline void operator=(void* other) noexcept = delete;
	inline void operator=(std::nullptr_t t) noexcept {
		link.Dispose();
	}

	inline T* operator->() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline T& operator*() noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}

	inline T const& operator*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}

	inline bool operator==(const ObjectPtr<T>& ptr) const noexcept {
		return link.heapPtr == ptr.link.heapPtr;
	}
	inline bool operator!=(const ObjectPtr<T>& ptr) const noexcept {
		return link.heapPtr != ptr.link.heapPtr;
	}
};

template<typename T>
class ObjectPtr<T[]> {

private:
	friend class ObjWeakPtr<T[]>;
	PtrLink link;
	inline ObjectPtr(T* ptr, funcPtr_t<void(void*)> disposer) noexcept : link(ptr, disposer) {
	}
	T* GetPtr() const noexcept {
		return reinterpret_cast<T*>(reinterpret_cast<size_t>(link.heapPtr->ptr) + link.offset);
	}

public:
	ObjectPtr(const PtrLink& link, size_t addOffset) noexcept : link(link) {
		this->link.offset += addOffset;
	}
	ObjectPtr(PtrLink&& link, size_t addOffset) noexcept : link(std::move(link)) {
		this->link.offset += addOffset;
	}
	inline ObjectPtr() noexcept : link() {}
	inline ObjectPtr(std::nullptr_t) noexcept : link() {
	}
	inline ObjectPtr(const ObjectPtr<T[]>& ptr) noexcept : link(ptr.link) {
	}
	inline ObjectPtr(ObjectPtr<T[]>&& ptr) noexcept : link(std::move(ptr.link)) {
	}
	inline ObjectPtr(const ObjWeakPtr<T[]>& ptr) noexcept;
	static ObjectPtr<T[]> MakePtr(T* ptr) noexcept {
		return ObjectPtr<T[]>(ptr, [](void* ptr) -> void {
			delete[](reinterpret_cast<T*>(ptr));
		});
	}
	static ObjectPtr<T[]> MakePtr(T* ptr, funcPtr_t<void(void*)> disposer) noexcept {
		return ObjectPtr<T[]>(ptr, disposer);
	}
	static ObjectPtr<T[]> MakePtr(ObjectPtr<T[]>) noexcept = delete;

	inline operator bool() const noexcept {
		return link.heapPtr != nullptr && link.heapPtr->ptr != nullptr;
	}
	bool operator!() const {
		return !operator bool();
	}

	inline operator T*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline void Destroy() noexcept {
		link.Destroy();
	}

	template<typename F>
	inline ObjectPtr<F[]> Reinterpret_CastTo(size_t offset) const& noexcept {
		return ObjectPtr<F[]>(link, offset);
	}
	template<typename F>
	inline ObjectPtr<F[]> Reinterpret_CastTo(size_t offset) && noexcept {
		return ObjectPtr<F[]>(std::move(link), offset);
	}
	inline void operator=(const ObjWeakPtr<T[]>& other) noexcept;
	inline void operator=(const ObjectPtr<T[]>& other) noexcept {
		link = other.link;
	}
	inline void operator=(ObjectPtr<T[]>&& other) noexcept {
		link = std::move(other.link);
	}

	inline void operator=(T* other) noexcept = delete;
	inline void operator=(void* other) noexcept = delete;
	inline void operator=(std::nullptr_t t) noexcept {
		link.Dispose();
	}

	inline T* operator->() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline T& operator*() noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}

	inline T const& operator*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}
	inline T& operator[](size_t key) noexcept {
		return GetPtr()[key];
	}

	inline T const& operator[](size_t key) const noexcept {
		return GetPtr()[key];
	}

	inline bool operator==(const ObjectPtr<T[]>& ptr) const noexcept {
		return link.heapPtr == ptr.link.heapPtr;
	}
	inline bool operator!=(const ObjectPtr<T[]>& ptr) const noexcept {
		return link.heapPtr != ptr.link.heapPtr;
	}
};

template<typename T>
class ObjWeakPtr {
private:
	friend class ObjectPtr<T>;
	PtrWeakLink link;
	T* GetPtr() const noexcept {
		return reinterpret_cast<T*>(reinterpret_cast<size_t>(link.heapPtr->ptr) + link.offset);
	}

public:
	inline ObjWeakPtr() noexcept : link() {}
	inline ObjWeakPtr(std::nullptr_t) noexcept : link() {
	}
	inline ObjWeakPtr(const ObjWeakPtr<T>& ptr) noexcept : link(ptr.link) {
	}
	inline ObjWeakPtr(ObjWeakPtr<T>&& ptr) noexcept : link(std::move(ptr.link)) {
	}
	inline ObjWeakPtr(const ObjectPtr<T>& ptr) noexcept : link(ptr.link) {
	}
	ObjWeakPtr(const PtrWeakLink& link, size_t addOffset) noexcept : link(link) {
		this->link.offset += addOffset;
	}
	ObjWeakPtr(PtrWeakLink&& link, size_t addOffset) noexcept : link(std::move(link)) {
		this->link.offset += addOffset;
	}

	inline operator bool() const noexcept {
		return link.heapPtr != nullptr && link.heapPtr->ptr != nullptr;
	}
	bool operator!() const {
		return !operator bool();
	}

	inline operator T*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline void Destroy() noexcept {
		link.Destroy();
	}

	template<typename F>
	inline ObjWeakPtr<F> CastTo() const& noexcept {
		return ObjWeakPtr<F>(link, vstd::GetOffset<T, F>());
	}
	template<typename F>
	inline ObjWeakPtr<F> Reinterpret_CastTo(size_t offset) const& noexcept {
		return ObjWeakPtr<F>(link, offset);
	}
	template<typename F>
	inline ObjWeakPtr<F> CastTo() && noexcept {
		return ObjWeakPtr<F>(std::move(link), vstd::GetOffset<T, F>());
	}
	template<typename F>
	inline ObjWeakPtr<F> Reinterpret_CastTo(size_t offset) && noexcept {
		return ObjWeakPtr<F>(std::move(link), offset);
	}
	template<typename F>
	ObjectPtr<F> InterfaceCast() const& noexcept {
		static_assert(std::is_same_v<T, VObject> || std::is_base_of_v<VObject, T> || std::_Always_false<F>, "Only vobject class can use this!");
		return ObjectPtr<F>(link, GetPtr()->GetInterfaceOffset<F>());
	}
	template<typename F>
	ObjectPtr<F> InterfaceCast() && noexcept {
		static_assert(std::is_same_v<T, VObject> || std::is_base_of_v<VObject, T> || std::_Always_false<F>, "Only vobject class can use this!");
		return ObjectPtr<F>(std::move(link), GetPtr()->GetInterfaceOffset<F>());
	}
	inline void operator=(const ObjWeakPtr<T>& other) noexcept {
		link = other.link;
	}
	inline void operator=(ObjWeakPtr<T>&& other) noexcept {
		link = std::move(other.link);
	}

	inline void operator=(const ObjectPtr<T>& other) noexcept {
		link = other.link;
	}

	inline void operator=(T* other) noexcept = delete;
	inline void operator=(void* other) noexcept = delete;
	inline void operator=(std::nullptr_t t) noexcept {
		link.Dispose();
	}

	inline T* operator->() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline T& operator*() noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}

	inline T const& operator*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}

	inline bool operator==(const ObjWeakPtr<T>& ptr) const noexcept {
		return link.heapPtr == ptr.link.heapPtr;
	}
	inline bool operator!=(const ObjWeakPtr<T>& ptr) const noexcept {
		return link.heapPtr != ptr.link.heapPtr;
	}
};

template<typename T>
class ObjWeakPtr<T[]> {
private:
	friend class ObjectPtr<T[]>;
	PtrWeakLink link;
	T* GetPtr() const noexcept {
		return reinterpret_cast<T*>(reinterpret_cast<size_t>(link.heapPtr->ptr) + link.offset);
	}

public:
	inline ObjWeakPtr() noexcept : link() {}
	inline ObjWeakPtr(std::nullptr_t) noexcept : link() {
	}
	inline ObjWeakPtr(const ObjWeakPtr<T[]>& ptr) noexcept : link(ptr.link) {
	}
	inline ObjWeakPtr(ObjWeakPtr<T[]>&& ptr) noexcept : link(std::move(ptr.link)) {
	}
	inline ObjWeakPtr(const ObjectPtr<T[]>& ptr) noexcept : link(ptr.link) {
	}
	ObjWeakPtr(const PtrWeakLink& link, size_t addOffset) noexcept : link(link) {
		this->link.offset += addOffset;
	}
	ObjWeakPtr(PtrWeakLink&& link, size_t addOffset) noexcept : link(std::move(link)) {
		this->link.offset += addOffset;
	}

	inline operator bool() const noexcept {
		return link.heapPtr != nullptr && link.heapPtr->ptr != nullptr;
	}
	bool operator!() const {
		return !operator bool();
	}

	inline operator T*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline void Destroy() noexcept {
		link.Destroy();
	}

	template<typename F>
	inline ObjWeakPtr<F[]> Reinterpret_CastTo(size_t offset) const noexcept {
		return ObjWeakPtr<F[]>(link, offset);
	}
	inline void operator=(const ObjWeakPtr<T[]>& other) noexcept {
		link = other.link;
	}
	inline void operator=(ObjWeakPtr<T[]>&& other) noexcept {
		link = std::move(other.link);
	}

	inline void operator=(const ObjectPtr<T[]>& other) noexcept {
		link = other.link;
	}

	inline void operator=(T* other) noexcept = delete;
	inline void operator=(void* other) noexcept = delete;
	inline void operator=(std::nullptr_t t) noexcept {
		link.Dispose();
	}

	inline T* operator->() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return GetPtr();
	}

	inline T& operator*() noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}
	inline T const& operator*() const noexcept {
#if defined(DEBUG)
		//Null Check!
		assert(link.heapPtr != nullptr);
#endif
		return *GetPtr();
	}
	inline T& operator[](size_t key) noexcept {
		return GetPtr()[key];
	}

	inline T const& operator[](size_t key) const noexcept {
		return GetPtr()[key];
	}
	inline bool operator==(const ObjWeakPtr<T[]>& ptr) const noexcept {
		return link.heapPtr == ptr.link.heapPtr;
	}
	inline bool operator!=(const ObjWeakPtr<T[]>& ptr) const noexcept {
		return link.heapPtr != ptr.link.heapPtr;
	}
};
template<typename T>
inline ObjectPtr<T>::ObjectPtr(const ObjWeakPtr<T>& ptr) noexcept : link(ptr.link) {
}
template<typename T>
inline ObjectPtr<T>::ObjectPtr(ObjWeakPtr<T>&& ptr) noexcept : link(std::move(ptr.link)) {
}
template<typename T>
inline void ObjectPtr<T>::operator=(const ObjWeakPtr<T>& other) noexcept {
	link = other.link;
}

template<typename T>
inline ObjectPtr<T[]>::ObjectPtr(const ObjWeakPtr<T[]>& ptr) noexcept : link(ptr.link) {
}
template<typename T>
inline void ObjectPtr<T[]>::operator=(const ObjWeakPtr<T[]>& other) noexcept {
	link = other.link;
}

template<typename T>
inline static ObjectPtr<T> MakeObjectPtr(T* ptr) noexcept {
	return ObjectPtr<T>::MakePtr(ptr);
}

template<typename T>
inline static ObjectPtr<T> MakeObjectPtr(T* ptr, funcPtr_t<void(void*)> disposer) noexcept {
	return ObjectPtr<T>::MakePtr(ptr, disposer);
}