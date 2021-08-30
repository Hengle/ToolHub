#pragma vengine_package vengine_dll
#include <JobSystem/ThreadTaskHandle.h>
#include <JobSystem/ThreadPool.h>
ThreadTaskHandle::TaskData::TaskData(
	Runnable<void()>&& func)
	: func(std::move(func)) {
	mainThreadLocker.New();
	state.store(static_cast<uint8_t>(TaskState::Waiting), std::memory_order_release);
}
ThreadTaskHandle::TaskData::TaskData() {
	mainThreadLocker.New();
	state.store(static_cast<uint8_t>(TaskState::Waiting), std::memory_order_release);
}
ThreadTaskHandle::TaskData::~TaskData() {
	if (refCount > 0) {
		mainThreadLocker.Delete();
	}
}

std::pair<std::mutex, std::condition_variable>* ThreadTaskHandle::TaskData::GetThreadLocker() {
	std::lock_guard lck(lockerMtx);
	if (refCount == 0) return nullptr;
	refCount++;
	return mainThreadLocker;
}
void ThreadTaskHandle::TaskData::ReleaseThreadLocker() {
	std::unique_lock lck(lockerMtx);
	if ((--refCount) == 0) {
		lck.unlock();
		mainThreadLocker.Delete();
	}
}

ThreadTaskHandle::ThreadTaskHandle(
	ThreadPool* pool) : pool(pool) {
	isArray = false;
	auto ptr = MakeObjectPtr(
		new TaskData());
	taskFlag.New(std::move(ptr));
}
ThreadTaskHandle::ThreadTaskHandle(
	ThreadPool* pool,
	Runnable<void()>&& func) : pool(pool) {
	isArray = false;
	auto ptr = MakeObjectPtr(new TaskData(std::move(func)));
	taskFlag.New(std::move(ptr));
}

ThreadTaskHandle::ThreadTaskHandle(
	ThreadPool* tPool,
	Runnable<void(size_t)>&& func,
	size_t parallelCount,
	size_t threadCount) : pool(tPool) {
	threadCount = Min(threadCount, tPool->workerThreadCount);
	isArray = true;
	taskFlags.New();
	auto&& tasks = *taskFlags;
	size_t eachJobCount = parallelCount / threadCount;
	tasks.reserve(eachJobCount + 1);

	auto AddTask = [&](size_t beg, size_t ed) {
		tasks.emplace_back(MakeObjectPtr(
			new TaskData([=]() {
				for (auto c : vstd::range(beg, ed)) {
					func(c);
				}
			})));
	};
	for (size_t i = 0; i < threadCount; ++i) {
		AddTask(i * eachJobCount, (i + 1) * eachJobCount);
	}
	size_t full = eachJobCount * threadCount;
	size_t lefted = parallelCount - full;
	if (lefted > 0) {
		AddTask(full, parallelCount);
	}
}

ThreadTaskHandle::ThreadTaskHandle(
	ThreadPool* tPool,
	Runnable<void(size_t, size_t)>&& func,
	size_t parallelCount,
	size_t threadCount) : pool(tPool) {
	threadCount = Min(threadCount, tPool->workerThreadCount);
	isArray = true;
	taskFlags.New();
	auto&& tasks = *taskFlags;
	size_t eachJobCount = parallelCount / threadCount;
	tasks.reserve(eachJobCount + 1);
	auto AddTask = [&](size_t beg, size_t ed) {
		tasks.emplace_back(MakeObjectPtr(
			new TaskData([=]() {
				func(beg, ed);
			})));
	};
	for (size_t i = 0; i < threadCount; ++i) {
		AddTask(i * eachJobCount, (i + 1) * eachJobCount);
	}
	size_t full = eachJobCount * threadCount;
	size_t lefted = parallelCount - full;
	if (lefted > 0) {
		AddTask(full, parallelCount);
	}
}

