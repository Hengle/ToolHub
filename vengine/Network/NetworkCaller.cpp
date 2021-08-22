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
	static constexpr uint UPDATE_GUID = DISPOSE_INDEX - 1;
	vstd::unique_ptr<ISocket> socket;
	vstd::optional<std::thread> readThread;
	vstd::optional<TaskThread> writeThread;
	LockFreeArrayQueue<vstd::vector<uint8_t>> writeCmd;
	HashMap<Type, uint64> rpcClassIndices;
	vstd::vector<ClassMemberFunctions> clsFunctions;
	bool isServer;
	vstd::Guid selfGuid;
	vstd::Guid remoteGuid;

	struct Func {
		NetworkCaller* ths;
		void operator()() const {
			ths->Write();
		}
	};
	Func func;
	size_t maxBufferSize;
	void InitializeRegistObject(IRegistObject* newPtr) {
		newPtr->AddDisposeFunc([this](IRegistObject* obj) {
			SendDisposeMessage(obj);
		});
	}
	void InitializeRegistObject(IRegistObject* newPtr, uint64 typeIndex) {
		InitializeRegistObject(newPtr);
		newPtr->typeIndex = typeIndex;
	}

	void Read() {
		auto objRegister = ObjectRegister::GetSingleton();
		vstd::vector<uint8_t> buffer;
		buffer.reserve(maxBufferSize);

		while (socket->Read(buffer, maxBufferSize)) {
			std::span<uint8_t> sp = buffer;
			while (sp.size() > 0) {
				if (buffer.empty()) break;
				FunctionCallCmd callCmd = vstd::SerDe<FunctionCallCmd>::Get(sp);
				switch (callCmd.funcIndex) {
					case CONSTRUCTOR_INDEX: {
						auto&& cls = clsFunctions[callCmd.typeIndex];
						auto newPtr = objRegister->CreateObjByRemote(cls.constructor, callCmd.guid);
						InitializeRegistObject(newPtr, callCmd.typeIndex);
					} break;

					case DISPOSE_INDEX: {
						auto instance = objRegister->GetObject(callCmd.guid);
						if (!instance) break;
						instance->netSer = this;
						instance->Dispose();
					} break;
					case UPDATE_GUID: {
						remoteGuid = callCmd.guid;
					} break;
					default: {
						if (callCmd.typeIndex >= clsFunctions.size()) break;
						auto&& funcvec = clsFunctions[callCmd.typeIndex].funcs;
						if (callCmd.funcIndex >= funcvec.size()) break;
						auto instance = objRegister->GetObject(callCmd.guid);
						if (!instance) break;
						instance->netSer = this;
						auto disp = vstd::create_disposer([&]() {
							instance->netSer = nullptr;
						});
						funcvec[callCmd.funcIndex](instance, sp);
					}
				}

				auto instance = objRegister->GetObject(callCmd.guid);
			}
		}
	}
	void Dispose() override {
		delete this;
	}
	void Write() {
		while (auto f = writeCmd.Pop()) {
			if (!socket->Write(*f)) {
				return;
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	bool enabled = true;
	struct FunctionCallCmd {
		vstd::Guid::GuidData guid;
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
		//Update Guid Command
		{
			FunctionCallCmd guidCmd = {
				selfGuid.ToBinary(),
				0, UPDATE_GUID};
			vstd::vector<uint8_t> vec;
			vstd::SerDe<FunctionCallCmd>::Set(guidCmd, vec);
			writeCmd.Push(std::move(vec));
		}
		writeThread->ExecuteNext();
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
		  socket(std::move(socket)),
		  selfGuid(true),
		  remoteGuid(false) {
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
		if (obj->netSer == this) return;
		auto typeIndex = obj->typeIndex;
		vstd::vector<uint8_t> vec;
		FunctionCallCmd cmd;
		cmd.guid = obj->GetGUID().ToBinary();
		cmd.typeIndex = typeIndex;
		cmd.funcIndex = DISPOSE_INDEX;
		vstd::SerDe<FunctionCallCmd>::Set(cmd, vec);
		writeCmd.Push(std::move(vec));
		RunNext();
	}
	void SendCreateMessage(IRegistObject* obj) {
		vstd::vector<uint8_t> vec;
		FunctionCallCmd cmd;
		cmd.guid = obj->GetGUID().ToBinary();
		cmd.typeIndex = obj->typeIndex;
		cmd.funcIndex = CONSTRUCTOR_INDEX;
		vstd::SerDe<FunctionCallCmd>::Set(cmd, vec);
		writeCmd.Push(std::move(vec));
		RunNext();
	}
	IRegistObject* CreateClass(Type tarType) override {
		auto ite = rpcClassIndices.Find(tarType);
		if (!ite) return nullptr;
		uint64 typeIndex = ite.Value();
		auto&& clsMember = clsFunctions[typeIndex];
		auto objRegister = ObjectRegister::GetSingleton();
		auto newPtr = objRegister->CreateObjLocally(clsMember.constructor);
		InitializeRegistObject(newPtr, typeIndex);
		SendCreateMessage(newPtr);
		return newPtr;
	}
	vstd::Guid const& GetSelfGuid() override {
		return selfGuid;
	}
	vstd::Guid const& GetRemoteGuid() override {
		return remoteGuid;
	}
	IRegistObject* m_GetObject(vstd::Guid const& id) override {
		auto objRegister = ObjectRegister::GetSingleton();
		return objRegister->GetObject(id);
	}
	void AddExternalClass(IRegistObject* obj) override {
		if (!obj->guid) return;
		InitializeRegistObject(obj);
		SendCreateMessage(obj);
	}
	bool CallMemberFunc(
		IRegistObject* ptr,
		vstd::string const& name,
		std::span<uint8_t> arg) override {
		if (!ptr->GetGUID()) {
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
		cmd.guid = ptr->GetGUID().ToBinary();
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