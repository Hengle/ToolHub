#pragma once
#include <Common/Common.h>
#include <Common/Runnable.h>
#include <span>
class VENGINE_DLL_COMMON FileUtility {
private:
	FileUtility() = delete;
	~FileUtility() = delete;

public:
	static bool ReadCommandFile(vstd::string const& path, HashMap<vstd::string, Runnable<void(vstd::string const&)>>& rnb);
	static void GetFiles(vstd::string const& path, vstd::vector<vstd::string>& files, HashMap<vstd::string, bool> const& ignoreFolders);
	static void GetFiles(vstd::string const& path, Runnable<void(vstd::string&&)> const& func);
	static void GetFiles(
		vstd::string const& path,
		Runnable<void(vstd::string&&)> const& func,//execute func(path)
		Runnable<bool(vstd::string const&, bool)> const& ignoreFileFunc);//ignore func(path, isFile)
	static void GetFilesFixedExtense(vstd::string const& path, vstd::vector<vstd::string>& files, HashMap<vstd::string, bool> const& extense);
	static void GetFiles(vstd::string const& path, vstd::vector<vstd::string>& files, vstd::vector<vstd::string>& folders, HashMap<vstd::string, bool> const& ignoreFolders);
	static void GetFolders(vstd::vector<vstd::string>& files);
	static void GetTrivialFiles(vstd::string const& path, vstd::vector<vstd::string>& files);
	static vstd::string GetProgramPath();
	static vstd::string GetFileExtension(vstd::string const& filePath);//.xxx
};