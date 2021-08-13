#pragma once
#include <Unity/IComponent.h>
#include <Network/INetworkService.h>
namespace toolhub {
template<typename T>
static void IComponent::InitializeService(net::INetworkService* service) {
	service->RegistClass<T>();
	service->NET_REGIST_MEMBER(T, OnCreateRemote);
	service->NET_REGIST_MEMBER(T, SetValueRemote);
	service->NET_REGIST_MEMBER(T, SetTrivialArrayRemote);
	service->NET_REGIST_MEMBER(T, SetTrivialStructRemote);
}
}// namespace net