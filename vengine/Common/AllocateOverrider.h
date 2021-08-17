#pragma once
#include <Common/Memory.h>
#include <stdint.h>

inline void* operator new(size_t n) {
	return vengine_malloc(n);
}
inline void operator delete(void* p) {
	vengine_free(p);
}