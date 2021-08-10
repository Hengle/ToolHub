#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/LockFreeArrayQueue.h>
#include <Network/FunctionSerializer.h>
namespace toolhub::net {
class ISocket;
class INetworkService : public vstd::IDisposable {
protected:
	virtual void AddFunc(
		vstd::string&& name,
		Runnable<void(std::span<uint8_t>)>&& func) = 0;
	virtual void SendMsg(vstd::string const& messageName, std::span<uint8_t> const& data) = 0;
	virtual ~INetworkService() = default;

public:
	template<typename Func>
	void AddCallback(
		vstd::string name,
		Func&& func) {
		AddFunc(
			std::move(name),
			vstd::SerDeAll<Func>::template Call(std::forward<Func>(func)));
	}

	template<typename Func>
	struct SendMsgCaller {
		friend class INetworkService;// in case msvc fxxk up
		INetworkService* ths;
		vstd::string const* name;
		template<typename... Args>
		void operator()(Args&&... args) const {
			ths->SendMsg(
				*name,
				vstd::SerDeAll<std::remove_cvref_t<Func>>::Ser(std::forward<Args>(args)...));
		}
	};

	template<typename Func>
	SendMsgCaller<Func> SendMessage(
		vstd::string const& name,
		Func&&) {
		return SendMsgCaller<Func>{this, &name};
	}

	virtual void Run() = 0;
	virtual ISocket* GetSocket() = 0;
};

#define NETSERVICE_REGIST_MESSAGE(func) AddCallback(#func##_sv, func)
#define NETSERVICE_SEND_MESSAGE(func, ...) SendMessage(#func##_sv, func)(__VA_ARGS__)
}// namespace toolhub::net