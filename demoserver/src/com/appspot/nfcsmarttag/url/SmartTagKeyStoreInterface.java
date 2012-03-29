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
 * Abstract interface to a key store. Implement your own.
 */

package com.appspot.nfcsmarttag.url; 

import java.util.List;


/**
 * A store of the NFC smart tag keys.
 */
public interface SmartTagKeyStoreInterface {
  /*
   * Number of bytes of a tag key.
   */
  public static final int KEY_BYTES = 16;
  
  /**
   * Get all potential keys (the tag has only one) for the tag id.
   * 
   * Providing several keys to a tag is useful when you change a tag key.
   * You can use both versions of keys in the transition period.
   * 
   * @param tagId to be used for looking up the tag key.
   * @return a list of tag keys.  Otherwise, empty list.
   * @throws InvalidInputException if the given tagId is out of expected format.
   */
  List<byte[]> getKeys(byte[] tagId) throws InvalidInputException;
}
