/* Copyright (c) 2001 IETF Trust and the persons identified as authors
 * of the code.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of Internet Society, IETF or IETF Trust, nor the names
 *   of specific contributors, may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Simple SHA-1 implementation derived from: http://www.ietf.org/rfc/rfc3174.txt
 *
 * Optimizations:
 * - Limit message length to 64kBits (8kBytes)
 * - Replace circular shift with assembly (2x speed-up)
 * - Do check overflow etc as our message size is naturally limited
 * - Do not check context state and null arguments
 */

#include <stdint.h>
#include <stdbool.h>

#include "avr_sha1.h"

typedef struct SHA1Context
{
  uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */
  uint16_t Length;             /* Message length in bits         */
  uint8_t Message_Block_Index; /* Index into message block array */
  uint8_t Message_Block[64];   /* 512-bit message blocks         */
} SHA1Context;

static void SHA1Reset(SHA1Context *);
static void SHA1Input(SHA1Context *, const uint8_t *, unsigned int);
static void SHA1Result(SHA1Context *, uint8_t Message_Digest[SHA1HashSize]);
static void SHA1PadMessage(SHA1Context *);
static void SHA1ProcessMessageBlock(SHA1Context *);


/*
 *  Define the SHA1 circular left shift macro
 */
#define SHA1CircularShift(bits,word) \
                (((word) << (bits)) | ((word) >> (32-(bits))))

/*
 * Optimized version of a cyclic left shift by 5.
 */
static uint32_t SHA1CircularShift5(uint32_t word) {
 uint8_t count = 5;

 asm volatile(
      "L_%=: lsl %A0\n\t"
      "rol %B0\n\t"
      "rol %C0\n\t"
      "rol %D0\n\t"
      "brcc C_%=\n\t"
      "ori %A0, 0x01\n\t"
      "C_%=: dec %A1\n\t"
      "BRNE L_%=\n\t"
      : "+d" (word) : "r" (count) : "r20");
  return word;
}

/*
 * Optimized version of a cyclic left shift by 30.
 */
static uint32_t SHA1CircularShift30(uint32_t word) {
 uint8_t count = 2; // Shift right 2 instead of 30 left

 asm volatile(
      "L_%=: lsr %D0\n\t"
      "ror %C0\n\t"
      "ror %B0\n\t"
      "ror %A0\n\t"
      "brcc C_%=\n\t"
      "ori %D0, 0x80\n\t"
      "C_%=: dec %A1\n\t"
      "BRNE L_%=\n\t"
      : "+d" (word) : "r" (count) : "r20");
  return word;
}

/*
 * Swap byte order in a word, from little endian to big endian
 * or vice versa.
 */
static uint32_t swap32(uint32_t value) {
  asm volatile(
      "mov __tmp_reg__, %A0" "\n\t"
      "mov %A0, %D0"         "\n\t"
      "mov %D0, __tmp_reg__" "\n\t"
      "mov __tmp_reg__, %B0" "\n\t"
      "mov %B0, %C0"         "\n\t"
      "mov %C0, __tmp_reg__" "\n\t"
      : "+r" (value));
  return value;
}

/*
 *  SHA1Reset
 *
 *  Description:
 *      This function will initialize the SHA1Context in preparation
 *      for computing a new SHA1 message digest.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to reset.
 *
 */
static void SHA1Reset(SHA1Context *context)
{
  context->Length                 = 0;
  context->Message_Block_Index    = 0;

  context->Intermediate_Hash[0]   = 0x67452301;
  context->Intermediate_Hash[1]   = 0xEFCDAB89;
  context->Intermediate_Hash[2]   = 0x98BADCFE;
  context->Intermediate_Hash[3]   = 0x10325476;
  context->Intermediate_Hash[4]   = 0xC3D2E1F0;
}

/*
 *  SHA1Result
 *
 *  Description:
 *      This function will return the 160-bit message digest into the
 *      Message_Digest array  provided by the caller.
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to use to calculate the SHA-1 hash.
 *      Message_Digest: [out]
 *          Where the digest is returned.
 *
 *  Optimizations:
 *  - No need to clear results
 *  - Word swap via assembly code
 */
static void SHA1Result(SHA1Context *context,
                       uint8_t Message_Digest[SHA1HashSize])
{
  uint8_t i;

  SHA1PadMessage(context);

  for (i = 0; i < SHA1HashSize / 4; ++i) {
    ((uint32_t *)Message_Digest)[i] = swap32(context->Intermediate_Hash[i]);
  }
}

/*
 *  SHA1Input
 *
 *  Description:
 *      This function accepts an array of octets as the next portion
 *      of the message.
 *
 *  Parameters:
 *      context: [in/out]
 *          The SHA context to update
 *      message_array: [in]
 *          An array of characters representing the next portion of
 *          the message.
 *      length: [in]
 *          The length of the message in message_array
 *
 */
static void SHA1Input(SHA1Context *context,
                     const uint8_t  *message_array,
                     unsigned length)
{
  while(length--) {
    context->Message_Block[context->Message_Block_Index++] =
        (*message_array & 0xFF);

    context->Length += 8;

    if (context->Message_Block_Index == 64) {
      SHA1ProcessMessageBlock(context);
    }
    message_array++;
  }
}

