#pragma once
#include <VEngineConfig.h>
#include <stdint.h>
class Hash
{
public:
	static constexpr size_t FNV_offset_basis = 14695981039346656037ULL;
	static constexpr size_t FNV_prime = 1099511628211ULL;
	static size_t Int32ArrayHash(
		const uint32_t* const First,
		const uint32_t* const End) noexcept { // accumulate range [_First, First + Count) into partial FNV-1a hash Val
		size_t Val = FNV_offset_basis;
		for (const uint32_t* i = First; i != End; ++i) {
			Val ^= static_cast<size_t>(*i);
			Val *= FNV_prime;
		}
		return Val;
	}
	static size_t CharArrayHash(
		const char* First,
		const size_t Count) noexcept { // accumulate range [_First, First + Count) into partial FNV-1a hash Val
		size_t Val = FNV_offset_basis;
		const uint32_t* IntPtrEnd;
		{
			const uint32_t* IntPtr = (const uint32_t*)First;
			IntPtrEnd = IntPtr + (Count / sizeof(uint32_t));
			for (; IntPtr != IntPtrEnd; ++IntPtr) {
				Val ^= static_cast<size_t>(*IntPtr);
				Val *= FNV_prime;
			}
		}
		const char* End = First + Count;
		for (const char* start = (const char*)IntPtrEnd; start != End; ++start)
		{
			Val ^= static_cast<size_t>(*start);
			Val *= FNV_prime;
		}
		return Val;
	}
};
namespace vstd
{
	template <typename K>
	struct hash
	{
		inline size_t operator()(K const& value) const noexcept
		{
			return Hash::CharArrayHash((char const*)&value, sizeof(K));
		}
	};
	inline static size_t GetIntegerHash(size_t a)
	{
		a = (a + 0xfd7046c5) + (a << 3);
		a = (a + 0xfd7046c5) + (a >> 3);
		a = (a ^ 0xb55a4f09) ^ (a << 16);
		a = (a ^ 0xb55a4f09) ^ (a >> 16);
		return a;
	}
	template <>
	struct hash<uint32_t>
	{
		inline size_t operator()(uint32_t value) const noexcept
		{
			return GetIntegerHash(value);
		}
	};

	template <>
	struct hash<int32_t>
	{
		inline size_t operator()(int32_t value) const noexcept
		{
			return GetIntegerHash(value);
		}
	};
	template <>
	struct hash<size_t>
	{
		inline size_t operator()(size_t value) const noexcept
		{
			return GetIntegerHash(value);
		}
	};

	template <>
	struct hash<int64_t>
	{
		inline size_t operator()(int64_t value) const noexcept
		{
			return GetIntegerHash(value);
		}
	};
}