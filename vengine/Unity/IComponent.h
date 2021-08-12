#pragma once

#include <Network/IRegistObject.h>
namespace toolhub {
namespace net {
class INetworkService;
}
class IGameObject;

class IComponent : public net::IRegistObject {
public:
	
	
	//virtual void SetCompValue(IndexArray idx, SerValue value) VENGINE_PURE_VIRTUAL;

	template<typename T>
	static void InitializeService(net::INetworkService* service);
};
}// namespace toolhub