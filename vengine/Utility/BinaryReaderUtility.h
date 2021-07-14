#pragma once
#include <Common/Common.h>
#include <CJsonObject/CJsonObject.hpp>
#include <Common/string_view.h>

class VENGINE_DLL_COMMON BinaryReaderUtility
{
public:
	struct FileDescriptor
	{
		uint fileIndex;
		uint64 offset;
		uint64 length;
	};
	static void Initialize();
	/*
	{
		"content path" : "filePath"
	}
	*/
	static bool GetFileDescriptor(vstd::string_view path, FileDescriptor* desc);
	static FILE* GetFile(uint fileIndex);
	static std::mutex* GetMutex();
	static void Dispose();

	BinaryReaderUtility() = delete;
	KILL_COPY_CONSTRUCT(BinaryReaderUtility)
};