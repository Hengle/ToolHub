#pragma once

#include <Common/Common.h>
#include <Network/INetworkService.h>
#include <Network/ISocket.h>
#include <IRegistObject.h>
namespace toolhub {
class ObjectRegister : public vstd::IOperatorNewBase {
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
	static constexpr uint64 CombineData(uint64 id, bool createdLocally) {
		return (id << 1) | (createdLocally ? 1 : 0);
	}
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
	static ObjectRegister* GetSingleton();
	static void DisposeSingleton();
	void CreateObjLocally(
		Runnable<IRegistObject*()> const& creater);
	void CreateObjByRemote(
		Runnable<IRegistObject*()> const& creater,
		uint64 remoteID);

	IRegistObject* GetObject(uint64 id, bool createLocally);
};
}// namespace toolhub