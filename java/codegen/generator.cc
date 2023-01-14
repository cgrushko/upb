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

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "google/protobuf/compiler/java/generator.h"


#include <memory>

#include "absl/strings/str_format.h"
#include "google/protobuf/compiler/java/file.h"
#include "google/protobuf/compiler/java/helpers.h"
#include "google/protobuf/compiler/java/name_resolver.h"
#include "google/protobuf/compiler/java/options.h"
#include "google/protobuf/compiler/java/shared_code_generator.h"
#include "google/protobuf/descriptor.pb.h"

#ifdef JUPB
#include "upb/upb.hpp"
#include "upb/reflection/def.hpp"
#endif

namespace google {
namespace protobuf {
namespace compiler {
namespace java {


JavaGenerator::JavaGenerator() {}
JavaGenerator::~JavaGenerator() {}

uint64_t JavaGenerator::GetSupportedFeatures() const {
  return CodeGenerator::Feature::FEATURE_PROTO3_OPTIONAL;
}

                             

bool JavaGenerator::Generate(const FileDescriptor* file,
                             const std::string& parameter,
                             GeneratorContext* context,
                             std::string* error) const {
#ifdef JUPB
  return false;
}

bool JavaGenerator::UpbGenerate(const upb::DefPool& upbPool32, const upb::DefPool& upbPool64,
                                const FileDescriptor* file,
                                const std::string& parameter,
                                GeneratorContext* context,
                                std::string* error) const {
#endif
  // -----------------------------------------------------------------
  // parse generator options

  std::vector<std::pair<std::string, std::string> > options;
  ParseGeneratorParameter(parameter, &options);
  Options file_options;

  file_options.opensource_runtime = opensource_runtime_;

  for (int i = 0; i < options.size(); i++) {
    if (options[i].first == "output_list_file") {
      file_options.output_list_file = options[i].second;
    } else if (options[i].first == "immutable") {
      file_options.generate_immutable_code = true;
    } else if (options[i].first == "mutable") {
      file_options.generate_mutable_code = true;
    } else if (options[i].first == "shared") {
      file_options.generate_shared_code = true;
    } else if (options[i].first == "lite") {
      // Note: Java Lite does not guarantee API/ABI stability. We may choose to
      // break existing API in order to boost performance / reduce code size.
      file_options.enforce_lite = true;
    } else if (options[i].first == "annotate_code") {
      file_options.annotate_code = true;
    } else if (options[i].first == "annotation_list_file") {
      file_options.annotation_list_file = options[i].second;
    } else {
      *error = "Unknown generator option: " + options[i].first;
      return false;
    }
  }

  if (file_options.enforce_lite && file_options.generate_mutable_code) {
    *error = "lite runtime generator option cannot be used with mutable API.";
    return false;
  }

  // By default we generate immutable code and shared code for immutable API.
  if (!file_options.generate_immutable_code &&
      !file_options.generate_mutable_code &&
      !file_options.generate_shared_code) {
    file_options.generate_immutable_code = true;
    file_options.generate_shared_code = true;
  }

  // -----------------------------------------------------------------


  std::vector<std::string> all_files;
  std::vector<std::string> all_annotations;


  std::vector<FileGenerator*> file_generators;
  if (file_options.generate_immutable_code) {
    file_generators.push_back(new FileGenerator(file, file_options,
#ifndef JUPB
                                                /* immutable = */ true));
#else
                                                upbPool32, upbPool64, /* immutable = */ true));
#endif
  }
  if (file_options.generate_mutable_code) {
    file_generators.push_back(new FileGenerator(file, file_options,
#ifndef JUPB
                                                /* mutable = */ false));
#else
                                                upbPool32, upbPool64, /* mutable = */ false));
#endif
  }

  for (int i = 0; i < file_generators.size(); ++i) {
    if (!file_generators[i]->Validate(error)) {
      for (int j = 0; j < file_generators.size(); ++j) {
        delete file_generators[j];
      }
      return false;
    }
  }

