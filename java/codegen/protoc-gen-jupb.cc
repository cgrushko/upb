#include <cstdint>
#include <memory>

#include "google/protobuf/descriptor.pb.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/substitute.h"
#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/compiler/plugin.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/wire_format.h"
#include "upb/mini_table/encode_internal.hpp"
#include "upb/mini_table/enum_internal.h"
#include "upb/mini_table/extension_internal.h"
#include "upbc/common.h"
#include "upbc/file_layout.h"
#include "upbc/names.h"
#include "google/protobuf/compiler/java/generator.h"

int main(int argc, char** argv) {
  std::unique_ptr<google::protobuf::compiler::CodeGenerator> generator(
      new google::protobuf::compiler::java::JavaGenerator());
  return google::protobuf::compiler::PluginMain(argc, argv, generator.get());
}
