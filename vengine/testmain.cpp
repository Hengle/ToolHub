#pragma vengine_package vengine_compute
#include <Common/Common.h>
#include <Common/DynamicDLL.h>
#include <Utility/BinaryReader.h>
#include <Network/NetworkInclude.h>
#include <Network/ISocket.h>
#include <Network/FunctionSerializer.h>
#include <Database/DatabaseExample.h>
#include <JobSystem/ThreadPool.h>
#include <Common/LockFreeArrayQueue.h>
#include <array>
void server() {
	using namespace toolhub::net;
	DllFactoryLoader<NetWork> loader("VEngine_Network.dll", "NetWork_GetFactory");
	auto network = loader();
	std::cout << "0: server\n"
			  << "other: client\n";
	vstd::string str;
	std::cin >> str;
	vstd::unique_ptr<ISocket> socket;
	vstd::unique_ptr<ISocketAcceptor> acceptor;
	if (str == "0") {
		acceptor = network->GenServerAcceptor(2001);
		while (true) {
			socket = network->GenServerTCPSock(acceptor.get());
			socket->Connect();
			std::cout << "Connected to client!\n";
			std::thread t([localSock = std::move(socket)]() {
				vstd::vector<uint8_t> data;
				while (true) {
					if (!localSock->Read(data, sizeof(uint64))) {
						std::cout << localSock->ErrorMessage() << '\n';
					}

					localSock->Read(data, *(uint64*)data.data());
					std::span<uint8_t const> sp = data;
					std::cout << vstd::SerDe<vstd::string>::Get(sp) << '\n';
				}
			});
			t.detach();
		}
	} else {
		socket = network->GenClientTCPSock(2001, "127.0.0.1");
		socket->Connect();
		std::cout << "Connected to server!\n";

		vstd::vector<uint8_t> data;
		while (true) {
			data.clear();
			vstd::string str;
			std::cin >> str;
			vstd::SerDe<vstd::string>::Set(str, data);
			uint64 sz = data.size();
			std::span<uint8_t> sp(reinterpret_cast<uint8_t*>(&sz), sizeof(uint64));
			socket->Write(sp);
			socket->Write(data);
		}
	}
}
ThreadTaskHandle TaskWithDepend(ThreadPool& pool, ThreadTaskHandle const& externalTask) {
	auto task0 = pool.GetTask([]() {
		// costly task
	});
	task0.AddDepend(externalTask);
	auto task1 = pool.GetTask([]() {

	});
	task1.AddDepend(task0);
	auto task2 = pool.GetTask([]() {

	});
	task2.AddDepend(task0);

	auto finalTask = pool.GetTask([]() {

	});
	finalTask.AddDepend({task1, task2});
	return finalTask;
}

ThreadTaskHandle TaskLayer(ThreadPool& pool, ThreadTaskHandle& externalTask) {
	return pool.GetTask(
		[&, externalTask = std::move(externalTask)]() {
			externalTask.Complete();
			auto task0 = pool.GetTask([]() {
				// costly task
			});
			auto task1 = pool.GetTask([]() {
				// costly task
			});
			auto finalTask = pool.GetTask([]() {
				// costly task
			});
			task0.Execute();
			task1.Execute();

			task0.Complete();
			task1.Complete();

			finalTask.Complete();
		});
}
int main() {
	ThreadPool pool(16);
	auto bg = clock();
	for (int i = 0; i < 3; ++i) {
		auto task0 = pool.GetTask([]() {
			// costly task
		});
		TaskLayer(pool, task0).Complete();
	}
	auto ed = clock();
	return 0;
}