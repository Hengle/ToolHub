#pragma vengine_package yaml_cpp
#include <Yaml/yaml_config.h>
#include "graphbuilderadapter.h"

#include <Yaml/yaml-cpp/parser.h>  // IWYU pragma: keep

namespace YAML {
class GraphBuilderInterface;

void* BuildGraphOfNextDocument(Parser& parser,
                               GraphBuilderInterface& graphBuilder) {
  GraphBuilderAdapter eventHandler(graphBuilder);
  if (parser.HandleNextDocument(eventHandler)) {
    return eventHandler.RootNode();
  }
  return nullptr;
}
}  // namespace YAML