void ThreadTaskHandle::Complete() const {
	struct TPoolCounter {
		ThreadPool* t;
		TPoolCounter(
			ThreadPool* t) : t(t) {
			t->pausedWorkingThread++;
		}
		~TPoolCounter() {
			t->pausedWorkingThread--;
		}
	};
	TPoolCounter tcounter(pool);
	auto checkExecuteFunc = [&](ObjectPtr<TaskData> const& p) {
		auto state = static_cast<TaskState>(p->state.load(std::memory_order_acquire));
		if (state != TaskState::Waiting) return;
		pool->ExecuteTask(p);
	};
	auto func = [&](TaskData* p) {
		auto state = static_cast<TaskState>(p->state.load(std::memory_order_acquire));
		if (state == TaskState::Finished) return;
		auto mtxPtr = p->GetThreadLocker();
		if (mtxPtr) {
			auto disp = vstd::create_disposer([&]() {
				p->ReleaseThreadLocker();
			});
			{
				std::unique_lock lck(mtxPtr->first);
				while (p->state.load(std::memory_order_acquire) != static_cast<uint8_t>(TaskState::Finished)) {
					mtxPtr->second.wait(lck);
				}
			}
		}
	};
	if (isArray) {
		for (auto& taskFlag : *taskFlags)
			checkExecuteFunc(taskFlag);
		pool->ActiveOneBackupThread();
		for (auto& taskFlag : *taskFlags) {
			func(taskFlag);
		}
	} else {
		checkExecuteFunc(*taskFlag);
		pool->ActiveOneBackupThread();
		func(*taskFlag);
	}
}

bool ThreadTaskHandle::IsComplete() const {
	if (!pool) return true;
	auto func = [&](TaskData* p) {
		return static_cast<TaskState>(p->state.load(std::memory_order_relaxed)) == TaskState::Finished;
	};
	if (isArray) {
		for (auto& taskFlag : *taskFlags) {
			if (!func(taskFlag)) return false;
		}
		return true;
	} else {
		return func(*taskFlag);
	}
}

void ThreadTaskHandle::AddDepend(ThreadTaskHandle const& handle) const {
	AddDepend(std::span<ThreadTaskHandle const>(&handle, 1));
}

void ThreadTaskHandle::AddDepend(std::span<ThreadTaskHandle const> handles) const {

	auto func = [&](ObjectPtr<TaskData> const& selfPtr, ObjectPtr<TaskData> const& dep, uint64& dependAdd) {
		TaskData* p = dep;
		TaskData* self = selfPtr;
		auto mtxPtr = p->GetThreadLocker();
		if (mtxPtr) {
			auto disp = vstd::create_disposer([&]() {
				p->ReleaseThreadLocker();
			});
			{
				std::unique_lock lck(mtxPtr->first);
				TaskState state = static_cast<TaskState>(p->state.load(std::memory_order_acquire));
				if ((uint8_t)state < (uint8_t)TaskState::Finished) {
					p->dependedJobs.push_back(selfPtr);
					lck.unlock();
					self->dependingJob.push_back(dep);
					dependAdd++;
				}
			}
		}
	};
	auto executeSelf = [&](ObjectPtr<TaskData> const& self, ThreadTaskHandle const& handle) {
		uint64 v = 0;
		if (handle.isArray) {
			for (auto& i : *handle.taskFlags) {
				func(self, i, v);
			}
		} else {
			func(self, *handle.taskFlag, v);
		}
		self->dependCount.fetch_add(v, std::memory_order_relaxed);
	};
	for (auto& handle : handles) {
		if (isArray) {
			for (auto& i : *taskFlags) {
				executeSelf(i, handle);
			}
		} else {
			executeSelf(*taskFlag, handle);
		}
	}
}

void ThreadTaskHandle::Execute() const {

	if (isArray) {
		for (auto& i : *taskFlags) {
			pool->ExecuteTask(i);
		}
	} else {
		pool->ExecuteTask(*taskFlag);
	}
}
ThreadTaskHandle::ThreadTaskHandle(ThreadTaskHandle const& v)
	: pool(v.pool),
	  isArray(v.isArray) {
	if (isArray) {
		taskFlags.New(*v.taskFlags);
	} else {
		taskFlag.New(*v.taskFlag);
	}
}
ThreadTaskHandle::ThreadTaskHandle(ThreadTaskHandle&& v)
	: pool(v.pool),
	  isArray(v.isArray) {
	if (isArray) {
		taskFlags.New(std::move(*v.taskFlags));
	} else {
		taskFlag.New(std::move(*v.taskFlag));
	}
}
ThreadTaskHandle::~ThreadTaskHandle() {
	if (isArray) {
		taskFlags.Delete();
	} else {
		taskFlag.Delete();
	}
}