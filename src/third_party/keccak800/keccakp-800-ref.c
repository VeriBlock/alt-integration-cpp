/*
The eXtended Keccak Code Package (XKCP)
https://github.com/XKCP/XKCP

The Keccak-p permutations, designed by Guido Bertoni, Joan Daemen, Michaël
Peeters and Gilles Van Assche.

Implementation by Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to the Keccak Team
website: https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/

---

This file implements Keccak-p[800] in a SnP-compatible way.
Please refer to SnP-documentation.h for more details.

This implementation comes with KeccakP-800-SnP.h in the same folder.
Please refer to LowLevel.build for the exact list of other files it must be
combined with.
*/

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "brg_endian.h"

typedef uint32_t tKeccakLane;

#define maxNrRounds 22
#define nrLanes 25
#define index(x, y) (((x) % 5) + 5 * ((y) % 5))

static const tKeccakLane KeccakRoundConstants[maxNrRounds] = {
    0x00000001, 0x00008082, 0x0000808a, 0x80008000, 0x0000808b, 0x80000001,
    0x80008081, 0x00008009, 0x0000008a, 0x00000088, 0x80008009, 0x8000000a,
    0x8000808b, 0x0000008b, 0x00008089, 0x00008003, 0x00008002, 0x00000080,
    0x0000800a, 0x8000000a, 0x80008081, 0x00008080,
};

static const unsigned int KeccakRhoOffsets[nrLanes] = {
    0,  1, 30, 28, 27, 4,  12, 6,  23, 20, 3,  10, 11,
    25, 7, 9,  13, 15, 21, 8,  18, 2,  29, 24, 14};

/* ---------------------------------------------------------------- */

void KeccakP800_Initialize(void *state) { memset(state, 0, 800 / 8); }

/* ---------------------------------------------------------------- */

void KeccakP800_AddByte(void *state, unsigned char byte, unsigned int offset) {
  assert(offset < 100);
  ((unsigned char *)state)[offset] ^= byte;
}

/* ---------------------------------------------------------------- */

void KeccakP800_AddBytes(void *state,
                         const unsigned char *data,
                         unsigned int offset,
                         unsigned int length) {
  unsigned int i;

  assert(offset < 100);
  assert(offset + length <= 100);
  for (i = 0; i < length; i++) ((unsigned char *)state)[offset + i] ^= data[i];
}

/* ---------------------------------------------------------------- */

void KeccakP800_OverwriteBytes(void *state,
                               const unsigned char *data,
                               unsigned int offset,
                               unsigned int length) {
  assert(offset < 100);
  assert(offset + length <= 100);
  memcpy((unsigned char *)state + offset, data, length);
}

/* ---------------------------------------------------------------- */

void KeccakP800_OverwriteWithZeroes(void *state, unsigned int byteCount) {
  assert(byteCount <= 100);
  memset(state, 0, byteCount);
}

/* ---------------------------------------------------------------- */

static void fromBytesToWords(tKeccakLane *stateAsWords,
                             const unsigned char *state);
static void fromWordsToBytes(unsigned char *state,
                             const tKeccakLane *stateAsWords);
void KeccakP800OnWords(tKeccakLane *state, unsigned int nrRounds);
void KeccakP800Round(tKeccakLane *state, unsigned int indexRound);
static void theta(tKeccakLane *A);
static void rho(tKeccakLane *A);
static void pi(tKeccakLane *A);
static void chi(tKeccakLane *A);
static void iota(tKeccakLane *A, unsigned int indexRound);

void KeccakP800_Permute_Nrounds(void *state, unsigned int nrounds) {
#if (PLATFORM_BYTE_ORDER != IS_LITTLE_ENDIAN)
  tKeccakLane stateAsWords[800 / 32];
#endif

#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
  KeccakP800OnWords((tKeccakLane *)state, nrounds);
#else
  fromBytesToWords(stateAsWords, (const unsigned char *)state);
  KeccakP800OnWords(stateAsWords, nrounds);
  fromWordsToBytes((unsigned char *)state, stateAsWords);
#endif
}

void KeccakP800_Permute_12rounds(void *state) {
#if (PLATFORM_BYTE_ORDER != IS_LITTLE_ENDIAN)
  tKeccakLane stateAsWords[800 / 32];
#endif

#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
  KeccakP800OnWords((tKeccakLane *)state, 12);
#else
  fromBytesToWords(stateAsWords, (const unsigned char *)state);
  KeccakP800OnWords(stateAsWords, 12);
  fromWordsToBytes((unsigned char *)state, stateAsWords);
#endif
}

