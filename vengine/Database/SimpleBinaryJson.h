#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
#include <Database/SimpleJsonValue.h>
#include <Utility/VGuid.h>
namespace toolhub::db {

class SimpleBinaryJson final : public IJsonSubDatabase, public vstd::IOperatorNewBase {
private:
	void Ser_CreateObj(
		vstd::Guid const& instanceID,
		std::span<uint8_t> sp,
		uint8_t targetType,
		vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs);
	bool Ser_PopValue(
		std::span<uint8_t>& sp,
		vstd::vector<std::pair<SimpleJsonObject*, std::span<uint8_t>>>& vecs);
	vstd::Guid index;
	IJsonDatabase* parent;
	bool enabled = true;

public:
	bool Enabled() const { return enabled; }
	using ObjMap = HashMap<vstd::Guid, std::pair<SimpleJsonObject*, uint8_t>>;
	vstd::Guid GetGUID() override { return index; }
	void DisposeProperty(std::pair<SimpleJsonObject*, uint8_t> const& data);
	void Dispose(ObjMap::Index id);
	void Dispose(ObjMap::Index id, IDatabaseEvtVisitor* evtVisitor);
	void MarkDirty(SimpleJsonObject* dict);
	void MarkDelete(SimpleJsonObject* dict);

	ObjMap jsonObjs;
	vstd::vector<vstd::variant<SimpleJsonObject*, vstd::Guid>> updateVec;
	Pool<SimpleJsonArray> arrPool;
	Pool<SimpleJsonDict> dictPool;
	Pool<SimpleJsonValueArray> arrValuePool;
	Pool<SimpleJsonValueDict> dictValuePool;
	vstd::Guid rootGuid;
	SimpleBinaryJson(vstd::Guid const& index, IJsonDatabase* parent);
	~SimpleBinaryJson();
	IJsonDatabase* GetParent() override { return parent; }
	IJsonRefDict* GetRootObject() override;
	IJsonRefDict* CreateJsonObject() override;
	IJsonRefArray* CreateJsonArray() override;
	IJsonRefDict* GetJsonObject(vstd::Guid const& id) override;
	IJsonRefArray* GetJsonArray(vstd::Guid const& id) override;
	ThreadTaskHandle CollectGarbage(ThreadPool* tPool) override;

	void Dispose(IJsonRefDict* jsonObj);
	void Dispose(IJsonRefArray* jsonArr);
	void Dispose() override;
	vstd::vector<uint8_t> IncreSerialize() override;
	vstd::vector<uint8_t> Serialize() override;

	void Read(
		std::span<uint8_t> data,
		IDatabaseEvtVisitor* evtVisitor) override;
	KILL_COPY_CONSTRUCT(SimpleBinaryJson)
	KILL_MOVE_CONSTRUCT(SimpleBinaryJson)
};
}// namespace toolhub::db