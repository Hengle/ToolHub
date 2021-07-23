#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/Pool.h>
#include <CJsonObject/CJsonObject.hpp>
#include <Database/SimpleBinaryJson.h>
namespace toolhub::db {

class SimpleBinaryJson;

SimpleBinaryJson::SimpleBinaryJson() {
	rootObj.jsonObj.db = this;
}
IJsonDict* SimpleBinaryJson::GetRootObject() {
	return &rootObj;
}
IJsonDict* SimpleBinaryJson::CreateJsonObject() {
	auto dict = dictObj.TryCreate(version, this);
	return dict;
}
IJsonArray* SimpleBinaryJson::CreateJsonArray() {
	return arrObj.TryCreate(version, this);
}
void SimpleBinaryJson::Dispose(IJsonDict* jsonObj) {
	dictObj.Remove(static_cast<SimpleJsonDict*>(jsonObj), this);
}
void SimpleBinaryJson::Dispose(IJsonArray* jsonArr) {
	arrObj.Remove(static_cast<SimpleJsonArray*>(jsonArr), this);
}

SimpleBinaryJson::SerializeHeader SimpleBinaryJson::GetHeader() const {
	return {
		arrObj.vec.size(),
		dictObj.vec.size(),
		version,
		rootObj.jsonObj.version};
}

vstd::string SimpleBinaryJson::GetSerializedString() {
	using namespace neb;
	CJsonObject rootObj;
	{
		CJsonObject headerObj;
		auto header = GetHeader();
		headerObj.Add(header.arrCount);
		headerObj.Add(header.dictCount);
		rootObj.Add("header"_sv, headerObj);
	}
	{
		CJsonObject arrJsons;
		for (auto&& i : arrObj.vec) {
			if (i.GetType() != 1) continue;
			CJsonObject arrJson;
			auto obj = *reinterpret_cast<SimpleJsonArray**>(i.GetPlaceHolder());
			for (auto&& v : obj->arrs) {
				auto func =
					[&](auto&& o) {
						arrJson.Add(o);
					};
				v.visit(
					func,
					func,
					func,
					[&](IJsonDict* dict) {
						arrJson.Add('d' + vstd::to_string(static_cast<SimpleJsonDict*>(dict)->jsonObj.instanceID));
					},
					[&](IJsonArray* dict) {
						arrJson.Add('a' + vstd::to_string(static_cast<SimpleJsonArray*>(dict)->jsonObj.instanceID));
					});
			}
			arrJsons.Add(vstd::to_string(obj->jsonObj.instanceID), arrJson);
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
					dictJson.Add(v.first, 'd' + vstd::to_string(static_cast<SimpleJsonDict*>(d)->jsonObj.instanceID));
				},
				[&](IJsonArray* d) {
					dictJson.Add(v.first, 'a' + vstd::to_string(static_cast<SimpleJsonArray*>(d)->jsonObj.instanceID));
				});
		}
	};
	{
		CJsonObject dictJsons;
		for (auto&& i : dictObj.vec) {
			if (i.GetType() != 1) continue;
			CJsonObject dictJson;
			auto obj = *reinterpret_cast<SimpleJsonDict**>(i.GetPlaceHolder());
			PrintDict(dictJson, obj);
			dictJsons.Add(vstd::to_string(obj->jsonObj.instanceID), dictJson);
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
vstd::vector<uint8_t> SimpleBinaryJson::Serialize() {
	vstd::vector<uint8_t> serData;
	serData.reserve(65536);
	auto Push = [&]<typename T>(T&& v) {
		PushDataToVector<T>(std::forward<T>(v), serData);
	};

	auto PushObj = [&](auto&& arr) {
		arr->M_GetSerData(serData);
	};
	Push.operator()<uint8_t>(254);
	Push(GetHeader());
	/////////////// Root Obj
	PushObj(&rootObj);
	/////////////// Array Obj
	for (auto&& i : arrObj.vec) {
		i.visit(
			[](auto&&) {},
			PushObj
		);
	}
	/////////////// Dict Obj
	for (auto&& i : dictObj.vec) {
		i.visit(
			[](auto&&) {},
			PushObj);
	}
	Push(std::numeric_limits<uint8_t>::max());
	return serData;
}
vstd::vector<uint8_t> SimpleBinaryJson::IncreSerialize(uint64 version) {
	vstd::vector<uint8_t> serData;
	serData.reserve(1024);
	PushDataToVector<uint8_t>(253, serData);
	PushDataToVector(GetHeader(), serData);

	return serData;
}
void SimpleBinaryJson::Read(vstd::vector<uint8_t>&& data) {
	vec = std::move(data);
	std::span<uint8_t> sp = vec;
	auto serType = PopValue<uint8_t>(sp);
	auto CreateObj = [&](uint64 version, uint64 instanceID, std::span<uint8_t> sp, auto&& map) {
		auto v = map.TryCreate(version, instanceID, this);
		//TODO:version
		v->jsonObj.loader.Reset();
		v->jsonObj.loader.dataChunk = sp;
		return v;
	};
	auto CreateRoot = [&](std::span<uint8_t> sp) {
		rootObj.jsonObj.loader.Reset();
		rootObj.jsonObj.loader.dataChunk = sp;
		rootObj.jsonObj.db = this;
		return &rootObj;
	};
	auto DeserSubObj = [&]<bool isRoot>(auto&& map) {
		uint64 version = PopValue<uint64>(sp);
		uint64 instanceID = PopValue<uint64>(sp);
		uint64 spanSize = PopValue<uint64>(sp);
		if constexpr (isRoot) {
			if (instanceID == 0) {
				CreateRoot(sp);
				return;
			}
		} else {
			if (instanceID == 0) return;
		}
		CreateObj(version, instanceID, sp, map);
		sp = std::span<uint8_t>(sp.data() + spanSize, sp.size() - spanSize);
	};
	switch (serType) {
		//Array
		case 0: {
			DeserSubObj.operator()<false>(arrObj);
		} break;
		//Dict
		case 1: {
			DeserSubObj.operator()<true>(arrObj);
		} break;
		//Rebuild all
		case 254: {
			arrObj.vec.clear();
			dictObj.vec.clear();
			SerializeHeader header = PopValue<SerializeHeader>(sp);
			arrObj.vec.resize(header.arrCount);
			dictObj.vec.resize(header.dictCount);
			version = header.version;
			rootObj.jsonObj.version = header.rootVersion;
			auto PopObj = [&]<bool isRoot>() {
				uint8_t type = PopValue<uint8_t>(sp);
				if (type == std::numeric_limits<uint8_t>::max()) return false;
				uint64 version = PopValue<uint64>(sp);
				uint64 instanceID = PopValue<uint64>(sp);
				uint64 spanSize = PopValue<uint64>(sp);
				if constexpr (isRoot) {
					if (instanceID != 0 || type != 1) return false;
					CreateRoot(std::span<uint8_t>(sp.data(), spanSize));
				} else {

					switch (type) {
						//Array
						case 0:
							CreateObj(version, instanceID, sp, arrObj);
							break;
						//Dict
						case 1:
							CreateObj(version, instanceID, sp, dictObj);
							break;
					}
				}
				sp = std::span<uint8_t>(sp.data() + spanSize, sp.size() - spanSize);
				return true;
			};
			// Root
			PopObj.operator()<true>();
			while (PopObj.operator()<false>()) {}
		} break;
	}
}

IJsonDataBase* CreateSimpleJsonDB() {
	return new SimpleBinaryJson();
}
}// namespace toolhub::db