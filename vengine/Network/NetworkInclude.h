#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
// Entry:
// toolhub::net::NetWork const* NetWork_GetFactory()
namespace toolhub::net {
class ISocket;
class INetworkService;
class IFileStream;
class NetWork {
public:
	virtual ISocket* GenServerTCPSock(
		uint16_t port) const = 0;
	virtual ISocket* GenClientTCPSock(
		uint16_t port, char const* address) const = 0;
	virtual INetworkService* GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		size_t maxBufferSize = 0x400000) const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT

class NetWorkImpl : public NetWork {
public:
	void* service;
	NetWorkImpl();
	~NetWorkImpl();
	ISocket* GenServerTCPSock(
		uint16_t port) const override;
	ISocket* GenClientTCPSock(
		uint16_t port,
		char const* address) const override;
	INetworkService* GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		size_t maxBufferSize) const override;
};
#endif
}// namespace toolhub::net