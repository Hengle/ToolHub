#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/LockFreeArrayQueue.h>
namespace toolhub::net {
class ISocket;
class NetworkCaller {
public:
	using Function = std::pair<Runnable<void(std::span<uint8_t>)>, vstd::string>;

private:

	vstd::unique_ptr<ISocket> socket;
	HashMap<vstd::string, uint> messageMap;
	vstd::vector<Function> funcMap;
	vstd::optional<std::thread> readThread;
	vstd::optional<std::thread> writeThread;
	LockFreeArrayQueue<vstd::vector<uint8_t>> writeCmd;

	size_t maxBufferSize;

	void Read();
	void Write();
	void InitMessageMap(std::span<uint8_t> sp);
	vstd::vector<uint8_t> WriteMessageMap();

public:
	NetworkCaller(
		vstd::unique_ptr<ISocket>&& socket,
		vstd::vector<Function>&& funcs,
		size_t maxBufferSize = 0x400000);// Default 4M
	~NetworkCaller();
	void SendMessage(vstd::string const& messageName, std::span<uint8_t> const& data);
	KILL_COPY_CONSTRUCT(NetworkCaller)
	KILL_MOVE_CONSTRUCT(NetworkCaller)
};
}// namespace toolhub::net