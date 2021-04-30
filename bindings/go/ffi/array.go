// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

package ffi

// #cgo pkg-config: veriblock-pop-cpp
// #include <veriblock/pop/c/array.h>
// #include <string.h>
// #include <stdlib.h>
import "C"
import (
	"runtime"
	"unsafe"
)

func freeArrayU8(array *C.pop_array_u8_t) {
	C.pop_array_u8_free(array)
}

func createBytes(array *C.pop_array_u8_t) []byte {
	res := make([]byte, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return res
}

func freeArrayArrayU8(array *C.pop_array_array_u8_t) {
	C.pop_array_array_u8_free(array)
}

func createArrayOfArraysU8(array *C.pop_array_array_u8_t)

func freeArrayChar(array *C.pop_array_string_t) {
	C.pop_array_string_free(array)
}

func createString(array *C.pop_array_string_t) string {
	res := make([]byte, array.size)
	if array.size != 0 {
		C.memcpy(unsafe.Pointer(&res[0]), unsafe.Pointer(array.data), array.size)
	}
	return string(res)
}

func createCString(str string) (res C.pop_array_string_t) {
	res.size = C.size_t(len(str))
	res.data = C.CString(str)
	runtime.SetFinalizer(&res, func(v *C.pop_array_string_t) {
		// we use C.free because CString is allocated memory using malloc
		C.free(unsafe.Pointer(v.data))
	})
	return res
}
