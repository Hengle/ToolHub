#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
namespace toolhub::db {

class SimpleBinaryJson : public IJsonDataBase {

public:
	template<typename T>
	struct JsonObj {
		HashMap<uint64, T*> map;
		HashMap<uint64, std::span<uint8_t>> unserData;
		std::mutex mtx;
		uint64 count = 0;
		Pool<T> alloc;
		JsonObj() : alloc(256) {}
		template<typename... Args>
		T* Create(SimpleBinaryJson* db, Args&&... args) {
			T* ptr = alloc.New(std::forward<Args>(args)...);
			uint64 curCount;
			mtx.lock();
			auto ite = map.Emplace(++count, ptr);
			curCount = count;
			mtx.unlock();
			ptr->instanceID = curCount;
			ptr->db = db;
			return ptr;
		}
		template<typename... Args>
		T* Create(uint64 instanceID, SimpleBinaryJson* db, Args&&... args) {
			T* ptr = alloc.New(std::forward<Args>(args)...);
			mtx.lock();
			auto ite = map.Emplace(instanceID, ptr);
			mtx.unlock();
			ptr->instanceID = instanceID;
			ptr->db = db;
			return ptr;
		}

		void Remove(T* ptr) {
			std::lock_guard lck(mtx);
			auto ite = map.Find(ptr->instanceID);
#ifdef DEBUG
			if (!ite || ite.Value() != ptr) {
				VEngine_Log("Error Remove!\n");
				VENGINE_EXIT;
			}
#endif
			map.Remove(ite);
		}
	};

	JsonObj<SimpleJsonArray> arrObj;
	JsonObj<SimpleJsonDict> dictObj;
	SimpleJsonDict rootObj;
	vstd::vector<uint8_t> vec;
	SimpleBinaryJson();
	IJsonDict* GetRootObject() override;
	IJsonDict* CreateJsonObject() override;
	IJsonArray* CreateJsonArray() override;
	void Dispose(IJsonDict* jsonObj) override;
	void Dispose(IJsonArray* jsonArr) override;
	struct SerializeHeader {
		uint64 arrID;
		uint64 dictID;
		uint64 arrCount;
		uint64 dictCount;
	};
	SerializeHeader GetHeader() const;

	vstd::string GetSerializedString() override;
	vstd::vector<uint8_t> Save() override;
	void Read(vstd::vector<uint8_t>&& data) override;
};
}