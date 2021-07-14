#pragma vengine_package vengine_dll
#include <CJsonObject/BinaryJson.h>
#include <Utility/BinaryReader.h>
void BinaryJson::DisposeSelf() {
	if (isArray) {
		if (!arrayDatas->datas.empty())
			vengine_free(arrayDatas->allocatedPtr);
		arrayDatas.Delete();
	} else {
		keyValueDatas.Delete();
	}
}
namespace SerializeStruct {
SerializedData::SerializedData(int64 intValue) : type(SerializeStruct::ObjectType::Int) {
	this->intValue = intValue;
}
SerializedData::SerializedData(double floatValue) : type(SerializeStruct::ObjectType::Float) {
	this->floatValue = floatValue;
}
SerializedData::SerializedData(bool value) : type(value ? SerializeStruct::ObjectType::True : SerializeStruct::ObjectType::False) {
}
SerializedData::SerializedData(char const* start, char const* end) : type(SerializeStruct::ObjectType::String) {
	this->str.New(start, end);
}
SerializedData::SerializedData(char const*& ptr, bool isArray) : type(isArray ? SerializeStruct::ObjectType::JsonArray : SerializeStruct::ObjectType::JsonObject) {
	obj.New(ptr, isArray);
}
void SerializedData::Dispose() {
	using namespace SerializeStruct;
	switch (type) {
		case ObjectType::JsonArray:
		case ObjectType::JsonObject:
			obj.Delete();
			break;
		case ObjectType::String:
			str.Delete();
			break;
	}
}
SerializedData::~SerializedData() {
	Dispose();
}
SerializedData::SerializedData(std::nullptr_t) : type(SerializeStruct::ObjectType::Null) {
}
}// namespace SerializeStruct
void BinaryJson::Parse(char const*& ptr, bool isArray) {
	using namespace SerializeStruct;
	uint64 elementCount = *Read<uint64>(ptr);
	if (!initialized) {
		initialized = true;
		if (isArray) {
			arrayDatas.New();
			arrayDatas->allocatedPtr = vengine_malloc(sizeof(SerializedData) * elementCount);
			arrayDatas->datas.resize(elementCount);
		} else {
			keyValueDatas.New(elementCount);
		}
	} else if (isArray != this->isArray) {
		return;
	}
	this->isArray = isArray;

	auto parseLambda = [&](SerializedData* dataPtr) -> void {
		switch (*Read<ObjectType>(ptr)) {
			case ObjectType::False:
				new (dataPtr) SerializedData(false);
				break;
			case ObjectType::True:
				new (dataPtr) SerializedData(true);
				break;
			case ObjectType::Null:
				new (dataPtr) SerializedData(nullptr);
				break;
			case ObjectType::String: {
				uint64 stringLen = *Read<uint64>(ptr);
				char const* end = ptr + stringLen;
				new (dataPtr) SerializedData(ptr, end);
				ptr = end;
			} break;
			case ObjectType::Int: {
				int64 intValue = *Read<int64>(ptr);
				new (dataPtr) SerializedData(intValue);
			} break;
			case ObjectType::Float: {
				double floatValue = *Read<double>(ptr);
				new (dataPtr) SerializedData(floatValue);
			} break;
			case ObjectType::JsonArray:
				new (dataPtr) SerializedData(ptr, true);
				break;
			case ObjectType::JsonObject:
				new (dataPtr) SerializedData(ptr, false);
				break;
		}
	};
	if (isArray) {
		for (uint64 i = 0; i < elementCount; ++i) {
			auto dataPtr = (SerializedData*)arrayDatas->allocatedPtr + i;
			arrayDatas->datas[i] = dataPtr;
			parseLambda(dataPtr);
		}
	} else {
		vstd::string str;
		for (uint64 i = 0; i < elementCount; ++i) {
			ObjectType type = *Read<ObjectType>(ptr);
			if (type != ObjectType::String) {
				throw "Invalid Format!";
			}
			uint64 stringLen = *Read<uint64>(ptr);
			str.clear();
			str.push_back_all(ptr, stringLen);
			ptr += stringLen;
			auto ite = keyValueDatas->Emplace(str, new SerializedData());
			if (ite.Value()->initialized) {
				ite.Value()->Dispose();
			}
			parseLambda(ite.Value().get());
		}
	}
}
BinaryJson::BinaryJson(ArrayList<char> const& data) {
	using namespace SerializeStruct;
	char const* ptr = data.data();
	ObjectType* typePtr = (ObjectType*)ptr;
	ptr += sizeof(ObjectType);
	if (*typePtr == ObjectType::JsonArray) {
		Parse(ptr, true);
	} else if (*typePtr == ObjectType::JsonObject) {
		Parse(ptr, false);
	} else {
		isArray = true;
		arrayDatas.New();
	}
}
BinaryJson::BinaryJson(vstd::vector<vstd::string> const& paths) {
	using namespace SerializeStruct;
	for (auto&& path : paths) {
		BinaryReader ifs(path);
		if (!ifs) {
			isArray = true;
			arrayDatas.New();

			return;
		}

		vstd::vector<char> data(ifs.GetLength());
		ifs.Read(data.data(), data.size());
		char const* ptr = data.data();
		ObjectType* typePtr = (ObjectType*)ptr;
		ptr += sizeof(ObjectType);
		if (*typePtr == ObjectType::JsonArray) {
			Parse(ptr, true);
		} else if (*typePtr == ObjectType::JsonObject) {
			Parse(ptr, false);
		} else {
			isArray = true;
			arrayDatas.New();

			return;
		}
	}
}
BinaryJson::BinaryJson(BinaryReader& ifs) {
	using namespace SerializeStruct;
	if (!ifs) {
		isArray = true;
		arrayDatas.New();
		return;
	}

	vstd::vector<char> data(ifs.GetLength());
	ifs.Read(data.data(), data.size());
	char const* ptr = data.data();
	ObjectType* typePtr = (ObjectType*)ptr;
	ptr += sizeof(ObjectType);
	if (*typePtr == ObjectType::JsonArray) {
		Parse(ptr, true);
	} else if (*typePtr == ObjectType::JsonObject) {
		Parse(ptr, false);
	} else {
		isArray = true;
		arrayDatas.New();
	}
}
BinaryJson::BinaryJson(vstd::string const& path) {
	using namespace SerializeStruct;
	BinaryReader ifs(path);
	if (!ifs) {
		isArray = true;
		arrayDatas.New();
		return;
	}

	vstd::vector<char> data(ifs.GetLength());
	ifs.Read(data.data(), data.size());
	char const* ptr = data.data();
	ObjectType* typePtr = (ObjectType*)ptr;
	ptr += sizeof(ObjectType);
	if (*typePtr == ObjectType::JsonArray) {
		Parse(ptr, true);
	} else if (*typePtr == ObjectType::JsonObject) {
		Parse(ptr, false);
	} else {
		isArray = true;
		arrayDatas.New();
	}
}
uint64 BinaryJson::GetArraySize() const {
	if (!isArray) return keyValueDatas->size();
	return arrayDatas->datas.size();
}
bool BinaryJson::Get(vstd::string const& name, vstd::string& str) const {
	if (isArray)
		return false;
	auto ite = keyValueDatas->Find(name);
	if (!ite)
		return false;
	if (ite.Value()->type != SerializeStruct::ObjectType::String) return false;
	str = *ite.Value()->str;
	return true;
}
bool BinaryJson::Get(vstd::string const& name, int64& intValue) const {
	if (isArray) return false;
	auto ite = keyValueDatas->Find(name);
	if (!ite) return false;
	if (ite.Value()->type == SerializeStruct::ObjectType::Int) {
		intValue = ite.Value()->intValue;
		return true;
	} else if (ite.Value()->type == SerializeStruct::ObjectType::Float) {
		intValue = (int64)ite.Value()->floatValue;
		return true;
	}
	return false;
}
bool BinaryJson::Get(vstd::string const& name, double& floatValue) const {
	if (isArray) return false;
	auto ite = keyValueDatas->Find(name);
	if (!ite) return false;
	if (ite.Value()->type == SerializeStruct::ObjectType::Int) {
		floatValue = (double)ite.Value()->intValue;
		return true;
	} else if (ite.Value()->type == SerializeStruct::ObjectType::Float) {
		floatValue = ite.Value()->floatValue;
		return true;
	}
	return false;
}
bool BinaryJson::Get(vstd::string const& name, bool& boolValue) const {
	if (isArray) return false;
	auto ite = keyValueDatas->Find(name);
	if (!ite) return false;
	uint8_t value = (uint8_t)ite.Value()->type - 3;
	if (value > 1) {
		return false;
	}
	boolValue = value;
	return true;
}
bool BinaryJson::Get(vstd::string const& name, BinaryJson*& objValue) const {
	if (isArray) return false;
	auto ite = keyValueDatas->Find(name);
	if (!ite) return false;
	uint8_t typeValue = (uint8_t)ite.Value()->type;
	if (typeValue > 1) return false;
	objValue = ite.Value()->obj;
	return true;
}
bool BinaryJson::ContainedKey(vstd::string const& name) {
	if (isArray) return false;
	return keyValueDatas->Find(name);
}
bool BinaryJson::Get(uint64 iWitch, vstd::string& str) const {
	if (!isArray) return false;
	auto&& ite = arrayDatas->datas[iWitch];
	if (ite->type != SerializeStruct::ObjectType::String) return false;
	str = *ite->str;
	return true;
}
bool BinaryJson::Get(uint64 iWitch, int64& intValue) const {
	if (!isArray) return false;
	auto&& ite = arrayDatas->datas[iWitch];
	if (ite->type == SerializeStruct::ObjectType::Int) {
		intValue = ite->intValue;
		return true;
	} else if (ite->type == SerializeStruct::ObjectType::Float) {
		intValue = (int64)ite->floatValue;
		return true;
	}
	return false;
}
bool BinaryJson::Get(uint64 iWitch, double& floatValue) const {
	if (!isArray) return false;
	auto&& ite = arrayDatas->datas[iWitch];
	if (ite->type == SerializeStruct::ObjectType::Int) {
		floatValue = (double)ite->intValue;
		return true;
	} else if (ite->type == SerializeStruct::ObjectType::Float) {
		floatValue = ite->floatValue;
		return true;
	}
	return false;
}
bool BinaryJson::Get(uint64 iWitch, bool& boolValue) const {
	if (!isArray) return false;
	auto&& ite = arrayDatas->datas[iWitch];
	uint8_t value = (uint8_t)ite->type - 3;
	if (value > 1) {
		return false;
	}
	boolValue = value;
	return true;
}
bool BinaryJson::Get(uint64 iWitch, BinaryJson*& objValue) const {
	if (!isArray) return false;
	auto&& ite = arrayDatas->datas[iWitch];
	uint8_t typeValue = (uint8_t)ite->type;
	if (typeValue > 1) return false;
	objValue = ite->obj;
	return true;
}