void KeccakP800_Permute_22rounds(void *state) {
#if (PLATFORM_BYTE_ORDER != IS_LITTLE_ENDIAN)
  tKeccakLane stateAsWords[800 / 32];
#endif

#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
  KeccakP800OnWords((tKeccakLane *)state, 22);
#else
  fromBytesToWords(stateAsWords, (const unsigned char *)state);
  KeccakP800OnWords(stateAsWords, 22);
  fromWordsToBytes((unsigned char *)state, stateAsWords);
#endif
}

static void fromBytesToWords(tKeccakLane *stateAsWords,
                             const unsigned char *state) {
  unsigned int i, j;

  for (i = 0; i < nrLanes; i++) {
    stateAsWords[i] = 0;
    for (j = 0; j < sizeof(tKeccakLane); j++)
      stateAsWords[i] |= (tKeccakLane)(state[i * sizeof(tKeccakLane) + j])
                         << (8 * j);
  }
}

static void fromWordsToBytes(unsigned char *state,
                             const tKeccakLane *stateAsWords) {
  unsigned int i, j;

  for (i = 0; i < nrLanes; i++)
    for (j = 0; j < sizeof(tKeccakLane); j++)
      state[i * sizeof(tKeccakLane) + j] = (stateAsWords[i] >> (8 * j)) & 0xFF;
}

void KeccakP800OnWords(tKeccakLane *state, unsigned int nrRounds) {
  unsigned int i;

#ifdef KeccakReference
  displayStateAsLanes(3, "Same, with lanes as 32-bit words", state, 800);
#endif

  for (i = (maxNrRounds - nrRounds); i < maxNrRounds; i++)
    KeccakP800Round(state, i);
}

void KeccakP800Round(tKeccakLane *state, unsigned int indexRound) {
#ifdef KeccakReference
  displayRoundNumber(3, indexRound);
#endif

  theta(state);
#ifdef KeccakReference
  displayStateAsLanes(3, "After theta", state, 800);
#endif

  rho(state);
#ifdef KeccakReference
  displayStateAsLanes(3, "After rho", state, 800);
#endif

  pi(state);
#ifdef KeccakReference
  displayStateAsLanes(3, "After pi", state, 800);
#endif

  chi(state);
#ifdef KeccakReference
  displayStateAsLanes(3, "After chi", state, 800);
#endif

  iota(state, indexRound);
#ifdef KeccakReference
  displayStateAsLanes(3, "After iota", state, 800);
#endif
}

#define ROL32(a, offset)                                                      \
  ((offset != 0) ? ((((tKeccakLane)a) << offset) ^                            \
                    (((tKeccakLane)a) >> (sizeof(tKeccakLane) * 8 - offset))) \
                 : a)

static void theta(tKeccakLane *A) {
  unsigned int x, y;
  tKeccakLane C[5], D[5];

  for (x = 0; x < 5; x++) {
    C[x] = 0;
    for (y = 0; y < 5; y++) C[x] ^= A[index(x, y)];
  }
  for (x = 0; x < 5; x++) D[x] = ROL32(C[(x + 1) % 5], 1) ^ C[(x + 4) % 5];
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++) A[index(x, y)] ^= D[x];
}

static void rho(tKeccakLane *A) {
  unsigned int x, y;

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      A[index(x, y)] = ROL32(A[index(x, y)], KeccakRhoOffsets[index(x, y)]);
}

static void pi(tKeccakLane *A) {
  unsigned int x, y;
  tKeccakLane tempA[25];

  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++) tempA[index(x, y)] = A[index(x, y)];
  for (x = 0; x < 5; x++)
    for (y = 0; y < 5; y++)
      A[index(0 * x + 1 * y, 2 * x + 3 * y)] = tempA[index(x, y)];
}

static void chi(tKeccakLane *A) {
  unsigned int x, y;
  tKeccakLane C[5];

  for (y = 0; y < 5; y++) {
    for (x = 0; x < 5; x++)
      C[x] = A[index(x, y)] ^ ((~A[index(x + 1, y)]) & A[index(x + 2, y)]);
    for (x = 0; x < 5; x++) A[index(x, y)] = C[x];
  }
}

static void iota(tKeccakLane *A, unsigned int indexRound) {
  A[index(0, 0)] ^= KeccakRoundConstants[indexRound];
}

/* ---------------------------------------------------------------- */

void KeccakP800_ExtractBytes(const void *state,
                             unsigned char *data,
                             unsigned int offset,
                             unsigned int length) {
  assert(offset < 100);
  assert(offset + length <= 100);
  memcpy(data, (unsigned char *)state + offset, length);
}

/* ---------------------------------------------------------------- */

void KeccakP800_ExtractAndAddBytes(const void *state,
                                   const unsigned char *input,
                                   unsigned char *output,
                                   unsigned int offset,
                                   unsigned int length) {
  unsigned int i;

  assert(offset < 100);
  assert(offset + length <= 100);
  for (i = 0; i < length; i++)
    output[i] = input[i] ^ ((unsigned char *)state)[offset + i];
}
