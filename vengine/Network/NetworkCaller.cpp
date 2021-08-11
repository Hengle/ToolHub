#pragma vengine_package vengine_network

#include <Network/INetworkService.h>
#include <Network/ISocket.h>
#include <Utility/TaskThread.h>
#include <Network/NetworkInclude.h>
#include <Network/FunctionSerializer.h>
#include <Network/IRegistObject.h>
#include <Network/ObjectRegister.h>
namespace toolhub::net {

static constexpr uint8_t REGIST_MESSAGE_FLAG = 1;
static constexpr uint8_t CALL_FUNC_FLAG = 2;

template<typename T>
T PopValue(std::span<uint8_t>& sp) {
	return vstd::SerDe<std::remove_cvref_t<T>>::Get(sp);
};
template<typename T>
void PushValue(T const& data, vstd::vector<uint8_t>& arr) {
	return vstd::SerDe<std::remove_cvref_t<T>>::Set(arr);
}
class NetworkCaller final : public INetworkService, public vstd::IOperatorNewBase {
public:
	using Function = std::pair<Runnable<void(std::span<uint8_t>)>, vstd::string>;

private:
	struct ClassMemberFunctions {
		Runnable<IRegistObject*()> constructor;
		HashMap<
			vstd::string,
			uint64>
			memberFuncs;
		vstd::vector<Runnable<void(IRegistObject*, std::span<uint8_t>)>> funcs;
		ClassMemberFunctions() {}
	};
	static constexpr uint CONSTRUCTOR_INDEX = std::numeric_limits<uint>::max();
	static constexpr uint DISPOSE_INDEX = CONSTRUCTOR_INDEX - 1;
	ObjectRegister objRegister;
	vstd::unique_ptr<ISocket> socket;
	vstd::optional<std::thread> readThread;
	vstd::optional<TaskThread> writeThread;
	LockFreeArrayQueue<vstd::vector<uint8_t>> writeCmd;
	HashMap<Type, uint64> rpcClassIndices;
	vstd::vector<ClassMemberFunctions> clsFunctions;
	bool isServer;

	struct Func {
		NetworkCaller* ths;
		void operator()() const {
			ths->Write();
		}
	};
	Func func;
	size_t maxBufferSize;
	void InitializeRegistObject(IRegistObject* newPtr, uint64 typeIndex) {
		newPtr->AddDisposeFunc([this](IRegistObject* obj) {
			SendDisposeMessage(obj);
		});
		newPtr->netSer = this;
		newPtr->typeIndex = typeIndex;
	}
	void Read() {
		vstd::vector<uint8_t> buffer;
		buffer.reserve(maxBufferSize);

		while (socket->Read(buffer, maxBufferSize)) {
			std::span<uint8_t> sp = buffer;
			if (buffer.empty()) continue;
			FunctionCallCmd callCmd = vstd::SerDe<FunctionCallCmd>::Get(sp);
			if (callCmd.funcIndex == CONSTRUCTOR_INDEX) {
				auto&& cls = clsFunctions[callCmd.typeIndex];
				auto newPtr = objRegister.CreateObjByRemote(cls.constructor, callCmd.instanceID);
				InitializeRegistObject(newPtr, callCmd.typeIndex);
				continue;
			}
			auto instance = objRegister.GetObject(callCmd.instanceID);
			if (!instance) continue;
			if (callCmd.funcIndex == DISPOSE_INDEX) {
				instance->msgSended = true;
				instance->Dispose();
				continue;
			}
			if (callCmd.typeIndex >= clsFunctions.size()) continue;
			auto&& funcvec = clsFunctions[callCmd.typeIndex].funcs;
			if (callCmd.funcIndex >= funcvec.size()) continue;
			funcvec[callCmd.funcIndex](instance, sp);
		}
	}
	void Dispose() override {
		delete this;
	}
	void Write() {
		while (auto f = writeCmd.Pop()) {
			if (!socket->Write(*f)) return;
		}
	}

