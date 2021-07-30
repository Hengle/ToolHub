#pragma once

#include <ISocket.h>
#include <NetworkInclude.h>
#include <boost/asio.hpp>
namespace toolhub::net {

class TCPIOBase {
public:
	asio::io_service* service;
	asio::ip::tcp::endpoint ep;
	asio::ip::tcp::socket socket;
	TCPIOBase(
		asio::io_service* service, uint16_t port, asio::ip::tcp&& tcp)
		: service(service),
		  ep(std::move(tcp), port),
		  socket(*service) {
	}
	TCPIOBase(
		asio::io_service* service, uint16_t port, asio::ip::address&& add)
		: service(service),
		  ep(std::move(add), port),
		  socket(*service) {
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
	//Server
	TCPServer_Impl(asio::io_service* service, uint16_t port) {
		io.New(service, port, asio::ip::tcp::v4());
		acc.New(*io->service, io->ep);
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
	bool successAccept;
	//Server
	vstd::string errorMsg;
	vstd::string const& ErrorMessage() override {
		return errorMsg;
	}

	TCPClient_Impl(asio::io_service* service, uint16_t port, asio::ip::address&& add)
		: io(service, port, std::move(add)) {
	}
	bool Connect() override {
		if (successAccept) return true;
		successAccept = io.socket.try_connect(errorMsg, io.ep);
		return successAccept;
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
	uint16_t port) const {
	return new TCPServer_Impl(
		reinterpret_cast<asio::io_service*>(service),
		port);
}

vstd::unique_ptr<ISocket> NetWorkImpl::GenClientTCPSock(
	uint16_t port,
	char const* address) const {
	return new TCPClient_Impl(
		reinterpret_cast<asio::io_service*>(service),
		port,
		asio::ip::address::from_string(address));
}
}// namespace toolhub::net