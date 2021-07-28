#pragma vengine_package vengine_network

#include <NetworkInclude.h>
#include <Common/DynamicLink.h>
namespace toolhub::net {
static NetWorkImpl network;
}
toolhub::net::NetWork const* NetWork_GetFactory() {
	return &toolhub::net::network;
}

VENGINE_LINK_FUNC(NetWork_GetFactory);