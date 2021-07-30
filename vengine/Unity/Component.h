#pragma once
#include <Common/Common.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
namespace toolhub::game {
class Component {
private:
	toolhub::db::IJsonDict* dict;

public:
	Component(
		toolhub::db::IJsonDict* dict);
	~Component();
};
}// namespace toolhub::game