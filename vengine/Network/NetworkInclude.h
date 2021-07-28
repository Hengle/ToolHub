#pragma once
#include <Common/Common.h>
namespace toolhub::net {
class ISocket;
class NetWork {
public:
	virtual ISocket* GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const = 0;
	virtual ISocket* GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port, char const* address) const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT
class NetWorkImpl : public NetWork {
public:
	ISocket* GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const override;
	ISocket* GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port,
		char const* address) const override;
};
#endif
}// namespace toolhub::net

//toolhub::net::NetWork const* NetWork_GetFactory()