#pragma vengine_package yaml_cpp
#include <IYamlNode.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/node.h>
namespace YAML {
class YamlNodeImpl;
struct PoolStruct {
	Pool<YamlNodeImpl, VEngine_AllocType::VEngine, false> pool;
	std::mutex mtx;
	PoolStruct() : pool(64, false) {}
};

class YamlIterator : public vstd::linq::Iterator<const IYamlNode::Variant> {
public:
	Node* node;
	PoolStruct* pool;
	YAML::iterator ite;
	IYamlNode::Variant value;
	YamlIterator(Node* node, PoolStruct* pool)
		: node(node), pool(pool) {
	}
	YamlIterator(YamlIterator const& o)
		: node(o.node) {
	}
	YamlIterator(YamlIterator&& o)
		: node(o.node) {
	}
	using BaseType = vstd::linq::Iterator<const IYamlNode::Variant>;
	VENGINE_LINQ_DECLARE_COPY_MOVE(BaseType, YamlIterator)
	void GetValue(YAML::iterator const& value, IYamlNode::Variant& var);
	IYamlNode::Variant const* Init() override {
		ite = node->begin();
		return &value;
	}
	IYamlNode::Variant const* Available() override {
		if (ite == node->end()) return nullptr;
		GetValue(ite, value);
		return &value;
	}
	void GetNext() override {
		++ite;
	}
};

class YamlNodeImpl : public IYamlNode {
private:
	PoolStruct* pool;
	Node node;

public:
	Node& GetNode() { return node; }
	YamlNodeImpl(
		PoolStruct* pool)
		: pool(pool),
		  node(Node::ZombieNode) {
	}
	template<typename... T>
	YamlNodeImpl(
		PoolStruct* pool,
		T&&... args)
		: pool(pool),
		  node(std::forward<T>(args)...) {}
	void Dispose() override {
		pool->pool.Delete_Lock(pool->mtx, this);
	}
	YAML::Mark Mark() override {
		return node.Mark();
	}
	NodeType::value Type() override {
		return node.Type();
	}

	vstd::unique_ptr<
		vstd::linq::Iterator<
			const Variant>>
	GetIterator() override {
		return new YamlIterator(&node, pool);
	}
	vstd::unique_ptr<IYamlNode>
	 Get(vstd::string_view strv) override {
		return pool->pool.New_Lock(pool->mtx, pool, node[strv]);
	}
	vstd::unique_ptr<IYamlNode>  Get(IYamlNode* nn) override {
		return pool->pool.New_Lock(pool->mtx, pool, node[static_cast<YamlNodeImpl*>(nn)->GetNode()]);
	}
	vstd::unique_ptr<IYamlNode>  Get(int64 arrayIndex) override {
		return pool->pool.New_Lock(pool->mtx, pool, node[arrayIndex]);
	}
	vstd::string_view ToString() override {
		return node.as<vstd::string_view>();
	}
	int64 ToInt() override {
		return node.as<int64>();
	}
	double ToFloat() override {
		return node.as<double>();
	}
	bool IsArray() override {
		return node.IsSequence();
	}
	void operator=(vstd::string_view str) override {
		node = str;
	}
	void operator=(int64 intValue) override {
		node = intValue;
	}
	void operator=(double floatValue) override {
		node = floatValue;
	}
	void operator=(IYamlNode* ptr) override {
		node = static_cast<YamlNodeImpl*>(ptr)->GetNode();
	}
};

class YamlFactory : public IYamlFactory {
public:
	PoolStruct pool;
	YamlFactory() {}
	vstd::unique_ptr<IYamlNode> GetNodeFromFile(vstd::string const& filePath) {
		return pool.pool.New_Lock(pool.mtx, &pool, YAML::LoadFile(filePath.c_str()));
	}
	vstd::unique_ptr<IYamlNode> GetNodeFromString(vstd::string const& strValue) {
		return pool.pool.New_Lock(pool.mtx, &pool, YAML::Load(strValue.c_str()));
	}
};

void YamlIterator::GetValue(YAML::iterator const& value, IYamlNode::Variant& var) {
	if (!value->second.IsValid()) {
		if (!value->first.IsValid()) {
			var = IYamlNode::Variant();
		} else {
			var = IYamlNode::Variant(pool->pool.New_Lock(pool->mtx, pool, value->first));
		}
	} else {
		var = IYamlNode::Variant(
			vstd::unique_ptr<IYamlNode>(pool->pool.New_Lock(pool->mtx, pool, value->first)),
			vstd::unique_ptr<IYamlNode>(pool->pool.New_Lock(pool->mtx, pool, value->second)));
	}
}

static YamlFactory factory;
}// namespace YAML
YAML_CPP_API_EXTERNC YAML::IYamlFactory* Yaml_GetFactory() {
	return &YAML::factory;
}