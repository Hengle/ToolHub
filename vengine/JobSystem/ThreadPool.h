#pragma once
#include <Common/Common.h>
#include <Common/VObject.h>
#include <Common/LockFreeArrayQueue.h>
#include <JobSystem/ThreadTaskHandle.h>
class VENGINE_DLL_COMMON ThreadPool final {
	friend class ThreadTaskHandle;
	vstd::vector<std::thread> threads;
	vstd::vector<std::thread> backupThreads;
	LockFreeArrayQueue<ObjectPtr<ThreadTaskHandle::TaskData>> taskList;
	void ThreadExecute(ThreadTaskHandle::TaskData*);
	std::atomic_flag enabled;
	std::mutex threadLock;
	std::mutex backupThreadLock;
	spin_mutex threadVectorLock;
	std::condition_variable cv;
	std::condition_variable backupCV;
	void ExecuteTask(ObjectPtr<ThreadTaskHandle::TaskData> const& task);
	size_t workerThreadCount;
	void ActiveOneBackupThread();
	ThreadTaskHandle M_GetParallelTask(Runnable<void(size_t)>&& func, size_t parallelCount, size_t threadCount);
	ThreadTaskHandle M_GetBeginEndTask(Runnable<void(size_t, size_t)>&& func, size_t parallelCount, size_t threadCount);
	Runnable<void(size_t)> runBackupThread;
	int64_t waitingBackupThread = 0;
	std::atomic_int64_t pausedWorkingThread = 0;
public:
	//Thread Execute
	ThreadPool(size_t targetThreadCount);
	~ThreadPool();
	static bool IsWorkerThread();
	ThreadTaskHandle GetTask(Runnable<void()> func);
	ThreadTaskHandle GetFence();
	ThreadTaskHandle GetParallelTask(Runnable<void(size_t)> func, size_t parallelCount, size_t threadCount) {
		return M_GetParallelTask(std::move(func), parallelCount, threadCount);
	}
	ThreadTaskHandle GetParallelTask(Runnable<void(size_t)> func, size_t parallelCount) {
		return M_GetParallelTask(std::move(func), parallelCount, parallelCount);
	}

	ThreadTaskHandle GetBeginEndTask(Runnable<void(size_t, size_t)> func, size_t parallelCount, size_t threadCount) {
		return M_GetBeginEndTask(std::move(func), parallelCount, threadCount);
	}
	ThreadTaskHandle GetBeginEndTask(Runnable<void(size_t, size_t)> func, size_t parallelCount) {
		return M_GetBeginEndTask(std::move(func), parallelCount, parallelCount);
	}
};