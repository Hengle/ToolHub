#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Database/IJsonDatabase.h>
#include <Database/JsonObject.h>
namespace toolhub::db {
class SimpleBinaryJson;
enum class ValueType : uint8_t {
	Int,
	Float,
	Bool,
	String,
	Dict,
	Array
};
template<typename T>
T PopValue(std::span<uint8_t>& arr) {
	T* ptr = reinterpret_cast<T*>(arr.data());
	arr = std::span<uint8_t>(arr.data() + sizeof(T), arr.size() - sizeof(T));
	return T(std::move(*ptr));
}
ValueType type;
class SimpleJsonLoader {
	std::atomic_bool loaded;
	std::mutex mtx;

public:
	std::span<uint8_t> dataChunk;
	//TODO
	SimpleJsonLoader()
		: loaded(false) {
	}
	template<typename T>
	void Load(T&& t) {
		if (loaded.load(std::memory_order_acquire)) return;
		std::lock_guard lck(mtx);
		if (loaded.load(std::memory_order_acquire)) return;
		if (!dataChunk.empty())
			t.Load(dataChunk);
		loaded.store(true, std::memory_order_release);
	}
	static JsonVariant Serialize(std::span<uint8_t>& arr, SimpleBinaryJson* db);
};
class SimpleJsonArray : public IJsonArray {
public:
	vstd::vector<JsonVariant> arrs;
	SimpleJsonLoader loader;
	uint64 instanceID;
	SimpleBinaryJson* db;
	void ExecuteLoad() {
		loader.Load(*this);
	}
	//Array DeSerialize
	void Load(std::span<uint8_t> sp) {
		uint64 arrSize = PopValue<uint64>(sp);
		arrs.reserve(arrSize);
		arrs.push_back_func(
			[&](size_t i) {
				return SimpleJsonLoader::Serialize(sp, db);
			},
			arrSize);
	}
	size_t Length() override {
		ExecuteLoad();
		return arrs.size();
	}
	JsonVariant Get(size_t index) override {
		ExecuteLoad();
		return arrs[index];
	}
	void Set(size_t index, JsonVariant value) override {
		ExecuteLoad();
		arrs[index] = std::move(value);
	}
	void Remove(size_t index) override {
		ExecuteLoad();
		arrs.erase(arrs.begin() + index);
	}
	void Add(JsonVariant value) override {
		ExecuteLoad();
		arrs.emplace_back(std::move(value));
	}
	vstd::linq::Iterator<JsonVariant>* GetIterator() override {
		ExecuteLoad();
		return new vstd::linq::IEnumerator(arrs);
	}
};
class SimpleJsonDict : public IJsonDict {
public:
	HashMap<vstd::string, JsonVariant> vars;
	SimpleJsonLoader loader;
	uint64 instanceID;
	SimpleBinaryJson* db;
	void ExecuteLoad() {
		loader.Load(*this);
	}
	//Dict Deserialize
	void Load(std::span<uint8_t> sp) {
		uint64 arrSize = PopValue<uint64>(sp);
		vars.reserve(arrSize);
		auto GetNextKeyValue = [&]() {
			uint64 strSize = PopValue<uint64>(sp);
			auto strv = vstd::string_view((char const*)sp.data(), strSize);
			sp = std::span<uint8_t>(sp.data() + strSize, sp.size() - strSize);
			return std::pair<vstd::string, JsonVariant>(strv, SimpleJsonLoader::Serialize(sp, db));
		};
		for (auto i : vstd::range(arrSize)) {
			auto kv = GetNextKeyValue();
			vars.Emplace(std::move(kv.first), std::move(kv.second));
		}
	}
	JsonVariant Get(vstd::string_view key) override {
		auto ite = vars.Find(key);
		return ite ? ite.Value() : JsonVariant();
	}
	void Set(vstd::string key, JsonVariant value) override {
		vars.ForceEmplace(std::move(key), std::move(value));
	}
	void Remove(vstd::string const& key) override {
		vars.Remove(key);
	}
	vstd::linq::Iterator<JsonKeyPair>* GetIterator() override {
		return vstd::linq::IEnumerator(vars)
			.make_transformer(
				[](auto&& kv) {
					return JsonKeyPair{kv.first, kv.second};
				})
			.MoveNew();
	}
};

class SimpleBinaryJson : public IJsonDataBase {
private:
	template<typename T>
	struct JsonObj {
		HashMap<uint64, T*> map;
		HashMap<uint64, std::span<uint8_t>> unserData;
		std::mutex mtx;
		uint64 count = 0;
		Pool<T> alloc;
		JsonObj() : alloc(256) {}
		template<typename... Args>
		T* Create(Args&&... args, SimpleBinaryJson* db) {
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

public:
	vstd::optional<SimpleJsonDict> rootObj;
	SimpleBinaryJson() {
		//TODO: root
	}
	IJsonDict* GetRootObject() override {
		return rootObj ? rootObj.GetPtr() : nullptr;
	}
	IJsonDict* CreateJsonObject() override {
		auto dict = dictObj.Create(this);
		return dict;
	}
	IJsonArray* CreateJsonArray() override {
		return arrObj.Create(this);
	}
	void Dispose(IJsonDict* jsonObj) override {
		dictObj.Remove(static_cast<SimpleJsonDict*>(jsonObj));
	}
	void Dispose(IJsonArray* jsonArr) override {
		arrObj.Remove(static_cast<SimpleJsonArray*>(jsonArr));
	}
	void Save() override {}
};
JsonVariant SimpleJsonLoader::Serialize(std::span<uint8_t>& arr, SimpleBinaryJson* db) {
	ValueType type = PopValue<ValueType>(arr);
	switch (type) {
		case ValueType::Int: {
			int64 v = PopValue<int64>(arr);
			return JsonVariant(v);
		}
		case ValueType::Float: {
			double d = PopValue<double>(arr);
			return JsonVariant(d);
		}
		case ValueType::Bool: {
			bool b = PopValue<bool>(arr);
			return JsonVariant(b);
		}
		case ValueType::String: {
			uint64 strSize = PopValue<uint64>(arr);
			auto strv = vstd::string_view((char const*)arr.data(), strSize);
			arr = std::span<uint8_t>(arr.data() + strSize, arr.size() - strSize);
			return JsonVariant(strv);
		}
		case ValueType::Dict: {
			uint64 arrSize = PopValue<uint64>(arr);
			auto newSpan = std::span<uint8_t>(arr.data(), arrSize);
			arr = std::span<uint8_t>(arr.data() + arrSize, arr.size() - arrSize);
			auto dictObj = static_cast<SimpleJsonDict*>(db->CreateJsonObject());
			dictObj->loader.dataChunk = newSpan;
			return JsonVariant(dictObj);
		}
		case ValueType::Array: {
			uint64 arrSize = PopValue<uint64>(arr);
			auto newSpan = std::span<uint8_t>(arr.data(), arrSize);
			arr = std::span<uint8_t>(arr.data() + arrSize, arr.size() - arrSize);
			auto arrayObj = static_cast<SimpleJsonArray*>(db->CreateJsonArray());
			arrayObj->loader.dataChunk = newSpan;
			return JsonVariant(arrayObj);
		}
		default:
			return JsonVariant();
	}
}
}// namespace toolhub::db