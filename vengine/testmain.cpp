#pragma vengine_package vengine_compute

#include <Common/Common.h>
#include <Common/VObject.h>
#include <Database/IJsonDatabase.h>
#include <Database/JsonObject.h>
#include <Common/unique_ptr.h>
#include <Network/TCPSocket.h>
#include <Network/NetworkInclude.h>
#include <Common/DynamicLink.h>
#include <Common/DynamicDLL.h>

void test() {
	using namespace toolhub::db;
	auto db = CreateSimpleJsonDB();
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
	subArr->Set(1, 10.5);
	auto updateV = db->Sync();
	std::cout << "Update Size: " << updateV.size() << " bytes\n";

	/////////////// Clone
	auto cloneDB = CreateSimpleJsonDB();
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
static toolhub::net::NetWork const* network;

void server() {
	std::cout << "start server\n";
	auto ser = vstd::make_unique(network->GenServerTCPSock(1, 2001));
	vstd::string s = "fuck";
	vstd::vector<uint8_t> data;
	while (true) {
		if (!ser->Write(std::span<uint8_t>((uint8_t*)s.data(), s.size() + 1))) return;
		if (!ser->Read(data, 1024)) return;
		std::cout << ((char const*)data.data()) << '\n';
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
void client() {
	std::cout << "start client\n";
	auto cli = vstd::make_unique(network->GenClientTCPSock(1, 2001, "127.0.0.1"));
	vstd::vector<uint8_t> vec;
	vstd::string s = "shit";
	while (true) {
		if (!cli->Read(vec, 1024)) return;
		if (!cli->Write(std::span<uint8_t>((uint8_t*)s.data(), s.size() + 1))) return;
		std::cout << ((char const*)vec.data()) << '\n';
	}
}

//#define SERVER

int main() {
	vengine_init_malloc();
	DynamicDLL dll("VEngine_Network.dll");
	auto v = vstd::TryGetFunction<toolhub::net::NetWork const*()>("NetWork_GetFactory");
	network = v();
#ifdef SERVER
	server();
#else
	client();
#endif

	system("pause");
}

#undef ASIO_STANDALONE