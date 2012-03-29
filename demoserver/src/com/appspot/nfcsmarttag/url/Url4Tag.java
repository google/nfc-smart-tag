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
 * Class to hold NFC Smart Tag URl data. 
 * Can be instantiated from an encoded URL.
 */

package com.appspot.nfcsmarttag.url; 

import com.google.protobuf.InvalidProtocolBufferException;

import org.keyczar.exceptions.Base64DecodingException;
import org.keyczar.util.Base64Coder;

import java.util.Arrays;
import java.util.List;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.ShortBufferException;

/**
 * A class for decoding the NFC Smart Tag parameter.
 */
public class Url4Tag {
  private final byte[] tagId;
  private final byte[] idm;
  private final long counter;
  private final NfcSmartTag.NfcSmartTagInfo nfcSmartTagInfo;

  public static final int IDM_LENGTH = 8;
  public static final int STATION_ID_LENGTH = 8;

  /**
   * Use {@link #fromEncodedValue(SmartTagKeyStoreInterface, String)} to get an instance.
   */
  protected Url4Tag(byte[] tagId, byte[] idm, long counter,
      NfcSmartTag.NfcSmartTagInfo nfcSmartTagInfo) {
    this.tagId = tagId;
    this.idm = idm;
    this.counter = counter;
    this.nfcSmartTagInfo = nfcSmartTagInfo;
  }

  private static boolean isEmptyOrWhitespace(String string) {
    return string == null || string.trim().isEmpty();
  }

  /**
   * Decodes an encoded URL. References a {@link SmartTagKeyStoreInterface} that
   * maps the tagID to a tagKey.
   *
   * @param keyStore Key Store object that stores tag keys.
   * @param encodedValue Base64 encoded value specified at the URL parameter 'v'.
   * @return an instance of Url4Tag.
   * @throws InvalidInputException on error.
   */
  public static Url4Tag fromEncodedValue(SmartTagKeyStoreInterface keyStore,
                                         String encodedValue) throws InvalidInputException {
    try {
      byte[] value;

      if (keyStore == null) {
        throw new InvalidInputException("keyStore is null");
      }

      if (isEmptyOrWhitespace(encodedValue)) {
        throw new InvalidInputException("encodedValue is empty or white space");
      }

      value = Base64Coder.decodeWebSafe(encodedValue);
      return decode(value, keyStore);
    } catch (IllegalBlockSizeException e) {
      throw new InvalidInputException("URL decoding failed.", e);
    } catch (Base64DecodingException e) {
      throw new InvalidInputException("URL decoding failed.", e);
    } catch (InvalidProtocolBufferException e) {
      throw new InvalidInputException("URL decoding failed.", e);
    }
  }

