#pragma vengine_package vengine_database

#include <Database/SimpleBinaryJson.h>
#include <Database/SimpleJsonValue.h>
namespace toolhub::db {
enum class CSharpKeyType : uint {
	Int64,
	String,
	Guid
};
enum class CSharpValueType : uint {
	Int64,
	Double,
	String,
	Dict,
	Array,
	Guid

};
VENGINE_UNITY_EXTERN void db_get_new(SimpleBinaryJson** pp) {
	*pp = new SimpleBinaryJson();
}
VENGINE_UNITY_EXTERN void db_dispose(SimpleBinaryJson* p) {
	p->Dispose();
}
VENGINE_UNITY_EXTERN void db_get_rootnode(SimpleBinaryJson* db, SimpleJsonValueDict** pp) {
	*pp = static_cast<SimpleJsonValueDict*>(db->GetRootNode());
}
VENGINE_UNITY_EXTERN void db_create_dict(SimpleBinaryJson* db, SimpleJsonValueDict** pp) {
	*pp = db->CreateDict_Nake();
}
VENGINE_UNITY_EXTERN void db_create_array(SimpleBinaryJson* db, SimpleJsonValueArray** pp) {
	*pp = db->CreateArray_Nake();
}
VENGINE_UNITY_EXTERN void db_serialize(SimpleBinaryJson* db, funcPtr_t<void(uint8_t*, uint64)> callback) {
	auto vec = db->Serialize();
	callback(vec.data(), vec.size());
}
VENGINE_UNITY_EXTERN void db_deser(SimpleBinaryJson* db, uint8_t* ptr, uint64 len) {
	db->Read(std::span<uint8_t const>(ptr, len));
}
VENGINE_UNITY_EXTERN void db_dispose_arr(SimpleJsonValueArray* p) {
	p->Dispose();
}
////////////////// Dict Area
using DictIterator = decltype(std::declval<SimpleJsonValueDict>().vars)::Iterator;
using ArrayIterator = decltype(std::declval<SimpleJsonValueArray>().arr.begin());

VENGINE_UNITY_EXTERN void db_dispose_dict(SimpleJsonValueDict* ptr) {
	ptr->Dispose();
}
Key GetCSharpKey(void* ptr, CSharpKeyType keyType) {
	switch (keyType) {
		case CSharpKeyType::Int64:
			return Key(*reinterpret_cast<int64*>(ptr));
		case CSharpKeyType::Guid:
			return Key(*reinterpret_cast<vstd::Guid*>(ptr));
		case CSharpKeyType::String:
			return Key(*reinterpret_cast<vstd::string_view*>(ptr));
		default:
			return Key();
	}
}
void SetCSharpKey(void* ptr, CSharpKeyType keyType, Key const& key) {
	switch (keyType) {
		case CSharpKeyType::Int64: {
			*reinterpret_cast<int64*>(ptr) =
				(key.IsTypeOf<int64>())
					? key.force_get<int64>()
					: 0;
		} break;
		case CSharpKeyType::Guid: {
			*reinterpret_cast<vstd::Guid*>(ptr) =
				(key.IsTypeOf<vstd::Guid>())
					? key.force_get<vstd::Guid>()
					: vstd::Guid(false);
		} break;
		case CSharpKeyType::String: {
			*reinterpret_cast<vstd::string_view*>(ptr) =
				(key.IsTypeOf<vstd::string_view>())
					? key.force_get<vstd::string_view>()
					: vstd::string_view(nullptr, (size_t)0);
		} break;
	}
}
WriteJsonVariant GetCSharpWriteValue(void* ptr, CSharpValueType valueType) {
	switch (valueType) {
		case CSharpValueType::Array:
			return WriteJsonVariant(vstd::unique_ptr<IJsonArray>(*reinterpret_cast<SimpleJsonValueArray**>(ptr)));
		case CSharpValueType::Dict:
			return WriteJsonVariant(vstd::unique_ptr<IJsonDict>(*reinterpret_cast<SimpleJsonValueDict**>(ptr)));
		case CSharpValueType::Double:
			return WriteJsonVariant(*reinterpret_cast<double*>(ptr));
		case CSharpValueType::Guid:
			return WriteJsonVariant(*reinterpret_cast<vstd::Guid*>(ptr));
		case CSharpValueType::Int64:
			return WriteJsonVariant(*reinterpret_cast<int64*>(ptr));
		case CSharpValueType::String:
			return WriteJsonVariant(*reinterpret_cast<vstd::string_view*>(ptr));
		default:
			return WriteJsonVariant();
	}
}
void SetCSharpReadValue(void* ptr, CSharpValueType valueType, ReadJsonVariant const& readValue) {
	switch (valueType) {

		case CSharpValueType::Array:
			*reinterpret_cast<SimpleJsonValueArray**>(ptr) =
				(readValue.IsTypeOf<IJsonArray*>())
					? (static_cast<SimpleJsonValueArray*>(readValue.force_get<IJsonArray*>()))
					: nullptr;
			break;

		case CSharpValueType::Dict:
			*reinterpret_cast<SimpleJsonValueDict**>(ptr) =
				(readValue.IsTypeOf<IJsonDict*>())
					? (static_cast<SimpleJsonValueDict*>(readValue.force_get<IJsonDict*>()))
					: nullptr;
			break;
		case CSharpValueType::Double:
			if (readValue.IsTypeOf<int64>()) {
				*reinterpret_cast<double*>(ptr) = readValue.force_get<int64>();
			} else if (readValue.IsTypeOf<double>()) {
				*reinterpret_cast<double*>(ptr) = readValue.force_get<double>();
			}
			break;
		case CSharpValueType::Guid:
			*reinterpret_cast<vstd::Guid*>(ptr) =
				(readValue.IsTypeOf<vstd::Guid>())
					? (readValue.force_get<vstd::Guid>())
					: vstd::Guid(false);
			break;
		case CSharpValueType::Int64:
			if (readValue.IsTypeOf<int64>()) {
				*reinterpret_cast<int64*>(ptr) = readValue.force_get<int64>();
			} else if (readValue.IsTypeOf<double>()) {
				*reinterpret_cast<int64*>(ptr) = readValue.force_get<double>();
			}
			break;
		case CSharpValueType::String:
			*reinterpret_cast<vstd::string_view*>(ptr) =
				(readValue.IsTypeOf<vstd::string_view>())
					? readValue.force_get<vstd::string_view>()
					: vstd::string_view(nullptr, (size_t)0);
			break;
	}
}

VENGINE_UNITY_EXTERN void db_dict_set(
	SimpleJsonValueDict* dict,
	void* keyPtr,
	CSharpKeyType keyType,
	void* valuePtr,
	CSharpValueType valueType) {
	Key key;
	WriteJsonVariant value;
	dict->Set(GetCSharpKey(keyPtr, keyType), GetCSharpWriteValue(valuePtr, valueType));
}
VENGINE_UNITY_EXTERN void db_dict_get(
	SimpleJsonValueDict* dict,
	void* keyPtr,
	CSharpKeyType keyType,
	CSharpValueType targetValueType,
	void* valuePtr) {
	SetCSharpReadValue(valuePtr, targetValueType, dict->Get(GetCSharpKey(keyPtr, keyType)));
}
VENGINE_UNITY_EXTERN void db_dict_remove(SimpleJsonValueDict* dict, void* keyPtr, CSharpKeyType keyType) {
	dict->Remove(GetCSharpKey(keyPtr, keyType));
}
VENGINE_UNITY_EXTERN void db_dict_len(SimpleJsonValueDict* dict, int32* sz) { *sz = dict->Length(); }
VENGINE_UNITY_EXTERN void db_dict_itebegin(SimpleJsonValueDict* dict, DictIterator* ptr) { *ptr = dict->vars.begin(); }
VENGINE_UNITY_EXTERN void db_dict_iteend(SimpleJsonValueDict* dict, DictIterator* end, bool* result) { *result = (*end == dict->vars.end()); }
VENGINE_UNITY_EXTERN void db_dict_ite_next(DictIterator* end) { (*end)++; }
VENGINE_UNITY_EXTERN void db_dict_ite_get(DictIterator ite, void* valuePtr, CSharpValueType valueType) {
	SetCSharpReadValue(valuePtr, valueType, ite->second.GetVariant());
}

VENGINE_UNITY_EXTERN void db_dict_ite_getkey(DictIterator ite, void* keyPtr, CSharpKeyType keyType) {
	SetCSharpKey(keyPtr, keyType, ite->first.GetKey());
}
////////////////// Array Area
VENGINE_UNITY_EXTERN void db_arr_len(SimpleJsonValueArray* arr, int32* sz) {
	*sz = arr->Length();
}
VENGINE_UNITY_EXTERN void db_arr_get_value(SimpleJsonValueArray* arr, int32 index, void* valuePtr, CSharpValueType valueType) {
	SetCSharpReadValue(valuePtr, valueType, arr->Get(index));
}
VENGINE_UNITY_EXTERN void db_arr_set_value(SimpleJsonValueArray* arr, int32 index, void* valuePtr, CSharpValueType valueType) {
	arr->Set(index, GetCSharpWriteValue(valuePtr, valueType));
}
VENGINE_UNITY_EXTERN void db_arr_add_value(SimpleJsonValueArray* arr, void* valuePtr, CSharpValueType valueType) {
	arr->Add(GetCSharpWriteValue(valuePtr, valueType));
}
VENGINE_UNITY_EXTERN void db_arr_remove(SimpleJsonValueArray* arr, int32 index) {
	arr->Remove(index);
}
VENGINE_UNITY_EXTERN void db_arr_itebegin(SimpleJsonValueArray* arr, ArrayIterator* ptr) {
	*ptr = arr->arr.begin();
}
VENGINE_UNITY_EXTERN void db_arr_iteend(SimpleJsonValueArray* arr, ArrayIterator* ptr, bool* result) {
	*result = (*ptr == arr->arr.end());
}
VENGINE_UNITY_EXTERN void db_arr_ite_next(SimpleJsonValueArray* arr, ArrayIterator* ptr) {
	(*ptr)++;
}
VENGINE_UNITY_EXTERN void db_arr_ite_get(ArrayIterator ite, void* valuePtr, CSharpValueType valueType) {
	SetCSharpReadValue(valuePtr, valueType, ite->GetVariant());
}

}// namespace toolhub::db