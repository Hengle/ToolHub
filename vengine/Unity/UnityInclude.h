#pragma once
#include <Common/Common.h>
namespace toolhub {
struct CSharpString {
	char const* ptr = nullptr;
	uint64 size = 0;
	CSharpString() {}
	CSharpString(vstd::string_view strV)
		: ptr(strV.begin()), size(strV.size()) {
	}
	CSharpString(vstd::string& str)
		: ptr(str.data()), size(str.size()) {
	}
	vstd::string ToString() const;
	vstd::string_view ToSV() const;
};

struct BinaryArray {
	void* ptr = nullptr;
	int32 elementCount = 0;
	int32 stride = 0;
	BinaryArray() {}
	BinaryArray(
		void* ptr,
		int32 elementCount,
		int32 stride)
		: ptr(ptr),
		  elementCount(elementCount),
		  stride(stride) {
	}
};
enum class ComponentType : uint {
	Assets,
	Component,
	Value
};
}// namespace toolhub