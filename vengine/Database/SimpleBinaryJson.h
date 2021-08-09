#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
#include <Database/SimpleJsonValue.h>
namespace toolhub::db {

class SimpleBinaryJson final : public IJsonSubDatabase, public vstd::IOperatorNewBase {
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
	uint64 index;
	IJsonDatabase* parent;
	bool enabled = true;

public:
	bool Enabled() const { return enabled; }
	using ObjMap = HashMap<uint64, std::pair<SimpleJsonObject*, uint8_t>>;

	void DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& data);
	void Dispose(ObjMap::Index id);
	void Dispose(ObjMap::Index id, IDatabaseEvtVisitor* evtVisitor);
	void MarkDirty(SimpleJsonObject* dict);
	void MarkDelete(SimpleJsonObject* dict);

	ObjMap jsonObjs;
	vstd::vector<vstd::variant<SimpleJsonObject*, uint64>> updateVec;
	Pool<SimpleJsonArray> arrPool;
	Pool<SimpleJsonDict> dictPool;
	Pool<SimpleJsonValueArray> arrValuePool;
	Pool<SimpleJsonValueDict> dictValuePool;
	SimpleJsonDict rootObj;
	uint64 instanceCount = 0;
	SimpleBinaryJson(uint64 index, IJsonDatabase* parent);
	~SimpleBinaryJson();
	IJsonDatabase* GetParent() override { return parent; }
	IJsonRefDict* GetRootObject() override;
	IJsonRefDict* CreateJsonObject() override;
	IJsonRefArray* CreateJsonArray() override;
	uint64 GetIndex() override { return index; }
	IJsonRefDict* GetJsonObject(uint64 id) override;
	IJsonRefArray* GetJsonArray(uint64 id) override;

	void Dispose(IJsonRefDict* jsonObj);
	void Dispose(IJsonRefArray* jsonArr);
	void Dispose() override;
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