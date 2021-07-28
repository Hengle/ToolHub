#pragma once
#include <Common/linq.h>
#include <Common/Runnable.h>
namespace toolhub::net {
class ISocket;
class INetworkService;
class NetWork {
public:
	using Function = std::pair<Runnable<void(std::span<uint8_t>)>, vstd::string>;
	virtual vstd::unique_ptr<ISocket> GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const = 0;
	virtual vstd::unique_ptr<ISocket> GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port, char const* address) const = 0;
	virtual vstd::unique_ptr<INetworkService> GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		vstd::linq::Iterator<Function>& funcs,
		size_t maxBufferSize = 0x400000) const = 0;

};
#ifdef VENGINE_NETWORK_PROJECT
class NetWorkImpl : public NetWork {
public:
	vstd::unique_ptr<ISocket> GenServerTCPSock(
		uint concurrent_thread,
		uint16_t port) const override;
	vstd::unique_ptr<ISocket> GenClientTCPSock(
		uint concurrent_thread,
		uint16_t port,
		char const* address) const override;
	vstd::unique_ptr<INetworkService> GetNetworkService(
		vstd::unique_ptr<ISocket>&& socket,
		vstd::linq::Iterator<Function>& funcs,
		size_t maxBufferSize) const override;
};
#endif
}// namespace toolhub::net

//toolhub::net::NetWork const* NetWork_GetFactory()