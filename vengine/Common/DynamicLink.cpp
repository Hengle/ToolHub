#pragma vengine_package vengine_dll
#include <Common/DynamicLink.h>
namespace vstd {
struct LinkTarget {
	Runnable<void(), VEngine_AllocType::Default> funcPtr;
#ifdef DEBUG
	Type funcType;
	LinkTarget(
		Type funcType,
		Runnable<void(), VEngine_AllocType::Default>&& funcPtr) : funcType(funcType), funcPtr(std::move(funcPtr)) {}
#else
	LinkTarget(
		Runnable<void(), VEngine_AllocType::Default>&& funcPtr) : funcPtr(std::move(funcPtr)) {}
#endif
	void const* Ptr() const {
		return reinterpret_cast<void const*>(&funcPtr);
	}
};
struct LinkMap {
	using HashMapType = HashMap<string_view, LinkTarget, hash<string_view>, std::equal_to<string_view>, VEngine_AllocType::Default>;
	StackObject<HashMapType> map;
	spin_mutex mtx;
	LinkMap() {
		{
			std::lock_guard lck(mtx);
			map.New(256);
		}
	}
	HashMapType* operator->() { return map; }
	~LinkMap() {
		map.Delete();
	}
};
static LinkMap& GetLinkerHashMap() {
	static LinkMap hashMap;
	return hashMap;
}
void AddFunc(
	string_view const& name,
	Type funcType,
	Runnable<void(), VEngine_AllocType::Default>&& funcPtr) {
	auto&& map = GetLinkerHashMap();
	{
		std::lock_guard lck(map.mtx);
#ifdef DEBUG
		auto ite = map->TryEmplace(
			name,
			funcType,
			std::move(funcPtr));
		if (!ite.second) {
			VEngine_Log(
				{"Functor Name Conflict: ",
				 name});
			VENGINE_EXIT;
		}
#else
		map->Emplace(
			name,
			std::move(funcPtr));
#endif
	}
}
void RemoveFunc(
	string_view const& name) {
	auto&& map = GetLinkerHashMap();
	map->Remove(name);
}

void const* GetFuncPair(
	Type checkType,
	string_view const& name) {
	auto&& map = GetLinkerHashMap();
	LinkMap::HashMapType::Index ite;
	{
		std::lock_guard lck(map.mtx);
		ite = map->Find(name);
	}
	if (ite) {
		auto&& v = ite.Value();
		//Not Same Type!
#ifdef DEBUG
		if (strcmp(v.funcType.GetType().name(), checkType.GetType().name()) != 0) {
			VEngine_Log(
				{"Try to access function: ",
				 name,
				 " with wrong type!\n",
				 "input type: ",
				 checkType.GetType().name(),
				 "\ntarget type: ",
				 v.funcType.GetType().name()});
			VENGINE_EXIT;
			return nullptr;
		}
#endif
		return v.Ptr();
	}
	return 0;
}

}// namespace vstd