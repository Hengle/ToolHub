#pragma vengine_package vengine_network

#include <INetworkService.h>
#include <ISocket.h>
#include <Utility/TaskThread.h>
#include <NetworkInclude.h>
namespace toolhub::net {

static constexpr uint8_t REGIST_MESSAGE_FLAG = 1;
static constexpr uint8_t CALL_FUNC_FLAG = 2;

template<typename T>
T PopValue(std::span<uint8_t>& sp) {
	if constexpr (std::is_same_v<T, vstd::string>) {
		auto strLen = PopValue<uint16_t>(sp);
		auto ptr = sp.data();
		sp = std::span<uint8_t>(ptr + strLen, sp.size() - strLen);
		return vstd::string(vstd::string_view(
			reinterpret_cast<char const*>(ptr),
			strLen));
	} else {
		T const* ptr = reinterpret_cast<T const*>(sp.data());
		sp = std::span<uint8_t>(sp.data() + sizeof(T), sp.size() - sizeof(T));
		return *ptr;
	}
};
template<typename T>
void PushValue(T const& data, vstd::vector<uint8_t>& arr) {
	if constexpr (std::is_same_v<T, vstd::string> || std::is_same_v<T, vstd::string_view>) {
		PushValue<uint16_t>(data.size(), arr);
		arr.push_back_all(reinterpret_cast<uint8_t const*>(data.data()), data.size());
	} else {
		arr.push_back_all(reinterpret_cast<uint8_t const*>(&data), sizeof(data));
	}
}
class NetworkCaller final : public INetworkService {
public:
	using Function = std::pair<Runnable<void(std::span<uint8_t>)>, vstd::string>;

private:
	vstd::unique_ptr<ISocket> socket;
	HashMap<vstd::string, uint> messageMap;
	vstd::vector<Function> funcMap;
	vstd::optional<std::thread> readThread;
	vstd::optional<TaskThread> writeThread;
	LockFreeArrayQueue<vstd::vector<uint8_t>> writeCmd;
	struct Func {
		NetworkCaller* ths;
		void operator()() const {
			ths->Write();
		}
	};
	Func func;
	size_t maxBufferSize;

	void Read() {
		vstd::vector<uint8_t> buffer;
		buffer.reserve(maxBufferSize);

		while (socket->Read(buffer, maxBufferSize)) {
			std::span<uint8_t> sp = buffer;
			if (buffer.empty()) continue;
			auto tag = PopValue<uint8_t>(sp);
			switch (tag) {
				case REGIST_MESSAGE_FLAG: {
					InitMessageMap(sp);
				} break;
				case CALL_FUNC_FLAG: {
					auto callID = PopValue<uint>(sp);
					if (callID < funcMap.size()) {
						funcMap[callID].first(sp);
					}
				} break;
			}
		}
	}
	void Write() {
		// Init Method Table
		if (!socket->Write(WriteMessageMap()))
			return;
		// Message
		while (auto f = writeCmd.Pop()) {
			if (!socket->Write(*f)) return;
		}
	}
	void InitMessageMap(std::span<uint8_t> sp) {
		while (sp.size() > 0) {
			auto name = PopValue<vstd::string>(sp);
			auto id = PopValue<uint>(sp);
			messageMap.Emplace(std::move(name), id);
		}
	}
	vstd::vector<uint8_t> WriteMessageMap() {
		vstd::vector<uint8_t> data;
		PushValue<uint8_t>(REGIST_MESSAGE_FLAG, data);
		uint v = 0;
		for (auto&& i : funcMap) {
			PushValue<vstd::string>(i.second, data);
			PushValue<uint>(v, data);
			v++;
		}
		return data;
	}

public:
	NetworkCaller(
		vstd::unique_ptr<ISocket>&& socket,
		vstd::vector<Function>&& funcs,
		size_t maxBufferSize)
		: funcMap(std::move(funcs)),
		  maxBufferSize(maxBufferSize),
		  socket(std::move(socket)) {
		assert(this->socket->ConcurrentThread() >= 2);
		readThread.New(
			[this]() {
				Read();
			});

		writeThread.New();
		func.ths = this;
		writeThread->SetFunctor(func);
		writeThread->ExecuteNext();
	}
	~NetworkCaller() {
		auto disposeThread = [&](vstd::optional<std::thread>& td) {
			if (td) {
				td->join();
			}
		};
		disposeThread(readThread);
	}
	void SendMessage(vstd::string const& messageName, std::span<uint8_t> const& data) override {
		auto ite = messageMap.Find(messageName);
		if (!ite) return;
		vstd::vector<uint8_t> vec;
		vec.reserve(data.size() + sizeof(uint64_t));
		PushValue<uint8_t>(CALL_FUNC_FLAG, vec);
		PushValue<uint>(ite.Value(), vec);
		auto lastSize = vec.size();
		vec.resize(lastSize + data.size());
		memcpy(vec.data() + lastSize, data.data(), data.size());
		writeCmd.Push(
			std::move(vec));
		writeThread->ExecuteNext();
	}
	KILL_COPY_CONSTRUCT(NetworkCaller)
	KILL_MOVE_CONSTRUCT(NetworkCaller)
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};

vstd::unique_ptr<INetworkService> NetWorkImpl::GetNetworkService(
	vstd::unique_ptr<ISocket>&& socket,
	vstd::linq::Iterator<Function>& funcs,
	size_t maxBufferSize) const {
	vstd::vector<Function> vec;
	LINQ_LOOP(i, funcs) {
		vec.push_back(std::move(*i));
	}
	return new NetworkCaller(
		std::move(socket),
		std::move(vec),
		maxBufferSize);
}
}// namespace toolhub::net