#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Utility/ObjectTracker.h>
namespace toolhub::net {
class ObjectRegister;
class INetworkService;
class NetworkCaller;
class IRegistObject : public vstd::IDisposable, public vstd::IOperatorNewBase {
	friend class ObjectRegister;
	friend class NetworkCaller;

private:
	INetworkService* netSer = nullptr;
	vstd::vector<Runnable<void(IRegistObject*)>> disposeFuncs;
	uint64 id = std::numeric_limits<uint64>::max();
	uint64 typeIndex = std::numeric_limits<uint64>::max();
	bool msgSended = false;

protected:
	virtual ~IRegistObject() {
		for (auto&& i : disposeFuncs) {
			i(this);
		}
	}

public:
	virtual void Dispose() override { delete this; }
	uint64 GetLocalID() const { return id; }
	uint64 GetGlobalID() const { return id >> 1; }
	INetworkService* GetNetworkService() const { return netSer; }
	void AddDisposeFunc(Runnable<void(IRegistObject*)>&& func) {
		disposeFuncs.emplace_back(std::move(func));
	}
};

}// namespace toolhub::net