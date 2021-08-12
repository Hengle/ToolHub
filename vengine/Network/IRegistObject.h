#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Utility/ObjectTracker.h>
#include <Network/FunctionSerializer.h>
#include <Utility/VGuid.h>
namespace toolhub::net {
class ObjectRegister;
class NetworkCaller;
class INetworkService;
class IRegistObject : public vstd::IDisposable, public vstd::IOperatorNewBase {
	friend class ObjectRegister;
	friend class NetworkCaller;

private:
	vstd::vector<Runnable<void(IRegistObject*)>> disposeFuncs;
	vstd::Guid guid;
	uint64 typeIndex;
	INetworkService* netSer = nullptr;

protected:
	IRegistObject() : guid(false) {}
	virtual ~IRegistObject() {
		for (auto&& i : disposeFuncs) {
			i(this);
		}
	}

public:
	/// <summary>
	/// Should only be called in network thread
	/// </summary>
	/// <returns></returns>
	INetworkService* GetNetService() const { return netSer; }
	virtual void Dispose() override { delete this; }
	vstd::Guid const& GetGUID() const { return guid; }
	void AddDisposeFunc(Runnable<void(IRegistObject*)>&& func) {
		disposeFuncs.emplace_back(std::move(func));
	}

};

}// namespace toolhub::net