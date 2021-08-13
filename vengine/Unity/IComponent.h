#pragma once

#include <Network/IRegistObject.h>
#include <Unity/UnitySerialize.h>
namespace toolhub {
namespace net {
class INetworkService;
}
class IGameObject;

class IComponent : public net::IRegistObject {
public:
	virtual void OnCreateRemote(
		vstd::Guid&& goInstID,
		vstd::string&& compTypeName) = 0;
	virtual void SetValueRemote(
		IndexArray&& idx,
		SerializeValue&& value) = 0;
	virtual void SetTrivialArrayRemote(
		IndexArray&& idx,
		vstd::vector<SerializeValue>&& values) = 0;
	virtual void SetTrivialStructRemote(
		IndexArray&& idx,
		vstd::vector<std::pair<vstd::string, SerializeValue>>&& members) = 0;

	template<typename T>
	static void InitializeService(net::INetworkService* service);
};
}// namespace toolhub