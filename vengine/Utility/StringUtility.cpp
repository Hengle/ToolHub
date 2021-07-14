#pragma vengine_package vengine_dll
#include <Utility/StringUtility.h>
void StringUtil::IndicesOf(vstd::string const& str, vstd::string const& sign, vstd::vector<uint>& v) {
	v.clear();
	if (str.empty()) return;
	int32_t count = str.length() - sign.length() + 1;
	v.reserve(10);
	for (int32_t i = 0; i < count; ++i) {
		bool success = true;
		for (int32_t j = 0; j < sign.length(); ++j) {
			if (sign[j] != str[i + j]) {
				success = false;
				break;
			}
		}
		if (success)
			v.push_back(i);
	}
}
void StringUtil::IndicesOf(vstd::string const& str, char sign, vstd::vector<uint>& v) {
	v.clear();
	int32_t count = str.length();
	v.reserve(10);
	for (int32_t i = 0; i < count; ++i) {
		if (sign == str[i]) {
			v.push_back(i);
		}
	}
}
void StringUtil::CutToLine(const char* str, int64_t size, vstd::vector<vstd::string>& lines) {
	lines.clear();
	lines.reserve(32);
	vstd::string buffer;
	buffer.reserve(32);
	for (size_t i = 0; i < size; ++i) {
		auto&& value = str[i];
		if (value == '\0') break;
		if (!(value == '\n' || value == '\r')) {
			buffer.push_back(value);
		} else {
			if (value == '\n' || (value == '\r' && i < size - 1 && str[i + 1] == '\n')) {
				if (!buffer.empty())
					lines.push_back(buffer);
				buffer.clear();
			}
		}
	}
	if (!buffer.empty())
		lines.push_back(buffer);
}
void StringUtil::CutToLine(vstd::string const& str, vstd::vector<vstd::string>& lines) {
	lines.clear();
	lines.reserve(32);
	vstd::string buffer;
	buffer.reserve(32);
	for (size_t i = 0; i < str.length(); ++i) {
		if (!(str[i] == '\n' || str[i] == '\r')) {
			buffer.push_back(str[i]);
		} else {
			if (str[i] == '\n' || (str[i] == '\r' && i < str.length() - 1 && str[i + 1] == '\n')) {
				if (!buffer.empty())
					lines.push_back(buffer);
				buffer.clear();
			}
		}
	}
	if (!buffer.empty())
		lines.push_back(buffer);
}
void StringUtil::ReadLines(std::ifstream& ifs, vstd::vector<vstd::string>& lines) {
	ifs.seekg(0, std::ios::end);
	int64_t size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	vstd::vector<char> buffer(size);
	memset(buffer.data(), 0, size);
	ifs.read(buffer.data(), size);
	CutToLine(buffer.data(), size, lines);
}
int32_t StringUtil::GetFirstIndexOf(vstd::string const& str, char sign) {
	int32_t count = str.length();
	for (int32_t i = 0; i < count; ++i) {
		if (sign == str[i]) {
			return i;
		}
	}
	return -1;
}
int32_t StringUtil::GetFirstIndexOf(vstd::string const& str, vstd::string const& sign) {
	int32_t count = str.length() - sign.length() + 1;
	for (int32_t i = 0; i < count; ++i) {
		bool success = true;
		for (int32_t j = 0; j < sign.length(); ++j) {
			if (sign[j] != str[i + j]) {
				success = false;
				break;
			}
		}
		if (success)
			return i;
	}
	return -1;
}
void StringUtil::Split(vstd::string const& str, char sign, vstd::vector<vstd::string>& v) {
	vstd::vector<uint> indices;
	IndicesOf(str, sign, indices);
	v.clear();
	v.reserve(10);
	vstd::string s;
	s.reserve(str.size());
	int32_t startPos = 0;
	for (auto index = indices.begin(); index != indices.end(); ++index) {
		s.clear();
		for (int32_t i = startPos; i < *index; ++i) {
			s.push_back(str[i]);
		}
		startPos = *index + 1;
		if (!s.empty())
			v.push_back(s);
	}
	s.clear();
	for (int32_t i = startPos; i < str.length(); ++i) {
		s.push_back(str[i]);
	}
	if (!s.empty())
		v.push_back(s);
}
void StringUtil::Split(vstd::string const& str, vstd::string const& sign, vstd::vector<vstd::string>& v) {
	vstd::vector<uint> indices;
	IndicesOf(str, sign, indices);
	v.clear();
	v.reserve(10);
	vstd::string s;
	s.reserve(str.size());
	int32_t startPos = 0;
	for (auto index = indices.begin(); index != indices.end(); ++index) {
		s.clear();
		for (int32_t i = startPos; i < *index; ++i) {
			s.push_back(str[i]);
		}
		startPos = *index + sign.size();
		if (!s.empty())
			v.push_back(s);
	}
	s.clear();
	for (int32_t i = startPos; i < str.length(); ++i) {
		s.push_back(str[i]);
	}
	if (!s.empty())
		v.push_back(s);
}
void StringUtil::GetDataFromAttribute(vstd::string const& str, vstd::string& result) {
	int32_t firstIndex = GetFirstIndexOf(str, '[');
	result.clear();
	if (firstIndex < 0) return;
	result.reserve(5);
	for (int32_t i = firstIndex + 1; str[i] != ']' && i < str.length(); ++i) {
		result.push_back(str[i]);
	}
}
void StringUtil::GetDataFromBrackets(vstd::string const& str, vstd::string& result) {
	int32_t firstIndex = GetFirstIndexOf(str, '<');
	result.clear();
	if (firstIndex < 0) return;
	result.reserve(5);
	for (int32_t i = firstIndex + 1; str[i] != '>' && i < str.length(); ++i) {
		result.push_back(str[i]);
	}
}
int32_t StringUtil::StringToInteger(vstd::string const& str) {
	if (str.empty()) return 0;
	uint i;
	int32_t value = 0;
	int32_t rate;
	if (str[0] == '-') {
		rate = -1;
		i = 1;
	} else {
		rate = 1;
		i = 0;
	}
	for (; i < str.length(); ++i) {
		value *= 10;
		value += (int32_t)str[i] - 48;
	}
	return value * rate;
}
double StringUtil::StringToFloat(vstd::string const& str) {
	if (str.empty()) return 0;
	uint i;
	double value = 0;
	int32_t rate;
	if (str[0] == '-') {
		rate = -1;
		i = 1;
	} else {
		rate = 1;
		i = 0;
	}
	for (; i < str.length(); ++i) {
		auto c = str[i];
		if (c == '.') {
			i++;
			break;
		}
		value *= 10;
		value += (int32_t)c - 48;
	}
	double afterPointRate = 1;
	for (; i < str.length(); ++i) {
		afterPointRate *= 0.1;
		value += afterPointRate * ((int32_t)str[i] - 48);
	}
	return value * rate;
}
int32_t StringUtil::StringToInteger(vstd::string_view str) {
	if (str.size() == 0) return 0;
	uint i;
	int32_t value = 0;
	int32_t rate;
	if (str.begin()[0] == '-') {
		rate = -1;
		i = 1;
	} else {
		rate = 1;
		i = 0;
	}
	for (; i < str.size(); ++i) {
		value *= 10;
		value += (int32_t)str.begin()[i] - 48;
	}
	return value * rate;
}
double StringUtil::StringToFloat(vstd::string_view str) {
	if (str.size() == 0) return 0;
	uint i;
	double value = 0;
	int32_t rate;
	if (str.begin()[0] == '-') {
		rate = -1;
		i = 1;
	} else {
		rate = 1;
		i = 0;
	}
	for (; i < str.size(); ++i) {
		auto c = str.begin()[i];
		if (c == '.') {
			i++;
			break;
		}
		value *= 10;
		value += (int32_t)c - 48;
	}
	double afterPointRate = 1;
	for (; i < str.size(); ++i) {
		afterPointRate *= 0.1;
		value += afterPointRate * ((int32_t)str.begin()[i] - 48);
	}
	return value * rate;
}
inline void mtolower(char& c) {
	if ((c >= 'A') && (c <= 'Z'))
		c = c + ('a' - 'A');
}
inline void mtoupper(char& c) {
	if ((c >= 'a') && (c <= 'z'))
		c = c + ('A' - 'a');
}

