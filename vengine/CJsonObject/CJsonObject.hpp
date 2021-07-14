/*******************************************************************************
 * Project:  neb
 * @file     CJsonObject.hpp
 * @brief    Json
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#ifndef CJSONOBJECT_HPP_
#define CJSONOBJECT_HPP_
#include <Common/vector.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string>
#include <fstream>
#include <list>
#include <CJsonObject/cJSON.h>
#include <Common/HashMap.h>
#include <Common/vstring.h>
#include <Common/Runnable.h>
#include <Common/VObject.h>
template<typename T, bool b>
class StackObject;
namespace neb {
class VENGINE_DLL_COMMON CJsonObject : public VObject{
public:// method of ordinary json object or json array
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
	CJsonObject();
	CJsonObject(const vstd::string& strJson);
	CJsonObject(const CJsonObject* pJsonObject);
	CJsonObject(const CJsonObject& oJsonObject);
	virtual ~CJsonObject();

	CJsonObject& operator=(const CJsonObject& oJsonObject);
	bool operator==(const CJsonObject& oJsonObject) const;
	bool Parse(const vstd::string& strJson);
	void Clear();
	bool IsEmpty() const;
	bool IsArray() const;
	vstd::string ToString() const;
	vstd::string ToFormattedString() const;
	const vstd::string& GetErrMsg() const {
		return (m_strErrMsg);
	}

	///////////////// method of ordinary json object
	bool AddEmptySubObject(const vstd::string& strKey);
	bool AddEmptySubArray(const vstd::string& strKey);
	bool GetKey(vstd::string& strKey);
	void ResetTraversing();
	CJsonObject& operator[](const vstd::string& strKey);
	vstd::string operator()(const vstd::string& strKey) const;
	bool Get(const vstd::string& strKey, CJsonObject& oJsonObject) const;
	cJSON* Get(const vstd::string& strKey) const;
	bool Get(const vstd::string& strKey, vstd::string& strValue) const;
	bool Get(const vstd::string& strKey, int32& iValue) const;
	bool Get(const vstd::string& strKey, uint& uiValue) const;
	bool Get(const vstd::string& strKey, int64& llValue) const;
	bool Get(const vstd::string& strKey, uint64& ullValue) const;
	bool Get(const vstd::string& strKey, bool& bValue) const;
	bool Get(const vstd::string& strKey, float& fValue) const;
	bool Get(const vstd::string& strKey, double& dValue) const;
	bool GetCJsonObject(const vstd::string& strKey, Runnable<void(CJsonObject&&)> const& func) const;
	bool GetString(const vstd::string& strKey, Runnable<void(vstd::string_view)> const& func) const;
	bool GetInteger(const vstd::string& strKey, Runnable<void(int64)> const& iValue) const;
	bool GetBool(const vstd::string& strKey, Runnable<void(bool)> const& iValue) const;
	bool GetFloat(const vstd::string& strKey, Runnable<void(double)> const& iValue) const;

	bool IsNull(const vstd::string& strKey) const;
	bool Add(const vstd::string& strKey, const CJsonObject& oJsonObject);
	bool Add(const vstd::string& strKey, const vstd::string& strValue);
	bool Add(const vstd::string& strKey, int32 iValue);
	bool Add(const vstd::string& strKey, uint uiValue);
	bool Add(const vstd::string& strKey, int64 llValue);
	bool Add(const vstd::string& strKey, uint64 ullValue);
	bool Add(const vstd::string& strKey, bool bValue, bool bValueAgain);
	bool Add(const vstd::string& strKey, float fValue);
	bool Add(const vstd::string& strKey, double dValue);
	bool AddNull(const vstd::string& strKey);// add null like this:   "key":null
	bool Delete(const vstd::string& strKey);
	bool Replace(const vstd::string& strKey, const CJsonObject& oJsonObject);
	bool Replace(const vstd::string& strKey, const vstd::string& strValue);
	bool Replace(const vstd::string& strKey, int32 iValue);
	bool Replace(const vstd::string& strKey, uint uiValue);
	bool Replace(const vstd::string& strKey, int64 llValue);
	bool Replace(const vstd::string& strKey, uint64 ullValue);
	bool Replace(const vstd::string& strKey, bool bValue, bool bValueAgain);
	bool Replace(const vstd::string& strKey, float fValue);
	bool Replace(const vstd::string& strKey, double dValue);
	bool ReplaceWithNull(const vstd::string& strKey);// replace value with null

	///////////////// method of json array
	int GetArraySize() const;
	CJsonObject& operator[](uint32_t uiWhich);
	vstd::string operator()(uint32_t uiWhich) const;
	bool Get(int iWhich, CJsonObject& oJsonObject) const;
	cJSON* Get(int iWhich) const;
	bool Get(int iWhich, vstd::string& strValue) const;
	bool Get(int iWhich, int32& iValue) const;
	bool Get(int iWhich, uint& uiValue) const;
	bool Get(int iWhich, int64& llValue) const;
	bool Get(int iWhich, uint64& ullValue) const;
	bool Get(int iWhich, bool& bValue) const;
	bool Get(int iWhich, float& fValue) const;
	bool Get(int iWhich, double& dValue) const;

	bool GetCJsonObject(int iWhich, Runnable<void(CJsonObject&&)> const& func) const;
	bool GetString(int iWhich, Runnable<void(vstd::string_view)> const& func) const;
	bool GetInteger(int iWhich, Runnable<void(int64)> const& iValue) const;
	bool GetBool(int iWhich, Runnable<void(bool)> const& iValue) const;
	bool GetFloat(int iWhich, Runnable<void(double)> const& iValue) const;

	bool IsNull(int iWhich) const;
	bool Add(const CJsonObject& oJsonObject);
	bool Add(const vstd::string& strValue);
	bool Add(int32 iValue);
	bool Add(uint uiValue);
	bool Add(int64 llValue);
	bool Add(uint64 ullValue);
	bool Add(int iAnywhere, bool bValue);
	bool Add(float fValue);
	bool Add(double dValue);
	bool AddNull();// add a null value
	bool AddAsFirst(const CJsonObject& oJsonObject);
	bool AddAsFirst(const vstd::string& strValue);
	bool AddAsFirst(int32 iValue);
	bool AddAsFirst(uint uiValue);
	bool AddAsFirst(int64 llValue);
	bool AddAsFirst(uint64 ullValue);
	bool AddAsFirst(int iAnywhere, bool bValue);
	bool AddAsFirst(float fValue);
	bool AddAsFirst(double dValue);
	bool AddNullAsFirst();// add a null value
	bool Delete(int iWhich);
	bool Replace(int iWhich, const CJsonObject& oJsonObject);
	bool Replace(int iWhich, const vstd::string& strValue);
	bool Replace(int iWhich, int32 iValue);
	bool Replace(int iWhich, uint uiValue);
	bool Replace(int iWhich, int64 llValue);
	bool Replace(int iWhich, uint64 ullValue);
	bool Replace(int iWhich, bool bValue, bool bValueAgain);
	bool Replace(int iWhich, float fValue);
	bool Replace(int iWhich, double dValue);
	bool ReplaceWithNull(int iWhich);// replace with a null value

private:
	CJsonObject(cJSON* pJsonData);

private:
	cJSON* m_pJsonData;
	cJSON* m_pExternJsonDataRef;
	cJSON* m_pKeyTravers;
	vstd::string m_strErrMsg;
	HashMap<uint32_t, CJsonObject*> m_mapJsonArrayRef;
	HashMap<vstd::string, CJsonObject*> m_mapJsonObjectRef;
	vstd::vector<uint32_t> deleteKeys;
};

}// namespace neb

template<typename T>
struct JsonKeyValuePair {
	const vstd::string& key;
	PureType_t<T>* value;
	JsonKeyValuePair(const vstd::string& str, PureType_t<T>* value) : value(value), key(str) {}
	JsonKeyValuePair(const vstd::string& str, PureType_t<T>& value) : value(&value), key(str) {}
};

template<typename... Args>
inline void GetValuesFromJson(neb::CJsonObject* cjson, Args&&... args) {
	char c[] = {(cjson->Get(args.key, *args.value), 0)...};
}

VENGINE_DLL_COMMON neb::CJsonObject* ReadJson(const vstd::string& filePath);
VENGINE_DLL_COMMON void ReadJson(const vstd::string& filePath, StackObject<neb::CJsonObject, true>& obj);
VENGINE_DLL_COMMON double GetDoubleFromChar(char* c, size_t t);
VENGINE_DLL_COMMON float GetFloatFromChar(char* c, size_t t);
VENGINE_DLL_COMMON int GetIntFromChar(char* c, size_t t);
//void  ReadJson(const vstd::string& filePath, StackObject<neb::CJsonObject, true>& placementPtr);
template<typename T>
void ReadStringToVector(char* cPtr, size_t t, T* vec) {
	const size_t floatNum = sizeof(T) / sizeof(float);
	float* vecPointer = (float*)vec;
	size_t count = 0;
	size_t floatOffset = 0;
	char* start = cPtr;
	for (size_t i = 0; i < t; ++i) {
		char c = cPtr[i];
		if (c == ',') {
			if (floatOffset >= floatNum) return;
			vecPointer[floatOffset] = GetFloatFromChar(start, count);
			start = cPtr + i + 1;
			count = 0;
			floatOffset++;
		} else {
			count++;
		}
	}
	if (floatOffset >= floatNum) return;
	vecPointer[floatOffset] = GetFloatFromChar(start, count);
}

template<typename T>
void ReadStringToDoubleVector(char* cPtr, size_t t, T* vec) {
	const size_t floatNum = sizeof(T) / sizeof(double);
	double* vecPointer = (double*)vec;
	size_t count = 0;
	size_t floatOffset = 0;
	char* start = cPtr;
	for (size_t i = 0; i < t; ++i) {
		char c = cPtr[i];
		if (c == ',') {
			if (floatOffset >= floatNum) return;
			vecPointer[floatOffset] = GetDoubleFromChar(start, count);
			start = cPtr + i + 1;
			count = 0;
			floatOffset++;
		} else {
			count++;
		}
	}
	if (floatOffset >= floatNum) return;
	vecPointer[floatOffset] = GetDoubleFromChar(start, count);
}

template<typename T>
void ReadStringToIntVector(char* cPtr, size_t t, T* vec) {
	const size_t intNum = sizeof(T) / sizeof(int);
	int* vecPointer = (int*)vec;
	size_t count = 0;
	size_t floatOffset = 0;
	char* start = cPtr;
	for (size_t i = 0; i < t; ++i) {
		char c = cPtr[i];
		if (c == ',') {
			if (floatOffset >= intNum) return;
			vecPointer[floatOffset] = GetIntFromChar(start, count);
			start = cPtr + i + 1;
			count = 0;
			floatOffset++;
		} else {
			count++;
		}
	}
	if (floatOffset >= intNum) return;
	vecPointer[floatOffset] = GetIntFromChar(start, count);
}
#endif
