#pragma once

#include <Common/Common.h>
#include <Network/INetworkService.h>
#include <Network/ISocket.h>
#include <Network/IRegistObject.h>
namespace toolhub::net {
class ObjectRegister : public vstd::IOperatorNewBase {
public:
	static constexpr uint64 CombineData(uint64 id, bool createdLocally) {
		return (id << 1) | (createdLocally ? 1 : 0);
	}

private:
	struct RegistObj {
		IRegistObject* ptr;
		RegistObj(IRegistObject* ptr)
			: ptr(ptr) {
		}
		~RegistObj() {
			if (ptr)
				ptr->Dispose();
		}
		IRegistObject* get() const {
			return ptr;
		}
	};
	HashMap<uint64, RegistObj> allObjects;
	std::mutex mtx;

	std::atomic_uint64_t incrementalID;
	void DisposeObj(
		uint64 id,
		bool createLocally);
	IRegistObject* CreateObj(
		Runnable<IRegistObject*()> const& creater);

public:
	KILL_COPY_CONSTRUCT(ObjectRegister)
	KILL_MOVE_CONSTRUCT(ObjectRegister)
	ObjectRegister();
	~ObjectRegister();
	IRegistObject* CreateObjLocally(
		Runnable<IRegistObject*()> const& creater);
	IRegistObject* CreateObjByRemote(
		Runnable<IRegistObject*()> const& creater,
		uint64 remoteID);

	IRegistObject* GetObject(uint64 id, bool createLocally);
	IRegistObject* GetObject(uint64 id);
};
}// namespace toolhub::net