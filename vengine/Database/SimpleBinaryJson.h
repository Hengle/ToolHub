#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
namespace toolhub::db {

class SimpleBinaryJson final : public IJsonDataBase {
private:
	void Ser_CreateObj(
		uint64 instanceID,
		std::span<uint8_t> sp,
		uint8_t targetType,
		vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs);
	bool Ser_PopValue(
		std::span<uint8_t>& sp,
		std::span<uint8_t>& rootChunk,
		vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs);

public:
	using ObjMap = HashMap<uint64, std::pair<SimpleJsonObject*, uint8_t>>;

	void DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& data);
	bool DisposeProperty(ObjMap::Index data, SimpleJsonObject* obj);
	void Dispose(uint64 instanceID);
	void Dispose(uint64 instanceID, IDatabaseEvtVisitor* evtVisitor);
	void MarkDirty(SimpleJsonObject* dict);
	void MarkDelete(SimpleJsonObject* dict);

	ObjMap jsonObjs;
	vstd::vector<vstd::variant<SimpleJsonObject*, uint64>> updateVec;
	Pool<SimpleJsonArray> arrPool;
	Pool<SimpleJsonDict> dictPool;
	SimpleJsonDict rootObj;
	uint64 instanceCount = 0;
	SimpleBinaryJson();
	IJsonDict* GetRootObject() override;
	IJsonDict* CreateJsonObject() override;
	IJsonArray* CreateJsonArray() override;

	IJsonDict* GetJsonObject(uint64 id) override;
	IJsonArray* GetJsonArray(uint64 id) override;

	bool Dispose(IJsonDict* jsonObj) override;
	bool Dispose(IJsonArray* jsonArr) override;
	vstd::vector<uint8_t> IncreSerialize() override;

	struct SerializeHeader {
		uint64 instanceCount;
	};
	SerializeHeader GetHeader() const {
		return {instanceCount};
	}

	vstd::vector<uint8_t> Serialize() override;

	void Read(std::span<uint8_t> data) override;
	void Read(
		std::span<uint8_t> data,
		IDatabaseEvtVisitor* evtVisitor) override;
	KILL_COPY_CONSTRUCT(SimpleBinaryJson)
	KILL_MOVE_CONSTRUCT(SimpleBinaryJson)
};
}// namespace toolhub::db