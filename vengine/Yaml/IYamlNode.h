#pragma once
#include <Common/Memory.h>
#include <Yaml/yaml-cpp/node/type.h>
#include <Yaml/yaml-cpp/mark.h>
#include <Common/linq.h>
namespace YAML {
class IYamlNode : public vstd::IDisposable {
protected:
	~IYamlNode() = default;

public:
	virtual YAML::Mark Mark() = 0;
	virtual NodeType::value Type() = 0;
	using KeyValue =
		std::pair<
			vstd::unique_ptr<IYamlNode>,
			vstd::unique_ptr<IYamlNode>>;
	using Variant = vstd::variant<
		vstd::unique_ptr<IYamlNode>,
		KeyValue>;
	virtual vstd::unique_ptr<
		vstd::linq::Iterator<
			const Variant>>
	GetIterator() = 0;
	virtual vstd::unique_ptr<IYamlNode> Get(vstd::string_view strv) = 0;
	virtual vstd::unique_ptr<IYamlNode> Get(IYamlNode* node) = 0;
	virtual vstd::unique_ptr<IYamlNode> Get(int64 arrayIndex) = 0;
	virtual vstd::string_view ToString() = 0;
	virtual int64 ToInt() = 0;
	virtual double ToFloat() = 0;
	virtual bool IsArray() = 0;
	virtual void operator=(vstd::string_view str) = 0;
	virtual void operator=(int64 intValue) = 0;
	virtual void operator=(double floatValue) = 0;
	virtual void operator=(IYamlNode* ptr) = 0;
	inline void operator=(vstd::unique_ptr<IYamlNode> const& ptr) {
		operator=(ptr.get());
	}
};
class IYamlFactory {
protected:
	~IYamlFactory() = default;

public:
	virtual vstd::unique_ptr<IYamlNode> GetNodeFromFile(vstd::string const& filePath) = 0;
	virtual vstd::unique_ptr<IYamlNode> GetNodeFromString(vstd::string const& strValue) = 0;
};
//IYamlFactory const* Yaml_GetFactory();
}// namespace YAML