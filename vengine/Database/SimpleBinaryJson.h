#pragma once
#include <Database/SimpleJsonDict.h>
#include <Database/SimpleJsonArray.h>
namespace toolhub::db {

class SimpleBinaryJson : public IJsonDataBase {

public:
	template<typename T>
	struct JsonObj {
		vstd::vector<vstd::variant<uint64, T*>> vec;
		std::mutex mtx;
		Pool<T> alloc;
		JsonObj() : alloc(256) {}

		void Clear() {
			std::lock_guard lck(mtx);
			for (auto&& i : vec) {
				i.visit(
					[](auto&&) {},
					[](auto&& ptr) {
						alloc.Delete(ptr);
					});
			}
			vec.clear();
		}

		template<typename... Args>
		T* TryCreate(uint64 version, uint64 instanceID, SimpleBinaryJson* db, Args&&... args) {
			std::lock_guard lck(mtx);
			if (vec.size() < instanceID) {
				vec.resize(instanceID);
			}
			auto&& v = vec[instanceID];
			if (v.GetType() == 1) {
				auto&& obj = *reinterpret_cast<T**>(v.GetPlaceHolder());
				if (obj->jsonObj.version < version) {
					alloc.Delete(obj);
					obj = nullptr;
				} else {
					obj->jsonObj.db = db;
					return obj;
				}
			}
			T* ptr = alloc.New(std::forward<Args>(args)...);
			v = ptr;
			ptr->jsonObj.version = version;
			ptr->jsonObj.instanceID = instanceID;
			ptr->jsonObj.db = db;
			return ptr;
		}

		template<typename... Args>
		T* TryCreate(uint64 version, SimpleBinaryJson* db, Args&&... args) {
			std::lock_guard lck(mtx);
			auto instanceID = vec.size();
			T* ptr = alloc.New(std::forward<Args>(args)...);
			vec.emplace_back(ptr);
			ptr->jsonObj.version = version;
			ptr->jsonObj.instanceID = instanceID;
			ptr->jsonObj.db = db;
			return ptr;
		}

		void Remove(T* ptr, SimpleBinaryJson* db) {
			std::lock_guard lck(mtx);
			auto id = ptr->jsonObj.instanceID;
			auto error = []() {
				VEngine_Log("Error Remove!\n");
				VENGINE_EXIT;
			};
			if (id < vec.size()) {
				auto&& v = vec[id];
				if (v.GetType() == 1) {
					auto&& obj = *reinterpret_cast<T**>(v.GetPlaceHolder());
					if (obj != ptr) {
						error();
					}
					alloc.Delete(obj);
					v = db->version;
				} else {
					error();
				}
			}
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
		uint64 arrCount;
		uint64 dictCount;
		uint64 version;
		uint64 rootVersion;
	};
	uint64 version = 0;
	SerializeHeader GetHeader() const;

	vstd::string GetSerializedString() override;
	vstd::vector<uint8_t> Serialize() override;
	uint64 GetVersion() override { return version; }

	vstd::vector<uint8_t> IncreSerialize(uint64 version) override;
	void Read(vstd::vector<uint8_t>&& data) override;
};
}// namespace toolhub::db