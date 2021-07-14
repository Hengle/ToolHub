#pragma once
#include <Common/Common.h>
namespace vstd {
class VENGINE_DLL_COMMON Guid {
public:
	friend VENGINE_DLL_COMMON std::ostream& operator<<(std::ostream& out, const Guid& obj) noexcept;

	uint64_t data[2];
	Guid();
	Guid(uint64_t g0, uint64_t g1) {
		data[0] = g0;
		data[1] = g1;
	}
	bool operator==(Guid const& g) const {
		return data[0] == g.data[0] && data[1] == g.data[1];
	}
	bool operator!=(Guid const& g) const {
		return !operator==(g);
	}
};
template<>
struct hash<Guid> {
	size_t operator()(Guid const& guid) const {
		return Hash::Int32ArrayHash(
			reinterpret_cast<uint32_t const*>(&guid.data),
			reinterpret_cast<uint32_t const*>(&guid.data + 2));
	}
};
}// namespace vstd