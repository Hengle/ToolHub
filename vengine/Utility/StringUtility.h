#pragma once
#include <Common/vector.h>
#include <fstream>
#include <Common/vstring.h>
#include <Common/Memory.h>
#include <Common/string_view.h>
typedef uint32_t uint;
class VENGINE_DLL_COMMON StringUtil
{
private:
	StringUtil() = delete;
	KILL_COPY_CONSTRUCT(StringUtil)
public:
	static void IndicesOf(vstd::string const& str, vstd::string const& sign, vstd::vector<uint>& v);
	static void IndicesOf(vstd::string const& str, char, vstd::vector<uint>& v);
	static void CutToLine(vstd::string const& str, vstd::vector<vstd::string>& lines);
	static void CutToLine(const char* str, int64_t size, vstd::vector<vstd::string>& lines);
	static void ReadLines(std::ifstream& ifs, vstd::vector<vstd::string>& lines);
	static int32_t GetFirstIndexOf(vstd::string const& str, vstd::string const& sign);
	static int32_t GetFirstIndexOf(vstd::string const& str, char sign);
	static void Split(vstd::string const& str, vstd::string const& sign, vstd::vector<vstd::string>& v);
	static void Split(vstd::string const& str, char sign, vstd::vector<vstd::string>& v);
	static void GetDataFromAttribute(vstd::string const& str, vstd::string& result);
	static void GetDataFromBrackets(vstd::string const& str, vstd::string& result);
	static int32_t StringToInteger(vstd::string const& str);
	static double StringToFloat(vstd::string const& str);
	static int32_t StringToInteger(vstd::string_view str);
	static double StringToFloat(vstd::string_view str);
	static void ToLower(vstd::string& str);
	static void ToUpper(vstd::string& str);

	static vstd::string ToLower(vstd::string_view str);
	static vstd::string ToUpper(vstd::string_view str);
	static bool EqualIgnoreCapital(vstd::string_view a, vstd::string_view b);
	static vstd::string_view GetExtension(vstd::string const& path);
	static void TransformWCharToChar(
		wchar_t const* src,
		char* dst,
		size_t sz);
};