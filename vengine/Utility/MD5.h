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

}// namespace vstd