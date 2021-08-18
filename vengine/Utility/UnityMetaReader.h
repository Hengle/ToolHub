#pragma once
#include <Common/Common.h>

namespace Unity {
class VENGINE_DLL_COMMON UnityMetaReader {
private:
	vstd::vector<vstd::string_view> words;
	HashMap<vstd::string_view, vstd::string_view> keyValues;

public:
	UnityMetaReader(
		vstd::string_view view);
	~UnityMetaReader();
	vstd::string_view Get(vstd::string_view name) const;
};
}// namespace Unity