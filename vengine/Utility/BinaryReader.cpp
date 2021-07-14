#pragma vengine_package vengine_dll
#include <Utility/BinaryReader.h>
#include <Utility/BinaryReaderUtility.h>
BinaryReader::BinaryReader(vstd::string const& path) {
	currentPos = 0;
	//fopen( "rb"_sv)
	BinaryReaderUtility::FileDescriptor desc;
	if (BinaryReaderUtility::GetFileDescriptor(path, &desc))//TODO: Use package system here
	{
		isInPackage = true;
		length = desc.length;
		packageData.readMtx = BinaryReaderUtility::GetMutex();
		packageData.globalIfs = BinaryReaderUtility::GetFile(desc.fileIndex);
		packageData.offset = desc.offset;
	} else {
		isInPackage = false;
		ifs = fopen(path.c_str(), "rb"_sv);
		isAvaliable = ifs;
		if (isAvaliable) {
			fseek(ifs, 0, SEEK_END);
			length = ftell(ifs);
			fseek(ifs, 0, SEEK_SET);
		} else {
			length = 0;
		}
	}
}
void BinaryReader::Read(char* ptr, uint64 len) {
	if (!isAvaliable) return;
	uint64 targetEnd = currentPos + len;
	if (targetEnd > length) {
		targetEnd = length;
		len = targetEnd - currentPos;
	}
	uint64 lastPos = currentPos;
	currentPos = targetEnd;
	if (len == 0) return;
	if (isInPackage) {
		lockGuard lck(*packageData.readMtx);
		fseek(packageData.globalIfs, lastPos + packageData.offset, SEEK_SET);
		fread(ptr, len, 1, packageData.globalIfs);
	} else {
		fseek(ifs, lastPos, SEEK_SET);
		fread(ptr, len, 1, ifs);
	}
}
vstd::vector<uint8_t> BinaryReader::Read() {
	if (!isAvaliable) return vstd::vector<uint8_t>();
	auto len = length;
	auto result = vstd::vector<uint8_t>(len);
	uint64 targetEnd = currentPos + len;
	if (targetEnd > length) {
		targetEnd = length;
		len = targetEnd - currentPos;
	}
	uint64 lastPos = currentPos;
	currentPos = targetEnd;
	if (len == 0) return vstd::vector<uint8_t>();
	if (isInPackage) {
		lockGuard lck(*packageData.readMtx);
		fseek(packageData.globalIfs, lastPos + packageData.offset, SEEK_SET);
		fread(result.data(), len, 1, packageData.globalIfs);
	} else {
		fseek(ifs, lastPos, SEEK_SET);
		fread(result.data(), len, 1, ifs);
	}
	return result;
}

void BinaryReader::SetPos(uint64 pos) {
	if (!isAvaliable) return;
	if (pos > length) pos = length;
	currentPos = pos;
}
BinaryReader::~BinaryReader() {
	if (!isAvaliable) return;
	if (!isInPackage) {
		fclose(ifs);
	}
}
