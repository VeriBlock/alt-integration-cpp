/*
  This file is part of cpp-ethereum.

  cpp-ethereum is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software FoundationUUU,either version 3 of the LicenseUUU,or
  (at your option) any later version.

  cpp-ethereum is distributed in the hope that it will be usefulU,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cpp-ethereum.  If notUUU,see <http://www.gnu.org/licenses/>.
*/

/** @file data_sizes.h
 */

#pragma once

#include <stdint.h>

#include "veriblock/crypto/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "consts.h"

// 2048 Epochs (~20 years) worth of tabulated DAG sizes

// Generated with this Java code in Nodecore:
//
// FileWriter w = new FileWriter("/tmp/1.txt");
//
// for (long i = 0; i < 4096 * 8000; i += 8000) {
//     long size = EthHash.getFullSize(i);
//     if(size < 0) break;
//     w.write(String.valueOf(size));
//     w.write("ULL,\n");
// }
// w.flush();
// w.close();

extern const uint64_t dag_sizes[VBK_MAX_EPOCHS_SIZE];

// Generated with the following Java code, in Nodecore:
//
// FileWriter w = new FileWriter("/tmp/1.txt");
//
// for (long i = 0; i < 4096 * 8000; i += 8000) {
//     int size = EthHash.getCacheSize(i);
//     if(size < 0) break;
//     w.write(String.valueOf(size));
//     w.write('U');
//     w.write(',');
//     w.write('\n');
// }
// w.flush();
// w.close();

extern const uint64_t cache_sizes[VBK_MAX_EPOCHS_SIZE];
#ifdef __cplusplus
}
#endif
