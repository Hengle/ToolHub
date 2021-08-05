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
	void* ptr;
	int32 byteSize;
	int32 stride;
};
enum class ComponentType : uint {
	Assets,
	Component,
	Value
};
}// namespace toolhub