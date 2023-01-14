// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GOOGLE_PROTOBUF_COMPILER_JAVA_CONTEXT_H__
#define GOOGLE_PROTOBUF_COMPILER_JAVA_CONTEXT_H__

#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "google/protobuf/compiler/java/helpers.h"
#include "google/protobuf/compiler/java/options.h"
#include "google/protobuf/port.h"

#ifdef JUPB
#include "upb/reflection/def.hpp"
#endif

namespace google {
namespace protobuf {
class FileDescriptor;
class FieldDescriptor;
class OneofDescriptor;
class Descriptor;
class EnumDescriptor;
namespace compiler {
namespace java {
class ClassNameResolver;  // name_resolver.h
}
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

struct FieldGeneratorInfo;
struct OneofGeneratorInfo;
// A context object holds the information that is shared among all code
// generators.
class Context {
 public:
#ifndef JUPB
  Context(const FileDescriptor* file, const Options& options);
#else
  Context(const FileDescriptor* file, const upb::DefPool& upbPool32, const upb::DefPool& upbPool64, const Options& options);
#endif
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
  ~Context();

  // Get the name resolver associated with this context. The resolver
  // can be used to map descriptors to Java class names.
  ClassNameResolver* GetNameResolver() const;

  // Get the FieldGeneratorInfo for a given field.
  const FieldGeneratorInfo* GetFieldGeneratorInfo(
      const FieldDescriptor* field) const;

  // Get the OneofGeneratorInfo for a given oneof.
  const OneofGeneratorInfo* GetOneofGeneratorInfo(
      const OneofDescriptor* oneof) const;

  const Options& options() const { return options_; }

  // Enforces all the files (including transitive dependencies) to use
  // LiteRuntime.

  bool EnforceLite() const { return options_.enforce_lite; }

  // Does this message class have generated parsing, serialization, and other
  // standard methods for which reflection-based fallback implementations exist?
  bool HasGeneratedMethods(const Descriptor* descriptor) const;

#ifdef JUPB
  upb::MessageDefPtr GetUpbMessage32(const Descriptor& m) const {
    return upbPool32_.FindMessageByName(m.full_name().c_str());
  }

  upb::MessageDefPtr GetUpbMessage64(const Descriptor& m) const {
    return upbPool64_.FindMessageByName(m.full_name().c_str());
  }

  upb::FieldDefPtr GetUpbField32(const FieldDescriptor& f) const {
    return GetUpbFieldFromPool(&upbPool32_, f);
  }

  upb::FieldDefPtr GetUpbField64(const FieldDescriptor& f) const {
    return GetUpbFieldFromPool(&upbPool64_, f);
  }
  // absl::flat_hash_map<const FieldDescriptor*, const upb::FieldDefPtr> cppFieldToUpb_;
  // absl::flat_hash_map<const Descriptor*, const upb::MessageDefPtr> cppMessageToUpb_;
#endif

 private:
  void InitializeFieldGeneratorInfo(const FileDescriptor* file);
  void InitializeFieldGeneratorInfoForMessage(const Descriptor* message);
  void InitializeFieldGeneratorInfoForFields(
      const std::vector<const FieldDescriptor*>& fields);

#ifdef JUPB
  static upb::FieldDefPtr GetUpbFieldFromPool(const upb::DefPool* pool,
                                                    const FieldDescriptor& f) {
    if (f.is_extension()) {
      return pool->FindExtensionByName(f.full_name().c_str());
    } else {
      return pool->FindMessageByName(f.containing_type()->full_name().c_str())
          .FindFieldByNumber(f.number());
    }
  }
#endif

  std::unique_ptr<ClassNameResolver> name_resolver_;
  absl::flat_hash_map<const FieldDescriptor*, FieldGeneratorInfo>
      field_generator_info_map_;
  absl::flat_hash_map<const OneofDescriptor*, OneofGeneratorInfo>
      oneof_generator_info_map_;
  Options options_;
#ifdef JUPB
  const upb::DefPool& upbPool32_;
  const upb::DefPool& upbPool64_;
#endif
};

template <typename Descriptor>
void MaybePrintGeneratedAnnotation(Context* context, io::Printer* printer,
                                   Descriptor* descriptor, bool immutable,
                                   const std::string& suffix = "") {
  if (IsOwnFile(descriptor, immutable)) {
    PrintGeneratedAnnotation(printer, '$',
                             context->options().annotate_code
                                 ? AnnotationFileName(descriptor, suffix)
                                 : "",
                             context->options());
  }
}


}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_JAVA_CONTEXT_H__
