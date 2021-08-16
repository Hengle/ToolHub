/**
 * @file md5.h
 * @The header file of md5.
 * @author Jiewei Wei
 * @mail weijieweijerry@163.com
 * @github https://github.com/JieweiWei
 * @data Oct 19 2014
 *
 */
#pragma once
/* Parameters of MD5. */

/**
 * @Basic MD5 functions.
 *
 * @param there uint32_t.
 *
 * @return one uint32_t.
 */

/* Define of btye.*/
/* Define of uint8_t. */
#include <Common/Common.h>
namespace vstd {
static constexpr size_t MD5_SIZE = 16;

VENGINE_DLL_COMMON std::array<uint8_t, MD5_SIZE> GetMD5FromString(vstd::string const& str);
VENGINE_DLL_COMMON std::array<uint8_t, MD5_SIZE> GetMD5FromArray(std::span<uint8_t> data);
//Used for unity
class VENGINE_DLL_COMMON MD5 {
public:
	struct MD5Data {
		uint64 data0;
		uint64 data1;
		uint64 binLen;
	};

private:
	MD5Data data;

public:
	MD5Data const& ToBinary() const { return data; }
	uint64 OriginLen() const { return data.binLen; }
	MD5(vstd::string const& str);
	MD5(vstd::string_view str);
	MD5(std::span<uint8_t> bin);
	MD5(MD5 const&) = default;
	MD5(MD5&&) = default;
	MD5(MD5Data const& data);
	vstd::string ToString(bool upper = true) const;
	template<typename T>
	MD5& operator=(T&& t) {
		this->~MD5();
		new (this) MD5(std::forward<T>(t));
		return *this;
	}
	~MD5() = default;
	bool operator==(MD5 const& m) const;
	bool operator!=(MD5 const& m) const;
};
template<>
struct hash<MD5::MD5Data> {
	size_t operator()(MD5::MD5Data const& m) const {
		uint const* ptr = reinterpret_cast<uint const*>(&m.data0);
		auto endPtr = ptr + sizeof(MD5::MD5Data) / sizeof(uint);
		return Hash::Int32ArrayHash(ptr, endPtr);
	}
};
template<>
struct hash<MD5> {
	size_t operator()(MD5 const& m) const {
		static hash<MD5::MD5Data> dataHasher;
		return dataHasher(m.ToBinary());
	}
};
}// namespace vstd