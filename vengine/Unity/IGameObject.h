#pragma once

#include <Common/Common.h>
#include <Network/IRegistObject.h>

namespace toolhub {
namespace net {
class INetworkService;
}
class IComponent;
class IGameObject : public net::IRegistObject {
public:
	///////////////////// Remote Callback
	virtual void AddCompByRemote(uint64 id) VENGINE_PURE_VIRTUAL;
	virtual void RemoveCompByRemote(uint64 id) VENGINE_PURE_VIRTUAL;
	virtual void AddSubGORemote(uint64 id) VENGINE_PURE_VIRTUAL;
	virtual void RemoveSubGORemote(uint64 id) VENGINE_PURE_VIRTUAL;
	virtual void UpdateNameRemote(vstd::string newName) VENGINE_PURE_VIRTUAL;
	virtual void UpdateTagRemote(vstd::string newTag) VENGINE_PURE_VIRTUAL;
	virtual void UpdateLayerRemote(uint newLayer) VENGINE_PURE_VIRTUAL;
	virtual void UpdateIsStaticRemote(bool isStatic) VENGINE_PURE_VIRTUAL;
	///////////////////// Common Methods
	template <typename T>
	static void InitializeService(net::INetworkService* service);
	virtual ~IGameObject() = default;
};
}// namespace toolhub