	bool enabled = true;
	struct FunctionCallCmd {
		uint64 instanceID;
		uint typeIndex;
		uint funcIndex;
	};
	void RunNext() {
		writeThread->ExecuteNext();
	}

public:
	void Run() override {
		readThread.New(
			[this]() {
				Read();
			});
		writeThread.New();
		writeThread->SetFunctor(func);
	}
	void AddFunc(
		Type tarType,
		vstd::string&& name,
		Runnable<void(IRegistObject*, std::span<uint8_t>)> func) override {
		auto ite = rpcClassIndices.Find(tarType);
		if (!ite) {
			ite = rpcClassIndices.Emplace(tarType, clsFunctions.size());
			clsFunctions.emplace_back();
		}
		auto&& clsMembers = *(clsFunctions.end() - 1);
		clsMembers.memberFuncs.Emplace(
			std::move(name),//Key
			clsMembers.funcs.size());
		clsMembers.funcs.emplace_back(std::move(func));
	}
	void SetConstructor(
		Type tarType,
		Runnable<IRegistObject*()> constructor) override {
		auto ite = rpcClassIndices.Find(tarType);
		if (!ite) {
			ite = rpcClassIndices.Emplace(tarType, clsFunctions.size());
			clsFunctions.emplace_back();
		}
		(clsFunctions.end() - 1)->constructor = std::move(constructor);
	}
	NetworkCaller(
		vstd::unique_ptr<ISocket>&& socket,
		bool isServer,
		size_t maxBufferSize)
		: maxBufferSize(maxBufferSize),
		  isServer(isServer),
		  socket(std::move(socket)) {
		func.ths = this;
	}
	~NetworkCaller() {
		enabled = false;
		auto disposeThread = [&](vstd::optional<std::thread>& td) {
			if (td) {
				td->join();
			}
		};
		disposeThread(readThread);
	}

	void SendDisposeMessage(IRegistObject* obj) {
		if (!enabled) return;
		if (obj->msgSended) return;
		obj->msgSended = true;
		auto typeIndex = obj->typeIndex;
		vstd::vector<uint8_t> vec;
		FunctionCallCmd cmd;
		cmd.instanceID = obj->GetGlobalID();
		cmd.typeIndex = typeIndex;
		cmd.funcIndex = DISPOSE_INDEX;
		vstd::SerDe<FunctionCallCmd>::Set(cmd, vec);
		writeCmd.Push(std::move(vec));
		RunNext();
	}

	IRegistObject* CreateClass(Type tarType) override {
		auto ite = rpcClassIndices.Find(tarType);
		if (!ite) return nullptr;
		uint64 typeIndex = ite.Value();
		auto&& clsMember = clsFunctions[typeIndex];
		auto newPtr = objRegister.CreateObjLocally(clsMember.constructor, isServer);
		InitializeRegistObject(newPtr, typeIndex);
		vstd::vector<uint8_t> vec;
		FunctionCallCmd cmd;
		cmd.instanceID = newPtr->GetLocalID();
		cmd.typeIndex = typeIndex;
		cmd.funcIndex = CONSTRUCTOR_INDEX;
		vstd::SerDe<FunctionCallCmd>::Set(cmd, vec);
		writeCmd.Push(std::move(vec));
		RunNext();
		return newPtr;
	}
	IRegistObject* m_GetObject(uint64 id) override {
		return objRegister.GetObject(id);
	}
	bool CallMemberFunc(
		IRegistObject* ptr,
		vstd::string const& name,
		std::span<uint8_t> arg) override {
		if (ptr->GetLocalID() == std::numeric_limits<uint64>::max()) {
			VEngine_Log("Illegal Network Object!\n");
			VENGINE_EXIT;
			return false;
		}
		auto typeIndex = ptr->typeIndex;
		auto&& cls = clsFunctions[typeIndex];
		auto funcIte = cls.memberFuncs.Find(name);
		if (!funcIte) return false;
		vstd::vector<uint8_t> vec;
		vec.reserve(sizeof(FunctionCallCmd) + arg.size());
		FunctionCallCmd cmd;
		cmd.instanceID = ptr->GetLocalID();
		cmd.typeIndex = typeIndex;
		cmd.funcIndex = funcIte.Value();
		vstd::SerDe<FunctionCallCmd>::Set(cmd, vec);
		vec.push_back_all(arg);
		writeCmd.Push(std::move(vec));
		RunNext();
		return true;
	}
	ISocket* GetSocket() override {
		return socket.get();
	}

	KILL_COPY_CONSTRUCT(NetworkCaller)
	KILL_MOVE_CONSTRUCT(NetworkCaller)
	DECLARE_VENGINE_OVERRIDE_OPERATOR_NEW
};

INetworkService* NetWorkImpl::GetNetworkService(
	vstd::unique_ptr<ISocket>&& socket,
	bool isServer,
	size_t maxBufferSize) const {
	return new NetworkCaller(
		std::move(socket),
		isServer,
		maxBufferSize);
}
}// namespace toolhub::net