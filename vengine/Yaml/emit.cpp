#pragma vengine_package yaml_cpp
#include <Yaml/yaml_config.h>
#include <Yaml/yaml-cpp/node/emit.h>
#include <nodeevents.h>
#include <Yaml/yaml-cpp/emitfromevents.h>
#include <Yaml/yaml-cpp/emitter.h>

namespace YAML {
Emitter& operator<<(Emitter& out, const Node& node) {
  EmitFromEvents emitFromEvents(out);
  NodeEvents events(node);
  events.Emit(emitFromEvents);
  return out;
}

std::ostream& operator<<(std::ostream& out, const Node& node) {
  Emitter emitter(out);
  emitter << node;
  return out;
}

std::string Dump(const Node& node) {
  Emitter emitter;
  emitter << node;
  return emitter.c_str();
}
}  // namespace YAML
