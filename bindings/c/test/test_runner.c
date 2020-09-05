// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <assert.h>
#include <string.h>

extern void extern_test_suite();

extern void config_test_suite();

int main() {
  extern_test_suite();
  config_test_suite();

  return 0;
}