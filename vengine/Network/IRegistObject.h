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
	bool createdLocally = false;
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
	INetworkService* GetNetworkService() const { return netSer; }
	bool IsCreatedLocally() const { return createdLocally; }
	void AddDisposeFunc(Runnable<void(IRegistObject*)>&& func) {
		disposeFuncs.emplace_back(std::move(func));
	}
};
}// namespace toolhub::net