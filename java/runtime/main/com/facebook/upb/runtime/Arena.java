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

import java.lang.ref.PhantomReference;
import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;
import java.util.HashSet;

public class Arena {
  static {
    // Class-load Messages so JNI can find it.
    Messages.forceLoadClass();

    System.loadLibrary("javaupbruntime");
  }

  private static final ReferenceQueue<Arena> referenceQueue = new ReferenceQueue<>();
  private static final HashSet<PhantomReference> refs = new HashSet<>();

  static {
    new Thread() {
      @Override
      public void run() {
        while (true) {
          try {
            Reference<?> ref = referenceQueue.remove();
            freeUpbArena(((ArenaPhantomReference) ref).pointerToFree);
            refs.remove(ref);
            ref.clear();
          } catch (InterruptedException e) {
            // Continue running; this thread should never be interrupted.
          }
        }
      }
    }.start();
  }

  public final long pointer;

  public Arena() {
    pointer = initNative();
    refs.add(new ArenaPhantomReference(this, referenceQueue, pointer));
  }

  private native long initNative();

  private static native void freeUpbArena(long pointer);

  private static class ArenaPhantomReference extends PhantomReference<Arena> {
    public final long pointerToFree;

    public ArenaPhantomReference(
        Arena referent, ReferenceQueue<? super Arena> q, long pointerToFree) {
      super(referent, q);
      this.pointerToFree = pointerToFree;
    }
  }
}
