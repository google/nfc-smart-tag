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
 * Class to encode raw data into a Smart Tag URL.
 */

package com.appspot.nfcsmarttag.url; 

import java.util.Arrays;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.ShortBufferException;

import org.keyczar.util.Base64Coder;

/**
 * A class to encrypt NFC information.
 *
 * transmitted information:
 * Use tag key to AES-ECB encrypt one block with:
 * FeliCa ID (64 bit)
 * counter (32 bit)
 * all zero or other well-known content (32 bit)
 * Transmitted without encryption:
 * tag id (64 bit)
 * reserved (8 bit)
 */
public class TagUrlEncoder {
  private String encodedValue;
  private byte[] tagId;
  private byte[] idm;
  private long counter;
  private byte[] tagKey;
  private NfcSmartTag.NfcSmartTagInfo nfcSmartTagInfo;

  private boolean isEncoded;
  private boolean isError;
  
  private static final byte RESERVED = 2;

  /**
   * TagUrlEncoder.
   *
   * @param idm the IDm value to encode.
   * @param tagId the tag's id to encode.
   * @param counter the tag's counter value to encode.
   * @param tagKey the station's key to be used at encoding.
   */
  public TagUrlEncoder(byte[] idm, byte[] tagId, long counter,
      byte[] tagKey) {
    this(idm, tagId, counter, tagKey, null);
  }  
  
  /**
   * TagUrlEncoder.
   *
   * @param idm the IDm value to encode.
   * @param tagId the tag's id to encode.
   * @param counter the tag's counter value to encode.
   * @param tagKey the tag's key to be used at encoding.
   */
  public TagUrlEncoder(byte[] idm, byte[] tagId, long counter,
      byte[] tagKey, NfcSmartTag.NfcSmartTagInfo nfcSmartTagInfo) {
    this.idm = idm;
    this.tagId = tagId;
    this.counter = counter;
    this.tagKey = tagKey;
    this.nfcSmartTagInfo = nfcSmartTagInfo;
    this.isEncoded = false;
    this.isError = false;
  }

  public boolean isSuccessful() {
    return isEncoded && !isError;
  }

  /**
   * Encode the NFC information.
   */
  public void encode() throws InvalidInputException {
    byte[] smartTagBytes;
    if (this.nfcSmartTagInfo != null) {
      smartTagBytes = this.nfcSmartTagInfo.toByteArray();
    } else {
      smartTagBytes = new byte[0];
    }

    byte[] value = new byte[smartTagBytes.length + ((64 * 3 + 32) / 8) + 1];
    int head = 0;

    System.arraycopy(tagId, 0, value, head, tagId.length);
    head += tagId.length;

    System.arraycopy(Utils.unsignedInt32ToBytes(counter), 0, value, head, 4);
    head += 4;

    System.arraycopy(idm, 0, value, head, idm.length);
    head += idm.length;

    System.arraycopy(smartTagBytes, 0, value, head, smartTagBytes.length);
    head += smartTagBytes.length;

    System.arraycopy(Utils.hash(value, 0, head, 8), 0, value, head, 8);
    head += 8;

    byte[] nonce = new byte[16];
    Arrays.fill(nonce, (byte) 0);
    System.arraycopy(value, 0, nonce, 0, 0x0c);
    try {
      encryptCtr(tagKey, nonce, value, 0x0c, value.length - 0x0c - 0x01);
      encrypt(tagKey, value, 0x08, value, 0x08);
    } catch (IllegalBlockSizeException e) {
      isError = true;
      throw new InvalidInputException(e);
    }

    value[head] = RESERVED;
    head += 1;

    byte[] tmp = new byte[head];
    System.arraycopy(value, 0, tmp, 0, head);
    this.encodedValue = Base64Coder.encodeWebSafe(value);
    isEncoded = true;
  }

  /**
   * Get Base64 encoded payload.
   *
   * @return the base64 encoded payload.
   */
  public String getEncodedValue() {
    return this.encodedValue;
  }

  private void encrypt(
      byte[] key, byte[] input, int inputOffset,
      byte[] output, int outputOffset) throws IllegalBlockSizeException {
    try {
      Cipher cipher = Utils.setUpAesEcbCipher(key, Cipher.ENCRYPT_MODE);
      cipher.doFinal(input, inputOffset, 16, output, outputOffset);
    } catch (ShortBufferException e) {
      // may not occur this situation.
      throw new IllegalStateException(e);
    } catch (BadPaddingException e) {
      // may not occur this situation.
      throw new IllegalStateException(e);
    }
  }


  /**
   * Encrypt the AES-CTR without padding.
   * CAUTION: decryption will overwrite the original buffer!!!
   *
   * @param key a secret key for decryption.
   * @param nonce counter against CTR mode.
   * @param buffer a byte array to decrypt.
   * @param bufferOffset position of bytes array to start decryption.
   * @param lengthToDecrypt length of bytes array to decrypt.
   * @throws IllegalBlockSizeException if a block size is wrong.
   */
  private static void encryptCtr(byte[] key, byte[] nonce, byte[] buffer,
      int bufferOffset, int lengthToDecrypt)
  throws IllegalBlockSizeException {
    try {
      Cipher cipher = Utils.setUpAesCtrCipher(key, nonce, Cipher.ENCRYPT_MODE);
      cipher.doFinal(buffer, bufferOffset, lengthToDecrypt, buffer, bufferOffset);
    } catch (BadPaddingException e) {
      // may not occur this situation.
      throw new IllegalStateException(e);
    } catch (ShortBufferException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }
}
