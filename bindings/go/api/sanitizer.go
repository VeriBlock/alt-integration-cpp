// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package api

// #ifdef WITH_ASAN
// void __lsan_do_leak_check(void);
// #else
// void __lsan_do_leak_check(void) {}
// #endif
import "C"

func doSanitizerCheck() {
	C.__lsan_do_leak_check()
}
