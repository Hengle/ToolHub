#pragma once

#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Utility/ObjectTracker.h>
namespace toolhub {
class IRegistObject : public vstd::IDisposable, public vstd::IOperatorNewBase {
private:
	vstd::vector<Runnable<void(IRegistObject*)>> disposeFuncs;
	uint64 id;
	bool createdLocally;

protected:
	virtual ~IRegistObject();
	IRegistObject(
		uint64 id,
		bool createdLocally);

public:
	virtual void Dispose() override { delete this; }
	uint64 GetLocalID() const { return id; }
	bool IsCreatedLocally() const { return createdLocally; }
	void AddDisposeFunc(Runnable<void(IRegistObject*)>&& func);
};
}// namespace toolhub