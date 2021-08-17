#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Yaml/yaml-cpp/yaml.h>
int main() {
	vengine_init_malloc();
	YAML::Node config = YAML::LoadFile("Binaries.meta");
	std::cout << (config.Type() == YAML::NodeType::Map) << '\n';
	for (auto&& i : config) {
		if (i.IsValid())
		std::cout << i.as<std::string>() << '\n';
	}
	return 0;
}