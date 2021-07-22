#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/Pool.h>
#include <CJsonObject/CJsonObject.hpp>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

class SimpleBinaryJson;

SimpleBinaryJson::SimpleBinaryJson() {
	//TODO: root
	rootObj.instanceID = 0;
}
IJsonDict* SimpleBinaryJson::GetRootObject() {
	return &rootObj;
}
IJsonDict* SimpleBinaryJson::CreateJsonObject() {
	auto dict = dictObj.Create(this);
	return dict;
}
IJsonArray* SimpleBinaryJson::CreateJsonArray() {
	return arrObj.Create(this);
}
void SimpleBinaryJson::Dispose(IJsonDict* jsonObj) {
	dictObj.Remove(static_cast<SimpleJsonDict*>(jsonObj));
}
void SimpleBinaryJson::Dispose(IJsonArray* jsonArr) {
	arrObj.Remove(static_cast<SimpleJsonArray*>(jsonArr));
}

SimpleBinaryJson::SerializeHeader SimpleBinaryJson::GetHeader() const {
	return {
		arrObj.count,
		dictObj.count,
		arrObj.map.size(),
		dictObj.map.size()};
}

vstd::string SimpleBinaryJson::GetSerializedString() {
	using namespace neb;
	CJsonObject rootObj;
	{
		CJsonObject headerObj;
		auto header = GetHeader();
		headerObj.Add(header.arrID);
		headerObj.Add(header.dictID);
		headerObj.Add(header.arrCount);
		headerObj.Add(header.dictCount);
		rootObj.Add("header"_sv, headerObj);
	}
	{
		CJsonObject arrJsons;
		for (auto&& i : arrObj.map) {
			CJsonObject arrJson;
			for (auto&& v : i.second->arrs) {
				auto func =
					[&](auto&& o) {
						arrJson.Add(o);
					};
				v.visit(
					func,
					func,
					func,
					[&](IJsonDict* dict) {
						arrJson.Add('d' + vstd::to_string(static_cast<SimpleJsonDict*>(dict)->instanceID));
					},
					[&](IJsonArray* dict) {
						arrJson.Add('a' + vstd::to_string(static_cast<SimpleJsonArray*>(dict)->instanceID));
					});
			}
			arrJsons.Add(vstd::to_string(i.first), arrJson);
		}
		rootObj.Add("arrays"_sv, arrJsons);
	}
	auto PrintDict = [&](CJsonObject& dictJson, SimpleJsonDict* dict) {
		for (auto&& v : dict->vars) {
			auto func =
				[&](auto&& o) {
					dictJson.Add(v.first, o);
				};
			v.second.visit(
				func,
				func,
				func,
				[&](IJsonDict* d) {
					dictJson.Add(v.first, 'd' + vstd::to_string(static_cast<SimpleJsonDict*>(d)->instanceID));
				},
				[&](IJsonArray* d) {
					dictJson.Add(v.first, 'a' + vstd::to_string(static_cast<SimpleJsonArray*>(d)->instanceID));
				});
		}
	};
	{
		CJsonObject dictJsons;
		for (auto&& i : dictObj.map) {
			CJsonObject dictJson;
			PrintDict(dictJson, i.second);
			dictJsons.Add(vstd::to_string(i.first), dictJson);
		}
		rootObj.Add("dicts"_sv, dictJsons);
	}
	{
		CJsonObject rootData;
		PrintDict(rootData, &this->rootObj);
		rootObj.Add("root"_sv, rootData);
	}
	return rootObj.ToFormattedString();
}
vstd::vector<uint8_t> SimpleBinaryJson::Save() {
	vstd::vector<uint8_t> serData;
	serData.reserve(65536);
	auto Push = [&]<typename T>(T&& v) {
		PushDataToVector<T>(std::forward<T>(v), serData);
	};
	auto Reserve = [&](size_t i) {
		auto lastLen = serData.size();
		serData.resize(lastLen + i);
		return lastLen;
	};
	auto Set = [&]<typename T>(T&& t, size_t offset) {
		using TT = std::remove_cvref_t<decltype(t)>;
		*reinterpret_cast<TT*>(serData.data() + offset) = t;
	};

	auto PushVariant = [&](JsonVariant const& v) {
		SimpleJsonLoader::Serialize(v, serData);
	};
	auto PushObj = [&](auto&& arr) {
		arr->M_GetSerData(serData);
	};
	Push(GetHeader());
	/////////////// Root Obj
	PushObj(&rootObj);
	/////////////// Array Obj
	Push.operator()<uint64>(arrObj.map.size());
	for (auto&& i : arrObj.map) {
		PushObj(i.second);
	}
	/////////////// Dict Obj
	Push.operator()<uint64>(dictObj.map.size());
	for (auto&& i : dictObj.map) {
		PushObj(i.second);
	}
	return serData;
}
void SimpleBinaryJson::Read(vstd::vector<uint8_t>&& data) {
	vec = std::move(data);
	std::span<uint8_t> sp = vec;
	SerializeHeader header = PopValue<SerializeHeader>(sp);
	arrObj.count = header.arrCount;
	dictObj.count = header.dictCount;
	arrObj.map.reserve(header.arrID);
	dictObj.map.reserve(header.dictID);
	auto PopObj = [&](auto&& createFunc) {
		uint64 instanceID = PopValue<uint64>(sp);
		uint64 spanSize = PopValue<uint64>(sp);
		auto dict = createFunc(instanceID, std::span<uint8_t>(sp.data(), spanSize));
		sp = std::span<uint8_t>(sp.data() + spanSize, sp.size() - spanSize);
	};
	PopObj([&](uint64 instanceID, std::span<uint8_t> sp) {
		rootObj.instanceID = instanceID;
		rootObj.loader.Reset();
		rootObj.loader.dataChunk = sp;
		rootObj.db = this;
		return &rootObj;
	});
	auto CreateObj = [&](uint64 instanceID, std::span<uint8_t> sp, auto&& map) {
		auto v = map.Create(instanceID, this);
		v->loader.Reset();
		v->loader.dataChunk = sp;
		return v;
	};
	auto arrayCount = PopValue<uint64>(sp);
	for (auto i : vstd::range(arrayCount)) {
		PopObj([&](uint64 instanceID, std::span<uint8_t> sp) {
			return CreateObj(instanceID, sp, arrObj);
		});
	}
	auto dictCount = PopValue<uint64>(sp);
	for (auto i : vstd::range(arrayCount)) {
		PopObj([&](uint64 instanceID, std::span<uint8_t> sp) {
			return CreateObj(instanceID, sp, dictObj);
		});
	}
}

IJsonDataBase* CreateSimpleJsonDB() {
	return new SimpleBinaryJson();
}
}// namespace toolhub::db