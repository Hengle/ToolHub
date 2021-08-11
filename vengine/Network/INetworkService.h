#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/LockFreeArrayQueue.h>
#include <Network/FunctionSerializer.h>
namespace toolhub::net {
class ISocket;
class IRegistObject;
class INetworkService : public vstd::IDisposable {
protected:
	// Initialize
	virtual void AddFunc(
		Type tarType,
		vstd::string&& name,
		Runnable<void(IRegistObject*, std::span<uint8_t>)> func) = 0;
	virtual void SetConstructor(
		Type tarType,
		Runnable<IRegistObject*()> constructor) = 0;
	// Call
	virtual IRegistObject* CreateClass(
		Type tarType) = 0;
	virtual bool CallMemberFunc(
		IRegistObject* ptr,
		vstd::string const& name,
		std::span<uint8_t> arg) = 0;
	virtual IRegistObject* m_GetObject(uint64 id) = 0;
	virtual ~INetworkService() = default;
	template<typename T>
	struct memFuncPtr {
	};

	template<typename Class, typename _Ret, typename... Args>
	struct memFuncPtr<_Ret (Class::*)(Args...)> {
		using ClassType = Class;
	};

	template<typename Class, typename _Ret, typename... Args>
	struct memFuncPtr<_Ret (Class::*)(Args...) const> {
		using ClassType = Class;
	};

	template<typename Func>
	using MemberClassT = typename memFuncPtr<Func>::ClassType;

public:
	template<typename T>
	T* CreateClass() {
		static_assert(std::is_base_of_v<IRegistObject, T>, "Type must be based of IRegistObject!");
		return static_cast<T*>(CreateClass(typeid(T)));
	}
	template<typename T>
	T* GetObject(uint64 id) {
		static_assert(std::is_base_of_v<IRegistObject, T>, "Type must be based of IRegistObject!");
		return static_cast<T*>(m_GetObject(id));
	}
	template<typename T>
	void RegistClass() {
		static_assert(std::is_base_of_v<IRegistObject, T>, "Type must be based of IRegistObject!");
		SetConstructor(typeid(T), []() {
			return new T();
		});
	}

	template<typename Func>
	void RegistMemberFunc(
		vstd::string name,
		Func func) {
		using ClassT = MemberClassT<Func>;
		static_assert(std::is_base_of_v<IRegistObject, ClassT>, "Type must be based of IRegistObject!");

		AddFunc(
			typeid(ClassT),
			std::move(name),
			[func](IRegistObject* originPtr, std::span<uint8_t> data) {
				ClassT* ptr = static_cast<ClassT*>(originPtr);
				vstd::SerDeAll_Member<Func>::template CallMemberFunc(ptr, func, data);
			});
	}
	template<typename Func>
	struct CallStruct {
		INetworkService* ths;
		IRegistObject* obj;
		vstd::string const* name;
		template<typename... Args>
		void operator()(Args&&... args) const {
			ths->CallMemberFunc(obj, *name, vstd::SerDeAll_Member<Func>::Ser(args...));
		}
	};

	template<typename T, typename Func>
	CallStruct<Func> Call(
		T* obj,
		vstd::string const& name) {
		static_assert(std::is_base_of_v<IRegistObject, T>, "Type must be based of IRegistObject!");
		return CallStruct<Func>{this, static_cast<IRegistObject*>(obj), &name};
	}

	virtual void Run() = 0;
	virtual ISocket* GetSocket() = 0;
};
#define NET_REGIST_MEMBER(clsName, funcName) RegistMemberFunc(#funcName##_sv, &(clsName::funcName))
#define NET_CALL_FUNC(classPtr, funcName, ...) Call<std::remove_cvref_t<decltype(*classPtr)>, decltype(&std::remove_cvref_t<decltype(*classPtr)>::funcName)>((classPtr), #funcName##_sv)(__VA_ARGS__)

}// namespace toolhub::net