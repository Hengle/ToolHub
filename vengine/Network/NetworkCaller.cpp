#pragma vengine_package vengine_network

#include <NetworkCaller.h>
#include <ISocket.h>
namespace toolhub::net {
static constexpr uint8_t REGIST_MESSAGE_FLAG = 1;
static constexpr uint8_t CALL_FUNC_FLAG = 2;

template<typename T>
T PopValue(std::span<uint8_t>& sp) {
	if constexpr (std::is_same_v<T, vstd::string>) {
		auto strLen = PopValue<uint16_t>(sp);
		auto ptr = sp.data() + sizeof(uint16_t);
		sp = std::span<uint8_t>(ptr + strLen, sp.size() - (strLen + sizeof(uint16_t)));
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

void NetworkCaller::Read() {
	vstd::vector<uint8_t> buffer;
	buffer.reserve(maxBufferSize);
	std::span<uint8_t> sp = buffer;

	while (socket->Read(buffer, maxBufferSize)) {
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
void NetworkCaller::Write() {
	while (auto f = writeCmd.Pop()) {
		//PushValue<uint8_t>(CALL_FUNC_FLAG, data);
	}
}
void NetworkCaller::InitMessageMap(std::span<uint8_t> sp) {
	while (sp.size() > 0) {
		auto name = PopValue<vstd::string>(sp);
		auto id = PopValue<uint>(sp);
		messageMap.Emplace(std::move(name), id);
	}
}
vstd::vector<uint8_t> NetworkCaller::WriteMessageMap() {
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
NetworkCaller::NetworkCaller(
	vstd::unique_ptr<ISocket>&& socket,
	vstd::vector<Function>&& funcs,
	size_t maxBufferSize)
	: funcMap(std::move(funcs)),
	  maxBufferSize(maxBufferSize),
	  socket(std::move(socket)) {
	assert(socket->ConcurrentThread() >= 2);
	readThread.New(
		[this]() {
			Read();
		});
	writeThread.New(
		[this]() {
			Write();
		});
}
NetworkCaller::~NetworkCaller() {
	auto disposeThread = [&](vstd::optional<std::thread>& td) {
		if (td) {
			td->join();
		}
	};
	disposeThread(readThread);
	disposeThread(writeThread);
}
void NetworkCaller::SendMessage(
	vstd::string const& messageName,
	std::span<uint8_t> const& data) {
	vstd::vector<uint8_t> vec;
	vec.reserve(messageName.size() + data.size() + sizeof(uint64));
	PushValue(messageName, vec);
	auto lastSize = vec.size();
	vec.resize(lastSize + data.size());
	memcpy(vec.data() + lastSize, data.data(), data.size());
	writeCmd.Push(
		std::move(vec));
	//TODO
}
}// namespace toolhub::net