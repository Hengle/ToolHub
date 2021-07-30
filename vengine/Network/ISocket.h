#pragma once
#include <Common/Common.h>
#include <Common/unique_ptr.h>
namespace toolhub::net {

class ISocket {
protected:
	ISocket() = default;

public:
	virtual ~ISocket() = default;
	virtual bool Read(vstd::vector<uint8_t>& data, size_t maxSize) = 0;
	virtual bool Write(std::span<uint8_t> data) = 0;
	virtual bool Connect() = 0;
	virtual vstd::string const& ErrorMessage() = 0;
};
}// namespace toolhub::net
