#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <Common/VObject.h>
#include <span>
class ThreadPool;
class VENGINE_DLL_COMMON ThreadTaskHandle {
	friend class ThreadPool;

public:
	enum class TaskState : uint8_t {
		Waiting,
		Executed,
		Working,
		Finished
	};

private:
	struct VENGINE_DLL_COMMON TaskData : public vstd::IOperatorNewBase {
		std::atomic_uint8_t state;
		Runnable<void()> func;
		//ThreadPool Set
		/* std::mutex mtx;
		std::condition_variable cv;*/
		int64 refCount = 1;
		StackObject<std::pair<std::mutex, std::condition_variable>> mainThreadLocker;
		spin_mutex lockerMtx;
		vstd::vector<ObjectPtr<TaskData>> dependedJobs;
		vstd::vector<ObjectPtr<TaskData>> dependingJob;
		std::atomic_size_t dependCount = 0;
		TaskData();
		TaskData(Runnable<void()>&& func);
		~TaskData();
		std::pair<std::mutex, std::condition_variable>* GetThreadLocker();
		void ReleaseThreadLocker();
	};

	bool isArray;
	union {
		StackObject<ObjectPtr<TaskData>> taskFlag;
		StackObject<vstd::vector<ObjectPtr<TaskData>>> taskFlags;
	};
	ThreadPool* pool;
	ThreadTaskHandle(
		ThreadPool* pool);
	ThreadTaskHandle(
		ThreadPool* pool,
		Runnable<void()>&& func);
	ThreadTaskHandle(
		ThreadPool* pool,
		Runnable<void(size_t)>&& func,
		size_t parallelCount,
		size_t threadCount);
	ThreadTaskHandle(
		ThreadPool* pool,
		Runnable<void(size_t, size_t)>&& func,
		size_t parallelCount,
		size_t threadCount);

public:
	~ThreadTaskHandle();
	ThreadTaskHandle();
	ThreadTaskHandle(ThreadTaskHandle const& v);
	void AddDepend(ThreadTaskHandle const& handle) const;
	void AddDepend(std::span<ThreadTaskHandle const> handles) const;
	void AddDepend(std::initializer_list<ThreadTaskHandle const> handles) const {
		AddDepend(std::span<ThreadTaskHandle const>(handles.begin(), handles.end()));
	}
	ThreadTaskHandle(ThreadTaskHandle&& v);
	void operator=(ThreadTaskHandle const& v) {
		this->~ThreadTaskHandle();
		new (this) ThreadTaskHandle(v);
	}
	void operator=(ThreadTaskHandle&& v) {
		this->~ThreadTaskHandle();
		new (this) ThreadTaskHandle(std::move(v));
	}
	void Complete() const;
	bool IsComplete() const;
	void Execute() const;
};