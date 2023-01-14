# Copyright (c) 2009-2021, Google LLC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of Google LLC nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Google LLC BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("@rules_proto//proto:defs.bzl", "proto_lang_toolchain", "proto_library")

licenses(["notice"])

exports_files(
    ["workspace_deps.bzl"],
    visibility = ["//cmake:__pkg__"],
)

py_binary(
    name = "amalgamate",
    srcs = ["amalgamate.py"],
    visibility = ["//:__pkg__"],
)

# py_proto_library() is private rule, only intended for internal use by upb.
# Hopefully py_proto_library() will eventually be availble in rules_proto or
# another upstream package.
bzl_library(
    name = "py_proto_library_bzl",
    srcs = ["py_proto_library.bzl"],
)

bzl_library(
    name = "upb_proto_library_bzl",
    srcs = ["upb_proto_library.bzl"],
    visibility = ["//visibility:public"],
    deps = [
        "@bazel_skylib//lib:paths",
        "@bazel_tools//tools/cpp:toolchain_utils.bzl",
        "@rules_proto//proto:defs",
    ],
)

# Use by calling
# bazel build --check_visibility=false --linkopt=-Wl,-no_fixup_chains  //upb/test:empty_jupb_proto --proto_toolchain_for_javalite=//bazel:jupb_toolchain

proto_lang_toolchain(
    name = "jupb_toolchain",
    # keep this in sync w/ LITE_WELL_KNOWN_PROTO_MAP in //:BUILD
    blacklisted_protos = [
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:api_proto",
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:empty_proto",
        "@com_google_protobuf//:field_mask_proto",
        "@com_google_protobuf//:source_context_proto",
        "@com_google_protobuf//:struct_proto",
        "@com_google_protobuf//:timestamp_proto",
        "@com_google_protobuf//:type_proto",
        "@com_google_protobuf//:wrappers_proto",
    ],
  progress_message = "Generating jupb proto_library %{label}",
  command_line = "--PLUGIN_jupb_out=lite:$(OUT)",
  plugin_format_flag = "--plugin=protoc-gen-PLUGIN_jupb=%s",
  plugin = "//java/codegen:protoc-gen-jupb",
    runtime = "//java/runtime/main/com/facebook/upb/runtime:runtime",
    visibility = ["//visibility:public"],
)
