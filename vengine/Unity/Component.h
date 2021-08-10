#pragma once
#include <Unity/UnityInclude.h>

namespace toolhub {
namespace db {
class IJsonDict;
}
class Component final : public vstd::IOperatorNewBase, public vstd::IDisposable {
private:
public:
	void Dispose() override {
		delete this;
	}
};
}// namespace toolhub