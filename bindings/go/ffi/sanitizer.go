// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #ifdef WITH_ASAN
// void __lsan_do_leak_check(void);
// #else
// void __lsan_do_leak_check(void) {}
// #endif
import "C"
import (
	"fmt"
	"runtime"
)

func doSanitizerCheck() {
	fmt.Println("Do the garbage collection")
	runtime.GC()
	fmt.Println("Run sanitizer check")
	C.__lsan_do_leak_check()
}
