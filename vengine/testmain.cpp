#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/VObject.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Common/unique_ptr.h>
#include <Network/ISocket.h>
#include <Network/NetworkInclude.h>
#include <Common/DynamicDLL.h>
#include <Utility/BinaryReader.h>
#include <Network/INetworkService.h>
#include <Database/DatabaseInclude.h>
static toolhub::net::NetWork const* network;
static toolhub::db::Database const* database;

//#define SERVER
void Fuck(vstd::string v) {
	std::cout << v << '\n';
};

void server() {
	vstd::unique_ptr<toolhub::net::INetworkService> service;
	std::cout << "server(Y) or client(N)?" << '\n';
	vstd::string cmd;
	std::cin >> cmd;
	while (true) {
		if (cmd.size() != 1) continue;
		if (cmd[0] == 'y' || cmd[0] == 'Y') {
			std::cout << "starting server...\n";

			service = network->GetNetworkService(
				network->GenServerTCPSock(2, 2001));
			break;
		} else if (cmd[0] == 'n' || cmd[0] == 'N') {
			std::cout << "starting client...\n";
			BinaryReader reader("ip.txt");
			auto data = reader.Read();
			service = network->GetNetworkService(
				network->GenClientTCPSock(2, 2001, (char const*)data.data()));
			break;
		}
	}
	NETSERVICE_REGIST_MESSAGE(service.get(), Fuck);
	if (service->GetSocket()->Connect()) {
		std::cout << "start success!\n";
	} else {
		std::cout << "start failed! error message: " << service->GetSocket()->ErrorMessage() << '\n';
	}
	service->Run();
	vstd::string s;
	while (true) {
		std::cin >> s;
		NETSERVICE_SEND_MESSAGE(service.get(), Fuck, s);
	}
}

int main() {
	vengine_init_malloc();
	DynamicDLL dll("VEngine_Network.dll");
	DynamicDLL dll1("VEngine_Database.dll");
	network = dll.GetDLLFunc<toolhub::net::NetWork const*()>("NetWork_GetFactory")();
	database = dll1.GetDLLFunc<toolhub::db::Database const*()>("Database_GetFactory")();
	//jsonTest();
	server();
	//network->Test();
	system("pause");
	return 0;
}

#undef ASIO_STANDALONE