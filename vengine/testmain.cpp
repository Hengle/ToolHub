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
#include <Database/DatabaseExample.h>
#include <Network/IRegistObject.h>
static toolhub::net::NetWork const* network;
static toolhub::db::Database const* database;
bool isServer;
vstd::vector<vstd::unique_ptr<toolhub::net::INetworkService>> services;
std::mutex mtx;
//#define SERVER

class TestClass : public toolhub::net::IRegistObject {
public:
	void Run(vstd::string&& str) { std::cout << "from remote: " << str << '\n'; }
};

void server(uint port) {
	Runnable<toolhub::net::INetworkService*()> getService;
	Runnable<toolhub::net::INetworkService*()> getServiceWithDebugInfo;
	std::cout << "server(Y) or client(N)?" << '\n';
	vstd::string cmd;
	std::cin >> cmd;
	while (true) {
		if (cmd.size() != 1) continue;
		if (cmd[0] == 'y' || cmd[0] == 'Y') {
			isServer = true;
			getService = [&]() {
				std::cout << "starting server...\n";
				return network->GetNetworkService(network->GenServerTCPSock(port), isServer);
			};
			break;
		} else if (cmd[0] == 'n' || cmd[0] == 'N') {
			isServer = false;
			getService = [&]() {
				std::cout << "starting client...\n";
				BinaryReader reader("ip.txt");
				auto data = reader.Read();
				return network->GetNetworkService(
					network->GenClientTCPSock(port, (char const*)data.data()), isServer);
			};
			break;
		}
	}
	getServiceWithDebugInfo = [&]() {
		auto ptr = getService();
		if (ptr->GetSocket()->Connect()) {
			std::cout << "start success!\n";
		} else {
			std::cout << "start failed! error message: " << ptr->GetSocket()->ErrorMessage() << '\n';
		}
		return ptr;
	};
	auto initService = [&](auto ptr) {
		ptr->RegistClass<TestClass>();
		ptr->NET_REGIST_MEMBER(TestClass, Run);
		ptr->Run();
	};
	if (isServer) {
		std::cout << "main thread waiting...\n";
		TestClass* tt = nullptr;
		std::thread t([&]() {
			while (true) {
				auto ptr = getServiceWithDebugInfo();
				port++;
				initService(ptr);
				std::lock_guard lck(mtx);
				if (tt) {
					ptr->AddExternalClass(tt);
				}
				services.emplace_back(ptr);
			}
		});
		system("pause");
		vstd::string s;
		while (true) {
			std::cin >> s;
			if (!tt) {
				std::lock_guard lck(mtx);
				if (services.empty()) {
					std::cout << "no connection yet!\n";
					continue;
				}
				tt = services[0]->CreateClass<TestClass>();
				if (services.size() > 1) {
					for (auto&& i : vstd::ptr_range(&services[1], services.end())) {
						i->AddExternalClass(tt);
					}
				}
			}
			for (auto&& i : services) {
				i->NET_CALL_FUNC(tt, Run, s);
			}
		}

		t.join();
	} else {
		vstd::unique_ptr<toolhub::net::INetworkService> service = getServiceWithDebugInfo();
		initService(service.get());
		while (true) {
			std::this_thread::sleep_for(std::chrono::hours(1000));
		}
	}
}
#include <Utility/VGuid.h>
#include <Utility/StringUtility.h>
int main(int argc, char* argv[]) {
	vengine_init_malloc();
	/*
	if (argc != 2) {
		std::cout << "error argcount!";
		system("pause");
		return 0;
	}
	*/
	auto port = 2001;
	//StringUtil::StringToInteger(vstd::string_view(argv[1]));

	DynamicDLL dll("VEngine_Network.dll");
	DynamicDLL dll1("VEngine_Database.dll");
	network = dll.GetDLLFunc<toolhub::net::NetWork const*()>("NetWork_GetFactory")();

	database = dll1.GetDLLFunc<toolhub::db::Database const*()>("Database_GetFactory")();
	jsonTest(database);
	//jsonTest(database);
	//server(port);
	system("pause");

	return 0;
}