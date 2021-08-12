#pragma once

#include <Common/Common.h>
#include <Network/IRegistObject.h>
#include <Network/FunctionSerializer.h>
#include <Utility/VGuid.h>
namespace toolhub {
using IndexArray = vstd::vector<vstd::variant<vstd::string, uint64>>;
struct SerializeValue {
	enum class RefType : uint8_t {
		Resource,
		Object,
	};
	struct Reference {
		std::array<uint8_t, sizeof(vstd::Guid)> guidData;
		RefType refType;
	};
	vstd::variant<
		bool,
		int64,
		double,
		vstd::string,
		Reference>
		value;
};
}// namespace toolhub
namespace vstd {
template<>
struct SerDe<toolhub::SerializeValue::Reference> {
	using Value = toolhub::SerializeValue::Reference;
	static Value Get(std::span<uint8_t>& sp) {
		return Value{vstd::SerDe<decltype(Value::guidData)>::Get(sp), vstd::SerDe<decltype(Value::refType)>::Get(sp)};
	}
};
}// namespace vstd