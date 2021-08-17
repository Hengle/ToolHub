#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Yaml/IYamlNode.h>
#include <Common/DynamicDLL.h>
int main() {
	vengine_init_malloc();
	//(*node)["guid"]
	/* YAML::Node config = YAML::LoadFile("Binaries.meta");
	auto vv = config[vstd::string_view("guid")];
	std::cout << vv.as<vstd::string_view>() << '\n';
	std::cout << config.size() << '\n';
	auto iterateFunc = [&](auto const& selfFunc, YAML::Node const& node) -> void {
		for (auto&& i : node) {
			if (!i.second.IsScalar()) {
				if (i.second.IsMap()) {
					selfFunc(selfFunc, i.second);
				} else {
					std::cout << i.first  << ": \n";
				}
			} else {
				std::cout << i.first.as<std::string>() << ": "
						  << i.second.as<std::string>() << '\n';
			}
		}
	};
	iterateFunc(iterateFunc, config);*/

	return 0;
}