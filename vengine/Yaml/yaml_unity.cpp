#pragma vengine_package yaml_cpp
#include <IYamlNode.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/node.h>
#include <Utility/StringUtility.h>

VENGINE_UNITY_EXTERN uint read_unity_file(
	funcPtr_t<void(char const*, uint)> callBack,
	wchar_t const* filePath,
	uint filePathLen) {
	
	using namespace YAML;
	try {
		auto IterateFunc = [&](auto const& self, Node const& node) -> void {
			switch (node.Type()) {
				case NodeType::Sequence:
					for (auto&& i : node) {
						self(self, i.first);
					}
					break;
				case NodeType::Map: {
					//Find Guid Node:
					Node guidNode = node["guid"];
					if (guidNode.IsValid()
						&& guidNode.Type() == NodeType::Scalar) {
						auto strv = guidNode.as<vstd::string_view>();
						callBack(strv.c_str(), strv.size());
					}
					for (auto&& i : node) {
						self(self, i.second);
					}

				} break;
			}
		};
		std::string str;
		str.resize(filePathLen);
		StringUtil::TransformWCharToChar(filePath, str.data(), filePathLen);
		auto rootNode = LoadFile(str);
		if (rootNode.Type() != NodeType::Map) return 0;
		for (auto&& i : rootNode) {
			IterateFunc(IterateFunc, i.second);
		}
	} catch (ParserException e) {
		return 1;
	} catch (BadFile e) {
		return 2;
	}
	return 0;
}