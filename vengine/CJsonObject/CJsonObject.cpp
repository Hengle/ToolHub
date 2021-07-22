#pragma vengine_package vengine_dll
/*******************************************************************************
 * Project:  neb
 * @file     CJsonObject.cpp
 * @brief
 * @author   bwarliao
 * @date:    2014-7-16
 * @note
 * Modify history:
 ******************************************************************************/

#include <CJsonObject/CJsonObject.hpp>
namespace neb {

	CJsonObject::CJsonObject()
		: m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
		// m_pJsonData = cJSON_CreateObject();
	}

	CJsonObject::CJsonObject(const vstd::string& strJson)
		: m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
		Parse(strJson);
	}

	CJsonObject::CJsonObject(const CJsonObject* pJsonObject)
		: m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
		if (pJsonObject) {
			Parse(pJsonObject->ToString());
		}
	}

	CJsonObject::CJsonObject(const CJsonObject& oJsonObject)
		: m_pJsonData(NULL), m_pExternJsonDataRef(NULL), m_pKeyTravers(NULL) {
		Parse(oJsonObject.ToString());
	}

	CJsonObject::~CJsonObject() {
		Clear();
	}

	CJsonObject& CJsonObject::operator=(const CJsonObject& oJsonObject) {
		Parse(oJsonObject.ToString());
		return (*this);
	}

	bool CJsonObject::operator==(const CJsonObject& oJsonObject) const {
		return (this->ToString() == oJsonObject.ToString());
	}

	bool CJsonObject::AddEmptySubObject(const vstd::string& strKey) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateObject();
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("create sub empty object error!");
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::AddEmptySubArray(const vstd::string& strKey) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateArray();
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("create sub empty array error!");
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::GetKey(vstd::string& strKey) {
		if (IsArray()) {
			return (false);
		}
		if (m_pKeyTravers == NULL) {
			if (m_pJsonData != NULL) {
				m_pKeyTravers = m_pJsonData;
			}
			else if (m_pExternJsonDataRef != NULL) {
				m_pKeyTravers = m_pExternJsonDataRef;
			}
			return (false);
		}
		else if (m_pKeyTravers == m_pJsonData || m_pKeyTravers == m_pExternJsonDataRef) {
			cJSON* c = m_pKeyTravers->child;
			if (c) {
				strKey = c->string;
				m_pKeyTravers = c->next;
				return (true);
			}
			else {
				return (false);
			}
		}
		else {
			strKey = m_pKeyTravers->string;
			m_pKeyTravers = m_pKeyTravers->next;
			return (true);
		}
	}

	void CJsonObject::ResetTraversing() {
		if (m_pJsonData != NULL) {
			m_pKeyTravers = m_pJsonData;
		}
		else {
			m_pKeyTravers = m_pExternJsonDataRef;
		}
	}

	CJsonObject& CJsonObject::operator[](const vstd::string& strKey) {
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (!iter) {
			cJSON* pJsonStruct = NULL;
			if (m_pJsonData != NULL) {
				if (m_pJsonData->type == cJSON_Object) {
					pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
				}
			}
			else if (m_pExternJsonDataRef != NULL) {
				if (m_pExternJsonDataRef->type == cJSON_Object) {
					pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
				}
			}
			if (pJsonStruct == NULL) {
				CJsonObject* pJsonObject = new CJsonObject();
				m_mapJsonObjectRef.ForceEmplace(strKey, pJsonObject);
				return (*pJsonObject);
			}
			else {
				CJsonObject* pJsonObject = new CJsonObject(pJsonStruct);
				m_mapJsonObjectRef.ForceEmplace(strKey, pJsonObject);
				return (*pJsonObject);
			}
		}
		else {
			return (*(iter.Value()));
		}
	}

	CJsonObject& CJsonObject::operator[](uint32_t uiWhich) {

		auto iter = m_mapJsonArrayRef.Find(uiWhich);
		if (!iter) {
			cJSON* pJsonStruct = NULL;
			if (m_pJsonData != NULL) {
				if (m_pJsonData->type == cJSON_Array) {
					pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
				}
			}
			else if (m_pExternJsonDataRef != NULL) {
				if (m_pExternJsonDataRef->type == cJSON_Array) {
					pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
				}
			}
			if (pJsonStruct == NULL) {
				CJsonObject* pJsonObject = new CJsonObject();
				m_mapJsonArrayRef.ForceEmplace(uiWhich, pJsonObject);
				return (*pJsonObject);
			}
			else {
				CJsonObject* pJsonObject = new CJsonObject(pJsonStruct);
				m_mapJsonArrayRef.ForceEmplace(uiWhich, pJsonObject);
				return (*pJsonObject);
			}
		}
		else {
			return (*(iter.Value()));
		}
	}

	vstd::string CJsonObject::operator()(const vstd::string& strKey) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (vstd::string(""));
		}
		if (pJsonStruct->type == cJSON_String) {
			return (pJsonStruct->valuestring);
		}
		else if (pJsonStruct->type == cJSON_Int) {
			char szNumber[128] = { 0 };
			if (pJsonStruct->sign == -1) {
				if (pJsonStruct->valueint <= (int64)INT_MAX && (int64)pJsonStruct->valueint >= (int64)INT_MIN) {
					snprintf(szNumber, sizeof(szNumber), "%d", (int32)pJsonStruct->valueint);
				}
				else {
					snprintf(szNumber, sizeof(szNumber), "%lld", (int64)pJsonStruct->valueint);
				}
			}
			else {
				if ((uint64)pJsonStruct->valueint <= (uint64)UINT_MAX) {
					snprintf(szNumber, sizeof(szNumber), "%u", (uint)pJsonStruct->valueint);
				}
				else {
					snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
				}
			}
			return (vstd::string(szNumber));
		}
		else if (pJsonStruct->type == cJSON_Double) {
			char szNumber[128] = { 0 };
			if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
				snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
			}
			else {
				snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
			}
		}
		else if (pJsonStruct->type == cJSON_False) {
			return (vstd::string("false"));
		}
		else if (pJsonStruct->type == cJSON_True) {
			return (vstd::string("true"));
		}
		return (vstd::string(""));
	}

	vstd::string CJsonObject::operator()(uint32_t uiWhich) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, uiWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, uiWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (vstd::string(""));
		}
		if (pJsonStruct->type == cJSON_String) {
			return (pJsonStruct->valuestring);
		}
		else if (pJsonStruct->type == cJSON_Int) {
			char szNumber[128] = { 0 };
			if (pJsonStruct->sign == -1) {
				if (pJsonStruct->valueint <= (int64)INT_MAX && (int64)pJsonStruct->valueint >= (int64)INT_MIN) {
					snprintf(szNumber, sizeof(szNumber), "%d", (int32)pJsonStruct->valueint);
				}
				else {
					snprintf(szNumber, sizeof(szNumber), "%lld", (int64)pJsonStruct->valueint);
				}
			}
			else {
				if ((uint64)pJsonStruct->valueint <= (uint64)UINT_MAX) {
					snprintf(szNumber, sizeof(szNumber), "%u", (uint)pJsonStruct->valueint);
				}
				else {
					snprintf(szNumber, sizeof(szNumber), "%llu", pJsonStruct->valueint);
				}
			}
			return (vstd::string(szNumber));
		}
		else if (pJsonStruct->type == cJSON_Double) {
			char szNumber[128] = { 0 };
			if (fabs(pJsonStruct->valuedouble) < 1.0e-6 || fabs(pJsonStruct->valuedouble) > 1.0e9) {
				snprintf(szNumber, sizeof(szNumber), "%e", pJsonStruct->valuedouble);
			}
			else {
				snprintf(szNumber, sizeof(szNumber), "%f", pJsonStruct->valuedouble);
			}
		}
		else if (pJsonStruct->type == cJSON_False) {
			return (vstd::string("false"));
		}
		else if (pJsonStruct->type == cJSON_True) {
			return (vstd::string("true"));
		}
		return (vstd::string(""));
	}

	bool CJsonObject::Parse(const vstd::string& strJson) {
		Clear();
		m_pJsonData = cJSON_Parse(strJson.c_str());
		m_pKeyTravers = m_pJsonData;
		if (m_pJsonData == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		return (true);
	}

	void CJsonObject::Clear() {
		m_pExternJsonDataRef = NULL;
		m_pKeyTravers = NULL;
		if (m_pJsonData != NULL) {
			cJSON_Delete(m_pJsonData);
			m_pJsonData = NULL;
		}
		for (auto&& i : m_mapJsonArrayRef) {
			if (i.second != nullptr) {
				delete i.second;
				i.second = nullptr;
			}
		}
		m_mapJsonArrayRef.Clear();
		for (auto&& i : m_mapJsonObjectRef) {
			if (i.second != nullptr) {
				delete i.second;
				i.second = nullptr;
			}
		}
		m_mapJsonObjectRef.Clear();
	}

	bool CJsonObject::IsEmpty() const {
		if (m_pJsonData != NULL) {
			return (false);
		}
		else if (m_pExternJsonDataRef != NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::IsArray() const {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}

		if (pFocusData == NULL) {
			return (false);
		}

		if (pFocusData->type == cJSON_Array) {
			return (true);
		}
		else {
			return (false);
		}
	}

	vstd::string CJsonObject::ToString() const {
		char* pJsonString = NULL;
		vstd::string strJsonData = "";
		if (m_pJsonData != NULL) {
			pJsonString = cJSON_PrintUnformatted(m_pJsonData);
		}
		else if (m_pExternJsonDataRef != NULL) {
			pJsonString = cJSON_PrintUnformatted(m_pExternJsonDataRef);
		}
		if (pJsonString != NULL) {
			strJsonData = pJsonString;
			vengine_free(pJsonString);
		}
		return (strJsonData);
	}

	vstd::string CJsonObject::ToFormattedString() const {
		char* pJsonString = NULL;
		vstd::string strJsonData = "";
		if (m_pJsonData != NULL) {
			pJsonString = cJSON_Print(m_pJsonData);
		}
		else if (m_pExternJsonDataRef != NULL) {
			pJsonString = cJSON_Print(m_pExternJsonDataRef);
		}
		if (pJsonString != NULL) {
			strJsonData = pJsonString;
			vengine_free(pJsonString);
		}
		return (strJsonData);
	}

	bool CJsonObject::Get(const vstd::string& strKey, CJsonObject& oJsonObject) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		char* pJsonString = cJSON_Print(pJsonStruct);
		vstd::string strJsonData = pJsonString;
		vengine_free(pJsonString);
		if (oJsonObject.Parse(strJsonData)) {
			return (true);
		}
		else {
			return (false);
		}
	}

	cJSON* CJsonObject::Get(const vstd::string& strKey) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		return pJsonStruct;
	}

	bool CJsonObject::Get(const vstd::string& strKey, vstd::string& strValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type != cJSON_String) {
			return (false);
		}
		strValue = pJsonStruct->valuestring;
		return (true);
	}

	bool CJsonObject::Get(const vstd::string& strKey, int32& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			iValue = (int32)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			iValue = (int32)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(const vstd::string& strKey, uint& uiValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			uiValue = (uint)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			uiValue = (uint)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(const vstd::string& strKey, int64& llValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			llValue = (int64)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			llValue = (int64)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(const vstd::string& strKey, uint64& ullValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			ullValue = (uint64)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			ullValue = (uint64)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(const vstd::string& strKey, bool& bValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type > cJSON_True) {
			return (false);
		}
		bValue = pJsonStruct->type;
		return (true);
	}

	bool CJsonObject::Get(const vstd::string& strKey, float& fValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			fValue = (float)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(const vstd::string& strKey, double& dValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			dValue = pJsonStruct->valuedouble;
			return (true);
		}
		return (false);
	}

	bool CJsonObject::GetCJsonObject(const vstd::string& strKey, Runnable<void(CJsonObject&&)> const& func) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		char* pJsonString = cJSON_Print(pJsonStruct);
		vstd::string strJsonData = pJsonString;
		vengine_free(pJsonString);
		CJsonObject oJsonObject;
		if (oJsonObject.Parse(strJsonData)) {
			func(std::move(oJsonObject));
		}
		return true;
	}

	bool CJsonObject::GetString(const vstd::string& strKey, Runnable<void(vstd::string_view)> const& func) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type != cJSON_String) {
			return false;
		}
		func(pJsonStruct->valuestring);
		return true;
	}

	bool CJsonObject::GetInteger(const vstd::string& strKey, Runnable<void(int64)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type == cJSON_Int) {
			iValue(pJsonStruct->valueint);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			iValue(static_cast<int64>(pJsonStruct->valueint));
		}
		return true;
	}

	bool CJsonObject::GetBool(const vstd::string& strKey, Runnable<void(bool)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type > cJSON_True) {
			return false;
		}
		iValue(pJsonStruct->type);
		return true;
	}

	bool CJsonObject::GetFloat(const vstd::string& strKey, Runnable<void(double)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			iValue(pJsonStruct->valuedouble);
		}
		return true;
	}

	bool CJsonObject::IsNull(const vstd::string& strKey) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pJsonData, strKey.c_str());
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Object) {
				pJsonStruct = cJSON_GetObjectItem(m_pExternJsonDataRef, strKey.c_str());
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type != cJSON_NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, const CJsonObject& oJsonObject) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, const vstd::string& strValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, int32 iValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, uint uiValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, int64 llValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, uint64 ullValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt(ullValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, bool bValue, bool bValueAgain) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateBool(bValue);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, float fValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strKey, double dValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::AddNull(const vstd::string& strKey) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateObject();
			m_pKeyTravers = m_pJsonData;
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) != NULL) {
			m_strErrMsg = "key exists!";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateNull();
		if (pJsonStruct == NULL) {
			return (false);
		}
		cJSON_AddItemToObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Delete(const vstd::string& strKey) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON_DeleteItemFromObject(pFocusData, strKey.c_str());
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		m_pKeyTravers = pFocusData;
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, const CJsonObject& oJsonObject) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, const vstd::string& strValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, int32 iValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, uint uiValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, int64 llValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, uint64 ullValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, bool bValue, bool bValueAgain) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateBool(bValue);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, float fValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(const vstd::string& strKey, double dValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::ReplaceWithNull(const vstd::string& strKey) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Object) {
			m_strErrMsg = "not a json object! json array?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateNull();
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonObjectRef.Find(strKey);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonObjectRef.Remove(iter);
		}
		cJSON_ReplaceItemInObject(pFocusData, strKey.c_str(), pJsonStruct);
		if (cJSON_GetObjectItem(pFocusData, strKey.c_str()) == NULL) {
			return (false);
		}
		return (true);
	}

	int CJsonObject::GetArraySize() const {
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				return (cJSON_GetArraySize(m_pJsonData));
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				return (cJSON_GetArraySize(m_pExternJsonDataRef));
			}
		}
		return (0);
	}

	bool CJsonObject::Get(int iWhich, CJsonObject& oJsonObject) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		char* pJsonString = cJSON_Print(pJsonStruct);
		vstd::string strJsonData = pJsonString;
		vengine_free(pJsonString);
		if (oJsonObject.Parse(strJsonData)) {
			return (true);
		}
		else {
			return (false);
		}
	}

	cJSON* CJsonObject::Get(int iWhich) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		return pJsonStruct;
	}

	bool CJsonObject::Get(int iWhich, vstd::string& strValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type != cJSON_String) {
			return (false);
		}
		strValue = pJsonStruct->valuestring;
		return (true);
	}

	bool CJsonObject::Get(int iWhich, int32& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			iValue = (int32)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			iValue = (int32)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(int iWhich, uint& uiValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			uiValue = (uint)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			uiValue = (uint)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(int iWhich, int64& llValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			llValue = (int64)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			llValue = (int64)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(int iWhich, uint64& ullValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Int) {
			ullValue = (uint64)(pJsonStruct->valueint);
			return (true);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			ullValue = (uint64)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(int iWhich, bool& bValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type > cJSON_True) {
			return (false);
		}
		bValue = pJsonStruct->type;
		return (true);
	}

	bool CJsonObject::Get(int iWhich, float& fValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			fValue = (float)(pJsonStruct->valuedouble);
			return (true);
		}
		return (false);
	}

	bool CJsonObject::Get(int iWhich, double& dValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			dValue = pJsonStruct->valuedouble;
			return (true);
		}
		return (false);
	}

	bool CJsonObject::GetCJsonObject(int iWhich, Runnable<void(CJsonObject&&)> const& func) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		char* pJsonString = cJSON_Print(pJsonStruct);
		vstd::string strJsonData = pJsonString;
		vengine_free(pJsonString);
		CJsonObject oJsonObject;
		if (oJsonObject.Parse(strJsonData)) {
			func(std::move(oJsonObject));
		}
		return true;
	}

	bool CJsonObject::GetString(int iWhich, Runnable<void(vstd::string_view)> const& func) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type != cJSON_String) {
			return false;
		}
		func(pJsonStruct->valuestring);
		return true;
	}

	bool CJsonObject::GetInteger(int iWhich, Runnable<void(int64)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type == cJSON_Int) {
			iValue(pJsonStruct->valueint);
		}
		else if (pJsonStruct->type == cJSON_Double) {
			iValue(static_cast<int64>(pJsonStruct->valuedouble));
		}
		return true;
	}

	bool CJsonObject::GetBool(int iWhich, Runnable<void(bool)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type > cJSON_True) {
			return false;
		}
		iValue(pJsonStruct->type);
		return true;
	}

	bool CJsonObject::GetFloat(int iWhich, Runnable<void(double)> const& iValue) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return false;
		}
		if (pJsonStruct->type == cJSON_Double || pJsonStruct->type == cJSON_Int) {
			iValue(pJsonStruct->valuedouble);
		}
		return true;
	}

	bool CJsonObject::IsNull(int iWhich) const {
		cJSON* pJsonStruct = NULL;
		if (m_pJsonData != NULL) {
			if (m_pJsonData->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pJsonData, iWhich);
			}
		}
		else if (m_pExternJsonDataRef != NULL) {
			if (m_pExternJsonDataRef->type == cJSON_Array) {
				pJsonStruct = cJSON_GetArrayItem(m_pExternJsonDataRef, iWhich);
			}
		}
		if (pJsonStruct == NULL) {
			return (false);
		}
		if (pJsonStruct->type != cJSON_NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(const CJsonObject& oJsonObject) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		uint32_t uiLastIndex = (uint32_t)cJSON_GetArraySize(pFocusData) - 1;
		deleteKeys.reserve(10);
		for (auto&& i : m_mapJsonArrayRef) {
			if (i.first >= uiLastIndex) {
				if (i.second != NULL) {
					delete i.second;
					i.second = NULL;
				}
				deleteKeys.push_back(i.first);
			}
		}


		for (auto ite = deleteKeys.begin(); ite != deleteKeys.end(); ++ite) {
			m_mapJsonArrayRef.Remove(*ite);
		}
		deleteKeys.clear();
		return (true);
	}

	bool CJsonObject::Add(const vstd::string& strValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(int32 iValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(uint uiValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(int64 llValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(uint64 ullValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(int iAnywhere, bool bValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateBool(bValue);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(float fValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Add(double dValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddNull() {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateNull();
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArray(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(const CJsonObject& oJsonObject) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		deleteKeys.reserve(10);
		for (auto&& i : m_mapJsonArrayRef) {
			if (i.second != NULL) {
				delete i.second;
				i.second = NULL;
			}
			deleteKeys.push_back(i.first);
		}
		for (auto ite = deleteKeys.begin(); ite != deleteKeys.end(); ++ite) {
			m_mapJsonArrayRef.Remove(*ite);
		}
		deleteKeys.clear();
		return (true);
	}

	bool CJsonObject::AddAsFirst(const vstd::string& strValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(int32 iValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(uint uiValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(int64 llValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)llValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(uint64 ullValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(int iAnywhere, bool bValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateBool(bValue);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(float fValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddAsFirst(double dValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::AddNullAsFirst() {
		cJSON* pFocusData = NULL;
		if (m_pJsonData != NULL) {
			pFocusData = m_pJsonData;
		}
		else if (m_pExternJsonDataRef != NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			m_pJsonData = cJSON_CreateArray();
			pFocusData = m_pJsonData;
		}

		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateNull();
		if (pJsonStruct == NULL) {
			return (false);
		}
		int iArraySizeBeforeAdd = cJSON_GetArraySize(pFocusData);
		cJSON_AddItemToArrayHead(pFocusData, pJsonStruct);
		int iArraySizeAfterAdd = cJSON_GetArraySize(pFocusData);
		if (iArraySizeAfterAdd == iArraySizeBeforeAdd) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Delete(int iWhich) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON_DeleteItemFromArray(pFocusData, iWhich);
		deleteKeys.reserve(10);
		for (auto&& i : m_mapJsonArrayRef) {
			if (i.first >= (uint32_t)iWhich) {
				if (i.second != NULL) {
					delete i.second;
					i.second = NULL;
				}
				deleteKeys.push_back(i.first);
			}
		}
		for (auto ite = deleteKeys.begin(); ite != deleteKeys.end(); ++ite) {
			m_mapJsonArrayRef.Remove(*ite);
		}
		deleteKeys.clear();
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, const CJsonObject& oJsonObject) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_Parse(oJsonObject.ToString().c_str());
		if (pJsonStruct == NULL) {
			m_strErrMsg = vstd::string("prase json string error at ") + cJSON_GetErrorPtr();
			return (false);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, const vstd::string& strValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateString(strValue.c_str());
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, int32 iValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)iValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, uint uiValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)uiValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, int64 llValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)((uint64)llValue), -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, uint64 ullValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateInt((uint64)ullValue, 1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, bool bValue, bool bValueAgain) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateBool(bValue);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, float fValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)fValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::Replace(int iWhich, double dValue) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateDouble((double)dValue, -1);
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	bool CJsonObject::ReplaceWithNull(int iWhich) {
		cJSON* pFocusData = NULL;
		if (m_pJsonData == NULL) {
			pFocusData = m_pExternJsonDataRef;
		}
		else {
			pFocusData = m_pJsonData;
		}
		if (pFocusData == NULL) {
			m_strErrMsg = "json data is null!";
			return (false);
		}
		if (pFocusData->type != cJSON_Array) {
			m_strErrMsg = "not a json array! json object?";
			return (false);
		}
		cJSON* pJsonStruct = cJSON_CreateNull();
		if (pJsonStruct == NULL) {
			return (false);
		}
		auto iter = m_mapJsonArrayRef.Find(iWhich);
		if (iter) {
			if (iter.Value() != NULL) {
				delete (iter.Value());
				iter.Value() = NULL;
			}
			m_mapJsonArrayRef.Remove(iter);
		}
		cJSON_ReplaceItemInArray(pFocusData, iWhich, pJsonStruct);
		if (cJSON_GetArrayItem(pFocusData, iWhich) == NULL) {
			return (false);
		}
		return (true);
	}

	CJsonObject::CJsonObject(cJSON* pJsonData)
		: m_pJsonData(NULL), m_pExternJsonDataRef(pJsonData), m_pKeyTravers(pJsonData) {
	}

}// namespace neb

neb::CJsonObject* ReadJson(const vstd::string& filePath) {
	std::ifstream ifs(filePath.data());
	if (!ifs) return nullptr;
	ifs.seekg(0, std::ios::end);
	size_t len = ifs.tellg();
	ifs.seekg(0, 0);
	vstd::string s(len + 1, ' ');
	ifs.read((char*)s.data(), len);
	neb::CJsonObject* jsonObj = new neb::CJsonObject(s);
	if (jsonObj->IsEmpty()) {
		delete jsonObj;
		return nullptr;
	}
	return jsonObj;
}

void ReadJson(const vstd::string& filePath, StackObject<neb::CJsonObject, true>& obj) {
	std::ifstream ifs(filePath.data());
	if (!ifs) {
		obj.Delete();
		return;
	}
	ifs.seekg(0, std::ios::end);
	size_t len = ifs.tellg();
	ifs.seekg(0, 0);
	vstd::string s(len + 1, ' ');
	ifs.read((char*)s.data(), len);
	obj.New();
	obj->Clear();
	obj->Parse(s);
	if (obj->IsEmpty()) {
		obj.Delete();
		return;
	}
}

float GetFloatFromChar(char* c, size_t t) {
	if (t == 0) return 0;
	float num = 0;
	float pointer = 0;
	float rate = 1;
	float type = 1;
	size_t i = 0;
	if (c[i] == '-') {
		type = -1;
		i++;
	}
	for (; i < t; ++i) {
		if (c[i] == '.') {
			++i;
			break;
		}
		char n = c[i] - 48;
		num *= 10;
		num += n;
	}
	for (; i < t; ++i) {
		rate *= 0.1f;
		pointer += (c[i] - 48) * rate;
	}
	return (pointer + num) * type;
}

int GetIntFromChar(char* c, size_t t) {
	if (t == 0) return 0;
	int num = 0;
	int type = 1;
	size_t i = 0;
	if (c[i] == '-') {
		type = -1;
		i++;
	}
	for (; i < t; ++i) {
		if (c[i] == '.') {
			++i;
			break;
		}
		char n = c[i] - 48;
		num *= 10;
		num += n;
	}
	return num * type;
}
//#endif
double GetDoubleFromChar(char* c, size_t t) {
	if (t == 0) return 0;
	double num = 0;
	double pointer = 0;
	double rate = 1;
	double type = 1;
	size_t i = 0;
	if (c[i] == '-') {
		type = -1;
		i++;
	}
	for (; i < t; ++i) {
		if (c[i] == '.') {
			++i;
			break;
		}
		char n = c[i] - 48;
		num *= 10;
		num += n;
	}
	for (; i < t; ++i) {
		rate *= 0.1;
		pointer += (c[i] - 48) * rate;
	}
	return (pointer + num) * type;
}
/*
void ReadJson(const vstd::string& filePath, StackObject<neb::CJsonObject, true>& placementPtr)
{
	std::ifstream ifs(filePath.data());
	if (!ifs) return;
	ifs.seekg(0, std::ios::end);
	size_t len = ifs.tellg();
	ifs.seekg(0, 0);
	vstd::string s(len + 1, ' ');
	ifs.read((char*)s.data(), len);
	placementPtr.New();
	if (!placementPtr->Parse(s))
	{
		placementPtr.Delete();
	}
}*/