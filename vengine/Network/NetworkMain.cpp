#pragma vengine_package vengine_network

#include <NetworkInclude.h>
namespace toolhub::net {
static NetWorkImpl network;
}

VENGINE_UNITY_EXTERN toolhub::net::NetWork const* NetWork_GetFactory() {
	return &toolhub::net::network;
}
