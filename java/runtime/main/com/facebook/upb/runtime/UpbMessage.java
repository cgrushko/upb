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

import java.io.IOException;

public abstract class UpbMessage {
  protected final Minitable minitable;
  protected final long pointer;
  protected final Arena arena;

  protected UpbMessage(Minitable minitable, long pointer, Arena arena) {
    this.minitable = minitable;
    this.pointer = pointer;
    this.arena = arena;
  }

  protected UpbMessage(Arena arena, Minitable minitable) {
    this(minitable, Messages.createMessage(minitable, arena), arena);
  }

  protected abstract <MessageType extends UpbMessage> MessageType cloneMessage(Arena arena);

  public byte[] toByteArray() {
    try {
      return Messages.encodeMessage(this);
    } catch (ProtocolBufferEncodingException e) {
      throw new RuntimeException(e);
    }
  }

  public static class Builder<MessageType extends UpbMessage, BuilderType extends Builder<MessageType, BuilderType>> {
    protected MessageType instance;
    private boolean isDirty;

    protected Builder(MessageType instance) {
      this.instance = instance;
    }

    protected void copyOnWrite() {
      if (isDirty) {
        return;
      }
      isDirty = true;
      instance = instance.cloneMessage(instance.arena);
      // todo: implement.
    }

    public MessageType build() {
      isDirty = false;
      return instance;
    }
  }
}
