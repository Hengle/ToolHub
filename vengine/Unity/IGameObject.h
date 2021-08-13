#pragma once

#include <Common/Common.h>
#include <Network/IRegistObject.h>
#include <Utility/VGuid.h>

namespace toolhub {
namespace net {
class INetworkService;
}
class IComponent;
class IGameObject : public net::IRegistObject {
public:
	///////////////////// Remote Callback

	virtual void AddCompByRemote(vstd::Guid&& compInstID) = 0;
	virtual void RemoveCompByRemote(vstd::Guid&& compInstID) = 0;
	virtual void AddSubGORemote(vstd::Guid&& goInstID) = 0;
	virtual void RemoveSubGORemote(vstd::Guid&& goInstID) = 0;
	virtual void UpdateNameRemote(vstd::string&& newName) = 0;
	virtual void UpdateTagRemote(vstd::string&& newTag) = 0;
	virtual void UpdateLayerRemote(uint newLayer) = 0;
	virtual void UpdateIsStaticRemote(bool isStatic) = 0;
	virtual void SetParentRemote(vstd::Guid&& parentGO) = 0;

	///////////////////// Common Methods
	template<typename T>
	static void InitializeService(net::INetworkService* service);
	virtual ~IGameObject() = default;
};
}// namespace toolhub