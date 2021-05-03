// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// void __lsan_do_leak_check(void);
import "C"

func DoLeakCheck() {
	C.__lsan_do_leak_check()
}
