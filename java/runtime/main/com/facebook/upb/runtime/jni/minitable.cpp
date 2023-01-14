/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name Meta nor the 
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "minitable.h"
#include "jni_helper.h"
// #include <android/log.h>
#include "upb/upb.h"
#include <array>
#include <stdio.h>
#include <stdlib.h>
#include "upb/mini_table/decode.h"

namespace Minitable {

jlong upb_MiniTable_Build(JNIEnv* env, jobject /* thisz */, jbyteArray serializedMinitable, jlong arenaPtr) {
  void* data = env->GetPrimitiveArrayCritical(serializedMinitable, NULL);
  assertNoPendingJniException(env);
  jsize len = env->GetArrayLength(serializedMinitable);
  assertNoPendingJniException(env);

  upb_Status status;
  jlong result = (jlong) upb_MiniTable_Build((const char*)data, len, (upb_Arena*)arenaPtr, &status);
  if (!result) {
    logErrorMessageAndDie(status.msg);
  }
  env->ReleasePrimitiveArrayCritical(serializedMinitable, data, JNI_ABORT);
  assertNoPendingJniException(env);
  return result;
}

namespace {
auto constexpr kClassName = "com/facebook/upb/runtime/Minitable";

std::array<JNINativeMethod, 1> methods = {{
    // long upb_MiniTable_Build(byte[] serializedMinitable, long arenaPtr);
    {"upb_MiniTable_Build",
     "([BJ)J",
     (void*)Minitable::upb_MiniTable_Build},
}};

} // namespace

void registerNatives(JNIEnv* env) {
  jclass clazz = env->FindClass(kClassName);
  assertNoPendingJniException(env);
  env->RegisterNatives(clazz, methods.begin(), methods.size());
  assertNoPendingJniException(env);
}

} // namespace Minitable
