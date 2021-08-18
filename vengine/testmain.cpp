#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Yaml/IYamlNode.h>
#include <Common/DynamicDLL.h>
namespace Unity {
class UnityMetaReader : public vstd::IOperatorNewBase{
private:
	vstd::vector<vstd::string_view> words;
	HashMap<vstd::string_view, vstd::string_view> keyValues;

public:
	UnityMetaReader(
		char const* start,
		char const* end) {
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
		char const* lastPtr = start;
		for (auto i = start; i != end; ++i) {
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
			words.emplace_back(lastPtr, end);
		}
		for (auto&& i : words) {
			if (i != ':') continue;
			if ((&i - words.begin()) > 0 && (words.end() - &i) > 1) {
				keyValues.Emplace(*(&i - 1), *(&i + 1));
			}
		}
	}
	~UnityMetaReader() {}
	vstd::string_view Get(vstd::string_view name) const {
		auto ite = keyValues.Find(name);
		if (!ite) return vstd::string_view();
		return ite.Value();
	}
};
}// namespace Unity
#include <Utility/BinaryReader.h>
int main() {
	/* DynamicDLL dll("Yaml_CPP.dll");
	auto funcPtr = dll.GetDLLFunc<YAML::IYamlFactory*()>("Yaml_GetFactory");
	auto factory = funcPtr();
	auto node = factory->GetNodeFromFile("Binaries.meta");
	//*node->Get("guid") = "fuck";
	node->SaveNodeToFile("Binaries.meta");
	//std::cout << node->ToString() << '\n';
	*/
	BinaryReader reader("Binaries.meta");
	auto vec = reader.Read();
	Unity::UnityMetaReader guid((char const*)vec.begin(), (char const*)vec.end());
	auto str = guid.Get("guid");
	std::cout << str << '\n';

	return 0;
}