inline char mtolower_value(char c) {
	if ((c >= 'A') && (c <= 'Z'))
		return c + ('a' - 'A');
	return c;
}
inline char mtoupper_value(char c) {
	if ((c >= 'a') && (c <= 'z'))
		return c + ('A' - 'a');
	return c;
}
void StringUtil::ToLower(vstd::string& str) {
	char* c = str.data();
	const uint size = str.length();
	for (uint i = 0; i < size; ++i) {
		mtolower(c[i]);
	}
}
void StringUtil::ToUpper(vstd::string& str) {
	char* c = str.data();
	const uint size = str.length();
	for (uint i = 0; i < size; ++i) {
		mtoupper(c[i]);
	}
}

vstd::string StringUtil::ToLower(vstd::string_view str) {
	vstd::string s;
	s.resize(str.size());
	for (auto i : vstd::range(str.size())) {
		auto&& v = s[i];
		v = str[i];
		mtolower(v);
	}
	return s;
}
vstd::string StringUtil::ToUpper(vstd::string_view str) {
	vstd::string s;
	s.resize(str.size());
	for (auto i : vstd::range(str.size())) {
		auto&& v = s[i];
		v = str[i];
		mtoupper(v);
	}
	return s;
}

bool StringUtil::EqualIgnoreCapital(vstd::string_view a, vstd::string_view b) {
	if (a.size() != b.size()) return false;
	for (auto i : vstd::range(a.size())) {
		if (mtoupper_value(a[i]) != mtoupper_value(b[i])) return false;
	}
	return true;
}

vstd::string_view StringUtil::GetExtension(vstd::string const& path) {
	char const* ptr = path.c_str() + path.length() - 1;
	for (; ptr != path.c_str(); --ptr) {
		if (*ptr == '.') {
			return vstd::string_view(ptr + 1, path.c_str() + path.length());
		}
	}
	return vstd::string_view(path);
}
