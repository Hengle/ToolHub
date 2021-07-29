#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/VObject.h>
#include <Database/IJsonDatabase.h>
#include <Database/IJsonObject.h>
#include <Common/unique_ptr.h>
#include <Network/ISocket.h>
#include <Network/NetworkInclude.h>
#include <Common/DynamicLink.h>
#include <Common/DynamicDLL.h>
#include <Utility/BinaryReader.h>
#include <Network/INetworkService.h>
#include <Database/DatabaseInclude.h>
static toolhub::net::NetWork const* network;
static toolhub::db::Database const* database;

void jsonTest() {
	using namespace toolhub::db;
	auto db = database->CreateSimpleJsonDB();
	auto rootObj = db->GetRootObject();
	auto subArr = db->CreateJsonArray();
	subArr->Add(5);
	subArr->Add(8.3);
	subArr->Add("fuck"_sv);

	auto subObj = db->CreateJsonObject();
	subObj->Set("shit"_sv, 53);
	subObj->Set("shit1"_sv, 12.5);

	rootObj->Set("array"_sv, subArr);
	rootObj->Set("dict"_sv, subObj);

	auto vec = db->Serialize();
	std::cout << "Serialize Size: " << vec.size() << " bytes\n";

	auto updateV = db->Sync();
	std::cout << "Update Size: " << updateV.size() << " bytes\n";

	/////////////// Clone
	auto cloneDB = database->CreateSimpleJsonDB();
	cloneDB->Read(vec);
	cloneDB->Read(updateV);
	auto cloneRoot = cloneDB->GetRootObject();
	auto rIte = cloneRoot->GetIterator();
	auto cloneArr = cloneRoot->GetArray("array"_sv);
	if (cloneArr) {
		auto ite = (*cloneArr)->GetIterator();
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			i->visit(
				func,
				func,
				func,
				func,
				func);
		}
	}
	auto cloneDict = cloneRoot->GetDict("dict"_sv);
	if (cloneDict) {
		auto ite = (*cloneDict)->GetIterator();
		LINQ_LOOP(i, *ite) {
			auto func = [](auto&& f) {
				std::cout << f << '\n';
			};
			std::cout << "key: " << i->key << " Value: ";
			i->value.visit(
				func,
				func,
				func,
				func,
				func);
		}
	}
}
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
		if (cmd.size() != 0) continue;
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
	NETSERVICE_REGIST_MESSAGE(service, Fuck);
	std::cout << "start success!\n";
	service->Run();
	vstd::string s;
	while (true) {
		std::cin >> s;
		NETSERVICE_SEND_MESSAGE(service, Fuck, s);
	}
}

int main() {
	vengine_init_malloc();
	DynamicDLL dll("VEngine_Network.dll");
	DynamicDLL dll1("VEngine_Database.dll");
	network = vstd::TryGetFunction<toolhub::net::NetWork const*()>("NetWork_GetFactory")();
	database = vstd::TryGetFunction<toolhub::db::Database const*()>("Database_GetFactory")();
	jsonTest();
	//server();
	//network->Test();
	system("pause");
	return 0;
}

#undef ASIO_STANDALONE