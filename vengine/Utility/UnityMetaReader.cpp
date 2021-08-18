#pragma vengine_package vengine_dll
#include <Utility/UnityMetaReader.h>
namespace Unity {
namespace detail {

static vstd::vector<vstd::string_view> SeparateMetaFile(vstd::string_view view) {
	vstd::vector<vstd::string_view> words;
	bool isEmpty = true;
	bool isChar = false;
	auto IsCharSpace = [](char c) {
		switch (c) {
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				return true;
			default:
				return false;
		}
	};
	auto IsCharacter = [](char c) {
		return (c >= '0' && c <= '9')
			   || (c >= 'a' && c <= 'z')
			   || (c >= 'A' && c <= 'Z');
	};
	char const* lastPtr = view.begin();
	for (auto i = lastPtr; i != view.end(); ++i) {
		if (isEmpty) {
			if (!IsCharSpace(*i)) {
				lastPtr = i;
				isEmpty = false;
				isChar = IsCharacter(*i);
			}
		} else {
			if (IsCharSpace(*i)) {
				isEmpty = true;
				words.emplace_back(lastPtr, i);
				lastPtr = i;
			} else if (isChar != IsCharacter(*i)) {
				isChar = !isChar;
				words.emplace_back(lastPtr, i);
				lastPtr = i;
			}
		}
	}
	if (!isEmpty) {
		words.emplace_back(lastPtr, view.end());
	}
	return words;
}
}// namespace detail
UnityMetaReader::UnityMetaReader(
	vstd::string_view view) {
	words = detail::SeparateMetaFile(view);
	for (auto&& i : words) {
		if (i != ':') continue;
		if ((&i - words.begin()) > 0 && (words.end() - &i) > 1) {
			keyValues.Emplace(*(&i - 1), *(&i + 1));
		}
	}
}
UnityMetaReader::~UnityMetaReader() {}
vstd::string_view UnityMetaReader::Get(vstd::string_view name) const {
	auto ite = keyValues.Find(name);
	if (!ite) return vstd::string_view();
	return ite.Value();
}
}// namespace Unity

#ifdef EXPORT_UNITY_FUNCTION
#include <Utility/VGuid.h>
VENGINE_UNITY_EXTERN void parse_unity_metafile(
	uint8_t const* fileData,
	uint64 fileSize,
	vstd::Guid* guid) {
	using namespace Unity;
	auto words = detail::SeparateMetaFile(vstd::string_view(reinterpret_cast<char const*>(fileData), fileSize));
	for (auto&& i : words) {
		if (i != ':') continue;
		if ((&i - words.begin()) > 0 && (words.end() - &i) > 1) {
			if (*(&i - 1) == "guid") {
				*guid = *(&i + 1);
				return;
			}
		}
	}
	guid->Reset();
}
#endif