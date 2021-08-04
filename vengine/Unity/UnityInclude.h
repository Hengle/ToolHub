#pragma once
#include <Common/Common.h>
namespace toolhub {
struct CSharpString {
	char* ptr;
	uint64 size;
	vstd::string ToString();
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