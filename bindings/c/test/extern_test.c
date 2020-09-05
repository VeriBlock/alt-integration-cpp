// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <assert.h>
#include <string.h>

#include "../lib/extern.h"

// AltChainConfig externs
int64_t VBK_getAltchainId() { return 25; }

const char* VBK_getBootstrapBlock() { return "test_header"; }

void VBK_getBlockHeaderHash(const uint8_t* in,
                            int inlen,
                            uint8_t* out,
                            int* outlen) {
  memcpy(out, in, inlen);
  *outlen = inlen;
}

// PayloadsProvider externs
void VBK_getATV(const uint8_t* id_bytes,
                int id_size,
                uint8_t* atv_bytes_out,
                int* atv_bytes_len) {
  memcpy(atv_bytes_out, id_bytes, id_size);
  *atv_bytes_len = id_size;
}

void VBK_getVTB(const uint8_t* id_bytes,
                int id_size,
                uint8_t* vtb_bytes_out,
                int* vtb_bytes_len) {
  memcpy(vtb_bytes_out, id_bytes, id_size);
  *vtb_bytes_len = id_size;
}

void VBK_getVBK(const uint8_t* id_bytes,
                int id_size,
                uint8_t* vbk_bytes_out,
                int* vbk_bytes_len) {
  memcpy(vbk_bytes_out, id_bytes, id_size);
  *vbk_bytes_len = id_size;
}

// tests
void VBK_getAltchainId_test() {
  int res = VBK_getAltchainId();
  assert(res == 25);
}

void VBK_getBootstrapBlock_test() {
  const char* in_value = "hello_world";
  const char out_value[12] = "";
  int out_size = 0;
  VBK_getBlockHeaderHash(
      (const uint8_t*)in_value, 12, (uint8_t*)&out_value, &out_size);

  assert(strcmp(in_value, out_value) == 0);
  assert(out_size == 12);
}

void VBK_getBlockHeaderHash_test() {
  const char* header = VBK_getBootstrapBlock();
  assert(strcmp(header, "test_header") == 0);
}

void VBK_getATV_test() {
  const char* in_value = "hello_world";
  const char out_value[12] = "";
  int out_size = 0;

  VBK_getATV((const uint8_t*)in_value, 12, (uint8_t*)&out_value, &out_size);

  assert(strcmp(in_value, out_value) == 0);
  assert(out_size == 12);
}

void VBK_getVTB_test() {
  const char* in_value = "hello_world";
  const char out_value[12] = "";
  int out_size = 0;

  VBK_getVTB((const uint8_t*)in_value, 12, (uint8_t*)&out_value, &out_size);

  assert(strcmp(in_value, out_value) == 0);
  assert(out_size == 12);
}

void VBK_getVBK_test() {
  const char* in_value = "hello_world";
  const char out_value[12] = "";
  int out_size = 0;

  VBK_getVBK((const uint8_t*)in_value, 12, (uint8_t*)&out_value, &out_size);

  assert(strcmp(in_value, out_value) == 0);
  assert(out_size == 12);
}

void extern_test_suite() {
  VBK_getAltchainId_test();
  VBK_getBootstrapBlock_test();
  VBK_getBlockHeaderHash_test();
  VBK_getATV_test();
  VBK_getVTB_test();
  VBK_getVBK_test();
}