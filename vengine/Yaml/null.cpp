#pragma vengine_package yaml_cpp
#include <Yaml/yaml_config.h>
#include <Yaml/yaml-cpp/null.h>

namespace YAML {
_Null Null;

bool IsNullString(const std::string& str) {
  return str.empty() || str == "~" || str == "null" || str == "Null" ||
         str == "NULL";
}
}  // namespace YAML
