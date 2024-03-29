#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
// Entry:
// toolhub::net::NetWork* NetWork_GetFactory()
namespace toolhub::net {
class ISocket;
class ISocketAcceptor;
class INetworkService;
class IFileStream;
class NetWork {
protected:
	~NetWork() = default;

public:
	virtual ISocketAcceptor* GenServerAcceptor(
		uint16_t port) const = 0;
	virtual ISocket* GenServerTCPSock(
		ISocketAcceptor* acceptor) const = 0;
	virtual ISocket* GenClientTCPSock(
		uint16_t port, char const* address) const = 0;
	virtual INetworkService* GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		bool isServer,
		size_t maxBufferSize = 0x400000) const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT

class NetWorkImpl : public NetWork {
public:
	void* service;
	NetWorkImpl();
	~NetWorkImpl();
	ISocketAcceptor* GenServerAcceptor(
		uint16_t port) const override;
	ISocket* GenServerTCPSock(
		ISocketAcceptor* acceptor) const override;
	ISocket* GenClientTCPSock(
		uint16_t port,
		char const* address) const override;
	INetworkService* GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		bool isServer,
		size_t maxBufferSize) const override;
};
#endif
}// namespace toolhub::net