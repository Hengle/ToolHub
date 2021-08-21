#pragma once
#pragma once
#include <Database/SimpleJsonValue.h>
#include <Utility/VGuid.h>
namespace toolhub::db {

class SimpleBinaryJson final : public IJsonDatabase, public vstd::IOperatorNewBase {
protected:
public:
	SimpleJsonValueDict root;
	SimpleBinaryJson();
	~SimpleBinaryJson();
	Pool<SimpleJsonValueArray, VEngine_AllocType::VEngine, true> arrValuePool;
	Pool<SimpleJsonValueDict, VEngine_AllocType::VEngine, true> dictValuePool;
	vstd::vector<uint8_t> Serialize() override;
	void Read(
		std::span<uint8_t> data) override;
	IJsonDict* GetRootNode() override;
	vstd::unique_ptr<IJsonDict> CreateDict() override;
	vstd::unique_ptr<IJsonArray> CreateArray() override;
	void Dispose() override {
		delete this;
	}
	KILL_COPY_CONSTRUCT(SimpleBinaryJson)
	KILL_MOVE_CONSTRUCT(SimpleBinaryJson)
};
}// namespace toolhub::db