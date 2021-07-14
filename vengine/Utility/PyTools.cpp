#pragma vengine_package //vengine_dll
//#define PY_TOOLS
#ifdef PY_TOOLS
#include <Common/Common.h>
#include <JobSystem/ThreadPool.h>
#include <Utility/FileUtility.h>
#include <Common/Tuple.h>
#include <Utility/StringUtility.h>
#include <Utility/QuickSort.h>
namespace vepy {
vstd::optional<ThreadPool> tPool;

vstd::vector<vstd::string> paths;
vstd::optional<HashMap<vstd::string, bool>> ignoreMap;
vstd::optional<HashMap<vstd::string, bool>> avaliableExt;
vstd::optional<HashMap<vstd::string, vstd::vector<vstd::string>>> packPathes;
VENGINE_UNITY_EXTERN void Py_InitFileSys(char const* mallocPath) {
	if (mallocPath == nullptr || strlen(mallocPath) == 0)
		vengine_init_malloc_custom(malloc, free);
	else
		vengine_init_malloc_path(mallocPath);
	tPool.New(std::thread::hardware_concurrency());
	ignoreMap.New();
	avaliableExt.New();
	packPathes.New();
	printf("DLL Load Success!\n");
}

VENGINE_UNITY_EXTERN void Py_AddIgnorePath(
	char const* path) {
	ignoreMap->Emplace(path, true);
}

VENGINE_UNITY_EXTERN void Py_AddExtension(
	char const* ext) {
	avaliableExt->Emplace(ext, true);
}

void ExecuteFileSys() {
	paths.clear();
	auto GetPath = [](auto&& str) {
		if (str.size() < 2) return vstd::string();
		return vstd::string(str.begin() + 2, str.end());
	};
	FileUtility::GetFiles(
		".",
		[&](vstd::string&& str) { paths.emplace_back(GetPath(str)); },
		[&](vstd::string const& str, bool isFile) {
			auto newStr = GetPath(str);
			if (isFile) {
				for (auto i = newStr.end(); i != newStr.begin(); i--) {
					if (*(i - 1) == '.') {
						return static_cast<bool>(avaliableExt->Find(vstd::string(i, newStr.end())));
					}
				}
				return false;
			} else {
				if (ignoreMap->Find(newStr)) {
					return false;
				}
			}
			return true;
		});
	std::mutex mtx;
	auto getPathJob = tPool->GetParallelTask(
		[&](size_t i) {
			vstd::vector<char, VEngine_AllocType::Stack> vec(513);
			vec[512] = 0;
			std::ifstream ifs(paths[i].c_str());
			ifs.getline(vec.data(), 512);
			ifs.close();
			size_t lenStr = strlen(vec.data());
			auto IsEmpty = [](char c) {
				switch (c) {
					case '\t':
					case '\n':
					case '\r':
					case ' ':
						return true;
					default:
						return false;
				}
			};
			auto GetCharacter = [&](vstd::string_view filePath) {
				//Cull Empty
				auto Default = []() {
					return vstd::optional<
						Tuple<vstd::string_view,
							  vstd::string_view>>();
				};
				if (filePath.size() == 0) {
					return Default();
				}
				auto GetNextWord = [&](char const* cur, char const* end, bool isEmpty) {
					for (; cur != end; ++cur) {
						if (IsEmpty(*cur) == isEmpty)
							break;
					}
					return cur;
				};

				auto firstNonEmpty = GetNextWord(filePath.begin(), filePath.end(), false);
				auto firstEmpty = GetNextWord(firstNonEmpty, filePath.end(), true);
				if (firstNonEmpty == firstEmpty)
					return Default();

				return vstd::optional<
						   Tuple<vstd::string_view,
								 vstd::string_view>>()
					.New(
						vstd::string_view(firstNonEmpty, firstEmpty),
						vstd::string_view(firstEmpty, filePath.end()));
			};
			auto v = GetCharacter(vstd::string_view(vec.data(), lenStr));
			if (!v || v->Get<0>() != "#pragma"_sv) {
				return;
			}
			v = GetCharacter(v->Get<1>());
			if (!v || !StringUtil::EqualIgnoreCapital(v->Get<0>(), "vengine_package"_sv))
				return;
			v = GetCharacter(v->Get<1>());
			if (!v) return;
			size_t ed = lenStr;
			for (auto i : vstd::range(lenStr - 1, -1, -1)) {
				if (IsEmpty(vec[i])) {
					ed = i;
				} else
					break;
			}
			auto pkgName = StringUtil::ToLower(vstd::string_view(v->Get<0>().begin(), vec.data() + ed));
			{
				std::lock_guard lck(mtx);
				auto ite = packPathes->Emplace(
					std::move(pkgName));
				ite.Value().emplace_back(std::move(paths[i]));
			}
		},
		paths.size());
	getPathJob.Complete();
	vstd::vector<vstd::vector<vstd::string> const*, VEngine_AllocType::Stack> strs;
	strs.reserve(packPathes->size());
	for (auto&& i : (*packPathes)) {
		strs.push_back(&i.second);
	}
	auto sortPathJob = tPool->GetParallelTask(
		[&](size_t i) {
			QuicksortStackCustomCompare<vstd::string>(
				strs[i]->data(),
				[](vstd::string const& a, vstd::string const& b) {
					auto sz = Min(a.size(), b.size());

					for (auto i : vstd::range(sz)) {
						if (a[i] > b[i])
							return 1;
						else if (a[i] < b[i])
							return -1;
					}
					if (a.size() > b.size()) return 1;
					else if (a.size() == b.size())
						return 0;
					return -1;
				},
				0, strs[i]->size() - 1);
		},
		strs.size());
	sortPathJob.Complete();
}
vstd::optional<ThreadTaskHandle> task;
VENGINE_UNITY_EXTERN void Py_ExecuteFileSys() {
	task = tPool->GetTask([]() { ExecuteFileSys(); });
	task->Execute();
}
vstd::vector<vstd::string>* vecPtr = nullptr;

VENGINE_UNITY_EXTERN void Py_SetPackageName(char const* name) {
	vstd::string lowName = name;
	StringUtil::ToLower(lowName);
	task->Complete();
	auto ite = packPathes->Find(lowName);
	vecPtr = (ite) ? &ite.Value() : nullptr;
}

VENGINE_UNITY_EXTERN uint Py_PathSize() {
	return vecPtr ? vecPtr->size() : 0;
}

VENGINE_UNITY_EXTERN char const* Py_GetPath(uint v) {
	return vecPtr ? (*vecPtr)[v].c_str() : nullptr;
}

VENGINE_UNITY_EXTERN void Py_DisposeFileSys() {
	tPool.Delete();
	printf("DLL Dispose Success!\n");
}
}// namespace vepy
#endif