#pragma once
#include <Unity/UnityInclude.h>

namespace toolhub {
namespace db {
class IJsonDict;
}
class Component final : public vstd::IOperatorNewBase {
private:
	using ValueStructType = vstd::variant<
		int64, 
		double,
		bool,
		vstd::string,				
		vstd::unique_ptr<Component>,
		vstd::vector<bool>,
		vstd::vector<int64>,
		vstd::vector<double>,
		vstd::vector<Component*>>;
	using ValueMap = HashMap<vstd::string, ValueStructType>;

	vstd::variant<
		ValueMap,
		vstd::unique_ptr<db::IJsonDict>>
		data;

public:
	//CPP Only
	Component(vstd::unique_ptr<db::IJsonDict>&& ptr);

	//Interface
	Component(CSharpString& typeName, void*& parentDatabase);
	~Component();
	bool GetBool(CSharpString& name);
	int64 GetInt(CSharpString& name);
	double GetFloat(CSharpString& name);
	CSharpString GetString(CSharpString& name);
	void* GetComponent(CSharpString& name);
	BinaryArray GetBoolArray(CSharpString& name);
	BinaryArray GetIntArray(CSharpString& name);
	BinaryArray GetFloatArray(CSharpString& name);
	BinaryArray GetComponentArray(CSharpString& name);
	void SetInt(CSharpString& name, int64& value);
	void SetBool(CSharpString& name, bool& value);
	void SetFloat(CSharpString& name, double& value);
	void SetString(CSharpString& name, CSharpString& value);
	void SetComponent(CSharpString& name, void*& comp);
	void SetIntArray(CSharpString& name, BinaryArray& value);
	void SetFloatArray(CSharpString& name, BinaryArray& value);
	void SetComponentArray(CSharpString& name, BinaryArray& value);
	void SetBoolArray(CSharpString& name, BinaryArray& value);
	void Reset();
	void* GetHandle();
	bool IsValueType();
};
}// namespace toolhub