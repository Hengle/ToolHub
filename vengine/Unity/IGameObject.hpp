#pragma once
#include <Unity/IGameObject.h>
#include <Network/INetworkService.h>
namespace toolhub {

template<typename T>
void IGameObject::InitializeService(net::INetworkService* service) {
	static_assert(std::is_base_of_v<IGameObject, T>, "type must be base of IGameObject");
	service->RegistClass<T>();
	service->NET_REGIST_MEMBER(T, AddCompByRemote);
	service->NET_REGIST_MEMBER(T, RemoveCompByRemote);
	service->NET_REGIST_MEMBER(T, AddSubGORemote);
	service->NET_REGIST_MEMBER(T, RemoveSubGORemote);
	service->NET_REGIST_MEMBER(T, UpdateNameRemote);
	service->NET_REGIST_MEMBER(T, UpdateTagRemote);
	service->NET_REGIST_MEMBER(T, UpdateLayerRemote);
	service->NET_REGIST_MEMBER(T, UpdateIsStaticRemote);
	service->NET_REGIST_MEMBER(T, SetParentRemote);
}
}// namespace toolhub