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
	template<typename SelfType>
	struct TypeAdder {
		template<typename... Types>
		static void AddTypes(VObjectClass* o) {
			o->allowCastClass.Emplace(typeid(SelfType), [](VObject* vobj) -> void* {
				return static_cast<SelfType*>(vobj);
				});
			if constexpr (sizeof...(Types) > 0) {
				auto&& v = { (o->AddType<SelfType, Types>(), 0)... };
			}
		}
	};
	HashMap<Type, funcPtr_t<void* (VObject*)>, vstd::hash<Type>, std::equal_to<Type>, VEngine_AllocType::Default> allowCastClass;
	VObjectClass();
	VObjectClass(Type type, funcPtr_t<void* (VObject*)>);
	~VObjectClass();
	VObjectClass* SetBase(VObjectClass const* base);
	template<typename T>
	static T* GetCastPtr(VObjectClass const* cls, VObject* ptr);
};

//__VA_ARGS__
#define REGIST_VOBJ_CLASS(CLSNAME, ...)                                 \
	static VObjectClass* Get##CLSNAME##Cls_() {                         \
		static VObjectClass vobj;                                       \
		VObjectClass::TypeAdder<CLSNAME>::AddTypes<__VA_ARGS__>(&vobj); \
		return &vobj;                                                   \
	}                                                                   \
	static VObjectClass* CLSNAME##Cls_ = Get##CLSNAME##Cls_();

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
	T* M_GetInterface(VObject* ptr) const {
		return VObjectClass::GetCastPtr<T>(originClassDesc, ptr);
	}

public:
	Type GetType() const noexcept {
		return typeid(*const_cast<VObject*>(this));
	}
	template<typename T>
	T* GetInterface() {
		return M_GetInterface(this);
	}
	template<typename T>
	T const* GetInterface() const {
		return M_GetInterface(const_cast<VObject*>(this));
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
	VObjectClass const* basePtr = ths->baseLevel.load(std::memory_order_acquire);
	if (basePtr) {
		return GetCastPtr<T>(basePtr, ptr);
	}
}
#include <Common/ObjectPtr.h>