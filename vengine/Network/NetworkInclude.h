#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
namespace toolhub::net {
class ISocket;
class INetworkService;
class IFileStream;
class NetWork {
public:
	virtual vstd::unique_ptr<ISocket> GenServerTCPSock(
		uint16_t port) const = 0;
	virtual vstd::unique_ptr<ISocket> GenClientTCPSock(
		uint16_t port, char const* address) const = 0;
	virtual vstd::unique_ptr<INetworkService> GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		size_t maxBufferSize = 0x400000) const = 0;
	virtual vstd::unique_ptr<IFileStream> GetFileStream(
		INetworkService* service) const = 0;
};
#ifdef VENGINE_NETWORK_PROJECT

class NetWorkImpl : public NetWork {
public:
	void* service;
	NetWorkImpl();
	~NetWorkImpl();
	vstd::unique_ptr<ISocket> GenServerTCPSock(
		uint16_t port) const override;
	vstd::unique_ptr<ISocket> GenClientTCPSock(
		uint16_t port,
		char const* address) const override;
	vstd::unique_ptr<INetworkService> GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		size_t maxBufferSize) const override;
	vstd::unique_ptr<IFileStream> GetFileStream(
		INetworkService* service) const override;
};
#endif
}// namespace toolhub::net

//toolhub::net::NetWork const* NetWork_GetFactory()
//toolhub::db::Database const* Database_GetFactory()