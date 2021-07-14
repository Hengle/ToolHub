#pragma vengine_package vengine_dll
#include <Utility/BinaryReaderUtility.h>
#include <CJsonObject/CJsonObject.hpp>
namespace BinaryReaderUtilityGlobal {

struct GlobalData {
	struct ifstreamPackage {
		FILE* file;
		bool containing;
		ifstreamPackage(char const* c) : containing(true) {
			file = fopen(c, "rb"_sv);
		}
		ifstreamPackage(ifstreamPackage& data) {
			file = data.file;
			containing = true;
			data.containing = false;
		}
		ifstreamPackage(ifstreamPackage&& data) : ifstreamPackage(data) {}
		~ifstreamPackage() {
			if (containing && file)
				fclose(file);
		}
	};
	struct FilePackage {
		ifstreamPackage ifs;
		uint64 fileSize;
		FilePackage(char const* c) : ifs(c) {}
		FilePackage(FilePackage& data) : ifs(data.ifs), fileSize(data.fileSize) {
		}
		FilePackage(FilePackage&& data) : FilePackage(data) {}
	};
	std::mutex globalMtx;
	void* allocatedStringMemory;
	vstd::vector<ifstreamPackage> fileHandles;
	StackObject<HashMap<vstd::string_view, BinaryReaderUtility::FileDescriptor>> allFileDescriptors;
	GlobalData(neb::CJsonObject& cjson) {
		vstd::vector<FilePackage> contentIfstreams;
		cjson.ResetTraversing();
		vstd::string contentPath;
		vstd::string filePath;
		uint64 stringSize = 0;
		uint64 fileSize = 0;
		while (cjson.GetKey(contentPath)) {
			if (cjson.Get(contentPath, filePath)) {
				FilePackage& content = contentIfstreams.emplace_back(contentPath.c_str());
				ifstreamPackage& file = fileHandles.emplace_back(filePath.c_str());
				std::pair<uint64, uint64> stringAndFileSize;
				fread(&stringAndFileSize, sizeof(stringAndFileSize), 1, content.ifs.file);
				stringSize += stringAndFileSize.first;
				fileSize += stringAndFileSize.second;
				content.fileSize = stringAndFileSize.second;
				if (file.file == nullptr) {
					VEngine_Log("Cannot find file!\n"_sv);
					VENGINE_EXIT;
				}
			}
		}
		allocatedStringMemory = vengine_default_malloc(stringSize);
		allFileDescriptors.New((uint64)(fileSize / 0.75));
		char* charPtr = (char*)allocatedStringMemory;
		for (uint content = 0; content < contentIfstreams.size(); ++content) {
			auto&& c = contentIfstreams[content];
			for (uint64 i = 0; i < c.fileSize; ++i) {
				uint pathLen = 0;
				fread(&pathLen, sizeof(pathLen), 1, c.ifs.file);
				fread(charPtr, pathLen, 1, c.ifs.file);
				std::pair<uint64, uint64> offsetAndCount;
				fread(&offsetAndCount, sizeof(offsetAndCount), 1, c.ifs.file);
				BinaryReaderUtility::FileDescriptor desc =
					{
						content,
						offsetAndCount.first,
						offsetAndCount.second};
				allFileDescriptors->ForceEmplace(vstd::string_view(charPtr, pathLen), desc);
				charPtr += pathLen;
			}
		}
	}
	~GlobalData() {
		allFileDescriptors.Delete();
		vengine_default_free(allocatedStringMemory);
	}
};
vstd::optional<GlobalData> glbData;
}// namespace BinaryReaderUtilityGlobal
void BinaryReaderUtility::Initialize() {
	using namespace BinaryReaderUtilityGlobal;
	std::unique_ptr<neb::CJsonObject> cjson(ReadJson("Data/FilePackage.json"_sv));
	if (cjson) {
		glbData.New(*cjson);
	} else {
		auto jsonObj = neb::CJsonObject();
		glbData.New(jsonObj);
	}
}
void BinaryReaderUtility::Dispose() {
	using namespace BinaryReaderUtilityGlobal;
	glbData.Delete();
}
bool BinaryReaderUtility::GetFileDescriptor(vstd::string_view path, FileDescriptor* desc) {
	using namespace BinaryReaderUtilityGlobal;
	if (!glbData) return false;
	auto ite = glbData->allFileDescriptors->Find(path);
	if (!ite) return false;
	*desc = ite.Value();
}
FILE* BinaryReaderUtility::GetFile(uint fileIndex) {
	using namespace BinaryReaderUtilityGlobal;
	if (fileIndex >= glbData->fileHandles.size())
		return nullptr;
	return glbData->fileHandles[fileIndex].file;
}
std::mutex* BinaryReaderUtility::GetMutex() {
	using namespace BinaryReaderUtilityGlobal;
	return &glbData->globalMtx;
}
