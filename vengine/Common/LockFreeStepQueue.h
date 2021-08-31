#pragma once
#include <Common/LockFreeArrayQueue.h>
template<typename T, size_t QUEUE_SET_POW = 2, VEngine_AllocType allocType = VEngine_AllocType::VEngine>
class LockFreeStepQueue {
	static constexpr size_t QUEUE_SET = 1 << QUEUE_SET_POW;
	StackObject<LockFreeArrayQueue<T, allocType>> queues[QUEUE_SET];
	std::atomic_uint count = 0;
	std::atomic_uint readCount = 0;

public:
	LockFreeStepQueue() {
		for (auto&& i : queues) {
			i.New();
		}
	}
	LockFreeStepQueue(size_t capacity) {
		for (auto&& i : queues) {
			i.New(capacity);
		}
	}
	~LockFreeStepQueue() {
		for (auto&& i : queues) {
			i.Delete();
		}
	}
	template<typename... Args>
	void Push(Args&&... args) {
		uint v = (count.fetch_add(1, std::memory_order_relaxed)) & (QUEUE_SET - 1);
		queues[v]->Push(std::forward<Args>(args)...);
	}
	vstd::optional<T> Pop() {
		uint start = (readCount.fetch_add(1, std::memory_order_relaxed)) & (QUEUE_SET - 1);
		uint end = start + QUEUE_SET;
		for (uint i = start; i < end; ++i) {
			auto opt = queues[i & (QUEUE_SET - 1)]->Pop();
			if (opt) return opt;
		}
		return vstd::optional<T>();
	}
};
