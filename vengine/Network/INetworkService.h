#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/LockFreeArrayQueue.h>
namespace toolhub::net {
class ISocket;
class INetworkService {
public:
	virtual ~INetworkService() = default;
	virtual void SendMessage(vstd::string const& messageName, std::span<uint8_t> const& data) = 0;
};

}// namespace toolhub::net