  /**
   * Decode an URL for the NFC smart tag.
   *
   * @param value Base64 decoded value to be decrypted.
   * @param keyStore Key Store object that stores tag keys.
   * @return an instance of Url4Tag.
   * @throws InvalidInputException if input is broken.
   * @throws IllegalBlockSizeException will not usually occur.
   * @throws InvalidProtocolBufferException if payload protocol buffer is broken.
   */
  private static Url4Tag decode(byte[] value, SmartTagKeyStoreInterface keyStore)
    throws InvalidInputException, IllegalBlockSizeException, InvalidProtocolBufferException {
    List<byte[]> tagKeys;

    /*
     * Transmitted information:
     *
     * 0x00  : Tag ID (8 bytes)
     * 0x08  : Counter (4 bytes, encrypted)
     * 0x0c  : Felica ID (8 bytes, encrypted)
     * 0x14  : arbitrary data(n bytes, encrypted)
     * 0x14+n: First 64 bits of SHA1 hash value (8 bytes, encrypted)
     * 0x1c+n: reserved (1 byte)
     */
    final int STATION_ID_POSITION = 0x00;
    final int COUNTER_POSITION = 0x08;
    final int IDM_POSITION = 0x0c;
    final int CTR_NONCE_POSITION = 0x00;
    final int CTR_NONCE_SIZE = 0x0c; // remaining 0x04 bytes should be filled with 0x00.
    final int STATION_INFO_POSITION = 0x14;
    final int CHECKSUM_SIZE = 0x08;
    final int RESERVED_SIZE = 0x01;

    byte[] tagId = new byte[STATION_ID_LENGTH];
    System.arraycopy(value, STATION_ID_POSITION, tagId, 0, tagId.length);
    tagKeys = keyStore.getKeys(tagId);
    if (tagKeys.isEmpty()) {
      throw new InvalidInputException("Tag key not found.");
    }

    for (byte[] tagKey : tagKeys) {
      byte[] tmp = value.clone();
      // decrypt encrypted counter (4 bytes) + 12 bytes
      decrypt(tagKey, tmp, COUNTER_POSITION);
      long counter = Utils.littleBytesToLong(tmp, COUNTER_POSITION, 4);

      // decrypt FeliCa ID + arbitrary data + hash value.
      // nonce is tag id + counter + internal counter that is incremented in each block.
      byte[] nonce = new byte[16];
      System.arraycopy(tmp, CTR_NONCE_POSITION, nonce, 0, CTR_NONCE_SIZE);
      decryptCtr(tagKey, nonce, tmp, CTR_NONCE_SIZE,
          tmp.length - CTR_NONCE_SIZE - RESERVED_SIZE);

      // verifies checksum
      byte[] checkSum = new byte[CHECKSUM_SIZE];
      System.arraycopy(tmp, tmp.length - RESERVED_SIZE - CHECKSUM_SIZE, checkSum, 0,
          checkSum.length);
      byte[] digest = Utils.hash(tmp, 0, tmp.length - RESERVED_SIZE - CHECKSUM_SIZE,
          checkSum.length);
      if (Arrays.equals(checkSum, digest)) {
        byte[] idm = new byte[IDM_LENGTH];
        System.arraycopy(tmp, IDM_POSITION, idm, 0, idm.length);

        int payload_length = tmp.length - STATION_INFO_POSITION -CHECKSUM_SIZE - RESERVED_SIZE;
        byte[] payload = new byte[payload_length];
        System.arraycopy(tmp, STATION_INFO_POSITION, payload, 0, payload_length);
        NfcSmartTag.NfcSmartTagInfo nfcSmartTagInfo = NfcSmartTag.NfcSmartTagInfo.parseFrom(payload);
        
        return new Url4Tag(tagId, idm, counter, nfcSmartTagInfo);
      }
    }
    throw new InvalidInputException("Cannot find the tag key to decode.");
  }

  /**
   * Returns a tag id.
   *
   * @return tag id number, or null if failed to decode the URL.
   */
  public byte[] getTagId() {
    return tagId;
  }

  /**
   * Returns a FeliCa id (aka. IDm).
   *
   * @return IDm, or null if failed to decode the URL.
   */
  public byte[] getIdm() {
    return idm;
  }

  /**
   * Returns a counter value of the smart tag.
   *
   * @return counter value, or -1 if failed to decode the URL.
   */
  public long getCounter() {
    return counter;
  }
  
  /**
   * Returns an instance of NfcSmartTagInfo protocol buffer come from the smart tag.
   * 
   * @return an instance of NfcSmartTagInfo protocol buffer.
   */
  public NfcSmartTag.NfcSmartTagInfo getNfcSmartTagInfo() {
    return nfcSmartTagInfo;
  }

  /**
   * Decrypt the AES block cipher.
   * CAUTION: decryption will overwrite the original buffer!!!
   *
   * @param key a secret key for decryption.
   * @param input a byte array to decrypt.
   * @param inputOffset a position of byte array to start decryption.
   * @throws IllegalBlockSizeException if a block size is wrong.
   */
  private static void decrypt(byte[] key, byte[] input, int inputOffset)
  throws IllegalBlockSizeException {
    try {
      Cipher cipher = Utils.setUpAesEcbCipher(key, Cipher.DECRYPT_MODE);
      cipher.doFinal(input, inputOffset, 16, input, inputOffset);
    } catch (BadPaddingException e) {
      // may not occur this situation.
      throw new IllegalStateException(e);
    } catch (ShortBufferException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }

  /**
   * Decrypt the AES-CTR without padding.
   * CAUTION: decryption will overwrite the original buffer!!!
   *
   * @param key a secret key for decryption.
   * @param nonce counter against CTR mode.
   * @param input a byte array to decrypt.
   * @param inputOffset position of bytes array to start decryption.
   * @param inputLength length of bytes array to decrypt.
   * @throws IllegalBlockSizeException if a block size is wrong.
   */
  private static void decryptCtr(byte[] key, byte[] nonce, byte[] input, int inputOffset,
      int inputLength)
  throws IllegalBlockSizeException {
    try {
      Cipher cipher = Utils.setUpAesCtrCipher(key, nonce, Cipher.DECRYPT_MODE);
      cipher.doFinal(input, inputOffset, inputLength, input, inputOffset);
    } catch (BadPaddingException e) {
      // may not occur this situation.
      throw new IllegalStateException(e);
    } catch (ShortBufferException e) {
      // may not occur.
      throw new IllegalStateException(e);
    }
  }
}
