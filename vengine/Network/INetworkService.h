#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/LockFreeArrayQueue.h>
#include <Network/FunctionSerializer.h>
namespace toolhub::net {
class ISocket;
class INetworkService  : public vstd::IDisposable{
protected:
	virtual void AddFunc(
		vstd::string&& name,
		Runnable<void(std::span<uint8_t>)>&& func) = 0;
	virtual void SendMessage(vstd::string const& messageName, std::span<uint8_t> const& data) = 0;
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
	template<typename T>
	struct SendMsgStruct {
		template<typename... Args>
		static void SendMsg(
			INetworkService* ser,
			vstd::string const& name,
			Args&&... args) {
			ser->SendMessage(
				name,
				vstd::SerDeAll<T>::Ser(std::move(args)...));
		}
	};

	virtual void Run() = 0;
	virtual ISocket* GetSocket() = 0;
};
#define NETSERVICE_REGIST_MESSAGE(service, func) (service)->AddCallback(#func##_sv, func)
#define NETSERVICE_SEND_MESSAGE(service, func, ...) toolhub::net::INetworkService::SendMsgStruct<decltype(func)>::template SendMsg((service), #func##_sv, __VA_ARGS__)
}// namespace toolhub::net