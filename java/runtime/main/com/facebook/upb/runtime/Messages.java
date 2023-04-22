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

package com.facebook.upb.runtime;

import java.lang.reflect.Field;
import java.nio.charset.StandardCharsets;
// import javax.annotation.Nullable;
import sun.misc.Unsafe;

public class Messages {
  private static final Unsafe UNSAFE;
  private static final boolean is64bit;

  static void forceLoadClass() {
    // No-op.
  }

  static {
    System.loadLibrary("javaupbruntime");

    try {
      Field f = Unsafe.class.getDeclaredField("theUnsafe");
      f.setAccessible(true);
      UNSAFE = (Unsafe) f.get(null);
    } catch (Exception e) {
      throw new Error(e);
    }

    is64bit = getIs64();
  }

  public static long createMessage(Minitable minitable, Arena arena) {
    return upb_Message_New(arena.pointer, minitable.pointer);
  }

  public static long cloneMessage(UpbMessage message, Arena arena) {
    return upb_Message_DeepClone(message.pointer, message.minitable.pointer, arena.pointer);
  }

  public static void decodeMessage(
      byte[] buf,
      UpbMessage message) throws ProtocolBufferDecodingException {
    upb_Decode(buf, message.pointer, message.minitable.pointer, message.arena.pointer);
  }

  public static byte[] encodeMessage(UpbMessage message) throws ProtocolBufferEncodingException {
    return upb_Encode(message.pointer, message.minitable.pointer, message.arena.pointer);
  }

  private static native long upb_Message_New(long arenaPtr, long minitablePtr);

  private static native long upb_Message_DeepClone(long messagePtr, long minitablePtr, long arenaPtr);

  // public static native long _upb_Message_New(
  // long minitablesPointer, int fileIndex, int msgIndex, long arenaPointer);

  public static boolean UPB_PTR_AT_boolean(long pointer) {
    return UNSAFE.getByte(pointer) != 0;
  }

  public static float UPB_PTR_AT_float(long pointer) {
    return UNSAFE.getFloat(pointer);
  }

  public static int getInt32(long msgPointer, long offset) {
    return UNSAFE.getInt(msgPointer + offset);
  }

  public static int getInt32(long msgPointer, long offset, int hasBitIndex, int defaultValue) {
    return _upb_hasbit(msgPointer, hasBitIndex) ? UNSAFE.getInt(msgPointer + offset) : defaultValue;
  }

  public static double UPB_PTR_AT_double(long pointer) {
    return UNSAFE.getDouble(pointer);
  }

  public static long UPB_PTR_AT_long(long pointer) {
    return UNSAFE.getLong(pointer);
  }

  public static void UPB_PTR_AT_boolean(long pointer, boolean value) {
    UNSAFE.putByte(pointer, value ? (byte) 1 : (byte) 0);
  }

  public static void UPB_PTR_AT_float(long pointer, float value) {
    UNSAFE.putFloat(pointer, value);
  }

  public static void setInt32(long msgPointer, long offset, int value) {
    UNSAFE.putInt(msgPointer + offset, value);
  }

  public static void setInt32(long msgPointer, long offset, int hasBitIndex, int value) {
    _upb_sethas(msgPointer, hasBitIndex);
    UNSAFE.putInt(msgPointer + offset, value);
  }

  public static void UPB_PTR_AT_double(long pointer, double value) {
    UNSAFE.putDouble(pointer, value);
  }

  public static void UPB_PTR_AT_long(long pointer, long value) {
    UNSAFE.putLong(pointer, value);
  }

  public static String UPB_PTR_AT_String(long pointer) {
    String result = UPB_PTR_AT_String_internal(pointer);
    return result == null ? "" : result;
  }

  // @Nullable
  public static native String UPB_PTR_AT_String_internal(long pointer);

  public static void UPB_PTR_AT_String(long arenaPointer, long pointer, String value) {
    UPB_PTR_AT_String_internal(arenaPointer, pointer, value.getBytes(StandardCharsets.UTF_8));
  }

  public static native void UPB_PTR_AT_String_internal(
      long arenaPointer, long pointer, byte[] value);

  public static int UPB_SIZE(int size32, int size64) {
    return is64bit ? size64 : size32;
  }

  public static String UPB_STR(String str32, String str64) {
    return is64bit ? str64 : str32;
  }

  public static native void _upb_sethas(long messagePointer, int index);

  public static native boolean _upb_hasbit(long messagePointer, int index);

  public static native void _upb_clearhas(long messagePointer, int index);

  /** Don't call directly - use is64bit directly. */
  private static native boolean getIs64();

  private static native void upb_Decode(
      byte[] buf,
      long msgPointer,
      long minitablePointer,
      long arenaPointer) throws ProtocolBufferDecodingException;

  private static native byte[] upb_Encode(
      long msgPointer, long minitablePointer, long arenaPointer) throws ProtocolBufferEncodingException;
}
