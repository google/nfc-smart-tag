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
 */

package com.appspot.nfcsmarttag.url; 

import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * Utility class for Url4Tag / TagUrlEncoder.
 */
public class Utils {
  /**
   * Convert bytes to long assuming the value is little endian.
   *
   * @param src the source to be decoded.
   * @param srcPos the position of the source to be decoded.
   * @param length the length of source to be decoded.
   * @return decoded uint32t value.
   */
  public static long littleBytesToLong(byte[] src, int srcPos, int length) {
    long ret = 0;
    for (int i = length - 1; i >= 0; i--) {
      ret *= 0x100;
      ret |= src[srcPos + i] & 0xff;
    }
    return ret;
  }

  /**
   * Convert unsigned int 32 bit value in long to the byte array.
   *
   * @param v the unsigned int value to be converted.
   * @return converted byte array.
   */
  public static byte[] unsignedInt32ToBytes(long v) {
    byte[] ary = new byte[4];
    for (int i = 0; i < 4; i++) {
      ary[i] = (byte)(v & 0xff);
      v >>= 8;
    }
    return ary;
  }

  /**
   * Calc SHA1 hash value with specified size.
   *
   * @param source bytes to calc hash.
   * @param sourcePosition position to start calc.
   * @param sourceLength length to calc hash.
   * @param size the size of digest.
   * @return hash value calculated from given source.
   */
  public static byte[] hash(byte[] source, int sourcePosition, int sourceLength, int size) {
    try {
      MessageDigest md = MessageDigest.getInstance("SHA1");
      md.update(source, sourcePosition, sourceLength);
      byte[] digest = new byte[size];
      System.arraycopy(md.digest(), 0, digest, 0, digest.length);
      return digest;
    } catch (NoSuchAlgorithmException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }

  public static Cipher setUpAesEcbCipher(byte[] key, int mode) {
    try {
      Cipher cipher = Cipher.getInstance("AES/ECB/NoPadding");
      SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
      cipher.init(mode, secretKeySpec);
      return cipher;
    } catch (NoSuchAlgorithmException e) {
      // may not occur.
      throw new IllegalStateException(e);
    } catch (NoSuchPaddingException e) {
      // may not occur.
      throw new IllegalStateException(e);
    } catch (InvalidKeyException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }

  public static Cipher setUpAesCtrCipher(byte[] key, byte[] nonce, int mode) {
    try {
      Cipher cipher = Cipher.getInstance("AES/CTR/NoPadding");
      SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
      cipher.init(mode, secretKeySpec, new IvParameterSpec(nonce));
      return cipher;
    } catch (NoSuchAlgorithmException e) {
      // may not occur.
      throw new IllegalStateException(e);
    } catch (NoSuchPaddingException e) {
      // may not occur.
      throw new IllegalStateException(e);
    } catch (InvalidKeyException e) {
      // may not occur.
      throw new IllegalStateException(e);
    } catch (InvalidAlgorithmParameterException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }
}
