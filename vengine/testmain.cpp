#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Common/DynamicDLL.h>
#include <Utility/BinaryReader.h>
#include <Database/DatabaseInclude.h>
#include <Database/DatabaseExample.h>
template<typename FactoryType>
struct DllFactoryLoader {
private:
	vstd::optional<DynamicDLL> dll;
	funcPtr_t<FactoryType*()> funcPtr;

public:
	DllFactoryLoader(
		char const* dllName,
		char const* factoryFuncName) {
		dll.New(dllName);
		funcPtr = dll->GetDLLFunc<FactoryType*()>(factoryFuncName);
	}
	void UnloadDll() {
		dll.Delete();
	}
	FactoryType* operator()() const {
		return funcPtr();
	}
};

int main() {
	DllFactoryLoader<toolhub::db::Database> loader("VEngine_Database.dll", "Database_GetFactory");
	auto factory = loader();
	jsonTest(factory);
	return 0;
}