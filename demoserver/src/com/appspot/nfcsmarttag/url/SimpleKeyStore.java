/*
 * Copyright 2012 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Naive implementation of of SmartTagKeyStore. Always returns key 00..00. 
 */

package com.appspot.nfcsmarttag.url; 

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Trivial implementation of SmartTagKeyStore. Always returns key 00..00.
 */
public class SimpleKeyStore implements SmartTagKeyStoreInterface {

  /**
   * Returns zero-filled 16 bytes key regardless of the tag id.
   * 
   * @see SmartTagKeyStoreInterface#getKeys(byte[])
   */
  @Override
  public List<byte[]> getKeys(byte[] tagId) {
    List<byte[]> list = new ArrayList<byte[]>();
    list.add(new byte[16]);
    return Collections.singletonList(new byte[KEY_BYTES]);
  }
}
