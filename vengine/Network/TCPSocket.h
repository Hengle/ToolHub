#pragma once
#include <Common/Common.h>
#include <Common/unique_ptr.h>
namespace toolhub::net {

class ITCPSocket {
protected:
	ITCPSocket() = default;

public:
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
	virtual ~ITCPSocket() = default;
	virtual bool Read(vstd::vector<uint8_t>& data, size_t maxSize) = 0;
	virtual bool Write(std::span<uint8_t>  data) = 0;
};
}// namespace toolhub::net