  for (int i = 0; i < file_generators.size(); ++i) {
    FileGenerator* file_generator = file_generators[i];

    std::string package_dir = JavaPackageToDir(file_generator->java_package());

    std::string java_filename = package_dir;
    java_filename += file_generator->classname();
    java_filename += ".java";
    all_files.push_back(java_filename);
    std::string info_full_path = java_filename + ".pb.meta";
    if (file_options.annotate_code) {
      all_annotations.push_back(info_full_path);
    }

    // Generate main java file.
    std::unique_ptr<io::ZeroCopyOutputStream> output(
        context->Open(java_filename));
    GeneratedCodeInfo annotations;
    io::AnnotationProtoCollector<GeneratedCodeInfo> annotation_collector(
        &annotations);
    io::Printer printer(
        output.get(), '$',
        file_options.annotate_code ? &annotation_collector : NULL);

    file_generator->Generate(&printer);

    // Generate sibling files.
    file_generator->GenerateSiblings(package_dir, context, &all_files,
                                     &all_annotations);

    if (file_options.annotate_code) {
      std::unique_ptr<io::ZeroCopyOutputStream> info_output(
          context->Open(info_full_path));
      annotations.SerializeToZeroCopyStream(info_output.get());
    }
  }


  for (int i = 0; i < file_generators.size(); ++i) {
    delete file_generators[i];
  }
  file_generators.clear();

  // Generate output list if requested.
  if (!file_options.output_list_file.empty()) {
    // Generate output list.  This is just a simple text file placed in a
    // deterministic location which lists the .java files being generated.
    std::unique_ptr<io::ZeroCopyOutputStream> srclist_raw_output(
        context->Open(file_options.output_list_file));
    io::Printer srclist_printer(srclist_raw_output.get(), '$');
    for (int i = 0; i < all_files.size(); i++) {
      srclist_printer.Print("$filename$\n", "filename", all_files[i]);
    }
  }

  if (!file_options.annotation_list_file.empty()) {
    // Generate output list.  This is just a simple text file placed in a
    // deterministic location which lists the .java files being generated.
    std::unique_ptr<io::ZeroCopyOutputStream> annotation_list_raw_output(
        context->Open(file_options.annotation_list_file));
    io::Printer annotation_list_printer(annotation_list_raw_output.get(), '$');
    for (int i = 0; i < all_annotations.size(); i++) {
      annotation_list_printer.Print("$filename$\n", "filename",
                                    all_annotations[i]);
    }
  }

  return true;
}

#ifdef JUPB
bool JavaGenerator::GenerateAll(const std::vector<const FileDescriptor*>& files,
                                const std::string& parameter,
                                GeneratorContext* generator_context,
                                std::string* error) const {

  // Construct a DefPool from |files|. We get |files| as FileDescripto, which we serialize to c++ FileDescriptorProto,
  // then deserialize into upb FileDescriptorProto, and eventually we pass it to DefPool.AddFile.
  upb::Arena arena;
  upb::DefPool pool32;
  pool32._SetPlatform(kUpb_MiniTablePlatform_32Bit);
  upb::DefPool pool64;
  pool64._SetPlatform(kUpb_MiniTablePlatform_64Bit);

  upb::Status status;
  std::vector<upb::FileDefPtr> upbFiles;
  for (auto cpp_file_descriptor : files) {
    FileDescriptorProto cpp_file_descriptor_proto;
    cpp_file_descriptor->CopyTo(&cpp_file_descriptor_proto);
    cpp_file_descriptor->CopySourceCodeInfoTo(&cpp_file_descriptor_proto);
    std::string data = cpp_file_descriptor_proto.SerializeAsString();
    
    const google_protobuf_FileDescriptorProto* upb_file_descriptor = google_protobuf_FileDescriptorProto_parse(data.c_str(), data.size(), arena.ptr());

    pool32.AddFile(upb_file_descriptor, &status);
    if (!status.ok()) {
      *error = status.error_message();
      return false;
    }
    pool64.AddFile(upb_file_descriptor, &status);
    if (!status.ok()) {
      *error = status.error_message();
      return false;
    }
  }

  return InheritedGenerateAll(pool32, pool64, files, parameter, generator_context, error);
}

bool JavaGenerator::InheritedGenerateAll(const upb::DefPool& upbPool32, const upb::DefPool& upbPool64, const std::vector<const FileDescriptor*>& files,
                                const std::string& parameter,
                                GeneratorContext* generator_context,
                                std::string* error) const {
  // Default implementation is just to call the per file method, and prefix any
  // error string with the file to provide context.
  bool succeeded = true;
  for (int i = 0; i < files.size(); i++) {
    const FileDescriptor* file = files[i];
    succeeded = UpbGenerate(upbPool32, upbPool64, file, parameter, generator_context, error);
    if (!succeeded && error && error->empty()) {
      *error =
          "Code generator returned false but provided no error "
          "description.";
    }
    if (error && !error->empty()) {
      *error = file->name() + ": " + *error;
      break;
    }
    if (!succeeded) {
      break;
    }
  }
  return succeeded;
}

#endif

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
