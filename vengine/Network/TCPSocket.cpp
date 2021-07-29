#pragma vengine_package vengine_network

#include <ISocket.h>
#include <NetworkInclude.h>
#include <boost/asio.hpp>
namespace toolhub::net {

class TCPIOBase {
public:
	asio::io_service service;
	asio::ip::tcp::endpoint ep;
	asio::ip::tcp::socket socket;
	TCPIOBase(uint concurrent_thread, uint16_t port, asio::ip::tcp&& tcp)
		: service(concurrent_thread),
		  ep(std::move(tcp), port),
		  socket(service) {
	}
	TCPIOBase(uint concurrent_thread, uint16_t port, asio::ip::address&& add)
		: service(concurrent_thread),
		  ep(std::move(add), port),
		  socket(service) {
	}
	bool Read(
		vstd::string& errorMsg,
		vstd::vector<uint8_t>& data, size_t maxSize) {
		data.clear();
		data.reserve(maxSize);
		auto size = socket.try_read_some(errorMsg, asio::buffer(data.data(), maxSize));
		if (!size) return false;
		data.resize(*size);
		return true;
	}
	bool Write(
		vstd::string& errorMsg,
		std::span<uint8_t> data) {
		return socket.try_write_some(errorMsg, asio::buffer(data.data(), data.size()));
	}
};

class TCPServer_Impl final : public ISocket {
public:
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW

	vstd::string errorMsg;
	vstd::optional<asio::ip::tcp::acceptor> acc;
	vstd::optional<TCPIOBase> io;
	bool successAccept = false;
	uint concurrent_thread;
	//Server
	TCPServer_Impl(uint concurrent_thread, uint16_t port)
		: concurrent_thread(concurrent_thread) {
		io.New(concurrent_thread, port, asio::ip::tcp::v4());
		acc.New(io->service, io->ep);
	}
	uint ConcurrentThread() const override {
		return concurrent_thread;
	}
	bool Connect() override {
		if (successAccept) return true;
		successAccept = acc->try_accept(errorMsg, io->socket);
		return successAccept;
	}
	vstd::string const& ErrorMessage() override {
		return errorMsg;
	}
	~TCPServer_Impl() {
	}
	bool Read(vstd::vector<uint8_t>& data, size_t maxSize) override {
		if (!successAccept) return false;
		return io->Read(errorMsg, data, maxSize);
	}
	bool Write(std::span<uint8_t> data) override {
		if (!successAccept) return false;
		return io->Write(errorMsg, data);
	}
};

class TCPClient_Impl final : public ISocket {
public:
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW

	TCPIOBase io;
	uint concurrent_thread;
	bool successAccept;
	//Server
	vstd::string errorMsg;
	vstd::string const& ErrorMessage() override {
		return errorMsg;
	}

	TCPClient_Impl(uint concurrent_thread, uint16_t port, asio::ip::address&& add)
		: io(concurrent_thread, port, std::move(add)),
		  concurrent_thread(concurrent_thread) {
	}
	bool Connect() override {
		if (successAccept) return true;
		successAccept = io.socket.try_connect(errorMsg, io.ep);
		return successAccept;
	}
	uint ConcurrentThread() const override {
		return concurrent_thread;
	}

	~TCPClient_Impl() {
	}
	bool Read(vstd::vector<uint8_t>& data, size_t maxSize) override {
		if (!successAccept) return false;
		return io.Read(errorMsg, data, maxSize);
	}
	bool Write(std::span<uint8_t> data) override {
		if (!successAccept) return false;
		return io.Write(errorMsg, data);
	}
};

vstd::unique_ptr<ISocket> NetWorkImpl::GenServerTCPSock(
	uint concurrent_thread,
	uint16_t port) const {
	return new TCPServer_Impl(concurrent_thread, port);
}

vstd::unique_ptr<ISocket> NetWorkImpl::GenClientTCPSock(
	uint concurrent_thread,
	uint16_t port,
	char const* address) const {
	return new TCPClient_Impl(
		concurrent_thread,
		port,
		asio::ip::address::from_string(address));
}
}// namespace toolhub::net