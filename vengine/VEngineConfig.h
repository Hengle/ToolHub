#pragma once
///////////////////////// Switchers
//#define VENGINE_REL_WITH_DEBUG_INFO
//#define VENGINE_CLANG_COMPILER
#define VENGINE_WINDOWS



#ifdef VENGINE_WINDOWS
#define MSVC
#endif
#define NOMINMAX
///////////////////////// Clang
#ifdef VENGINE_CLANG_COMPILER
#define _XM_NO_INTRINSICS_
#define m128_f32 vector4_f32
#define m128_u32 vector4_u32
#include <cstdlib>
#define VENGINE_EXIT exit(1)
#else
#include <cstdlib>
#define VENGINE_EXIT throw 0
#endif
#ifndef UNICODE
#define UNICODE //Disable this in non-unicode system
#endif


#ifdef VENGINE_REL_WITH_DEBUG_INFO
#define DEBUG
#endif
#if defined(_DEBUG)
#define DEBUG
#endif

//////////////////////// Renderer Switcher
#define VENGINE_PLATFORM_DIRECTX_12 1
#pragma endregion

//////////////////////// Main Engine Switcher
#define VENGINE_USE_GPU_DRIVEN 1
#define VENGINE_LOAD_SCENE 1
#define VENGINE_USE_RAYTRACING 1

//////////////////////// DLL
#ifdef DLL_DEBUG
#define VENGINE_DLL_COMMON
#define VENGINE_DLL_RENDERER
#define VENGINE_DLL_FUNC
#define VENGINE_DLL_COMPUTE
#define VENGINE_DLL_NETWORK
#else

#ifdef COMMON_DLL_PROJECT
#define VENGINE_DLL_COMMON _declspec(dllexport)
#define VENGINE_C_FUNC_COMMON extern "C" _declspec(dllexport)
#else
#define VENGINE_DLL_COMMON _declspec(dllimport)
#define VENGINE_C_FUNC_COMMON extern "C" _declspec(dllimport)
#endif

#ifdef DX12_RENDERER_DLL_PROJECT
#define VENGINE_DLL_RENDERER _declspec(dllexport)
#else
#define VENGINE_DLL_RENDERER _declspec(dllimport)
#endif

#ifdef VENGINE_COMPUTE_PROJECT
#define VENGINE_DLL_COMPUTE _declspec(dllexport)
#else
#define VENGINE_DLL_COMPUTE _declspec(dllimport)
#endif

#ifdef TOOLKIT_DLL_PROJECT
#define VENGINE_DLL_TOOLKIT _declspec(dllexport)
#else
#define VENGINE_DLL_TOOLKIT _declspec(dllimport)
#endif

#endif//DLL_DEBUG

#define VENGINE_CDECL _cdecl
#define VENGINE_STD_CALL _stdcall
#define VENGINE_VECTOR_CALL _vectorcall
#define VENGINE_FAST_CALL _fastcall

#define VENGINE_UNITY_EXTERN extern "C" _declspec(dllexport)


/////////////////////// THREAD PAUSE
