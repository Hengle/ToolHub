#pragma once
#include <Common/Common.h>
namespace toolhub::net {
class ITCPSocket;
class NetWork {
public:
	virtual ITCPSocket* GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const = 0;
	virtual ITCPSocket* GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port, char const* address) const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT
class NetWorkImpl : public NetWork {
public:
	ITCPSocket* GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const override;
	ITCPSocket* GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port,
		char const* address) const override;
};
#endif
}// namespace toolhub::net

//toolhub::net::NetWork const* NetWork_GetFactory()