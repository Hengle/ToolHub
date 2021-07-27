#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#ifdef DEBUG
#include <string.h>
#endif
namespace vstd {
VENGINE_DLL_COMMON void AddFunc(
	string_view const& name,
	Type funcType,
	Runnable<void(), VEngine_AllocType::Default>&& funcPtr);

VENGINE_DLL_COMMON void RemoveFunc(
	string_view const& name);
VENGINE_DLL_COMMON void const* GetFuncPair(
	Type checkType,
	string_view const& name);
template<typename T>
void LoadFunction_T(
	string_view const& name,
	Runnable<functor_t<T>, VEngine_AllocType::Default>&& func) {
	AddFunc(name, typeid(functor_t<T>), reinterpret_cast<Runnable<void(), VEngine_AllocType::Default>&&>(func));
}
template<typename T>
struct FunctionLoader {
	string_view name;
	FunctionLoader(
		string_view const& name,
		Runnable<functor_t<T>, VEngine_AllocType::Default> func)
		: name(name) {
		LoadFunction_T<T>(name, std::move(func));
	}
	~FunctionLoader() {
		RemoveFunc(name);
	}
};
template<typename T>
Runnable<functor_t<T>, VEngine_AllocType::Default> const& TryGetFunction(
	string_view const& name) {
	auto pair = GetFuncPair(typeid(functor_t<T>), name);
#ifdef DEBUG
	if (pair == 0) {
		VEngine_Log(
			{"Try Get Function "_sv,
			 name,
			 " Failed!\n"});
		VENGINE_EXIT;
	}
#endif
	return *reinterpret_cast<Runnable<functor_t<T>, VEngine_AllocType::Default> const*>(pair);
}
}// namespace vstd
#define VENGINE_LINK_FUNC(FUNC) static vstd::FunctionLoader<decltype(FUNC)> VE_##FUNC(#FUNC##_sv, FUNC)
#define VENGINE_LINK_CLASS(FUNC, ...) static vstd::FunctionLoader<decltype(FUNC)> VE_##FUNC(#FUNC##_sv, FUNC(__VA_ARGS__))