#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Yaml/IYamlNode.h>
#include <Common/DynamicDLL.h>
#include <Utility/BinaryReader.h>
int main() {
	DynamicDLL dll("Yaml_CPP.dll");
	auto funcPtr = dll.GetDLLFunc<uint(funcPtr_t<void(vstd::string_view)> , char const*, uint)>("read_unity_file");
	vstd::string_view strv = "PostProcessResources.asset"_sv;
	uint v = funcPtr(
		[](vstd::string_view sv) {
			std::cout << "guid: " << sv << '\n';
		},
		strv.c_str(), strv.size());
	std::cout << "Result: " << v << '\n';

	//*node->Get("guid") = "fuck";
	//std::cout << node->ToString() << '\n';

	return 0;
}