/*
 *  SHA1ProcessMessageBlock
 *
 *  Description:
 *      This function will process the next 512 bits of the message
 *      stored in the Message_Block array.
 *
 *  Parameters:
 *      None.
 *
 *  Returns:
 *      Nothing.
 *
 *  Comments:

 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 * Optimizations:
 * - Merged into single loop to reduce code size
 * - Compute W on the fly to save stack space
 *
 */
static void SHA1ProcessMessageBlock(SHA1Context *context)
{
    const uint32_t K[] =    {       /* Constants defined in SHA-1   */
                            0x5A827999,
                            0x6ED9EBA1,
                            0x8F1BBCDC,
                            0xCA62C1D6
                            };
    uint8_t   t;                 /* Loop counter                */
    uint32_t  temp;              /* Temporary word value        */
    uint32_t  W[16];             /* Word sequence               */
    uint32_t  A, B, C, D, E;     /* Word buffers                */

    //  Initialize the first 16 words in the array W
    for(t = 0; t < 16; t++) {
      W[t] = swap32(((uint32_t *)(context->Message_Block))[t]);
    }

    A = context->Intermediate_Hash[0];
    B = context->Intermediate_Hash[1];
    C = context->Intermediate_Hash[2];
    D = context->Intermediate_Hash[3];
    E = context->Intermediate_Hash[4];

   for (t = 0; t < 80; t++) {
      // Compute W past 16 on the fly
      if (t >= 16) {
        W[t & 15] = SHA1CircularShift(1,W[(t-3) & 15] ^ W[(t-8) & 15]
                    ^ W[(t-14) & 15] ^ W[(t-16) & 15]);
      }
      temp = SHA1CircularShift5(A) + E + W[t & 15];
      if (t < 20) {
        temp += ((B & C) | ((~B) & D)) + K[0];
      }
      else if ( t < 40) {
        temp += (B ^ C ^ D) + K[1];
      } else if (t < 60) {
        temp += ((B & C) | (B & D) | (C & D)) + K[2];
      } else {
        temp += (B ^ C ^ D) + K[3];
      }
      E = D;
      D = C;
      C = SHA1CircularShift30(B);
      B = A;
      A = temp;
    }

    context->Intermediate_Hash[0] += A;
    context->Intermediate_Hash[1] += B;
    context->Intermediate_Hash[2] += C;
    context->Intermediate_Hash[3] += D;
    context->Intermediate_Hash[4] += E;

    context->Message_Block_Index = 0;
}

/*
 *  SHA1PadMessage
 *

 *  Description:
 *      According to the standard, the message must be padded to an even
 *      512 bits.  The first padding bit must be a '1'.  The last 64
 *      bits represent the length of the original message.  All bits in
 *      between should be 0.  This function will pad the message
 *      according to those rules by filling the Message_Block array
 *      accordingly.  It will also call the ProcessMessageBlock function
 *      provided appropriately.  When it returns, it can be assumed that
 *      the message digest has been computed.
 *
 *  Parameters:
 *      context: [in/out]
 *          The context to pad
 *      ProcessMessageBlock: [in]
 *          The appropriate SHA*ProcessMessageBlock function
 *  Returns:
 *      Nothing.
 *
 */

static void SHA1PadMessage(SHA1Context *context)
{  /*
   *  Check to see if the current message block is too small to hold
   *  the initial padding bits and length.  If so, we will pad the
   *  block, process it, and then continue padding into a second
   *  block.
   */
  if (context->Message_Block_Index > 55) {
    context->Message_Block[context->Message_Block_Index++] = 0x80;
    while(context->Message_Block_Index < 64) {
      context->Message_Block[context->Message_Block_Index++] = 0;
    }

    SHA1ProcessMessageBlock(context);

    while(context->Message_Block_Index < 56) {
      context->Message_Block[context->Message_Block_Index++] = 0;
    }
  } else {
    context->Message_Block[context->Message_Block_Index++] = 0x80;
    while(context->Message_Block_Index < 56) {
      context->Message_Block[context->Message_Block_Index++] = 0;
    }
  }

  //  Store the message length as the last 8 octets
  context->Message_Block[56] = 0;
  context->Message_Block[57] = 0;
  context->Message_Block[58] = 0;
  context->Message_Block[59] = 0;
  context->Message_Block[60] = 0;
  context->Message_Block[61] = 0;
  context->Message_Block[62] = context->Length >> 8;
  context->Message_Block[63] = context->Length;

  SHA1ProcessMessageBlock(context);
}

/*
 * Compute a SHA1 hash from the supplied buffer.
 *
 * Arguments
 * hash: the resulting has value
 * buffer: the data to be hashed
 * buffer_size: the buffer size in bytes
 */
void sha1(sha1_hash_t hash, uint8_t *buffer, uint16_t buffer_size)
{
  SHA1Context sha;

  SHA1Reset(&sha);
  SHA1Input(&sha, buffer, buffer_size);
  SHA1Result(&sha, hash);
}
